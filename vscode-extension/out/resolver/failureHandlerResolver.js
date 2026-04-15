"use strict";
/**
 * @file failureHandlerResolver.ts
 *
 * Implements the Failure Handler Resolution algorithm (design §5.3).
 *
 * Given a cursor position on a line containing `_pfcnFailureHandler` or
 * `JUNO_FAILURE_HANDLER`, FailureHandlerResolver resolves the concrete
 * handler function(s) associated with the module root type that owns the
 * member being accessed.
 *
 * Resolution steps:
 *   1. Confirm the line contains a failure handler reference; return early if
 *      not, so callers can safely invoke this resolver on any cursor position.
 *   2. If the line is a handler assignment (`= funcName`), navigate directly
 *      to the RHS function's definition — the most precise answer for the
 *      specific assignment being written.
 *   3. Walk the LHS primary variable's derived type up to the root type via
 *      the derivation chain, then query failureHandlerAssignments for all
 *      handlers registered to that root type.
 */
Object.defineProperty(exports, "__esModule", { value: true });
exports.FailureHandlerResolver = void 0;
const resolverUtils_1 = require("./resolverUtils");
/** Matches either macro name or underlying member name. */
const FAILURE_HANDLER_PRESENCE_RE = /_pfcnFailureHandler|JUNO_FAILURE_HANDLER/;
/**
 * Captures the LHS expression (group 1) and the RHS function name (group 2)
 * of a failure handler assignment.
 *
 * Examples matched:
 *   ptEngineApp->tRoot.JUNO_FAILURE_HANDLER = MyHandler
 *   ptModule->_pfcnFailureHandler = handler
 *   ptModule.JUNO_FAILURE_HANDLER = CbHandler
 */
const ASSIGNMENT_RE = /(\w+(?:\s*(?:->|\.)\s*\w+)*)\s*(?:->|\.)\s*(?:JUNO_FAILURE_HANDLER|_pfcnFailureHandler)\s*=\s*(\w+)/;
/**
 * Captures the first identifier in an access chain that precedes `->` or `.`,
 * used to extract the primary variable name from a non-assignment reference.
 */
const PRIMARY_VAR_RE = /(\w+)\s*(?:->|\.)/;
/**
 * Resolves LibJuno failure handler references to their concrete handler
 * function definition locations.
 */
class FailureHandlerResolver {
    constructor(index) {
        this.index = index;
    }
    /**
     * Resolve the failure handler reference at the given cursor position.
     *
     * @param file          Absolute path of the C source file.
     * @param line          1-based line number of the cursor.
     * @param column        0-based column number of the cursor (currently used
     *                      only to constrain future per-token matching; the
     *                      single-token-per-line pattern makes line-level
     *                      matching sufficient).
     * @param lineText      Full text of the source line at the cursor position.
     * @param functionName  Optional: name of the enclosing C function. When
     *                      omitted, inferred from the NavigationIndex.
     * @returns VtableResolutionResult with found locations or an error message.
     */
    resolve(file, line, column, lineText, functionName) {
        if (!FAILURE_HANDLER_PRESENCE_RE.test(lineText)) {
            return {
                found: false,
                locations: [],
                errorMsg: 'Line does not contain a failure handler reference.',
            };
        }
        const enclosingFunc = functionName ?? (0, resolverUtils_1.findEnclosingFunction)(this.index, file, line);
        // Step 1: Assignment form — navigate to the explicitly named RHS function.
        const assignMatch = ASSIGNMENT_RE.exec(lineText);
        if (assignMatch) {
            const rhsFuncName = assignMatch[2];
            const defs = this.index.functionDefinitions.get(rhsFuncName);
            if (defs && defs.length > 0) {
                return {
                    found: true,
                    locations: defs.map(d => ({
                        functionName: rhsFuncName,
                        file: d.file,
                        line: d.line,
                    })),
                };
            }
        }
        // Step 2: Walk the LHS primary variable's type to the root, then look
        // up all registered handlers for that root type.
        const primaryIdent = this.extractPrimaryIdent(assignMatch, lineText);
        if (primaryIdent && enclosingFunc) {
            const typeInfo = (0, resolverUtils_1.lookupVariableType)(this.index, file, enclosingFunc, primaryIdent);
            if (typeInfo) {
                const rootType = (0, resolverUtils_1.walkToRootType)(this.index, typeInfo.typeName);
                const locations = this.index.failureHandlerAssignments.get(rootType);
                if (locations && locations.length > 0) {
                    return { found: true, locations };
                }
                return {
                    found: false,
                    locations: [],
                    errorMsg: `No failure handler registered for '${rootType}'.`,
                };
            }
        }
        return {
            found: false,
            locations: [],
            errorMsg: 'Could not resolve failure handler: enclosing function or variable type unknown.',
        };
    }
    // -----------------------------------------------------------------------
    // Private helpers
    // -----------------------------------------------------------------------
    /**
     * Extracts the primary (first) identifier from the LHS of a potential
     * assignment or from the line text directly.
     *
     * When an assignment match is available, the first word of the LHS
     * expression (group 1) is used — this is the outermost variable in the
     * chain (e.g., "ptEngineApp" from "ptEngineApp->tRoot.JUNO_FAILURE_HANDLER").
     * Otherwise, the first identifier followed by `->` or `.` on the line is
     * used.
     */
    extractPrimaryIdent(assignMatch, lineText) {
        if (assignMatch) {
            const lhsFirstWord = /^(\w+)/.exec(assignMatch[1]);
            if (lhsFirstWord) {
                return lhsFirstWord[1];
            }
        }
        return PRIMARY_VAR_RE.exec(lineText)?.[1];
    }
}
exports.FailureHandlerResolver = FailureHandlerResolver;
//# sourceMappingURL=failureHandlerResolver.js.map