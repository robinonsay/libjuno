> Part of: [Test Case Specification](index.md) — Sections 17-18: Navigation and QuickPick Tests

## Section 17: Failure Handler Navigation Tests

These test cases verify end-to-end navigation for failure handler assignments (REQ-VSCODE-016),
including the `JUNO_FAILURE_HANDLER` macro form, which is now handled by the `JunoFailureHandler` alternation token (closing the gap previously identified in TC-P10-002).

**Test double approach:** Inject a `NavigationIndex` stub and a source-text getter stub into
`FailureHandlerResolver`. Use the same resolver infrastructure as the vtable resolution tests.

---

### Test Case ID: TC-FH-001
**Scenario:** Ctrl+Click on `_pfcnFailureHandler` assignment resolves to handler function definition  
**Setup:**
- Index state:
  ```typescript
  failureHandlerAssignments: {
    "JUNO_DS_HEAP_ROOT_T": [
      { functionName: "MyHeapFailureHandler", file: "src/init.c", line: 42 }
    ]
  }
  ```
- Stub source text at call site line: `    ptHeap->_pfcnFailureHandler = MyHeapFailureHandler;`
- LocalTypeInfo provides: `JUNO_DS_HEAP_ROOT_T *ptHeap`.
- Cursor: column on `_pfcnFailureHandler`.

**Expected result:**
```typescript
{ found: true, locations: [{ functionName: "MyHeapFailureHandler", file: "src/init.c", line: 42 }] }
```

**Requirement:** REQ-VSCODE-016

---

### Test Case ID: TC-FH-002
**Scenario:** `JUNO_FAILURE_HANDLER` macro form — visitFailureHandlerAssignment resolves assignment  
**Setup:**
- This test verifies that the `JunoFailureHandler` alternation token correctly handles the macro form (gap from TC-P10-002 is closed).
- Index state:
  ```typescript
  failureHandlerAssignments: {
    "JUNO_APP_ROOT_T": [
      { functionName: "pfcnFailureHandler", file: "examples/example_project/engine/src/engine_app.c", line: 111 }
    ]
  }
  ```
- Stub source line (from `engine_app.c`, line 111):
  `    ptEngineApp->tRoot.JUNO_FAILURE_HANDLER = pfcnFailureHandler;`
- The visitFailureHandlerAssignment visitor (via the `JunoFailureHandler` alternation token) correctly parses this form.
- LocalTypeInfo context: `ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp);`
  → root type resolved via derivation chain: `ENGINE_APP_T` → `JUNO_APP_ROOT_T`.

**Expected result:**
```typescript
{ found: true, locations: [{ functionName: "pfcnFailureHandler", file: "examples/.../engine_app.c", line: 111 }] }
```

**Note:** The `JunoFailureHandler` alternation token (`/JUNO_FAILURE_HANDLER\b|_pfcnFailureHandler\b/`) handles both the macro form and the expanded member name as the same token, closing the previously identified gap. The resolver traces `pfcnFailureHandler` through the call chain to the concrete handler definition.

**Requirement:** REQ-VSCODE-016

---

### Test Case ID: TC-FH-003
**Scenario:** Failure handler with multiple assignments across files shows QuickPick  
**Setup:**
- Index state:
  ```typescript
  failureHandlerAssignments: {
    "JUNO_APP_ROOT_T": [
      { functionName: "EngineFailureHandler", file: "engine/src/engine_app.c", line: 111 },
      { functionName: "SysManFailureHandler", file: "sys_manager/src/sys_manager_app.c", line: 88 }
    ]
  }
  ```
- Stub source line: `    ptApp->_pfcnFailureHandler = EngineFailureHandler;`
- Backward type: `JUNO_APP_ROOT_T *ptApp`.

**Expected result:**
- `FailureHandlerResolver.resolve()` returns `{ found: true, locations: [2 entries] }`.
- `JunoDefinitionProvider.provideDefinition()` returns `undefined` (navigation delegated to QuickPick).
- `vscode.window.showQuickPick` is called with two items.

**Requirement:** REQ-VSCODE-016

---

### Test Case ID: TC-FH-004
**Scenario:** Failure handler with no assignments shows error  
**Setup:**
- `failureHandlerAssignments` contains no entry for `"JUNO_LOG_ROOT_T"`.
- Stub source line: `    ptLogger->_pfcnFailureHandler = SomeHandler;`
- Backward type: `JUNO_LOG_ROOT_T *ptLogger`.

**Expected result:**
```typescript
{ found: false, errorMsg: "No failure handler registered for 'JUNO_LOG_ROOT_T'." }
```
- Status bar message is shown (TC-ERR-001 behavior applies).

**Requirement:** REQ-VSCODE-016

---

## Section 18: Multi-Implementation QuickPick Tests

These test cases verify the QuickPick presentation described in design Section 5.2 and 6.3
(REQ-VSCODE-006).

**Test double approach:** Spy on `vscode.window.showQuickPick`. Inject it into the
`JunoDefinitionProvider` or `QuickPickHelper`. Capture the items array passed to it.

---

### Test Case ID: TC-QP-001
**Scenario:** QuickPick items show function name as label  
**Setup:**
- `VtableResolver.resolve()` returns two locations:
  ```typescript
  locations: [
    { functionName: "OnStart", file: "engine/src/engine_app.c", line: 128 },
    { functionName: "OnStart", file: "sys_manager/src/sys_manager_app.c", line: 77 }
  ]
  ```
- Spy on `vscode.window.showQuickPick`.

**Expected result:**
- `showQuickPick` is called with items where every item has `label: "OnStart"`.

**Requirement:** REQ-VSCODE-006

---

### Test Case ID: TC-QP-002
**Scenario:** QuickPick items show `file:line` as description  
**Setup:** Same as TC-QP-001.

**Expected result:**
- Item 0 has `description: "engine_app.c:128"` (basename + colon + line number).
- Item 1 has `description: "sys_manager_app.c:77"`.

**Requirement:** REQ-VSCODE-006

---

### Test Case ID: TC-QP-003
**Scenario:** QuickPick items show workspace-relative path as detail  
**Setup:** Same as TC-QP-001. Workspace root: `/workspace`.

**Expected result:**
- Item 0 has `detail: "engine/src/engine_app.c"` (workspace-relative path).
- Item 1 has `detail: "sys_manager/src/sys_manager_app.c"`.
- Neither `detail` string is an absolute path starting with `/`.

**Requirement:** REQ-VSCODE-006

---

### Test Case ID: TC-QP-004
**Scenario:** Selecting a QuickPick item navigates to the correct file and line  
**Setup:**
- Stub `vscode.window.showQuickPick` to return a Promise resolving to item 0 (the first entry).
- Spy on `vscode.window.showTextDocument` and `vscode.workspace.openTextDocument`.
- Trigger `provideDefinition` with a two-location resolution result.

**Expected result:**
- `vscode.workspace.openTextDocument` is called with the URI of `engine/src/engine_app.c`.
- `vscode.window.showTextDocument` is called with a `selection` range starting at line 128
  (0-based: 127).

**Requirement:** REQ-VSCODE-006

---

### Test Case ID: TC-QP-005
**Scenario:** Cancelling QuickPick does not navigate anywhere  
**Setup:**
- Stub `vscode.window.showQuickPick` to return a Promise resolving to `undefined`
  (user pressed Escape).
- Spy on `vscode.window.showTextDocument`.
- Trigger `provideDefinition` with a two-location resolution result.

**Expected result:**
- `vscode.window.showTextDocument` is NOT called.
- No navigation occurs.
- No error or status bar message is shown for cancellation.

**Requirement:** REQ-VSCODE-006

---
