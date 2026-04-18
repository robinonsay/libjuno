// @{"req": ["REQ-VSCODE-016", "REQ-VSCODE-022", "REQ-VSCODE-023", "REQ-VSCODE-024", "REQ-VSCODE-025", "REQ-VSCODE-026"]}
/**
 * @file failureHandlerResolver.ts
 *
 * Implements the Failure Handler Resolution algorithm (design §5.3) and the
 * FAIL Macro Call Site Resolution extension (design §5.3.1).
 *
 * Given a cursor position on a line containing `_pfcnFailureHandler` or
 * `JUNO_FAILURE_HANDLER`, FailureHandlerResolver resolves the concrete
 * handler function(s) associated with the module root type that owns the
 * member being accessed.
 *
 * Resolution steps:
 *   0. (§5.3.1) Check if the line matches a FAIL macro call site pattern. If
 *      matched, extract the 2nd argument and resolve the handler or module
 *      pointer using the appropriate lookup strategy for the macro type.
 *   1. Confirm the line contains a failure handler reference; return early if
 *      not, so callers can safely invoke this resolver on any cursor position.
 *   2. If the line is a handler assignment (`= funcName`), navigate directly
 *      to the RHS function's definition — the most precise answer for the
 *      specific assignment being written.
 *   3. Walk the LHS primary variable's derived type up to the root type via
 *      the derivation chain, then query failureHandlerAssignments for all
 *      handlers registered to that root type.
 */

import { NavigationIndex, VtableResolutionResult } from '../parser/types';
import {
    findEnclosingFunction,
    lookupVariableType,
    walkToRootType,
} from './resolverUtils';

/** Matches either macro name or underlying member name. */
const FAILURE_HANDLER_PRESENCE_RE = /_pfcnFailureHandler|JUNO_FAILURE_HANDLER/;

/**
 * Matches one of the four FAIL/ASSERT macro call sites. Capture group 1 is
 * the macro name.
 */
const FAIL_MACRO_RE =
    /\b(JUNO_FAIL|JUNO_FAIL_MODULE|JUNO_FAIL_ROOT|JUNO_ASSERT_EXISTS_MODULE)\s*\(/;

/**
 * Captures the LHS expression (group 1) and the RHS function name (group 2)
 * of a failure handler assignment.
 *
 * Examples matched:
 *   ptEngineApp->tRoot.JUNO_FAILURE_HANDLER = MyHandler
 *   ptModule->_pfcnFailureHandler = handler
 *   ptModule.JUNO_FAILURE_HANDLER = CbHandler
 */
const ASSIGNMENT_RE =
    /(\w+(?:\s*(?:->|\.)\s*\w+)*)\s*(?:->|\.)\s*(?:JUNO_FAILURE_HANDLER|_pfcnFailureHandler)\s*=\s*(\w+)/;

/**
 * Captures the first identifier in an access chain that precedes `->` or `.`,
 * used to extract the primary variable name from a non-assignment reference.
 */
const PRIMARY_VAR_RE = /(\w+)\s*(?:->|\.)/;

/**
 * Resolves LibJuno failure handler references to their concrete handler
 * function definition locations.
 */
export class FailureHandlerResolver {
    constructor(private readonly index: NavigationIndex) {}

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
    resolve(
        file: string,
        line: number,
        column: number,
        lineText: string,
        functionName?: string
    ): VtableResolutionResult {
        // Step 0 (§5.3.1): Check for FAIL macro call site. If matched, resolve
        // using the FAIL macro algorithm and return — do not fall through to §5.3.
        const failMacroMatch = FAIL_MACRO_RE.exec(lineText);
        if (failMacroMatch) {
            const macroName = failMacroMatch[1];
            const enclosingFuncForMacro =
                functionName ?? findEnclosingFunction(this.index, file, line);
            return this.resolveFailMacro(macroName, lineText, file, enclosingFuncForMacro);
        }

        if (!FAILURE_HANDLER_PRESENCE_RE.test(lineText)) {
            return {
                found: false,
                locations: [],
                errorMsg: 'Line does not contain a failure handler reference.',
            };
        }

        const enclosingFunc = functionName ?? findEnclosingFunction(this.index, file, line);

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
            const typeInfo = lookupVariableType(
                this.index,
                file,
                enclosingFunc,
                primaryIdent
            );
            if (typeInfo) {
                const rootType = walkToRootType(this.index, typeInfo.typeName);
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
    private extractPrimaryIdent(
        assignMatch: RegExpExecArray | null,
        lineText: string
    ): string | undefined {
        if (assignMatch) {
            const lhsFirstWord = /^(\w+)/.exec(assignMatch[1]);
            if (lhsFirstWord) {
                return lhsFirstWord[1];
            }
        }
        return PRIMARY_VAR_RE.exec(lineText)?.[1];
    }

    // -----------------------------------------------------------------------
    // §5.3.1 — FAIL macro resolution helpers
    // -----------------------------------------------------------------------

    /**
     * Resolves a FAIL/ASSERT macro call site to its concrete handler or module
     * handler location(s).
     *
     * Implements design §5.3.1 Steps 1–2.
     *
     * @param macroName     One of the four recognised macro names.
     * @param lineText      Full text of the source line.
     * @param file          Absolute path of the C source file.
     * @param enclosingFunc Name of the enclosing function (may be undefined).
     * @returns VtableResolutionResult.
     */
    // @{"req": ["REQ-VSCODE-022", "REQ-VSCODE-023", "REQ-VSCODE-024", "REQ-VSCODE-025", "REQ-VSCODE-026"]}
    private resolveFailMacro(
        macroName: string,
        lineText: string,
        file: string,
        enclosingFunc: string | undefined
    ): VtableResolutionResult {
        const extractedArg = this.extractMacroArg(lineText, 1);
        if (extractedArg === undefined) {
            return {
                found: false,
                locations: [],
                errorMsg: `Could not extract 2nd argument from ${macroName} call.`,
            };
        }

        if (macroName === 'JUNO_FAIL') {
            // arg[1] is a function pointer variable or function name — look up directly.
            const defs = this.index.functionDefinitions.get(extractedArg);
            if (defs && defs.length > 0) {
                return {
                    found: true,
                    locations: defs.map(d => ({
                        functionName: extractedArg,
                        file: d.file,
                        line: d.line,
                    })),
                };
            }
            return {
                found: false,
                locations: [],
                errorMsg: `No definition found for failure handler '${extractedArg}'.`,
            };
        }

        if (macroName === 'JUNO_FAIL_MODULE' || macroName === 'JUNO_ASSERT_EXISTS_MODULE') {
            // arg[1] is a pointer to a derived or root module struct — walk derivation chain.
            if (!enclosingFunc) {
                return {
                    found: false,
                    locations: [],
                    errorMsg: `Could not resolve enclosing function for ${macroName} call.`,
                };
            }
            const typeInfo = lookupVariableType(this.index, file, enclosingFunc, extractedArg);
            if (!typeInfo) {
                return {
                    found: false,
                    locations: [],
                    errorMsg: `Could not resolve type of '${extractedArg}' in ${macroName} call.`,
                };
            }
            const rootType = walkToRootType(this.index, typeInfo.typeName);
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

        if (macroName === 'JUNO_FAIL_ROOT') {
            // arg[1] is already a root module pointer — no derivation chain walk.
            if (!enclosingFunc) {
                return {
                    found: false,
                    locations: [],
                    errorMsg: `Could not resolve enclosing function for ${macroName} call.`,
                };
            }
            const typeInfo = lookupVariableType(this.index, file, enclosingFunc, extractedArg);
            if (!typeInfo) {
                return {
                    found: false,
                    locations: [],
                    errorMsg: `Could not resolve type of '${extractedArg}' in ${macroName} call.`,
                };
            }
            const rootType = typeInfo.typeName;
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

        return {
            found: false,
            locations: [],
            errorMsg: `Unrecognised FAIL macro: '${macroName}'.`,
        };
    }

    /**
     * Extracts the argument at the given 0-based index from a macro call on
     * the line. Uses balanced-parenthesis tracking so nested calls like
     * `JUNO_FAIL(status, getHandler(ptMod), data, "msg")` are handled
     * correctly. Cast expressions such as `(TYPE *)ptr` are stripped from the
     * extracted argument before returning.
     *
     * Used by §5.3.1 to extract the second argument from a FAIL macro call.
     *
     * @param lineText  Full text of the source line.
     * @param argIndex  0-based index of the argument to extract.
     * @returns The bare identifier string, or undefined if not found.
     */
    private extractMacroArg(lineText: string, argIndex: number): string | undefined {
        // Locate the opening parenthesis of the macro call matched by FAIL_MACRO_RE.
        const macroMatch = FAIL_MACRO_RE.exec(lineText);
        if (!macroMatch) {
            return undefined;
        }

        // Start scanning from the character after the '(' that ends the regex match.
        // macroMatch.index + macroMatch[0].length positions us just after '('.
        const startPos = macroMatch.index + macroMatch[0].length;

        let depth = 1;
        let argStart = startPos;
        const args: string[] = [];

        for (let i = startPos; i < lineText.length; i++) {
            const ch = lineText[i];
            if (ch === '(') {
                depth++;
            } else if (ch === ')') {
                depth--;
                if (depth === 0) {
                    // End of argument list — collect the final argument.
                    args.push(lineText.slice(argStart, i));
                    break;
                }
            } else if (ch === ',' && depth === 1) {
                args.push(lineText.slice(argStart, i));
                argStart = i + 1;
            }
        }

        if (argIndex >= args.length) {
            return undefined;
        }

        // Strip surrounding whitespace then cast expressions like `(TYPE *)ptr`.
        let arg = args[argIndex].trim();
        arg = arg.replace(/^\(\s*[\w\s*]+\)\s*/, '');
        arg = arg.trim();

        return arg.length > 0 ? arg : undefined;
    }
}
