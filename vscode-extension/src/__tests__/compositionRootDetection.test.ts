/// <reference types="jest" />
/**
 * @file compositionRootDetection.test.ts
 *
 * Integration tests for comment/string immunity and multi-caller collection
 * in WorkspaceIndexer.resolveCompositionRoots() and resolveInitCallers().
 *
 * TC-COMPROOT-001  Line-comment immunity — false positive ignored           (REQ-VSCODE-044)
 * TC-COMPROOT-002  Block-comment immunity — false positive ignored          (REQ-VSCODE-044)
 * TC-COMPROOT-003  String-literal immunity — false positive ignored         (REQ-VSCODE-044)
 * TC-COMPROOT-004  All callers collected — allCompRoots has all entries     (REQ-VSCODE-045)
 * TC-COMPROOT-005  Single-caller baseline — allCompRoots has exactly 1     (REQ-VSCODE-045)
 */

import * as fs from 'fs';
import * as path from 'path';
import * as os from 'os';
import { WorkspaceIndexer } from '../indexer/workspaceIndexer';
import { ConcreteLocation } from '../parser/types';

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/**
 * Collect all ConcreteLocation objects that have an apiVarName matching
 * the given name.
 */
function locationsForVar(
    indexer: WorkspaceIndexer,
    apiVarName: string
): ConcreteLocation[] {
    const result: ConcreteLocation[] = [];
    for (const fieldMap of indexer.index.vtableAssignments.values()) {
        for (const locs of fieldMap.values()) {
            for (const loc of locs) {
                if (loc.apiVarName === apiVarName) {
                    result.push(loc);
                }
            }
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// Shared C source templates
// ---------------------------------------------------------------------------

/** api.h content — defines the FOO module API and root. */
const API_H = [
    '#ifndef FOO_API_H',
    '#define FOO_API_H',
    'typedef struct FOO_API_TAG FOO_API_T;',
    'struct FOO_API_TAG { void (*doThing)(void); };',
    '#endif',
].join('\n');

/** impl.c content — provides the vtable assignment. */
const IMPL_C = [
    '#include "api.h"',
    'static void foo_do_thing(void) {}',
    'FOO_API_T gtFooApi = { .doThing = foo_do_thing };',
].join('\n');

// ---------------------------------------------------------------------------
// Suite
// ---------------------------------------------------------------------------

describe('Composition root detection — comment and string immunity (REQ-VSCODE-044)', () => {

    let tempDir: string;
    let indexer: WorkspaceIndexer;

    beforeAll(() => {
        tempDir = fs.mkdtempSync(path.join(os.tmpdir(), 'libjuno-comproot-'));
    });

    afterAll(() => {
        fs.rmSync(tempDir, { recursive: true, force: true });
    });

    afterEach(() => {
        if (indexer) {
            indexer.dispose();
        }
    });

    // =========================================================================
    // TC-COMPROOT-001: Line-comment immunity
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-044"]}
    it('TC-COMPROOT-001: line-comment occurrence of &gtFooApi is not recorded as a call site', async () => {
        const dir = path.join(tempDir, 'tc001');
        fs.mkdirSync(dir, { recursive: true });

        fs.writeFileSync(path.join(dir, 'api.h'), API_H);
        fs.writeFileSync(path.join(dir, 'impl.c'), IMPL_C);

        const callerPath = path.join(dir, 'caller.c');
        // Line 3: a real call to a function that passes &gtFooApi.
        // Line 4: the same text, but inside a line comment — must NOT be recorded.
        fs.writeFileSync(
            callerPath,
            [
                '#include "api.h"',
                'void FooInit(FOO_API_T *ptApi) {}',
                'void AppInit(void) { FooInit(&gtFooApi); }',
                '// FooInit(&gtFooApi);',
            ].join('\n')
        );

        indexer = new WorkspaceIndexer(dir, []);
        await indexer.fullIndex();

        // The initCallIndex must contain gtFooApi.
        const sites = indexer.index.initCallIndex.get('gtFooApi');
        expect(sites).toBeDefined();
        expect(sites!.length).toBeGreaterThan(0);

        // All recorded sites must come from real code lines (line 3, 1-based),
        // NOT the comment line (line 4).
        const commentLine = 4;
        const commentSite = sites!.find(s => s.file === callerPath && s.line === commentLine);
        expect(commentSite).toBeUndefined();

        // The real call (line 3) must be recorded.
        const realLine = 3;
        const realSite = sites!.find(s => s.file === callerPath && s.line === realLine);
        expect(realSite).toBeDefined();

        // ConcreteLocation must be stamped with the real call line, not the comment line.
        const locs = locationsForVar(indexer, 'gtFooApi');
        expect(locs.length).toBeGreaterThan(0);
        const loc = locs[0];
        expect(loc.initCallFile).toBe(callerPath);
        expect(loc.initCallLine).toBe(realLine);
    });

    // =========================================================================
    // TC-COMPROOT-002: Block-comment immunity
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-044"]}
    it('TC-COMPROOT-002: block-comment occurrence of &gtFooApi is not recorded as a call site', async () => {
        const dir = path.join(tempDir, 'tc002');
        fs.mkdirSync(dir, { recursive: true });

        fs.writeFileSync(path.join(dir, 'api.h'), API_H);
        fs.writeFileSync(path.join(dir, 'impl.c'), IMPL_C);

        const callerPath = path.join(dir, 'caller.c');
        // Line 3: real call. Lines 4-6: block comment containing a false positive.
        fs.writeFileSync(
            callerPath,
            [
                '#include "api.h"',
                'void FooInit(FOO_API_T *ptApi) {}',
                'void AppInit(void) { FooInit(&gtFooApi); }',
                '/*',
                ' * FooInit(&gtFooApi);',
                ' */',
            ].join('\n')
        );

        indexer = new WorkspaceIndexer(dir, []);
        await indexer.fullIndex();

        const sites = indexer.index.initCallIndex.get('gtFooApi');
        expect(sites).toBeDefined();
        expect(sites!.length).toBeGreaterThan(0);

        // Lines 4, 5, 6 are inside the block comment — none must be recorded.
        for (const blockedLine of [4, 5, 6]) {
            const falseSite = sites!.find(s => s.file === callerPath && s.line === blockedLine);
            expect(falseSite).toBeUndefined();
        }

        // The real call on line 3 must be recorded.
        const realSite = sites!.find(s => s.file === callerPath && s.line === 3);
        expect(realSite).toBeDefined();

        // ConcreteLocation stamped correctly.
        const locs = locationsForVar(indexer, 'gtFooApi');
        expect(locs.length).toBeGreaterThan(0);
        expect(locs[0].initCallFile).toBe(callerPath);
        expect(locs[0].initCallLine).toBe(3);
    });

    // =========================================================================
    // TC-COMPROOT-003: String-literal immunity
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-044"]}
    it('TC-COMPROOT-003: string-literal occurrence of &gtFooApi is not recorded as a call site', async () => {
        const dir = path.join(tempDir, 'tc003');
        fs.mkdirSync(dir, { recursive: true });

        fs.writeFileSync(path.join(dir, 'api.h'), API_H);
        fs.writeFileSync(path.join(dir, 'impl.c'), IMPL_C);

        const callerPath = path.join(dir, 'caller.c');
        // Line 3: real call. Line 4: false positive inside a string literal.
        fs.writeFileSync(
            callerPath,
            [
                '#include "api.h"',
                'void FooInit(FOO_API_T *ptApi) {}',
                'void AppInit(void) { FooInit(&gtFooApi); }',
                'const char *gszDesc = "FooInit(&gtFooApi)";',
            ].join('\n')
        );

        indexer = new WorkspaceIndexer(dir, []);
        await indexer.fullIndex();

        const sites = indexer.index.initCallIndex.get('gtFooApi');
        expect(sites).toBeDefined();
        expect(sites!.length).toBeGreaterThan(0);

        // Line 4 is a string literal — must NOT be recorded.
        const stringSite = sites!.find(s => s.file === callerPath && s.line === 4);
        expect(stringSite).toBeUndefined();

        // The real call on line 3 must be recorded.
        const realSite = sites!.find(s => s.file === callerPath && s.line === 3);
        expect(realSite).toBeDefined();

        // ConcreteLocation stamped with the real call site.
        const locs = locationsForVar(indexer, 'gtFooApi');
        expect(locs.length).toBeGreaterThan(0);
        expect(locs[0].initCallFile).toBe(callerPath);
        expect(locs[0].initCallLine).toBe(3);
    });

});

describe('Composition root detection — all callers collected (REQ-VSCODE-045)', () => {

    let tempDir: string;
    let indexer: WorkspaceIndexer;

    beforeAll(() => {
        tempDir = fs.mkdtempSync(path.join(os.tmpdir(), 'libjuno-allroots-'));
    });

    afterAll(() => {
        fs.rmSync(tempDir, { recursive: true, force: true });
    });

    afterEach(() => {
        if (indexer) {
            indexer.dispose();
        }
    });

    // =========================================================================
    // TC-COMPROOT-004: Multiple callers → allCompRoots contains all entries
    //
    // Scenario:
    //   impl.c defines gtFooApi (vtable assignment, apiVarName = "gtFooApi")
    //   wiring.c defines AppInit() which calls FooInit(&gtFooApi, ...)
    //   caller_a.c defines CallerA() which calls AppInit()
    //   caller_b.c defines CallerB() which calls AppInit()
    //
    //   resolveCompositionRoots() stamps initCallFile = wiring.c (line of &gtFooApi).
    //   resolveInitCallers() finds enclosing function = AppInit, then searches
    //   all .c files for callers of AppInit → finds caller_a.c and caller_b.c.
    //   allCompRoots must contain both; entries sorted by file path.
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-045"]}
    it('TC-COMPROOT-004: allCompRoots contains all callers of the Init enclosing function, sorted by file', async () => {
        const dir = path.join(tempDir, 'tc004');
        fs.mkdirSync(dir, { recursive: true });

        fs.writeFileSync(path.join(dir, 'api.h'), API_H);

        // impl.c: vtable assignment with a function definition for doThing.
        fs.writeFileSync(
            path.join(dir, 'impl.c'),
            [
                '#include "api.h"',
                'static void foo_do_thing(void) {}',
                'FOO_API_T gtFooApi = { .doThing = foo_do_thing };',
            ].join('\n')
        );

        // wiring.c: defines AppInit which passes &gtFooApi to FooInit.
        // FooInit is declared here so the parser sees a function definition.
        const wiringPath = path.join(dir, 'wiring.c');
        fs.writeFileSync(
            wiringPath,
            [
                '#include "api.h"',
                'static FOO_API_T tRoot;',
                'void FooInit(FOO_API_T *ptApi) {}',
                'void AppInit(void) { FooInit(&gtFooApi); }',
            ].join('\n')
        );

        // caller_a.c: calls AppInit.
        const callerAPath = path.join(dir, 'caller_a.c');
        fs.writeFileSync(
            callerAPath,
            // No forward declaration — avoid the declaration line matching the call pattern.
            'void CallerA(void) { AppInit(); }\n'
        );

        // caller_b.c: also calls AppInit.
        const callerBPath = path.join(dir, 'caller_b.c');
        fs.writeFileSync(
            callerBPath,
            'void CallerB(void) { AppInit(); }\n'
        );

        indexer = new WorkspaceIndexer(dir, []);
        await indexer.fullIndex();

        const locs = locationsForVar(indexer, 'gtFooApi');
        expect(locs.length).toBeGreaterThan(0);

        const loc = locs[0];

        // allCompRoots must be defined and contain exactly 2 entries
        // (one from caller_a.c, one from caller_b.c).
        expect(loc.allCompRoots).toBeDefined();
        expect(loc.allCompRoots!.length).toBe(2);

        // Entries must be sorted by file path ascending.
        const files = loc.allCompRoots!.map(e => e.file);
        const sorted = [...files].sort();
        expect(files).toEqual(sorted);

        // Both caller files must appear in allCompRoots.
        expect(files).toContain(callerAPath);
        expect(files).toContain(callerBPath);

        // compRootFile must equal the first sorted entry.
        expect(loc.compRootFile).toBe(loc.allCompRoots![0].file);

        // compRootLine must equal the first sorted entry's line.
        expect(loc.compRootLine).toBe(loc.allCompRoots![0].line);
    });

    // =========================================================================
    // TC-COMPROOT-005: Single caller baseline — allCompRoots has exactly 1 entry
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-045"]}
    it('TC-COMPROOT-005: single caller of the Init enclosing function → allCompRoots has exactly 1 entry', async () => {
        const dir = path.join(tempDir, 'tc005');
        fs.mkdirSync(dir, { recursive: true });

        fs.writeFileSync(path.join(dir, 'api.h'), API_H);

        fs.writeFileSync(
            path.join(dir, 'impl.c'),
            [
                '#include "api.h"',
                'static void foo_do_thing(void) {}',
                'FOO_API_T gtFooApi = { .doThing = foo_do_thing };',
            ].join('\n')
        );

        // wiring.c: defines AppInit which passes &gtFooApi to FooInit.
        fs.writeFileSync(
            path.join(dir, 'wiring.c'),
            [
                '#include "api.h"',
                'void FooInit(FOO_API_T *ptApi) {}',
                'void AppInit(void) { FooInit(&gtFooApi); }',
            ].join('\n')
        );

        // sole_caller.c: the only caller of AppInit.
        const soleCallerPath = path.join(dir, 'sole_caller.c');
        fs.writeFileSync(
            soleCallerPath,
            // No forward declaration — avoid the declaration line matching the call pattern.
            'void Main(void) { AppInit(); }\n'
        );

        indexer = new WorkspaceIndexer(dir, []);
        await indexer.fullIndex();

        const locs = locationsForVar(indexer, 'gtFooApi');
        expect(locs.length).toBeGreaterThan(0);

        const loc = locs[0];

        // allCompRoots must be defined with exactly 1 entry.
        expect(loc.allCompRoots).toBeDefined();
        expect(loc.allCompRoots!.length).toBe(1);

        // The sole entry must be in sole_caller.c.
        expect(loc.allCompRoots![0].file).toBe(soleCallerPath);

        // compRootFile and compRootLine must be set and match allCompRoots[0].
        expect(loc.compRootFile).toBe(soleCallerPath);
        expect(loc.compRootLine).toBe(loc.allCompRoots![0].line);
    });

});
