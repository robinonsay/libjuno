/// <reference types="jest" />
/**
 * @file CacheManager test suite — TC-CACHE-001, TC-CACHE-002, TC-CACHE-006,
 *       TC-CACHE-009, TC-CACHE-010, TC-CACHE-NEG-001, TC-CACHE-BND-001
 */

import * as fs from 'fs';
import * as path from 'path';
import * as os from 'os';
import {
    loadCache,
    saveCache,
    indexToCache,
    cacheToIndex,
    CACHE_VERSION,
} from '../cacheManager';
import { createEmptyIndex } from '../../indexer/navigationIndex';
import { WorkspaceIndexer } from '../../indexer/workspaceIndexer';
import {
    ConcreteLocation,
    TypeInfo,
    LocalTypeInfo,
    CacheFile,
} from '../../parser/types';

describe('CacheManager', () => {
    let tempDir: string;

    beforeAll(() => {
        tempDir = fs.mkdtempSync(path.join(os.tmpdir(), 'libjuno-cache-test-'));
    });

    afterAll(() => {
        fs.rmSync(tempDir, { recursive: true, force: true });
    });

    // =========================================================
    // TC-CACHE-001 — Full roundtrip preserves all 9 Map types
    // =========================================================
    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-CACHE-001: full roundtrip preserves all 9 Map types', () => {
        const index = createEmptyIndex();

        // moduleRoots: Map<string, string>
        index.moduleRoots.set('MY_ROOT_T', 'MY_API_T');

        // traitRoots: Map<string, string>
        index.traitRoots.set('MY_TRAIT_T', 'MY_TRAIT_API_T');

        // derivationChain: Map<string, string>
        index.derivationChain.set('MY_IMPL_T', 'MY_ROOT_T');

        // apiStructFields: Map<string, string[]>
        index.apiStructFields.set('MY_API_T', ['DoThing', 'DoOther']);

        // vtableAssignments: Map<string, Map<string, ConcreteLocation[]>>
        const innerMap = new Map<string, ConcreteLocation[]>();
        innerMap.set('DoThing', [{ functionName: 'MyImpl_DoThing', file: 'src/myImpl.c', line: 42 }]);
        index.vtableAssignments.set('MY_API_T', innerMap);

        // failureHandlerAssignments: Map<string, ConcreteLocation[]>
        index.failureHandlerAssignments.set('MY_ROOT_T', [
            { functionName: 'MyFailure', file: 'src/myImpl.c', line: 10 },
        ]);

        // apiMemberRegistry: Map<string, string>
        index.apiMemberRegistry.set('ptMyApi', 'MY_API_T');

        // functionDefinitions: Map<string, FunctionDefinitionRecord[]>
        index.functionDefinitions.set('MyImpl_DoThing', [
            { functionName: 'MyImpl_DoThing', file: 'src/myImpl.c', line: 42, isStatic: false },
        ]);

        // localTypeInfo: Map<string, LocalTypeInfo> — both nested Maps
        const varMap = new Map<string, TypeInfo>();
        varMap.set('ptSelf', { name: 'ptSelf', typeName: 'MY_ROOT_T', isPointer: true, isConst: false, isArray: false });
        const localVars = new Map<string, Map<string, TypeInfo>>();
        localVars.set('MyFunc', varMap);
        const funcParams = new Map<string, TypeInfo[]>();
        funcParams.set('MyFunc', [
            { name: 'ptSelf', typeName: 'MY_ROOT_T', isPointer: true, isConst: false, isArray: false },
        ]);
        const lti: LocalTypeInfo = { localVariables: localVars, functionParameters: funcParams };
        index.localTypeInfo.set('src/myImpl.c', lti);

        const fileHashes = new Map<string, string>();
        fileHashes.set('src/myImpl.c', 'abc123');

        // Roundtrip: indexToCache → JSON.stringify → JSON.parse → cacheToIndex
        const cacheFile = indexToCache(index, fileHashes);
        const json = JSON.stringify(cacheFile);
        const parsed = JSON.parse(json) as CacheFile;
        const restored = cacheToIndex(parsed);

        // moduleRoots
        expect(restored.moduleRoots).toBeInstanceOf(Map);
        expect(restored.moduleRoots.get('MY_ROOT_T')).toBe('MY_API_T');
        expect(restored.moduleRoots.size).toBe(1);

        // traitRoots
        expect(restored.traitRoots).toBeInstanceOf(Map);
        expect(restored.traitRoots.get('MY_TRAIT_T')).toBe('MY_TRAIT_API_T');

        // derivationChain
        expect(restored.derivationChain).toBeInstanceOf(Map);
        expect(restored.derivationChain.get('MY_IMPL_T')).toBe('MY_ROOT_T');

        // apiStructFields
        expect(restored.apiStructFields).toBeInstanceOf(Map);
        expect(restored.apiStructFields.get('MY_API_T')).toEqual(['DoThing', 'DoOther']);

        // vtableAssignments — inner value must be a real Map, not a plain object
        expect(restored.vtableAssignments).toBeInstanceOf(Map);
        const restoredInner = restored.vtableAssignments.get('MY_API_T');
        expect(restoredInner).toBeInstanceOf(Map);
        const restoredLocs = restoredInner!.get('DoThing');
        expect(restoredLocs).toEqual([{ functionName: 'MyImpl_DoThing', file: 'src/myImpl.c', line: 42 }]);

        // failureHandlerAssignments
        expect(restored.failureHandlerAssignments).toBeInstanceOf(Map);
        expect(restored.failureHandlerAssignments.get('MY_ROOT_T')).toEqual([
            { functionName: 'MyFailure', file: 'src/myImpl.c', line: 10 },
        ]);

        // apiMemberRegistry
        expect(restored.apiMemberRegistry).toBeInstanceOf(Map);
        expect(restored.apiMemberRegistry.get('ptMyApi')).toBe('MY_API_T');

        // functionDefinitions — functionName is re-synthesised from the map key
        expect(restored.functionDefinitions).toBeInstanceOf(Map);
        const restoredDefs = restored.functionDefinitions.get('MyImpl_DoThing');
        expect(restoredDefs).toBeDefined();
        expect(restoredDefs![0]).toMatchObject({
            functionName: 'MyImpl_DoThing',
            file: 'src/myImpl.c',
            line: 42,
            isStatic: false,
        });

        // localTypeInfo — both nested Maps must be real Map instances
        expect(restored.localTypeInfo).toBeInstanceOf(Map);
        const restoredLti = restored.localTypeInfo.get('src/myImpl.c');
        expect(restoredLti).toBeDefined();
        expect(restoredLti!.localVariables).toBeInstanceOf(Map);
        const restoredVarMap = restoredLti!.localVariables.get('MyFunc');
        expect(restoredVarMap).toBeInstanceOf(Map);
        expect(restoredVarMap!.get('ptSelf')).toEqual({
            name: 'ptSelf',
            typeName: 'MY_ROOT_T',
            isPointer: true,
            isConst: false,
            isArray: false,
        });
        expect(restoredLti!.functionParameters).toBeInstanceOf(Map);
        expect(restoredLti!.functionParameters.get('MyFunc')).toEqual([
            { name: 'ptSelf', typeName: 'MY_ROOT_T', isPointer: true, isConst: false, isArray: false },
        ]);
    });

    // =========================================================
    // TC-CACHE-002 — Version mismatch returns null
    // =========================================================
    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-CACHE-002: version mismatch returns null', () => {
        const filePath = path.join(tempDir, 'cache-v999.json');
        const staleCache = {
            version: '999',
            generatedAt: new Date().toISOString(),
            fileHashes: {},
            moduleRoots: {},
            traitRoots: {},
            derivationChain: {},
            apiStructFields: {},
            vtableAssignments: {},
            failureHandlerAssignments: {},
            apiMemberRegistry: {},
            functionDefinitions: {},
            localTypeInfo: {},
        };
        fs.writeFileSync(filePath, JSON.stringify(staleCache), 'utf8');

        const result = loadCache(filePath);
        expect(result).toBeNull();
    });

    // =========================================================
    // TC-CACHE-006 — saveCache writes valid JSON to disk
    // =========================================================
    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-CACHE-006: saveCache writes a readable JSON file with the correct structure', async () => {
        const cachePath = path.join(tempDir, 'cache-006.json');
        const index = createEmptyIndex();
        index.moduleRoots.set('ROOT_T', 'API_T');
        const fileHashes = new Map<string, string>();
        fileHashes.set('src/foo.c', 'deadbeef');

        const cacheFile = indexToCache(index, fileHashes);
        await saveCache(cachePath, cacheFile);

        const raw = fs.readFileSync(cachePath, 'utf8');
        const parsedBack = JSON.parse(raw);

        expect(parsedBack.version).toBe(CACHE_VERSION);
        expect(parsedBack.moduleRoots).toMatchObject({ ROOT_T: 'API_T' });
        expect(parsedBack.fileHashes).toMatchObject({ 'src/foo.c': 'deadbeef' });
        expect(parsedBack).toHaveProperty('traitRoots');
        expect(parsedBack).toHaveProperty('derivationChain');
        expect(parsedBack).toHaveProperty('vtableAssignments');
        expect(parsedBack).toHaveProperty('localTypeInfo');
        expect(parsedBack).toHaveProperty('generatedAt');

        // Verify loadCache reads the same file successfully (success path)
        const loaded = loadCache(cachePath);
        expect(loaded).not.toBeNull();
        expect(loaded!.version).toBe(CACHE_VERSION);
        expect(loaded!.moduleRoots['ROOT_T']).toBe('API_T');
    });

    // =========================================================
    // TC-CACHE-009 — Atomic write via temp file + rename
    // =========================================================
    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-CACHE-009: saveCache writes to a .tmp path then renames to the final path', async () => {
        const finalPath = path.join(tempDir, 'cache-009.json');
        const index = createEmptyIndex();
        const fileHashes = new Map<string, string>();
        const cacheFile = indexToCache(index, fileHashes);

        const writeSpy = jest.spyOn(fs.promises, 'writeFile');
        const renameSpy = jest.spyOn(fs.promises, 'rename');

        try {
            await saveCache(finalPath, cacheFile);

            // writeFile must be called exactly once with the .tmp path
            expect(writeSpy).toHaveBeenCalledTimes(1);
            const writeCallPath = writeSpy.mock.calls[0][0] as string;
            expect(writeCallPath).toMatch(/\.tmp$/);
            expect(writeCallPath).not.toBe(finalPath);

            // rename must move .tmp → final path in one call
            expect(renameSpy).toHaveBeenCalledTimes(1);
            expect(renameSpy).toHaveBeenCalledWith(writeCallPath, finalPath);
        } finally {
            writeSpy.mockRestore();
            renameSpy.mockRestore();
        }
    });

    // =========================================================
    // TC-CACHE-010 — Corrupted field (null) should not throw
    //
    // All 9 cache fields (moduleRoots, traitRoots, derivationChain,
    // apiStructFields, vtableAssignments, failureHandlerAssignments,
    // apiMemberRegistry, functionDefinitions, localTypeInfo) are now
    // null-guarded with `?? {}` in cacheToIndex(), so a null field no
    // longer throws TypeError.
    // =========================================================
    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-CACHE-010: cacheToIndex handles null vtableAssignments without throwing', () => {
        const filePath = path.join(tempDir, 'cache-010.json');
        const corruptedCache = {
            version: CACHE_VERSION,
            generatedAt: new Date().toISOString(),
            fileHashes: {},
            moduleRoots: {},
            traitRoots: {},
            derivationChain: {},
            apiStructFields: {},
            vtableAssignments: null,   // intentionally corrupted
            failureHandlerAssignments: {},
            apiMemberRegistry: {},
            functionDefinitions: {},
            localTypeInfo: {},
        };
        fs.writeFileSync(filePath, JSON.stringify(corruptedCache), 'utf8');

        const cache = loadCache(filePath);
        expect(cache).not.toBeNull();

        // Should not throw, but currently throws:
        // TypeError: Cannot convert undefined or null to object (Object.entries)
        expect(() => cacheToIndex(cache!)).not.toThrow();
    });

    // =========================================================
    // TC-CACHE-NEG-001 — Non-existent file returns null
    // =========================================================
    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-CACHE-NEG-001: loadCache returns null for a non-existent file path', () => {
        const result = loadCache(path.join(os.tmpdir(), 'nonexistent-path-12345', 'no-such-file.json'));
        expect(result).toBeNull();
    });

    // =========================================================
    // TC-CACHE-BND-001 — Sparse roundtrip (only moduleRoots populated)
    // =========================================================
    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-CACHE-BND-001: sparse roundtrip preserves moduleRoots and leaves all other Maps empty', () => {
        const index = createEmptyIndex();
        index.moduleRoots.set('SPARSE_ROOT_T', 'SPARSE_API_T');

        const fileHashes = new Map<string, string>();
        const cacheFile = indexToCache(index, fileHashes);
        const json = JSON.stringify(cacheFile);
        const parsed = JSON.parse(json) as CacheFile;
        const restored = cacheToIndex(parsed);

        // The one populated map is preserved
        expect(restored.moduleRoots).toBeInstanceOf(Map);
        expect(restored.moduleRoots.size).toBe(1);
        expect(restored.moduleRoots.get('SPARSE_ROOT_T')).toBe('SPARSE_API_T');

        // All remaining 8 Maps are empty (not undefined, not missing)
        expect(restored.traitRoots).toBeInstanceOf(Map);
        expect(restored.traitRoots.size).toBe(0);

        expect(restored.derivationChain).toBeInstanceOf(Map);
        expect(restored.derivationChain.size).toBe(0);

        expect(restored.apiStructFields).toBeInstanceOf(Map);
        expect(restored.apiStructFields.size).toBe(0);

        expect(restored.vtableAssignments).toBeInstanceOf(Map);
        expect(restored.vtableAssignments.size).toBe(0);

        expect(restored.failureHandlerAssignments).toBeInstanceOf(Map);
        expect(restored.failureHandlerAssignments.size).toBe(0);

        expect(restored.apiMemberRegistry).toBeInstanceOf(Map);
        expect(restored.apiMemberRegistry.size).toBe(0);

        expect(restored.functionDefinitions).toBeInstanceOf(Map);
        expect(restored.functionDefinitions.size).toBe(0);

        expect(restored.localTypeInfo).toBeInstanceOf(Map);
        expect(restored.localTypeInfo.size).toBe(0);
    });

    // =========================================================
    // TC-CACHE-NEG-002 — JSON that parses to a non-object returns null
    // (covers loadCache line 41, if-arm 0: typeof parsed !== "object")
    // =========================================================
    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-CACHE-NEG-002: loadCache returns null when file contains a JSON number (not an object)', () => {
        const filePath = path.join(tempDir, 'cache-neg-002.json');
        fs.writeFileSync(filePath, '42', 'utf8');

        const result = loadCache(filePath);

        expect(result).toBeNull();
    });

    // =========================================================
    // TC-CACHE-NEG-003 — JSON that parses to null returns null
    // (covers loadCache line 41, parsed === null arm)
    // =========================================================
    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-CACHE-NEG-003: loadCache returns null when file contains JSON null', () => {
        const filePath = path.join(tempDir, 'cache-neg-003.json');
        fs.writeFileSync(filePath, 'null', 'utf8');

        const result = loadCache(filePath);

        expect(result).toBeNull();
    });

    // =========================================================
    // TC-CACHE-011 — cacheToIndex with all optional fields undefined
    // triggers all 8 ?? {} fallbacks (lines 182, 187, 192, 197, 211, 216, 221, 232)
    // =========================================================
    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-CACHE-011: cacheToIndex produces empty Maps when all optional fields are undefined', () => {
        // Construct a minimal cache where every optional field is absent (undefined),
        // forcing every `field ?? {}` expression to take the {} fallback branch.
        const minimalCache = {
            version: CACHE_VERSION,
            generatedAt: '2026-01-01T00:00:00.000Z',
            fileHashes: {},
            // All 8 optional map fields intentionally omitted
        } as unknown as CacheFile;

        let result: ReturnType<typeof cacheToIndex>;
        expect(() => { result = cacheToIndex(minimalCache); }).not.toThrow();

        // Every Map must be an empty Map instance — not thrown, not undefined
        expect(result!.moduleRoots).toBeInstanceOf(Map);
        expect(result!.moduleRoots.size).toBe(0);

        expect(result!.traitRoots).toBeInstanceOf(Map);
        expect(result!.traitRoots.size).toBe(0);

        expect(result!.derivationChain).toBeInstanceOf(Map);
        expect(result!.derivationChain.size).toBe(0);

        expect(result!.apiStructFields).toBeInstanceOf(Map);
        expect(result!.apiStructFields.size).toBe(0);

        expect(result!.vtableAssignments).toBeInstanceOf(Map);
        expect(result!.vtableAssignments.size).toBe(0);

        expect(result!.failureHandlerAssignments).toBeInstanceOf(Map);
        expect(result!.failureHandlerAssignments.size).toBe(0);

        expect(result!.apiMemberRegistry).toBeInstanceOf(Map);
        expect(result!.apiMemberRegistry.size).toBe(0);

        expect(result!.functionDefinitions).toBeInstanceOf(Map);
        expect(result!.functionDefinitions.size).toBe(0);

        expect(result!.localTypeInfo).toBeInstanceOf(Map);
        expect(result!.localTypeInfo.size).toBe(0);
    });
});

// =============================================================================
// TC-CACHE-PIPE-001 — Full pipeline roundtrip smoke test
//
// Verifies: populate (fullIndex) → save cache → fresh indexer →
//           loadFromCache → restored index matches original.
// =============================================================================

// @{"verify": ["REQ-VSCODE-001"]}
describe('CacheManager — Full Pipeline Roundtrip', () => {
    let pipelineTempDir: string;

    beforeAll(() => {
        pipelineTempDir = fs.mkdtempSync(path.join(os.tmpdir(), 'libjuno-cache-pipe-'));
    });

    afterAll(() => {
        fs.rmSync(pipelineTempDir, { recursive: true, force: true });
    });

    // =========================================================
    // TC-CACHE-PIPE-001 — save → clear → load → verify roundtrip
    // =========================================================
    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-CACHE-PIPE-001: fullIndex saves cache; fresh indexer loadFromCache restores moduleRoots, apiStructFields, and vtableAssignments', async () => {
        // Write a synthetic C file with a module root, API struct, and vtable assignment
        const srcFile = path.join(pipelineTempDir, 'smoke.c');
        fs.writeFileSync(srcFile, [
            'typedef struct SMOKE_API_TAG SMOKE_API_T;',
            'typedef struct SMOKE_ROOT_TAG SMOKE_ROOT_T;',
            '',
            'struct SMOKE_ROOT_TAG JUNO_MODULE_ROOT(SMOKE_API_T, );',
            '',
            'struct SMOKE_API_TAG',
            '{',
            '    JUNO_STATUS_T (*DoSmoke)(SMOKE_ROOT_T *ptSelf);',
            '};',
            '',
            'static JUNO_STATUS_T SmokeImpl_DoSmoke(SMOKE_ROOT_T *ptSelf)',
            '{',
            '    return JUNO_STATUS_SUCCESS;',
            '}',
            '',
            'static const SMOKE_API_T tSmokeApi = {',
            '    .DoSmoke = SmokeImpl_DoSmoke,',
            '};',
            '',
        ].join('\n'), 'utf8');

        // Step 1: Full index — parses the file and saves cache to disk
        const indexer1 = new WorkspaceIndexer(pipelineTempDir, []);
        await indexer1.fullIndex();

        // Step 2: Capture original index state for later comparison
        const originalModuleRootsSize = indexer1.index.moduleRoots.size;
        const originalApiType = indexer1.index.moduleRoots.get('SMOKE_ROOT_T');
        const originalApiFields = indexer1.index.apiStructFields.get('SMOKE_API_T');
        const originalVtable = indexer1.index.vtableAssignments.get('SMOKE_API_T');

        // Sanity: indexer1 actually populated the index (not a no-op stub check)
        expect(originalModuleRootsSize).toBeGreaterThan(0);
        expect(originalApiType).toBe('SMOKE_API_T');
        expect(originalApiFields).toContain('DoSmoke');
        expect(originalVtable?.get('DoSmoke')?.[0]).toMatchObject({ functionName: 'SmokeImpl_DoSmoke' });

        // Step 3: Create a fresh indexer — same workspace root, empty in-memory index
        const indexer2 = new WorkspaceIndexer(pipelineTempDir, []);
        const loaded = await indexer2.loadFromCache();

        // Step 4: Cache must have been found (file was written by fullIndex)
        expect(loaded).toBe(true);

        // Step 5: Verify moduleRoots restored
        expect(indexer2.index.moduleRoots.size).toBe(originalModuleRootsSize);
        expect(indexer2.index.moduleRoots.get('SMOKE_ROOT_T')).toBe('SMOKE_API_T');

        // Step 6: Verify apiStructFields restored
        const restoredFields = indexer2.index.apiStructFields.get('SMOKE_API_T');
        expect(restoredFields).toBeDefined();
        expect(restoredFields).toContain('DoSmoke');

        // Step 7: Verify vtableAssignments restored with correct concrete location
        const restoredVtable = indexer2.index.vtableAssignments.get('SMOKE_API_T');
        expect(restoredVtable).toBeInstanceOf(Map);
        const restoredAssignments = restoredVtable!.get('DoSmoke');
        expect(restoredAssignments).toBeDefined();
        expect(restoredAssignments!.length).toBeGreaterThan(0);
        expect(restoredAssignments![0]).toMatchObject({ functionName: 'SmokeImpl_DoSmoke' });
    });
});
