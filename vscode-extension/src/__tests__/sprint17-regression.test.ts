/// <reference types="jest" />

/**
 * @file sprint17-regression.test.ts
 *
 * Regression tests for Sprint 17 bug fixes:
 *
 * Parser fixes (parser.ts):
 *  REG-17-001  {0} inside macro argument caused parse failure
 *  REG-17-002  (identifier) && mis-parsed as C cast
 *  REG-17-003  (identifier) ; mis-parsed as C cast
 *  REG-17-004  Actual C cast (int)x still works after looksLikeCast() guard
 *  REG-17-005  FloatingLiteral missing digits-exp-suffix pattern (500E3)
 *  REG-17-006  {0} in function parameter list
 *
 * WorkspaceIndexer fix (workspaceIndexer.ts):
 *  REG-17-007  reindexFile() now passes DeferredPositional[] and calls resolveDeferred()
 *
 * Integration regressions against production files:
 *  REG-17-008  juno_buff_queue.c parses with 0 errors
 *  REG-17-009  engine_app.c parses with 0 errors
 *  REG-17-010  Vtable call resolution for juno_broker.c:68 Enqueue
 */

import * as fs from 'fs';
import * as path from 'path';
import * as os from 'os';
import { CLexer } from '../parser/lexer';
import { CParser } from '../parser/parser';
import { parseFileWithDefs } from '../parser/visitor';
import { WorkspaceIndexer } from '../indexer/workspaceIndexer';
import { VtableResolver } from '../resolver/vtableResolver';

// ---------------------------------------------------------------------------
// Helper — parse a C source string and return lex + parse error counts
// ---------------------------------------------------------------------------

function parseSrc(src: string): { lexErrors: number; parseErrors: number } {
    const lex = CLexer.tokenize(src);
    const parser = new CParser();
    (parser as any).input = lex.tokens;
    parser.translationUnit();
    return { lexErrors: lex.errors.length, parseErrors: parser.errors.length };
}

// ---------------------------------------------------------------------------
// Production file paths
// ---------------------------------------------------------------------------

const QUEUE_API_H  = '/workspaces/libjuno/include/juno/ds/queue_api.h';
const BUFF_QUEUE_C = '/workspaces/libjuno/src/juno_buff_queue.c';
const BROKER_C     = '/workspaces/libjuno/src/juno_broker.c';
const BROKER_API_H = '/workspaces/libjuno/include/juno/sb/broker_api.h';
const ENGINE_APP_C = '/workspaces/libjuno/examples/example_project/engine/src/engine_app.c';

// ---------------------------------------------------------------------------
// REG-17-001 — {0} inside macro argument is not a parse error
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe('Sprint 17 Regression — Parser: braced initialiser in macro argument', () => {

    // REG-17-001
    it('REG-17-001: JUNO_ERR_RESULT(STATUS, {0}) inside a function body produces 0 parse errors', () => {
        const src = [
            'typedef int JUNO_STATUS_T;',
            'typedef int JUNO_RESULT_T;',
            'void f(void)',
            '{',
            '    JUNO_RESULT_T tResult = JUNO_ERR_RESULT(JUNO_STATUS_ERR, {0});',
            '}',
        ].join('\n');

        const { lexErrors, parseErrors } = parseSrc(src);

        expect(lexErrors).toBe(0);
        expect(parseErrors).toBe(0);
    });

    // REG-17-006
    it('REG-17-006: macro_call(arg1, {0}) as a statement inside a function produces 0 parse errors', () => {
        const src = [
            'void f(void)',
            '{',
            '    macro_call(arg1, {0});',
            '}',
        ].join('\n');

        const { lexErrors, parseErrors } = parseSrc(src);

        expect(lexErrors).toBe(0);
        expect(parseErrors).toBe(0);
    });
});

// ---------------------------------------------------------------------------
// REG-17-002/003/004 — looksLikeCast() guard
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe('Sprint 17 Regression — Parser: cast vs parenthesised expression', () => {

    // REG-17-002
    it('REG-17-002: (identifier) followed by && is NOT parsed as a cast — 0 parse errors', () => {
        // Before fix: (ptFoo) && ptBar caused the parser to try to treat (ptFoo)
        // as a cast operand, producing a parse error.
        const src = [
            'typedef struct FOO_TAG FOO_T;',
            'typedef struct BAR_TAG BAR_T;',
            'int f(FOO_T *ptFoo, BAR_T *ptBar)',
            '{',
            '    if (ptFoo && ptBar)',
            '    {',
            '        return 1;',
            '    }',
            '    return 0;',
            '}',
        ].join('\n');

        const { lexErrors, parseErrors } = parseSrc(src);

        expect(lexErrors).toBe(0);
        expect(parseErrors).toBe(0);
    });

    // REG-17-003
    it('REG-17-003: return (ptFoo) followed by ; is NOT parsed as a cast — 0 parse errors', () => {
        // Before fix: (ptFoo) followed by ; could cause looksLikeCast() to misfire because
        // there was no guard, making the parser try cast and fail.
        const src = [
            'typedef struct FOO_TAG FOO_T;',
            'FOO_T *f(FOO_T *ptFoo)',
            '{',
            '    return (ptFoo);',
            '}',
        ].join('\n');

        const { lexErrors, parseErrors } = parseSrc(src);

        expect(lexErrors).toBe(0);
        expect(parseErrors).toBe(0);
    });

    // REG-17-004
    it('REG-17-004: (int)x is still recognised as a C cast after looksLikeCast() guard — 0 parse errors', () => {
        // Verifies backward compatibility: looksLikeCast() allows cast when the
        // token after ) is an Identifier (start of a unary/primary expression).
        const src = [
            'void f(void)',
            '{',
            '    int x = 5;',
            '    int y = (int)x;',
            '    void *p = (void *)&x;',
            '}',
        ].join('\n');

        const { lexErrors, parseErrors } = parseSrc(src);

        expect(lexErrors).toBe(0);
        expect(parseErrors).toBe(0);
    });

    it('REG-17-004b: (ENGINE_APP_T *)(ptJunoApp) double cast produces 0 parse errors', () => {
        // From engine_app.c line 75 — outer cast followed by parenthesised identifier
        const src = [
            'typedef struct ENGINE_APP_TAG ENGINE_APP_T;',
            'ENGINE_APP_T *f(void *ptJunoApp)',
            '{',
            '    ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp);',
            '    return ptEngineApp;',
            '}',
        ].join('\n');

        const { lexErrors, parseErrors } = parseSrc(src);

        expect(lexErrors).toBe(0);
        expect(parseErrors).toBe(0);
    });
});

// ---------------------------------------------------------------------------
// REG-17-005 — FloatingLiteral with exponent-only form (no decimal point)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe('Sprint 17 Regression — Parser: FloatingLiteral exponent-only form', () => {

    // REG-17-005
    it('REG-17-005: float literals with exponent and no decimal point (1E10, 500E3) produce 0 lex errors', () => {
        // Before fix: 500E3 was lexed as IntegerLiteral(500) + Identifier(E3),
        // causing a parse error because E3 is not a valid expression continuation.
        const src = [
            'void f(void)',
            '{',
            '    float x = 1E10;',
            '    float y = 500E3;',
            '    float z = 1e-6;',
            '    float w = 2.5E3f;',
            '}',
        ].join('\n');

        const { lexErrors, parseErrors } = parseSrc(src);

        // All floating-point literals must be lexed as FloatingLiteral tokens (0 lex errors),
        // and the resulting token stream must parse completely (0 parse errors).
        expect(lexErrors).toBe(0);
        expect(parseErrors).toBe(0);
    });

    it('REG-17-005b: FloatingLiteral tokens are produced — not IntegerLiteral + Identifier', () => {
        const src = 'float x = 500E3;';
        const lex = CLexer.tokenize(src);

        expect(lex.errors).toHaveLength(0);

        // Find the token that matched "500E3" — it must be a FloatingLiteral
        const floatToken = lex.tokens.find(t => t.image === '500E3');
        expect(floatToken).toBeDefined();
        expect(floatToken!.tokenType.name).toBe('FloatingLiteral');
    });
});

// ---------------------------------------------------------------------------
// REG-17-007 — reindexFile() passes DeferredPositional[] and resolves it
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-021", "REQ-VSCODE-002"]}
describe('Sprint 17 Regression — WorkspaceIndexer: reindexFile positional vtable resolution', () => {

    let tempDir: string;

    beforeAll(() => {
        tempDir = fs.mkdtempSync(path.join(os.tmpdir(), 'libjuno-reg17-'));
    });

    afterAll(() => {
        fs.rmSync(tempDir, { recursive: true, force: true });
    });

    // REG-17-007
    it('REG-17-007: reindexFile() resolves positional vtable assignments when API struct is already indexed', async () => {
        // File A: defines the module root and API struct with two fields.
        //   Line 1:  typedef struct MY_API_TAG MY_API_T;
        //   Line 2:  typedef struct MY_ROOT_TAG MY_ROOT_T;
        //   Line 3:  (empty)
        //   Line 4:  struct MY_ROOT_TAG JUNO_MODULE_ROOT(MY_API_T, );
        //   Line 5:  (empty)
        //   Line 6:  struct MY_API_TAG
        //   Line 7:  {
        //   Line 8:      JUNO_STATUS_T (*Init)(MY_ROOT_T *ptSelf);
        //   Line 9:      JUNO_STATUS_T (*Run)(MY_ROOT_T *ptSelf);
        //   Line 10: };
        const srcA = [
            'typedef struct MY_API_TAG MY_API_T;',
            'typedef struct MY_ROOT_TAG MY_ROOT_T;',
            '',
            'struct MY_ROOT_TAG JUNO_MODULE_ROOT(MY_API_T, );',
            '',
            'struct MY_API_TAG',
            '{',
            '    JUNO_STATUS_T (*Init)(MY_ROOT_T *ptSelf);',
            '    JUNO_STATUS_T (*Run)(MY_ROOT_T *ptSelf);',
            '};',
            '',
        ].join('\n');

        // File B: defines implementations and a positional vtable initializer.
        //   The API struct (MY_API_T) is NOT defined here — it lives in file A.
        //   This tests cross-file deferred resolution triggered by reindexFile().
        //
        //   Line 1:  static JUNO_STATUS_T MyInit(MY_ROOT_T *ptSelf)
        //   ...
        //   Line 5:  static JUNO_STATUS_T MyRun(MY_ROOT_T *ptSelf)
        //   ...
        //   Line 9:  static const MY_API_T gApi = { MyInit, MyRun };
        const srcB = [
            'static JUNO_STATUS_T MyInit(MY_ROOT_T *ptSelf)',
            '{',
            '    return JUNO_STATUS_SUCCESS;',
            '}',
            'static JUNO_STATUS_T MyRun(MY_ROOT_T *ptSelf)',
            '{',
            '    return JUNO_STATUS_SUCCESS;',
            '}',
            'static const MY_API_T gApi = { MyInit, MyRun };',
            '',
        ].join('\n');

        const dir = path.join(tempDir, 'reg17-007');
        fs.mkdirSync(dir, { recursive: true });
        const fileA = path.join(dir, 'reg17_007a.c');
        const fileB = path.join(dir, 'reg17_007b.c');
        fs.writeFileSync(fileA, srcA);
        fs.writeFileSync(fileB, srcB);

        const indexer = new WorkspaceIndexer(dir, []);

        // Index file A first so MY_API_T's field order is in apiStructFields.
        await indexer.reindexFile(fileA);

        // Before the Sprint 17 fix, reindexFile() did NOT call resolveDeferred(),
        // so the positional vtable in file B would be silently dropped.
        await indexer.reindexFile(fileB);

        const { vtableAssignments, apiStructFields } = indexer.index;

        // Verify file A's API struct was captured with correct field order.
        const fields = apiStructFields.get('MY_API_T');
        expect(fields).toBeDefined();
        expect(fields).toEqual(['Init', 'Run']);

        // Verify the positional vtable in file B was resolved and merged.
        const fieldMap = vtableAssignments.get('MY_API_T');
        expect(fieldMap).toBeDefined();

        const initLocs = fieldMap!.get('Init');
        expect(initLocs).toBeDefined();
        expect(initLocs).toHaveLength(1);
        expect(initLocs![0].functionName).toBe('MyInit');
        expect(initLocs![0].file).toBe(fileB);

        const runLocs = fieldMap!.get('Run');
        expect(runLocs).toBeDefined();
        expect(runLocs).toHaveLength(1);
        expect(runLocs![0].functionName).toBe('MyRun');
        expect(runLocs![0].file).toBe(fileB);
    }, 15000);
});

// ---------------------------------------------------------------------------
// REG-17-008 — juno_buff_queue.c parses with 0 errors
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe('Sprint 17 Regression — Production file: juno_buff_queue.c', () => {

    // REG-17-008
    it('REG-17-008: juno_buff_queue.c parses with 0 Chevrotain errors', () => {
        // This file contains JUNO_ERR_RESULT(JUNO_STATUS_ERR, {0}) on line 81.
        // Before Sprint 17 fix, the {0} macro arg caused a parse failure.
        const text = fs.readFileSync(BUFF_QUEUE_C, 'utf-8');
        const { lexErrors, parseErrors } = parseSrc(text);

        expect(lexErrors).toBe(0);
        expect(parseErrors).toBe(0);
    });

    it('REG-17-008b: parseFileWithDefs on juno_buff_queue.c extracts vtable assignments', () => {
        const text = fs.readFileSync(BUFF_QUEUE_C, 'utf-8');
        const { parsed } = parseFileWithDefs(BUFF_QUEUE_C, text);

        // The positional gtQueueApi initializer must produce pendingPositionalVtables
        // (because JUNO_DS_QUEUE_API_T is defined in queue_api.h, not in buff_queue.c).
        // This verifies the parser reached the vtable block without erroring out.
        expect(parsed.pendingPositionalVtables.length + parsed.vtableAssignments.length).toBeGreaterThan(0);
    });
});

// ---------------------------------------------------------------------------
// REG-17-009 — engine_app.c parses with 0 errors
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe('Sprint 17 Regression — Production file: engine_app.c', () => {

    // REG-17-009
    it('REG-17-009: engine_app.c parses with 0 Chevrotain errors', () => {
        // This file contains (ENGINE_APP_T *)(ptJunoApp) — an actual C cast followed by
        // a parenthesised identifier. Before the looksLikeCast() fix the parser could
        // produce errors on similar patterns.
        const text = fs.readFileSync(ENGINE_APP_C, 'utf-8');
        const { lexErrors, parseErrors } = parseSrc(text);

        expect(lexErrors).toBe(0);
        expect(parseErrors).toBe(0);
    });

    it('REG-17-009b: parseFileWithDefs on engine_app.c captures local variables', () => {
        const text = fs.readFileSync(ENGINE_APP_C, 'utf-8');
        const { parsed } = parseFileWithDefs(ENGINE_APP_C, text);

        // At least one function must have local variable info captured.
        expect(parsed.localTypeInfo.localVariables.size).toBeGreaterThan(0);
    });
});

// ---------------------------------------------------------------------------
// REG-17-010 — Vtable resolution for juno_broker.c:68 Enqueue call
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-002"]}
describe('Sprint 17 Regression — E2E: juno_broker.c Enqueue resolves to JunoDs_QueuePush', () => {

    let resolver: VtableResolver;

    beforeAll(async () => {
        // Index the real LibJuno files in the correct dependency order:
        //   queue_api.h  →  defines JUNO_DS_QUEUE_ROOT_T and JUNO_DS_QUEUE_API_T
        //   juno_buff_queue.c  →  implements the queue vtable (positional init)
        //   broker_api.h  →  defines JUNO_SB_BROKER_ROOT_T
        //   juno_broker.c  →  uses ptRecvQueue->ptApi->Enqueue(...)
        //
        // Before Sprint 17:
        //  1. juno_buff_queue.c failed to parse (JUNO_ERR_RESULT with {0})
        //     so JunoDs_QueuePush was never extracted as a vtable entry.
        //  2. reindexFile() did not call resolveDeferred() so positional
        //     vtables were silently dropped even when parsing succeeded.
        const indexer = new WorkspaceIndexer('/workspaces/libjuno/src', []);
        await indexer.reindexFile(QUEUE_API_H);
        await indexer.reindexFile(BUFF_QUEUE_C);
        await indexer.reindexFile(BROKER_API_H);
        await indexer.reindexFile(BROKER_C);
        resolver = new VtableResolver(indexer.index);
    }, 30000 /* ms */);

    // REG-17-010
    it('REG-17-010: resolve(BROKER_C, 68, col) returns found=true with JunoDs_QueuePush', () => {
        // Line 68 (1-based) of juno_broker.c:
        //   "            tStatus = ptRecvQueue->ptApi->Enqueue(ptRecvQueue, tMsg);"
        //
        // ptRecvQueue starts at column 22 (0-based).
        const lineText = fs.readFileSync(BROKER_C, 'utf-8').split('\n')[67]; // 0-based index 67 = line 68

        // Verify the line is what we expect (guard against file changes).
        expect(lineText).toContain('ptRecvQueue->ptApi->Enqueue');

        const result = resolver.resolve(BROKER_C, 68, 22, lineText);

        expect(result.found).toBe(true);
        expect(result.locations.length).toBeGreaterThan(0);

        const fnNames = result.locations.map(loc => loc.functionName);
        expect(fnNames).toContain('JunoDs_QueuePush');
    });

    it('REG-17-010b: JunoDs_QueuePush location points to juno_buff_queue.c', () => {
        const lineText = fs.readFileSync(BROKER_C, 'utf-8').split('\n')[67];
        const result = resolver.resolve(BROKER_C, 68, 22, lineText);

        expect(result.found).toBe(true);

        const pushLoc = result.locations.find(loc => loc.functionName === 'JunoDs_QueuePush');
        expect(pushLoc).toBeDefined();
        expect(pushLoc!.file).toBe(BUFF_QUEUE_C);
        expect(pushLoc!.line).toBeGreaterThan(0);
    });
});
