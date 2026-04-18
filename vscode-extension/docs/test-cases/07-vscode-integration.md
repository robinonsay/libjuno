> Part of: [Test Case Specification](index.md) — Sections 13-14: VSCode Integration and Error UX Tests

## Section 13: VSCode Integration Tests

These test cases verify the extension's integration with the VSCode Extension API: activation,
provider registration, command registration, and passthrough behavior on non-LibJuno call sites.
They correspond to REQ-VSCODE-001, REQ-VSCODE-002, and REQ-VSCODE-007.

**Test double approach:** Use a VSCode API stub that captures `registerDefinitionProvider`,
`registerCommand`, and `languages` calls via jest spy functions injected into the
`JunoDefinitionProvider` and command handlers under test.

---

### Test Case ID: TC-VSC-001
**Scenario:** Extension activates when a C file is opened  
**Setup:**
- Stub `vscode.workspace.findFiles` to return a list of `.c` and `.h` files.
- Stub `vscode.languages.registerDefinitionProvider` as a jest spy.
- Stub `vscode.commands.registerCommand` as a jest spy.
- Call the exported `activate(context)` function with a mock `ExtensionContext`.

**Expected result:**
- `activate()` completes without throwing.
- `vscode.languages.registerDefinitionProvider` is called at least once before `activate()` returns.
- `vscode.commands.registerCommand` is called at least twice (for the two registered commands).

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-VSC-002
**Scenario:** `JunoDefinitionProvider` is registered for both `c` and `cpp` language IDs  
**Setup:**
- Capture the first argument to `vscode.languages.registerDefinitionProvider` during activation.

**Expected result:**
- The document selector argument contains an entry with `{ language: 'c' }`.
- The document selector argument contains an entry with `{ language: 'cpp' }`.
- The second argument is an instance implementing `provideDefinition`.

**Requirement:** REQ-VSCODE-001, REQ-VSCODE-007

---

### Test Case ID: TC-VSC-003
**Scenario:** F12 / Ctrl+Click on a vtable call site triggers `JunoDefinitionProvider` and returns a location  
**Setup:**
- Pre-populate `NavigationIndex` with sufficient data to resolve `ptTime->ptApi->Now(ptTime)`:
  ```typescript
  moduleRoots: { "JUNO_TIME_ROOT_T": "JUNO_TIME_API_T" }
  vtableAssignments: {
    "JUNO_TIME_API_T": {
      "Now": [{ functionName: "Now", file: "examples/example_project/main.c", line: 110 }]
    }
  }
  ```
- Stub `provideDefinition` context: document line =
  `    JUNO_TIMESTAMP_RESULT_T tTimestampResult = ptTime->ptApi->Now(ptTime);`
  with surrounding lines providing `const JUNO_TIME_ROOT_T *ptTime = ptEngineApp->ptTime;`.
- Position: line containing `Now`, column on `Now`.

**Expected result:**
- `provideDefinition` returns a non-null `LocationLink[]` with one entry.
- The entry's `targetUri` resolves to `examples/example_project/main.c`.
- The entry's `targetRange` starts at line 110 (0-based: 109).

**Requirement:** REQ-VSCODE-002, REQ-VSCODE-007

---

### Test Case ID: TC-VSC-004
**Scenario:** F12 on a non-vtable call falls through (provider returns `undefined`)  
**Setup:**
- Document line: `    printf("hello, world\n");`
- Position: column on `printf`.
- `NavigationIndex` may be empty.

**Expected result:**
- `provideDefinition` returns `undefined` (not an error, not an empty array).
- No status bar message is shown for this case (non-libjuno call is silently ignored).

**Note:** Returning `undefined` allows VSCode to fall through to the default C/C++ language
server provider, preserving native Go to Definition for ordinary function calls.

**Requirement:** REQ-VSCODE-007

---

### Test Case ID: TC-VSC-005
**Scenario:** `libjuno.goToImplementation` command is registered  
**Setup:**
- Capture all `registerCommand` calls during `activate()`.

**Expected result:**
- At least one call has command ID `"libjuno.goToImplementation"`.
- The registered handler is a function.

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-VSC-006
**Scenario:** `libjuno.reindexWorkspace` command clears cache and rebuilds the index  
**Setup:**
- Spy on `WorkspaceIndexer.reindex()` and `CacheManager.clear()`.
- Invoke the `libjuno.reindexWorkspace` command handler directly.

**Expected result:**
- `CacheManager.clear()` is called (or the in-memory index is reset) before indexing begins.
- `WorkspaceIndexer.reindex()` (or equivalent full-scan entry point) is called exactly once.
- After the handler resolves, `NavigationIndex` is non-empty (assuming stub files were provided).

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-VSC-007
**Scenario:** `provideDefinition` returns `undefined` and does NOT throw on a line that matches
`->fieldName(` syntax but whose type cannot be determined by any strategy  
**Setup:**
- Document line: `    tStatus = pUnknown->ptApi->Mystery(pUnknown);`
- Index state: no API type contains field `"Mystery"`.
- No surrounding lines provide a type declaration for `pUnknown`.

**Expected result:**
- `provideDefinition` returns `undefined` without throwing.
- `VtableResolver.resolve()` returns `{ found: false, errorMsg: "..." }`.
- A status bar message is triggered (verified separately in TC-ERR tests).

**Requirement:** REQ-VSCODE-002, REQ-VSCODE-004

---

## Section 14: Error Handling UX Tests

These test cases verify the non-intrusive error reporting behavior described in design Section 8.
They cover REQ-VSCODE-004 and REQ-VSCODE-013.

**Test double approach:** Inject a stub `StatusBarHelper` and stub `vscode.window` into
`JunoDefinitionProvider`. Capture all `showInformationMessage`, `showErrorMessage`, and
`statusBarItem.text` calls.

---

### Test Case ID: TC-ERR-001
**Scenario:** Status bar message is displayed when resolution fails (non-intrusive)  
**Setup:**
- Wire `JunoDefinitionProvider` with a `VtableResolver` stub that returns
  `{ found: false, errorMsg: "No implementation found for 'JUNO_APP_API_T::Launch'." }`.
- Provide a spy `statusBarItem` injected into `StatusBarHelper`.
- Trigger `provideDefinition` at a call site that the resolver cannot satisfy.

**Expected result:**
- `statusBarItem.text` is set to a string containing `"LibJuno"` and the error message.
- `statusBarItem.show()` is called exactly once.
- `vscode.window.showErrorMessage` is NOT called.

**Requirement:** REQ-VSCODE-004, REQ-VSCODE-013

---

### Test Case ID: TC-ERR-002
**Scenario:** Status bar message auto-clears after 5 seconds  
**Setup:**
- Use Jest fake timers (`jest.useFakeTimers()`).
- Trigger resolution failure as in TC-ERR-001.
- Advance timers by 4999 ms.

**Expected result (at 4999 ms):**
- `statusBarItem.hide()` has NOT been called.

**Then advance timers by 1 ms (total 5000 ms):**
- `statusBarItem.hide()` is called.

**Requirement:** REQ-VSCODE-013

---

### Test Case ID: TC-ERR-003
**Scenario:** Information message with "Show Details" appears on repeated failure within 10 seconds  
**Setup:**
- Use Jest fake timers.
- Stub `vscode.window.showInformationMessage` as a jest spy returning a Promise resolving to `undefined`.
- Trigger resolution failure at T=0 ms.
- Advance timers by 5000 ms (first status bar clears).
- Trigger resolution failure again at T=8000 ms (within the 10-second window).

**Expected result:**
- `vscode.window.showInformationMessage` is called with:
  - First argument: the error message string.
  - Second argument: `"Show Details"`.
- This is the second failure, so the information message fires on the second trigger.

**Requirement:** REQ-VSCODE-013

---

### Test Case ID: TC-ERR-004
**Scenario:** No modal dialog (`showErrorMessage`) is ever shown for resolution failures  
**Setup:**
- Spy on `vscode.window.showErrorMessage`.
- Trigger three consecutive resolution failures.

**Expected result:**
- `vscode.window.showErrorMessage` is NEVER called.

**Requirement:** REQ-VSCODE-013

---

### Test Case ID: TC-ERR-005
**Scenario:** Error message includes the specific failure reason with API type and field name  
**Setup:**
- Index state: `JUNO_DS_HEAP_API_T` exists in `apiStructFields` with field `"Insert"`, but
  `vtableAssignments["JUNO_DS_HEAP_API_T"]["Insert"]` is empty (no concrete implementation).
- Trigger `VtableResolver.resolve()` for a call site on `Insert` with `apiType = "JUNO_DS_HEAP_API_T"`.

**Expected result:**
- `VtableResolutionResult.errorMsg` contains both `"JUNO_DS_HEAP_API_T"` and `"Insert"`.
  (For example: `"No implementation found for 'JUNO_DS_HEAP_API_T::Insert'."`)
- The status bar message text propagates this specific error message (not a generic one).

**Requirement:** REQ-VSCODE-004, REQ-VSCODE-013

---

### Test Case ID: TC-ERR-006
**Scenario:** MCP tool returns a proper error object (not an HTTP error code) when resolution fails  
**Setup:**
- Start the MCP server with a `VtableResolver` stub returning
  `{ found: false, errorMsg: "No implementation found for 'JUNO_DS_HEAP_API_T::Insert'." }`.
- Send a `resolve_vtable_call` request with a file/line/column that the stub will fail on.

**Expected result:**
- HTTP response status: `200 OK` (not 4xx or 5xx).
- Response body JSON:
  ```json
  {
    "found": false,
    "locations": [],
    "error": "No implementation found for 'JUNO_DS_HEAP_API_T::Insert'."
  }
  ```
- The MCP `result` object contains `isError: true` (per MCP protocol).

**Requirement:** REQ-VSCODE-004

---
