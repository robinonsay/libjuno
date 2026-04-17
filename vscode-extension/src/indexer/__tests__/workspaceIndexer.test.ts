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
 * TC-WI-BND-001 fullIndex() on empty workspace → empty index (REQ-VSCODE-001)
 * TC-CACHE-003  loadFromCache() re-parses only modified file on hash mismatch (REQ-VSCODE-001)
 * TC-CACHE-004  loadFromCache() parses file added after initial fullIndex() (REQ-VSCODE-001)
 * TC-CACHE-005  loadFromCache() retains stale entries for deleted file — M4 (REQ-VSCODE-001)
 * TC-FILE-001   fullIndex() discovers all 6 C/C++ file extensions (REQ-VSCODE-021)
 * TC-WI-007     Positional vtable initializer resolved, same-file (REQ-VSCODE-012)
 * TC-WI-008     Failure handler root-type resolution via localTypeInfo (REQ-VSCODE-016)
 */

import * as fs from 'fs';
import * as path from 'path';
import * as os from 'os';
import { WorkspaceIndexer } from '../workspaceIndexer';
import * as visitorModule from '../../parser/visitor';

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
        // NOTE on cross-file deferred mechanism:
        //   The DeferredPositional interface and resolveDeferred() exist in
        //   workspaceIndexer.ts to support cross-file positional resolution, BUT
        //   the producer side is not yet implemented: when the visitor's
        //   extractPositionalVtable() cannot find the API struct in the current
        //   file it silently returns without pushing any DeferredPositional entry.
        //   As a result, cross-file positional initializers are dropped (no vtable
        //   assignments emitted). This test verifies the same-file path which IS
        //   fully implemented.
        //
        //   Additionally, resolveDeferred() has a consumer-side defect: its
        //   duplicate-check compares e.line against the assignment line (d.lines[i]),
        //   but loc.line stores the definition line (from resolveDefinitionLine()).
        //   When the producer bug is fixed, the mismatched comparison will cause
        //   duplicate ConcreteLocation entries on repeated calls.
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
        // NOTE on multi-module limitation:
        //   resolveFailureHandlerRootType() walks ALL localTypeInfo.functionParameters
        //   in the file and returns the FIRST matching known root type it finds.
        //   When two modules share the same file, both failure handlers are attributed
        //   to whichever root type appears first in the Map iteration order. This is
        //   a known single-file disambiguation bug. This test verifies the working
        //   (single-module) case where there is no ambiguity.
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

    }); // end 'File Discovery and Cross-File Resolution'

});
