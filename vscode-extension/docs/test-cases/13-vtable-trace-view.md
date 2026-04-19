> Part of: [Test Case Specification](index.md) — Section 25: Vtable Resolution Trace View

## Section 25: Vtable Resolution Trace View (REQ-VSCODE-027–032)

These test cases verify the `VtableTraceProvider` component described in design §11. They cover
the construction of the three-node trace tree (call-site → composition-root → implementation),
the WebviewPanel HTML generation and security posture, command/keybinding registration, and
navigation from file links inside the panel.

**Test double approach:** Inject a mock `VtableResolver` through constructor injection into
`VtableTraceProvider`. Capture the HTML assigned to `webviewPanel.webview.html` via a mock
`vscode.window.createWebviewPanel` stub. Manually invoke `onDidReceiveMessage` handlers to
test postMessage navigation without a real webview.

**Prerequisite — ConcreteLocation fields:** TC-TRACE-005 requires `ConcreteLocation.assignmentFile`
and `ConcreteLocation.assignmentLine` to be populated by the visitor (WI-23.0). Tests using these
fields must construct `ConcreteLocation` objects with both fields set. If these fields are absent,
the composition-root node cannot be built and the provider must fall back gracefully (see TC-TRACE-015).

---

### TC-TRACE-001: Single implementation produces a 3-node trace

**Requirement:** REQ-VSCODE-030, REQ-VSCODE-031, REQ-VSCODE-032

**Scenario:** When the resolver returns exactly one location, `VtableTraceProvider` builds a
`VtableTrace` containing three nodes: a call-site node, a composition-root node, and an
implementation node.

**Setup:**
- `VtableResolver` mock returns:
  ```typescript
  {
    found: true,
    locations: [{
      functionName: 'JunoDs_BuffQueue_Dequeue',
      file: 'src/juno_buff_queue.c',
      line: 112,
      assignmentFile: 'examples/example_project/engine_app.c',
      assignmentLine: 45
    }]
  }
  ```
- `FunctionDefinitionRecord` for `JunoDs_BuffQueue_Dequeue`:
  `{ signature: 'JUNO_STATUS_T JunoDs_BuffQueue_Dequeue(JUNO_DS_BUFF_QUEUE_ROOT_T *ptQueue, JUNO_POINTER_T *ptOut)' }`
- Index state: `functionDefinitions` map contains the above record keyed by `'JunoDs_BuffQueue_Dequeue'`.

**Input:** cursor position — file: `examples/example_project/engine_app.c`, line: 223, column: 22;
lineText: `    tStatus = ptCmdPipeApi->Dequeue(ptCmdPipe, &tMsg);`

**Expected:**
- A `VtableTrace` object is produced with exactly three nodes.
- `callSite.type === 'call-site'`; `callSite.file === 'examples/example_project/engine_app.c'`; `callSite.line === 223`.
- `compositionRoot.type === 'composition-root'`; `compositionRoot.file === 'examples/example_project/engine_app.c'`; `compositionRoot.line === 45`.
- `implementation.type === 'implementation'`; `implementation.file === 'src/juno_buff_queue.c'`; `implementation.line === 112`.
- `WebviewPanel` is created (mock `createWebviewPanel` call count is 1).

**Notes:** This is the primary happy-path test. It verifies that all three node types are constructed
when all required data is present.

---

### TC-TRACE-002: Multiple implementations produce subtrees with shared call-site node

**Requirement:** REQ-VSCODE-027

**Scenario:** When the resolver returns two locations, the trace view contains two
composition-root/implementation subtrees and a single shared call-site node at the top.

**Setup:**
- `VtableResolver` mock returns:
  ```typescript
  {
    found: true,
    locations: [
      {
        functionName: 'EngineApp_OnStart',
        file: 'src/engine_app.c',
        line: 88,
        assignmentFile: 'examples/example_project/main.c',
        assignmentLine: 60
      },
      {
        functionName: 'SystemManagerApp_OnStart',
        file: 'src/system_manager_app.c',
        line: 74,
        assignmentFile: 'examples/example_project/main.c',
        assignmentLine: 67
      }
    ]
  }
  ```
- Index state: `functionDefinitions` map contains records for both function names.

**Input:** cursor position — file: `examples/example_project/main.c`, line: 120, column: 30;
lineText: `    ptAppList[i]->ptApi->OnStart(ptAppList[i]);`

**Expected:**
- The HTML assigned to `webviewPanel.webview.html` contains two separate composition-root/implementation
  subtree sections (e.g., two `.composition-root` div elements).
- The HTML contains exactly one `.call-site` div element.
- Both function names (`EngineApp_OnStart`, `SystemManagerApp_OnStart`) appear in the HTML.
- `createWebviewPanel` is called exactly once (single panel for all results).

**Notes:** Per design §11.4, multiple locations render as collapsible sections, each with its own
composition-root → implementation pair. Assert on the count of `.composition-root` elements in the
generated HTML.

---

### TC-TRACE-003: Resolver returns found:false — error shown, panel not created

**Requirement:** REQ-VSCODE-027

**Scenario:** When the resolver cannot find a resolution (found: false), the provider calls
`StatusBarHelper.showError()` and does NOT create a `WebviewPanel`.

**Setup:**
- `VtableResolver` mock returns:
  ```typescript
  { found: false, errorMsg: "No implementation found for 'JUNO_APP_API_T::OnStart'." }
  ```
- `StatusBarHelper` is injected as a mock with a `showError` spy.
- `vscode.window.createWebviewPanel` is a jest spy.

**Input:** cursor position — any file, line, column; lineText: `    ptApp->ptApi->OnStart(ptApp);`

**Expected:**
- `StatusBarHelper.showError()` is called exactly once with a message containing `'OnStart'`.
- `vscode.window.createWebviewPanel` is NOT called (call count is 0).

**Notes:** Verifies the resolver-failure branch in §11.2 Step 1. The error reporting path is
identical to the one used by `JunoDefinitionProvider` (§8.1).

---

### TC-TRACE-004: Call-site node fields are populated from cursor context

**Requirement:** REQ-VSCODE-030

**Scenario:** The call-site `TraceNode` uses the call expression text as its `label`, the
current file as `file`, and the cursor line number as `line`.

**Setup:**
- `VtableResolver` mock returns `found: true` with one location (any valid location).
- Index state: minimal — resolver stub bypasses actual index lookup.

**Input:** cursor position — file: `src/my_module.c`, line: 77, column: 18;
lineText: `    eStatus = ptHeap->ptApi->Insert(ptHeap, tValue);`

**Expected:**
- `callSite.file === 'src/my_module.c'`.
- `callSite.line === 77`.
- `callSite.label` contains the substring `'ptHeap->ptApi->Insert'` (the extracted call expression).
- `callSite.type === 'call-site'`.

**Notes:** The `extractCallExpression` helper (§11.2 Step 2) extracts the receiver + field name
text from `lineText`. Test that the label is derived from `lineText`, not hardcoded.

---

### TC-TRACE-005: Composition-root node uses assignmentFile and assignmentLine

**Requirement:** REQ-VSCODE-031

**Scenario:** The composition-root `TraceNode` is populated from
`ConcreteLocation.assignmentFile` and `ConcreteLocation.assignmentLine`, not from the
function definition file/line.

**Setup:**
- `VtableResolver` mock returns:
  ```typescript
  {
    found: true,
    locations: [{
      functionName: 'JunoDs_Heap_Insert',
      file: 'src/juno_heap.c',          // definition file
      line: 259,                         // definition line
      assignmentFile: 'src/juno_heap.c', // composition root
      assignmentLine: 18                 // composition root line
    }]
  }
  ```
- Note: `assignmentFile` and `assignmentLine` may differ from `file`/`line` — this test uses
  the same file for simplicity but the fields are semantically distinct.

**Input:** cursor position — file: `examples/example_project/main.c`, line: 90, column: 25;
lineText: `    ptHeap->ptApi->Insert(ptHeap, tValue);`

**Expected:**
- `compositionRoot.file === 'src/juno_heap.c'` (from `assignmentFile`).
- `compositionRoot.line === 18` (from `assignmentLine`).
- `compositionRoot.line !== implementation.line` (composition root line ≠ definition line).
- `implementation.line === 259` (definition line — unchanged).

**Notes:** Prerequisite: `ConcreteLocation.assignmentFile`/`assignmentLine` must be populated by
the visitor (WI-23.0). If these fields are undefined, the provider must not crash — test that
separately in TC-TRACE-015.

---

### TC-TRACE-006: Implementation node label and detail use functionName and signature

**Requirement:** REQ-VSCODE-032

**Scenario:** The implementation `TraceNode` uses `location.functionName` as its `label` and
`FunctionDefinitionRecord.signature` as its `detail`.

**Setup:**
- `VtableResolver` mock returns one location with `functionName: 'Publish'`.
- `NavigationIndex.functionDefinitions` contains:
  ```typescript
  new Map([['Publish', [{
    functionName: 'Publish',
    file: 'src/juno_broker.c',
    line: 51,
    isStatic: true,
    signature: 'static JUNO_STATUS_T Publish(JUNO_BROKER_ROOT_T *ptBroker, JUNO_BROKER_TOPIC_T tTopic, JUNO_POINTER_T tData)'
  }]]])
  ```

**Input:** cursor position — any file, line: 100, column: 20;
lineText: `    ptBroker->ptApi->Publish(ptBroker, tTopic, tData);`

**Expected:**
- `implementation.label === 'Publish'`.
- `implementation.detail === 'static JUNO_STATUS_T Publish(JUNO_BROKER_ROOT_T *ptBroker, JUNO_BROKER_TOPIC_T tTopic, JUNO_POINTER_T tData)'`.
- `implementation.type === 'implementation'`.

**Notes:** If no `FunctionDefinitionRecord` exists for the function name, `detail` falls back to
`location.functionName` (design §11.2 Step 4: `detail: location.signature ?? location.functionName`).
Write a separate assertion confirming that the signature from the index record is used, not a
hardcoded value.

---

### TC-TRACE-007: File paths with HTML special characters are escaped in panel content

**Requirement:** REQ-VSCODE-027

**Scenario:** A file path containing `<`, `>`, or `&` characters is HTML-escaped in the generated
WebviewPanel content, preventing XSS injection.

**Setup:**
- `VtableResolver` mock returns one location with:
  ```typescript
  {
    functionName: 'Handler',
    file: 'src/<script>alert(1)</script>.c',
    line: 10,
    assignmentFile: 'src/<script>alert(1)</script>.c',
    assignmentLine: 5
  }
  ```
- *(synthetic)* — this file path is constructed to verify HTML escaping.

**Input:** cursor position — any file, line: 1, column: 1;
lineText: `    ptApi->Handler();`

**Expected:**
- The HTML string assigned to `webviewPanel.webview.html` does NOT contain the literal substring
  `<script>alert(1)</script>`.
- The HTML string contains the HTML-escaped form: `&lt;script&gt;alert(1)&lt;/script&gt;`.
- `createWebviewPanel` is called (panel is created despite the unusual path).

**Notes:** Captures the HTML via `mockPanel.webview.html` assignment spy. Assert using
`expect(capturedHtml).not.toContain('<script>')` and
`expect(capturedHtml).toContain('&lt;script&gt;')`.

---

### TC-TRACE-008: CSP nonce appears in both the meta tag and the inline script tag

**Requirement:** REQ-VSCODE-027

**Scenario:** The generated HTML panel includes a Content-Security-Policy meta tag whose
`nonce` value matches the `nonce` attribute on the inline `<script>` tag.

**Setup:**
- `VtableResolver` mock returns one valid location.
- Capture the HTML string assigned to `webviewPanel.webview.html`.

**Input:** cursor position — any valid position; lineText: `    ptApi->DoThing();`

**Expected:**
- The captured HTML contains a `<meta http-equiv="Content-Security-Policy"` element.
- The CSP meta tag includes `script-src 'nonce-XXXX'` for some nonce string `XXXX`.
- The captured HTML contains `<script nonce="XXXX"` where `XXXX` is the same nonce value.
- The nonce value is non-empty and does not appear in the file path or function name data
  (i.e., it is a generated random value, not a constant).

**Notes:** Extract the nonce from the CSP meta tag using a regex such as
`/nonce-([a-zA-Z0-9+/=]+)/`. Then assert the same string appears in the `<script nonce="...">`
attribute. This verifies that the nonce is consistent within a single HTML generation call.

---

### TC-TRACE-009: File link click triggers vscode.window.showTextDocument

**Requirement:** REQ-VSCODE-027

**Scenario:** When the WebviewPanel webview sends a `{type: 'navigate', file: '...', line: N}`
message, the extension host calls `vscode.window.showTextDocument` with the correct file URI
and cursor position.

**Setup:**
- `VtableResolver` mock returns one location with `file: 'src/juno_heap.c'`, `line: 259`.
- `vscode.window.showTextDocument` is a jest spy returning `Promise.resolve()`.
- `vscode.Uri.file` is a jest spy returning a mock URI.
- After creating the trace panel, capture the `onDidReceiveMessage` handler registered on
  `webviewPanel.webview`.

**Input:** Manually invoke the captured `onDidReceiveMessage` handler with:
```typescript
{ type: 'navigate', file: 'src/juno_heap.c', line: 259 }
```

**Expected:**
- `vscode.Uri.file` is called with `'src/juno_heap.c'`.
- `vscode.window.showTextDocument` is called with the mock URI and options specifying line 259
  (0-based: `{ selection: new vscode.Range(258, 0, 258, 0) }` or equivalent).
- `showTextDocument` call count is 1.

**Notes:** The `onDidReceiveMessage` handler can be retrieved from the mock panel's registered
listeners. Manually invoking it allows testing the navigation behavior without a real webview
environment. Await the handler call if it returns a Promise.

---

### TC-TRACE-010: `libjuno.showVtableTrace` command is registered during activation

**Requirement:** REQ-VSCODE-028, REQ-VSCODE-029

**Scenario:** After calling `activate()`, the command `libjuno.showVtableTrace` is registered
with `vscode.commands.registerCommand`.

**Setup:**
- `vscode.commands.registerCommand` is a jest spy.
- Call `activate(mockContext)` with a stub `ExtensionContext`.

**Input:** (activation gesture)

**Expected:**
- `vscode.commands.registerCommand` is called at least once with first argument
  `'libjuno.showVtableTrace'`.
- The registered handler is a function (not undefined/null).
- The command ID `'libjuno.showVtableTrace'` appears in the list of `registerCommand` call
  arguments (spy `mock.calls` inspection).

**Notes:** This test does not require invoking the command handler — it only checks that
registration occurs. See TC-TRACE-001 for end-to-end handler invocation tests.

---

### TC-TRACE-011: Keybinding Ctrl+Shift+T defined in package.json for libjuno.showVtableTrace

**Requirement:** REQ-VSCODE-028

**Scenario:** The `package.json` `contributes.keybindings` array contains an entry binding
`ctrl+shift+t` to `libjuno.showVtableTrace` with an appropriate `when` clause.

**Setup:**
- Read `package.json` from the extension root at test time using `require('../../package.json')`.
- No runtime VSCode mock is needed — this is a static inspection test.

**Input:** (package.json contents)

**Expected:**
- `contributes.keybindings` is an array.
- Exactly one entry satisfies all of:
  - `command === 'libjuno.showVtableTrace'`
  - `key === 'ctrl+shift+t'` (case-insensitive)
  - `when` contains `'editorTextFocus'`
  - `when` contains `'resourceLangId'`

**Notes:** Design §11.5 specifies `"when": "editorTextFocus && resourceLangId =~ /^c/"`.
The test should assert the `when` clause contains both sub-expressions but need not require
the exact regex syntax to be byte-identical — it suffices that both `'editorTextFocus'` and
`'resourceLangId'` appear in the `when` string.

---

### TC-TRACE-012: Context menu entry defined in package.json for libjuno.showVtableTrace

**Requirement:** REQ-VSCODE-029

**Scenario:** The `package.json` `contributes.menus['editor/context']` array contains an entry
for `libjuno.showVtableTrace`.

**Setup:**
- Read `package.json` from the extension root using `require('../../package.json')`.
- No runtime VSCode mock is needed — static inspection only.

**Input:** (package.json contents)

**Expected:**
- `contributes.menus` is an object containing the key `'editor/context'`.
- `contributes.menus['editor/context']` is an array.
- At least one entry satisfies:
  - `command === 'libjuno.showVtableTrace'`
  - `when` contains `'resourceLangId'`
  - `group` is `'navigation'` (per design §11.5)

**Notes:** This test is complementary to TC-TRACE-011. Both tests read `package.json` statically;
neither requires starting the extension. If `contributes.menus` is absent, the test fails with a
descriptive message.

---

### TC-TRACE-013: WebviewPanel created with enableScripts:true and retainContextWhenHidden:true

**Requirement:** REQ-VSCODE-027

**Scenario:** The `WebviewPanel` is created with `enableScripts: true` and
`retainContextWhenHidden: true` in its options.

**Setup:**
- `vscode.window.createWebviewPanel` is a jest spy capturing its arguments.
- `VtableResolver` mock returns one valid location.

**Input:** cursor position — any valid file, line, column; lineText: `    ptApi->Method();`

**Expected:**
- `vscode.window.createWebviewPanel` is called with options object satisfying:
  - `options.enableScripts === true`
  - `options.retainContextWhenHidden === true`
- The `viewType` argument (first parameter) is `'libjunoVtableTrace'` or contains `'vtableTrace'`.
- `createWebviewPanel` call count is 1.

**Notes:** `retainContextWhenHidden: true` is required to preserve panel state when the user
switches editor tabs and returns. Asserting on these option flags confirms the panel is
configured correctly for interactive use.

---

### TC-TRACE-014: Generated HTML contains no external http:// or https:// URLs

**Requirement:** REQ-VSCODE-027

**Scenario:** The HTML content assigned to `webviewPanel.webview.html` does not reference any
external resources via `http://` or `https://` URLs (e.g., no CDN scripts, no remote fonts).

**Setup:**
- `VtableResolver` mock returns one valid location with typical file paths and a function
  signature containing no URL-like strings.
- Capture the HTML string assigned to `webviewPanel.webview.html`.

**Input:** cursor position — any valid position; lineText: `    ptApi->Method();`

**Expected:**
- The captured HTML string does NOT contain the substring `'http://'`.
- The captured HTML string does NOT contain the substring `'https://'`.

**Notes:** Per design §11.3 and §11.6, the panel is entirely self-contained with inline CSS and a
nonce-based inline script. This test guards against accidental introduction of external
resource references (e.g., a Google Fonts link or a CDN-hosted script).

---

### TC-TRACE-015: Resolver returns found:false with empty locations array — error shown, no panel

**Requirement:** REQ-VSCODE-027

**Scenario:** When the resolver returns `found: false` with an explicitly empty `locations` array,
the provider shows an error and does not create a `WebviewPanel`. This is a distinct error-path
test from TC-TRACE-003, which uses a simple not-found result without an empty array field.

**Setup:**
- `VtableResolver` mock returns:
  ```typescript
  { found: false, locations: [], errorMsg: "No LibJuno API call pattern found at cursor position." }
  ```
- `StatusBarHelper.showError` spy is injected.
- `vscode.window.createWebviewPanel` is a jest spy.

**Input:** cursor position — file: `src/my_app.c`, line: 42, column: 8;
lineText: `    JUNO_MODULE_SUPER(ptSelf, MY_ROOT_T)->DoThing(ptSelf);`

**Notes on input:** Per the lessons-learned file (2026-04-17), `JUNO_MODULE_SUPER(...)` does
NOT match generalRe because the `(` immediately follows the identifier, preventing the
`->field(` pattern from connecting. The resolver correctly returns `found: false` for this line.

**Expected:**
- `StatusBarHelper.showError()` is called exactly once.
- The error message passed to `showError` contains `'No LibJuno API call pattern found'`.
- `vscode.window.createWebviewPanel` is NOT called (call count remains 0).

**Notes:** This test covers the edge case where `locations` is present but empty, ensuring the
provider does not attempt to render an empty trace. It also documents known behavior for the
`JUNO_MODULE_SUPER` pattern.

---

### TC-TRACE-016: walkVtableDeclaration stamps varName on each VtableAssignmentRecord

**Requirement:** REQ-VSCODE-036

**Scenario:** For a top-level designated vtable struct declaration, `parseFileWithDefs` sets `varName` on every emitted `VtableAssignmentRecord` to the variable name of the API struct.

**Setup:** Source containing `static const JUNO_LOG_API_T gtMyLoggerApi = { .LogInfo = JunoLog_DebugLogger_LogInfo, .LogError = JunoLog_DebugLogger_LogError };`

**Expected:** `parsed.vtableAssignments[0].varName === 'gtMyLoggerApi'` and `parsed.vtableAssignments[1].varName === 'gtMyLoggerApi'`.

---

### TC-TRACE-017: Direct assignment inside function body does not populate varName

**Requirement:** REQ-VSCODE-036

**Scenario:** A direct vtable assignment inside a function body does not set `varName` (varName is only for top-level struct declarations).

**Expected:** `parsed.vtableAssignments[0].varName === undefined`.

---

### TC-TRACE-018: resolveCompositionRoots stamps initCallFile/Line on ConcreteLocation

**Requirement:** REQ-VSCODE-036

**Scenario:** After `fullIndex()`, a `ConcreteLocation` whose API variable is passed by address in a call site has `initCallFile` and `initCallLine` pointing to that call site.

**Expected:** `loc.initCallFile` ends with the source file containing the call; `loc.initCallLine` equals the call site line.

---

### TC-TRACE-019: No false positive for non-API variable address

**Requirement:** REQ-VSCODE-036

**Scenario:** Taking the address of a local variable that is NOT a known vtable API variable does not pollute `initCallIndex` for that local name.

**Expected:** `index.initCallIndex.has('local') === false`.

---

### TC-TRACE-020: clearIndex empties initCallIndex

**Requirement:** REQ-VSCODE-036

**Scenario:** After `clearIndex()`, `initCallIndex` is empty.

**Expected:** `index.initCallIndex.size === 0`.

---

### TC-TRACE-021: Composition root node uses initCallFile/initCallLine when present

**Requirement:** REQ-VSCODE-036

**Scenario:** When `ConcreteLocation` has both `initCallFile`/`initCallLine` and `assignmentFile`/`assignmentLine`, the composition-root node in the Vtable Trace View uses the init call values.

**Expected:** HTML contains the init call file link and line; the `assignmentLine` value is not used.

---

### TC-TRACE-022: Composition root falls back to assignmentFile/assignmentLine when initCallFile absent

**Requirement:** REQ-VSCODE-036

**Scenario:** When `ConcreteLocation` has no `initCallFile`/`initCallLine`, the composition root falls back to `assignmentFile`/`assignmentLine`.

**Expected:** `compositionRoot.file === assignmentFile`; `compositionRoot.line === assignmentLine`.

---

### TC-TRACE-023 — Positional Vtable Composition Root Resolved via varName Threading
**Requirement:** REQ-VSCODE-036
**File:** `src/indexer/__tests__/workspaceIndexer.test.ts`
**Scenario:** A positional vtable initializer (e.g., `static const MY_API_T gtMyApi = { FuncA };`) has its `varName` threaded through `PendingPositionalVtable → DeferredPositional → ConcreteLocation.apiVarName`. After `fullIndex()`, `resolveCompositionRoots()` scans for `&gtMyApi` in source files and stamps `initCallFile`/`initCallLine` on the matching `ConcreteLocation`.
**Expected:** `ConcreteLocation.initCallFile` is set to the file containing the `&gtMyApi` call site.

### TC-TRACE-024 — varName Populated for Positional Vtable Initializer
**Requirement:** REQ-VSCODE-036
**File:** `src/parser/__tests__/visitor-vtable.test.ts`
**Scenario:** A file contains an explicit-tag struct definition and a matching positional vtable initializer assigned to a named variable. The visitor resolves positional vtable in-file and emits `VtableAssignmentRecord` entries.
**Expected:** Each emitted `VtableAssignmentRecord` has `varName` equal to the variable name of the vtable struct (e.g., `"gtMyLogApi"`).

### TC-TRACE-025 — resolveInitCallers Stamps compRootFile/compRootLine
**Requirement:** REQ-VSCODE-037
**File:** `src/indexer/__tests__/workspaceIndexer.test.ts`
**Scenario:** After `fullIndex()`, `resolveInitCallers()` finds the caller of the Init function (the function that contains the `&apiVar` site) and stamps `compRootFile`/`compRootLine` on the matching `ConcreteLocation`. Two temp files are used: one with a positional vtable initializer and an Init-function body containing `&apiVar`, and a second with a call to that Init function.
**Expected:** `ConcreteLocation.compRootFile` ends with the second temp file name; `compRootLine` points to the line containing the Init function call.

### TC-TRACE-026 — 4-Node Chain Rendered When compRootFile and initCallFile Both Present
**Requirement:** REQ-VSCODE-037
**File:** `src/providers/__tests__/vtableTraceProvider.test.ts`
**Scenario:** A `ConcreteLocation` with both `compRootFile` and `initCallFile` set is passed through `VtableTraceProvider.showTrace()`. The generated HTML must include the initialization implementation node.
**Expected:** HTML contains `class="trace-node init-impl"`; composition root link targets `compRootFile:compRootLine`; init-impl link targets `initCallFile:initCallLine`.

### TC-TRACE-027 — 3-Node Fallback When compRootFile Absent
**Requirement:** REQ-VSCODE-037
**File:** `src/providers/__tests__/vtableTraceProvider.test.ts`
**Scenario:** A `ConcreteLocation` with `initCallFile` set but no `compRootFile` is passed through `VtableTraceProvider.showTrace()`. The initialization implementation node must not appear.
**Expected:** HTML does NOT contain `<div class="trace-node init-impl">`.

---
