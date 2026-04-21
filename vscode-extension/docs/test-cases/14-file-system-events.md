> Part of: [Test Case Specification](index.md) — Section 26: File System Event Handling Tests

## Section 26: File System Event Handling Tests

These test cases verify the extension's `FileSystemWatcher` integration for automatic indexing
of newly created C/C++ files and automatic index cleanup on file deletion. They correspond to
REQ-VSCODE-042 (automatic indexing of newly created files) and REQ-VSCODE-043 (automatic index
cleanup on file deletion).

**Test double approach:** Use the existing `__mocks__/vscode.ts` FSW stub. Inject a
`WorkspaceIndexer` instance constructed with a pre-populated `NavigationIndex` and a
`readFile` stub (for ENOENT injection). Verify watcher handler registration via spy calls on
the FSW stub, and verify index state by inspecting `NavigationIndex` fields directly after
callbacks fire.

---

### Test Case ID: TC-FSE-001
**Scenario:** `onDidCreate` watcher registered on activation

**Setup:**
- Spy on `vscode.workspace.createFileSystemWatcher` so the returned watcher object is
  captured.
- Call the exported `activate(context)` function with a mock `ExtensionContext`.

**Expected result:**
- The watcher object returned by `createFileSystemWatcher` has had `onDidCreate` called
  at least once, with a function argument.
- The registered callback is of type `function` (not `null` or `undefined`).

**Requirement:** REQ-VSCODE-042

---

### Test Case ID: TC-FSE-002
**Scenario:** New `.c` file created — vtable assignment indexed

**Setup:**
- Initialize `WorkspaceIndexer` with an empty `NavigationIndex`.
- Construct synthetic `.c` file content containing:
  ```c
  JUNO_LOG_API_T gtApi = { .LogInfo = MyLogInfo };
  ```
- Inject a `readFile` stub that returns the synthetic content when called with the target
  file path `"/workspace/newfile.c"`.
- Fire the `onDidCreate` callback with a URI for `"/workspace/newfile.c"`.
- Await the async handler to complete.

**Expected result:**
- `index.vtableAssignments` contains a record with:
  - `apiType: "JUNO_LOG_API_T"`
  - `field: "LogInfo"`
  - `functionName: "MyLogInfo"`
- The `file` field of the record resolves to `"/workspace/newfile.c"`.
- `index.vtableAssignments` was not populated before the callback fired (confirming
  the record was added by the event handler, not by prior state).

**Requirement:** REQ-VSCODE-042

---

### Test Case ID: TC-FSE-003
**Scenario:** New `.h` file created — module root record indexed

**Setup:**
- Initialize `WorkspaceIndexer` with an empty `NavigationIndex`.
- Construct synthetic `.h` file content containing:
  ```c
  struct MY_MODULE_ROOT_TAG {
      JUNO_MODULE_ROOT(MY_MODULE_API_T, MY_DERIVE_T)
  };
  ```
- Inject a `readFile` stub that returns the synthetic content for `"/workspace/module.h"`.
- Fire the `onDidCreate` callback with a URI for `"/workspace/module.h"`.
- Await the async handler to complete.

**Expected result:**
- `index.moduleRoots` contains an entry mapping `"MY_MODULE_ROOT_T"` to `"MY_MODULE_API_T"`.
- `index.moduleRoots` was empty before the callback fired.

**Requirement:** REQ-VSCODE-042

---

### Test Case ID: TC-FSE-004
**Scenario:** Negative — non-C file created, not indexed

**Setup:**
- Capture the glob argument passed to `vscode.workspace.createFileSystemWatcher` during
  `activate()`.

**Expected result:**
- The glob string passed to `createFileSystemWatcher` matches only recognized C/C++
  extensions: `.c`, `.h`, `.cpp`, `.hpp`, `.hh`, `.cc`.
- The glob string does NOT contain `**/*.txt` and does NOT use `**/*.*` (which would
  match any extension).
- Specifically, the glob evaluates to a pattern such as
  `**/*.{c,h,cpp,hpp,hh,cc}` or equivalent that excludes non-C extensions at the
  VSCode platform level.

**Note:** Because VSCode filters `onDidCreate` events using the registered glob before
delivering them to the extension, no `.txt` file event will ever reach the handler.
This test validates the glob registration, not handler-side filtering.

**Requirement:** REQ-VSCODE-042

---

### Test Case ID: TC-FSE-005
**Scenario:** `onDidDelete` watcher registered on activation

**Setup:**
- Spy on `vscode.workspace.createFileSystemWatcher` so the returned watcher object is
  captured (same spy setup as TC-FSE-001).
- Call the exported `activate(context)` function with a mock `ExtensionContext`.

**Expected result:**
- The watcher object returned by `createFileSystemWatcher` has had `onDidDelete` called
  at least once, with a function argument.
- The registered callback is of type `function` (not `null` or `undefined`).

**Requirement:** REQ-VSCODE-043

---

### Test Case ID: TC-FSE-006
**Scenario:** Existing file deleted — all index records for that file removed

**Setup:**
- Initialize `WorkspaceIndexer` with a `NavigationIndex` pre-populated with records
  sourced from `"/workspace/foo.c"`:
  - A function definition record: `{ functionName: "MyHandler", file: "/workspace/foo.c", line: 5 }`
  - A vtable assignment record: `{ apiType: "JUNO_LOG_API_T", field: "LogInfo", functionName: "MyLogInfo", file: "/workspace/foo.c", line: 12 }`
- Also populate records from a different file `"/workspace/bar.c"` to confirm those are
  NOT removed:
  - A function definition record: `{ functionName: "OtherFunc", file: "/workspace/bar.c", line: 3 }`
- Fire the `onDidDelete` callback with a URI for `"/workspace/foo.c"`.
- Await the async handler to complete.

**Expected result:**
- `index.vtableAssignments` no longer contains any record with `file: "/workspace/foo.c"`.
- `index.functionDefinitions` no longer contains any record with `file: "/workspace/foo.c"`.
- Records sourced from `"/workspace/bar.c"` remain intact in the index.

**Requirement:** REQ-VSCODE-043

---

### Test Case ID: TC-FSE-007
**Scenario:** Deleted file's hash removed from cache and save scheduled

**Setup:**
- Initialize `WorkspaceIndexer` with a cache containing:
  - `cache.fileHashes["/workspace/foo.c"] = "abc123"`
- Use `jest.useFakeTimers()` to intercept the debounce timer used by `scheduleSave()`.
- Fire the `onDidDelete` callback with a URI for `"/workspace/foo.c"`.
- Await the async handler to complete (flush any microtasks with `await Promise.resolve()`).

**Expected result:**
- `cache.fileHashes["/workspace/foo.c"]` is `undefined` after the handler runs.
- `jest.getTimerCount()` is greater than 0, OR the save debounce spy has been called,
  confirming `scheduleSave()` was invoked. (Advance fake timers if needed to confirm
  the debounce timeout was set.)

**Requirement:** REQ-VSCODE-043

---

### Test Case ID: TC-FSE-008
**Scenario:** Delete sole implementation file — resolver returns `found: false`

**Setup:**
- Pre-populate `NavigationIndex` with:
  - Vtable assignment: `{ apiType: "JUNO_LOG_API_T", field: "LogInfo", functionName: "MyLogInfo", file: "/workspace/foo.c", line: 8 }`
  - Function definition: `{ functionName: "MyLogInfo", file: "/workspace/foo.c", line: 10, signature: "static void MyLogInfo(..." }`
  - API struct field: `apiStructFields["JUNO_LOG_API_T"] = ["LogInfo"]`
- Fire the `onDidDelete` callback with a URI for `"/workspace/foo.c"`.
- Await the async handler to complete.
- Construct a `VtableResolver` backed by the now-cleaned `NavigationIndex`.
- Call `vtableResolver.resolve()` for a call site with field `"LogInfo"` and
  `apiType: "JUNO_LOG_API_T"` (simulated from a different file, e.g. `"/workspace/caller.c"`).

**Expected result:**
- `result.found === false`
- `result.errorMsg` contains `"JUNO_LOG_API_T"` and `"LogInfo"` (confirming the resolver
  attempted to find an implementation and reported a meaningful error, not a silent failure).

**Requirement:** REQ-VSCODE-042, REQ-VSCODE-043

---

### Test Case ID: TC-FSE-009
**Scenario:** Boundary — file created then immediately deleted; `readFile` throws ENOENT; no crash, no stale entry

**Setup:**
- Inject a `readFile` stub that throws `Object.assign(new Error("ENOENT"), { code: 'ENOENT' })`
  when called with `"/workspace/ghost.c"`.
- Initialize `WorkspaceIndexer` with an empty `NavigationIndex` and an empty
  `cache.fileHashes` map.
- Fire the `onDidCreate` callback with a URI for `"/workspace/ghost.c"`.
- Await the async handler (including any `.catch()` microtask) by calling
  `await Promise.resolve()` after triggering the callback.

**Expected result:**
- No exception is propagated from the `onDidCreate` handler (the handler catches ENOENT
  internally and does not rethrow).
- `index.vtableAssignments` is empty — no partial records were inserted for `ghost.c`.
- `index.moduleRoots` is empty — no partial records were inserted for `ghost.c`.
- `index.functionDefinitions` is empty — no partial records were inserted for `ghost.c`.
- `cache.fileHashes["/workspace/ghost.c"]` is `undefined`.

**Requirement:** REQ-VSCODE-042

---

## Implementation Notes for Jest Developer

1. **FSW stub:** Use the existing `__mocks__/vscode.ts` FSW stub. Verify it exposes
   `onDidCreate` and `onDidDelete` as jest spy functions so the test can capture the
   registered callback and then invoke it directly.

2. **Index injection for TC-FSE-002/003/006/008:** Construct a `NavigationIndex` and inject
   it directly into `WorkspaceIndexer` via the constructor DI boundary. No actual file system
   access is needed for pre-population.

3. **`readFile` injection for TC-FSE-009:** Inject the `readFile` stub via the
   `WorkspaceIndexer` constructor or a test-accessible override parameter. Do NOT use
   `jest.mock('fs')` unless the production code reads files through the `fs` module directly;
   prefer DI injection through the same boundary used by production code.

4. **`scheduleSave()` verification:** Use `jest.useFakeTimers()` in TC-FSE-007. After firing
   the delete event, confirm the debounce timeout was scheduled by asserting
   `jest.getTimerCount() > 0` before advancing timers, or by spying on the underlying
   `setTimeout` call.

5. **Async flush:** For any test where the handler uses fire-and-forget
   `reindexFile().catch(...)` semantics, call `await Promise.resolve()` once after
   triggering the callback to flush the microtask queue before asserting state.

6. **Test doubles must be injected through the production DI boundary.** Do NOT use
   `jest.mock()` on internal modules. All doubles must be passed through the same
   constructor or configuration parameter that production code uses.
