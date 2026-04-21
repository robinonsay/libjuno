/// <reference types="jest" />
/**
 * @file fileSystemEvents.test.ts
 *
 * Tests for FileSystemWatcher integration in the LibJuno VSCode extension.
 * Verifies automatic indexing on file creation (REQ-VSCODE-042) and automatic
 * index cleanup on file deletion (REQ-VSCODE-043).
 *
 * TC-FSE-001  onDidCreate registered on activation                          (REQ-VSCODE-042)
 * TC-FSE-002  New .c file created → vtable assignment indexed               (REQ-VSCODE-042)
 * TC-FSE-003  New .h file created → module root record indexed              (REQ-VSCODE-042)
 * TC-FSE-004  Negative — FSW glob excludes non-C files                      (REQ-VSCODE-042)
 * TC-FSE-005  onDidDelete registered on activation                          (REQ-VSCODE-043)
 * TC-FSE-006  File deleted → all records removed, other files intact        (REQ-VSCODE-043)
 * TC-FSE-007  Deleted file's hash removed → scheduleSave called             (REQ-VSCODE-043)
 * TC-FSE-008  Delete sole implementation file → resolver returns found:false (REQ-VSCODE-042, REQ-VSCODE-043)
 * TC-FSE-009  ENOENT on create → no crash, no stale entry                  (REQ-VSCODE-042)
 */

import * as fs from 'fs';
import * as path from 'path';
import * as os from 'os';
import * as vscode from 'vscode';
import { createMockExtensionContext, resetMocks } from '../__mocks__/vscode';
import { VtableResolver } from '../resolver/vtableResolver';

// ---------------------------------------------------------------------------
// Prevent MCP server from binding a real TCP port during activation tests.
// ---------------------------------------------------------------------------
jest.mock('../mcp/mcpServer', () => ({
    McpServer: jest.fn().mockImplementation(() => ({
        start: jest.fn().mockResolvedValue(6543),
        stop: jest.fn(),
    })),
}));

// ---------------------------------------------------------------------------
// Mock WorkspaceIndexer for activation tests only (TC-FSE-001, 004, 005).
// Real indexer tests use jest.requireActual to bypass this mock.
// ---------------------------------------------------------------------------
jest.mock('../indexer/workspaceIndexer', () => {
    const actual = jest.requireActual('../indexer/workspaceIndexer');
    return {
        ...actual,
        WorkspaceIndexer: jest.fn().mockImplementation(() => ({
            loadFromCache: jest.fn().mockResolvedValue(false),
            fullIndex: jest.fn().mockResolvedValue(undefined),
            reindexFile: jest.fn().mockResolvedValue(undefined),
            removeFile: jest.fn(),
            index: {
                localTypeInfo: new Map(),
                vtableAssignments: new Map(),
                functionDefinitions: new Map(),
                moduleRoots: new Map(),
                traitRoots: new Map(),
                derivationChain: new Map(),
                apiStructFields: new Map(),
                failureHandlerAssignments: new Map(),
                apiMemberRegistry: new Map(),
                initCallIndex: new Map(),
            },
            dispose: jest.fn(),
        })),
    };
});

// Import activate AFTER jest.mock() calls so mocks are in place.
import { activate } from '../extension';

// Real WorkspaceIndexer constructor — bypasses the module-level mock above.
// eslint-disable-next-line @typescript-eslint/no-var-requires
const RealWorkspaceIndexer: typeof import('../indexer/workspaceIndexer').WorkspaceIndexer =
    jest.requireActual('../indexer/workspaceIndexer').WorkspaceIndexer;

// ---------------------------------------------------------------------------
// Helper: get the FSW watcher returned by createFileSystemWatcher.
// ---------------------------------------------------------------------------
function getActivatedWatcher(): any {
    return (vscode.workspace.createFileSystemWatcher as jest.Mock).mock.results[0].value;
}

// ===========================================================================
// TC-FSE-001, TC-FSE-004, TC-FSE-005 — Activation-level watcher registration
// ===========================================================================

describe('FileSystemWatcher — activation registration', () => {

    beforeEach(() => {
        resetMocks();
    });

    afterEach(() => {
        jest.restoreAllMocks();
    });

    // @{"verify": ["REQ-VSCODE-042"]}
    it('TC-FSE-001: onDidCreate is registered with a function callback on activation', async () => {
        const context = createMockExtensionContext();
        await activate(context);

        const watcher = getActivatedWatcher();

        expect(watcher.onDidCreate).toHaveBeenCalledWith(expect.any(Function));
        expect(typeof watcher._onDidCreateCallback).toBe('function');
    });

    // @{"verify": ["REQ-VSCODE-042"]}
    it('TC-FSE-004: createFileSystemWatcher is called with the exact C/C++ glob pattern', async () => {
        const context = createMockExtensionContext();
        await activate(context);

        const glob = (vscode.workspace.createFileSystemWatcher as jest.Mock).mock.calls[0][0];

        expect(glob).toBe('**/*.{c,h,cpp,hpp,hh,cc}');
        expect(glob).not.toContain('**/*.*');
        expect(glob).not.toContain('.txt');
    });

    // @{"verify": ["REQ-VSCODE-043"]}
    it('TC-FSE-005: onDidDelete is registered with a function callback on activation', async () => {
        const context = createMockExtensionContext();
        await activate(context);

        const watcher = getActivatedWatcher();

        expect(watcher.onDidDelete).toHaveBeenCalledWith(expect.any(Function));
        expect(typeof watcher._onDidDeleteCallback).toBe('function');
    });

});

// ===========================================================================
// TC-FSE-002, 003, 006–009 — Real indexer integration tests
// ===========================================================================

describe('FileSystemWatcher — real indexer integration', () => {

    let tempDir: string;
    let indexer: InstanceType<typeof RealWorkspaceIndexer>;

    beforeAll(() => {
        tempDir = fs.mkdtempSync(path.join(os.tmpdir(), 'libjuno-fse-'));
    });

    afterAll(() => {
        fs.rmSync(tempDir, { recursive: true, force: true });
    });

    afterEach(() => {
        if (indexer) {
            indexer.dispose();
        }
        jest.restoreAllMocks();
        jest.useRealTimers();
    });

    // =========================================================================
    // TC-FSE-002: New .c file created → vtable assignment indexed
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-042"]}
    it('TC-FSE-002: reindexFile() on a new .c file indexes a vtable assignment', async () => {
        const dir = path.join(tempDir, 'tc002');
        fs.mkdirSync(dir, { recursive: true });

        const filePath = path.join(dir, 'newfile.c');
        fs.writeFileSync(
            filePath,
            'JUNO_LOG_API_T gtApi = { .LogInfo = MyLogInfo };\n'
        );

        indexer = new RealWorkspaceIndexer(dir, []);

        // Pre-condition: index is empty before reindexFile is called.
        expect(indexer.index.vtableAssignments.size).toBe(0);

        await indexer.reindexFile(filePath);

        const fieldMap = indexer.index.vtableAssignments.get('JUNO_LOG_API_T');
        expect(fieldMap).toBeDefined();
        const locs = fieldMap!.get('LogInfo');
        expect(locs).toBeDefined();
        expect(locs!.length).toBeGreaterThan(0);
        // The concrete location must reference the newly created file.
        expect(locs!.some(l => l.file === filePath)).toBe(true);
    });

    // =========================================================================
    // TC-FSE-003: New .h file created → module root record indexed
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-042"]}
    it('TC-FSE-003: reindexFile() on a new .h file indexes a module root mapping', async () => {
        const dir = path.join(tempDir, 'tc003');
        fs.mkdirSync(dir, { recursive: true });

        const filePath = path.join(dir, 'module.h');
        fs.writeFileSync(
            filePath,
            [
                'typedef struct MY_MODULE_ROOT_TAG MY_MODULE_ROOT_T;',
                'typedef struct MY_MODULE_API_TAG MY_MODULE_API_T;',
                'struct MY_MODULE_ROOT_TAG JUNO_MODULE_ROOT(MY_MODULE_API_T, );',
            ].join('\n')
        );

        indexer = new RealWorkspaceIndexer(dir, []);

        // Pre-condition: index is empty before reindexFile is called.
        expect(indexer.index.moduleRoots.size).toBe(0);

        await indexer.reindexFile(filePath);

        expect(indexer.index.moduleRoots.get('MY_MODULE_ROOT_T')).toBe('MY_MODULE_API_T');
    });

    // =========================================================================
    // TC-FSE-006: File deleted → all records removed, other files intact
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-043"]}
    it('TC-FSE-006: removeFile() removes all records for the deleted file, leaves other files intact', async () => {
        const dir = path.join(tempDir, 'tc006');
        fs.mkdirSync(dir, { recursive: true });

        const fooPath = path.join(dir, 'foo.c');
        const barPath = path.join(dir, 'bar.c');

        fs.writeFileSync(
            fooPath,
            [
                'JUNO_LOG_API_T gtFooApi = { .LogInfo = FooLogInfo };',
                'static void FooLogInfo(void) {}',
            ].join('\n')
        );
        fs.writeFileSync(barPath, 'static void BarFunction(void) {}');

        indexer = new RealWorkspaceIndexer(dir, []);
        await indexer.fullIndex();

        // Pre-condition: both files contribute records to the index.
        const hasFooDef = Array.from(indexer.index.functionDefinitions.values())
            .some(defs => defs.some(d => d.file === fooPath));
        const hasBarDef = Array.from(indexer.index.functionDefinitions.values())
            .some(defs => defs.some(d => d.file === barPath));
        expect(hasFooDef).toBe(true);
        expect(hasBarDef).toBe(true);

        // Simulate onDidDelete firing for foo.c.
        indexer.removeFile(fooPath);

        // All vtable assignment records from foo.c must be gone.
        const fooVtableRemains = Array.from(indexer.index.vtableAssignments.values())
            .some(fieldMap =>
                Array.from(fieldMap.values())
                    .some(locs => locs.some(l => l.file === fooPath))
            );
        expect(fooVtableRemains).toBe(false);

        // All function definition records from foo.c must be gone.
        const fooDefRemains = Array.from(indexer.index.functionDefinitions.values())
            .some(defs => defs.some(d => d.file === fooPath));
        expect(fooDefRemains).toBe(false);

        // Records from bar.c must remain intact.
        const barDefStillPresent = Array.from(indexer.index.functionDefinitions.values())
            .some(defs => defs.some(d => d.file === barPath));
        expect(barDefStillPresent).toBe(true);
    });

    // =========================================================================
    // TC-FSE-007: Deleted file's hash removed → scheduleSave called
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-043"]}
    it('TC-FSE-007: removeFile() schedules a debounced cache save (timer is scheduled)', async () => {
        const dir = path.join(tempDir, 'tc007');
        fs.mkdirSync(dir, { recursive: true });

        const filePath = path.join(dir, 'foo.c');
        fs.writeFileSync(filePath, 'static void Foo007(void) {}\n');

        indexer = new RealWorkspaceIndexer(dir, []);

        // Call fullIndex() with real timers before switching to fake timers.
        // fullIndex() calls saveToCache() directly (not via scheduleSave), so the
        // cache write completes before we install the spy and fake timers.
        await indexer.fullIndex();

        // Switch to fake timers so scheduleSave()'s setTimeout is captured.
        jest.useFakeTimers();

        const saveSpy = jest
            .spyOn(indexer, 'saveToCache')
            .mockResolvedValue(undefined);

        // Confirm no debounce timer is pending before removeFile().
        expect(jest.getTimerCount()).toBe(0);

        indexer.removeFile(filePath);

        // Verify the file hash entry was deleted.
        const relPath = path.relative(dir, filePath);
        expect((indexer as any).fileHashes.has(relPath)).toBe(false);

        // scheduleSave() sets a 500 ms debounce timer.
        expect(jest.getTimerCount()).toBeGreaterThan(0);

        // Advance fake timers so the debounce callback fires.
        jest.runAllTimers();

        // saveToCache must have been invoked exactly once by the timer callback.
        expect(saveSpy).toHaveBeenCalledTimes(1);
    });

    // =========================================================================
    // TC-FSE-008: Delete sole implementation file → resolver returns found:false
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-042", "REQ-VSCODE-043"]}
    it('TC-FSE-008: after removeFile() removes the sole implementation, resolver returns found:false', async () => {
        const dir = path.join(tempDir, 'tc008');
        fs.mkdirSync(dir, { recursive: true });

        const fooPath = path.join(dir, 'foo.c');
        fs.writeFileSync(
            fooPath,
            [
                'JUNO_LOG_API_T gtMyApi = { .LogInfo = MyLogInfo };',
                'static void MyLogInfo(void) {}',
            ].join('\n')
        );

        // caller.c is indexed so the resolver does not reject it as "not indexed".
        const callerFile = path.join(dir, 'caller.c');
        fs.writeFileSync(callerFile, 'static void Caller008(void) {}\n');

        indexer = new RealWorkspaceIndexer(dir, []);
        await indexer.fullIndex();

        // Pre-condition: vtable assignment for JUNO_LOG_API_T.LogInfo must be present.
        const fieldMapBefore = indexer.index.vtableAssignments.get('JUNO_LOG_API_T');
        expect(fieldMapBefore).toBeDefined();
        expect(fieldMapBefore!.has('LogInfo')).toBe(true);

        // Simulate onDidDelete firing for foo.c.
        indexer.removeFile(fooPath);

        // Post-condition: the vtable entry is gone.
        const fieldMapAfter = indexer.index.vtableAssignments.get('JUNO_LOG_API_T');
        expect(fieldMapAfter).toBeUndefined();

        // A resolver backed by the cleaned index must return found:false.
        const resolver = new VtableResolver(indexer.index);
        const result = resolver.resolve(callerFile, 1, 5, 'ptApi->LogInfo(');

        expect(result.found).toBe(false);
        // Error message must reference the missing field (meaningful error, not silent failure).
        expect(result.errorMsg).toContain('LogInfo');
    });

    // =========================================================================
    // TC-FSE-009: ENOENT on create → no crash, no stale entry
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-042"]}
    it('TC-FSE-009: reindexFile() on a nonexistent path is a silent no-op with empty index', async () => {
        const dir = path.join(tempDir, 'tc009');
        fs.mkdirSync(dir, { recursive: true });

        const ghostPath = path.join(dir, 'ghost.c');
        // Confirm the file does NOT exist — this simulates a file created then
        // immediately deleted before the onDidCreate handler ran.
        expect(fs.existsSync(ghostPath)).toBe(false);

        indexer = new RealWorkspaceIndexer(dir, []);

        // reindexFile() must not throw for a nonexistent path.
        await expect(indexer.reindexFile(ghostPath)).resolves.toBeUndefined();

        // The index must remain completely empty — no partial records.
        expect(indexer.index.vtableAssignments.size).toBe(0);
        expect(indexer.index.moduleRoots.size).toBe(0);
        expect(indexer.index.functionDefinitions.size).toBe(0);
    });
});
