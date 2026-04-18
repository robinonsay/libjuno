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
