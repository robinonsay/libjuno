/// <reference types="jest" />

/**
 * @file c11-grammar-conformance.test.ts
 *
 * Permanent regression guard for C11 grammar conformance in `parser.ts`.
 *
 * This suite catalogues canonical C11 expression (§6.5) and statement (§6.8)
 * patterns that historically have either broken or come close to breaking
 * the Chevrotain grammar. Each test wraps the pattern inside a minimal
 * `static int Fn_NNN_*(void) { ... return 0; }` function body and asserts
 * that `parseFileWithDefs()` produces exactly one `FunctionDefinitionRecord`
 * for the wrapping function — i.e. the parser consumed the full body and
 * emitted the definition.
 *
 * Strategy: if the grammar bails mid-body, the visitor will not emit the
 * definition record (or will emit a malformed one), and the
 * `toHaveLength(1)` assertion will fail.
 *
 * Before the Phase 2 fix the following cases are expected to be RED:
 *   - TC-CONF-006 (indirection cast assignment)
 *   - TC-CONF-012 (compound-literal assignment — the original bug shape)
 * Other cases may already be GREEN; they stand as regression guards.
 */

import { parseFileWithDefs } from '../visitor';

// ---------------------------------------------------------------------------
// Helper: parse the snippet and assert the named function was indexed.
// ---------------------------------------------------------------------------

/**
 * Parse `src` and assert that the parser emitted exactly one
 * FunctionDefinitionRecord for `fnName`. This encodes the invariant that
 * "the parser did not bail on the function body; it emitted the record."
 */
function assertFunctionIndexed(src: string, fnName: string): void {
    const result = parseFileWithDefs('/test/conf.c', src);
    const defs = result.functionDefs.filter((d) => d.functionName === fnName);
    expect(defs).toHaveLength(1);
}

// @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
describe('C11 Grammar Conformance — parser.ts regression guard', () => {

    // =======================================================================
    // Group A — Unary Expressions (C11 §6.5.3)
    //
    // Root-cause area for the Go-to-Definition bug. The fixed grammar must
    // keep `++` / `--` routed to unaryExpression and `& * + - ~ !` routed
    // to castExpression.
    // =======================================================================

    describe('Group A — Unary expressions (§6.5.3)', () => {

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-001: prefix ++ on identifier', () => {
            const src = `static int Fn_001_PrefixIncrement(void) { int iX = 0; ++iX; return iX; }`;
            assertFunctionIndexed(src, 'Fn_001_PrefixIncrement');
        });

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-002: prefix -- on identifier', () => {
            const src = `static int Fn_002_PrefixDecrement(void) { int iX = 0; --iX; return iX; }`;
            assertFunctionIndexed(src, 'Fn_002_PrefixDecrement');
        });

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-003: postfix ++ on identifier', () => {
            const src = `static int Fn_003_PostfixIncrement(void) { int iX = 0; iX++; return iX; }`;
            assertFunctionIndexed(src, 'Fn_003_PostfixIncrement');
        });

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-004: postfix -- on identifier', () => {
            const src = `static int Fn_004_PostfixDecrement(void) { int iX = 0; iX--; return iX; }`;
            assertFunctionIndexed(src, 'Fn_004_PostfixDecrement');
        });

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-005: address-of applied to indirection of cast', () => {
            const src = `static int Fn_005_AddressOfCast(void) { void *pv = 0; int *p = &*(int *) pv; (void)p; return 0; }`;
            assertFunctionIndexed(src, 'Fn_005_AddressOfCast');
        });

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-006: indirection of cast as assignment target (one of the Phase 0 bug shapes)', () => {
            const src = `static int Fn_006_DerefCastAssign(void) { void *pv = 0; *(int *) pv = 0; return 0; }`;
            assertFunctionIndexed(src, 'Fn_006_DerefCastAssign');
        });

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-007: unary minus on cast expression', () => {
            const src = `static int Fn_007_UnaryMinusCast(void) { int iX = 1; int iY = -(int) iX; return iY; }`;
            assertFunctionIndexed(src, 'Fn_007_UnaryMinusCast');
        });

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-008: bitwise not on cast expression', () => {
            const src = `static int Fn_008_BitwiseNotCast(void) { int iX = 1; int iY = (int) ~(unsigned) iX; return iY; }`;
            assertFunctionIndexed(src, 'Fn_008_BitwiseNotCast');
        });

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-009: logical not on cast expression', () => {
            const src = `static int Fn_009_LogicalNotCast(void) { int iX = 1; if (!(int) iX) { return 0; } return 1; }`;
            assertFunctionIndexed(src, 'Fn_009_LogicalNotCast');
        });
    });

    // =======================================================================
    // Group B — Cast and Compound Literals (C11 §6.5.4, §6.5.2.5)
    // =======================================================================

    describe('Group B — Cast and compound literals (§6.5.4, §6.5.2.5)', () => {

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-010: simple cast in initializer', () => {
            const src = `static int Fn_010_SimpleCast(void) { int iX = 1; int iY = (int) iX; return iY; }`;
            assertFunctionIndexed(src, 'Fn_010_SimpleCast');
        });

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-011: nested cast', () => {
            const src = `static int Fn_011_NestedCast(void) { int iX = 1; int iY = (int) (unsigned) iX; return iY; }`;
            assertFunctionIndexed(src, 'Fn_011_NestedCast');
        });

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-012: compound-literal assignment through cast indirection (original bug shape)', () => {
            const src = [
                'typedef struct { int iA; } STRUCT_T;',
                'static int Fn_012_CompoundLiteralAssign(void) {',
                '    void *pv = 0;',
                '    *(STRUCT_T *) pv = (STRUCT_T){0};',
                '    return 0;',
                '}',
            ].join('\n');
            assertFunctionIndexed(src, 'Fn_012_CompoundLiteralAssign');
        });

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-013: designated initializer in declaration', () => {
            const src = [
                'typedef struct { int iA; } STRUCT_T;',
                'static int Fn_013_DesignatedInitializer(void) {',
                '    STRUCT_T t = { .iA = 1 };',
                '    return t.iA;',
                '}',
            ].join('\n');
            assertFunctionIndexed(src, 'Fn_013_DesignatedInitializer');
        });

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-014: compound literal as function argument', () => {
            const src = [
                'typedef struct { int iA; } STRUCT_T;',
                'static void Foo(STRUCT_T t) { (void)t; }',
                'static int Fn_014_CompoundLiteralArg(void) {',
                '    Foo((STRUCT_T){ .iA = 1 });',
                '    return 0;',
                '}',
            ].join('\n');
            assertFunctionIndexed(src, 'Fn_014_CompoundLiteralArg');
        });
    });

    // =======================================================================
    // Group C — Postfix Expressions (C11 §6.5.2)
    // =======================================================================

    describe('Group C — Postfix expressions (§6.5.2)', () => {

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-015: array subscript expression', () => {
            const src = [
                'static int aiX[4];',
                'static int Fn_015_ArraySubscript(void) {',
                '    int iY = aiX[0];',
                '    return iY;',
                '}',
            ].join('\n');
            assertFunctionIndexed(src, 'Fn_015_ArraySubscript');
        });

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-016: member access through pointer', () => {
            const src = [
                'typedef struct { int iA; } STRUCT_T;',
                'static int Fn_016_MemberArrow(STRUCT_T *ptS) {',
                '    int iY = ptS->iA;',
                '    return iY;',
                '}',
            ].join('\n');
            assertFunctionIndexed(src, 'Fn_016_MemberArrow');
        });

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-017: function call embedded in arithmetic expression', () => {
            const src = [
                'static int Foo(int i) { return i; }',
                'static int Fn_017_CallInExpression(void) {',
                '    int iX = 1;',
                '    int iY = Foo(iX) + 1;',
                '    return iY;',
                '}',
            ].join('\n');
            assertFunctionIndexed(src, 'Fn_017_CallInExpression');
        });
    });

    // =======================================================================
    // Group D — Statements (C11 §6.8)
    // =======================================================================

    describe('Group D — Statements (§6.8)', () => {

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-018: compound statement with nested blocks', () => {
            const src = [
                'static int Fn_018_NestedBlocks(void) {',
                '    int iX = 1;',
                '    {',
                '        {',
                '            int iY = iX;',
                '            (void)iY;',
                '        }',
                '    }',
                '    return 0;',
                '}',
            ].join('\n');
            assertFunctionIndexed(src, 'Fn_018_NestedBlocks');
        });

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-019: do-while loop with compound body', () => {
            const src = [
                'static int Fn_019_DoWhile(void) {',
                '    int iX = 0;',
                '    do {',
                '        ++iX;',
                '    } while (iX < 3);',
                '    return iX;',
                '}',
            ].join('\n');
            assertFunctionIndexed(src, 'Fn_019_DoWhile');
        });

        // @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}
        it('TC-CONF-020: for loop with declaration in init clause', () => {
            const src = [
                'static int Fn_020_ForDeclInit(void) {',
                '    for (int i = 0; i < 3; ++i) {',
                '        (void)i;',
                '    }',
                '    return 0;',
                '}',
            ].join('\n');
            assertFunctionIndexed(src, 'Fn_020_ForDeclInit');
        });
    });
});
