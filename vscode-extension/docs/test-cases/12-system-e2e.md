> Part of: [Test Case Specification](index.md) — Section 19: System-Level End-to-End Tests

## Section 19: System-Level End-to-End Tests

These test cases exercise the **complete extension against a real VSCode instance** using
`@vscode/test-electron`. Unlike Sections 12–18, these tests do NOT use stubs or mock objects.
They launch a real VSCode process, open the `examples/example_project/` workspace, wait for
the extension to activate and fully build its index from real source files, then invoke the
Go to Definition provider via `vscode.commands.executeCommand('vscode.executeDefinitionProvider',
...)` on real call sites and assert that the editor navigates to the correct file and line.

**Test framework:** `@vscode/test-electron` + Mocha (the standard VS Code extension E2E runner).

**Shared `before()` / `suiteSetup()` hook (applied to the entire Section 19 suite):**
1. Launch VSCode with `examples/example_project/` as the workspace folder.
2. Poll `vscode.extensions.getExtension('libjuno.libjuno-nav')?.isActive` until `true`
   (timeout: 30 s, 500 ms poll interval).
3. Poll until either `vscode.workspace.getConfiguration('libjuno').get('indexReady') === true`
   or the file `.libjuno/navigation-cache.json` exists in the workspace root, indicating that
   the index build is complete (timeout: 60 s).

**Shared `after()` / `suiteTeardown()` hook:**
- Execute `vscode.commands.executeCommand('workbench.action.closeAllEditors')`.
- No workspace cleanup is required — these tests are read-only against the example project.

**Key assertion helper used in all navigation tests:**
```typescript
async function goToDefinition(
  relPath: string, line: number, col: number
): Promise<vscode.Location[]> {
  const wsRoot = vscode.workspace.workspaceFolders![0].uri.fsPath;
  const uri = vscode.Uri.file(path.join(wsRoot, relPath));
  const doc = await vscode.workspace.openTextDocument(uri);
  await vscode.window.showTextDocument(doc);
  // convert 1-based line/col to 0-based Position
  const pos = new vscode.Position(line - 1, col - 1);
  return vscode.commands.executeCommand<vscode.Location[]>(
    'vscode.executeDefinitionProvider', uri, pos
  ) as Promise<vscode.Location[]>;
}
```

**What makes these tests different from Sections 12–18:**
Sections 12–18 exercise individual components (resolvers, parsers, cache, QuickPick) in
isolation using Jest with injected stubs — the VS Code API and the file system are mocked.
Section 19 tests run inside a real VS Code process against real source files. They validate
the full vertical slice: file indexing → cache → resolution algorithm → VS Code provider
registration → navigation result. These tests are slower (each suite launch takes ~10–30 s)
but catch class-of-bugs that stub-based tests cannot, such as wrong workspace-root path
construction, URI scheme mismatches, index-build race conditions, and incorrect column offsets
on real multi-byte source lines.

---

### Test Case ID: TC-SYS-001
**Scenario:** Indirect API pointer — `ptLoggerApi->LogInfo` in `engine_app.c` navigates to the static `LogInfo` definition in `main.c`
**Setup:**
- Open `examples/example_project/engine/src/engine_app.c` via `goToDefinition()`.
- Position: line 140, column 18 (on `LogInfo` in `ptLoggerApi->LogInfo(ptLogger, "Engine App Initialized")`).
- Chain-walk (Category 3 — direct API pointer) applies: LocalTypeInfo resolves `const JUNO_LOG_API_T *ptLoggerApi = ptLogger->ptApi;` at line 138, establishing `apiType = "JUNO_LOG_API_T"`.

**Expected result:**
- `Location[]` has exactly **1** entry.
- `locations[0].uri.fsPath` ends with `examples/example_project/main.c`.
- `locations[0].range.start.line` is approximately 61 (0-based; ≈ line 62 in the source, the `static void LogInfo(...)` definition).

**Note:** More than one location indicates a chain-walk (Category 3 — direct API pointer) regression — the indirect API pointer must resolve to a single static implementation.
**Requirement:** REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005

---

### Test Case ID: TC-SYS-002
**Scenario:** Chained member access — `ptEngineApp->ptBroker->ptApi->RegisterSubscriber` navigates to the static `RegisterSubscriber` definition in `juno_broker.c`
**Setup:**
- Open `examples/example_project/engine/src/engine_app.c`.
- Position: line 151, column 45 (on `RegisterSubscriber` in `ptEngineApp->ptBroker->ptApi->RegisterSubscriber(...)`).
- Chain-walk (Category 1) applies: pre-`ptApi` expression is `ptEngineApp->ptBroker`; chained resolution traces `ptBroker` as a member of `ENGINE_APP_T` with type `JUNO_SB_BROKER_ROOT_T *`, then `apiType = "JUNO_SB_BROKER_API_T"`.

**Expected result:**
- `Location[]` has exactly **1** entry.
- `locations[0].uri.fsPath` ends with `src/juno_broker.c`.
- `locations[0].range.start.line` is approximately 75 (0-based; ≈ line 76, the `static JUNO_STATUS_T RegisterSubscriber(...)` definition).

**Requirement:** REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005

---

### Test Case ID: TC-SYS-003
**Scenario:** Standard `ptApi` access — `ptTime->ptApi->Now` navigates to the static `Now` definition in `main.c`
**Setup:**
- Open `examples/example_project/engine/src/engine_app.c`.
- Position: line 204, on `Now` (within `ptTime->ptApi->Now(ptTime)`).
- Chain-walk (Category 1) applies: `ptTime` LocalTypeInfo resolves to `const JUNO_TIME_ROOT_T *ptTime`; `apiType = "JUNO_TIME_API_T"`.

**Expected result:**
- `Location[]` has exactly **1** entry.
- `locations[0].uri.fsPath` ends with `examples/example_project/main.c`.
- `locations[0].range.start.line` is approximately 101 (0-based; ≈ line 102, the static `Now` function definition).

**Requirement:** REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005

---

### Test Case ID: TC-SYS-004
**Scenario:** Standard `ptApi` access — `ptBroker->ptApi->Publish` navigates to the static `Publish` definition in `juno_broker.c`
**Setup:**
- Open `examples/example_project/engine/src/engine_app.c`.
- Position: line 238, on `Publish` (within `ptBroker->ptApi->Publish(ptBroker, ENGINE_TLM_MSG_MID, tEngineTlmPointer)`).
- Chain-walk (Category 1) applies: `ptBroker` LocalTypeInfo resolves to `JUNO_SB_BROKER_ROOT_T *ptBroker`; `apiType = "JUNO_SB_BROKER_API_T"`.

**Expected result:**
- `Location[]` has exactly **1** entry.
- `locations[0].uri.fsPath` ends with `src/juno_broker.c`.
- `locations[0].range.start.line` is approximately 50 (0-based; ≈ line 51, the `static JUNO_STATUS_T Publish(...)` definition).

**Requirement:** REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005

---

### Test Case ID: TC-SYS-005
**Scenario:** Array subscript receiver — `ptAppList[i]->ptApi->OnStart` returns multiple locations for QuickPick
**Setup:**
- Open `examples/example_project/main.c`.
- Position: line 203, on `OnStart` (within `ptAppList[i]->ptApi->OnStart(ptAppList[i])`).
- Chain-walk (Category 1) applies: `ptAppList[i]` strips the `[i]` subscript to `ptAppList`; LocalTypeInfo resolves `static JUNO_APP_ROOT_T *ptAppList[2]`, resolving `rootType = "JUNO_APP_ROOT_T"` → `apiType = "JUNO_APP_API_T"`.

**Expected result:**
- `Location[]` has **2 or more** entries.
- At least one `uri.fsPath` contains `engine/src/engine_app.c`; its `range.start.line` points to the `static JUNO_STATUS_T OnStart(...)` definition in that file.
- At least one `uri.fsPath` contains `system_manager/src/system_manager_app.c`; its `range.start.line` points to the `static JUNO_STATUS_T OnStart(...)` definition in that file.

**Note:** The two-or-more-location result triggers QuickPick in the real extension. The test verifies the returned `Location[]` contents only; QuickPick user interaction is not driven in this E2E test.
**Requirement:** REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-006

---

### Test Case ID: TC-SYS-006
**Scenario:** Non-static library function — `ptTime->ptApi->SubtractTime` navigates to the `JunoTime_SubtractTime` definition in `juno_time.c`
**Setup:**
- Open `examples/example_project/system_manager/src/system_manager_app.c`.
- Position: line 166, on `SubtractTime` (within `ptTime->ptApi->SubtractTime(ptTime, &tTlmMsg.tTimestamp, ptSystemManagerApp->tEngineStart)`).
- Chain-walk (Category 1) applies: `ptTime` LocalTypeInfo resolves to `const JUNO_TIME_ROOT_T *ptTime`; `apiType = "JUNO_TIME_API_T"`.

**Expected result:**
- `Location[]` has exactly **1** entry.
- `locations[0].uri.fsPath` ends with `src/juno_time.c`.
- `locations[0].range.start.line` is approximately 49 (0-based; ≈ line 50, the `JunoTime_SubtractTime` function definition, which is non-static).

**Note:** This test validates that the indexer captures non-static (library-linkage) function definitions, not only `static` ones.
**Requirement:** REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005

---

### Test Case ID: TC-SYS-007
**Scenario:** Dot-chained member access — `.tRoot.ptApi->Dequeue` navigates to the `Dequeue` implementation in `juno_buff_queue.c`
**Setup:**
- Open `examples/example_project/system_manager/src/system_manager_app.c`.
- Position: line 159, on `Dequeue` (within `ptSystemManagerApp->tEngineTlmPipe.tRoot.ptApi->Dequeue(&ptSystemManagerApp->tEngineTlmPipe.tRoot, tTlmMsgPointer)`).
- Chain-walk (Category 2) applies: `.tEngineTlmPipe.tRoot.ptApi->Dequeue` — `tRoot` resolves to `JUNO_DS_QUEUE_ROOT_T`, giving `apiType = "JUNO_DS_QUEUE_API_T"`.

**Expected result:**
- `Location[]` has exactly **1** entry.
- `locations[0].uri.fsPath` ends with `src/juno_buff_queue.c`.
- The range points to the function assigned to the `Dequeue` slot in the positional initializer for `JUNO_DS_QUEUE_API_T` (i.e., `JunoDs_QueuePop` or equivalent).

**Note:** This test additionally validates that the positional initializer indexer (REQ-VSCODE-012) has correctly mapped the `Dequeue` slot by field order from the `JUNO_DS_QUEUE_API_T` struct definition.
**Requirement:** REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005, REQ-VSCODE-012

---

### Test Case ID: TC-SYS-008
**Scenario:** Failure handler assignment via `JUNO_FAILURE_HANDLER` macro form navigates to `FailureHandler` in `main.c`
**Setup:**
- Open `examples/example_project/engine/src/engine_app.c`.
- Position: line 111, on `JUNO_FAILURE_HANDLER` (the assignment `ptEngineApp->tRoot.JUNO_FAILURE_HANDLER = pfcnFailureHandler`).
- The `JunoFailureHandler` alternation token matches the macro form: the resolver traces `pfcnFailureHandler` through the `EngineApp_Init` call chain in `main.c` to the `void FailureHandler(...)` definition at approximately line 164.

**Expected result:**
- `Location[]` has exactly **1** entry.
- `locations[0].uri.fsPath` ends with `examples/example_project/main.c`.
- `locations[0].range.start.line` is approximately 163 (0-based; ≈ line 164, the `void FailureHandler(...)` definition).

**Note:** The `JunoFailureHandler` alternation token (`/JUNO_FAILURE_HANDLER\b|_pfcnFailureHandler\b/`) handles both the macro form and the expanded member name as the same token, closing the previously identified gap. The resolver traces `pfcnFailureHandler` through the call chain to the concrete handler definition.
**Requirement:** REQ-VSCODE-016

---

### Test Case ID: TC-SYS-009
**Scenario:** Array subscript with variable index — `ptAppList[iCounter]->ptApi->OnProcess` returns multiple locations
**Setup:**
- Open `examples/example_project/main.c`.
- Position: line 204, on `OnProcess` (within `ptAppList[iCounter]->ptApi->OnProcess(ptAppList[iCounter])`).
- Chain-walk (Category 1) applies: `ptAppList[iCounter]` strips the `[iCounter]` subscript to `ptAppList`; same LocalTypeInfo result as TC-SYS-005 (`JUNO_APP_ROOT_T *ptAppList[2]` → `apiType = "JUNO_APP_API_T"`).

**Expected result:**
- `Location[]` has **2 or more** entries.
- At least one `uri.fsPath` contains `engine/src/engine_app.c`; its range points to the `static JUNO_STATUS_T OnProcess(...)` definition.
- At least one `uri.fsPath` contains `system_manager/src/system_manager_app.c`; its range points to the `static JUNO_STATUS_T OnProcess(...)` definition.

**Requirement:** REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-006

---

### Test Case ID: TC-SYS-010
**Scenario:** Cross-file resolution — `ptLoggerApi->LogInfo` in `system_manager_app.c` navigates to the same static `LogInfo` definition in `main.c` as TC-SYS-001
**Setup:**
- Open `examples/example_project/system_manager/src/system_manager_app.c`.
- Position: approximately line 106, on `LogInfo` (within `ptLoggerApi->LogInfo(ptLogger, "SystemManager App Initialized")`).
- Chain-walk (Category 3 — direct API pointer) applies: LocalTypeInfo resolves `const JUNO_LOG_API_T *ptLoggerApi = ptLogger->ptApi;` earlier in the same function.

**Expected result:**
- `Location[]` has exactly **1** entry.
- `locations[0].uri.fsPath` ends with `examples/example_project/main.c`.
- `locations[0].range.start.line` is approximately 61 (0-based; ≈ line 62) — the identical `LogInfo` definition reached by TC-SYS-001 from a different source file.

**Note:** This test validates that the index correctly shares API implementation records across all files that use the same vtable, not just the first file that was indexed.
**Requirement:** REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005

---

### Test Case ID: TC-SYS-011
**Scenario:** Extension activation with the real example project succeeds and builds a valid index
**Setup:**
- The `before()` hook (shared suite setup) has already launched VS Code with
  `examples/example_project/` and polled for activation and index completion.
- Read `.libjuno/navigation-cache.json` from the workspace root using
  `vscode.workspace.fs.readFile`.
- The VS Code Output channel for the extension (`"LibJuno Navigation"`) is captured for
  error messages.

**Expected result:**
1. `vscode.extensions.getExtension('libjuno.libjuno-nav')?.isActive` is `true`.
2. The `.libjuno/navigation-cache.json` file exists and parses as valid JSON containing a
   non-empty `moduleRoots` map (at minimum, `"JUNO_LOG_ROOT_T"`, `"JUNO_APP_ROOT_T"`,
   `"JUNO_SB_BROKER_ROOT_T"` are present).
3. The extension Output channel contains no lines with the prefix `[ERROR]` during activation.

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-SYS-012
**Scenario:** Non-vtable call site is not intercepted by the extension — the default C language provider handles it
**Setup:**
- Open `examples/example_project/engine/src/engine_app.c`.
- Position cursor on a direct (non-vtable) function call such as `EngineCmdMsg_ArrayInit`
  (a free-function call, not a `->ptApi->` call).
- Execute `vscode.commands.executeCommand('vscode.executeDefinitionProvider', uri, position)`.

**Expected result:**
- The returned `Location[]` is non-empty (the built-in C/C++ language server returns at
  least one location pointing to the function's declaration or definition).
- The test does NOT assert that the result is empty — rather, it asserts that the extension
  did not break normal Go to Definition for non-vtable calls.

**Note:** Since `executeDefinitionProvider` aggregates results from all registered providers,
this test cannot directly attribute which provider returned the result. The important assertion
is that a valid non-empty result is returned, confirming the extension does not swallow or
corrupt the query for plain function calls.
**Requirement:** REQ-VSCODE-007

---

### Test Case ID: TC-SYS-013
**Scenario:** Designated initializer indexing — the `tEngineAppApi` vtable in `engine_app.c` is correctly indexed for all three fields
**Setup:**
- After the shared suite `before()` hook confirms index completion, query the MCP
  `resolve_vtable_call` tool for each of the three fields declared in the designated
  initializer at approximately lines 52–56 of
  `examples/example_project/engine/src/engine_app.c`:
  ```c
  static const JUNO_APP_API_T tEngineAppApi = {
      .OnStart   = OnStart,
      .OnProcess = OnProcess,
      .OnExit    = OnExit,
  };
  ```
- For each field, POST to `http://127.0.0.1:6543/mcp` with a call-site from `main.c` that
  invokes `ptAppList[i]->ptApi-><fieldName>(ptAppList[i])` — the same call sites used by
  TC-SYS-005 and TC-SYS-009.

**Expected result:**
- For `OnStart`: MCP response has `"found": true`; the `locations` array contains an entry
  with a path containing `engine/src/engine_app.c`, pointing to the `OnStart` static function.
- For `OnProcess`: MCP response has `"found": true`; the `locations` array contains an entry
  with a path containing `engine/src/engine_app.c`, pointing to the `OnProcess` static function.
- For `OnExit`: MCP response has `"found": true`; the `locations` array contains an entry
  with a path containing `engine/src/engine_app.c`, pointing to the `OnExit` static function.
- All three HTTP responses return status `200 OK`.

**Note:** TC-SYS-005 and TC-SYS-009 indirectly verify two of these fields. This test
eliminates ambiguity by directly validating all three designated slots and confirming no
off-by-one or field-name mismatch in the designated initializer indexer (P6).
**Requirement:** REQ-VSCODE-010

---

### Test Case ID: TC-SYS-014
**Scenario:** Positional initializer indexing — the `gtMyLoggerApi` in `main.c` maps all four fields correctly by position
**Setup:**
- After the shared suite `before()` hook confirms index completion, query the MCP
  `resolve_vtable_call` tool for each of the four fields in the positional initializer at
  approximately lines 152–157 of `examples/example_project/main.c`:
  ```c
  static const JUNO_LOG_API_T gtMyLoggerApi = { LogDebug, LogInfo, LogWarning, LogError };
  ```
  The four functions are mapped by position to the field order of `JUNO_LOG_API_T`
  `["LogDebug", "LogInfo", "LogWarning", "LogError"]` (positional slots 0–3).
- For each field, POST to the MCP `resolve_vtable_call` endpoint with the relevant
  call-site coordinates from `engine_app.c` or `system_manager_app.c`.

**Expected result:**
- For `LogDebug` (slot 0): MCP returns `"found": true`; location points to
  `static void LogDebug(...)` in `examples/example_project/main.c`.
- For `LogInfo` (slot 1): MCP returns `"found": true`; location points to
  `static void LogInfo(...)` in `main.c` (≈ line 62) — the same definition reached by
  TC-SYS-001 and TC-SYS-010.
- For `LogWarning` (slot 2): MCP returns `"found": true`; location points to
  `static void LogWarning(...)` in `main.c`.
- For `LogError` (slot 3): MCP returns `"found": true`; location points to
  `static void LogError(...)` in `main.c`.
- All four MCP responses return status `200 OK`.

**Note:** TC-SYS-001 and TC-SYS-010 verify slot 1 (`LogInfo`) indirectly. This test validates
all four positional slots and independently confirms there is no off-by-one error in the
field-order zip performed by the P8 positional indexer.
