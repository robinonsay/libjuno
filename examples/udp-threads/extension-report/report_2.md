# Extension Report 2 — MCP Tool Evaluation: Thread Module Test Session

**Date:** 2026-04-19
**Author:** Software Test Engineer (claude-sonnet-4-6)
**Task:** WI-31.15 — Write `tests/test_thread_module.cpp`

---

## 1. MCP Tool Calls Attempted and Results

The brief mandated use of two MCP tools when writing vtable dispatch code:

| Tool | Expression Provided | Result |
|------|---------------------|--------|
| `mcp__libjuno__resolve_vtable_call` | `ptRoot->ptApi->Create(ptRoot, pfcnEntry, pvArg)` | **Tool not available** — `No such tool available` |
| `mcp__libjuno__resolve_failure_handler` | `ptRoot->_pfcnFailureHandler(tStatus, pcMessage, ptRoot->_pvFailureUserData)` | **Tool not available** — `No such tool available` |

Both tools returned `tool_use_error` indicating they are not registered in the current agent environment.
The tools were invoked exactly as specified in the brief before writing any vtable dispatch code.

---

## 2. Bugs and Unexpected Behavior

### 2.1 Tools not registered in agent environment

Both `mcp__libjuno__resolve_vtable_call` and `mcp__libjuno__resolve_failure_handler` are absent from
the MCP tool registry available to this agent instance. The brief states "You must use them when writing
vtable call code" but the tools could not be invoked.

**Workaround used:** The vtable call canonical forms were derived manually by reading
`include/juno/thread.h` (inline wrappers) and `src/juno_thread_init.cpp` (init implementation).
Specifically:

- `JunoThread_Create` dispatches via `ptRoot->ptApi->Create(ptRoot, pfcnEntry, pvArg)` (thread.h:216)
- `JunoThread_Stop` dispatches via `ptRoot->ptApi->Stop(ptRoot)` (thread.h:229)
- `JunoThread_Join` dispatches via `ptRoot->ptApi->Join(ptRoot)` (thread.h:241)
- Failure handler stored in `ptRoot->_pfcnFailureHandler`; user data in `ptRoot->_pvFailureUserData`
  (confirmed from juno_thread_init.cpp lines 60-61)

### 2.2 `linux_thread_impl.cpp` is a stub

The file `src/linux_thread_impl.cpp` contains only `// Stub implementation - to be completed`.
This means `g_junoThreadLinuxApi` vtable functions are undefined at link time for `udp_threads_lib`.
The test binary links correctly because tests use the fake vtable `s_tTestApi` exclusively and never
reference `g_junoThreadLinuxApi`.

The `udp_threads_main` executable has a pre-existing linker failure (`undefined reference to 'main'`)
unrelated to this test file.

### 2.3 `REQ-THREAD-015` cannot be verified against the real implementation

Because `linux_thread_impl.cpp` is a stub, the double-create guard returning
`JUNO_STATUS_REF_IN_USE_ERROR` cannot be verified against production code. A second fake vtable
(`s_tGuardApi`) was defined that implements the guard contract. A comment in the test directs future
maintainers to switch to `g_junoThreadLinuxApi` once the implementation is complete.

---

## 3. Feature Requests

### 3.1 `resolve_vtable_call` — accept a module name instead of a raw expression

Currently the tool takes an `expression` and `file_path`. It would be more useful if it also accepted a
`module_name` string (e.g., `"THREAD"`) and resolved the canonical vtable member from the module
registry. This would be useful during test-planning before code is written.

### 3.2 `resolve_failure_handler` — return the expanded macro form

When provided with a root type name (e.g., `JUNO_THREAD_ROOT_T`), the tool should return the fully
expanded `JUNO_FAIL_ROOT(...)` call the module uses. This removes the need to read macro definitions
manually to write precise handler-invocation assertions.

### 3.3 Graceful degradation with a structured warning

When a tool is unavailable, returning a structured warning object (rather than a hard error) would allow
automated pipelines to distinguish "tool absent — proceed manually" from "tool call failed due to bad
input".

---

## 4. What Was Helpful

Even without working MCP tools, the brief provided precise context:

- Exact vtable field names (`Create`, `Stop`, `Join`), root field names (`_uHandle`, `bStop`,
  `_pfcnFailureHandler`, `_pvFailureUserData`), and the `ptApi` member were all specified correctly
  and matched the actual header verbatim, requiring no corrections.
- The fake vtable template was accurate and required no modifications after reading the real header.
- The `// @{"verify": ["REQ-THREAD-NNN"]}` tagging convention is simple and worked seamlessly once
  REQ IDs were confirmed from `requirements/thread/requirements.json`.

---

## 5. Test Summary

15 tests written and passing (run via `udp_threads_tests --gtest_filter=ThreadModule*`):

| Test | REQ Covered | Scenario |
|------|-------------|----------|
| `Init_NullRoot_ReturnsNullptrError` | REQ-THREAD-012 | NULL root returns JUNO_STATUS_NULLPTR_ERROR |
| `Init_NullApi_ReturnsNullptrError` | REQ-THREAD-012 | NULL api returns JUNO_STATUS_NULLPTR_ERROR |
| `Init_HappyPath_WiresVtableAndClearsState` | REQ-THREAD-012 | Vtable wired, handle=0, bStop=false |
| `Init_NullFailureHandler_IsAccepted` | REQ-THREAD-012 | NULL handler optional — succeeds |
| `Init_StoresFailureHandler` | REQ-THREAD-013 | Handler pointer stored in root |
| `Create_DispatchesViaVtable_HandleSet` | REQ-THREAD-003 | Create dispatches vtable, handle non-zero |
| `Create_ForwardsEntryAndArg` | REQ-THREAD-003 | Entry function and arg forwarded |
| `Stop_DispatchesViaVtable_SetsBStopTrue` | REQ-THREAD-006 | Stop dispatches vtable, bStop=true |
| `Join_DispatchesViaVtable_ClearsHandle` | REQ-THREAD-005 | Join dispatches vtable, handle reset to 0 |
| `BStop_ReadableAfterStop_TrueViaRootPointer` | REQ-THREAD-007 | bStop visible via root pointer |
| `Create_WhenVtableReturnsError_PropagatesExactStatus` | REQ-THREAD-010 | Create error propagated exactly |
| `Stop_WhenVtableReturnsError_PropagatesExactStatus` | REQ-THREAD-010 | Stop error propagated exactly |
| `Join_WhenVtableReturnsError_PropagatesExactStatus` | REQ-THREAD-010 | Join error propagated exactly |
| `Create_WhenAlreadyRunning_ReturnsRefInUseError` | REQ-THREAD-015 | Double-create returns REF_IN_USE |
| `FullLifecycle_CreateStopJoin_StateConsistent` | REQ-THREAD-003,005,006 | Full lifecycle, no handler calls |
