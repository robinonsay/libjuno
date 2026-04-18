> Part of: [Test Case Specification](index.md) — Section 10: visitFailureHandlerAssignment

## Section 10: visitFailureHandlerAssignment — Failure Handler

**Visitor:** `visitFailureHandlerAssignment`  
**Token:** `JunoFailureHandler` — alternation pattern `/JUNO_FAILURE_HANDLER\b|_pfcnFailureHandler\b/`  
**Description:** The visitor matches assignments to either `JUNO_FAILURE_HANDLER` (macro form) or
`_pfcnFailureHandler` (expanded member name) using a single unified token. Both forms are handled
identically — no dual-pattern workaround is required.  
**Requirement:** REQ-VSCODE-016

---

### Test Case ID: TC-P10-001
**Visitor:** `visitFailureHandlerAssignment` — Direct assignment, heap module  
**Source:** `src/juno_heap.c`, line 40  
**Input line:**
```c
    ptHeap->_pfcnFailureHandler = pfcnFailureHdlr;
```
**Expected visitor extraction:**
- variableName: `ptHeap`
- functionName: `pfcnFailureHdlr`

**Expected record:**
```typescript
{ variableName: "ptHeap", functionName: "pfcnFailureHdlr" }
```
**Requirement:** REQ-VSCODE-016

---

### Test Case ID: TC-P10-002
**Visitor:** `visitFailureHandlerAssignment` — Direct assignment via JUNO_FAILURE_HANDLER macro  
**Source:** `examples/example_project/engine/src/engine_app.c`, line 111  
**Input line:**
```c
    ptEngineApp->tRoot.JUNO_FAILURE_HANDLER = pfcnFailureHandler;
```
**Analysis:** The `JunoFailureHandler` token uses the alternation pattern
`/JUNO_FAILURE_HANDLER\b|_pfcnFailureHandler\b/`, matching both the macro form and the
expanded member name as a single token. The visitor handles chained member access
(`->tRoot.JUNO_FAILURE_HANDLER`) natively via the CST.

**Expected record:**
```typescript
{ variableName: "ptEngineApp", functionName: "pfcnFailureHandler" }
```
**Note:** Validates that the JunoFailureHandler alternation token handles the macro form correctly.
Previously a known gap; now closed by the unified token.  
**Requirement:** REQ-VSCODE-016

---

### Test Case ID: TC-P10-003
**Visitor:** `visitFailureHandlerAssignment` — Negative: non-failure-handler assignment must not match  
**Source:** `src/juno_heap.c` Init body *(similar line)*  
**Input line:**
```c
    ptHeap->ptApi = &tHeapApi;
```
**Expected:** no match (field name is `ptApi`, not `_pfcnFailureHandler`)  
**Requirement:** REQ-VSCODE-016

---

## Section 10b: FAIL Macro Failure Handler Navigation (REQ-VSCODE-022–026)

**Resolver:** `FailureHandlerResolver` — §5.3.1 FAIL Macro Call Site Resolution  
**Description:** These test cases verify the FAIL macro call site recognition and resolution path
described in design §5.3.1. Recognition fires at query time when the line text matches one of the
four FAIL macro patterns. Resolution uses the same `functionDefinitions` and
`failureHandlerAssignments` index data as the existing §5.3 algorithm.  
**Requirements:** REQ-VSCODE-022, REQ-VSCODE-023, REQ-VSCODE-024, REQ-VSCODE-025, REQ-VSCODE-026

**Test double approach:** Inject a pre-populated `NavigationIndex` stub and a `localTypeInfo` stub
into `FailureHandlerResolver`. Supply the `lineText` directly as a string — no file system access
required for unit tests in this section.

---

### TC-FAIL-001: JUNO_FAIL — handler name found in functionDefinitions

**Requirement:** REQ-VSCODE-023  
**Scenario:** Cursor on a `JUNO_FAIL` call site; the second argument is a bare function-pointer
identifier that exists in `functionDefinitions`; resolver navigates directly to the handler.  
**Index state:**
- `functionDefinitions`:
  ```typescript
  "MyFailureHandler": [{ functionName: "MyFailureHandler", file: "src/module.c", line: 55 }]
  ```
- `failureHandlerAssignments`: empty (not consulted for `JUNO_FAIL`)
- `derivationChain`: empty (not consulted for `JUNO_FAIL`)
- `localTypeInfo`: empty (not consulted for `JUNO_FAIL`)

**Input:**
```
file:       "src/caller.c"
line:       87
column:     4   (cursor anywhere on the JUNO_FAIL token)
lineText:   "    JUNO_FAIL(eStatus, MyFailureHandler, NULL, \"operation failed\");"
functionName: "MyFailureHandler"  (extracted from arg[1])
```

**Expected:**
```typescript
{ found: true, locations: [{ functionName: "MyFailureHandler", file: "src/module.c", line: 55 }] }
```

**Notes:** The resolver extracts `MyFailureHandler` as arg[1] (0-indexed) of the comma-separated
argument list and looks it up in `functionDefinitions` directly. No derivation chain walk is
performed.

---

### TC-FAIL-002: JUNO_FAIL — unknown handler name not in functionDefinitions

**Requirement:** REQ-VSCODE-023  
**Scenario:** Cursor on a `JUNO_FAIL` call site; the handler name in arg[1] does not appear in
`functionDefinitions`; resolver returns `found: false`.  
**Index state:**
- `functionDefinitions`: empty map (no entry for `UnknownHandler`)
- `failureHandlerAssignments`: empty
- `derivationChain`: empty
- `localTypeInfo`: empty

**Input:**
```
file:       "src/caller.c"
line:       102
column:     4
lineText:   "    JUNO_FAIL(eStatus, UnknownHandler, pvUserData, \"msg\");"
functionName: "UnknownHandler"  (extracted from arg[1])
```

**Expected:**
```typescript
{ found: false, errorMsg: "No definition found for failure handler 'UnknownHandler'." }
```

**Notes:** The error message must name the identifier that was looked up so the developer can
identify the missing handler.

---

### TC-FAIL-003: JUNO_FAIL_MODULE — derived type walks chain to find registered handler

**Requirement:** REQ-VSCODE-024  
**Scenario:** Cursor on a `JUNO_FAIL_MODULE` call site; arg[1] is a pointer to a derived module
type; the resolver walks the derivation chain to the root type and finds the registered handler.  
**Index state:**
- `derivationChain`:
  ```typescript
  "ENGINE_APP_T" → "JUNO_APP_ROOT_T"
  ```
- `failureHandlerAssignments`:
  ```typescript
  "JUNO_APP_ROOT_T": [{ functionName: "EngineFailureHandler", file: "engine/src/engine_app.c", line: 111 }]
  ```
- `functionDefinitions`: `{ "OtherFunc": [{ functionName: "OtherFunc", file: "other.c", line: 5, isStatic: false }] }` (non-empty; injected to verify resolver does NOT consult functionDefinitions for JUNO_FAIL_MODULE — result must still be found: true via failureHandlerAssignments)
- `localTypeInfo` for containing function:
  ```typescript
  "ptEngineApp": { type: "ENGINE_APP_T", isPointer: true }
  ```

**Input:**
```
file:       "engine/src/engine_app.c"
line:       210
column:     4
lineText:   "    JUNO_FAIL_MODULE(eStatus, ptEngineApp, \"init failed\");"
```

**Expected:**
```typescript
{ found: true, locations: [{ functionName: "EngineFailureHandler", file: "engine/src/engine_app.c", line: 111 }] }
```

**Notes:** The resolver resolves `ptEngineApp` → `ENGINE_APP_T` via localTypeInfo, then walks
`derivationChain` to `JUNO_APP_ROOT_T`, then looks up `failureHandlerAssignments["JUNO_APP_ROOT_T"]`.

---

### TC-FAIL-004: JUNO_FAIL_MODULE — no handler registered for resolved root type

**Requirement:** REQ-VSCODE-024  
**Scenario:** Cursor on a `JUNO_FAIL_MODULE` call site; derivation chain resolves successfully but
`failureHandlerAssignments` has no entry for the root type.  
**Index state:**
- `derivationChain`:
  ```typescript
  "MY_SENSOR_T" → "JUNO_APP_ROOT_T"
  ```
- `failureHandlerAssignments`: empty (no entry for `JUNO_APP_ROOT_T`)
- `localTypeInfo` for containing function:
  ```typescript
  "ptSensor": { type: "MY_SENSOR_T", isPointer: true }
  ```

**Input:**
```
file:       "sensors/src/sensor.c"
line:       78
column:     4
lineText:   "    JUNO_FAIL_MODULE(eStatus, ptSensor, \"sensor error\");"
```

**Expected:**
```typescript
{ found: false, errorMsg: "No failure handler registered for 'JUNO_APP_ROOT_T'." }
```

**Notes:** The error message identifies the resolved root type, not the original declared type, so
the developer knows which handler registration is missing.

---

### TC-FAIL-005: JUNO_FAIL_ROOT — root type pointer directly, no chain walk, handler found

**Requirement:** REQ-VSCODE-025  
**Scenario:** Cursor on a `JUNO_FAIL_ROOT` call site; arg[1] is a pointer to a root type directly
(no derivation chain walk needed); resolver finds the registered handler.  
**Index state:**
- `derivationChain`: empty (not consulted for `JUNO_FAIL_ROOT` — root type already known)
- `failureHandlerAssignments`:
  ```typescript
  "JUNO_LOG_ROOT_T": [{ functionName: "LogFailureHandler", file: "src/logger.c", line: 33 }]
  ```
- `localTypeInfo` for containing function:
  ```typescript
  "ptLogRoot": { type: "JUNO_LOG_ROOT_T", isPointer: true }
  ```

**Input:**
```
file:       "src/logger.c"
line:       120
column:     4
lineText:   "    JUNO_FAIL_ROOT(eStatus, ptLogRoot, \"logger failure\");"
```

**Expected:**
```typescript
{ found: true, locations: [{ functionName: "LogFailureHandler", file: "src/logger.c", line: 33 }] }
```

**Notes:** Unlike `JUNO_FAIL_MODULE`, `JUNO_FAIL_ROOT` does not walk the derivation chain. The
resolved type from localTypeInfo is used directly as the root type for the
`failureHandlerAssignments` lookup.

---

### TC-FAIL-006: JUNO_FAIL_ROOT — no handler registered for root type

**Requirement:** REQ-VSCODE-025  
**Scenario:** Cursor on a `JUNO_FAIL_ROOT` call site; localTypeInfo resolves the pointer to a root
type but `failureHandlerAssignments` has no entry for it.  
**Index state:**
- `derivationChain`: empty
- `failureHandlerAssignments`: empty (no entry for `JUNO_DS_HEAP_ROOT_T`)
- `localTypeInfo` for containing function:
  ```typescript
  "ptHeapRoot": { type: "JUNO_DS_HEAP_ROOT_T", isPointer: true }
  ```

**Input:**
```
file:       "src/heap_usage.c"
line:       45
column:     4
lineText:   "    JUNO_FAIL_ROOT(eStatus, ptHeapRoot, \"heap overflow\");"
```

**Expected:**
```typescript
{ found: false, errorMsg: "No failure handler registered for 'JUNO_DS_HEAP_ROOT_T'." }
```

**Notes:** No derivation chain walk occurs for `JUNO_FAIL_ROOT`. The resolver uses the type
resolved from localTypeInfo directly.

---

### TC-FAIL-007: JUNO_ASSERT_EXISTS_MODULE — derived type walks chain to find handler (found)

**Requirement:** REQ-VSCODE-026  
**Scenario:** Cursor on a `JUNO_ASSERT_EXISTS_MODULE` call site; arg[1] is a derived module
pointer; the resolver walks the derivation chain to the root type and finds the handler — same
algorithm as TC-FAIL-003 but using the `JUNO_ASSERT_EXISTS_MODULE` macro form.  
**Index state:**
- `derivationChain`:
  ```typescript
  "JUNO_SB_PIPE_T" → "JUNO_DS_QUEUE_ROOT_T"
  ```
- `failureHandlerAssignments`:
  ```typescript
  "JUNO_DS_QUEUE_ROOT_T": [{ functionName: "QueueFailureHandler", file: "src/juno_queue.c", line: 28 }]
  ```
- `localTypeInfo` for containing function:
  ```typescript
  "ptPipe": { type: "JUNO_SB_PIPE_T", isPointer: true }
  ```

**Input:**
```
file:       "src/broker.c"
line:       66
column:     4
lineText:   "    JUNO_ASSERT_EXISTS_MODULE(ptPipe != NULL, ptPipe, \"pipe must exist\");"
```

**Expected:**
```typescript
{ found: true, locations: [{ functionName: "QueueFailureHandler", file: "src/juno_queue.c", line: 28 }] }
```

**Notes:** `JUNO_ASSERT_EXISTS_MODULE` uses the same derivation-chain algorithm as
`JUNO_FAIL_MODULE`. The first argument (`ptPipe != NULL`) is a boolean condition; the second
argument (`ptPipe`) is the module pointer whose type is resolved.

---

### TC-FAIL-008: JUNO_ASSERT_EXISTS_MODULE — no handler registered

**Requirement:** REQ-VSCODE-026  
**Scenario:** Cursor on a `JUNO_ASSERT_EXISTS_MODULE` call site; derivation chain resolves
successfully but no failure handler is registered for the root type.  
**Index state:**
- `derivationChain`:
  ```typescript
  "MY_IMPL_T" → "JUNO_POINTER_ROOT_T"
  ```
- `failureHandlerAssignments`: empty (no entry for `JUNO_POINTER_ROOT_T`)
- `localTypeInfo` for containing function:
  ```typescript
  "ptImpl": { type: "MY_IMPL_T", isPointer: true }
  ```

**Input:**
```
file:       "src/init.c"
line:       91
column:     4
lineText:   "    JUNO_ASSERT_EXISTS_MODULE(ptImpl != NULL, ptImpl, \"impl required\");"
```

**Expected:**
```typescript
{ found: false, errorMsg: "No failure handler registered for 'JUNO_POINTER_ROOT_T'." }
```

**Notes:** Same algorithm as `JUNO_FAIL_MODULE` error path (TC-FAIL-004). The error message names
the root type reached after walking the chain, not the original declared type.

---

### TC-FAIL-009: Non-macro line — falls through to §5.3 algorithm

**Requirement:** REQ-VSCODE-022  
**Scenario:** Line text does not match any of the four FAIL macro patterns (but contains
`_pfcnFailureHandler`); Step 0 of §5.3.1 finds no match and falls through to the existing §5.3
failure handler resolution algorithm.  
**Index state:**
- `failureHandlerAssignments`:
  ```typescript
  "JUNO_DS_HEAP_ROOT_T": [{ functionName: "HeapFailureHandler", file: "src/heap.c", line: 77 }]
  ```
- `functionDefinitions`: empty (§5.3 fall-through path does not consult functionDefinitions)
- `localTypeInfo` for containing function:
  ```typescript
  "ptHeap": { type: "JUNO_DS_HEAP_ROOT_T", isPointer: true }
  ```

**Input:**
```
file:       "src/init.c"
line:       40
column:     12  (cursor on _pfcnFailureHandler token)
lineText:   "    ptHeap->_pfcnFailureHandler = HeapFailureHandler;"
```

**Expected:**
```typescript
{ found: true, locations: [{ functionName: "HeapFailureHandler", file: "src/heap.c", line: 77 }] }
```

**Notes:** This test confirms that the FAIL macro check in Step 0 does NOT intercept non-FAIL-macro
lines. The line does not match `/\bJUNO_FAIL\s*\(/`, `/\bJUNO_FAIL_MODULE\s*\(/`,
`/\bJUNO_FAIL_ROOT\s*\(/`, or `/\bJUNO_ASSERT_EXISTS_MODULE\s*\(/`, so the resolver falls through
to the §5.3 cursor-on-JunoFailureHandler path and resolves correctly.

---

### TC-FAIL-010: JUNO_FAIL — nested parentheses in arg[1] (not a bare identifier)

**Requirement:** REQ-VSCODE-023  
**Scenario:** The second argument of `JUNO_FAIL` is a function call expression rather than a bare
identifier; the resolver cannot look it up in `functionDefinitions` and returns `found: false`.  
**Index state:**
- `functionDefinitions`: populated with various entries but none matching the compound expression
- `failureHandlerAssignments`: empty
- `derivationChain`: empty
- `localTypeInfo`: empty

**Input:**
```
file:       "src/caller.c"
line:       130
column:     4
lineText:   "    JUNO_FAIL(eStatus, getHandler(ptMod), NULL, \"msg\");"
```

**Expected:**
```typescript
{ found: false, errorMsg: "No definition found for failure handler 'getHandler(ptMod)'." }
```

**Notes:** The argument extraction uses balanced-parenthesis tracking. `getHandler(ptMod)` is a
non-trivial expression, not a bare identifier. The resolver attempts a lookup of the full trimmed
token `getHandler(ptMod)` in `functionDefinitions`, finds no match, and returns `found: false`.
This documents that only bare identifiers (simple function pointer variable names or function names)
are supported for `JUNO_FAIL` resolution; compound expressions are gracefully unresolvable.

---

### TC-FAIL-011: JUNO_FAIL_MODULE — cast expression in arg[1] is stripped to bare identifier

**Requirement:** REQ-VSCODE-024  
**Scenario:** The second argument of `JUNO_FAIL_MODULE` contains a C cast expression; the resolver
strips the cast to obtain the bare identifier and resolves the type via localTypeInfo and the
derivation chain.  
**Index state:**
- `derivationChain`:
  ```typescript
  "MY_MOD_T" → "JUNO_APP_ROOT_T"
  ```
- `failureHandlerAssignments`:
  ```typescript
  "JUNO_APP_ROOT_T": [{ functionName: "AppFailureHandler", file: "src/app.c", line: 19 }]
  ```
- `localTypeInfo` for containing function:
  ```typescript
  "ptMod": { type: "MY_MOD_T", isPointer: true }
  ```

**Input:**
```
file:       "src/app.c"
line:       155
column:     4
lineText:   "    JUNO_FAIL_MODULE(eStatus, (MY_MOD_T *)ptMod, \"cast call\");"
```

**Expected:**
```typescript
{ found: true, locations: [{ functionName: "AppFailureHandler", file: "src/app.c", line: 19 }] }
```

**Notes:** Per §5.3.1 Step 1, the resolver strips cast expressions of the form `(TYPE *)identifier`
from the extracted argument before looking up the type in localTypeInfo. The bare identifier
`ptMod` is what gets resolved. This test confirms the cast-stripping logic works end-to-end.

---

### TC-FAIL-012: JUNO_FAIL_MODULE — two-level derivation chain walks to root type

**Requirement:** REQ-VSCODE-024  
**Scenario:** The second argument of `JUNO_FAIL_MODULE` has a type that is two derivation levels
deep; the resolver iterates the chain twice to reach the root type.  
**Index state:**
- `derivationChain`:
  ```typescript
  "TYPE_A_T" → "TYPE_B_T"
  "TYPE_B_T" → "ROOT_T"
  ```
- `failureHandlerAssignments`:
  ```typescript
  "ROOT_T": [{ functionName: "RootFailureHandler", file: "src/root_module.c", line: 9 }]
  ```
- `localTypeInfo` for containing function:
  ```typescript
  "ptTypeA": { type: "TYPE_A_T", isPointer: true }
  ```

**Input:**
```
file:       "src/deep_caller.c"
line:       200
column:     4
lineText:   "    JUNO_FAIL_MODULE(eStatus, ptTypeA, \"deep error\");"
```

**Expected:**
```typescript
{ found: true, locations: [{ functionName: "RootFailureHandler", file: "src/root_module.c", line: 9 }] }
```

**Notes:** The WHILE loop in §5.3.1 Step 2 iterates:
1. `current = "TYPE_A_T"` → `derivationChain.has("TYPE_A_T")` = true → `current = "TYPE_B_T"`
2. `current = "TYPE_B_T"` → `derivationChain.has("TYPE_B_T")` = true → `current = "ROOT_T"`
3. `current = "ROOT_T"` → `derivationChain.has("ROOT_T")` = false → loop exits
`rootType = "ROOT_T"`. The handler lookup then succeeds. This verifies the chain walk is unbounded
(not hardcoded to one hop).

---
