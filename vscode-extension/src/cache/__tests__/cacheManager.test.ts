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
});
