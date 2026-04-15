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
exports.WorkspaceIndexer = void 0;
// @{"req": ["REQ-VSCODE-021", "REQ-VSCODE-001"]}
const fs = __importStar(require("fs"));
const path = __importStar(require("path"));
const crypto = __importStar(require("crypto"));
const visitor_1 = require("../parser/visitor");
const navigationIndex_1 = require("./navigationIndex");
const cacheManager_1 = require("../cache/cacheManager");
/** C and C++ source file extensions to scan (REQ-VSCODE-021). */
const C_FILE_EXTENSIONS = new Set([".c", ".h", ".cpp", ".hpp", ".hh", ".cc"]);
/**
 * Scans and indexes all C/C++ files in a workspace, maintains an in-memory
 * NavigationIndex, and coordinates with the Cache Manager for persistence.
 */
class WorkspaceIndexer {
    constructor(workspaceRoot, excludedDirs) {
        this.workspaceRoot = workspaceRoot;
        this.excludedDirs = new Set(excludedDirs);
        this.index = (0, navigationIndex_1.createEmptyIndex)();
        this.cachePath = path.join(workspaceRoot, ".libjuno", "navigation-cache.json");
        this.fileHashes = new Map();
    }
    // -----------------------------------------------------------------------
    // Public API
    // -----------------------------------------------------------------------
    /**
     * Re-indexes one file: removes its old records, parses it, and merges the
     * new records into the index. Updates the file hash.
     *
     * @param filePath Absolute path of the file to re-index.
     */
    async reindexFile(filePath) {
        (0, navigationIndex_1.removeFileRecords)(this.index, filePath);
        const relPath = path.relative(this.workspaceRoot, filePath);
        this.fileHashes.delete(relPath);
        await this.indexFile(filePath);
    }
    /**
     * Removes all records sourced from a file and clears its cached hash.
     *
     * @param filePath Absolute path of the file to remove.
     */
    removeFile(filePath) {
        (0, navigationIndex_1.removeFileRecords)(this.index, filePath);
        const relPath = path.relative(this.workspaceRoot, filePath);
        this.fileHashes.delete(relPath);
    }
    /**
     * Performs a full workspace scan. Parses every C/C++ file that has changed
     * relative to the stored hashes, merges results into the index, then resolves
     * any deferred positional vtable initializers and saves the cache.
     */
    async fullIndex() {
        const allFiles = this.scanFiles(this.workspaceRoot);
        const deferred = [];
        for (const filePath of allFiles) {
            await this.indexFile(filePath, deferred);
        }
        // Cross-file deferred positional initializer resolution
        this.resolveDeferred(deferred);
        await this.saveToCache();
    }
    /**
     * Loads the navigation index from the JSON cache.
     * Re-indexes any files whose SHA-256 hash has changed.
     *
     * @returns true if a usable cache was found and loaded; false if a full scan is needed.
     */
    async loadFromCache() {
        const cache = (0, cacheManager_1.loadCache)(this.cachePath);
        if (!cache) {
            return false;
        }
        // Populate index from cache
        const cachedIndex = (0, cacheManager_1.cacheToIndex)(cache);
        this.copyIndex(cachedIndex);
        // Restore cached file hashes
        for (const [relPath, hash] of Object.entries(cache.fileHashes)) {
            this.fileHashes.set(relPath, hash);
        }
        // Re-index any stale or new files
        const allFiles = this.scanFiles(this.workspaceRoot);
        const deferred = [];
        let anyChanged = false;
        for (const filePath of allFiles) {
            const relPath = path.relative(this.workspaceRoot, filePath);
            const currentHash = this.hashFile(filePath);
            if (currentHash === this.fileHashes.get(relPath)) {
                continue; // cached — skip
            }
            anyChanged = true;
            (0, navigationIndex_1.removeFileRecords)(this.index, filePath);
            await this.indexFile(filePath, deferred);
        }
        if (anyChanged) {
            this.resolveDeferred(deferred);
            await this.saveToCache();
        }
        return true;
    }
    /**
     * Serializes the current index and file hashes to the JSON cache.
     */
    async saveToCache() {
        const cache = (0, cacheManager_1.indexToCache)(this.index, this.fileHashes);
        await (0, cacheManager_1.saveCache)(this.cachePath, cache);
    }
    // -----------------------------------------------------------------------
    // Private helpers
    // -----------------------------------------------------------------------
    /**
     * Parses one file and merges its results into the index.
     * Appends any unresolved positional vtable initializers to `deferred`.
     */
    async indexFile(filePath, deferred) {
        let text;
        try {
            text = await fs.promises.readFile(filePath, "utf8");
        }
        catch {
            return;
        }
        const hash = hashText(text);
        const relPath = path.relative(this.workspaceRoot, filePath);
        this.fileHashes.set(relPath, hash);
        const { parsed, functionDefs, apiMemberRegistry } = (0, visitor_1.parseFileWithDefs)(filePath, text);
        this.mergeInto(parsed, functionDefs, apiMemberRegistry, deferred);
    }
    /**
     * Merges parsed records into the NavigationIndex.
     */
    mergeInto(parsed, functionDefs, apiMemberRegistry, deferred) {
        const idx = this.index;
        // moduleRoots: overwrite (one authoritative definition per type)
        for (const r of parsed.moduleRoots) {
            idx.moduleRoots.set(r.rootType, r.apiType);
        }
        // traitRoots: overwrite
        for (const r of parsed.traitRoots) {
            idx.traitRoots.set(r.rootType, r.apiType);
        }
        // derivationChain: overwrite
        for (const r of parsed.derivations) {
            idx.derivationChain.set(r.derivedType, r.rootType);
        }
        // apiStructFields: overwrite
        for (const r of parsed.apiStructDefinitions) {
            idx.apiStructFields.set(r.apiType, r.fields);
        }
        // vtableAssignments: append
        for (const r of parsed.vtableAssignments) {
            const loc = {
                functionName: r.functionName,
                file: r.file,
                line: this.resolveDefinitionLine(r.functionName, r.file, functionDefs),
            };
            let fieldMap = idx.vtableAssignments.get(r.apiType);
            if (!fieldMap) {
                fieldMap = new Map();
                idx.vtableAssignments.set(r.apiType, fieldMap);
            }
            const existing = fieldMap.get(r.field) ?? [];
            existing.push(loc);
            fieldMap.set(r.field, existing);
        }
        // failureHandlerAssignments: append (with rootType resolution)
        for (const r of parsed.failureHandlerAssigns) {
            const resolvedRootType = this.resolveFailureHandlerRootType(r, parsed);
            if (!resolvedRootType) {
                continue;
            }
            const loc = {
                functionName: r.functionName,
                file: r.file,
                line: this.resolveDefinitionLine(r.functionName, r.file, functionDefs),
            };
            const existing = idx.failureHandlerAssignments.get(resolvedRootType) ?? [];
            existing.push(loc);
            idx.failureHandlerAssignments.set(resolvedRootType, existing);
        }
        // apiMemberRegistry: overwrite
        for (const [memberName, apiType] of apiMemberRegistry) {
            idx.apiMemberRegistry.set(memberName, apiType);
        }
        // functionDefinitions: append
        for (const def of functionDefs) {
            const existing = idx.functionDefinitions.get(def.functionName) ?? [];
            existing.push(def);
            idx.functionDefinitions.set(def.functionName, existing);
        }
        // localTypeInfo: keyed by file — set (one entry per file)
        idx.localTypeInfo.set(parsed.filePath, parsed.localTypeInfo);
    }
    /**
     * Resolves the definition line for a function name by looking it up in the
     * functionDefs collected from the same parse pass.
     * Falls back to the assignment line (0) if not found.
     */
    resolveDefinitionLine(functionName, file, functionDefs) {
        // Prefer a same-file definition
        const samefile = functionDefs.find((d) => d.functionName === functionName && d.file === file);
        if (samefile) {
            return samefile.line;
        }
        // Fall back to any file-scope definition already in the index
        const indexDefs = this.index.functionDefinitions.get(functionName);
        if (indexDefs?.length) {
            return indexDefs[0].line;
        }
        return 0;
    }
    /**
     * Resolves the rootType for a FailureHandlerRecord using LocalTypeInfo
     * from the parsed file. Returns empty string if it cannot be resolved.
     */
    resolveFailureHandlerRootType(r, parsed) {
        if (r.rootType) {
            return r.rootType;
        }
        // Walk all function parameters and local vars looking for a type that
        // is a known module root type (ends in _ROOT_T or _T that matches moduleRoots/traitRoots).
        // The failure handler record doesn't carry the variable name, so we use
        // context heuristics: look for the first module root type found in any
        // local var or parameter of any function in the file.
        const lti = parsed.localTypeInfo;
        const knownRoots = new Set([...this.index.moduleRoots.keys(), ...this.index.traitRoots.keys()]);
        for (const varMap of lti.localVariables.values()) {
            for (const ti of varMap.values()) {
                if (knownRoots.has(ti.typeName)) {
                    return ti.typeName;
                }
            }
        }
        for (const params of lti.functionParameters.values()) {
            for (const ti of params) {
                if (knownRoots.has(ti.typeName)) {
                    return ti.typeName;
                }
            }
        }
        return "";
    }
    /**
     * Resolves deferred positional vtable initializers after all files have
     * been indexed, so cross-file API struct field orders are available.
     */
    resolveDeferred(deferred) {
        for (const d of deferred) {
            const fields = this.index.apiStructFields.get(d.apiType);
            if (!fields) {
                // Still unavailable — log and skip
                console.warn(`[LibJuno] Cannot resolve positional vtable for ${d.apiType}: API struct not found`);
                continue;
            }
            let fieldMap = this.index.vtableAssignments.get(d.apiType);
            if (!fieldMap) {
                fieldMap = new Map();
                this.index.vtableAssignments.set(d.apiType, fieldMap);
            }
            for (let i = 0; i < d.initializers.length && i < fields.length; i++) {
                const fnName = d.initializers[i];
                const line = d.lines[i] ?? 0;
                const loc = {
                    functionName: fnName,
                    file: d.filePath,
                    line: this.resolveDefinitionLine(fnName, d.filePath, []),
                };
                const existing = fieldMap.get(fields[i]) ?? [];
                // Avoid duplicates
                if (!existing.some((e) => e.functionName === fnName && e.file === d.filePath && e.line === line)) {
                    existing.push(loc);
                }
                fieldMap.set(fields[i], existing);
            }
        }
    }
    /**
     * Recursively scans a directory for C/C++ files, excluding configured dirs.
     * @param dir Absolute path to the directory to scan.
     * @returns   Array of absolute file paths matching C_FILE_EXTENSIONS.
     */
    scanFiles(dir) {
        const results = [];
        let entries;
        try {
            entries = fs.readdirSync(dir, { withFileTypes: true });
        }
        catch {
            return results;
        }
        for (const entry of entries) {
            const fullPath = path.join(dir, entry.name);
            if (entry.isDirectory()) {
                if (this.excludedDirs.has(entry.name)) {
                    continue;
                }
                // Always skip the cache directory
                if (entry.name === ".libjuno") {
                    continue;
                }
                results.push(...this.scanFiles(fullPath));
            }
            else if (entry.isFile()) {
                const ext = path.extname(entry.name);
                if (C_FILE_EXTENSIONS.has(ext)) {
                    results.push(fullPath);
                }
            }
        }
        return results;
    }
    /**
     * Computes the SHA-256 hex digest of a file's contents.
     * Returns empty string on read error.
     */
    hashFile(filePath) {
        try {
            const content = fs.readFileSync(filePath);
            return crypto.createHash("sha256").update(content).digest("hex");
        }
        catch {
            return "";
        }
    }
    /**
     * Copies all entries from a source NavigationIndex into this.index.
     * Used when restoring from cache.
     */
    copyIndex(src) {
        const dst = this.index;
        for (const [k, v] of src.moduleRoots) {
            dst.moduleRoots.set(k, v);
        }
        for (const [k, v] of src.traitRoots) {
            dst.traitRoots.set(k, v);
        }
        for (const [k, v] of src.derivationChain) {
            dst.derivationChain.set(k, v);
        }
        for (const [k, v] of src.apiStructFields) {
            dst.apiStructFields.set(k, v);
        }
        for (const [k, v] of src.vtableAssignments) {
            dst.vtableAssignments.set(k, v);
        }
        for (const [k, v] of src.failureHandlerAssignments) {
            dst.failureHandlerAssignments.set(k, v);
        }
        for (const [k, v] of src.apiMemberRegistry) {
            dst.apiMemberRegistry.set(k, v);
        }
        for (const [k, v] of src.functionDefinitions) {
            dst.functionDefinitions.set(k, v);
        }
        for (const [k, v] of src.localTypeInfo) {
            dst.localTypeInfo.set(k, v);
        }
    }
}
exports.WorkspaceIndexer = WorkspaceIndexer;
// ---------------------------------------------------------------------------
// Module-level helpers
// ---------------------------------------------------------------------------
function hashText(text) {
    return crypto.createHash("sha256").update(text, "utf8").digest("hex");
}
//# sourceMappingURL=workspaceIndexer.js.map