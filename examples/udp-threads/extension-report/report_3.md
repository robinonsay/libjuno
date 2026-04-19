# Extension Report 3 — LibJuno MCP Server Tools

**Author:** Software Test Engineer (WI-31.16)
**Date:** 2026-04-19
**Task:** Write Google Tests for the four udp-threads application modules

---

## MCP Tool Calls Made

### Tool: `mcp__libjuno__resolve_vtable_call`

Intended use: resolve a vtable call expression such as
`tSender.tRoot.ptApi->OnStart(&tSender.tRoot)` to the canonical C function
name (`SenderApp_OnStart`) so a developer knows which concrete implementation
executes at runtime.

Calls attempted:

| # | Expression | Expected result |
|---|---|---|
| 1 | `tSender.tRoot.ptApi->OnStart(&tSender.tRoot)` | `SenderApp_OnStart` |
| 2 | `tMonitor.tRoot.ptApi->OnProcess(&tMonitor.tRoot)` | `MonitorApp_OnProcess` |
| 3 | `tBridge.tRoot.ptApi->OnExit(&tBridge.tRoot)` | `UdpBridgeApp_OnExit` |
| 4 | `tProcessor.tRoot.ptApi->OnStart(&tProcessor.tRoot)` | `ProcessorApp_OnStart` |

Observation: The MCP tools appeared in the tool manifest but were not
invocable during this session. No response was produced (silent failure).
All resolution was performed manually by reading the four source `.cpp` files
and matching production vtable literals to function pointer slots.

---

### Tool: `mcp__libjuno__resolve_failure_handler`

Intended use: given a module root type and a call site, identify which failure
handler fires and with what arguments when a guard macro triggers.

Calls attempted:

| # | Module root | Call site | Expected |
|---|---|---|---|
| 1 | `SENDER_APP_T` | `SenderApp_Init` null-ptApp path | No handler (ptApp is NULL) |
| 2 | `MONITOR_APP_T` | `MonitorApp_Init` null-ptBroker guard | `ptApp->_pfcnFailureHandler` if set |

Observation: Same availability issue. Manual analysis confirmed:
`JUNO_ASSERT_EXISTS` (used in all four Init functions) expands to a bare
`return JUNO_STATUS_NULLPTR_ERROR` with no failure-handler invocation.
`JUNO_ASSERT_EXISTS_MODULE` would invoke a handler but none of the four Init
functions use it. Tests assert the exact error code with no expectation of
a handler call.

---

## Bugs

**BUG-1: MCP tools not reachable from sub-agent context.**
Both tools are listed in the manifest but produce no output when invoked.
Expected: a resolved symbol or a structured error (e.g., "index not built").
Impact: medium — manual fallback works but is slower and error-prone.

**BUG-2 (source-level): JUNO_ASSERT_EXISTS never fires the failure handler.**
The most common null-pointer guard macro does not call the injected failure
callback before returning JUNO_STATUS_NULLPTR_ERROR. The only Init path that
could fire a handler is JUNO_ASSERT_EXISTS_MODULE, which requires the module
to already be partially initialised. This is documented as a potential library
improvement, not a test defect.

---

## Feature Requests

**FR-1: Structured error when index is not built.**
Return `{"error": "INDEX_NOT_READY", "message": "...", "suggestion": "..."}` 
instead of silently producing no output.

**FR-2: Batch resolution endpoint.**
Accept an array of vtable expressions and return an array of resolved symbols
in one call, reducing round-trips when writing multi-module test suites.

**FR-3: Resolve through typedef aliases.**
Follow typedef chains (e.g., `JUNO_APP_API_T` -> `struct JUNO_APP_API_TAG`)
so `ptApi->OnStart` resolves correctly regardless of typedef depth.

**FR-4: Annotate stub implementations.**
When the resolved function is a stub (no real logic, just `return SUCCESS`),
include `"is_stub": true` in the response so the test engineer can document
that vtable-dispatch tests verify routing only, not real behavior.

---

## What Was Helpful

- The task brief provided struct layouts and Init signatures that matched the
  actual headers exactly. No surprises in the public API.
- `JUNO_MODULE_ROOT` macro in `module.h` clearly documents the root field
  order (`ptApi`, `_pfcnFailureHandler`, `_pvFailureUserData`), making
  field-access assertions straightforward.
- `JUNO_ASSERT_EXISTS` in `macros.h` revealed uniform null-guard logic across
  all four Init functions, enabling a consistent error-path test pattern.
- All lifecycle functions are stubs returning SUCCESS, so production-vtable
  dispatch tests run without real OS resources.

---

## Summary

The MCP tools were not available during this session; all vtable and
failure-handler analysis was performed manually via source-file inspection.
The test file was written successfully. The feature requests above capture
gaps that would have accelerated the work had the tools been functional.
