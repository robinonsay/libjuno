/// <reference types="jest" />
/**
 * @file compositionRootOrdering.test.ts
 *
 * Integration tests for resolveInitCallers() in WorkspaceIndexer.
 *
 * TC-COMPROOT-006  main() recognized as a valid composition root caller   (REQ-VSCODE-047)
 * TC-COMPROOT-007  Multiple callers sorted by filename ascending           (REQ-VSCODE-048)
 * TC-COMPROOT-008  Multiple callers in same file sorted by line ascending  (REQ-VSCODE-048)
 */

import * as fs from 'fs';
import * as path from 'path';
import * as os from 'os';
import { WorkspaceIndexer } from '../indexer/workspaceIndexer';

// ---------------------------------------------------------------------------
// Shared temp directory — created once, torn down after all tests complete.
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Shared C source helpers.
//
// Pattern (mirrors TC-TRACE-025):
//   api.h   — typedef + JUNO_MODULE_ROOT declaration
//   impl.c  — positional vtable assignment + FooInit() that holds &gtFooApi
//   caller  — calls FooInit(); becomes the composition root
//
// The impl.c uses explicit struct-tag syntax so the positional vtable is
// resolved in the same file without deferring (same-file resolution path).
// ---------------------------------------------------------------------------

/** Standard api.h for all three test cases. */
function writeApiHeader(dir: string): string {
    const p = path.join(dir, 'api.h');
    fs.writeFileSync(p, [
        'typedef struct FOO_API_TAG FOO_API_T;',
        'typedef struct FOO_ROOT_TAG FOO_ROOT_T;',
        'struct FOO_ROOT_TAG JUNO_MODULE_ROOT(FOO_API_T, );',
        'struct FOO_API_TAG { void (*DoWork)(FOO_ROOT_T *pt); };',
    ].join('\n'));
    return p;
}

/**
 * Standard impl.c:
 *   - explicit struct tag so the positional vtable is resolved immediately
 *   - &gtFooApi inside FooInit() → resolveCompositionRoots stamps initCallFile
 *   - FooInit calls the Init function, resolveInitCallers then finds who calls FooInit
 */
function writeImplFile(dir: string): string {
    const p = path.join(dir, 'impl.c');
    fs.writeFileSync(p, [
        'static void FooDoWork(FOO_ROOT_T *pt) {}',
        'static const FOO_API_T gtFooApi = { FooDoWork };',
        'void FooInit(FOO_ROOT_T *ptRoot) { ptRoot->ptApi = &gtFooApi; }',
    ].join('\n'));
    return p;
}

// ===========================================================================
// TC-COMPROOT-006 — main() recognized as a valid composition root caller
// ===========================================================================

// @{"verify": ["REQ-VSCODE-047"]}
it('TC-COMPROOT-006: main() calling FooInit is recognised as a valid composition root, not skipped', async () => {
    const dir = path.join(tempDir, 'tc006');
    fs.mkdirSync(dir, { recursive: true });

    writeApiHeader(dir);
    writeImplFile(dir);

    // main.c — main() directly calls FooInit; it must NOT be filtered out.
    const mainPath = path.join(dir, 'main.c');
    fs.writeFileSync(mainPath, [
        '#include "api.h"',
        'int main(void) {',
        '    FOO_ROOT_T tRoot;',
        '    FooInit(&tRoot);',
        '    return 0;',
        '}',
    ].join('\n'));

    indexer = new WorkspaceIndexer(dir, []);
    await indexer.fullIndex();

    // Locate the ConcreteLocation stamped by resolveInitCallers.
    const fieldMap = indexer.index.vtableAssignments.get('FOO_API_T');
    expect(fieldMap).toBeDefined();

    const locs = fieldMap!.get('DoWork');
    expect(locs).toBeDefined();
    expect(locs!.length).toBeGreaterThan(0);

    // At least one location must have compRootFile stamped.
    const loc = locs!.find(l => l.compRootFile !== undefined);
    expect(loc).toBeDefined();

    // compRootFile must point to main.c — main() must NOT be skipped.
    expect(loc!.compRootFile!.endsWith('main.c')).toBe(true);

    // allCompRoots must list exactly one entry (only main.c calls FooInit).
    expect(loc!.allCompRoots).toBeDefined();
    expect(loc!.allCompRoots!.length).toBe(1);
    expect(loc!.allCompRoots![0].file.endsWith('main.c')).toBe(true);
});

// ===========================================================================
// TC-COMPROOT-007 — Multiple callers sorted by filename ascending
// ===========================================================================

// @{"verify": ["REQ-VSCODE-048"]}
it('TC-COMPROOT-007: two callers in different files are sorted by file path ascending', async () => {
    const dir = path.join(tempDir, 'tc007');
    fs.mkdirSync(dir, { recursive: true });

    writeApiHeader(dir);
    writeImplFile(dir);

    // zebra.c — alphabetically last; calls FooInit.
    const zebraPath = path.join(dir, 'zebra.c');
    fs.writeFileSync(zebraPath, [
        '#include "api.h"',
        'void ZebraSetup(void) { FOO_ROOT_T tRoot; FooInit(&tRoot); }',
    ].join('\n'));

    // alpha.c — alphabetically first; also calls FooInit.
    const alphaPath = path.join(dir, 'alpha.c');
    fs.writeFileSync(alphaPath, [
        '#include "api.h"',
        'void AlphaSetup(void) { FOO_ROOT_T tRoot; FooInit(&tRoot); }',
    ].join('\n'));

    indexer = new WorkspaceIndexer(dir, []);
    await indexer.fullIndex();

    const fieldMap = indexer.index.vtableAssignments.get('FOO_API_T');
    expect(fieldMap).toBeDefined();

    const locs = fieldMap!.get('DoWork');
    expect(locs).toBeDefined();

    const loc = locs!.find(l => l.allCompRoots !== undefined);
    expect(loc).toBeDefined();

    const roots = loc!.allCompRoots!;

    // Must have exactly two callers.
    expect(roots.length).toBe(2);

    // alpha.c must come first (alphabetically before zebra.c).
    expect(roots[0].file.endsWith('alpha.c')).toBe(true);
    expect(roots[1].file.endsWith('zebra.c')).toBe(true);

    // compRootFile is the first sorted entry — alpha.c.
    expect(loc!.compRootFile!.endsWith('alpha.c')).toBe(true);
});

// ===========================================================================
// TC-COMPROOT-008 — Multiple callers in same file sorted by line ascending
// ===========================================================================

// @{"verify": ["REQ-VSCODE-048"]}
it('TC-COMPROOT-008: two calls in the same file are sorted by line number ascending', async () => {
    const dir = path.join(tempDir, 'tc008');
    fs.mkdirSync(dir, { recursive: true });

    writeApiHeader(dir);
    writeImplFile(dir);

    // caller.c — two distinct functions each calling FooInit, at different lines.
    // Line 2: Init1 calls FooInit (lower line number — must be allCompRoots[0]).
    // Line 3: Init2 calls FooInit (higher line number — must be allCompRoots[1]).
    const callerPath = path.join(dir, 'caller.c');
    fs.writeFileSync(callerPath, [
        '#include "api.h"',
        'void Init1(void) { FOO_ROOT_T tRoot; FooInit(&tRoot); }',
        'void Init2(void) { FOO_ROOT_T tRoot; FooInit(&tRoot); }',
    ].join('\n'));

    indexer = new WorkspaceIndexer(dir, []);
    await indexer.fullIndex();

    const fieldMap = indexer.index.vtableAssignments.get('FOO_API_T');
    expect(fieldMap).toBeDefined();

    const locs = fieldMap!.get('DoWork');
    expect(locs).toBeDefined();

    const loc = locs!.find(l => l.allCompRoots !== undefined);
    expect(loc).toBeDefined();

    const roots = loc!.allCompRoots!;

    // Must have exactly two callers — one per Init function.
    expect(roots.length).toBe(2);

    // Both callers are in the same file.
    expect(roots[0].file.endsWith('caller.c')).toBe(true);
    expect(roots[1].file.endsWith('caller.c')).toBe(true);

    // Lower-line call must come first.
    expect(roots[0].line).toBeLessThan(roots[1].line);
});
