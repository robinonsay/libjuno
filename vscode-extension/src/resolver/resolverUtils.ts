// @{"req": ["REQ-VSCODE-002", "REQ-VSCODE-009", "REQ-VSCODE-015", "REQ-VSCODE-016"]}
/**
 * @file resolverUtils.ts
 *
 * Shared utility functions used by VtableResolver and FailureHandlerResolver.
 * Provides index-query helpers for enclosing function discovery, variable type
 * lookup, derivation chain walking, and intermediate-member chain parsing.
 */

import { NavigationIndex, TypeInfo } from '../parser/types';

/**
 * Returns the name of the function whose definition line is the highest value
 * that is still <= the given query line number within the given file.
 *
 * This is used to determine the enclosing function scope for localTypeInfo
 * lookup when the caller does not supply an explicit function name.
 *
 * @param index      The populated NavigationIndex.
 * @param file       Absolute path of the file being queried.
 * @param line       1-based line number of the cursor position.
 * @returns The enclosing function name, or undefined if none found.
 */
export function findEnclosingFunction(
    index: NavigationIndex,
    file: string,
    line: number
): string | undefined {
    let bestLine = -1;
    let bestName: string | undefined;
    for (const [name, defs] of index.functionDefinitions) {
        for (const def of defs) {
            if (def.file === file && def.line <= line && def.line > bestLine) {
                bestLine = def.line;
                bestName = name;
            }
        }
    }
    return bestName;
}

/**
 * Looks up the TypeInfo for a named variable or parameter in a given function
 * scope. LocalVariables are searched first; function parameters are the
 * fallback.
 *
 * @param index     The populated NavigationIndex.
 * @param file      Absolute path of the file being queried.
 * @param funcName  Name of the enclosing function.
 * @param varName   Variable or parameter name to look up.
 * @returns The TypeInfo entry, or undefined if not found.
 */
export function lookupVariableType(
    index: NavigationIndex,
    file: string,
    funcName: string,
    varName: string
): TypeInfo | undefined {
    const fileInfo = index.localTypeInfo.get(file);
    if (!fileInfo) {
        return undefined;
    }

    const locals = fileInfo.localVariables.get(funcName);
    if (locals?.has(varName)) {
        return locals.get(varName);
    }

    const params = fileInfo.functionParameters.get(funcName);
    return params?.find(p => p.name === varName);
}

/**
 * Walks the NavigationIndex derivation chain from the given type name upward
 * to its topmost root type. Cycle detection is provided via a visited set.
 *
 * @param index     The populated NavigationIndex.
 * @param typeName  Starting type name, e.g. "JUNO_DS_HEAP_IMPL_T".
 * @returns The topmost root type name reachable from typeName.
 */
export function walkToRootType(index: NavigationIndex, typeName: string): string {
    let current = typeName;
    const visited = new Set<string>();
    while (index.derivationChain.has(current) && !visited.has(current)) {
        visited.add(current);
        current = index.derivationChain.get(current)!;
    }
    return current;
}

/**
 * Parses an intermediate member access chain string (e.g. "->ptApi",
 * "->ptBroker->ptApi", or ".tOk->ptApi") into an ordered list of member
 * names.
 *
 * @param chainStr  The raw chain segment captured by the resolver regex.
 * @returns Ordered list of member name strings; empty array for empty input.
 */
export function parseIntermediates(chainStr: string): string[] {
    if (!chainStr.trim()) {
        return [];
    }
    const re = /(?:->|\.)\s*(\w+)/g;
    const members: string[] = [];
    let m: RegExpExecArray | null;
    while ((m = re.exec(chainStr)) !== null) {
        members.push(m[1]);
    }
    return members;
}
