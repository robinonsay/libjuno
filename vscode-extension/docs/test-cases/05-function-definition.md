> Part of: [Test Case Specification](index.md) — Section 11: visitFunctionDefinition

## Section 11: visitFunctionDefinition — Function Definition

**Visitor:** `visitFunctionDefinition`  
**CST path:** `functionDefinition` → `declarator` → `directDeclarator` (function name)  
**Description:** The visitor captures function name, static qualifier, and source location for each
function definition. Because the Chevrotain grammar is whitespace-insensitive, both K&R
(`){ ... }` on same line) and Allman (`)
{`) brace styles are handled natively — no special
lookahead is required.  
**Requirement:** REQ-VSCODE-003 (dependency — visitFunctionDefinition resolves declaration line for vtable assignments)

---

### Test Case ID: TC-P11-001
**Visitor:** `visitFunctionDefinition` — Static function with complex return type (Lesson Learned #8)  
**Source:** `examples/example_project/main.c`, line 60  
**Function name substituted:** `LogInfo`  
**Input line:**
```c
static JUNO_STATUS_T LogInfo(const JUNO_LOG_ROOT_T *ptJunoLog, const char *pcMsg, ...) {
```
**Expected:** MATCH at line 60  
**Captured group 1:** `LogInfo`  
**isStatic:** `true` (leading `static` keyword present)  
**Lesson Learned:** #8 — varied return types  
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-P11-002
**Visitor:** `visitFunctionDefinition` — Non-static public function  
**Source:** Line numbers within `src/juno_heap.c` (exact line TBD during indexing; function signature from `include/juno/ds/heap_api.h`)  
**Function name substituted:** `JunoDs_Heap_Insert`  
**Input line:**
```c
JUNO_STATUS_T JunoDs_Heap_Insert(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tValue) {
```
**Expected:** MATCH  
**isStatic:** `false`  
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-P11-003
**Visitor:** `visitFunctionDefinition` — `static inline` function, Allman brace style (Lesson Learned #8)  
**Source:** `examples/example_project/engine/src/engine_app.c`, line 68 (Verify function)  
**Function name substituted:** `Verify`  
**Input line:**
```c
static inline JUNO_STATUS_T Verify(JUNO_APP_ROOT_T *ptJunoApp)
{
```
**Note:** The `{` appears on the NEXT line (Allman brace style). The Chevrotain grammar is
whitespace-insensitive: `declarator` ends at `)` and `compoundStatement` begins at the next `{`,
regardless of intervening newlines. No special lookahead is needed.
**Expected:** MATCH — the grammar handles Allman brace style natively.
**isStatic:** `true`
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-P11-004
**Visitor:** `visitFunctionDefinition` — Non-static `void` return type failure handler (Lesson Learned #8)  
**Source:** `examples/example_project/main.c`, line 163  
**Function name substituted:** `FailureHandler`  
**Input line:**
```c
void FailureHandler(JUNO_STATUS_T tStatus, const char *pcMsg, JUNO_USER_DATA_T *pvUserData) {
```
**Expected:** MATCH  
**isStatic:** `false`  
**Lesson Learned:** #8  
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-P11-005
**Visitor:** `visitFunctionDefinition` — Non-standard result return type (Lesson Learned #8)  
**Source:** `examples/example_project/main.c`, around line 110  
**Function name substituted:** `Now`  
**Input line:**
```c
static JUNO_TIMESTAMP_RESULT_T Now(const JUNO_TIME_ROOT_T *ptTime) {
```
**Expected:** MATCH  
**isStatic:** `true`  
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-P11-006
**Visitor:** `visitFunctionDefinition` — Negative: forward declaration (ends with `;`) must NOT match (Lesson Learned #6)  
**Source:** `src/juno_broker.c`, line 25  
**Function name substituted:** `Publish`  
**Input line:**
```c
static JUNO_STATUS_T Publish(JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_SB_MID_T tMid, JUNO_POINTER_T tMsg);
```
**Expected:** NO MATCH — forward declarations end with `;` and parse as `declaration`, not `functionDefinition`. The grammar naturally excludes them.  
**Lesson Learned:** #6 — forward declarations must be excluded  
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-P11-007
**Visitor:** `visitFunctionDefinition` — Negative: function prototype in header (ends with `;`)  
**Source:** `include/juno/ds/heap_api.h` (public API declaration)  
**Function name substituted:** `JunoDs_Heap_Insert`  
**Input line:**
```c
JUNO_STATUS_T JunoDs_Heap_Insert(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tValue);
```
**Expected:** NO MATCH — function prototypes end with `;` and parse as `declaration`, not `functionDefinition`.  
**Requirement:** REQ-VSCODE-003

---
