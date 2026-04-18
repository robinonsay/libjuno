/// <reference types="jest" />
/**
 * @file workspaceIndexer.test.ts
 *
 * Core tests for the WorkspaceIndexer class using real file system I/O and
 * the real Chevrotain parser. Synthetic C source files are written to a
 * temporary directory per test, indexed through the real WorkspaceIndexer
 * public API, and verified against the resulting NavigationIndex.
 *
 * TC-WI-001     fullIndex() indexes 2+ files (REQ-VSCODE-001, REQ-VSCODE-021)
 * TC-WI-002     loadFromCache() — cache valid → skips re-parse (REQ-VSCODE-001)
 * TC-WI-003     loadFromCache() — no cache → returns false (REQ-VSCODE-001)
 * TC-WI-004     reindexFile() — single file re-parsed (REQ-VSCODE-001)
 * TC-WI-005     removeFile() — records removed from index (REQ-VSCODE-001)
 * TC-WI-006     mergeInto() multi-file — no key collision (REQ-VSCODE-001, REQ-VSCODE-021)
 * TC-WI-NEG-001 reindexFile() on never-indexed file → no error (REQ-VSCODE-001)
 * TC-WI-NEG-002 reindexFile() with nonexistent path → silent no-op, index unchanged (REQ-VSCODE-001)
 * TC-WI-BND-001 fullIndex() on empty workspace → empty index (REQ-VSCODE-001)
 * TC-CACHE-003  loadFromCache() re-parses only modified file on hash mismatch (REQ-VSCODE-001)
 * TC-CACHE-004  loadFromCache() parses file added after initial fullIndex() (REQ-VSCODE-001)
 * TC-CACHE-005  loadFromCache() retains stale entries for deleted file — M4 (REQ-VSCODE-001)
 * TC-FILE-001   fullIndex() discovers all 6 C/C++ file extensions (REQ-VSCODE-021)
 * TC-WI-007     Positional vtable initializer resolved, same-file (REQ-VSCODE-012)
 * TC-WI-007b    Cross-file positional vtable resolution (REQ-VSCODE-012)
 * TC-WI-008     Failure handler root-type resolution via localTypeInfo (REQ-VSCODE-016)
 * TC-WI-008b    Multi-module FH disambiguation in same file (REQ-VSCODE-016)
 * TC-WI-009     Hash consistency for BOM files (REQ-VSCODE-001)
 * TC-WI-010     mergeInto() traitRoots loop — single trait root (REQ-VSCODE-001, WI-14.C7)
 * TC-WI-011     mergeInto() traitRoots loop — two files, two trait roots (REQ-VSCODE-001, WI-14.C7)
 */

import * as fs from 'fs';
import * as path from 'path';
import * as os from 'os';
import { WorkspaceIndexer } from '../workspaceIndexer';
import * as visitorModule from '../../parser/visitor';
import { ParsedFile } from '../../parser/types';

// @{"verify": ["REQ-VSCODE-001", "REQ-VSCODE-021"]}
describe('WorkspaceIndexer — core behaviour', () => {

    let tempDir: string;

    beforeAll(() => {
        tempDir = fs.mkdtempSync(path.join(os.tmpdir(), 'libjuno-wi-'));
    });

    afterAll(() => {
        fs.rmSync(tempDir, { recursive: true, force: true });
    });

    afterEach(() => {
        jest.restoreAllMocks();
    });

    // =========================================================================
    // TC-WI-001: fullIndex() indexes 2+ files
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-001", "REQ-VSCODE-021"]}
    it('TC-WI-001: fullIndex() populates moduleRoots, vtableAssignments, and functionDefinitions', async () => {
        const dir = path.join(tempDir, 'tc001');
        fs.mkdirSync(dir);

        // Header: module root declaration + API struct definition
        const header = [
            'typedef struct WI001_API_TAG WI001_API_T;',
            'typedef struct WI001_ROOT_TAG WI001_ROOT_T;',
            '',
            'struct WI001_ROOT_TAG JUNO_MODULE_ROOT(WI001_API_T, );',
            '',
            'struct WI001_API_TAG',
            '{',
            '    void (*DoWork)(WI001_ROOT_T *ptSelf);',
            '};',
            '',
        ].join('\n');

        // Source: function definition + vtable initializer
        const source = [
            'static void DoWork_Impl(WI001_ROOT_T *ptSelf) { (void)ptSelf; }',
            '',
            'static const WI001_API_T tWi001Api = {',
            '    .DoWork = DoWork_Impl,',
            '};',
            '',
        ].join('\n');

        fs.writeFileSync(path.join(dir, 'wi001.h'), header);
        fs.writeFileSync(path.join(dir, 'wi001.c'), source);

        const indexer = new WorkspaceIndexer(dir, []);
        await indexer.fullIndex();

        // moduleRoots: WI001_ROOT_T → WI001_API_T
        expect(indexer.index.moduleRoots.get('WI001_ROOT_T')).toBe('WI001_API_T');

        // vtableAssignments: WI001_API_T.DoWork → DoWork_Impl
        const fieldMap = indexer.index.vtableAssignments.get('WI001_API_T');
        expect(fieldMap).toBeDefined();
        const locs = fieldMap!.get('DoWork');
        expect(locs).toBeDefined();
        expect(locs!.length).toBeGreaterThanOrEqual(1);
        expect(locs![0].functionName).toBe('DoWork_Impl');

        // functionDefinitions: DoWork_Impl recorded
        const defs = indexer.index.functionDefinitions.get('DoWork_Impl');
        expect(defs).toBeDefined();
        expect(defs!.length).toBeGreaterThanOrEqual(1);
        expect(defs![0].functionName).toBe('DoWork_Impl');
    });

    // =========================================================================
    // TC-WI-002: loadFromCache() — cache valid → skips re-parse
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-WI-002: loadFromCache() returns true and does not call parseFileWithDefs on cache hit', async () => {
        const dir = path.join(tempDir, 'tc002');
        fs.mkdirSync(dir);

        const header = [
            'typedef struct WI002_API_TAG WI002_API_T;',
            'typedef struct WI002_ROOT_TAG WI002_ROOT_T;',
            '',
            'struct WI002_ROOT_TAG JUNO_MODULE_ROOT(WI002_API_T, );',
            '',
            'struct WI002_API_TAG',
            '{',
            '    void (*Run)(WI002_ROOT_T *ptSelf);',
            '};',
            '',
        ].join('\n');

        const source = [
            'static void Run_Impl(WI002_ROOT_T *ptSelf) { (void)ptSelf; }',
            '',
            'static const WI002_API_T tWi002Api = {',
            '    .Run = Run_Impl,',
            '};',
            '',
        ].join('\n');

        fs.writeFileSync(path.join(dir, 'wi002.h'), header);
        fs.writeFileSync(path.join(dir, 'wi002.c'), source);

        // Populate and persist cache via fullIndex()
        const indexer1 = new WorkspaceIndexer(dir, []);
        await indexer1.fullIndex();

        // New indexer on same directory — files unchanged, cache should be valid
        const indexer2 = new WorkspaceIndexer(dir, []);
        const spy = jest.spyOn(visitorModule, 'parseFileWithDefs');

        const loaded = await indexer2.loadFromCache();

        // Cache must have been found and loaded
        expect(loaded).toBe(true);
        // All file hashes match → no re-parsing should have occurred
        expect(spy).not.toHaveBeenCalled();
        // Index must be restored from cache
        expect(indexer2.index.moduleRoots.get('WI002_ROOT_T')).toBe('WI002_API_T');
        expect(indexer2.index.vtableAssignments.has('WI002_API_T')).toBe(true);
    });

    // =========================================================================
    // TC-WI-003: loadFromCache() — no cache → returns false
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-WI-003: loadFromCache() returns false and leaves index empty when no cache file exists', async () => {
        const dir = path.join(tempDir, 'tc003');
        fs.mkdirSync(dir);

        // C file present but no .libjuno/navigation-cache.json
        fs.writeFileSync(path.join(dir, 'wi003.c'), 'void PlainFunction(void) {}\n');

        const indexer = new WorkspaceIndexer(dir, []);
        const loaded = await indexer.loadFromCache();

        expect(loaded).toBe(false);
        expect(indexer.index.moduleRoots.size).toBe(0);
        expect(indexer.index.vtableAssignments.size).toBe(0);
        expect(indexer.index.functionDefinitions.size).toBe(0);
    });

    // =========================================================================
    // TC-WI-004: reindexFile() — single file re-parsed
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-WI-004: reindexFile() calls parseFileWithDefs exactly once and reflects the updated content', async () => {
        const dir = path.join(tempDir, 'tc004');
        fs.mkdirSync(dir);

        const srcPath = path.join(dir, 'wi004.c');

        // Initial version
        fs.writeFileSync(srcPath, 'void OldFunction(void) {}\n');

        const indexer = new WorkspaceIndexer(dir, []);
        await indexer.fullIndex();

        // OldFunction must be in the index before the change
        expect(indexer.index.functionDefinitions.has('OldFunction')).toBe(true);

        // Overwrite with new content
        fs.writeFileSync(srcPath, 'void NewFunction(void) {}\n');

        const spy = jest.spyOn(visitorModule, 'parseFileWithDefs');
        await indexer.reindexFile(srcPath);

        // Exactly one parse for the one modified file
        expect(spy).toHaveBeenCalledTimes(1);

        // Index must reflect the new function name
        expect(indexer.index.functionDefinitions.has('NewFunction')).toBe(true);
        const newDefs = indexer.index.functionDefinitions.get('NewFunction')!;
        expect(newDefs[0].functionName).toBe('NewFunction');

        // OldFunction records from this file must be gone
        expect(indexer.index.functionDefinitions.has('OldFunction')).toBe(false);
    });

    // =========================================================================
    // TC-WI-005: removeFile() — records removed
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-WI-005: removeFile() removes vtableAssignments and functionDefinitions sourced from the removed file', async () => {
        const dir = path.join(tempDir, 'tc005');
        fs.mkdirSync(dir);

        // Header with module root
        const header = [
            'typedef struct WI005_API_TAG WI005_API_T;',
            'typedef struct WI005_ROOT_TAG WI005_ROOT_T;',
            '',
            'struct WI005_ROOT_TAG JUNO_MODULE_ROOT(WI005_API_T, );',
            '',
            'struct WI005_API_TAG',
            '{',
            '    void (*Execute)(WI005_ROOT_T *ptSelf);',
            '};',
            '',
        ].join('\n');

        // Source with vtable init + function def
        const srcPath = path.join(dir, 'wi005.c');
        const source = [
            'static void Execute_Impl(WI005_ROOT_T *ptSelf) { (void)ptSelf; }',
            '',
            'static const WI005_API_T tWi005Api = {',
            '    .Execute = Execute_Impl,',
            '};',
            '',
        ].join('\n');

        fs.writeFileSync(path.join(dir, 'wi005.h'), header);
        fs.writeFileSync(srcPath, source);

        const indexer = new WorkspaceIndexer(dir, []);
        await indexer.fullIndex();

        // Precondition: vtableAssignments and functionDefinitions are populated
        expect(indexer.index.vtableAssignments.has('WI005_API_T')).toBe(true);
        const defsBefore = indexer.index.functionDefinitions.get('Execute_Impl');
        expect(defsBefore).toBeDefined();
        expect(defsBefore!.some((d) => d.file === srcPath)).toBe(true);

        // Remove the source file
        indexer.removeFile(srcPath);

        // vtableAssignments from the removed file must be gone
        expect(indexer.index.vtableAssignments.has('WI005_API_T')).toBe(false);

        // functionDefinitions from the removed file must be gone
        const defsAfter = indexer.index.functionDefinitions.get('Execute_Impl');
        const fromRemovedFile = (defsAfter ?? []).filter((d) => d.file === srcPath);
        expect(fromRemovedFile.length).toBe(0);
    });

    // =========================================================================
    // TC-WI-006: mergeInto() multi-file — no key collision
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-001", "REQ-VSCODE-021"]}
    it('TC-WI-006: fullIndex() with two independent modules stores both without cross-contamination', async () => {
        const dir = path.join(tempDir, 'tc006');
        fs.mkdirSync(dir);

        // Module A — completely independent types
        const srcA = [
            'typedef struct WI006A_API_TAG WI006A_API_T;',
            'typedef struct WI006A_ROOT_TAG WI006A_ROOT_T;',
            '',
            'struct WI006A_ROOT_TAG JUNO_MODULE_ROOT(WI006A_API_T, );',
            '',
            'struct WI006A_API_TAG',
            '{',
            '    void (*DoA)(WI006A_ROOT_T *ptSelf);',
            '};',
            '',
            'static void DoA_Impl(WI006A_ROOT_T *ptSelf) { (void)ptSelf; }',
            '',
            'static const WI006A_API_T tWi006aApi = {',
            '    .DoA = DoA_Impl,',
            '};',
            '',
        ].join('\n');

        // Module B — completely independent types
        const srcB = [
            'typedef struct WI006B_API_TAG WI006B_API_T;',
            'typedef struct WI006B_ROOT_TAG WI006B_ROOT_T;',
            '',
            'struct WI006B_ROOT_TAG JUNO_MODULE_ROOT(WI006B_API_T, );',
            '',
            'struct WI006B_API_TAG',
            '{',
            '    void (*DoB)(WI006B_ROOT_T *ptSelf);',
            '};',
            '',
            'static void DoB_Impl(WI006B_ROOT_T *ptSelf) { (void)ptSelf; }',
            '',
            'static const WI006B_API_T tWi006bApi = {',
            '    .DoB = DoB_Impl,',
            '};',
            '',
        ].join('\n');

        fs.writeFileSync(path.join(dir, 'wi006a.c'), srcA);
        fs.writeFileSync(path.join(dir, 'wi006b.c'), srcB);

        const indexer = new WorkspaceIndexer(dir, []);
        await indexer.fullIndex();

        // Both modules must appear in moduleRoots
        expect(indexer.index.moduleRoots.get('WI006A_ROOT_T')).toBe('WI006A_API_T');
        expect(indexer.index.moduleRoots.get('WI006B_ROOT_T')).toBe('WI006B_API_T');

        // Both must have independent vtableAssignments entries
        expect(indexer.index.vtableAssignments.has('WI006A_API_T')).toBe(true);
        expect(indexer.index.vtableAssignments.has('WI006B_API_T')).toBe(true);

        // No cross-contamination between A and B field maps
        const fieldMapA = indexer.index.vtableAssignments.get('WI006A_API_T')!;
        expect(fieldMapA.has('DoA')).toBe(true);
        expect(fieldMapA.has('DoB')).toBe(false);

        const fieldMapB = indexer.index.vtableAssignments.get('WI006B_API_T')!;
        expect(fieldMapB.has('DoB')).toBe(true);
        expect(fieldMapB.has('DoA')).toBe(false);
    });

    // =========================================================================
    // TC-WI-NEG-001: reindexFile() on never-indexed file → no error
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-WI-NEG-001: reindexFile() on a never-indexed file adds its records without throwing', async () => {
        const dir = path.join(tempDir, 'tcneg001');
        fs.mkdirSync(dir);

        // Indexer created before the file exists — file is unknown to the index
        const indexer = new WorkspaceIndexer(dir, []);

        const newFilePath = path.join(dir, 'wineg001.c');
        fs.writeFileSync(newFilePath, 'void FreshFunction(void) {}\n');

        // Must not throw
        await expect(indexer.reindexFile(newFilePath)).resolves.toBeUndefined();

        // FreshFunction must appear in functionDefinitions
        const defs = indexer.index.functionDefinitions.get('FreshFunction');
        expect(defs).toBeDefined();
        expect(defs!.length).toBeGreaterThanOrEqual(1);
        expect(defs![0].functionName).toBe('FreshFunction');
        expect(defs![0].file).toBe(newFilePath);
    });

    // =========================================================================
    // TC-WI-NEG-002: reindexFile() with nonexistent path → silent no-op
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-WI-NEG-002: reindexFile() with a nonexistent file path does not throw and leaves the index unchanged', async () => {
        const dir = path.join(tempDir, 'tcneg002');
        fs.mkdirSync(dir);

        const indexer = new WorkspaceIndexer(dir, []);

        // Index a real file first so there is a baseline to compare against
        const realFile = path.join(dir, 'real.c');
        fs.writeFileSync(realFile, 'void RealFunction(void) {}\n');
        await indexer.fullIndex();

        const beforeFuncSize = indexer.index.functionDefinitions.size;
        const beforeRootSize = indexer.index.moduleRoots.size;
        const beforeVtableSize = indexer.index.vtableAssignments.size;

        // Path that never existed — fs.promises.readFile throws ENOENT,
        // exercising the catch { return; } branch in indexFile() (line 163)
        const ghostPath = path.join(dir, 'does_not_exist.c');
        await expect(indexer.reindexFile(ghostPath)).resolves.toBeUndefined();

        // Index must be unchanged — no records added, none removed
        expect(indexer.index.functionDefinitions.size).toBe(beforeFuncSize);
        expect(indexer.index.moduleRoots.size).toBe(beforeRootSize);
        expect(indexer.index.vtableAssignments.size).toBe(beforeVtableSize);

        // The previously-indexed function must still be present
        expect(indexer.index.functionDefinitions.has('RealFunction')).toBe(true);
    });

    // =========================================================================
    // TC-WI-BND-001: fullIndex() on empty workspace → empty index
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-WI-BND-001: fullIndex() on an empty directory does not throw and produces an empty index', async () => {
        const dir = path.join(tempDir, 'tcbnd001');
        fs.mkdirSync(dir);
        // No C files in this directory

        const indexer = new WorkspaceIndexer(dir, []);

        await expect(indexer.fullIndex()).resolves.toBeUndefined();

        expect(indexer.index.moduleRoots.size).toBe(0);
        expect(indexer.index.vtableAssignments.size).toBe(0);
        expect(indexer.index.functionDefinitions.size).toBe(0);
    });

    // =========================================================================
    // Cache-Indexer Coordination (WI-8.2)
    // =========================================================================

    describe('Cache-Indexer Coordination', () => {

        let cacheDir: string;

        beforeAll(() => {
            cacheDir = fs.mkdtempSync(path.join(os.tmpdir(), 'libjuno-cache-'));
        });

        afterAll(() => {
            fs.rmSync(cacheDir, { recursive: true, force: true });
        });

        afterEach(() => {
            jest.restoreAllMocks();
        });

        // =====================================================================
        // TC-CACHE-003: Stale file triggers selective re-parse
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-001"]}
        it('TC-CACHE-003: loadFromCache() re-parses only the modified file on hash mismatch', async () => {
            const dir = path.join(cacheDir, 'tc-cache-003');
            fs.mkdirSync(dir);

            // Two independent files
            const fileA = path.join(dir, 'cache003a.h');
            const fileB = path.join(dir, 'cache003b.c');

            fs.writeFileSync(fileA, [
                'typedef struct C003_API_TAG C003_API_T;',
                'typedef struct C003_ROOT_TAG C003_ROOT_T;',
                'struct C003_ROOT_TAG JUNO_MODULE_ROOT(C003_API_T, );',
                'struct C003_API_TAG { void (*Process)(C003_ROOT_T *ptSelf); };',
                '',
            ].join('\n'));

            fs.writeFileSync(fileB, [
                'static void Process_V1(C003_ROOT_T *ptSelf) { (void)ptSelf; }',
                'static const C003_API_T tApi003 = { .Process = Process_V1 };',
                '',
            ].join('\n'));

            // Populate the cache via fullIndex()
            const indexer1 = new WorkspaceIndexer(dir, []);
            await indexer1.fullIndex();

            // Modify fileB — change the function name so hash changes
            fs.writeFileSync(fileB, [
                'static void Process_V2(C003_ROOT_T *ptSelf) { (void)ptSelf; }',
                'static const C003_API_T tApi003 = { .Process = Process_V2 };',
                '',
            ].join('\n'));

            // New indexer on the same directory
            const indexer2 = new WorkspaceIndexer(dir, []);
            const spy = jest.spyOn(visitorModule, 'parseFileWithDefs');

            const loaded = await indexer2.loadFromCache();

            // Cache must be found and used
            expect(loaded).toBe(true);

            // Only fileB changed — parseFileWithDefs must have been called exactly once
            expect(spy).toHaveBeenCalledTimes(1);
            expect(spy.mock.calls[0][0]).toBe(fileB);

            // Updated function from fileB must appear in the index
            expect(indexer2.index.functionDefinitions.has('Process_V2')).toBe(true);
            const defs = indexer2.index.functionDefinitions.get('Process_V2')!;
            expect(defs[0].functionName).toBe('Process_V2');
            expect(defs[0].file).toBe(fileB);

            // The vtable assignment for Process must now point to Process_V2
            const fieldMap = indexer2.index.vtableAssignments.get('C003_API_T');
            expect(fieldMap).toBeDefined();
            const locs = fieldMap!.get('Process');
            expect(locs).toBeDefined();
            expect(locs!.some((l) => l.functionName === 'Process_V2')).toBe(true);
        });

        // =====================================================================
        // TC-CACHE-004: New file indexed and added to cache
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-001"]}
        it('TC-CACHE-004: loadFromCache() parses a file that was added after the initial fullIndex()', async () => {
            const dir = path.join(cacheDir, 'tc-cache-004');
            fs.mkdirSync(dir);

            const existingFile = path.join(dir, 'cache004existing.c');
            fs.writeFileSync(existingFile, 'void Cache004Existing(void) {}\n');

            // Populate cache
            const indexer1 = new WorkspaceIndexer(dir, []);
            await indexer1.fullIndex();

            // Add a brand new file after the cache was written
            const newFile = path.join(dir, 'cache004new.c');
            fs.writeFileSync(newFile, 'void Cache004NewFunc(void) {}\n');

            const indexer2 = new WorkspaceIndexer(dir, []);
            const spy = jest.spyOn(visitorModule, 'parseFileWithDefs');

            const loaded = await indexer2.loadFromCache();

            // Cache found
            expect(loaded).toBe(true);

            // The new file has no cached hash → must be parsed
            expect(spy).toHaveBeenCalledTimes(1);
            expect(spy.mock.calls[0][0]).toBe(newFile);

            // The new file's function must appear in the index
            const defs = indexer2.index.functionDefinitions.get('Cache004NewFunc');
            expect(defs).toBeDefined();
            expect(defs!.length).toBeGreaterThanOrEqual(1);
            expect(defs![0].functionName).toBe('Cache004NewFunc');
            expect(defs![0].file).toBe(newFile);

            // The pre-existing function must also still be in the index (from cache)
            expect(indexer2.index.functionDefinitions.has('Cache004Existing')).toBe(true);
        });

        // =====================================================================
        // TC-CACHE-005: Deleted file — stale entries remain (known M4 behavior)
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-001"]}
        it('TC-CACHE-005: loadFromCache() returns true and retains stale index entries for a deleted file (known M4 behavior)', async () => {
            const dir = path.join(cacheDir, 'tc-cache-005');
            fs.mkdirSync(dir);

            // Two files — we will delete one after caching
            const fileToKeep = path.join(dir, 'cache005keep.c');
            const fileToDelete = path.join(dir, 'cache005delete.h');

            fs.writeFileSync(fileToKeep, 'void Cache005Kept(void) {}\n');

            fs.writeFileSync(fileToDelete, [
                'typedef struct C005_API_TAG C005_API_T;',
                'typedef struct C005_ROOT_TAG C005_ROOT_T;',
                'struct C005_ROOT_TAG JUNO_MODULE_ROOT(C005_API_T, );',
                '',
            ].join('\n'));

            // Populate cache — both files are indexed
            const indexer1 = new WorkspaceIndexer(dir, []);
            await indexer1.fullIndex();

            // Verify precondition: deleted file's module root is in the index
            expect(indexer1.index.moduleRoots.has('C005_ROOT_T')).toBe(true);

            // Delete the file
            fs.unlinkSync(fileToDelete);

            // New indexer — uses the cache but the deleted file no longer exists on disk
            const indexer2 = new WorkspaceIndexer(dir, []);
            const loaded = await indexer2.loadFromCache();

            // Cache is found and loaded
            expect(loaded).toBe(true);

            // Known M4 behavior: loadFromCache() loads ALL records from the cache via
            // copyIndex(), then only re-indexes files found by scanFiles(). Deleted files
            // are absent from scanFiles(), so their cached records are never cleaned up.
            // The stale moduleRoot entry from the deleted file persists in the index.
            expect(indexer2.index.moduleRoots.has('C005_ROOT_T')).toBe(true);
        });

    }); // end 'Cache-Indexer Coordination'

    // =========================================================================
    // File Discovery and Cross-File Resolution (WI-8.3)
    // =========================================================================

    describe('File Discovery and Cross-File Resolution', () => {

        let fileDir: string;

        beforeAll(() => {
            fileDir = fs.mkdtempSync(path.join(os.tmpdir(), 'libjuno-fileext-'));
        });

        afterAll(() => {
            fs.rmSync(fileDir, { recursive: true, force: true });
        });

        afterEach(() => {
            jest.restoreAllMocks();
        });

        // =====================================================================
        // TC-FILE-001: All 6 C/C++ file extensions are discovered
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-021"]}
        it('TC-FILE-001: fullIndex() discovers and indexes files with .c, .h, .cpp, .hpp, .hh, and .cc extensions', async () => {
            const dir = path.join(fileDir, 'tc-file-001');
            fs.mkdirSync(dir);

            // One unique function per extension so each file's parse is verifiable
            const files: Array<[string, string]> = [
                ['widget.c',   'void File001_C(void) {}\n'],
                ['widget.h',   'void File001_H(void) {}\n'],
                ['widget.cpp', 'void File001_CPP(void) {}\n'],
                ['widget.hpp', 'void File001_HPP(void) {}\n'],
                ['widget.hh',  'void File001_HH(void) {}\n'],
                ['widget.cc',  'void File001_CC(void) {}\n'],
            ];

            for (const [name, content] of files) {
                fs.writeFileSync(path.join(dir, name), content);
            }

            const indexer = new WorkspaceIndexer(dir, []);
            await indexer.fullIndex();

            // Every extension's unique function must appear in functionDefinitions
            const expected = ['File001_C', 'File001_H', 'File001_CPP', 'File001_HPP', 'File001_HH', 'File001_CC'];
            for (const fnName of expected) {
                const defs = indexer.index.functionDefinitions.get(fnName);
                expect(defs).toBeDefined();
                expect(defs!.length).toBeGreaterThanOrEqual(1);
                expect(defs![0].functionName).toBe(fnName);
            }
        });

        // =====================================================================
        // TC-WI-007: Positional initializer resolved when API struct is in same file
        //
        // This test verifies the same-file positional resolution path.
        // Cross-file positional resolution (Bug 1) and resolveDeferred()
        // duplicate-check (Bug 3) were fixed in Sprint 9 — see TC-WI-007b.
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-012"]}
        it('TC-WI-007: fullIndex() resolves positional vtable initializer when the API struct is defined in the same file', async () => {
            const dir = path.join(fileDir, 'tc-wi-007');
            fs.mkdirSync(dir);

            // Single file: API struct definition + function implementations + positional init
            const src = [
                'typedef struct POS007_API_TAG POS007_API_T;',
                'typedef struct POS007_ROOT_TAG POS007_ROOT_T;',
                '',
                'struct POS007_ROOT_TAG JUNO_MODULE_ROOT(POS007_API_T, );',
                '',
                'struct POS007_API_TAG',
                '{',
                '    void (*DoA)(POS007_ROOT_T *ptSelf);',
                '    void (*DoB)(POS007_ROOT_T *ptSelf);',
                '};',
                '',
                'static void DoAImpl(POS007_ROOT_T *ptSelf) { (void)ptSelf; }',
                'static void DoBImpl(POS007_ROOT_T *ptSelf) { (void)ptSelf; }',
                '',
                '/* Positional initializer — no designators; fields resolved by position */',
                'static const POS007_API_T tPosApi = { DoAImpl, DoBImpl };',
                '',
            ].join('\n');

            fs.writeFileSync(path.join(dir, 'pos007.c'), src);

            const indexer = new WorkspaceIndexer(dir, []);
            await indexer.fullIndex();

            // apiStructFields must have POS007_API_T with ordered fields [DoA, DoB]
            const fields = indexer.index.apiStructFields.get('POS007_API_T');
            expect(fields).toBeDefined();
            expect(fields).toEqual(['DoA', 'DoB']);

            // vtableAssignments must map positional order to correct function names
            const fieldMap = indexer.index.vtableAssignments.get('POS007_API_T');
            expect(fieldMap).toBeDefined();

            const doALocs = fieldMap!.get('DoA');
            expect(doALocs).toBeDefined();
            expect(doALocs!.length).toBeGreaterThanOrEqual(1);
            expect(doALocs![0].functionName).toBe('DoAImpl');

            const doBLocs = fieldMap!.get('DoB');
            expect(doBLocs).toBeDefined();
            expect(doBLocs!.length).toBeGreaterThanOrEqual(1);
            expect(doBLocs![0].functionName).toBe('DoBImpl');
        });

        // =====================================================================
        // TC-WI-008: Failure handler root-type resolution via localTypeInfo
        //
        // This test verifies the single-module case (no ambiguity).
        // Multi-module disambiguation (Bug 2) was fixed in Sprint 9 — see TC-WI-008b.
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-016"]}
        it('TC-WI-008: resolveFailureHandlerRootType() attributes the failure handler to the correct root type via function parameter type', async () => {
            const dir = path.join(fileDir, 'tc-wi-008');
            fs.mkdirSync(dir);

            // Single module: one root type, one Init function, one handler
            const src = [
                'typedef struct WI008_API_TAG WI008_API_T;',
                'typedef struct WI008_ROOT_TAG WI008_ROOT_T;',
                '',
                'struct WI008_ROOT_TAG JUNO_MODULE_ROOT(WI008_API_T, );',
                '',
                'struct WI008_API_TAG',
                '{',
                '    void (*Work)(WI008_ROOT_T *ptSelf);',
                '};',
                '',
                'static void WI008_FailureHandler(void) {}',
                '',
                'void WI008_Init(WI008_ROOT_T *ptSelf)',
                '{',
                '    ptSelf->_pfcnFailureHandler = WI008_FailureHandler;',
                '}',
                '',
            ].join('\n');

            fs.writeFileSync(path.join(dir, 'wi008.c'), src);

            const indexer = new WorkspaceIndexer(dir, []);
            await indexer.fullIndex();

            // moduleRoots must have the root type
            expect(indexer.index.moduleRoots.get('WI008_ROOT_T')).toBe('WI008_API_T');

            // failureHandlerAssignments must map WI008_ROOT_T → WI008_FailureHandler
            // resolveFailureHandlerRootType() uses functionParameters of WI008_Init
            // (which declares 'ptSelf: WI008_ROOT_T *') to identify the root type.
            const handlers = indexer.index.failureHandlerAssignments.get('WI008_ROOT_T');
            expect(handlers).toBeDefined();
            expect(handlers!.length).toBeGreaterThanOrEqual(1);
            expect(handlers![0].functionName).toBe('WI008_FailureHandler');

            // Ensure the handler function's definition line is resolved
            expect(handlers![0].file).toContain('wi008.c');
        });

        // =====================================================================
        // TC-WI-007b: Cross-file positional vtable initializer resolution
        //   API struct in crosspos.h; positional init in crosspos.c.
        //   Verifies Bug 1 fix (deferred positional producer) and
        //   Bug 3 fix (resolveDeferred() duplicate-check uses loc.line).
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-012"]}
        it('TC-WI-007b: fullIndex() resolves cross-file positional vtable initializer via deferred mechanism', async () => {
            const dir = path.join(fileDir, 'tc-wi-007b');
            fs.mkdirSync(dir);

            // Header: API struct definition only (no function implementations)
            const header = [
                'typedef struct XPOS_API_TAG XPOS_API_T;',
                'typedef struct XPOS_ROOT_TAG XPOS_ROOT_T;',
                '',
                'struct XPOS_ROOT_TAG JUNO_MODULE_ROOT(XPOS_API_T, );',
                '',
                'struct XPOS_API_TAG',
                '{',
                '    void (*Alpha)(XPOS_ROOT_T *ptSelf);',
                '    void (*Beta)(XPOS_ROOT_T *ptSelf);',
                '};',
                '',
            ].join('\n');

            // Source: function implementations + positional initializer (API struct NOT here)
            const source = [
                'static void AlphaImpl(XPOS_ROOT_T *ptSelf) { (void)ptSelf; }',
                'static void BetaImpl(XPOS_ROOT_T *ptSelf) { (void)ptSelf; }',
                '',
                'static const XPOS_API_T tXposApi = { AlphaImpl, BetaImpl };',
                '',
            ].join('\n');

            fs.writeFileSync(path.join(dir, 'crosspos.h'), header);
            fs.writeFileSync(path.join(dir, 'crosspos.c'), source);

            const indexer = new WorkspaceIndexer(dir, []);
            await indexer.fullIndex();

            // apiStructFields must have the field order from the header
            const fields = indexer.index.apiStructFields.get('XPOS_API_T');
            expect(fields).toBeDefined();
            expect(fields).toEqual(['Alpha', 'Beta']);

            // vtableAssignments must resolve the cross-file positional initializer
            const fieldMap = indexer.index.vtableAssignments.get('XPOS_API_T');
            expect(fieldMap).toBeDefined();

            const alphaLocs = fieldMap!.get('Alpha');
            expect(alphaLocs).toBeDefined();
            expect(alphaLocs!.length).toBe(1); // exactly 1 — no duplicates (Bug 3 fix)
            expect(alphaLocs![0].functionName).toBe('AlphaImpl');

            const betaLocs = fieldMap!.get('Beta');
            expect(betaLocs).toBeDefined();
            expect(betaLocs!.length).toBe(1); // exactly 1 — no duplicates (Bug 3 fix)
            expect(betaLocs![0].functionName).toBe('BetaImpl');
        });

        // =====================================================================
        // TC-WI-008b: Multi-module FH disambiguation in same file
        //   Two modules (MM_A, MM_B) defined in one file, each with its own
        //   Init function and FH assignment.
        //   Verifies Bug 2 fix (scoped containingFn search in
        //   resolveFailureHandlerRootType()).
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-016"]}
        it('TC-WI-008b: resolveFailureHandlerRootType() assigns each FH to its own root type when two modules share a file', async () => {
            const dir = path.join(fileDir, 'tc-wi-008b');
            fs.mkdirSync(dir);

            // Two modules in one file — each has its own Init function and FH assignment
            const src = [
                'typedef struct MM_A_API_TAG MM_A_API_T;',
                'typedef struct MM_A_ROOT_TAG MM_A_ROOT_T;',
                'typedef struct MM_B_API_TAG MM_B_API_T;',
                'typedef struct MM_B_ROOT_TAG MM_B_ROOT_T;',
                '',
                'struct MM_A_ROOT_TAG JUNO_MODULE_ROOT(MM_A_API_T, );',
                'struct MM_B_ROOT_TAG JUNO_MODULE_ROOT(MM_B_API_T, );',
                '',
                'struct MM_A_API_TAG { void (*WorkA)(MM_A_ROOT_T *ptSelf); };',
                'struct MM_B_API_TAG { void (*WorkB)(MM_B_ROOT_T *ptSelf); };',
                '',
                'static void FH_A(void) {}',
                'static void FH_B(void) {}',
                '',
                'void InitModuleA(MM_A_ROOT_T *ptSelf)',
                '{',
                '    ptSelf->_pfcnFailureHandler = FH_A;',
                '}',
                '',
                'void InitModuleB(MM_B_ROOT_T *ptSelf)',
                '{',
                '    ptSelf->_pfcnFailureHandler = FH_B;',
                '}',
                '',
            ].join('\n');

            fs.writeFileSync(path.join(dir, 'multimod.c'), src);

            const indexer = new WorkspaceIndexer(dir, []);
            await indexer.fullIndex();

            // Both module roots must be present
            expect(indexer.index.moduleRoots.get('MM_A_ROOT_T')).toBe('MM_A_API_T');
            expect(indexer.index.moduleRoots.get('MM_B_ROOT_T')).toBe('MM_B_API_T');

            // MM_A_ROOT_T must only have FH_A
            const handlersA = indexer.index.failureHandlerAssignments.get('MM_A_ROOT_T');
            expect(handlersA).toBeDefined();
            expect(handlersA!.some((h) => h.functionName === 'FH_A')).toBe(true);
            expect(handlersA!.some((h) => h.functionName === 'FH_B')).toBe(false);

            // MM_B_ROOT_T must only have FH_B
            const handlersB = indexer.index.failureHandlerAssignments.get('MM_B_ROOT_T');
            expect(handlersB).toBeDefined();
            expect(handlersB!.some((h) => h.functionName === 'FH_B')).toBe(true);
            expect(handlersB!.some((h) => h.functionName === 'FH_A')).toBe(false);
        });

        // =====================================================================
        // TC-WI-014-LOC: resolveRootTypeForFH containingFn found, root type in
        //   local variables (lines 319-322)
        //   The containing function has void params → params array is empty →
        //   falls through to localVariables check which finds the root type.
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-016"]}
        it('TC-WI-014-LOC: resolveRootTypeForFH() finds root type via localVariables when containing function has no root-type parameters', async () => {
            const dir = path.join(fileDir, 'tc-wi-014-loc');
            fs.mkdirSync(dir);

            // Single file: module root + void-param function with LOCAL variable of
            // root type.  resolveRootTypeForFH must exercise:
            //   line 314 – functionParameters.get("InitLocal") → []
            //   line 315 – if (params) → true, but loop body never entered (empty)
            //   line 319 – localVariables.get("InitLocal") → Map with tRoot
            //   line 320 – if (vars) → true
            //   line 322 – knownRoots.has("WI014L_ROOT_T") → return
            const src = [
                'typedef struct WI014L_API_TAG WI014L_API_T;',
                'typedef struct WI014L_ROOT_TAG WI014L_ROOT_T;',
                '',
                'struct WI014L_ROOT_TAG JUNO_MODULE_ROOT(WI014L_API_T, );',
                '',
                'struct WI014L_API_TAG { void (*Work)(WI014L_ROOT_T *ptSelf); };',
                '',
                'static void FH_WI014L(void) {}',
                '',
                'void InitLocal(void) {',
                '    WI014L_ROOT_T tRoot;',
                '    tRoot._pfcnFailureHandler = FH_WI014L;',
                '}',
                '',
            ].join('\n');

            fs.writeFileSync(path.join(dir, 'wi014l.c'), src);

            const indexer = new WorkspaceIndexer(dir, []);
            await indexer.fullIndex();

            // moduleRoots must have the root type
            expect(indexer.index.moduleRoots.get('WI014L_ROOT_T')).toBe('WI014L_API_T');

            // resolveRootTypeForFH must attribute the handler to WI014L_ROOT_T via
            // the local variable 'tRoot' of type WI014L_ROOT_T inside InitLocal()
            const handlers = indexer.index.failureHandlerAssignments.get('WI014L_ROOT_T');
            expect(handlers).toBeDefined();
            expect(handlers!.length).toBeGreaterThanOrEqual(1);
            expect(handlers!.some((h) => h.functionName === 'FH_WI014L')).toBe(true);
        });

        // =====================================================================
        // TC-WI-014-FBP: resolveRootTypeForFH fallback path — search all
        //   functionParameters (lines 325-327)
        //   findContainingFunction returns undefined when functionDefs is empty,
        //   so the fallback iterates all functionParameters entries and finds the
        //   root type there.
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-016"]}
        it('TC-WI-014-FBP: resolveRootTypeForFH() searches all functionParameters in fallback when no containing function is found', async () => {
            const dir = path.join(fileDir, 'tc-wi-014-fbp');
            fs.mkdirSync(dir);

            // Index the header first to populate moduleRoots with FALLP_ROOT_T
            const header = [
                'typedef struct FALLP_API_TAG FALLP_API_T;',
                'typedef struct FALLP_ROOT_TAG FALLP_ROOT_T;',
                '',
                'struct FALLP_ROOT_TAG JUNO_MODULE_ROOT(FALLP_API_T, );',
                '',
                'struct FALLP_API_TAG { void (*Work)(FALLP_ROOT_T *ptSelf); };',
                '',
            ].join('\n');

            const implPath = path.join(dir, 'fallp_impl.c');
            fs.writeFileSync(path.join(dir, 'fallp.h'), header);
            // Placeholder impl file — content is replaced by the mock below
            fs.writeFileSync(implPath, '/* placeholder */\n');

            const indexer = new WorkspaceIndexer(dir, []);
            // Index header only to seed moduleRoots; impl file will use the mock
            await indexer.reindexFile(path.join(dir, 'fallp.h'));
            expect(indexer.index.moduleRoots.get('FALLP_ROOT_T')).toBe('FALLP_API_T');

            // Craft a ParsedFile with a FH assignment and functionDefs=[] so that
            // findContainingFunction(10, implPath, []) returns undefined → fallback.
            // functionParameters has FALLP_ROOT_T → fallback finds root via params.
            const mockParsed: ParsedFile = {
                filePath: implPath,
                moduleRoots: [],
                traitRoots: [],
                derivations: [],
                apiStructDefinitions: [],
                vtableAssignments: [],
                failureHandlerAssigns: [{
                    rootType: '',
                    functionName: 'FH_FallParams',
                    file: implPath,
                    line: 10,
                }],
                apiCallSites: [],
                pendingPositionalVtables: [],
                initCallSites: [],
                localTypeInfo: {
                    functionParameters: new Map([
                        ['AnyFunc', [{
                            name: 'ptRoot', typeName: 'FALLP_ROOT_T',
                            isPointer: true, isConst: false, isArray: false,
                        }]],
                    ]),
                    localVariables: new Map(),
                },
            };

            jest.spyOn(visitorModule, 'parseFileWithDefs').mockReturnValueOnce({
                parsed: mockParsed,
                functionDefs: [],        // empty → findContainingFunction → undefined → fallback
                apiMemberRegistry: new Map(),
            });

            await indexer.reindexFile(implPath);

            // Fallback params loop (lines 325-327) found FALLP_ROOT_T
            const handlers = indexer.index.failureHandlerAssignments.get('FALLP_ROOT_T');
            expect(handlers).toBeDefined();
            expect(handlers!.length).toBeGreaterThanOrEqual(1);
            expect(handlers!.some((h) => h.functionName === 'FH_FallParams')).toBe(true);
        });

        // =====================================================================
        // TC-WI-014-FBV: resolveRootTypeForFH fallback path — search all
        //   localVariables (lines 329-333)
        //   functionParameters has only a non-root type, so the fallback falls
        //   through to the localVariables loop which finds the root type.
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-016"]}
        it('TC-WI-014-FBV: resolveRootTypeForFH() searches all localVariables in fallback when functionParameters yields no root-type match', async () => {
            const dir = path.join(fileDir, 'tc-wi-014-fbv');
            fs.mkdirSync(dir);

            // Index the header first to populate moduleRoots with FALLV_ROOT_T
            const header = [
                'typedef struct FALLV_API_TAG FALLV_API_T;',
                'typedef struct FALLV_ROOT_TAG FALLV_ROOT_T;',
                '',
                'struct FALLV_ROOT_TAG JUNO_MODULE_ROOT(FALLV_API_T, );',
                '',
                'struct FALLV_API_TAG { void (*Work)(FALLV_ROOT_T *ptSelf); };',
                '',
            ].join('\n');

            const implPath = path.join(dir, 'fallv_impl.c');
            fs.writeFileSync(path.join(dir, 'fallv.h'), header);
            fs.writeFileSync(implPath, '/* placeholder */\n');

            const indexer = new WorkspaceIndexer(dir, []);
            await indexer.reindexFile(path.join(dir, 'fallv.h'));
            expect(indexer.index.moduleRoots.get('FALLV_ROOT_T')).toBe('FALLV_API_T');

            // functionParameters has only a non-root 'int' param → fallback params
            // loop finds no match.  localVariables has FALLV_ROOT_T → fallback vars
            // loop (lines 329-333) finds it.
            const mockParsed: ParsedFile = {
                filePath: implPath,
                moduleRoots: [],
                traitRoots: [],
                derivations: [],
                apiStructDefinitions: [],
                vtableAssignments: [],
                failureHandlerAssigns: [{
                    rootType: '',
                    functionName: 'FH_FallVars',
                    file: implPath,
                    line: 10,
                }],
                apiCallSites: [],
                pendingPositionalVtables: [],
                initCallSites: [],
                localTypeInfo: {
                    functionParameters: new Map([
                        ['AnyFunc', [{
                            name: 'x', typeName: 'int',
                            isPointer: false, isConst: false, isArray: false,
                        }]],
                    ]),
                    localVariables: new Map([
                        ['AnyFunc', new Map([
                            ['myVar', {
                                name: 'myVar', typeName: 'FALLV_ROOT_T',
                                isPointer: false, isConst: false, isArray: false,
                            }],
                        ])],
                    ]),
                },
            };

            jest.spyOn(visitorModule, 'parseFileWithDefs').mockReturnValueOnce({
                parsed: mockParsed,
                functionDefs: [],        // empty → findContainingFunction → undefined → fallback
                apiMemberRegistry: new Map(),
            });

            await indexer.reindexFile(implPath);

            // Fallback vars loop (lines 329-333) found FALLV_ROOT_T
            const handlers = indexer.index.failureHandlerAssignments.get('FALLV_ROOT_T');
            expect(handlers).toBeDefined();
            expect(handlers!.length).toBeGreaterThanOrEqual(1);
            expect(handlers!.some((h) => h.functionName === 'FH_FallVars')).toBe(true);
        });

        // =====================================================================
        // TC-WI-C6-A: resolveDeferred() — API struct not found → warn + continue
        //   (workspaceIndexer.ts lines 370-371)
        //
        //   A positional vtable initializer for a type whose struct is NEVER
        //   defined anywhere produces a DeferredPositional entry. After all
        //   files are indexed, resolveDeferred() cannot find the type in
        //   apiStructFields → issues a console.warn and continues (skips).
        //
        //   Trigger: type name ends in _API_T (visitor line 474)
        //            and has no struct definition in any indexed file.
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-012"]}
        it('TC-WI-C6-A: resolveDeferred() warns and skips when API struct definition is absent from all indexed files', async () => {
            const dir = path.join(fileDir, 'tc-wi-c6-a');
            fs.mkdirSync(dir);

            // C file: positional vtable init for UNRSLV_API_T, which has NO struct
            // definition anywhere in the workspace.  The visitor creates a
            // PendingPositionalVtable; resolveDeferred() cannot look it up in
            // apiStructFields (undefined) → warn + continue (lines 369-371).
            const src = [
                'static void UnrslvFuncA(void) {}',
                'static void UnrslvFuncB(void) {}',
                '',
                'static const UNRSLV_API_T tUnrslv = { UnrslvFuncA, UnrslvFuncB };',
                '',
            ].join('\n');

            fs.writeFileSync(path.join(dir, 'unrslv.c'), src);

            const warnSpy = jest.spyOn(console, 'warn');
            const indexer = new WorkspaceIndexer(dir, []);
            await indexer.fullIndex();

            // The warning must have been emitted with the unknown API type name
            const warnCalls = warnSpy.mock.calls.map((args) => args.join(' '));
            expect(warnCalls.some((msg) => msg.includes('UNRSLV_API_T'))).toBe(true);

            // vtableAssignments must NOT have an entry for the unresolvable type
            expect(indexer.index.vtableAssignments.has('UNRSLV_API_T')).toBe(false);

            // apiStructFields must NOT have an entry (no struct def was ever parsed)
            expect(indexer.index.apiStructFields.has('UNRSLV_API_T')).toBe(false);
        });

        // =====================================================================
        // TC-WI-C6-B: scanFiles() — excluded directory is skipped (line 415)
        //
        //   WorkspaceIndexer constructed with excludedDirs=['build'] must not
        //   index any file inside a directory named 'build'.  Files at the
        //   workspace root level are still indexed normally.
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-021"]}
        it('TC-WI-C6-B: fullIndex() does not index C files inside an excluded directory but recurses into non-excluded subdirs', async () => {
            const dir = path.join(fileDir, 'tc-wi-c6-b');
            const excludedSubdir = path.join(dir, 'build');
            const includedSubdir = path.join(dir, 'src');
            fs.mkdirSync(dir);
            fs.mkdirSync(excludedSubdir);
            fs.mkdirSync(includedSubdir);

            // File in the excluded 'build' directory — must NOT be indexed
            fs.writeFileSync(
                path.join(excludedSubdir, 'excluded.c'),
                'void ExcludedC6Func(void) {}\n',
            );

            // File inside a non-excluded subdirectory 'src/' — MUST be indexed.
            // This triggers the recursive scanFiles() call at line 415.
            fs.writeFileSync(
                path.join(includedSubdir, 'subdir.c'),
                'void SubdirC6Func(void) {}\n',
            );

            // File at the workspace root — must still be indexed
            fs.writeFileSync(
                path.join(dir, 'included.c'),
                'void IncludedC6Func(void) {}\n',
            );

            // Construct with 'build' as an excluded directory
            const indexer = new WorkspaceIndexer(dir, ['build']);
            await indexer.fullIndex();

            // Root-level file is indexed
            expect(indexer.index.functionDefinitions.has('IncludedC6Func')).toBe(true);
            const incDefs = indexer.index.functionDefinitions.get('IncludedC6Func')!;
            expect(incDefs[0].functionName).toBe('IncludedC6Func');

            // File inside non-excluded 'src/' is indexed (recursive scan, line 415)
            expect(indexer.index.functionDefinitions.has('SubdirC6Func')).toBe(true);
            const subDefs = indexer.index.functionDefinitions.get('SubdirC6Func')!;
            expect(subDefs[0].functionName).toBe('SubdirC6Func');

            // File inside 'build/' is NOT indexed (excluded dir, line 412 → continue)
            expect(indexer.index.functionDefinitions.has('ExcludedC6Func')).toBe(false);
        });

        // =====================================================================
        // TC-WI-C6-C: hashFile() read error → returns "" (line 436)
        //
        //   loadFromCache() calls hashFile() for every file found by scanFiles().
        //   When readFileSync throws (e.g., EACCES), the catch block returns "".
        //   The empty string does NOT match the cached (non-empty) hash, so the
        //   file is treated as stale: removeFileRecords() runs, then indexFile()
        //   also fails silently (readFile throws).  The net result: the file's
        //   index entries are removed and not re-added.
        //
        //   Skipped when running as root (chmod has no effect under root).
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-001"]}
        it('TC-WI-C6-C: hashFile() returns "" on read error causing loadFromCache() to treat the file as stale', async () => {
            // Skip if process is root — chmod restrictions do not apply to root
            if (typeof process.getuid === 'function' && process.getuid() === 0) {
                return;
            }

            const dir = path.join(fileDir, 'tc-wi-c6-c');
            fs.mkdirSync(dir);

            const filePath = path.join(dir, 'hashfail.c');
            fs.writeFileSync(filePath, 'void HashFailFunc(void) {}\n');

            // Populate cache via fullIndex()
            const indexer1 = new WorkspaceIndexer(dir, []);
            await indexer1.fullIndex();

            // Precondition: HashFailFunc was indexed and cached
            expect(indexer1.index.functionDefinitions.has('HashFailFunc')).toBe(true);

            // Make the file unreadable — hashFile() will throw and return ""
            fs.chmodSync(filePath, 0o000);

            try {
                const indexer2 = new WorkspaceIndexer(dir, []);
                const spy = jest.spyOn(visitorModule, 'parseFileWithDefs');

                const loaded = await indexer2.loadFromCache();

                // Cache must be found
                expect(loaded).toBe(true);

                // hashFile() returned "" (read error) → hash mismatch → stale path:
                //   removeFileRecords() removed HashFailFunc from index
                //   indexFile() also failed to read → returned early (no re-parse)
                expect(spy).not.toHaveBeenCalled();

                // HashFailFunc must be absent: removed by removeFileRecords, not re-added
                expect(indexer2.index.functionDefinitions.has('HashFailFunc')).toBe(false);
            } finally {
                // Restore permissions so afterAll cleanup can delete the directory
                fs.chmodSync(filePath, 0o644);
            }
        });

        // =====================================================================
        // TC-WI-015: Cross-file designated vtable assignment resolves
        //   ConcreteLocation.file to the function DEFINITION file, not the
        //   vtable assignment file.
        //
        //   Bug: resolveDefinitionLine() (old) returned only a line number;
        //   ConcreteLocation.file was always r.file (assignment file).
        //   Fix: resolveDefinitionLocation() returns {file, line}; callers use
        //   defLoc.file when defLoc.line !== 0.
        //
        //   'a_def.c' sorts before 'b_vtable.c' → indexed first, so
        //   CrossFile15_DoThing is in this.index.functionDefinitions when the
        //   assignment file is processed (resolveDefinitionLocation step 2).
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-005"]}
        it('TC-WI-015: fullIndex() sets ConcreteLocation.file to the function definition file for a cross-file designated vtable assignment', async () => {
            const dir = path.join(fileDir, 'tc-wi-015');
            fs.mkdirSync(dir);

            // Definition file (indexed first): API struct + function definition
            const fileDefPath = path.join(dir, 'a_def.c');
            fs.writeFileSync(fileDefPath, [
                'typedef struct CROSS15_API_TAG CROSS15_API_T;',
                '',
                'struct CROSS15_API_TAG',
                '{',
                '    void (*DoThing)(void);',
                '};',
                '',
                'void CrossFile15_DoThing(void) {}',
                '',
            ].join('\n'));

            // Assignment file (indexed second): designated vtable init only
            const fileVtablePath = path.join(dir, 'b_vtable.c');
            fs.writeFileSync(fileVtablePath, [
                'static const CROSS15_API_T tApi15 = {',
                '    .DoThing = CrossFile15_DoThing,',
                '};',
                '',
            ].join('\n'));

            const indexer = new WorkspaceIndexer(dir, []);
            await indexer.fullIndex();

            const fieldMap = indexer.index.vtableAssignments.get('CROSS15_API_T');
            expect(fieldMap).toBeDefined();
            const locs = fieldMap!.get('DoThing');
            expect(locs).toBeDefined();
            expect(locs!).toHaveLength(1);

            // ConcreteLocation.file must be the DEFINITION file (a_def.c), not the
            // assignment file (b_vtable.c). Before the fix, file was always r.file.
            expect(locs![0].file).toBe(fileDefPath);
            expect(locs![0].line).toBeGreaterThan(0);
            expect(locs![0].functionName).toBe('CrossFile15_DoThing');
        });

        // =====================================================================
        // TC-WI-016: Same-file designated vtable — no regression after fix
        //   resolveDefinitionLocation() step 1 (same-file match) fires;
        //   ConcreteLocation.file must be the single source file and
        //   ConcreteLocation.line must be the definition line (before the vtable).
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-005"]}
        it('TC-WI-016: fullIndex() correctly resolves ConcreteLocation.file/line for a same-file designated vtable assignment (no regression)', async () => {
            const dir = path.join(fileDir, 'tc-wi-016');
            fs.mkdirSync(dir);

            const filePath = path.join(dir, 'samefi16.c');
            fs.writeFileSync(filePath, [
                'typedef struct SAME16_API_TAG SAME16_API_T;', // line 1
                '',                                             // line 2
                'struct SAME16_API_TAG',                       // line 3
                '{',                                            // line 4
                '    void (*Process)(void);',                  // line 5
                '};',                                           // line 6
                '',                                             // line 7
                'void SameFile16_Process(void) {}',            // line 8 — definition
                '',                                             // line 9
                'static const SAME16_API_T tApi16 = {',       // line 10 — vtable
                '    .Process = SameFile16_Process,',          // line 11
                '};',                                           // line 12
                '',
            ].join('\n'));

            const indexer = new WorkspaceIndexer(dir, []);
            await indexer.fullIndex();

            const fieldMap = indexer.index.vtableAssignments.get('SAME16_API_T');
            expect(fieldMap).toBeDefined();
            const locs = fieldMap!.get('Process');
            expect(locs).toBeDefined();
            expect(locs!).toHaveLength(1);

            // ConcreteLocation.file must be the same source file
            expect(locs![0].file).toBe(filePath);
            // ConcreteLocation.line must be the function definition line (line 8),
            // which precedes the vtable assignment (line 10) — proves definition
            // line is used, not the vtable assignment line
            expect(locs![0].line).toBeGreaterThan(0);
            expect(locs![0].line).toBeLessThan(10);
            expect(locs![0].functionName).toBe('SameFile16_Process');
        });

        // =====================================================================
        // TC-WI-017: Cross-file positional vtable (deferred path) resolves
        //   ConcreteLocation.file to the function definition file.
        //   resolveDeferred() calls resolveDefinitionLocation(fnName, d.filePath, [])
        //   with empty functionDefs → relies on this.index.functionDefinitions.
        //   'a_defs17.c' sorts before 'b_init17.c' → struct + defs indexed first.
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-005"]}
        it('TC-WI-017: fullIndex() sets ConcreteLocation.file to the definition file for cross-file positional vtable (deferred resolution path)', async () => {
            const dir = path.join(fileDir, 'tc-wi-017');
            fs.mkdirSync(dir);

            // Definition file (indexed first): API struct definition + function definitions
            const fileDefsPath = path.join(dir, 'a_defs17.c');
            fs.writeFileSync(fileDefsPath, [
                'typedef struct DEFER17_API_TAG DEFER17_API_T;',
                '',
                'struct DEFER17_API_TAG',
                '{',
                '    void (*Op1)(void);',
                '    void (*Op2)(void);',
                '};',
                '',
                'void Defer17_Impl_Op1(void) {}',
                'void Defer17_Impl_Op2(void) {}',
                '',
            ].join('\n'));

            // Initializer file (indexed second): positional vtable only
            const fileInitPath = path.join(dir, 'b_init17.c');
            fs.writeFileSync(fileInitPath, [
                'static const DEFER17_API_T tDefer17 = { Defer17_Impl_Op1, Defer17_Impl_Op2 };',
                '',
            ].join('\n'));

            const indexer = new WorkspaceIndexer(dir, []);
            await indexer.fullIndex();

            const fields = indexer.index.apiStructFields.get('DEFER17_API_T');
            expect(fields).toEqual(['Op1', 'Op2']);

            const fieldMap = indexer.index.vtableAssignments.get('DEFER17_API_T');
            expect(fieldMap).toBeDefined();

            const locs1 = fieldMap!.get('Op1');
            expect(locs1).toBeDefined();
            expect(locs1!).toHaveLength(1);
            // ConcreteLocation.file must be the DEFINITION file (a_defs17.c), not
            // the positional initializer file (b_init17.c). resolveDeferred()
            // uses defLoc.file when defLoc.line !== 0.
            expect(locs1![0].file).toBe(fileDefsPath);
            expect(locs1![0].line).toBeGreaterThan(0);
            expect(locs1![0].functionName).toBe('Defer17_Impl_Op1');

            const locs2 = fieldMap!.get('Op2');
            expect(locs2).toBeDefined();
            expect(locs2!).toHaveLength(1);
            expect(locs2![0].file).toBe(fileDefsPath);
            expect(locs2![0].functionName).toBe('Defer17_Impl_Op2');
        });

        // =====================================================================
        // TC-WI-018: Fallback to vtable assignment location when function
        //   definition is not found in any indexed file.
        //   A forward declaration produces no FunctionDefinitionRecord; 
        //   resolveDefinitionLocation() returns { file, line: 0 }.
        //   mergeInto() must fall back to r.file / r.line (not leave line as 0).
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-005"]}
        it('TC-WI-018: fullIndex() falls back to vtable assignment file/line when the function definition is not found in any indexed file', async () => {
            const dir = path.join(fileDir, 'tc-wi-018');
            fs.mkdirSync(dir);

            const filePath = path.join(dir, 'fallback18.c');
            fs.writeFileSync(filePath, [
                'typedef struct FALLB18_API_TAG FALLB18_API_T;', // line 1
                '',                                               // line 2
                'struct FALLB18_API_TAG',                        // line 3
                '{',                                              // line 4
                '    void (*SomeField)(void);',                  // line 5
                '};',                                             // line 6
                '',                                               // line 7
                '/* Forward declaration only — no function body */', // line 8
                'void Fallback18_Func(void);',                   // line 9
                '',                                               // line 10
                'static const FALLB18_API_T tFallb18 = {',      // line 11
                '    .SomeField = Fallback18_Func,',             // line 12
                '};',                                             // line 13
                '',
            ].join('\n'));

            const indexer = new WorkspaceIndexer(dir, []);
            await indexer.fullIndex();

            // Fallback18_Func has no body, so it must NOT be in functionDefinitions
            expect(indexer.index.functionDefinitions.has('Fallback18_Func')).toBe(false);

            const fieldMap = indexer.index.vtableAssignments.get('FALLB18_API_T');
            expect(fieldMap).toBeDefined();
            const locs = fieldMap!.get('SomeField');
            expect(locs).toBeDefined();
            expect(locs!).toHaveLength(1);

            // ConcreteLocation.file must be the ASSIGNMENT file (fallback path):
            // resolveDefinitionLocation returns { file: r.file, line: 0 } →
            // mergeInto uses r.file and r.line (not defLoc.file/line).
            expect(locs![0].file).toBe(filePath);
            // ConcreteLocation.line must be the non-zero assignment line (r.line),
            // not the "not found" sentinel value of 0.
            expect(locs![0].line).toBeGreaterThan(0);
            expect(locs![0].functionName).toBe('Fallback18_Func');
        });

    }); // end 'File Discovery and Cross-File Resolution'

    // =========================================================================
    // Hash Consistency (WI-9.1)
    // =========================================================================

    describe('Hash Consistency', () => {

        let hashDir: string;

        beforeAll(() => {
            hashDir = fs.mkdtempSync(path.join(os.tmpdir(), 'libjuno-hash-'));
        });

        afterAll(() => {
            fs.rmSync(hashDir, { recursive: true, force: true });
        });

        afterEach(() => {
            jest.restoreAllMocks();
        });

        // =====================================================================
        // TC-WI-009: Hash consistency between hashText and hashFile for BOM files
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-001"]}
        it('TC-WI-009: loadFromCache() does not re-parse a BOM-prefixed file whose content is unchanged', async () => {
            const dir = path.join(hashDir, 'tc-wi-009');
            fs.mkdirSync(dir);

            // Write a C file with a UTF-8 BOM prefix (\xEF\xBB\xBF)
            const bomPrefix = Buffer.from([0xEF, 0xBB, 0xBF]);
            const cContent = Buffer.from('void BomFunction(void) {}\n', 'utf8');
            const bomFile = path.join(dir, 'bomfile.c');
            fs.writeFileSync(bomFile, Buffer.concat([bomPrefix, cContent]));

            // Populate cache via fullIndex()
            const indexer1 = new WorkspaceIndexer(dir, []);
            await indexer1.fullIndex();

            // Create a fresh indexer — same directory, same file unchanged
            const indexer2 = new WorkspaceIndexer(dir, []);
            const spy = jest.spyOn(visitorModule, 'parseFileWithDefs');

            const loaded = await indexer2.loadFromCache();

            // Cache must be found
            expect(loaded).toBe(true);

            // hashFile() and hashText() both use utf8 read + utf8 update, so hashes
            // must match — the BOM file must NOT be treated as stale → 0 re-parses
            expect(spy).not.toHaveBeenCalled();

            // The BOM file's function must still be in the index (loaded from cache)
            expect(indexer2.index.functionDefinitions.has('BomFunction')).toBe(true);
        });

    }); // end 'Hash Consistency'

    // =========================================================================
    // TC-WI-010 / TC-WI-011: mergeInto() traitRoots branch (WI-14.C7)
    // Covers workspaceIndexer.ts line 193: idx.traitRoots.set(r.rootType, r.apiType)
    // =========================================================================

    describe('mergeInto traitRoots branch', () => {

        let traitDir: string;

        beforeAll(() => {
            traitDir = fs.mkdtempSync(path.join(os.tmpdir(), 'libjuno-trait-'));
        });

        afterAll(() => {
            fs.rmSync(traitDir, { recursive: true, force: true });
        });

        // @{"verify": ["REQ-VSCODE-001"]}
        it('TC-WI-010: fullIndex() with JUNO_TRAIT_ROOT populates traitRoots', async () => {
            const dir = path.join(traitDir, 'tc010');
            fs.mkdirSync(dir);

            const header = [
                'typedef struct MY_TRAIT_ROOT_TAG MY_TRAIT_ROOT_T;',
                'typedef struct MY_TRAIT_API_TAG MY_TRAIT_API_T;',
                '',
                'struct MY_TRAIT_ROOT_TAG JUNO_TRAIT_ROOT(MY_TRAIT_API_T, );',
                '',
                'struct MY_TRAIT_API_TAG',
                '{',
                '    int (*DoWork)(void);',
                '};',
                '',
            ].join('\n');

            fs.writeFileSync(path.join(dir, 'trait010.h'), header);

            const indexer = new WorkspaceIndexer(dir, []);
            await indexer.fullIndex();

            // traitRoots must be populated — line 193 in workspaceIndexer.ts
            expect(indexer.index.traitRoots.get('MY_TRAIT_ROOT_T')).toBe('MY_TRAIT_API_T');
        });

        // @{"verify": ["REQ-VSCODE-001"]}
        it('TC-WI-011: fullIndex() with two JUNO_TRAIT_ROOT in separate files stores both in traitRoots', async () => {
            const dir = path.join(traitDir, 'tc011');
            fs.mkdirSync(dir);

            const headerA = [
                'typedef struct TRAITA_ROOT_TAG TRAITA_ROOT_T;',
                'typedef struct TRAITA_API_TAG TRAITA_API_T;',
                '',
                'struct TRAITA_ROOT_TAG JUNO_TRAIT_ROOT(TRAITA_API_T, );',
                '',
                'struct TRAITA_API_TAG',
                '{',
                '    int (*DoA)(void);',
                '};',
                '',
            ].join('\n');

            const headerB = [
                'typedef struct TRAITB_ROOT_TAG TRAITB_ROOT_T;',
                'typedef struct TRAITB_API_TAG TRAITB_API_T;',
                '',
                'struct TRAITB_ROOT_TAG JUNO_TRAIT_ROOT(TRAITB_API_T, );',
                '',
                'struct TRAITB_API_TAG',
                '{',
                '    int (*DoB)(void);',
                '};',
                '',
            ].join('\n');

            fs.writeFileSync(path.join(dir, 'traita.h'), headerA);
            fs.writeFileSync(path.join(dir, 'traitb.h'), headerB);

            const indexer = new WorkspaceIndexer(dir, []);
            await indexer.fullIndex();

            // Both trait roots must appear — line 193 executed twice across mergeInto calls
            expect(indexer.index.traitRoots.get('TRAITA_ROOT_T')).toBe('TRAITA_API_T');
            expect(indexer.index.traitRoots.get('TRAITB_ROOT_T')).toBe('TRAITB_API_T');

            // No cross-contamination
            expect(indexer.index.traitRoots.has('TRAITA_API_T')).toBe(false);
            expect(indexer.index.traitRoots.has('TRAITB_API_T')).toBe(false);
        });

    }); // end 'mergeInto traitRoots branch'

    // =========================================================================
    // TC-CACHE-008: Debounced write — rapid saves produce one cache write
    // =========================================================================

    describe('Debounced cache write', () => {

        let debounceDir: string;

        beforeAll(() => {
            debounceDir = fs.mkdtempSync(path.join(os.tmpdir(), 'libjuno-debounce-'));
        });

        afterAll(() => {
            fs.rmSync(debounceDir, { recursive: true, force: true });
        });

        afterEach(() => {
            jest.restoreAllMocks();
            jest.useRealTimers();
        });

        // @{"verify": ["REQ-VSCODE-001"]}
        it('TC-CACHE-008: rapid reindexFile calls produce exactly one saveToCache via 500 ms debounce', async () => {
            jest.useFakeTimers();

            const dir = path.join(debounceDir, 'tc-cache-008');
            fs.mkdirSync(dir);

            const filePath = path.join(dir, 'debounce008.c');
            fs.writeFileSync(filePath, 'void Debounce008Func(void) {}\n');

            const indexer = new WorkspaceIndexer(dir, []);

            const saveSpy = jest.spyOn(indexer as unknown as { saveToCache: () => Promise<void> }, 'saveToCache')
                .mockResolvedValue(undefined);

            // Call reindexFile 5 times in rapid succession
            await indexer.reindexFile(filePath);
            await indexer.reindexFile(filePath);
            await indexer.reindexFile(filePath);
            await indexer.reindexFile(filePath);
            await indexer.reindexFile(filePath);

            // saveToCache must NOT have been called yet — timer is still pending
            expect(saveSpy).not.toHaveBeenCalled();

            // Advance fake timers past the 500 ms debounce window
            jest.advanceTimersByTime(600);

            // Exactly one debounced write must have fired
            expect(saveSpy).toHaveBeenCalledTimes(1);

            // Clean up the pending timer
            indexer.dispose();
        });

    }); // end 'Debounced cache write'

    // =========================================================================
    // TC-TRACE-018 through TC-TRACE-020: Composition Root Resolution (REQ-VSCODE-036)
    // =========================================================================

    describe('Composition root resolution (REQ-VSCODE-036)', () => {

        let crDir: string;
        let crIndexer: WorkspaceIndexer | undefined;

        beforeEach(() => {
            crDir = path.join(os.tmpdir(), 'libjuno-test-cr-' + Date.now());
            fs.mkdirSync(crDir, { recursive: true });
            crIndexer = undefined;
        });

        afterEach(() => {
            if (crIndexer) { crIndexer.dispose(); }
            fs.rmSync(crDir, { recursive: true, force: true });
            jest.restoreAllMocks();
        });

        // @{"verify": ["REQ-VSCODE-036"]}
        it('TC-TRACE-018: resolveCompositionRoots stamps initCallFile/initCallLine on ConcreteLocation', async () => {
            // File 1: vtable struct definition
            const apiImplSrc = [
                'typedef struct { int (*DoWork)(void); } MY_API_T;',
                'static int MyImpl_DoWork(void) { return 0; }',
                'static const MY_API_T gtMyApi = { .DoWork = MyImpl_DoWork };',
            ].join('\n');

            // File 2: composition root — passes &gtMyApi as an argument
            const appSrc = [
                'static int SomeModule_Init(void *m, const void *api, void *h, void *u) { return 0; }',
                'static int run(void) { void *m = 0; return SomeModule_Init(&m, &gtMyApi, 0, 0); }',
            ].join('\n');

            fs.writeFileSync(path.join(crDir, 'api_impl.c'), apiImplSrc);
            fs.writeFileSync(path.join(crDir, 'app.c'), appSrc);

            crIndexer = new WorkspaceIndexer(crDir, []);
            await crIndexer.fullIndex();

            const fieldMap = crIndexer.index.vtableAssignments.get('MY_API_T');
            expect(fieldMap).toBeDefined();
            const locs = fieldMap!.get('DoWork');
            expect(locs).toBeDefined();
            expect(locs!.length).toBeGreaterThan(0);

            const loc = locs![0];
            expect(loc.apiVarName).toBe('gtMyApi');
            expect(loc.initCallFile).toBeDefined();
            expect(loc.initCallFile!.endsWith('app.c')).toBe(true);
            expect(loc.initCallLine).toBe(2);
        });

        // @{"verify": ["REQ-VSCODE-036"]}
        it('TC-TRACE-019: address-of a non-API variable does not appear in initCallIndex for that variable', async () => {
            const src = [
                'typedef struct { int (*Foo)(void); } MY_API_T;',
                'static int MyFoo(void) { return 0; }',
                'static const MY_API_T gApi = { .Foo = MyFoo };',
                'static void bar(MY_API_T *p) { (void)p; }',
                'static void run(void) { MY_API_T local; local.Foo = MyFoo; bar(&local); }',
            ].join('\n');

            fs.writeFileSync(path.join(crDir, 'single.c'), src);

            crIndexer = new WorkspaceIndexer(crDir, []);
            await crIndexer.fullIndex();

            // 'local' is not a known API variable name — must not appear in initCallIndex
            expect(crIndexer.index.initCallIndex.has('local')).toBe(false);

            // The ConcreteLocation for Foo should NOT have initCallLine from 'local'
            const fieldMap = crIndexer.index.vtableAssignments.get('MY_API_T');
            if (fieldMap) {
                const locs = fieldMap.get('Foo');
                if (locs && locs.length > 0) {
                    // If initCallLine is set it must come from gApi, not local
                    if (locs[0].initCallLine !== undefined) {
                        expect(locs[0].apiVarName).toBe('gApi');
                    }
                }
            }
        });

        // @{"verify": ["REQ-VSCODE-036"]}
        it('TC-TRACE-020: clearIndex() empties initCallIndex', () => {
            const { createEmptyIndex, clearIndex } = require('../navigationIndex');
            const idx = createEmptyIndex();

            // Manually populate initCallIndex
            idx.initCallIndex.set('gtSomeApi', [{ file: '/some/file.c', line: 10 }]);
            expect(idx.initCallIndex.size).toBe(1);

            // Clear the index
            clearIndex(idx);
            expect(idx.initCallIndex.size).toBe(0);
        });

        // @{"verify": ["REQ-VSCODE-036"]}
        it('TC-TRACE-023: resolveCompositionRoots stamps initCallFile on ConcreteLocation for a positional vtable initializer', async () => {
            // File 1: API struct definition (explicit struct tag syntax so the parser
            // can resolve the positional initializer immediately in the same file),
            // function implementations, and the positional vtable initializer.
            const apiImplPath = path.join(crDir, 'pos023_api.c');
            fs.writeFileSync(apiImplPath, [
                'struct MY_API_TAG { void (*FuncA)(void); void (*FuncB)(void); };',
                'static void FuncA(void) {}',
                'static void FuncB(void) {}',
                'static const MY_API_T gtMyApi = { FuncA, FuncB };',
            ].join('\n'));

            // File 2: call site that passes &gtMyApi as an argument
            const callSitePath = path.join(crDir, 'pos023_app.c');
            fs.writeFileSync(callSitePath, [
                'static void someInit(void *m, const void *api, void *h) { (void)m; (void)api; (void)h; }',
                'static void run(void) { void *myModule = 0; someInit(&myModule, &gtMyApi, 0); }',
            ].join('\n'));

            crIndexer = new WorkspaceIndexer(crDir, []);
            // fullIndex() processes both files and then calls resolveDeferred() +
            // resolveCompositionRoots(). Using fullIndex() ensures cross-file
            // resolution completes before we inspect the index.
            await crIndexer.fullIndex();

            // vtableAssignments must contain MY_API_T with FuncA
            const fieldMap = crIndexer.index.vtableAssignments.get('MY_API_T');
            expect(fieldMap).toBeDefined();

            const funcALocs = fieldMap!.get('FuncA');
            expect(funcALocs).toBeDefined();
            expect(funcALocs!.length).toBeGreaterThan(0);

            // At least one ConcreteLocation must have initCallFile pointing to the call-site file.
            // This verifies that varName was threaded through the positional vtable pipeline
            // (PendingPositionalVtable.varName → VtableAssignmentRecord.varName →
            //  ConcreteLocation.apiVarName) so that resolveCompositionRoots() could
            // match &gtMyApi in the call site.
            const stampedLoc = funcALocs!.find(loc => loc.initCallFile !== undefined);
            expect(stampedLoc).toBeDefined();
            expect(stampedLoc!.initCallFile!.endsWith('pos023_app.c')).toBe(true);
        });

    }); // end 'Composition root resolution'

});
