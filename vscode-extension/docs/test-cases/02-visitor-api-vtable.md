> Part of: [Test Case Specification](index.md) — Sections 5-8: API Struct and Vtable Declarations

## Section 5: visitStructDefinition — API Struct Field Extraction

**Visitor:** `visitStructDefinition`  
**CST path:** `structOrUnionSpecifier` (tag ending in `_API_TAG`) → `structDeclarationList`  
**Description:** The visitor walks the `structDeclarationList` in document order, extracting each
function pointer field name. Field order is preserved exactly as declared, since it is required for
positional initializer resolution (Section 8).  
**Requirement:** REQ-VSCODE-012 (prerequisite for positional initializer resolution)

---

### Test Case ID: TC-P5-001
**Visitor:** `visitStructDefinition` (API struct field extraction) — JUNO_DS_HEAP_API_T, three fields  
**Source:** `include/juno/ds/heap_api.h`, lines 131–139  
**Input text:**
```c
struct JUNO_DS_HEAP_API_TAG
{
    JUNO_STATUS_T (*Insert)(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tValue);
    JUNO_STATUS_T (*Heapify)(JUNO_DS_HEAP_ROOT_T *ptHeap);
    JUNO_STATUS_T (*Pop)(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tReturn);
};
```
**CST match (struct tag):** `JUNO_DS_HEAP_API_TAG` → type: `JUNO_DS_HEAP_API_T`

**Field extraction from structDeclarationList (in order):**
1. `Insert`
2. `Heapify`
3. `Pop`

**Expected record:**
```typescript
{ apiType: "JUNO_DS_HEAP_API_T", fields: ["Insert", "Heapify", "Pop"] }
```
**Requirement:** REQ-VSCODE-012

---

### Test Case ID: TC-P5-002
**Visitor:** `visitStructDefinition` (API struct field extraction) — JUNO_LOG_API_T, four fields, verify order preserved  
**Source:** `include/juno/log/log_api.h`, lines 57–68  
**Input text:**
```c
struct JUNO_LOG_API_TAG
{
    JUNO_STATUS_T (*LogDebug)(const JUNO_LOG_ROOT_T *ptJunoLog, const char *pcMsg, ...);
    JUNO_STATUS_T (*LogInfo)(const JUNO_LOG_ROOT_T *ptJunoLog, const char *pcMsg, ...);
    JUNO_STATUS_T (*LogWarning)(const JUNO_LOG_ROOT_T *ptJunoLog, const char *pcMsg, ...);
    JUNO_STATUS_T (*LogError)(const JUNO_LOG_ROOT_T *ptJunoLog, const char *pcMsg, ...);
};
```
**Field extraction from structDeclarationList (in order):**
1. `LogDebug`
2. `LogInfo`
3. `LogWarning`
4. `LogError`

**Expected record:**
```typescript
{ apiType: "JUNO_LOG_API_T", fields: ["LogDebug", "LogInfo", "LogWarning", "LogError"] }
```
**Critical check:** order must be `LogDebug` before `LogInfo` before `LogWarning` before `LogError`.  
**Requirement:** REQ-VSCODE-012

---

### Test Case ID: TC-P5-003
**Visitor:** `visitStructDefinition` (API struct field extraction) — JUNO_DS_HEAP_POINTER_API_T, two-field pointer operations struct  
**Source:** `include/juno/ds/heap_api.h`, lines 112–122  
**Input text:**
```c
struct JUNO_DS_HEAP_POINTER_API_TAG
{
    JUNO_DS_HEAP_COMPARE_RESULT_T (*Compare)(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tParent, JUNO_POINTER_T tChild);
    JUNO_STATUS_T (*Swap)(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tLeft, JUNO_POINTER_T tRight);
};
```
**Field extraction from structDeclarationList (in order):**
1. `Compare`
2. `Swap`

**Expected record:**
```typescript
{ apiType: "JUNO_DS_HEAP_POINTER_API_T", fields: ["Compare", "Swap"] }
```
**Requirement:** REQ-VSCODE-012

---

### Test Case ID: TC-P5-004
**Visitor:** `visitStructDefinition` (API struct field extraction) — JUNO_POINTER_API_T, Copy and Reset  
**Source:** `include/juno/memory/pointer_api.h`, lines 77–87  
**Input text:**
```c
struct JUNO_POINTER_API_TAG
{
    JUNO_STATUS_T (*Copy)(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
    JUNO_STATUS_T (*Reset)(JUNO_POINTER_T tPointer);
};
```
**Field extraction from structDeclarationList (in order):**
1. `Copy`
2. `Reset`

**Expected record:**
```typescript
{ apiType: "JUNO_POINTER_API_T", fields: ["Copy", "Reset"] }
```
**Requirement:** REQ-VSCODE-012

---

### Test Case ID: TC-P5-005
**Visitor:** `visitStructDefinition` (API struct field extraction) — JUNO_SB_BROKER_API_T, Publish and RegisterSubscriber  
**Source:** `include/juno/sb/broker_api.h`, lines 90–102  
**Input text:**
```c
struct JUNO_SB_BROKER_API_TAG
{
    JUNO_STATUS_T (*Publish)(JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_SB_MID_T tMid, JUNO_POINTER_T tMsg);
    JUNO_STATUS_T (*RegisterSubscriber)(JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_SB_PIPE_T *ptPipe);
};
```
**Field extraction from structDeclarationList (in order):**
1. `Publish`
2. `RegisterSubscriber`

**Expected record:**
```typescript
{ apiType: "JUNO_SB_BROKER_API_T", fields: ["Publish", "RegisterSubscriber"] }
```
**Requirement:** REQ-VSCODE-012

---

## Section 6: visitVtableDeclaration — Designated Initializer

**Visitor:** `visitVtableDeclaration`  
**CST path:** `declaration` → `initDeclaratorList` → `initializer` (designated form: `.field = value`)  
**Description:** The visitor identifies `const _API_T` variable initializations and extracts
designated initializer entries (`.field = functionName`) as VtableAssignmentRecords.  
**Requirement:** REQ-VSCODE-010

---

### Test Case ID: TC-P6-001
**Visitor:** `visitVtableDeclaration` (designated initializer) — Designated initializer, engine app API (three fields)  
**Source:** `examples/example_project/engine/src/engine_app.c`, lines 53–57  
**Input text:**
```c
static const JUNO_APP_API_T tEngineAppApi = {
    .OnStart = OnStart,
    .OnProcess = OnProcess,
    .OnExit = OnExit
};
```
**CST match (API type):** `JUNO_APP_API_T`

**Designated initializers extracted:**
| Field | Function |
|-------|----------|
| `OnStart` | `OnStart` |
| `OnProcess` | `OnProcess` |
| `OnExit` | `OnExit` |

**Expected records (3 VtableAssignmentRecord entries):**
```typescript
[
  { apiType: "JUNO_APP_API_T", field: "OnStart",   functionName: "OnStart"   },
  { apiType: "JUNO_APP_API_T", field: "OnProcess", functionName: "OnProcess" },
  { apiType: "JUNO_APP_API_T", field: "OnExit",    functionName: "OnExit"    }
]
```
**Requirement:** REQ-VSCODE-010

---

### Test Case ID: TC-P6-002
**Visitor:** `visitVtableDeclaration` (designated initializer) — Negative: positional initializer must NOT produce designated-field matches  
**Source:** `src/juno_heap.c`, lines 26–30  
**Input text:**
```c
static const JUNO_DS_HEAP_API_T tHeapApi = {
    JunoDs_Heap_Insert,
    JunoDs_Heap_Heapify,
    JunoDs_Heap_Pop,
};
```
**visitVtableDeclaration:** recognizes `JUNO_DS_HEAP_API_T` initializer block; finds no `.field = value` designated entries.  
**Expected:** zero designated VtableAssignmentRecords → visitor falls through to positional initializer path.  
**Requirement:** REQ-VSCODE-010, REQ-VSCODE-012

---

### Test Case ID: TC-P6-003
**Visitor:** `visitVtableDeclaration` (designated initializer) — Negative: `const` variable of non-API type must not be captured  
**Input text *(synthetic)*:**
```c
static const JUNO_DS_HEAP_ROOT_T tHeap = { 0 };
```
**Expected:** no match — `visitVtableDeclaration` only processes initializers where the type name ends in `_API_T`; `JUNO_DS_HEAP_ROOT_T` does not qualify.  
**Requirement:** REQ-VSCODE-010

---

## Section 7: visitVtableDeclaration — Direct Assignment

**Visitor:** `visitVtableDeclaration`  
**CST path:** `expressionStatement` → `assignmentExpression` (form: `varName.field = functionName`)  
**Description:** The visitor identifies standalone assignment statements where a struct variable's
field is assigned a function pointer. The Indexer filters by variable names matching the `_API_T`
naming convention.  
**Note:** Direct assignment (dot syntax) is uncommon in the LibJuno codebase. Test cases are
*(synthetic)* but follow the API types and function names from real code.  
**Requirement:** REQ-VSCODE-011

---

### Test Case ID: TC-P7-001
**Visitor:** `visitVtableDeclaration` (direct assignment) — Direct assignment, heap API field *(synthetic)*  
**Input text:**
```c
    tHeapApi.Insert = JunoDs_Heap_Insert;
```
**Expected visitor extraction:**
- variableName: `tHeapApi`
- field: `Insert`
- functionName: `JunoDs_Heap_Insert`

**Expected record (before Indexer type validation):**
```typescript
{ variableName: "tHeapApi", field: "Insert", functionName: "JunoDs_Heap_Insert" }
```
**Indexer filter:** accepts because `tHeapApi` ends with `Api`, heuristic matches `_API_T` naming.  
**Requirement:** REQ-VSCODE-011

---

### Test Case ID: TC-P7-002
**Visitor:** `visitVtableDeclaration` (direct assignment) — Negative: designated initializer inside braces must not be double-counted  
**Input text *(synthetic)*:**
```c
static const JUNO_DS_HEAP_API_T tHeapApi = {
    .Insert = JunoDs_Heap_Insert,
};
```
**Expected:** `visitVtableDeclaration` handles designated-within-braces (Section 6) and standalone
assignments (this section) via different CST paths; no double-counting occurs. The grammar context
disambiguates the two forms automatically.  
**Significance:** No Indexer-level deduplication is required; the Chevrotain grammar prevents
double-counting by construction.  
**Requirement:** REQ-VSCODE-011

---

## Section 8: visitVtableDeclaration — Positional Initializer

**Visitor:** `visitVtableDeclaration`  
**CST path:** `declaration` → `initDeclaratorList` → `initializer` (positional form: values without designators)  
**Description:** When `visitVtableDeclaration` finds a `const _API_T` initializer block with no
designated entries, it reads the positional values in order and zips them against the field order
stored by `visitStructDefinition` for that API type. Field positions are 0-based.  
**Requirement:** REQ-VSCODE-012

---

### Test Case ID: TC-P8-001
**Visitor:** `visitVtableDeclaration` (positional initializer) — JUNO_DS_HEAP_API_T positional initializer in juno_heap.c  
**Source:** `src/juno_heap.c`, lines 26–30  
**Input text (full block):**
```c
static const JUNO_DS_HEAP_API_T tHeapApi = {
    JunoDs_Heap_Insert,
    JunoDs_Heap_Heapify,
    JunoDs_Heap_Pop,
};
```
**visitStructDefinition field order (prerequisite):** `["Insert", "Heapify", "Pop"]`  
**Tokens extracted (after comment strip):** `["JunoDs_Heap_Insert", "JunoDs_Heap_Heapify", "JunoDs_Heap_Pop"]`  
**Zip result:**

| Position | Field | Function |
|----------|-------|----------|
| 0 | `Insert` | `JunoDs_Heap_Insert` |
| 1 | `Heapify` | `JunoDs_Heap_Heapify` |
| 2 | `Pop` | `JunoDs_Heap_Pop` |

**Expected VtableAssignmentRecords:**
```typescript
[
  { apiType: "JUNO_DS_HEAP_API_T", field: "Insert",  functionName: "JunoDs_Heap_Insert"  },
  { apiType: "JUNO_DS_HEAP_API_T", field: "Heapify", functionName: "JunoDs_Heap_Heapify" },
  { apiType: "JUNO_DS_HEAP_API_T", field: "Pop",     functionName: "JunoDs_Heap_Pop"     }
]
```
**Lesson Learned coverage:** #7 (positional zip with visitStructDefinition field order)  
**Requirement:** REQ-VSCODE-012

---

### Test Case ID: TC-P8-002
**Visitor:** `visitVtableDeclaration` (positional initializer) — JUNO_SB_BROKER_API_T with static functions (forward-declared)  
**Source:** `src/juno_broker.c`, lines 31–34  
**Input text:**
```c
static const JUNO_SB_BROKER_API_T gtBrokerApi =
{
    Publish,
    RegisterSubscriber
};
```
**visitStructDefinition field order (prerequisite):** `["Publish", "RegisterSubscriber"]`  
**Tokens extracted:** `["Publish", "RegisterSubscriber"]`  
**Zip result:**

| Position | Field | Function |
|----------|-------|----------|
| 0 | `Publish` | `Publish` (static — defined in same file, juno_broker.c line 51) |
| 1 | `RegisterSubscriber` | `RegisterSubscriber` (static — juno_broker.c line 76) |

**Expected VtableAssignmentRecords:**
```typescript
[
  { apiType: "JUNO_SB_BROKER_API_T", field: "Publish",            functionName: "Publish"            },
  { apiType: "JUNO_SB_BROKER_API_T", field: "RegisterSubscriber", functionName: "RegisterSubscriber" }
]
```
**Note:** Because `Publish` and `RegisterSubscriber` are `static`, `visitFunctionDefinition`
resolution must restrict the definition search to `src/juno_broker.c` only.  
**Requirement:** REQ-VSCODE-012

---

### Test Case ID: TC-P8-003
**Visitor:** `visitVtableDeclaration` (positional initializer) — JUNO_LOG_API_T, four static functions in main.c  
**Source:** `examples/example_project/main.c`, lines 146–151  
**Input text:**
```c
static const JUNO_LOG_API_T gtMyLoggerApi ={
    LogDebug,
    LogInfo,
    LogWarning,
    LogError
};
```
**visitStructDefinition field order (prerequisite):** `["LogDebug", "LogInfo", "LogWarning", "LogError"]`  
**Tokens extracted:** `["LogDebug", "LogInfo", "LogWarning", "LogError"]`  
**Zip result:**

| Position | Field | Function |
|----------|-------|----------|
| 0 | `LogDebug` | `LogDebug` (static — main.c line 49) |
| 1 | `LogInfo` | `LogInfo` (static — main.c line 60) |
| 2 | `LogWarning` | `LogWarning` (static — main.c line 71) |
| 3 | `LogError` | `LogError` (static — main.c line 82) |

**Requirement:** REQ-VSCODE-012

---

### Test Case ID: TC-P8-004
**Visitor:** `visitVtableDeclaration` (positional initializer) — Negative: designated initializer must not fall through to positional path  
**Source:** `examples/example_project/engine/src/engine_app.c`, lines 53–57  
**Input text:**
```c
static const JUNO_APP_API_T tEngineAppApi = {
    .OnStart = OnStart,
    .OnProcess = OnProcess,
    .OnExit = OnExit
};
```
**Expected:** `visitVtableDeclaration` finds 3 designated assignments → positional path is NOT
entered. Designated fields take precedence.  
**Requirement:** REQ-VSCODE-012

---
