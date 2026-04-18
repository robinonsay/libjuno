> Part of: [Software Design Document](index.md) — Sections 8-9: Error Handling and Cache Design

// @{"design": ["REQ-VSCODE-004", "REQ-VSCODE-013"]}
## 8. Error Handling Design

### 8.1 Resolution Failure (REQ-VSCODE-004, REQ-VSCODE-013)

When a resolution fails:

1. **Status bar message (primary, non-intrusive):**  
   Display a temporary status bar item with the message:  
   `$(warning) LibJuno: Could not resolve implementation — ${errorMsg}`  
   The item auto-clears after 5 seconds.

2. **Information message (optional, on repeated failure):**  
   If the user triggers resolution and it fails again within 10 seconds, show:  
   `vscode.window.showInformationMessage(errorMsg, "Show Details")`  
   Selecting "Show Details" opens the Output Channel with full diagnostic information (matched patterns, index state for the relevant type).

3. **No modal dialogs.** `showErrorMessage` (which produces a modal in some contexts) is never used for resolution failures.

### 8.2 Indexing Errors

If a file cannot be read during indexing, it is skipped with a warning written to the extension's Output Channel (`LibJuno`). The error does not surface to the user unless all indexing fails.

### 8.3 MCP Server Errors

MCP tool errors are returned as standard MCP error responses (HTTP 200 with `isError: true` in the result), not HTTP error codes. The `error` field in the output schema carries the human-readable explanation.

---

// @{"design": ["REQ-VSCODE-001"]}
## 9. Cache Design

### 9.1 Cache File Location

`.libjuno/navigation-cache.json` in the workspace root folder. If the workspace has multiple root folders, one cache file is created per root folder.

### 9.2 Cache Validity

The cache is considered valid if:
1. `version` matches the extension's current cache format version string.
2. At least one file was indexed (non-empty `fileHashes`).

On load, files whose hash in `fileHashes` does not match the current on-disk hash are re-indexed. Files present on disk but absent from `fileHashes` are indexed as new. Files present in `fileHashes` but absent from disk are removed from the index.

### 9.3 File Watcher Invalidation

The extension registers a `vscode.FileSystemWatcher` for `**/*.{c,h,cpp,hpp,hh,cc}` (excluding the excluded directories). On `onDidChange`, `onDidCreate`, and `onDidDelete` events:

- `onDidChange` or `onDidCreate`: Re-parse the file, remove the old records for that file from the index, merge in the new records, update `fileHashes[filePath]`, schedule a debounced cache write (500 ms delay to batch rapid saves).
- `onDidDelete`: Remove all index records sourced from that file, remove `fileHashes[filePath]`, schedule debounced cache write.

### 9.4 Full Re-index

Triggered by:
- Extension activation when cache is missing, version-mismatched, or explicitly invalidated.
- The `libjuno.reindexWorkspace` command.

Full re-index clears the in-memory index and rebuilds it from scratch, then writes the cache.

### 9.5 Cache Write Strategy

Cache writes are debounced (500 ms) to avoid excessive disk I/O during bulk file events (e.g., `git checkout`). The extension uses `fs.writeFile` with a temporary file and rename to ensure atomic writes, preventing a corrupt cache if the extension process is killed during a write.
