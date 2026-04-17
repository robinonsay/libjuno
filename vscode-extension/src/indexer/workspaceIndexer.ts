// @{"req": ["REQ-VSCODE-021", "REQ-VSCODE-001"]}
import * as fs from "fs";
import * as path from "path";
import * as crypto from "crypto";
import {
    NavigationIndex,
    ParsedFile,
    ConcreteLocation,
    FunctionDefinitionRecord,
    LocalTypeInfo,
    FailureHandlerRecord,
} from "../parser/types";
import { parseFileWithDefs } from "../parser/visitor";
import { createEmptyIndex, removeFileRecords } from "./navigationIndex";
import { loadCache, saveCache, indexToCache, cacheToIndex } from "../cache/cacheManager";

/** C and C++ source file extensions to scan (REQ-VSCODE-021). */
export const C_FILE_EXTENSIONS = new Set([".c", ".h", ".cpp", ".hpp", ".hh", ".cc"]);

/** Positional vtable initializer deferred for cross-file resolution. */
interface DeferredPositional {
    apiType: string;
    initializers: string[]; // function names in positional order
    filePath: string;
    lines: number[];
}

/**
 * Scans and indexes all C/C++ files in a workspace, maintains an in-memory
 * NavigationIndex, and coordinates with the Cache Manager for persistence.
 */
export class WorkspaceIndexer {
    /** The populated NavigationIndex, shared by reference with resolvers. */
    public readonly index: NavigationIndex;

    private readonly workspaceRoot: string;
    private readonly excludedDirs: Set<string>;
    private readonly cachePath: string;
    private readonly fileHashes: Map<string, string>;

    constructor(workspaceRoot: string, excludedDirs: string[]) {
        this.workspaceRoot = workspaceRoot;
        this.excludedDirs = new Set(excludedDirs);
        this.index = createEmptyIndex();
        this.cachePath = path.join(workspaceRoot, ".libjuno", "navigation-cache.json");
        this.fileHashes = new Map<string, string>();
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
    public async reindexFile(filePath: string): Promise<void> {
        removeFileRecords(this.index, filePath);
        const relPath = path.relative(this.workspaceRoot, filePath);
        this.fileHashes.delete(relPath);
        await this.indexFile(filePath);
    }

    /**
     * Removes all records sourced from a file and clears its cached hash.
     *
     * @param filePath Absolute path of the file to remove.
     */
    public removeFile(filePath: string): void {
        removeFileRecords(this.index, filePath);
        const relPath = path.relative(this.workspaceRoot, filePath);
        this.fileHashes.delete(relPath);
    }

    /**
     * Performs a full workspace scan. Parses every C/C++ file that has changed
     * relative to the stored hashes, merges results into the index, then resolves
     * any deferred positional vtable initializers and saves the cache.
     */
    public async fullIndex(): Promise<void> {
        const allFiles = this.scanFiles(this.workspaceRoot);
        const deferred: DeferredPositional[] = [];

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
    public async loadFromCache(): Promise<boolean> {
        const cache = loadCache(this.cachePath);
        if (!cache) {
            return false;
        }

        // Populate index from cache
        const cachedIndex = cacheToIndex(cache);
        this.copyIndex(cachedIndex);

        // Restore cached file hashes
        for (const [relPath, hash] of Object.entries(cache.fileHashes)) {
            this.fileHashes.set(relPath, hash);
        }

        // Re-index any stale or new files
        const allFiles = this.scanFiles(this.workspaceRoot);
        const deferred: DeferredPositional[] = [];
        let anyChanged = false;

        for (const filePath of allFiles) {
            const relPath = path.relative(this.workspaceRoot, filePath);
            const currentHash = this.hashFile(filePath);
            if (currentHash === this.fileHashes.get(relPath)) {
                continue; // cached — skip
            }
            anyChanged = true;
            removeFileRecords(this.index, filePath);
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
    public async saveToCache(): Promise<void> {
        const cache = indexToCache(this.index, this.fileHashes);
        await saveCache(this.cachePath, cache);
    }

    // -----------------------------------------------------------------------
    // Private helpers
    // -----------------------------------------------------------------------

    /**
     * Parses one file and merges its results into the index.
     * Appends any unresolved positional vtable initializers to `deferred`.
     */
    private async indexFile(filePath: string, deferred?: DeferredPositional[]): Promise<void> {
        let text: string;
        try {
            text = await fs.promises.readFile(filePath, "utf8");
        } catch {
            return;
        }

        const hash = hashText(text);
        const relPath = path.relative(this.workspaceRoot, filePath);
        this.fileHashes.set(relPath, hash);

        const { parsed, functionDefs, apiMemberRegistry } = parseFileWithDefs(filePath, text);

        this.mergeInto(parsed, functionDefs, apiMemberRegistry, deferred);
    }

    /**
     * Merges parsed records into the NavigationIndex.
     */
    private mergeInto(
        parsed: ParsedFile,
        functionDefs: FunctionDefinitionRecord[],
        apiMemberRegistry: Map<string, string>,
        deferred?: DeferredPositional[]
    ): void {
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
            const loc: ConcreteLocation = {
                functionName: r.functionName,
                file: r.file,
                line: this.resolveDefinitionLine(r.functionName, r.file, functionDefs),
            };
            let fieldMap = idx.vtableAssignments.get(r.apiType);
            if (!fieldMap) {
                fieldMap = new Map<string, ConcreteLocation[]>();
                idx.vtableAssignments.set(r.apiType, fieldMap);
            }
            const existing = fieldMap.get(r.field) ?? [];
            existing.push(loc);
            fieldMap.set(r.field, existing);
        }

        // failureHandlerAssignments: append (with rootType resolution)
        for (const r of parsed.failureHandlerAssigns) {
            const resolvedRootType = this.resolveFailureHandlerRootType(r, parsed);
            if (!resolvedRootType) { continue; }
            const loc: ConcreteLocation = {
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
    private resolveDefinitionLine(
        functionName: string,
        file: string,
        functionDefs: FunctionDefinitionRecord[]
    ): number {
        // Prefer a same-file definition
        const samefile = functionDefs.find((d) => d.functionName === functionName && d.file === file);
        if (samefile) { return samefile.line; }
        // Fall back to any file-scope definition already in the index
        const indexDefs = this.index.functionDefinitions.get(functionName);
        if (indexDefs?.length) { return indexDefs[0].line; }
        return 0;
    }

    /**
     * Resolves the rootType for a FailureHandlerRecord using LocalTypeInfo
     * from the parsed file. Returns empty string if it cannot be resolved.
     */
    private resolveFailureHandlerRootType(r: FailureHandlerRecord, parsed: ParsedFile): string {
        if (r.rootType) { return r.rootType; }
        // Walk all function parameters and local vars looking for a type that
        // is a known module root type (ends in _ROOT_T or _T that matches moduleRoots/traitRoots).
        // The failure handler record doesn't carry the variable name, so we use
        // context heuristics: look for the first module root type found in any
        // local var or parameter of any function in the file.
        const lti: LocalTypeInfo = parsed.localTypeInfo;
        const knownRoots = new Set([...this.index.moduleRoots.keys(), ...this.index.traitRoots.keys()]);
        for (const varMap of lti.localVariables.values()) {
            for (const ti of varMap.values()) {
                if (knownRoots.has(ti.typeName)) { return ti.typeName; }
            }
        }
        for (const params of lti.functionParameters.values()) {
            for (const ti of params) {
                if (knownRoots.has(ti.typeName)) { return ti.typeName; }
            }
        }
        return "";
    }

    /**
     * Resolves deferred positional vtable initializers after all files have
     * been indexed, so cross-file API struct field orders are available.
     */
    private resolveDeferred(deferred: DeferredPositional[]): void {
        for (const d of deferred) {
            const fields = this.index.apiStructFields.get(d.apiType);
            if (!fields) {
                // Still unavailable — log and skip
                console.warn(`[LibJuno] Cannot resolve positional vtable for ${d.apiType}: API struct not found`);
                continue;
            }
            let fieldMap = this.index.vtableAssignments.get(d.apiType);
            if (!fieldMap) {
                fieldMap = new Map<string, ConcreteLocation[]>();
                this.index.vtableAssignments.set(d.apiType, fieldMap);
            }
            for (let i = 0; i < d.initializers.length && i < fields.length; i++) {
                const fnName = d.initializers[i];
                const line = d.lines[i] ?? 0;
                const loc: ConcreteLocation = {
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
    private scanFiles(dir: string): string[] {
        const results: string[] = [];
        let entries: fs.Dirent[];
        try {
            entries = fs.readdirSync(dir, { withFileTypes: true });
        } catch {
            return results;
        }
        for (const entry of entries) {
            const fullPath = path.join(dir, entry.name);
            if (entry.isDirectory()) {
                if (this.excludedDirs.has(entry.name)) { continue; }
                // Always skip the cache directory
                if (entry.name === ".libjuno") { continue; }
                results.push(...this.scanFiles(fullPath));
            } else if (entry.isFile()) {
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
    private hashFile(filePath: string): string {
        try {
            const content = fs.readFileSync(filePath);
            return crypto.createHash("sha256").update(content).digest("hex");
        } catch {
            return "";
        }
    }

    /**
     * Copies all entries from a source NavigationIndex into this.index.
     * Used when restoring from cache.
     */
    private copyIndex(src: NavigationIndex): void {
        const dst = this.index;
        for (const [k, v] of src.moduleRoots) { dst.moduleRoots.set(k, v); }
        for (const [k, v] of src.traitRoots) { dst.traitRoots.set(k, v); }
        for (const [k, v] of src.derivationChain) { dst.derivationChain.set(k, v); }
        for (const [k, v] of src.apiStructFields) { dst.apiStructFields.set(k, v); }
        for (const [k, v] of src.vtableAssignments) { dst.vtableAssignments.set(k, v); }
        for (const [k, v] of src.failureHandlerAssignments) { dst.failureHandlerAssignments.set(k, v); }
        for (const [k, v] of src.apiMemberRegistry) { dst.apiMemberRegistry.set(k, v); }
        for (const [k, v] of src.functionDefinitions) { dst.functionDefinitions.set(k, v); }
        for (const [k, v] of src.localTypeInfo) { dst.localTypeInfo.set(k, v); }
    }
}

// ---------------------------------------------------------------------------
// Module-level helpers
// ---------------------------------------------------------------------------

function hashText(text: string): string {
    return crypto.createHash("sha256").update(text, "utf8").digest("hex");
}
