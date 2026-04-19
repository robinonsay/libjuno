# LibJuno Extension MCP Server — Tool Observation Report

**Author:** Software Test Engineer (WI-31.14)
**Date:** 2026-04-19
**Context:** Writing Google Tests for the UDP module (`examples/udp-threads/tests/test_udp_module.cpp`)

---

## 1. Tool Invocations and Results

### 1.1 `resolve_vtable_call` — Discovering Required Parameters

First call passed only an `expression` field.

**Result:** Error `-32602 Invalid params: required fields file (string), line (number), column (number), lineText (string).`

**Observation:** The error message is clear and actionable.

---

### 1.2 `resolve_vtable_call` — Test File (Not Yet Indexed)

Called with the stub test file path, lineText `tUdp.tRoot.ptApi->Open(&tUdp.tRoot, &tCfg);`

**Result:** `{"found": false, "errorMsg": "No API type contains field 'Open'."}`

**Observation:** The stub test file was not indexed. The tool had no type information for `JUNO_UDP_API_T` in that file context. Expected behavior, but the error message is misleading (see BUG-1).

---

### 1.3 `resolve_vtable_call` — Source File With No Concrete Vtable Assignment

Called on `/workspaces/libjuno/examples/udp-threads/src/juno_udp_init.cpp` with `ptRoot->ptApi->Open(...)` as lineText.

**Result:** `{"found": false, "errorMsg": "No vtable assignments found for 'JUNO_UDP_API_T'."}`

**Observation:** The tool correctly identified the API type from the index but found no `const JUNO_UDP_API_T g_xyz = { ... }` assignment because `linux_udp_impl.cpp` is a stub. Accurate and informative.

---

### 1.4 `resolve_vtable_call` — Working Example (juno_buff_stack.c)

Called on `/workspaces/libjuno/src/juno_buff_stack.c:37`, lineText `ptBuffer->ptApi->SetAt(...)`.

**Result:** `found: true` — 10 implementation locations returned with full metadata including `functionName`, `file`, `line`, `assignmentFile`, `assignmentLine`, `apiVarName`, `initCallFile`, `initCallLine`, `compRootFile`, `compRootLine`.

**Observation:** Tool works correctly when the API type has concrete vtable assignments in the index.

---

### 1.5 `resolve_failure_handler` — juno_udp_init.cpp

Called with lineText `ptRoot->JUNO_FAILURE_HANDLER = pfcnFailureHandler;`

**Result:** `found: true` — three locations:
1. `juno_udp_init.cpp:60`
2. `sender_app.cpp:77`
3. `udp_bridge_app.cpp:60`

**Observation:** Correctly finds all sites where the `pfcnFailureHandler` parameter is wired into a module root.

---

## 2. Bugs Observed

### BUG-1: Misleading error message when file context lacks type info

Error: `"No API type contains field 'Open'."`

This implies globally no API type has a field `Open`, but the real problem is the file context has not been indexed. The message should say: `"No indexed API type in this file's context contains field 'Open'. Ensure the file has been compiled and indexed."`

---

### BUG-2: Relative vs. absolute path produces silent different failures

Passing a relative path produces a different (and less informative) error than an absolute path. The tool should validate path format and return `"file must be an absolute path"` immediately if a relative path is detected.

---

## 3. Feature Requests

### FR-1: Include known API type fields in the not-found response

When the API type is identified but has no vtable assignments, return the known fields:

```json
{
  "found": false,
  "apiType": "JUNO_UDP_API_T",
  "knownFields": ["Open", "Send", "Receive", "Close"],
  "errorMsg": "No vtable assignments found for 'JUNO_UDP_API_T'."
}
```

### FR-2: Explain why no vtable assignment was found

Add a `hint` field: `"Check that the implementing .c/.cpp file is within the indexer's source roots and has been built."` This prevents guessing whether the vtable is missing, misnamed, or outside scan scope.

### FR-3: Distinguish assignment vs. call-site in `resolve_failure_handler` results

Add a `"kind"` field (`"assignment"` or `"invocation"`) to each result location so callers can distinguish wiring sites from usage sites at a glance.

### FR-4: Accept cursor column anywhere within the vtable call expression

Currently the column must fall within a specific regex match window that includes the receiver (`ptApi->`). If the cursor sits on just the field name (`Open`), the match fails. Accepting any column within the full call expression would be more ergonomic.

---

## 4. What Was Helpful

- **Clear required-field error on missing params**: The `-32602` with the explicit field list gave immediate actionable guidance.
- **`resolve_vtable_call` rich metadata when successful**: `assignmentFile`, `assignmentLine`, `apiVarName`, `initCallFile`, `initCallLine`, `compRootFile`, `compRootLine` provide a complete navigation trail from call site to composition root.
- **`resolve_failure_handler` finds all wiring sites**: Given a handler assignment line, the tool finds every module that stores that handler — useful for tracing which modules consume a given failure callback.
- **Distinct error messages**: Separating `"No API type contains field X"` from `"No vtable assignments found for TYPE"` distinguishes two different root causes, which is better than a single generic error.
