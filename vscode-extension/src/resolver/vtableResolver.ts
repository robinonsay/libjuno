/**
 * @file vtableResolver.ts
 *
 * Implements the line-based Vtable Call Resolution algorithm (design §5.1).
 *
 * Given a cursor position (file, line, column) and the text of the line,
 * VtableResolver resolves the LibJuno vtable API call under the cursor to
 * one or more concrete function implementation locations.
 *
 * Resolution strategy (simplified line-based form of the chain-walk algorithm):
 *   1. JUNO_MODULE_GET_API macro pattern — explicit root type in source.
 *   2. Array-subscript chain — word[expr](accessors)->field(.
 *   3. General access chain — word(accessors)->field(.
 *
 * For each pattern, the primary identifier's type is looked up in localTypeInfo
 * for the enclosing function. The type is walked up the derivation chain to its
 * root, then mapped to an API type via moduleRoots / traitRoots. The
 * apiMemberRegistry is used as a secondary candidate when the matched chain
 * contains a named-API member (e.g. "ptHeapPointerApi"). A field-name search
 * across all apiStructFields provides the final fallback.
 */

import { NavigationIndex, VtableResolutionResult, ConcreteLocation } from '../parser/types';
import {
    findEnclosingFunction,
    lookupVariableType,
    walkToRootType,
    parseIntermediates,
} from './resolverUtils';

/**
 * Resolves LibJuno vtable API call sites to their concrete implementations.
 */
export class VtableResolver {
    constructor(private readonly index: NavigationIndex) {}

    /**
     * Resolve the vtable call at the given cursor position.
     *
     * @param file          Absolute path of the C source file.
     * @param line          1-based line number of the cursor.
     * @param column        0-based column number of the cursor within lineText.
     * @param lineText      Full text of the source line at the cursor position.
     * @param functionName  Optional: name of the enclosing C function. When
     *                      omitted, the enclosing function is inferred from
     *                      the NavigationIndex function definitions.
     * @returns VtableResolutionResult with found locations or an error message.
     */
    resolve(
        file: string,
        line: number,
        column: number,
        lineText: string,
        functionName?: string
    ): VtableResolutionResult {
        const enclosingFunc = functionName ?? findEnclosingFunction(this.index, file, line);

        // Strategy 1: JUNO_MODULE_GET_API(expr, TYPE)->field(
        const macroRe =
            /JUNO_MODULE_GET_API\s*\(\s*\w+\s*,\s*(\w+)\s*\)\s*->\s*(\w+)\s*\(/g;
        let m: RegExpExecArray | null;
        while ((m = macroRe.exec(lineText)) !== null) {
            if (column >= m.index && column < m.index + m[0].length) {
                return this.resolveByRootField(m[1], m[2]);
            }
        }

        // Strategy 2: word[expr](accessors)->field(
        const arrayRe =
            /(\w+)\s*\[.*?\]((?:\s*(?:->|\.)\s*\w+)*)\s*->\s*(\w+)\s*\(/g;
        while ((m = arrayRe.exec(lineText)) !== null) {
            if (column >= m.index && column < m.index + m[0].length) {
                return this.resolveChain(file, enclosingFunc, m[1], m[2], m[3]);
            }
        }

        // Strategy 3: word(accessors)->field(
        const generalRe =
            /(\w+)((?:\s*(?:->|\.)\s*\w+)*)\s*->\s*(\w+)\s*\(/g;
        while ((m = generalRe.exec(lineText)) !== null) {
            if (column >= m.index && column < m.index + m[0].length) {
                return this.resolveChain(file, enclosingFunc, m[1], m[2], m[3]);
            }
        }

        return {
            found: false,
            locations: [],
            errorMsg: 'No LibJuno API call pattern found at cursor position.',
        };
    }

    // -----------------------------------------------------------------------
    // Private: chain resolution
    // -----------------------------------------------------------------------

    /**
     * Resolve a call chain extracted by one of the regex strategies.
     *
     * Two API type candidates are collected before querying vtableAssignments:
     *   - From the primary identifier's declared type (via localTypeInfo and
     *     the derivation chain / moduleRoots / traitRoots maps).
     *   - From the last intermediate member name via apiMemberRegistry (covers
     *     named API pointers such as "ptHeapPointerApi").
     *
     * Both candidates are tried in order; the first successful vtable lookup
     * wins. A field-name search across all apiStructFields is the final
     * fallback.
     */
    private resolveChain(
        file: string,
        enclosingFunc: string | undefined,
        primary: string,
        chainStr: string,
        fieldName: string
    ): VtableResolutionResult {
        const intermediates = parseIntermediates(chainStr);
        const apiTypeCandidates: string[] = [];

        // Candidate 1: resolve from the primary identifier's declared type.
        if (enclosingFunc) {
            const typeInfo = lookupVariableType(this.index, file, enclosingFunc, primary);
            if (typeInfo) {
                const { typeName } = typeInfo;
                if (typeName.endsWith('_API_T')) {
                    // Primary IS an API pointer (e.g. ptLoggerApi->LogInfo).
                    apiTypeCandidates.push(typeName);
                } else {
                    // Primary is a root or derived type; walk to root and look up API.
                    const rootType = walkToRootType(this.index, typeName);
                    const apiType =
                        this.index.moduleRoots.get(rootType) ??
                        this.index.traitRoots.get(rootType);
                    if (apiType) {
                        apiTypeCandidates.push(apiType);
                    }
                }
            }
        }

        // Candidate 2: from apiMemberRegistry for the last intermediate member
        // (e.g., "ptHeapPointerApi" → "JUNO_DS_HEAP_POINTER_API_T").
        if (intermediates.length > 0) {
            const lastMember = intermediates[intermediates.length - 1];
            const apiType = this.index.apiMemberRegistry.get(lastMember);
            if (apiType && !apiTypeCandidates.includes(apiType)) {
                apiTypeCandidates.push(apiType);
            }
        }

        // Try each candidate; return on first match.
        for (const apiType of apiTypeCandidates) {
            const result = this.lookupVtable(apiType, fieldName);
            if (result.found) {
                return result;
            }
        }

        // Final fallback: field-name search across all known API types.
        const fallback = this.fieldNameFallback(fieldName);
        if (fallback.found) {
            return fallback;
        }

        // Return the most specific error: use the first resolved candidate's
        // error if available, otherwise the field-name fallback error.
        if (apiTypeCandidates.length > 0) {
            return this.lookupVtable(apiTypeCandidates[0], fieldName);
        }
        return fallback;
    }

    /**
     * Resolve a call made through JUNO_MODULE_GET_API(expr, ROOT_T)->field(.
     * The root type is explicit in the macro, so no localTypeInfo lookup is
     * needed.
     */
    private resolveByRootField(
        rootType: string,
        fieldName: string
    ): VtableResolutionResult {
        const walkedRoot = walkToRootType(this.index, rootType);
        const apiType =
            this.index.moduleRoots.get(walkedRoot) ??
            this.index.traitRoots.get(walkedRoot);
        if (!apiType) {
            return {
                found: false,
                locations: [],
                errorMsg: `No API type registered for root '${walkedRoot}'.`,
            };
        }
        return this.lookupVtable(apiType, fieldName);
    }

    /**
     * Look up concrete implementations for a known API type + field name pair
     * in the vtableAssignments index.
     */
    private lookupVtable(apiType: string, fieldName: string): VtableResolutionResult {
        const fieldMap = this.index.vtableAssignments.get(apiType);
        if (!fieldMap) {
            return {
                found: false,
                locations: [],
                errorMsg: `No vtable assignments found for '${apiType}'.`,
            };
        }
        const locations = fieldMap.get(fieldName);
        if (!locations || locations.length === 0) {
            return {
                found: false,
                locations: [],
                errorMsg: `No implementation found for '${apiType}::${fieldName}'.`,
            };
        }
        return { found: true, locations };
    }

    /**
     * Fallback: search every known API type in apiStructFields for fieldName.
     * Returns all concrete implementations found across all matching API types.
     * This handles cases where the receiver type could not be resolved via
     * localTypeInfo or apiMemberRegistry.
     */
    private fieldNameFallback(fieldName: string): VtableResolutionResult {
        const allLocations: ConcreteLocation[] = [];
        for (const [apiType, fields] of this.index.apiStructFields) {
            if (fields.includes(fieldName)) {
                const fieldMap = this.index.vtableAssignments.get(apiType);
                const locs = fieldMap?.get(fieldName) ?? [];
                allLocations.push(...locs);
            }
        }
        if (allLocations.length === 0) {
            return {
                found: false,
                locations: [],
                errorMsg: `No API type contains field '${fieldName}'.`,
            };
        }
        return { found: true, locations: allLocations };
    }
}
