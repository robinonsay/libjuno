"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    var desc = Object.getOwnPropertyDescriptor(m, k);
    if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
      desc = { enumerable: true, get: function() { return m[k]; } };
    }
    Object.defineProperty(o, k2, desc);
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || (function () {
    var ownKeys = function(o) {
        ownKeys = Object.getOwnPropertyNames || function (o) {
            var ar = [];
            for (var k in o) if (Object.prototype.hasOwnProperty.call(o, k)) ar[ar.length] = k;
            return ar;
        };
        return ownKeys(o);
    };
    return function (mod) {
        if (mod && mod.__esModule) return mod;
        var result = {};
        if (mod != null) for (var k = ownKeys(mod), i = 0; i < k.length; i++) if (k[i] !== "default") __createBinding(result, mod, k[i]);
        __setModuleDefault(result, mod);
        return result;
    };
})();
Object.defineProperty(exports, "__esModule", { value: true });
exports.CACHE_VERSION = void 0;
exports.loadCache = loadCache;
exports.saveCache = saveCache;
exports.indexToCache = indexToCache;
exports.cacheToIndex = cacheToIndex;
// @{"req": ["REQ-VSCODE-001"]}
const fs = __importStar(require("fs"));
const path = __importStar(require("path"));
const navigationIndex_1 = require("../indexer/navigationIndex");
/** Cache format version. Increment when the schema changes. */
exports.CACHE_VERSION = "1";
/**
 * Reads and validates the cache file at the given path.
 * Returns null if the file is missing, cannot be parsed, or has a version mismatch.
 *
 * @param cacheFilePath Absolute path to the JSON cache file.
 * @returns The parsed CacheFile, or null on any failure.
 */
function loadCache(cacheFilePath) {
    if (!fs.existsSync(cacheFilePath)) {
        return null;
    }
    let raw;
    try {
        raw = fs.readFileSync(cacheFilePath, "utf8");
    }
    catch {
        return null;
    }
    let parsed;
    try {
        parsed = JSON.parse(raw);
    }
    catch {
        return null;
    }
    if (typeof parsed !== "object" || parsed === null) {
        return null;
    }
    const obj = parsed;
    if (obj["version"] !== exports.CACHE_VERSION) {
        return null;
    }
    return parsed;
}
/**
 * Atomically writes the cache to disk by writing a temp file then renaming it.
 * Creates the `.libjuno/` directory if it does not exist.
 *
 * @param cacheFilePath Absolute path to the target JSON cache file.
 * @param cache         The CacheFile object to serialize.
 */
async function saveCache(cacheFilePath, cache) {
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
function indexToCache(index, fileHashes) {
    // moduleRoots: Map<string, string> → Record<string, string>
    const moduleRoots = {};
    for (const [k, v] of index.moduleRoots) {
        moduleRoots[k] = v;
    }
    // traitRoots: Map<string, string> → Record<string, string>
    const traitRoots = {};
    for (const [k, v] of index.traitRoots) {
        traitRoots[k] = v;
    }
    // derivationChain: Map<string, string> → Record<string, string>
    const derivationChain = {};
    for (const [k, v] of index.derivationChain) {
        derivationChain[k] = v;
    }
    // apiStructFields: Map<string, string[]> → Record<string, string[]>
    const apiStructFields = {};
    for (const [k, v] of index.apiStructFields) {
        apiStructFields[k] = v;
    }
    // vtableAssignments: Map<string, Map<string, ConcreteLocation[]>>
    //   → Record<string, Record<string, ConcreteLocation[]>>
    const vtableAssignments = {};
    for (const [apiType, fieldMap] of index.vtableAssignments) {
        vtableAssignments[apiType] = {};
        for (const [field, locs] of fieldMap) {
            vtableAssignments[apiType][field] = locs;
        }
    }
    // failureHandlerAssignments: Map<string, ConcreteLocation[]>
    //   → Record<string, ConcreteLocation[]>
    const failureHandlerAssignments = {};
    for (const [k, v] of index.failureHandlerAssignments) {
        failureHandlerAssignments[k] = v;
    }
    // apiMemberRegistry: Map<string, string> → Record<string, string>
    const apiMemberRegistry = {};
    for (const [k, v] of index.apiMemberRegistry) {
        apiMemberRegistry[k] = v;
    }
    // functionDefinitions: Map<string, FunctionDefinitionRecord[]>
    //   → Record<string, Array<Omit<FunctionDefinitionRecord, 'functionName'>>>
    const functionDefinitions = {};
    for (const [fnName, defs] of index.functionDefinitions) {
        functionDefinitions[fnName] = defs.map(({ file, line, isStatic }) => ({ file, line, isStatic }));
    }
    // localTypeInfo: Map<string, LocalTypeInfo> → Record<string, CacheLocalTypeInfo>
    const localTypeInfo = {};
    for (const [filePath, lti] of index.localTypeInfo) {
        const localVariables = {};
        for (const [fn, varMap] of lti.localVariables) {
            localVariables[fn] = {};
            for (const [varName, ti] of varMap) {
                localVariables[fn][varName] = ti;
            }
        }
        const functionParameters = {};
        for (const [fn, params] of lti.functionParameters) {
            functionParameters[fn] = params;
        }
        localTypeInfo[filePath] = { localVariables, functionParameters };
    }
    // fileHashes: Map<string, string> → Record<string, string>
    const fileHashesObj = {};
    for (const [k, v] of fileHashes) {
        fileHashesObj[k] = v;
    }
    return {
        version: exports.CACHE_VERSION,
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
function cacheToIndex(cache) {
    const index = (0, navigationIndex_1.createEmptyIndex)();
    // moduleRoots
    for (const [k, v] of Object.entries(cache.moduleRoots)) {
        index.moduleRoots.set(k, v);
    }
    // traitRoots
    for (const [k, v] of Object.entries(cache.traitRoots)) {
        index.traitRoots.set(k, v);
    }
    // derivationChain
    for (const [k, v] of Object.entries(cache.derivationChain)) {
        index.derivationChain.set(k, v);
    }
    // apiStructFields
    for (const [k, v] of Object.entries(cache.apiStructFields)) {
        index.apiStructFields.set(k, v);
    }
    // vtableAssignments
    for (const [apiType, fieldObj] of Object.entries(cache.vtableAssignments)) {
        const fieldMap = new Map();
        for (const [field, locs] of Object.entries(fieldObj)) {
            fieldMap.set(field, locs);
        }
        index.vtableAssignments.set(apiType, fieldMap);
    }
    // failureHandlerAssignments
    for (const [k, v] of Object.entries(cache.failureHandlerAssignments)) {
        index.failureHandlerAssignments.set(k, v);
    }
    // apiMemberRegistry
    for (const [k, v] of Object.entries(cache.apiMemberRegistry)) {
        index.apiMemberRegistry.set(k, v);
    }
    // functionDefinitions
    for (const [fnName, defs] of Object.entries(cache.functionDefinitions)) {
        const records = defs.map((d) => ({
            functionName: fnName,
            file: d.file,
            line: d.line,
            isStatic: d.isStatic,
        }));
        index.functionDefinitions.set(fnName, records);
    }
    // localTypeInfo
    for (const [filePath, clti] of Object.entries(cache.localTypeInfo)) {
        const localVariables = new Map();
        for (const [fn, varObj] of Object.entries(clti.localVariables)) {
            const varMap = new Map();
            for (const [varName, ti] of Object.entries(varObj)) {
                varMap.set(varName, ti);
            }
            localVariables.set(fn, varMap);
        }
        const functionParameters = new Map();
        for (const [fn, params] of Object.entries(clti.functionParameters)) {
            functionParameters.set(fn, params);
        }
        const lti = { localVariables, functionParameters };
        index.localTypeInfo.set(filePath, lti);
    }
    return index;
}
//# sourceMappingURL=cacheManager.js.map