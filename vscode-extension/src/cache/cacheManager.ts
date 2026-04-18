// @{"req": ["REQ-VSCODE-001"]}
import * as fs from "fs";
import * as path from "path";
import {
    CacheFile,
    NavigationIndex,
    ConcreteLocation,
    LocalTypeInfo,
    TypeInfo,
    FunctionDefinitionRecord,
    CacheLocalTypeInfo,
} from "../parser/types";
import { createEmptyIndex } from "../indexer/navigationIndex";

/** Cache format version. Increment when the schema changes. */
export const CACHE_VERSION = "1";

/**
 * Reads and validates the cache file at the given path.
 * Returns null if the file is missing, cannot be parsed, or has a version mismatch.
 *
 * @param cacheFilePath Absolute path to the JSON cache file.
 * @returns The parsed CacheFile, or null on any failure.
 */
export function loadCache(cacheFilePath: string): CacheFile | null {
    if (!fs.existsSync(cacheFilePath)) {
        return null;
    }
    let raw: string;
    try {
        raw = fs.readFileSync(cacheFilePath, "utf8");
    } catch {
        return null;
    }
    let parsed: unknown;
    try {
        parsed = JSON.parse(raw);
    } catch {
        return null;
    }
    if (typeof parsed !== "object" || parsed === null) {
        return null;
    }
    const obj = parsed as Record<string, unknown>;
    if (obj["version"] !== CACHE_VERSION) {
        return null;
    }
    return parsed as CacheFile;
}

/**
 * Atomically writes the cache to disk by writing a temp file then renaming it.
 * Creates the `.libjuno/` directory if it does not exist.
 *
 * @param cacheFilePath Absolute path to the target JSON cache file.
 * @param cache         The CacheFile object to serialize.
 */
export async function saveCache(cacheFilePath: string, cache: CacheFile): Promise<void> {
    const dir = path.dirname(cacheFilePath);
    if (!fs.existsSync(dir)) {
        fs.mkdirSync(dir, { recursive: true });
    }
    const json = JSON.stringify(cache, null, 2);
    const tmpPath = cacheFilePath + ".tmp";
    await fs.promises.writeFile(tmpPath, json, "utf8");
    await fs.promises.rename(tmpPath, cacheFilePath);
}

/**
 * Converts a NavigationIndex and file hashes into a CacheFile suitable for JSON serialization.
 * ES6 Maps are converted to plain objects.
 *
 * @param index      The in-memory NavigationIndex to serialize.
 * @param fileHashes Map of workspace-relative file path → SHA-256 hex digest.
 * @returns          A CacheFile ready for JSON.stringify.
 */
export function indexToCache(index: NavigationIndex, fileHashes: Map<string, string>): CacheFile {
    // moduleRoots: Map<string, string> → Record<string, string>
    const moduleRoots: Record<string, string> = {};
    for (const [k, v] of index.moduleRoots) {
        moduleRoots[k] = v;
    }

    // traitRoots: Map<string, string> → Record<string, string>
    const traitRoots: Record<string, string> = {};
    for (const [k, v] of index.traitRoots) {
        traitRoots[k] = v;
    }

    // derivationChain: Map<string, string> → Record<string, string>
    const derivationChain: Record<string, string> = {};
    for (const [k, v] of index.derivationChain) {
        derivationChain[k] = v;
    }

    // apiStructFields: Map<string, string[]> → Record<string, string[]>
    const apiStructFields: Record<string, string[]> = {};
    for (const [k, v] of index.apiStructFields) {
        apiStructFields[k] = v;
    }

    // vtableAssignments: Map<string, Map<string, ConcreteLocation[]>>
    //   → Record<string, Record<string, ConcreteLocation[]>>
    const vtableAssignments: Record<string, Record<string, ConcreteLocation[]>> = {};
    for (const [apiType, fieldMap] of index.vtableAssignments) {
        vtableAssignments[apiType] = {};
        for (const [field, locs] of fieldMap) {
            vtableAssignments[apiType][field] = locs;
        }
    }

    // failureHandlerAssignments: Map<string, ConcreteLocation[]>
    //   → Record<string, ConcreteLocation[]>
    const failureHandlerAssignments: Record<string, ConcreteLocation[]> = {};
    for (const [k, v] of index.failureHandlerAssignments) {
        failureHandlerAssignments[k] = v;
    }

    // apiMemberRegistry: Map<string, string> → Record<string, string>
    const apiMemberRegistry: Record<string, string> = {};
    for (const [k, v] of index.apiMemberRegistry) {
        apiMemberRegistry[k] = v;
    }

    // functionDefinitions: Map<string, FunctionDefinitionRecord[]>
    //   → Record<string, Array<Omit<FunctionDefinitionRecord, 'functionName'>>>
    const functionDefinitions: Record<string, Array<Omit<FunctionDefinitionRecord, "functionName">>> = {};
    for (const [fnName, defs] of index.functionDefinitions) {
        functionDefinitions[fnName] = defs.map(({ file, line, isStatic }) => ({ file, line, isStatic }));
    }

    // localTypeInfo: Map<string, LocalTypeInfo> → Record<string, CacheLocalTypeInfo>
    const localTypeInfo: Record<string, CacheLocalTypeInfo> = {};
    for (const [filePath, lti] of index.localTypeInfo) {
        const localVariables: Record<string, Record<string, TypeInfo>> = {};
        for (const [fn, varMap] of lti.localVariables) {
            localVariables[fn] = {};
            for (const [varName, ti] of varMap) {
                localVariables[fn][varName] = ti;
            }
        }
        const functionParameters: Record<string, TypeInfo[]> = {};
        for (const [fn, params] of lti.functionParameters) {
            functionParameters[fn] = params;
        }
        localTypeInfo[filePath] = { localVariables, functionParameters };
    }

    // fileHashes: Map<string, string> → Record<string, string>
    const fileHashesObj: Record<string, string> = {};
    for (const [k, v] of fileHashes) {
        fileHashesObj[k] = v;
    }

    return {
        version: CACHE_VERSION,
        generatedAt: new Date().toISOString(),
        fileHashes: fileHashesObj,
        moduleRoots,
        traitRoots,
        derivationChain,
        apiStructFields,
        vtableAssignments,
        failureHandlerAssignments,
        apiMemberRegistry,
        functionDefinitions,
        localTypeInfo,
    };
}

/**
 * Converts a CacheFile (loaded from JSON) back into an in-memory NavigationIndex.
 * Plain objects are converted to ES6 Maps.
 *
 * @param cache The CacheFile loaded from disk.
 * @returns     A fully populated NavigationIndex.
 */
export function cacheToIndex(cache: CacheFile): NavigationIndex {
    const index = createEmptyIndex();

    // moduleRoots
    for (const [k, v] of Object.entries(cache.moduleRoots ?? {})) {
        index.moduleRoots.set(k, v);
    }

    // traitRoots
    for (const [k, v] of Object.entries(cache.traitRoots ?? {})) {
        index.traitRoots.set(k, v);
    }

    // derivationChain
    for (const [k, v] of Object.entries(cache.derivationChain ?? {})) {
        index.derivationChain.set(k, v);
    }

    // apiStructFields
    for (const [k, v] of Object.entries(cache.apiStructFields ?? {})) {
        index.apiStructFields.set(k, v);
    }

    // vtableAssignments
    for (const [apiType, fieldObj] of Object.entries(cache.vtableAssignments ?? {})) {
        const fieldMap = new Map<string, ConcreteLocation[]>();
        for (const [field, locs] of Object.entries(fieldObj)) {
            fieldMap.set(field, locs);
        }
        index.vtableAssignments.set(apiType, fieldMap);
    }

    // failureHandlerAssignments
    for (const [k, v] of Object.entries(cache.failureHandlerAssignments ?? {})) {
        index.failureHandlerAssignments.set(k, v);
    }

    // apiMemberRegistry
    for (const [k, v] of Object.entries(cache.apiMemberRegistry ?? {})) {
        index.apiMemberRegistry.set(k, v);
    }

    // functionDefinitions
    for (const [fnName, defs] of Object.entries(cache.functionDefinitions ?? {})) {
        const records: FunctionDefinitionRecord[] = defs.map((d) => ({
            functionName: fnName,
            file: d.file,
            line: d.line,
            isStatic: d.isStatic,
        }));
        index.functionDefinitions.set(fnName, records);
    }

    // localTypeInfo
    for (const [filePath, clti] of Object.entries(cache.localTypeInfo ?? {})) {
        const localVariables = new Map<string, Map<string, TypeInfo>>();
        for (const [fn, varObj] of Object.entries(clti.localVariables)) {
            const varMap = new Map<string, TypeInfo>();
            for (const [varName, ti] of Object.entries(varObj)) {
                varMap.set(varName, ti);
            }
            localVariables.set(fn, varMap);
        }
        const functionParameters = new Map<string, TypeInfo[]>();
        for (const [fn, params] of Object.entries(clti.functionParameters)) {
            functionParameters.set(fn, params);
        }
        const lti: LocalTypeInfo = { localVariables, functionParameters };
        index.localTypeInfo.set(filePath, lti);
    }

    return index;
}
