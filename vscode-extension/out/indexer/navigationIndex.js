"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.createEmptyIndex = createEmptyIndex;
exports.clearIndex = clearIndex;
exports.removeFileRecords = removeFileRecords;
/**
 * Creates a new, empty NavigationIndex with all Maps initialized.
 * @returns A NavigationIndex with all fields set to empty Maps.
 */
function createEmptyIndex() {
    return {
        moduleRoots: new Map(),
        traitRoots: new Map(),
        derivationChain: new Map(),
        apiStructFields: new Map(),
        vtableAssignments: new Map(),
        failureHandlerAssignments: new Map(),
        apiMemberRegistry: new Map(),
        functionDefinitions: new Map(),
        localTypeInfo: new Map(),
    };
}
/**
 * Clears all entries from every Map in the index, resetting it to an empty state.
 * @param index The NavigationIndex to clear.
 */
function clearIndex(index) {
    index.moduleRoots.clear();
    index.traitRoots.clear();
    index.derivationChain.clear();
    index.apiStructFields.clear();
    index.vtableAssignments.clear();
    index.failureHandlerAssignments.clear();
    index.apiMemberRegistry.clear();
    index.functionDefinitions.clear();
    index.localTypeInfo.clear();
}
/**
 * Removes all records in the index that were sourced from the specified file path.
 * Used when a file is deleted or before re-indexing a changed file.
 * @param index    The NavigationIndex to update.
 * @param filePath Absolute path of the file whose records should be removed.
 */
function removeFileRecords(index, filePath) {
    // moduleRoots: rootType → apiType — remove entries whose file matches
    // (not directly stored in the flat map; we rely on re-indexing to overwrite)
    // moduleRoots is a Map<string, string> with no file field, so we cannot
    // filter by file directly. The design stores only the mapping without a file
    // pointer. We remove by scanning vtableAssignments (which have file),
    // and for the simple maps we leave stale entries to be overwritten on re-index.
    //
    // However, for Maps that store arrays of records with a `file` field we
    // CAN prune precisely.
    // vtableAssignments: Map<apiType, Map<fieldName, ConcreteLocation[]>>
    for (const [apiType, fieldMap] of index.vtableAssignments) {
        for (const [field, locs] of fieldMap) {
            const filtered = locs.filter((l) => l.file !== filePath);
            if (filtered.length === 0) {
                fieldMap.delete(field);
            }
            else {
                fieldMap.set(field, filtered);
            }
        }
        if (fieldMap.size === 0) {
            index.vtableAssignments.delete(apiType);
        }
    }
    // failureHandlerAssignments: Map<rootType, ConcreteLocation[]>
    for (const [rootType, locs] of index.failureHandlerAssignments) {
        const filtered = locs.filter((l) => l.file !== filePath);
        if (filtered.length === 0) {
            index.failureHandlerAssignments.delete(rootType);
        }
        else {
            index.failureHandlerAssignments.set(rootType, filtered);
        }
    }
    // functionDefinitions: Map<functionName, FunctionDefinitionRecord[]>
    for (const [fnName, defs] of index.functionDefinitions) {
        const filtered = defs.filter((d) => d.file !== filePath);
        if (filtered.length === 0) {
            index.functionDefinitions.delete(fnName);
        }
        else {
            index.functionDefinitions.set(fnName, filtered);
        }
    }
    // localTypeInfo: keyed by file path — direct removal
    index.localTypeInfo.delete(filePath);
    // The flat string→string maps (moduleRoots, traitRoots, derivationChain,
    // apiStructFields, apiMemberRegistry) do not carry a file pointer, so stale
    // entries will be overwritten when the file is re-indexed. No action needed.
}
//# sourceMappingURL=navigationIndex.js.map