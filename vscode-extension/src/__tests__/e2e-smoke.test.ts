/// <reference types="jest" />

// @{"verify": ["REQ-VSCODE-001", "REQ-VSCODE-002", "REQ-VSCODE-016"]}
/**
 * @file e2e-smoke.test.ts
 *
 * End-to-end smoke tests exercising the full LibJuno navigation pipeline
 * against *real* LibJuno C source files (not synthetic fixtures).
 *
 * Empirical findings (verified by probe run):
 *  - Static const struct initializers in real .c files ARE indexed into
 *    vtableAssignments — the parser handles them correctly.
 *  - JUNO_MODULE_ROOT declarations in real .h headers ARE indexed into
 *    moduleRoots now that the parser handles JUNO_MODULE_RESULT and
 *    JUNO_MODULE_OPTION macros with keyword type arguments (size_t, bool,
 *    double, void *, etc.).
 *  - failureHandlerAssignments for JUNO_DS_HEAP_ROOT_T IS populated from
 *    juno_heap.c because moduleRoots now contains the root type.
 *  - VtableResolver.resolve() returns found:false for real files
 *    when called on non-call-site lines (assignments, typedefs, etc.).
 *    This is *correct* behaviour — the resolver does not crash; it returns an
 *    informative error message.
 *
 * TC-E2E-SMOKE-001  Indexer populates vtableAssignments from real engine_app.c
 * TC-E2E-SMOKE-002  FailureHandlerResolver handles real juno_heap.c gracefully
 * TC-E2E-SMOKE-003  Header-only file → VtableResolver returns found:false cleanly
 */

import * as fs from 'fs';
import * as path from 'path';
import * as os from 'os';
import { WorkspaceIndexer } from '../indexer/workspaceIndexer';
import { VtableResolver } from '../resolver/vtableResolver';
import { FailureHandlerResolver } from '../resolver/failureHandlerResolver';

// ---------------------------------------------------------------------------
// Absolute paths to real LibJuno source files used as smoke-test inputs.
// ---------------------------------------------------------------------------

const ENGINE_APP_C =
    '/workspaces/libjuno/examples/example_project/engine/src/engine_app.c';
const APP_API_H =
    '/workspaces/libjuno/include/juno/app/app_api.h';
const JUNO_HEAP_C =
    '/workspaces/libjuno/src/juno_heap.c';
const HEAP_API_H =
    '/workspaces/libjuno/include/juno/ds/heap_api.h';

// ---------------------------------------------------------------------------
// E2E Smoke Tests — Real LibJuno C Files
// ---------------------------------------------------------------------------

describe('E2E Smoke Tests — Real LibJuno C Files', () => {
    let tempDir: string;

    // TC-E2E-SMOKE-001 — engine_app.c
    let engineAppFile: string;
    let indexerSmoke001: WorkspaceIndexer;
    let vtResSmoke001: VtableResolver;

    // TC-E2E-SMOKE-002 — juno_heap.c
    let heapCFile: string;
    let fhResSmoke002: FailureHandlerResolver;

    // TC-E2E-SMOKE-003 — app_api.h (header-only)
    let appApiHeaderFile: string;
    let indexerSmoke003: WorkspaceIndexer;
    let vtResSmoke003: VtableResolver;

    // -----------------------------------------------------------------------
    // Fixture setup — copy real files, index, build resolvers
    // -----------------------------------------------------------------------

    beforeAll(async () => {
        tempDir = fs.mkdtempSync(path.join(os.tmpdir(), 'libjuno-e2e-'));

        // -------------------------------------------------------------------
        // TC-E2E-SMOKE-001 — engine_app.c + app_api.h
        //
        // engine_app.c contains:
        //   static const JUNO_APP_API_T tEngineAppApi = {
        //       .OnStart = OnStart,
        //       .OnProcess = OnProcess,
        //       .OnExit = OnExit
        //   };
        //   ...
        //   ptEngineApp->tRoot.ptApi = &tEngineAppApi;  // line 110
        //
        // app_api.h defines JUNO_APP_ROOT_T via JUNO_MODULE_ROOT. The parser
        // correctly handles the JUNO_MODULE_EMPTY pattern in production headers,
        // so moduleRoots IS populated (JUNO_APP_ROOT_T → JUNO_APP_API_T).
        // vtableAssignments IS also populated from the .c file itself.
        // -------------------------------------------------------------------

        const dir001 = path.join(tempDir, 'smoke001');
        fs.mkdirSync(dir001);
        const appApiH001 = path.join(dir001, 'app_api.h');
        engineAppFile = path.join(dir001, 'engine_app.c');
        fs.copyFileSync(APP_API_H, appApiH001);
        fs.copyFileSync(ENGINE_APP_C, engineAppFile);

        indexerSmoke001 = new WorkspaceIndexer(dir001, []);
        // Index header first so any root definitions reach the impl file.
        await indexerSmoke001.reindexFile(appApiH001);
        await indexerSmoke001.reindexFile(engineAppFile);
        vtResSmoke001 = new VtableResolver(indexerSmoke001.index);

        // -------------------------------------------------------------------
        // TC-E2E-SMOKE-002 — juno_heap.c + heap_api.h
        //
        // juno_heap.c line 40:
        //   ptHeap->_pfcnFailureHandler = pfcnFailureHdlr;
        //
        // `pfcnFailureHdlr` is a function parameter (not a named function
        // definition in the compilation unit), so functionDefinitions and
        // failureHandlerAssignments remain empty for this root type.
        // -------------------------------------------------------------------

        const dir002 = path.join(tempDir, 'smoke002');
        fs.mkdirSync(dir002);
        const heapApiH002 = path.join(dir002, 'heap_api.h');
        heapCFile = path.join(dir002, 'juno_heap.c');
        fs.copyFileSync(HEAP_API_H, heapApiH002);
        fs.copyFileSync(JUNO_HEAP_C, heapCFile);

        const indexerSmoke002 = new WorkspaceIndexer(dir002, []);
        await indexerSmoke002.reindexFile(heapApiH002);
        await indexerSmoke002.reindexFile(heapCFile);
        fhResSmoke002 = new FailureHandlerResolver(indexerSmoke002.index);

        // -------------------------------------------------------------------
        // TC-E2E-SMOKE-003 — app_api.h (header-only)
        //
        // Contains JUNO_MODULE_ROOT and API struct definitions but NO vtable
        // call sites. The VtableResolver should return found:false cleanly.
        // -------------------------------------------------------------------

        const dir003 = path.join(tempDir, 'smoke003');
        fs.mkdirSync(dir003);
        appApiHeaderFile = path.join(dir003, 'app_api.h');
        fs.copyFileSync(APP_API_H, appApiHeaderFile);

        indexerSmoke003 = new WorkspaceIndexer(dir003, []);
        await indexerSmoke003.reindexFile(appApiHeaderFile);
        vtResSmoke003 = new VtableResolver(indexerSmoke003.index);
    });

    afterAll(() => {
        fs.rmSync(tempDir, { recursive: true, force: true });
    });

    // -----------------------------------------------------------------------
    // TC-E2E-SMOKE-001
    //
    // PRIMARY assertion: the WorkspaceIndexer correctly parses the static
    // const struct initializer in engine_app.c and populates vtableAssignments
    // with JUNO_APP_API_T → { OnStart, OnProcess, OnExit }.
    //
    // SECONDARY assertion: VtableResolver.resolve() does not throw when called
    // on the real vtable assignment line (line 110); it returns found:false
    // with the exact "No LibJuno API call pattern found" message because
    // `ptEngineApp->tRoot.ptApi = &tEngineAppApi;` is an *assignment*,
    // not a ->field( call site.
    // -----------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-001", "REQ-VSCODE-002"]}
    it('TC-E2E-SMOKE-001: indexes vtable assignments from real engine_app.c without crashing', () => {
        // PRIMARY — vtableAssignments is populated from the real .c file.
        const apiFieldMap = indexerSmoke001.index.vtableAssignments.get('JUNO_APP_API_T');
        expect(apiFieldMap).toBeDefined();
        expect(apiFieldMap!.has('OnStart')).toBe(true);
        expect(apiFieldMap!.has('OnProcess')).toBe(true);
        expect(apiFieldMap!.has('OnExit')).toBe(true);

        // Verify the ConcreteLocation for OnStart has the correct function name.
        // (line is 0 because complex #include/macro headers prevent indexing of
        // function definitions in this file; that is a known parser limitation.)
        const onStartLocs = apiFieldMap!.get('OnStart')!;
        expect(onStartLocs).toHaveLength(1);
        expect(onStartLocs[0].functionName).toBe('OnStart');
        expect(onStartLocs[0].file).toBe(engineAppFile);

        // moduleRoots is populated from app_api.h — JUNO_APP_ROOT_T is now
        // correctly indexed by the parser (JUNO_MODULE_EMPTY pattern supported).
        const rootEntry001 = indexerSmoke001.index.moduleRoots.get('JUNO_APP_ROOT_T');
        expect(rootEntry001).toBe('JUNO_APP_API_T');

        // SECONDARY — resolver handles line 110 (the vtable assignment) without
        // throwing.  Assignment lines do not match any of the three ->field(
        // strategies, so the resolver must return found:false gracefully.
        //
        // Line 110: `    ptEngineApp->tRoot.ptApi = &tEngineAppApi;`
        const result = vtResSmoke001.resolve(
            engineAppFile,
            110,
            4,
            'ptEngineApp->tRoot.ptApi = &tEngineAppApi;',
        );
        expect(result.found).toBe(false);
        expect(result.locations).toHaveLength(0);
        expect(result.errorMsg).toBe('No LibJuno API call pattern found at cursor position.');
    });

    // -----------------------------------------------------------------------
    // TC-E2E-SMOKE-002
    //
    // FailureHandlerResolver is called on the real failure-handler assignment
    // line in juno_heap.c (line 40):
    //   `ptHeap->_pfcnFailureHandler = pfcnFailureHdlr;`
    //
    // Expected behaviour (empirically verified):
    //  1. Pattern presence detected (_pfcnFailureHandler in lineText).
    //  2. ASSIGNMENT_RE matches; RHS = "pfcnFailureHdlr" → not in
    //     functionDefinitions (it is a parameter, not a definition).
    //  3. LHS primary "ptHeap" → type JUNO_DS_HEAP_ROOT_T (from function
    //     parameters indexed for JunoDs_Heap_Init).
    //  4. walkToRootType → JUNO_DS_HEAP_ROOT_T (no further derivation).
    //  5. failureHandlerAssignments.get("JUNO_DS_HEAP_ROOT_T") → empty
    //     (no concrete handler is assigned inside the indexed files).
    //  Result: found:false, exact errorMsg naming the resolved root type.
    //  The specific error proves the resolver completed type resolution
    //  correctly — it did not short-circuit or throw.
    // -----------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-016"]}
    it('TC-E2E-SMOKE-002: FailureHandlerResolver resolves root type from real juno_heap.c and returns registered failure handlers', () => {
        // Line 40 of juno_heap.c — trimmed to remove indentation.
        const line40text = 'ptHeap->_pfcnFailureHandler = pfcnFailureHdlr;';

        const result = fhResSmoke002.resolve(
            heapCFile,
            40,
            4,
            line40text,
        );

        // With heap_api.h now parsed correctly, JUNO_DS_HEAP_ROOT_T is indexed
        // in moduleRoots and failureHandlerAssignments is populated from
        // juno_heap.c.  The resolver walks ptHeap → JUNO_DS_HEAP_ROOT_T and
        // finds the registered handlers, so it must return found:true.
        expect(result.found).toBe(true);
        expect(result.locations.length).toBeGreaterThanOrEqual(1);
        // pfcnFailureHdlr is the primary handler stored in _pfcnFailureHandler.
        expect(result.locations.some(l => l.functionName === 'pfcnFailureHdlr')).toBe(true);
    });

    // -----------------------------------------------------------------------
    // TC-E2E-SMOKE-003
    //
    // app_api.h is a header-only file: it defines JUNO_APP_ROOT_T and
    // JUNO_APP_API_T but contains no vtable call sites. The VtableResolver
    // must return found:false with the "No LibJuno API call pattern found"
    // message when invoked on any non-call-site line from this file.
    // -----------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-001", "REQ-VSCODE-002"]}
    it('TC-E2E-SMOKE-003: VtableResolver returns found:false for header-only file with no vtable call sites', () => {
        // The header defines JUNO_APP_ROOT_T, which is now correctly indexed
        // by the parser (JUNO_MODULE_EMPTY pattern in production headers supported).
        const rootEntry003 = indexerSmoke003.index.moduleRoots.get('JUNO_APP_ROOT_T');
        expect(rootEntry003).toBe('JUNO_APP_API_T');

        // Line 47 of app_api.h: `typedef struct JUNO_APP_API_TAG JUNO_APP_API_T;`
        // No strategy matches a typedef statement (no `->field(` pattern).
        const result = vtResSmoke003.resolve(
            appApiHeaderFile,
            47,
            0,
            'typedef struct JUNO_APP_API_TAG JUNO_APP_API_T;',
        );

        expect(result.found).toBe(false);
        expect(result.locations).toHaveLength(0);
        expect(result.errorMsg).toBe('No LibJuno API call pattern found at cursor position.');
    });
});
