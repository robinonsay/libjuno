> Part of: [Test Case Specification](index.md) — Sections 15-16: MCP Server and Cache Tests

## Section 15: MCP Server Tests

These test cases verify the embedded MCP server described in design Section 7.
They cover REQ-VSCODE-017, REQ-VSCODE-018, REQ-VSCODE-019, and REQ-VSCODE-020.

**Test double approach:** Start the MCP server in-process during `beforeEach` with an injected
`NavigationIndex` stub and a configurable port. Use Node.js `http.request` or `fetch` to send
requests. Shut down the server in `afterEach`.

---

### Test Case ID: TC-MCP-001
**Scenario:** MCP server starts on extension activation  
**Setup:**
- Spy on `McpServer.start()` (or equivalent).
- Call `activate(context)` with a mock `ExtensionContext`.

**Expected result:**
- `McpServer.start()` is called before `activate()` returns.
- The server is listening on `127.0.0.1` at the configured port (default 6543).
- A subsequent HTTP GET to `http://127.0.0.1:6543/mcp` returns HTTP 200 or a valid MCP
  response (not a connection-refused error).

**Requirement:** REQ-VSCODE-017

---

### Test Case ID: TC-MCP-002
**Scenario:** `resolve_vtable_call` tool is registered with correct input/output schema  
**Setup:**
- Query the MCP server's tool list endpoint (or inspect the registered tool metadata directly
  from `McpServer`'s tool registry after construction).

**Expected result:**
- A tool named `"resolve_vtable_call"` is present.
- Its input schema requires `file` (string), `line` (integer), `column` (integer).
- Its output schema has `found` (boolean), `locations` (array), `error` (string, optional).

**Requirement:** REQ-VSCODE-018

---

### Test Case ID: TC-MCP-003
**Scenario:** `resolve_failure_handler` tool is registered with correct input/output schema  
**Setup:**
- Inspect the MCP server's tool registry after construction.

**Expected result:**
- A tool named `"resolve_failure_handler"` is present.
- Its input schema is identical to `resolve_vtable_call` (file, line, column).
- Its output schema is identical to `resolve_vtable_call` (found, locations, error).

**Requirement:** REQ-VSCODE-019

---

### Test Case ID: TC-MCP-004
**Scenario:** `resolve_vtable_call` with valid input returns `found: true` with correct locations  
**Setup:**
- Pre-populate `NavigationIndex`:
  ```typescript
  moduleRoots: { "JUNO_DS_HEAP_ROOT_T": "JUNO_DS_HEAP_API_T" }
  vtableAssignments: {
    "JUNO_DS_HEAP_API_T": {
      "Insert": [{ functionName: "JunoDs_Heap_Insert", file: "src/juno_heap.c", line: 259 }]
    }
  }
  ```
- Stub the source-text getter so that the call site line resolves with chain-walk (Category 1) using the
  above index.
- POST to `resolve_vtable_call` with `{ "file": "src/main.c", "line": 42, "column": 15 }`
  where the stub line at (file, 42) is:
  `    tStatus = ptHeap->ptApi->Insert(ptHeap, tValue);`
  and LocalTypeInfo provides `JUNO_DS_HEAP_ROOT_T *ptHeap`.

**Expected result:**
```json
{
  "found": true,
  "locations": [
    { "functionName": "JunoDs_Heap_Insert", "file": "src/juno_heap.c", "line": 259 }
  ]
}
```
**HTTP status:** 200

**Requirement:** REQ-VSCODE-018

---

### Test Case ID: TC-MCP-005
**Scenario:** `resolve_vtable_call` with no-match input returns `found: false` with error message  
**Setup:**
- `NavigationIndex` is empty (no API types registered).
- POST `{ "file": "src/main.c", "line": 1, "column": 5 }` where the stub line contains
  `    tStatus = ptFoo->ptApi->UnknownField(ptFoo);` with no matching type in LocalTypeInfo lookup.

**Expected result:**
```json
{
  "found": false,
  "locations": [],
  "error": "No API type contains field 'UnknownField'."
}
```
**HTTP status:** 200

**Requirement:** REQ-VSCODE-018

---

### Test Case ID: TC-MCP-006
**Scenario:** `resolve_failure_handler` with valid input returns `found: true`  
**Setup:**
- Pre-populate `NavigationIndex`:
  ```typescript
  failureHandlerAssignments: {
    "JUNO_DS_HEAP_ROOT_T": [
      { functionName: "MyFailureHandler", file: "examples/example.c", line: 42 }
    ]
  }
  ```
- Stub the source-text getter so the call-site line is:
  `    ptHeap->_pfcnFailureHandler = MyFailureHandler;`
  and LocalTypeInfo provides `JUNO_DS_HEAP_ROOT_T *ptHeap`.
- POST `{ "file": "src/init.c", "line": 99, "column": 12 }`.

**Expected result:**
```json
{
  "found": true,
  "locations": [
    { "functionName": "MyFailureHandler", "file": "examples/example.c", "line": 42 }
  ]
}
```

**Requirement:** REQ-VSCODE-019

---

### Test Case ID: TC-MCP-007
**Scenario:** `resolve_failure_handler` with invalid input (no handler registered) returns `found: false`  
**Setup:**
- `failureHandlerAssignments` contains no entry for `"JUNO_APP_ROOT_T"`.
- Stub line: `    ptApp->_pfcnFailureHandler = SomeHandler;` with LocalTypeInfo providing
  `JUNO_APP_ROOT_T *ptApp`.
- POST `{ "file": "src/unknown.c", "line": 10, "column": 8 }`.

**Expected result:**
```json
{
  "found": false,
  "locations": [],
  "error": "No failure handler registered for 'JUNO_APP_ROOT_T'."
}
```

**Requirement:** REQ-VSCODE-019

---

### Test Case ID: TC-MCP-008
**Scenario:** `.libjuno/mcp.json` discovery file is written on activation  
**Setup:**
- Stub `fs.writeFile` (or `fs.promises.writeFile`) as a jest spy.
- Call `activate(context)` with a workspace root of `/workspace`.

**Expected result:**
- `fs.writeFile` (or equivalent) is called with a path ending in `.libjuno/mcp.json`.
- The content written is valid JSON containing:
  ```json
  {
    "mcpServers": {
      "libjuno": {
        "url": "http://127.0.0.1:6543/mcp"
      }
    }
  }
  ```

**Requirement:** REQ-VSCODE-017

---

### Test Case ID: TC-MCP-009
**Scenario:** MCP server binds to `127.0.0.1` only (not externally accessible)  
**Setup:**
- Start `McpServer` with default configuration.
- Query the bound address from the underlying `http.Server` (`server.address()`).

**Expected result:**
- `server.address().address` equals `"127.0.0.1"` (not `"0.0.0.0"` or `"::"` or `"*"`).

**Requirement:** REQ-VSCODE-017, REQ-VSCODE-020

---

### Test Case ID: TC-MCP-010
**Scenario:** MCP error responses use `isError: true` in the MCP result, not HTTP error codes  
**Setup:**
- Trigger any resolution failure through the MCP server (as in TC-MCP-005).

**Expected result:**
- HTTP response status is `200 OK`.
- The MCP protocol wrapper includes `"isError": true` at the result level.
- No HTTP 4xx or 5xx status codes are used for application-level errors.

**Requirement:** REQ-VSCODE-017

---

### Test Case ID: TC-MCP-011
**Scenario:** MCP tools work without any VSCode UI (headless/AI-only mode)  
**Setup:**
- Instantiate `McpServer` directly with an injected `NavigationIndex` stub,
  WITHOUT calling `activate()` and WITHOUT a VSCode window context.
- Start the server; send a `resolve_vtable_call` request.

**Expected result:**
- Server responds correctly (found or not-found) without referencing any
  `vscode.window.*` API.
- No `vscode` API calls are made inside `McpServer.handleResolveVtableCall()`.

**Requirement:** REQ-VSCODE-017, REQ-VSCODE-020

---

### Test Case ID: TC-MCP-012
**Scenario:** MCP implementation does not use any platform-specific AI API  
**Setup:**
- Inspect `vscode-extension/src/mcp/mcpServer.ts` for any imports from:
  - `@github/copilot-*`
  - `@anthropic-ai/*`
  - `openai`
  - Any other AI-provider-specific SDK package.
- This is a static inspection / code review test case.

**Expected result:**
- No imports from AI-provider-specific packages are found in any file under
  `vscode-extension/src/mcp/`.
- The only AI-interface mechanism is the MCP HTTP protocol itself
  (standard HTTP + JSON, no vendor SDK).

**Requirement:** REQ-VSCODE-020

---

## Section 16: Cache Tests

These test cases verify the Cache Manager described in design Section 9.
Cache behavior is related to REQ-VSCODE-003 (performance: avoiding full re-scan on every activation)
and REQ-VSCODE-001 (activation behavior).

**Test double approach:** Inject a stub file-system adapter into `CacheManager` so that
`readFile`, `writeFile`, `rename`, and `mkdir` calls are intercepted. Use in-memory maps
to simulate file content and hashes.

---

### Test Case ID: TC-CACHE-001
**Scenario:** Cache file is created at `.libjuno/navigation-cache.json` on first index  
**Setup:**
- Stub file system: no `.libjuno/navigation-cache.json` exists initially.
- Run `WorkspaceIndexer.index()` on a workspace containing two stub C files.

**Expected result:**
- `fs.writeFile` (or `fs.promises.writeFile`) is called with a path ending in
  `.libjuno/navigation-cache.json` (after the temp-file-and-rename step, if applicable).
- The written content is valid JSON matching the cache schema (has `version`, `fileHashes`,
  `moduleRoots`, etc.).

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-CACHE-002
**Scenario:** Cache is loaded on activation when present and valid  
**Setup:**
- Stub file system returns a pre-built cache JSON with `version = "1"` and `fileHashes`
  matching the current stub file content hashes.
- Spy on `CParser.parse()`.
- Call `WorkspaceIndexer.index()`.

**Expected result:**
- `CParser.parse()` is NOT called for any file whose hash matches the cache.
- The `NavigationIndex` is populated from the cache data, not from re-parsing.

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-CACHE-003
**Scenario:** Stale file (hash mismatch) triggers re-index of that file only  
**Setup:**
- Cache contains two files: `src/foo.c` (hash `"aaa"`) and `src/bar.c` (hash `"bbb"`).
- Stub file system: `src/foo.c` now has hash `"ccc"` (changed); `src/bar.c` still has `"bbb"`.
- Spy on `CParser.parse()`.

**Expected result:**
- `CParser.parse()` is called exactly once, for `src/foo.c`.
- `CParser.parse()` is NOT called for `src/bar.c`.
- After indexing, `fileHashes["src/foo.c"]` is updated to `"ccc"` in the cache.

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-CACHE-004
**Scenario:** New file (not in cache) is indexed and added to cache  
**Setup:**
- Cache contains only `src/foo.c`.
- Stub file system also contains `src/new_module.c` (not in cache).
- Spy on `CParser.parse()`.

**Expected result:**
- `CParser.parse()` is called for `src/new_module.c`.
- After indexing, `fileHashes["src/new_module.c"]` appears in the updated cache.

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-CACHE-005
**Scenario:** Deleted file is removed from cache and index  
**Setup:**
- Cache contains `src/foo.c` and `src/deleted.c`.
- Stub file system: `src/deleted.c` no longer exists (file not found on read).
- Run `WorkspaceIndexer.index()`.

**Expected result:**
- `fileHashes["src/deleted.c"]` is absent from the updated cache.
- Any index records sourced from `src/deleted.c` are removed from `NavigationIndex`.

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-CACHE-006
**Scenario:** `version` field mismatch triggers full re-index  
**Setup:**
- Cache has `"version": "0"` (old format); current extension expects `"version": "1"`.
- Spy on `CParser.parse()`.
- Stub file system contains two files.

**Expected result:**
- `CParser.parse()` is called for ALL files (full re-index), not only changed ones.
- The rewritten cache has `"version": "1"`.

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-CACHE-007
**Scenario:** `FileSystemWatcher` triggers re-index on `.c`, `.h`, `.cpp` file changes  
**Setup:**
- Register a `FileSystemWatcher` stub that can fire `onDidChange`, `onDidCreate`, `onDidDelete`
  events programmatically.
- Spy on `WorkspaceIndexer._reindexFile()` (or equivalent single-file re-index method).
- Fire `onDidChange` for `src/foo.c`.

**Expected result:**
- `_reindexFile("src/foo.c")` is called within the debounce window.
- `NavigationIndex` records from `src/foo.c` are updated.

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-CACHE-008
**Scenario:** Debounced write: rapid saves produce only one cache write  
**Setup:**
- Use Jest fake timers.
- Fire `onDidChange` for `src/foo.c` five times in 100 ms intervals (all within a 500 ms window).
- Spy on `CacheManager.write()`.
- Advance timers by 600 ms (past the 500 ms debounce threshold).

**Expected result:**
- `CacheManager.write()` is called exactly once (debounced, not five times).

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-CACHE-009
**Scenario:** Cache write is atomic (temp file + rename)  
**Setup:**
- Spy on `fs.writeFile` and `fs.rename` (or `fs.promises` equivalents).
- Trigger a cache write.

**Expected result:**
- `fs.writeFile` is called with a path that is NOT the final cache path
  (e.g., ends with `.tmp` or a random suffix).
- `fs.rename` is subsequently called, moving the temp file to `.libjuno/navigation-cache.json`.
- `fs.writeFile` is NOT called directly with the final cache path name.

**Requirement:** REQ-VSCODE-001

---
