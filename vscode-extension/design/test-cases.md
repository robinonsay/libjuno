# LibJuno VSCode Extension — Test Case Specification

**Date:** 2026-04-14  
**Module:** VSCODE  
**Status:** Draft — for translation into Jest tests

All inputs are verbatim text from the LibJuno codebase (noted by file and line) unless explicitly
marked *(synthetic)*. Column numbers are 1-based. All test inputs and expected records are derived
from the Chevrotain parser/visitor architecture described in `vscode-extension/design/design.md`
Section 3.

---

## Lessons-Learned Coverage Map

| # | Lesson | Test Cases Covering It |
|---|--------|------------------------|
| 1 | Indirect API pointer (`ptLoggerApi->LogInfo`) — Category 3 (Direct API pointer) | TC-P9-301, TC-P9-302, TC-P9-303, TC-RES-001 |
| 2 | Array subscript in receiver (`ptAppList[i]->ptApi->OnStart`) — Category 1 strip `[i]` | TC-P9-103, TC-P9-104, TC-RES-002 |
| 3 | Chained member access (`ptEngineApp->ptBroker->ptApi->RegisterSubscriber`) | TC-P9-105, TC-RES-003 |
| 4 | Dot-accessed `ptApi` (`tReturn.ptApi->Copy`, `tPtrResult.tOk.ptApi->Copy`) | TC-P9-201, TC-P9-202, TC-RES-005 |
| 5 | Non-`ptApi` API member names (`ptHeap->ptHeapPointerApi->Compare`) — Category 4 (Named API member) | TC-P9-401, TC-P9-402, TC-RES-004 |
| 6 | Static function definitions — visitFunctionDefinition must capture `{` not `;` | TC-P11-005, TC-P11-006 |
| 7 | Positional initializers — visitVtableDeclaration (positional) must zip by field order from visitStructDefinition | TC-P8-001 through TC-P8-004 |
| 8 | Function definition varied return types — visitFunctionDefinition | TC-P11-001 through TC-P11-005 |

---

## Section 1: visitStructDefinition — JUNO_MODULE_ROOT

**Visitor:** `visitStructDefinition`  
**CST path:** `structOrUnionSpecifier` → `junoModuleRootMacro`  
**Requirement:** REQ-VSCODE-008

---

### Test Case ID: TC-P1-001
**Visitor:** `visitStructDefinition` (junoModuleRootMacro branch) — JUNO_MODULE_ROOT, minimal expansion (JUNO_MODULE_EMPTY)  
**Source:** `include/juno/log/log_api.h`, line 52  
**Input text:**
```c
struct JUNO_LOG_ROOT_TAG JUNO_MODULE_ROOT(JUNO_LOG_API_T, JUNO_MODULE_EMPTY);
```
**Expected visitor extraction:**
- Group 1 `JUNO_LOG_ROOT_TAG`: rootType (from struct tag with _TAG→_T conversion)
- Group 2 `JUNO_LOG_API_T`: apiType (first Identifier argument of macro)

**Expected record:**
```typescript
{ rootType: "JUNO_LOG_ROOT_T", apiType: "JUNO_LOG_API_T" }
```
**Requirement:** REQ-VSCODE-008

---

### Test Case ID: TC-P1-002
**Visitor:** `visitStructDefinition` (junoModuleRootMacro branch) — JUNO_MODULE_ROOT with extra members (multi-line macro body)  
**Source:** `include/juno/ds/heap_api.h`, lines 100–105  
**Input text:**
```c
struct JUNO_DS_HEAP_ROOT_TAG JUNO_MODULE_ROOT(JUNO_DS_HEAP_API_T,
    const JUNO_DS_HEAP_POINTER_API_T *ptHeapPointerApi;
    JUNO_DS_ARRAY_ROOT_T *ptHeapArray;
    size_t zLength;
);
```
**Expected visitor extraction (from line 100):**
- Group 1 `JUNO_DS_HEAP_ROOT_TAG`: rootType (from struct tag with _TAG→_T conversion)
- Group 2 `JUNO_DS_HEAP_API_T`: apiType (first Identifier argument of macro)

**Expected record:**
```typescript
{ rootType: "JUNO_DS_HEAP_ROOT_T", apiType: "JUNO_DS_HEAP_API_T" }
```
**Note:** The visitor reads the macro's first Identifier argument and stops; extra members
within the macro body argument list are irrelevant to this extraction.  
**Requirement:** REQ-VSCODE-008

---

### Test Case ID: TC-P1-003
**Visitor:** `visitStructDefinition` (junoModuleRootMacro branch) — JUNO_MODULE_ROOT, broker variant  
**Source:** `include/juno/sb/broker_api.h`, line 80  
**Input text:**
```c
struct JUNO_SB_BROKER_ROOT_TAG JUNO_MODULE_ROOT(JUNO_SB_BROKER_API_T,
```
**Expected visitor extraction:**
- Group 1 `JUNO_SB_BROKER_ROOT_TAG`: rootType (from struct tag with _TAG→_T conversion)
- Group 2 `JUNO_SB_BROKER_API_T`: apiType (first Identifier argument of macro)

**Expected record:**
```typescript
{ rootType: "JUNO_SB_BROKER_ROOT_T", apiType: "JUNO_SB_BROKER_API_T" }
```
**Requirement:** REQ-VSCODE-008

---

### Test Case ID: TC-P1-004
**Visitor:** `visitStructDefinition` (junoModuleRootMacro branch) — Negative: JUNO_MODULE_DERIVE must NOT match  
**Source:** `examples/example_project/engine/include/engine_app/engine_app.h`, line 83  
**Input text:**
```c
struct ENGINE_APP_TAG JUNO_MODULE_DERIVE(JUNO_APP_ROOT_T,
```
**Expected:** no match (parser dispatches to `junoModuleDeriveMacro` branch, not `junoModuleRootMacro`)  
**Requirement:** REQ-VSCODE-008

---

## Section 2: visitStructDefinition — JUNO_MODULE_DERIVE

**Visitor:** `visitStructDefinition`  
**CST path:** `structOrUnionSpecifier` → `junoModuleDeriveMacro`  
**Requirement:** REQ-VSCODE-009

---

### Test Case ID: TC-P2-001
**Visitor:** `visitStructDefinition` (junoModuleDeriveMacro branch) — JUNO_MODULE_DERIVE, application derivation  
**Source:** `examples/example_project/engine/include/engine_app/engine_app.h`, line 83  
**Input text:**
```c
struct ENGINE_APP_TAG JUNO_MODULE_DERIVE(JUNO_APP_ROOT_T,
```
**Expected visitor extraction:**
- Group 1 `ENGINE_APP_TAG`: derivedType (from struct tag with _TAG→_T conversion)
- Group 2 `JUNO_APP_ROOT_T`: rootType (first Identifier argument of macro)

**Expected record:**
```typescript
{ derivedType: "ENGINE_APP_T", rootType: "JUNO_APP_ROOT_T" }
```
**Requirement:** REQ-VSCODE-009

---

### Test Case ID: TC-P2-002
**Visitor:** `visitStructDefinition` (junoModuleDeriveMacro branch) — JUNO_MODULE_DERIVE, pipe derivation from queue root  
**Source:** `include/juno/sb/broker_api.h`, line 73  
**Input text:**
```c
struct JUNO_SB_PIPE_TAG JUNO_MODULE_DERIVE(JUNO_DS_QUEUE_ROOT_T,
```
**Expected visitor extraction:**
- Group 1 `JUNO_SB_PIPE_TAG`: derivedType (from struct tag with _TAG→_T conversion)
- Group 2 `JUNO_DS_QUEUE_ROOT_T`: rootType (first Identifier argument of macro)

**Expected record:**
```typescript
{ derivedType: "JUNO_SB_PIPE_T", rootType: "JUNO_DS_QUEUE_ROOT_T" }
```
**Requirement:** REQ-VSCODE-009

---

### Test Case ID: TC-P2-003
**Visitor:** `visitStructDefinition` (junoModuleDeriveMacro branch) — Negative: JUNO_MODULE_ROOT must NOT match  
**Source:** `include/juno/log/log_api.h`, line 52  
**Input text:**
```c
struct JUNO_LOG_ROOT_TAG JUNO_MODULE_ROOT(JUNO_LOG_API_T, JUNO_MODULE_EMPTY);
```
**Expected:** no match (parser dispatches to `junoModuleRootMacro` branch, not `junoModuleDeriveMacro`)  
**Requirement:** REQ-VSCODE-009

---

## Section 3: visitStructDefinition — JUNO_TRAIT_ROOT

**Visitor:** `visitStructDefinition`  
**CST path:** `structOrUnionSpecifier` → `junoTraitRootMacro`  
**Requirement:** REQ-VSCODE-014

---

### Test Case ID: TC-P3-001
**Visitor:** `visitStructDefinition` (junoTraitRootMacro branch) — JUNO_TRAIT_ROOT, JUNO_POINTER_T  
**Source:** `include/juno/memory/pointer_api.h`, line 56  
**Input text:**
```c
struct JUNO_POINTER_TAG JUNO_TRAIT_ROOT(JUNO_POINTER_API_T,
```
**Expected visitor extraction:**
- Group 1 `JUNO_POINTER_TAG`: rootType (from struct tag with _TAG→_T conversion)
- Group 2 `JUNO_POINTER_API_T`: apiType (first Identifier argument of macro)

**Expected record:**
```typescript
{ rootType: "JUNO_POINTER_T", apiType: "JUNO_POINTER_API_T" }
```
**Note:** `JUNO_POINTER_T` has `_TAG` → `_T` conversion yielding `JUNO_POINTER_T`.  
**Requirement:** REQ-VSCODE-014

---

### Test Case ID: TC-P3-002
**Visitor:** `visitStructDefinition` (junoTraitRootMacro branch) — Negative: JUNO_MODULE_ROOT must NOT match  
**Source:** `include/juno/log/log_api.h`, line 52  
**Input text:**
```c
struct JUNO_LOG_ROOT_TAG JUNO_MODULE_ROOT(JUNO_LOG_API_T, JUNO_MODULE_EMPTY);
```
**Expected:** no match  
**Requirement:** REQ-VSCODE-014

---

### Test Case ID: TC-P3-003
**Visitor:** `visitStructDefinition` (junoTraitRootMacro branch) — Negative: JUNO_MODULE_DERIVE must NOT match  
**Source:** `examples/example_project/engine/include/engine_app/engine_app.h`, line 83  
**Input text:**
```c
struct ENGINE_APP_TAG JUNO_MODULE_DERIVE(JUNO_APP_ROOT_T,
```
**Expected:** no match  
**Requirement:** REQ-VSCODE-014

---

## Section 4: visitStructDefinition — JUNO_TRAIT_DERIVE

**Visitor:** `visitStructDefinition`  
**CST path:** `structOrUnionSpecifier` → `junoTraitDeriveMacro`  
**Requirement:** REQ-VSCODE-015  
**Note:** No real occurrence of `JUNO_TRAIT_DERIVE` exists in the current codebase;
`module.h` line 170 defines it as an alias for `JUNO_MODULE_DERIVE`. Test cases are
*(synthetic)* based on the macro definition.

---

### Test Case ID: TC-P4-001
**Visitor:** `visitStructDefinition` (junoTraitDeriveMacro branch) — JUNO_TRAIT_DERIVE, trait extension *(synthetic)*  
**Input text:**
```c
struct MY_POINTER_IMPL_TAG JUNO_TRAIT_DERIVE(JUNO_POINTER_T,
    void *pvExtraState;
);
```
**Expected visitor extraction:**
- Group 1 `MY_POINTER_IMPL_TAG`: derivedType (from struct tag with _TAG→_T conversion)
- Group 2 `JUNO_POINTER_T`: rootType (first Identifier argument of macro)

**Expected record:**
```typescript
{ derivedType: "MY_POINTER_IMPL_T", rootType: "JUNO_POINTER_T" }
```
**Requirement:** REQ-VSCODE-015

---

### Test Case ID: TC-P4-002
**Visitor:** `visitStructDefinition` (junoTraitDeriveMacro branch) — Negative: JUNO_TRAIT_ROOT must NOT match *(synthetic)*  
**Input text:**
```c
struct JUNO_POINTER_TAG JUNO_TRAIT_ROOT(JUNO_POINTER_API_T,
```
**Expected:** no match  
**Requirement:** REQ-VSCODE-015

---

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

## Section 9: Chain-Walk Call Site Resolution

**Visitor:** `IndexBuildingVisitor` (chain-walk algorithm at resolution time)  
**Requirement:** REQ-VSCODE-003

The chain-walk inspects the call expression to the left of the cursor, resolves the receiver type
using `LocalTypeInfo` (populated by `visitLocalDeclaration` and `visitFunctionParameters`), and
walks the derivation chain to determine `apiType`. This replaces all Phase 1 / Phase 2 strategy
logic from the earlier regex-based design.

---

### Call Site Identification — Field Name from Cursor Position

---

### Test Case ID: TC-P9-001
**Description:** Call site identification — Simple ptApi call, cursor on field name  
**Source:** `examples/example_project/engine/src/engine_app.c`, line 204  
**Input line:**
```c
    JUNO_TIMESTAMP_RESULT_T tTimestampResult = ptTime->ptApi->Now(ptTime);
```
**Cursor column:** 56 (on `Now`)  
**Field name from CST:** cursor at column 56 is on `Now` within `->Now(` → fieldName = `"Now"`  
**Cursor validation:** column 56 falls within the `Now` token (columns 54–56) ✓  
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-P9-002
**Description:** Call site identification — Indirect API pointer call, cursor on field name  
**Source:** `examples/example_project/engine/src/engine_app.c`, line 140  
**Input line:**
```c
    ptLoggerApi->LogInfo(ptLogger, "Engine App Initialized");
```
**Cursor column:** 18 (on `LogInfo`)  
**Field name from CST:** cursor at column 18 is on `LogInfo` within `->LogInfo(` → fieldName = `"LogInfo"`  
**Cursor validation:** column 18 falls within the `LogInfo` token (columns 18–24) ✓  
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-P9-003
**Description:** Call site identification — Negative: regular member access (no trailing `(`)  
**Input line *(synthetic)*:**
```c
    size_t zLen = ptHeap->zLength;
```
**Cursor column:** 26 (on `zLength`)  
**CST field resolution:** `->zLength` has no following `(` — not a call expression; the CST produces no function call node  
**Expected:** RETURN `{ found: false, errorMsg: "Cursor is not on a LibJuno API call site." }`  
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-P9-004
**Description:** Call site identification — Negative: cursor on whitespace (not within any captured token)  
**Source:** `examples/example_project/engine/src/engine_app.c`, line 204 (same line as TC-P9-001)  
**Input line:**
```c
    JUNO_TIMESTAMP_RESULT_T tTimestampResult = ptTime->ptApi->Now(ptTime);
```
**Cursor column:** 45 (on whitespace between `=` and `ptTime`)  
**Expected:** no `->word(` match overlaps column 45  
**Expected result:** RETURN `{ found: false, errorMsg: "Cursor is not on a LibJuno API call site." }`  
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-P9-005
**Description:** Call site identification — Negative: free function call (no `->` before name)  
**Input line *(synthetic)*:**
```c
    printf("hello");
```
**Cursor column:** 5 (on `printf`)  
**CST field resolution:** no arrow-call expression found on this line  
**Expected:** RETURN `{ found: false, errorMsg: "Cursor is not on a LibJuno API call site." }`  
**Requirement:** REQ-VSCODE-003

---

### Category 1 — Simple `->ptApi->Field` chains

---

### Test Case ID: TC-P9-101
**Chain-walk:** Category 1 — Simple pointer receiver  
**Source:** `examples/example_project/engine/src/engine_app.c`, line 204  
**Input line:**
```c
    JUNO_TIMESTAMP_RESULT_T tTimestampResult = ptTime->ptApi->Now(ptTime);
```
**Cursor column:** 56 (on `Now`)  
**Chain-walk Steps:**
1. fieldName: `"Now"` (from cursor on `->Now(`)
2. Chain `ptTime->ptApi->Now(` — Category 1 (`->ptApi->` prefix found)
3. baseVar: `ptTime`
**LocalTypeInfo lookup:**
```c
    const JUNO_TIME_ROOT_T *ptTime = ptEngineApp->ptTime;
```
**rootType:** `JUNO_TIME_ROOT_T`  
**Derivation chain:** `JUNO_TIME_ROOT_T` has no parent → rootType = `JUNO_TIME_ROOT_T`  
**apiType lookup:** `index.moduleRoots.get("JUNO_TIME_ROOT_T")` → `"JUNO_TIME_API_T"`  
**Expected chain-walk category:** 1  
**Requirement:** REQ-VSCODE-003, REQ-VSCODE-005

---

### Test Case ID: TC-P9-102
**Chain-walk:** Category 1 — Simple pointer, broker Publish call  
**Source:** `examples/example_project/engine/src/engine_app.c`, line 238  
**Input line:**
```c
    ptBroker->ptApi->Publish(ptBroker, ENGINE_TLM_MSG_MID, tEngineTlmPointer);
```
**Cursor column:** 22 (on `Publish`)  
**Chain-walk Steps:**
1. fieldName: `"Publish"` (from cursor on `->Publish(`)
2. Chain `ptBroker->ptApi->Publish(` — Category 1
3. baseVar: `ptBroker`
**LocalTypeInfo lookup:**
```c
    JUNO_SB_BROKER_ROOT_T *ptBroker = ptEngineApp->ptBroker;
```
**rootType:** `JUNO_SB_BROKER_ROOT_T`  
**apiType:** `JUNO_SB_BROKER_API_T`  
**Expected chain-walk category:** 1  
**Requirement:** REQ-VSCODE-003, REQ-VSCODE-005

---

### Test Case ID: TC-P9-103
**Chain-walk:** Category 1 — Array subscript receiver (Lesson Learned #2)  
**Source:** `examples/example_project/main.c`, line 234  
**Input line:**
```c
        tStatus = ptAppList[i]->ptApi->OnStart(ptAppList[i]);
```
**Cursor column:** 40 (on `OnStart`)  
**Chain-walk Steps:**
1. fieldName: `"OnStart"` (from cursor on `->OnStart(`)
2. Chain `ptAppList[i]->ptApi->OnStart(` — Category 1; strip `[i]` → baseVar: `ptAppList`
**LocalTypeInfo lookup:**
```c
    static JUNO_APP_ROOT_T *ptAppList[2] = {
```
**rootType:** `JUNO_APP_ROOT_T`  
**apiType:** `JUNO_APP_API_T`  
**Expected chain-walk category:** 1 (with subscript strip)  
**Lesson Learned:** #2 — array subscript in receiver  
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-P9-104
**Chain-walk:** Category 1 — Array subscript with variable index (non-literal subscript)  
**Source:** `examples/example_project/main.c`, line 240  
**Input line:**
```c
        ptAppList[iCounter]->ptApi->OnProcess(ptAppList[iCounter]);
```
**Cursor column:** 38 (on `OnProcess`)  
**Chain-walk Steps:**
1. fieldName: `"OnProcess"` (from cursor on `->OnProcess(`)
2. Chain `ptAppList[iCounter]->ptApi->OnProcess(` — Category 1; strip `[iCounter]` → baseVar: `ptAppList`
**rootType:** `JUNO_APP_ROOT_T` (same LocalTypeInfo lookup as TC-P9-103)  
**apiType:** `JUNO_APP_API_T`  
**Expected chain-walk category:** 1  
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-P9-105
**Chain-walk:** Category 1 — Chained member access (Lesson Learned #3)  
**Source:** `examples/example_project/engine/src/engine_app.c`, line 151  
**Input line:**
```c
    tStatus = ptEngineApp->ptBroker->ptApi->RegisterSubscriber(ptEngineApp->ptBroker, &ptEngineApp->tCmdPipe);
```
**Cursor column:** 45 (on `RegisterSubscriber`)  
**Chain-walk Steps:**
1. fieldName: `"RegisterSubscriber"` (from cursor on `->RegisterSubscriber(`)
2. Chain `ptEngineApp->ptBroker->ptApi->RegisterSubscriber(` — Category 1 (chained)
3. Sub-expression `ptEngineApp->ptBroker`: resolve `ptEngineApp` type, look up member `ptBroker`
**LocalTypeInfo lookup for `ptEngineApp`:**
```c
    ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp);
```
**outerType:** `ENGINE_APP_T`  
**Member lookup** in `ENGINE_APP_T` struct definition: `JUNO_SB_BROKER_ROOT_T *ptBroker;`  
**rootType:** `JUNO_SB_BROKER_ROOT_T`  
**apiType:** `JUNO_SB_BROKER_API_T`  
**Expected chain-walk category:** 1 (chained)  
**Lesson Learned:** #3 — chained member access requires struct-member-type lookup  
**Requirement:** REQ-VSCODE-003

---

### Category 2 — Dot-accessed `.ptApi->Field`

---

### Test Case ID: TC-P9-201
**Chain-walk:** Category 2 — Stack value (dot-accessed ptApi) (Lesson Learned #4)  
**Source:** `src/juno_buff_stack.c`, line 66  
**Input line:**
```c
        tStatus = tReturn.ptApi->Copy(tReturn, tResult.tOk);
```
**Cursor column:** 27 (on `Copy`)  
**Chain-walk Steps:**
1. fieldName: `"Copy"` (from cursor on `->Copy(`)
2. Chain `tReturn.ptApi->Copy(` — Category 2 (dot-accessed `.ptApi`)
3. baseVar: `tReturn`
**LocalTypeInfo lookup:**
```c
JUNO_STATUS_T JunoDs_Stack_Pop(JUNO_DS_STACK_ROOT_T *ptStack, JUNO_POINTER_T tReturn) {
```
**declared type:** `JUNO_POINTER_T`  
**rootType:** `JUNO_POINTER_T`  
**Trait lookup:** `index.traitRoots.get("JUNO_POINTER_T")` → `"JUNO_POINTER_API_T"`  
**apiType:** `JUNO_POINTER_API_T`  
**Expected chain-walk category:** 2 (dot form)  
**Lesson Learned:** #4 — dot-accessed ptApi  
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-P9-202
**Chain-walk:** Category 2 — Nested dot access (`tPtrResult.tOk.ptApi->Copy`) (Lesson Learned #4)  
**Source:** `src/juno_buff_queue.c`, line 64  
**Input line:**
```c
        tStatus = tPtrResult.tOk.ptApi->Copy(tReturn, JUNO_OK(tPtrResult));
```
**Cursor column:** 36 (on `Copy`)  
**Chain-walk Steps:**
1. fieldName: `"Copy"` (from cursor on `->Copy(`)
2. Chain `tPtrResult.tOk.ptApi->Copy(` — Category 2 (nested dot chain)
3. Resolve `tPtrResult` type, then access `.tOk` member type
**LocalTypeInfo lookup for `tPtrResult`:**
```c
        JUNO_RESULT_POINTER_T tPtrResult = ptApi->GetAt(ptBuffer, iDequeueIndex);
```
**outerType:** `JUNO_RESULT_POINTER_T`  
**Member `.tOk`** in `JUNO_RESULT_POINTER_T` → `JUNO_POINTER_T`  
**rootType:** `JUNO_POINTER_T`  
**apiType:** `JUNO_POINTER_API_T`  
**Expected chain-walk category:** 2 (nested dot chain)  
**Lesson Learned:** #4  
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-P9-203
**Chain-walk:** Category 2 — Dot-accessed ptApi, map copy *(from real src)*  
**Source:** `src/juno_map.c`, line 152  
**Input line:**
```c
    tStatus = tItem.ptApi->Copy(tItemResult.tOk, tItem);
```
**Cursor column:** 23 (on `Copy`)  
**Chain-walk Steps:**
1. fieldName: `"Copy"` (from cursor on `->Copy(`)
2. Chain `tItem.ptApi->Copy(` — Category 2 (dot form)
**LocalTypeInfo resolves `tItem`:** `JUNO_POINTER_T`  
**apiType:** `JUNO_POINTER_API_T`  
**Expected chain-walk category:** 2 (dot form)  
**Requirement:** REQ-VSCODE-003

---

### Category 3 — Direct API pointer

---

### Test Case ID: TC-P9-301
**Chain-walk:** Category 3 — Indirect log API pointer, LogInfo (Lesson Learned #1)  
**Source:** `examples/example_project/engine/src/engine_app.c`, line 140  
**Input line (line 140):**
```c
    ptLoggerApi->LogInfo(ptLogger, "Engine App Initialized");
```
**Cursor column:** 18 (on `LogInfo`)  
**Chain-walk Steps:**
1. fieldName: `"LogInfo"` (from cursor on `->LogInfo(`)
2. Chain `ptLoggerApi->LogInfo(` — Category 3 (direct API pointer; no `->ptApi->` in chain)
3. LocalTypeInfo lookup for `ptLoggerApi` (from line 138):
```c
    const JUNO_LOG_API_T *ptLoggerApi = ptLogger->ptApi;
```
**apiType:** `JUNO_LOG_API_T`  
**Expected chain-walk category:** 3  
**Lesson Learned:** #1 — direct API pointer variable  
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-P9-302
**Chain-walk:** Category 3 — Indirect queue API pointer, Dequeue (Lesson Learned #1)  
**Source:** `examples/example_project/engine/src/engine_app.c`, line 223  
**Input line (line 223):**
```c
    tStatus = ptCmdPipeApi->Dequeue(&ptEngineApp->tCmdPipe.tRoot, tEngineCmdPointer);
```
**Cursor column:** 15 (on `Dequeue`)  
**Chain-walk Steps:**
1. fieldName: `"Dequeue"` (from cursor on `->Dequeue(`)
2. Chain `ptCmdPipeApi->Dequeue(` — Category 3 (direct API pointer)
3. LocalTypeInfo lookup for `ptCmdPipeApi` (from line 192):
```c
    const JUNO_DS_QUEUE_API_T *ptCmdPipeApi = ptEngineApp->tCmdPipe.tRoot.ptApi;
```
**apiType:** `JUNO_DS_QUEUE_API_T`  
**Expected chain-walk category:** 3  
**Lesson Learned:** #1  
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-P9-303
**Chain-walk:** Category 3 — Generic name `ptApi` variable (indirect, same-named pointer)  
**Source:** `src/juno_buff_queue.c`, line 62  
**Input line (line 62):**
```c
        JUNO_RESULT_POINTER_T tPtrResult = ptApi->GetAt(ptBuffer, iDequeueIndex);
```
**Cursor column:** 44 (on `GetAt`)  
**Chain-walk Steps:**
1. fieldName: `"GetAt"` (from cursor on `->GetAt(`)
2. Chain `ptApi->GetAt(` — the token before `->GetAt(` is `ptApi` with no preceding `->` or `.`; no
   `->ptApi->` sub-chain present → recognized as Category 3 (direct API pointer, variable named `ptApi`)
3. LocalTypeInfo lookup for `ptApi`:
```c
        const JUNO_DS_ARRAY_API_T *ptApi = ptBuffer->ptApi;
```
**Disambiguation:** The chain-walk checks whether the token immediately before `->GetAt(` has a
preceding `->` or `.`; it does not (bare `ptApi`), so this is Category 3, not Category 1.  
**apiType (via LocalTypeInfo):** `JUNO_DS_ARRAY_API_T`  
**Expected chain-walk category:** 3  
**Lesson Learned:** #1 (generic `ptApi` variable name)  
**Requirement:** REQ-VSCODE-003

---

### Category 4 — Named API member

---

### Test Case ID: TC-P9-401
**Chain-walk:** Category 4 — Non-ptApi member Compare call (Lesson Learned #5)  
**Source:** `src/juno_heap.c`, line 151  
**Input line:**
```c
        JUNO_DS_HEAP_COMPARE_RESULT_T tCompareResult = ptHeap->ptHeapPointerApi->Compare(ptHeap, JUNO_OK(tResultParent), JUNO_OK(tResultCurrent));
```
**Cursor column:** 64 (on `Compare`)  
**Chain-walk Steps:**
1. fieldName: `"Compare"` (from cursor on `->Compare(`)
2. Chain `ptHeap->ptHeapPointerApi->Compare(` — sub-member `ptHeapPointerApi` is a named API member → Category 4
3. `apiMemberRegistry.get("ptHeapPointerApi")` → `"JUNO_DS_HEAP_POINTER_API_T"` (no derivation chain step needed)  
**apiType:** `JUNO_DS_HEAP_POINTER_API_T`  
**Expected chain-walk category:** 4  
**Lesson Learned:** #5  
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-P9-402
**Chain-walk:** Category 4 — Non-ptApi member Swap call  
**Source:** `src/juno_heap.c`, line 155  
**Input line:**
```c
            tStatus = ptHeap->ptHeapPointerApi->Swap(ptHeap, JUNO_OK(tResultCurrent), JUNO_OK(tResultParent));
```
**Cursor column:** 51 (on `Swap`)  
**Chain-walk Steps:**
1. fieldName: `"Swap"` (from cursor on `->Swap(`)
2. Chain `ptHeap->ptHeapPointerApi->Swap(` — Category 4 (named API member)
3. `apiMemberRegistry.get("ptHeapPointerApi")` → `"JUNO_DS_HEAP_POINTER_API_T"`  
**Expected chain-walk category:** 4  
**Requirement:** REQ-VSCODE-003

---

### Category 5 — Macro-based (JUNO_MODULE_GET_API)

---

### Test Case ID: TC-P9-501
**Chain-walk:** Category 5 — Macro-based API access *(synthetic — pattern from design catalog)*  
**Input line:**
```c
    JUNO_STATUS_T tStatus = JUNO_MODULE_GET_API(ptModule, JUNO_DS_HEAP_ROOT_T)->Insert(ptModule, tValue);
```
**Cursor column:** 56 (on `Insert`)  
**Chain-walk Steps:**
1. fieldName: `"Insert"` (from cursor on `->Insert(`)
2. `JUNO_MODULE_GET_API(ptModule, JUNO_DS_HEAP_ROOT_T)` macro detected before `->Insert(` → Category 5
3. rootType explicitly stated in macro: `JUNO_DS_HEAP_ROOT_T`
4. apiType resolved from derivation chain  
**rootType:** `JUNO_DS_HEAP_ROOT_T` (explicit from macro)  
**apiType:** `JUNO_DS_HEAP_API_T`  
**Expected chain-walk category:** 5  
**Requirement:** REQ-VSCODE-003

---

### Category 6 — Fallback: Field Name Search Across All API Types

---

### Test Case ID: TC-P9-601
**Chain-walk:** Category 6 (fallback) — Unknown receiver, field name unique to one API type *(synthetic)*  
**Input line:**
```c
    tStatus = pUnknown->ptApi->OnExit(pUnknown);
```
**Index state:** `pUnknown` has no type entry in LocalTypeInfo.  
**Cursor column:** 29 (on `OnExit`)  
**Chain-walk Steps:**
1. fieldName: `"OnExit"` (from cursor on `->OnExit(`)
2. All chain-walk categories 1–5 fail (cannot determine `pUnknown` type)
3. Category 6 fallback activated:
- Search `apiStructFields` for all API types containing `"OnExit"`
- `JUNO_APP_API_T` has field `"OnExit"` ← only match
- `candidates = ["JUNO_APP_API_T"]`  

**apiType:** `JUNO_APP_API_T`  
→ GOTO Step 5 → look up `vtableAssignments["JUNO_APP_API_T"]["OnExit"]`  
**Expected chain-walk category:** 6  
**Expected:** `{ found: true, locations: [...OnExit implementations...] }`  
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-P9-602
**Chain-walk:** Category 6 (fallback) — Unknown receiver, field name shared by multiple API types *(synthetic)*  
**Input line:**
```c
    tStatus = pUnknown->ptApi->Copy(pUnknown, tSrc);
```
**Index state:** `pUnknown` has no type in LocalTypeInfo.  
**Chain-walk Steps:**
1. fieldName: `"Copy"` (from cursor on `->Copy(`)
2. Categories 1–5 fail; Category 6 fallback activated:
- `JUNO_POINTER_API_T` has field `"Copy"` 
- *(and potentially other API types if they also define `Copy`)*
- `candidates.length > 1` → collect locations from all matched API types  

**Expected result:** `{ found: true, locations: [all Copy implementations across matched types] }`  
**Note:** May include false positives. User is shown a QuickPick list.  
**Expected chain-walk category:** 6 (multi-match path)  
**Requirement:** REQ-VSCODE-003, REQ-VSCODE-006

---

### Test Case ID: TC-P9-603
**Chain-walk:** Category 6 (fallback) — Unknown receiver, field name found in NO API type *(synthetic)*  
**Input line:**
```c
    tStatus = pUnknown->ptApi->DoSomethingUnknown(pUnknown);
```
**Chain-walk Steps:**
1. fieldName: `"DoSomethingUnknown"` (from cursor)
2. All categories 1–6 fail:
**Expected:** `{ found: false, errorMsg: "No API type contains field 'DoSomethingUnknown'." }`  
**Requirement:** REQ-VSCODE-003, REQ-VSCODE-004

---

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

## Section 12: End-to-End Resolution Tests

These tests exercise the complete resolution algorithm (Steps 1–6 from design Section 5.1).
Each test describes the index state required and the expected output.

---

### Test Case ID: TC-RES-001
**Scenario:** Indirect API pointer — `ptLoggerApi->LogInfo` in engine_app.c (chain-walk (Category 3 — direct API pointer) path)  
**Lesson Learned:** #1  
**Source file context (`engine_app.c`, OnStart body, lines 138–140):**
```c
    const JUNO_LOG_API_T *ptLoggerApi = ptLogger->ptApi;
    // Log that the app was intialized
    ptLoggerApi->LogInfo(ptLogger, "Engine App Initialized");
```
**Cursor:** `engine_app.c`, line 140, column 18 (on `LogInfo`)  

**Index state required:**
```typescript
apiStructFields: { "JUNO_LOG_API_T": ["LogDebug", "LogInfo", "LogWarning", "LogError"] }
vtableAssignments: {
  "JUNO_LOG_API_T": {
    "LogInfo": [{ functionName: "LogInfo", file: "examples/example_project/main.c", line: 60 }]
  }
}
functionDefinitions: { "LogInfo": [{ file: "examples/example_project/main.c", line: 60, isStatic: true }] }
```

**Resolution trace:**
1. `fieldName = "LogInfo"` (chain-walk Step 3 (field name extraction))
2. Chain-walk (Category 1): no `ptApi->LogInfo(` → fail
3. Chain-walk (Category 4 — apiMemberRegistry): `ptLoggerApi` not in `apiMemberRegistry` → fail
4. Chain-walk (Category 3 — direct API pointer): `apiVar = "ptLoggerApi"`, LocalTypeInfo resolves `const JUNO_LOG_API_T *ptLoggerApi`
   → `apiType = "JUNO_LOG_API_T"`
5. Step 5: `vtableAssignments["JUNO_LOG_API_T"]["LogInfo"]` → `[{ file: "examples/example_project/main.c", line: 60 }]`
6. Step 6: `{ found: true, locations: [{ functionName: "LogInfo", file: "examples/example_project/main.c", line: 60 }] }`

**Expected result:**
```typescript
{ found: true, locations: [{ functionName: "LogInfo", file: "examples/example_project/main.c", line: 60 }] }
```
**Strategy used:** chain-walk (Category 3 — direct API pointer)  
**Requirement:** REQ-VSCODE-003, REQ-VSCODE-005

---

### Test Case ID: TC-RES-002
**Scenario:** `ptAppList[i]->ptApi->OnStart` — array subscript receiver (chain-walk (Category 1) path)  
**Lesson Learned:** #2  
**Source file context (`main.c`, lines 224–235):**
```c
    static JUNO_APP_ROOT_T *ptAppList[2] = {
        &tSystemManagerApp.tRoot,
        &tEngineApp.tRoot
    };
    ...
    for(size_t i = 0; i < 2; i++)
    {
        tStatus = ptAppList[i]->ptApi->OnStart(ptAppList[i]);
```
**Cursor:** `main.c`, line 234, column 40 (on `OnStart`)  

**Index state required:**
```typescript
moduleRoots: { "JUNO_APP_ROOT_T": "JUNO_APP_API_T" }
apiStructFields: { "JUNO_APP_API_T": ["OnStart", "OnProcess", "OnExit"] }
vtableAssignments: {
  "JUNO_APP_API_T": {
    "OnStart": [
      { functionName: "OnStart", file: "examples/example_project/engine/src/engine_app.c", line: 128 },
      { functionName: "OnStart", file: "examples/example_project/system_manager/src/system_manager_app.c", line: <TBD> }
    ]
  }
}
```

**Resolution trace:**
1. `fieldName = "OnStart"` (chain-walk Step 3 (field name extraction))
2. Chain-walk (Category 1): `ptApi->OnStart(` matched; pre-ptApi expression = `ptAppList[i]`
3. Strip `[i]` → `baseVar = "ptAppList"`
4. LocalTypeInfo lookup: `static JUNO_APP_ROOT_T *ptAppList[2]` → `rootType = "JUNO_APP_ROOT_T"`
5. Derivation chain: no parent → `rootType = "JUNO_APP_ROOT_T"`
6. `apiType = moduleRoots["JUNO_APP_ROOT_T"]` = `"JUNO_APP_API_T"`
7. `vtableAssignments["JUNO_APP_API_T"]["OnStart"]` → 2 entries (engine + system_manager)
8. `{ found: true, locations: [engine OnStart, system_manager OnStart] }`

**Expected result:** Two locations → QuickPick presented to user (REQ-VSCODE-006)  
**Strategy used:** chain-walk (Category 1) (with subscript stripping)  
**Requirement:** REQ-VSCODE-003, REQ-VSCODE-006

---

### Test Case ID: TC-RES-003
**Scenario:** `ptEngineApp->ptBroker->ptApi->RegisterSubscriber` — chained member access (chain-walk (Category 1))  
**Lesson Learned:** #3  
**Source file context (`engine_app.c`, line 151):**
```c
    tStatus = ptEngineApp->ptBroker->ptApi->RegisterSubscriber(ptEngineApp->ptBroker, &ptEngineApp->tCmdPipe);
```
**Cursor:** `engine_app.c`, line 151, column 45 (on `RegisterSubscriber`)  

**Index state required:**
```typescript
moduleRoots: { "JUNO_SB_BROKER_ROOT_T": "JUNO_SB_BROKER_API_T" }
derivationChain: { "ENGINE_APP_T": "JUNO_APP_ROOT_T" }
// struct member registry (from indexer BUILD step):
// ENGINE_APP_T has member ptBroker: JUNO_SB_BROKER_ROOT_T *
vtableAssignments: {
  "JUNO_SB_BROKER_API_T": {
    "RegisterSubscriber": [
      { functionName: "RegisterSubscriber", file: "src/juno_broker.c", line: 76 }
    ]
  }
}
functionDefinitions: {
  "RegisterSubscriber": [{ file: "src/juno_broker.c", line: 76, isStatic: true }]
}
```

**Resolution trace:**
1. `fieldName = "RegisterSubscriber"` (chain-walk Step 3 (field name extraction))
2. Chain-walk (Category 1): `ptApi->RegisterSubscriber(` matched; pre-ptApi sub-expression = `ptEngineApp->ptBroker`
3. Form: `outerVar->memberName`. Resolve `ptEngineApp` type via LocalTypeInfo lookup:
   `ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp)` → `outerType = "ENGINE_APP_T"`
4. Look up `ptBroker` as member of `ENGINE_APP_T` → `JUNO_SB_BROKER_ROOT_T *`
5. `rootType = "JUNO_SB_BROKER_ROOT_T"`
6. Derivation chain: no parent
7. `apiType = "JUNO_SB_BROKER_API_T"`
8. `vtableAssignments["JUNO_SB_BROKER_API_T"]["RegisterSubscriber"]` → 1 entry

**Expected result:**
```typescript
{ found: true, locations: [{ functionName: "RegisterSubscriber", file: "src/juno_broker.c", line: 76 }] }
```
**Strategy used:** chain-walk (Category 1) (chained resolution)  
**Requirement:** REQ-VSCODE-003, REQ-VSCODE-005

---

### Test Case ID: TC-RES-004
**Scenario:** `ptHeap->ptHeapPointerApi->Compare` — non-ptApi member (chain-walk (Category 4 — apiMemberRegistry) path)  
**Lesson Learned:** #5  
**Source file context (`src/juno_heap.c`, line 151):**
```c
        JUNO_DS_HEAP_COMPARE_RESULT_T tCompareResult = ptHeap->ptHeapPointerApi->Compare(ptHeap, JUNO_OK(tResultParent), JUNO_OK(tResultCurrent));
```
**Cursor:** `juno_heap.c`, line 151, column 64 (on `Compare`)  

**Index state required:**
```typescript
apiMemberRegistry: { "ptHeapPointerApi": "JUNO_DS_HEAP_POINTER_API_T" }
apiStructFields: { "JUNO_DS_HEAP_POINTER_API_T": ["Compare", "Swap"] }
vtableAssignments: {
  "JUNO_DS_HEAP_POINTER_API_T": {
    "Compare": [
      { functionName: "UserCompare", file: "examples/example_project/...", line: <user-provided> }
    ]
  }
}
```

**Resolution trace:**
1. `fieldName = "Compare"` (chain-walk Step 3 (field name extraction))
2. Chain-walk (Category 1): no `ptApi->Compare(` → fail
3. Chain-walk (Category 4 — apiMemberRegistry): `->ptHeapPointerApi->Compare(` matched. `ptHeapPointerApi` in `apiMemberRegistry`
   → `apiType = "JUNO_DS_HEAP_POINTER_API_T"` → GOTO Step 5 (skip Steps 3–4)
4. `vtableAssignments["JUNO_DS_HEAP_POINTER_API_T"]["Compare"]` → user-provided entries

**Expected result:** `{ found: true, locations: [user-provided Compare implementations] }`  
**Strategy used:** chain-walk (Category 4 — apiMemberRegistry)  
**Requirement:** REQ-VSCODE-003, REQ-VSCODE-005

---

### Test Case ID: TC-RES-005
**Scenario:** `tReturn.ptApi->Copy` — dot-accessed ptApi on JUNO_POINTER_T (chain-walk (Category 2))  
**Lesson Learned:** #4  
**Source file context (`src/juno_buff_stack.c`, line 66):**
```c
        tStatus = tReturn.ptApi->Copy(tReturn, tResult.tOk);
```
**Cursor:** `juno_buff_stack.c`, line 66, column 27 (on `Copy`)  

**Index state required:**
```typescript
traitRoots: { "JUNO_POINTER_T": "JUNO_POINTER_API_T" }
apiStructFields: { "JUNO_POINTER_API_T": ["Copy", "Reset"] }
vtableAssignments: {
  "JUNO_POINTER_API_T": {
    "Copy": [
      { functionName: "JunoMemory_Copy", file: "src/juno_memory_block.c", line: <TBD> }
    ]
  }
}
```

**Resolution trace:**
1. `fieldName = "Copy"` (chain-walk Step 3 (field name extraction))
2. Chain-walk (Category 2): `.ptApi->Copy(` matched (dot-form); pre-ptApi expression = `tReturn`
3. LocalTypeInfo lookup: `JUNO_POINTER_T tReturn` (function parameter) → `rootType = "JUNO_POINTER_T"`
4. Derivation chain: no parent
5. `apiType = index.traitRoots.get("JUNO_POINTER_T")` → `"JUNO_POINTER_API_T"`
6. `vtableAssignments["JUNO_POINTER_API_T"]["Copy"]` → concrete implementations

**Expected result:** `{ found: true, locations: [Copy implementation(s)] }`  
**Strategy used:** chain-walk (Category 2)  
**Requirement:** REQ-VSCODE-003, REQ-VSCODE-005

---

### Test Case ID: TC-RES-006
**Scenario:** No implementation found — error path  
**Source file context *(synthetic)*:**
```c
    tStatus = ptMyModule->ptApi->Launch(ptMyModule);
```
**Index state:** `JUNO_APP_API_T` has no field `"Launch"`. No other API type has `"Launch"`.  
**Cursor:** on `Launch`  

**Resolution trace:**
1. `fieldName = "Launch"` (chain-walk Step 3 (field name extraction))
2. Chain-walk categories 1–5: all fail or resolve to no match
3. Chain-walk (Category 6 — field name fallback): 0 API types contain `"Launch"` → `{ found: false, ... }`

**Expected result:** `{ found: false, errorMsg: "No API type contains field 'Launch'." }`  
**UI behavior:** Status bar message shown (informative, non-intrusive — REQ-VSCODE-013);
no navigation occurs.  
**Requirement:** REQ-VSCODE-004, REQ-VSCODE-013

---

### Test Case ID: TC-RES-007
**Scenario:** Multiple implementations found — QuickPick path  
**Source file context (`main.c`, line 234):**
```c
        tStatus = ptAppList[i]->ptApi->OnStart(ptAppList[i]);
```
**Index state (two apps registered):**
```typescript
vtableAssignments: {
  "JUNO_APP_API_T": {
    "OnStart": [
      { functionName: "OnStart", file: "examples/example_project/engine/src/engine_app.c", line: 128 },
      { functionName: "OnStart", file: "examples/example_project/system_manager/src/system_manager_app.c", line: <TBD> }
    ]
  }
}
```
**Resolution result:**
```typescript
{ found: true, locations: [engine OnStart, system_manager OnStart] }
```
**Expected UI behavior:** `vscode.window.showQuickPick` is presented with:
```
OnStart — engine_app.c:128  (examples/example_project/engine/src/engine_app.c)
OnStart — system_manager_app.c:<line>  (examples/example_project/system_manager/src/...)
```
User selects one → navigate to that file + line.  
**Requirement:** REQ-VSCODE-006

---

## Summary Table

| Test Case ID | Visitor / Section | Scenario | Category | Requirement |
|-------------|---------|----------|----------|-------------|
| TC-P1-001 | visitStructDefinition | JUNO_LOG_ROOT_TAG, JUNO_MODULE_EMPTY | — | REQ-VSCODE-008 |
| TC-P1-002 | visitStructDefinition | JUNO_DS_HEAP_ROOT_TAG, extra members | — | REQ-VSCODE-008 |
| TC-P1-003 | visitStructDefinition | JUNO_SB_BROKER_ROOT_TAG | — | REQ-VSCODE-008 |
| TC-P1-004 | visitStructDefinition | Negative: JUNO_MODULE_DERIVE | — | REQ-VSCODE-008 |
| TC-P2-001 | visitStructDefinition | ENGINE_APP_TAG derives JUNO_APP_ROOT_T | — | REQ-VSCODE-009 |
| TC-P2-002 | visitStructDefinition | JUNO_SB_PIPE_TAG derives JUNO_DS_QUEUE_ROOT_T | — | REQ-VSCODE-009 |
| TC-P2-003 | visitStructDefinition | Negative: JUNO_MODULE_ROOT | — | REQ-VSCODE-009 |
| TC-P3-001 | visitStructDefinition | JUNO_POINTER_TAG, JUNO_TRAIT_ROOT | — | REQ-VSCODE-014 |
| TC-P3-002 | visitStructDefinition | Negative: JUNO_MODULE_ROOT | — | REQ-VSCODE-014 |
| TC-P3-003 | visitStructDefinition | Negative: JUNO_MODULE_DERIVE | — | REQ-VSCODE-014 |
| TC-P4-001 | visitStructDefinition | MY_POINTER_IMPL_TAG, JUNO_TRAIT_DERIVE (synthetic) | — | REQ-VSCODE-015 |
| TC-P4-002 | visitStructDefinition | Negative: JUNO_TRAIT_ROOT (synthetic) | — | REQ-VSCODE-015 |
| TC-P5-001 | visitStructDefinition | JUNO_DS_HEAP_API_T: [Insert, Heapify, Pop] | — | REQ-VSCODE-012 |
| TC-P5-002 | visitStructDefinition | JUNO_LOG_API_T: [LogDebug, LogInfo, LogWarning, LogError] | — | REQ-VSCODE-012 |
| TC-P5-003 | visitStructDefinition | JUNO_DS_HEAP_POINTER_API_T: [Compare, Swap] | — | REQ-VSCODE-012 |
| TC-P5-004 | visitStructDefinition | JUNO_POINTER_API_T: [Copy, Reset] | — | REQ-VSCODE-012 |
| TC-P5-005 | visitStructDefinition | JUNO_SB_BROKER_API_T: [Publish, RegisterSubscriber] | — | REQ-VSCODE-012 |
| TC-P6-001 | visitVtableDeclaration | Designated: tEngineAppApi (OnStart, OnProcess, OnExit) | — | REQ-VSCODE-010 |
| TC-P6-002 | visitVtableDeclaration | Negative: positional initializer → zero designated matches | — | REQ-VSCODE-010 |
| TC-P6-003 | visitVtableDeclaration | Negative: non-API-T type variable | — | REQ-VSCODE-010 |
| TC-P7-001 | visitVtableDeclaration | Direct assignment tHeapApi.Insert (synthetic) | — | REQ-VSCODE-011 |
| TC-P7-002 | visitVtableDeclaration | Disambiguation vs P6 (synthetic) | — | REQ-VSCODE-011 |
| TC-P8-001 | visitVtableDeclaration | Positional: JUNO_DS_HEAP_API_T, juno_heap.c | — | REQ-VSCODE-012 |
| TC-P8-002 | visitVtableDeclaration | Positional: JUNO_SB_BROKER_API_T, static functions | — | REQ-VSCODE-012 |
| TC-P8-003 | visitVtableDeclaration | Positional: JUNO_LOG_API_T, main.c | — | REQ-VSCODE-012 |
| TC-P8-004 | visitVtableDeclaration | Negative: designated initializer → no fallthrough to P8 | — | REQ-VSCODE-012 |
| TC-P9-001 | Chain-walk | Now, cursor confirmed | — | REQ-VSCODE-003 |
| TC-P9-002 | Chain-walk | LogInfo via indirect variable | — | REQ-VSCODE-003 |
| TC-P9-003 | Chain-walk | Negative: plain member access, no `(` | — | REQ-VSCODE-003 |
| TC-P9-004 | Chain-walk | Negative: cursor on whitespace | — | REQ-VSCODE-003 |
| TC-P9-005 | Chain-walk | Negative: free function call | — | REQ-VSCODE-003 |
| TC-P9-101 | Chain-walk | Simple pointer: ptTime->ptApi->Now | Cat. 1 | REQ-VSCODE-003 |
| TC-P9-102 | Chain-walk | Simple pointer: ptBroker->ptApi->Publish | Cat. 1 | REQ-VSCODE-003 |
| TC-P9-103 | Chain-walk | Array subscript: ptAppList[i]->ptApi->OnStart | Cat. 1 | REQ-VSCODE-003 |
| TC-P9-104 | Chain-walk | Array subscript (var index): ptAppList[iCounter]->ptApi->OnProcess | Cat. 1 | REQ-VSCODE-003 |
| TC-P9-105 | Chain-walk | Chained member: ptEngineApp->ptBroker->ptApi->RegisterSubscriber | Cat. 1 | REQ-VSCODE-003 |
| TC-P9-201 | Chain-walk | Dot-accessed: tReturn.ptApi->Copy | Cat. 1 | REQ-VSCODE-003 |
| TC-P9-202 | Chain-walk | Nested dot: tPtrResult.tOk.ptApi->Copy | Cat. 1 | REQ-VSCODE-003 |
| TC-P9-203 | Chain-walk | Dot-accessed: tItem.ptApi->Copy | Cat. 1 | REQ-VSCODE-003 |
| TC-P9-301 | Chain-walk | Indirect: ptLoggerApi->LogInfo | Cat. 3 | REQ-VSCODE-003 |
| TC-P9-302 | Chain-walk | Indirect: ptCmdPipeApi->Dequeue | Cat. 3 | REQ-VSCODE-003 |
| TC-P9-303 | Chain-walk | Indirect: ptApi->GetAt (generic name) | Cat. 3 | REQ-VSCODE-003 |
| TC-P9-401 | Chain-walk | Non-ptApi member: ptHeapPointerApi->Compare | Cat. 4 | REQ-VSCODE-003 |
| TC-P9-402 | Chain-walk | Non-ptApi member: ptHeapPointerApi->Swap | Cat. 4 | REQ-VSCODE-003 |
| TC-P9-501 | Chain-walk | Macro: JUNO_MODULE_GET_API()->Insert (synthetic) | Cat. 5 | REQ-VSCODE-003 |
| TC-P9-601 | Chain-walk | Fallback: unknown receiver, unique field | Cat. 6 | REQ-VSCODE-003 |
| TC-P9-602 | Chain-walk | Fallback: unknown receiver, shared field name | Cat. 6 | REQ-VSCODE-003,006 |
| TC-P9-603 | Chain-walk | Fallback: unknown field → not found | Cat. 6 | REQ-VSCODE-003,004 |
| TC-P10-001 | visitFailureHandlerAssignment | Direct: ptHeap->_pfcnFailureHandler | — | REQ-VSCODE-016 |
| TC-P10-002 | visitFailureHandlerAssignment | Macro form: JUNO_FAILURE_HANDLER (alternation token) | — | REQ-VSCODE-016 |
| TC-P10-003 | visitFailureHandlerAssignment | Negative: ptApi assignment | — | REQ-VSCODE-016 |
| TC-P11-001 | visitFunctionDefinition | Match: static LogInfo (varied return type) | — | REQ-VSCODE-003 |
| TC-P11-002 | visitFunctionDefinition | Match: non-static JunoDs_Heap_Insert | — | REQ-VSCODE-003 |
| TC-P11-003 | visitFunctionDefinition | Allman braces: static inline Verify (grammar handles natively) | — | REQ-VSCODE-003 |
| TC-P11-004 | visitFunctionDefinition | Match: void FailureHandler | — | REQ-VSCODE-003 |
| TC-P11-005 | visitFunctionDefinition | Match: static result-type Now | — | REQ-VSCODE-003 |
| TC-P11-006 | visitFunctionDefinition | No match: forward declaration (`;`) | — | REQ-VSCODE-003 |
| TC-P11-007 | visitFunctionDefinition | No match: header prototype (`;`) | — | REQ-VSCODE-003 |
| TC-RES-001 | E2E Resolution | LogInfo via indirect API pointer (Cat. 3) | Cat. 3 | REQ-VSCODE-005 |
| TC-RES-002 | E2E Resolution | OnStart via array subscript (Cat. 1 + QuickPick) | Cat. 1 | REQ-VSCODE-006 |
| TC-RES-003 | E2E Resolution | RegisterSubscriber via chained member (Cat. 1) | Cat. 1 | REQ-VSCODE-005 |
| TC-RES-004 | E2E Resolution | Compare via non-ptApi member (Cat. 4) | Cat. 4 | REQ-VSCODE-005 |
| TC-RES-005 | E2E Resolution | Copy via dot-accessed ptApi (Cat. 2) | Cat. 1 | REQ-VSCODE-005 |
| TC-RES-006 | E2E Resolution | No implementation found — error path | Cat. 6 | REQ-VSCODE-004,013 |
| TC-RES-007 | E2E Resolution | Multiple implementations — QuickPick | Cat. 1 | REQ-VSCODE-006 |

---

## Implementation Notes for Jest Developer

1. **Test doubles:** Use a plain `NavigationIndex` object pre-populated with the "Index state" from each
   TC-RES test. Inject it into the `VtableResolver` constructor. No file system access needed for
   resolution tests.

2. **LocalTypeInfo injection:** For chain-walk tests (TC-P9-101 through TC-P9-603), pre-populate
   `LocalTypeInfo` with the variable/parameter type entries described in each test case's "Index state"
   section. Inject it alongside the `NavigationIndex`. Do not read actual files in unit tests.

3. **TC-P11-003 (Allman braces):** The Chevrotain grammar handles Allman brace style natively
   (whitespace-insensitive). Write the test expecting a MATCH. No special workaround is needed.

4. **TC-P10-002 (macro form):** The `JunoFailureHandler` alternation token
   (`/JUNO_FAILURE_HANDLER\b|_pfcnFailureHandler\b/`) closes the previously identified gap.
   Write the test expecting a MATCH. No separate "gap reminder" test is needed.

5. **Column numbers:** Jest tests should compute column numbers using `line.indexOf(fieldName)` on
   the test input string rather than hardcoding, to avoid brittleness if test strings are reformatted.

6. **Chain-walk category coverage:** For TC-P9-303, write an explicit test asserting that the
   chain-walk resolves `ptApi` as a direct API pointer (Category 3) rather than as a member access
   chain (Category 1). This prevents a regression where both resolution paths could claim the same
   call site.

---

## Section 13: VSCode Integration Tests

These test cases verify the extension's integration with the VSCode Extension API: activation,
provider registration, command registration, and passthrough behavior on non-LibJuno call sites.
They correspond to REQ-VSCODE-001, REQ-VSCODE-002, and REQ-VSCODE-007.

**Test double approach:** Use a VSCode API stub that captures `registerDefinitionProvider`,
`registerCommand`, and `languages` calls via jest spy functions injected into the
`JunoDefinitionProvider` and command handlers under test.

---

### Test Case ID: TC-VSC-001
**Scenario:** Extension activates when a C file is opened  
**Setup:**
- Stub `vscode.workspace.findFiles` to return a list of `.c` and `.h` files.
- Stub `vscode.languages.registerDefinitionProvider` as a jest spy.
- Stub `vscode.commands.registerCommand` as a jest spy.
- Call the exported `activate(context)` function with a mock `ExtensionContext`.

**Expected result:**
- `activate()` completes without throwing.
- `vscode.languages.registerDefinitionProvider` is called at least once before `activate()` returns.
- `vscode.commands.registerCommand` is called at least twice (for the two registered commands).

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-VSC-002
**Scenario:** `JunoDefinitionProvider` is registered for both `c` and `cpp` language IDs  
**Setup:**
- Capture the first argument to `vscode.languages.registerDefinitionProvider` during activation.

**Expected result:**
- The document selector argument contains an entry with `{ language: 'c' }`.
- The document selector argument contains an entry with `{ language: 'cpp' }`.
- The second argument is an instance implementing `provideDefinition`.

**Requirement:** REQ-VSCODE-001, REQ-VSCODE-007

---

### Test Case ID: TC-VSC-003
**Scenario:** F12 / Ctrl+Click on a vtable call site triggers `JunoDefinitionProvider` and returns a location  
**Setup:**
- Pre-populate `NavigationIndex` with sufficient data to resolve `ptTime->ptApi->Now(ptTime)`:
  ```typescript
  moduleRoots: { "JUNO_TIME_ROOT_T": "JUNO_TIME_API_T" }
  vtableAssignments: {
    "JUNO_TIME_API_T": {
      "Now": [{ functionName: "Now", file: "examples/example_project/main.c", line: 110 }]
    }
  }
  ```
- Stub `provideDefinition` context: document line =
  `    JUNO_TIMESTAMP_RESULT_T tTimestampResult = ptTime->ptApi->Now(ptTime);`
  with surrounding lines providing `const JUNO_TIME_ROOT_T *ptTime = ptEngineApp->ptTime;`.
- Position: line containing `Now`, column on `Now`.

**Expected result:**
- `provideDefinition` returns a non-null `LocationLink[]` with one entry.
- The entry's `targetUri` resolves to `examples/example_project/main.c`.
- The entry's `targetRange` starts at line 110 (0-based: 109).

**Requirement:** REQ-VSCODE-002, REQ-VSCODE-007

---

### Test Case ID: TC-VSC-004
**Scenario:** F12 on a non-vtable call falls through (provider returns `undefined`)  
**Setup:**
- Document line: `    printf("hello, world\n");`
- Position: column on `printf`.
- `NavigationIndex` may be empty.

**Expected result:**
- `provideDefinition` returns `undefined` (not an error, not an empty array).
- No status bar message is shown for this case (non-libjuno call is silently ignored).

**Note:** Returning `undefined` allows VSCode to fall through to the default C/C++ language
server provider, preserving native Go to Definition for ordinary function calls.

**Requirement:** REQ-VSCODE-007

---

### Test Case ID: TC-VSC-005
**Scenario:** `libjuno.goToImplementation` command is registered  
**Setup:**
- Capture all `registerCommand` calls during `activate()`.

**Expected result:**
- At least one call has command ID `"libjuno.goToImplementation"`.
- The registered handler is a function.

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-VSC-006
**Scenario:** `libjuno.reindexWorkspace` command clears cache and rebuilds the index  
**Setup:**
- Spy on `WorkspaceIndexer.reindex()` and `CacheManager.clear()`.
- Invoke the `libjuno.reindexWorkspace` command handler directly.

**Expected result:**
- `CacheManager.clear()` is called (or the in-memory index is reset) before indexing begins.
- `WorkspaceIndexer.reindex()` (or equivalent full-scan entry point) is called exactly once.
- After the handler resolves, `NavigationIndex` is non-empty (assuming stub files were provided).

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-VSC-007
**Scenario:** `provideDefinition` returns `undefined` and does NOT throw on a line that matches
`->fieldName(` syntax but whose type cannot be determined by any strategy  
**Setup:**
- Document line: `    tStatus = pUnknown->ptApi->Mystery(pUnknown);`
- Index state: no API type contains field `"Mystery"`.
- No surrounding lines provide a type declaration for `pUnknown`.

**Expected result:**
- `provideDefinition` returns `undefined` without throwing.
- `VtableResolver.resolve()` returns `{ found: false, errorMsg: "..." }`.
- A status bar message is triggered (verified separately in TC-ERR tests).

**Requirement:** REQ-VSCODE-002, REQ-VSCODE-004

---

## Section 14: Error Handling UX Tests

These test cases verify the non-intrusive error reporting behavior described in design Section 8.
They cover REQ-VSCODE-004 and REQ-VSCODE-013.

**Test double approach:** Inject a stub `StatusBarHelper` and stub `vscode.window` into
`JunoDefinitionProvider`. Capture all `showInformationMessage`, `showErrorMessage`, and
`statusBarItem.text` calls.

---

### Test Case ID: TC-ERR-001
**Scenario:** Status bar message is displayed when resolution fails (non-intrusive)  
**Setup:**
- Wire `JunoDefinitionProvider` with a `VtableResolver` stub that returns
  `{ found: false, errorMsg: "No implementation found for 'JUNO_APP_API_T::Launch'." }`.
- Provide a spy `statusBarItem` injected into `StatusBarHelper`.
- Trigger `provideDefinition` at a call site that the resolver cannot satisfy.

**Expected result:**
- `statusBarItem.text` is set to a string containing `"LibJuno"` and the error message.
- `statusBarItem.show()` is called exactly once.
- `vscode.window.showErrorMessage` is NOT called.

**Requirement:** REQ-VSCODE-004, REQ-VSCODE-013

---

### Test Case ID: TC-ERR-002
**Scenario:** Status bar message auto-clears after 5 seconds  
**Setup:**
- Use Jest fake timers (`jest.useFakeTimers()`).
- Trigger resolution failure as in TC-ERR-001.
- Advance timers by 4999 ms.

**Expected result (at 4999 ms):**
- `statusBarItem.hide()` has NOT been called.

**Then advance timers by 1 ms (total 5000 ms):**
- `statusBarItem.hide()` is called.

**Requirement:** REQ-VSCODE-013

---

### Test Case ID: TC-ERR-003
**Scenario:** Information message with "Show Details" appears on repeated failure within 10 seconds  
**Setup:**
- Use Jest fake timers.
- Stub `vscode.window.showInformationMessage` as a jest spy returning a Promise resolving to `undefined`.
- Trigger resolution failure at T=0 ms.
- Advance timers by 5000 ms (first status bar clears).
- Trigger resolution failure again at T=8000 ms (within the 10-second window).

**Expected result:**
- `vscode.window.showInformationMessage` is called with:
  - First argument: the error message string.
  - Second argument: `"Show Details"`.
- This is the second failure, so the information message fires on the second trigger.

**Requirement:** REQ-VSCODE-013

---

### Test Case ID: TC-ERR-004
**Scenario:** No modal dialog (`showErrorMessage`) is ever shown for resolution failures  
**Setup:**
- Spy on `vscode.window.showErrorMessage`.
- Trigger three consecutive resolution failures.

**Expected result:**
- `vscode.window.showErrorMessage` is NEVER called.

**Requirement:** REQ-VSCODE-013

---

### Test Case ID: TC-ERR-005
**Scenario:** Error message includes the specific failure reason with API type and field name  
**Setup:**
- Index state: `JUNO_DS_HEAP_API_T` exists in `apiStructFields` with field `"Insert"`, but
  `vtableAssignments["JUNO_DS_HEAP_API_T"]["Insert"]` is empty (no concrete implementation).
- Trigger `VtableResolver.resolve()` for a call site on `Insert` with `apiType = "JUNO_DS_HEAP_API_T"`.

**Expected result:**
- `VtableResolutionResult.errorMsg` contains both `"JUNO_DS_HEAP_API_T"` and `"Insert"`.
  (For example: `"No implementation found for 'JUNO_DS_HEAP_API_T::Insert'."`)
- The status bar message text propagates this specific error message (not a generic one).

**Requirement:** REQ-VSCODE-004, REQ-VSCODE-013

---

### Test Case ID: TC-ERR-006
**Scenario:** MCP tool returns a proper error object (not an HTTP error code) when resolution fails  
**Setup:**
- Start the MCP server with a `VtableResolver` stub returning
  `{ found: false, errorMsg: "No implementation found for 'JUNO_DS_HEAP_API_T::Insert'." }`.
- Send a `resolve_vtable_call` request with a file/line/column that the stub will fail on.

**Expected result:**
- HTTP response status: `200 OK` (not 4xx or 5xx).
- Response body JSON:
  ```json
  {
    "found": false,
    "locations": [],
    "error": "No implementation found for 'JUNO_DS_HEAP_API_T::Insert'."
  }
  ```
- The MCP `result` object contains `isError: true` (per MCP protocol).

**Requirement:** REQ-VSCODE-004

---

## Section 15: MCP Server Tests

These test cases verify the embedded MCP server described in design Section 7.
They cover REQ-VSCODE-017, REQ-VSCODE-018, REQ-VSCODE-019, and REQ-VSCODE-020.

**Test double approach:** Start the MCP server in-process during `beforeEach` with an injected
`NavigationIndex` stub and a configurable port. Use Node.js `http.request` or `fetch` to send
requests. Shut down the server in `afterEach`.

---

### Test Case ID: TC-MCP-001
**Scenario:** MCP server starts on extension activation  
**Setup:**
- Spy on `McpServer.start()` (or equivalent).
- Call `activate(context)` with a mock `ExtensionContext`.

**Expected result:**
- `McpServer.start()` is called before `activate()` returns.
- The server is listening on `127.0.0.1` at the configured port (default 6543).
- A subsequent HTTP GET to `http://127.0.0.1:6543/mcp` returns HTTP 200 or a valid MCP
  response (not a connection-refused error).

**Requirement:** REQ-VSCODE-017

---

### Test Case ID: TC-MCP-002
**Scenario:** `resolve_vtable_call` tool is registered with correct input/output schema  
**Setup:**
- Query the MCP server's tool list endpoint (or inspect the registered tool metadata directly
  from `McpServer`'s tool registry after construction).

**Expected result:**
- A tool named `"resolve_vtable_call"` is present.
- Its input schema requires `file` (string), `line` (integer), `column` (integer).
- Its output schema has `found` (boolean), `locations` (array), `error` (string, optional).

**Requirement:** REQ-VSCODE-018

---

### Test Case ID: TC-MCP-003
**Scenario:** `resolve_failure_handler` tool is registered with correct input/output schema  
**Setup:**
- Inspect the MCP server's tool registry after construction.

**Expected result:**
- A tool named `"resolve_failure_handler"` is present.
- Its input schema is identical to `resolve_vtable_call` (file, line, column).
- Its output schema is identical to `resolve_vtable_call` (found, locations, error).

**Requirement:** REQ-VSCODE-019

---

### Test Case ID: TC-MCP-004
**Scenario:** `resolve_vtable_call` with valid input returns `found: true` with correct locations  
**Setup:**
- Pre-populate `NavigationIndex`:
  ```typescript
  moduleRoots: { "JUNO_DS_HEAP_ROOT_T": "JUNO_DS_HEAP_API_T" }
  vtableAssignments: {
    "JUNO_DS_HEAP_API_T": {
      "Insert": [{ functionName: "JunoDs_Heap_Insert", file: "src/juno_heap.c", line: 259 }]
    }
  }
  ```
- Stub the source-text getter so that the call site line resolves with chain-walk (Category 1) using the
  above index.
- POST to `resolve_vtable_call` with `{ "file": "src/main.c", "line": 42, "column": 15 }`
  where the stub line at (file, 42) is:
  `    tStatus = ptHeap->ptApi->Insert(ptHeap, tValue);`
  and LocalTypeInfo provides `JUNO_DS_HEAP_ROOT_T *ptHeap`.

**Expected result:**
```json
{
  "found": true,
  "locations": [
    { "functionName": "JunoDs_Heap_Insert", "file": "src/juno_heap.c", "line": 259 }
  ]
}
```
**HTTP status:** 200

**Requirement:** REQ-VSCODE-018

---

### Test Case ID: TC-MCP-005
**Scenario:** `resolve_vtable_call` with no-match input returns `found: false` with error message  
**Setup:**
- `NavigationIndex` is empty (no API types registered).
- POST `{ "file": "src/main.c", "line": 1, "column": 5 }` where the stub line contains
  `    tStatus = ptFoo->ptApi->UnknownField(ptFoo);` with no matching type in LocalTypeInfo lookup.

**Expected result:**
```json
{
  "found": false,
  "locations": [],
  "error": "No API type contains field 'UnknownField'."
}
```
**HTTP status:** 200

**Requirement:** REQ-VSCODE-018

---

### Test Case ID: TC-MCP-006
**Scenario:** `resolve_failure_handler` with valid input returns `found: true`  
**Setup:**
- Pre-populate `NavigationIndex`:
  ```typescript
  failureHandlerAssignments: {
    "JUNO_DS_HEAP_ROOT_T": [
      { functionName: "MyFailureHandler", file: "examples/example.c", line: 42 }
    ]
  }
  ```
- Stub the source-text getter so the call-site line is:
  `    ptHeap->_pfcnFailureHandler = MyFailureHandler;`
  and LocalTypeInfo provides `JUNO_DS_HEAP_ROOT_T *ptHeap`.
- POST `{ "file": "src/init.c", "line": 99, "column": 12 }`.

**Expected result:**
```json
{
  "found": true,
  "locations": [
    { "functionName": "MyFailureHandler", "file": "examples/example.c", "line": 42 }
  ]
}
```

**Requirement:** REQ-VSCODE-019

---

### Test Case ID: TC-MCP-007
**Scenario:** `resolve_failure_handler` with invalid input (no handler registered) returns `found: false`  
**Setup:**
- `failureHandlerAssignments` contains no entry for `"JUNO_APP_ROOT_T"`.
- Stub line: `    ptApp->_pfcnFailureHandler = SomeHandler;` with LocalTypeInfo providing
  `JUNO_APP_ROOT_T *ptApp`.
- POST `{ "file": "src/unknown.c", "line": 10, "column": 8 }`.

**Expected result:**
```json
{
  "found": false,
  "locations": [],
  "error": "No failure handler registered for 'JUNO_APP_ROOT_T'."
}
```

**Requirement:** REQ-VSCODE-019

---

### Test Case ID: TC-MCP-008
**Scenario:** `.libjuno/mcp.json` discovery file is written on activation  
**Setup:**
- Stub `fs.writeFile` (or `fs.promises.writeFile`) as a jest spy.
- Call `activate(context)` with a workspace root of `/workspace`.

**Expected result:**
- `fs.writeFile` (or equivalent) is called with a path ending in `.libjuno/mcp.json`.
- The content written is valid JSON containing:
  ```json
  {
    "mcpServers": {
      "libjuno": {
        "url": "http://127.0.0.1:6543/mcp"
      }
    }
  }
  ```

**Requirement:** REQ-VSCODE-017

---

### Test Case ID: TC-MCP-009
**Scenario:** MCP server binds to `127.0.0.1` only (not externally accessible)  
**Setup:**
- Start `McpServer` with default configuration.
- Query the bound address from the underlying `http.Server` (`server.address()`).

**Expected result:**
- `server.address().address` equals `"127.0.0.1"` (not `"0.0.0.0"` or `"::"` or `"*"`).

**Requirement:** REQ-VSCODE-017, REQ-VSCODE-020

---

### Test Case ID: TC-MCP-010
**Scenario:** MCP error responses use `isError: true` in the MCP result, not HTTP error codes  
**Setup:**
- Trigger any resolution failure through the MCP server (as in TC-MCP-005).

**Expected result:**
- HTTP response status is `200 OK`.
- The MCP protocol wrapper includes `"isError": true` at the result level.
- No HTTP 4xx or 5xx status codes are used for application-level errors.

**Requirement:** REQ-VSCODE-017

---

### Test Case ID: TC-MCP-011
**Scenario:** MCP tools work without any VSCode UI (headless/AI-only mode)  
**Setup:**
- Instantiate `McpServer` directly with an injected `NavigationIndex` stub,
  WITHOUT calling `activate()` and WITHOUT a VSCode window context.
- Start the server; send a `resolve_vtable_call` request.

**Expected result:**
- Server responds correctly (found or not-found) without referencing any
  `vscode.window.*` API.
- No `vscode` API calls are made inside `McpServer.handleResolveVtableCall()`.

**Requirement:** REQ-VSCODE-017, REQ-VSCODE-020

---

### Test Case ID: TC-MCP-012
**Scenario:** MCP implementation does not use any platform-specific AI API  
**Setup:**
- Inspect `vscode-extension/src/mcp/mcpServer.ts` for any imports from:
  - `@github/copilot-*`
  - `@anthropic-ai/*`
  - `openai`
  - Any other AI-provider-specific SDK package.
- This is a static inspection / code review test case.

**Expected result:**
- No imports from AI-provider-specific packages are found in any file under
  `vscode-extension/src/mcp/`.
- The only AI-interface mechanism is the MCP HTTP protocol itself
  (standard HTTP + JSON, no vendor SDK).

**Requirement:** REQ-VSCODE-020

---

## Section 16: Cache Tests

These test cases verify the Cache Manager described in design Section 9.
Cache behavior is related to REQ-VSCODE-003 (performance: avoiding full re-scan on every activation)
and REQ-VSCODE-001 (activation behavior).

**Test double approach:** Inject a stub file-system adapter into `CacheManager` so that
`readFile`, `writeFile`, `rename`, and `mkdir` calls are intercepted. Use in-memory maps
to simulate file content and hashes.

---

### Test Case ID: TC-CACHE-001
**Scenario:** Cache file is created at `.libjuno/navigation-cache.json` on first index  
**Setup:**
- Stub file system: no `.libjuno/navigation-cache.json` exists initially.
- Run `WorkspaceIndexer.index()` on a workspace containing two stub C files.

**Expected result:**
- `fs.writeFile` (or `fs.promises.writeFile`) is called with a path ending in
  `.libjuno/navigation-cache.json` (after the temp-file-and-rename step, if applicable).
- The written content is valid JSON matching the cache schema (has `version`, `fileHashes`,
  `moduleRoots`, etc.).

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-CACHE-002
**Scenario:** Cache is loaded on activation when present and valid  
**Setup:**
- Stub file system returns a pre-built cache JSON with `version = "1"` and `fileHashes`
  matching the current stub file content hashes.
- Spy on `CParser.parse()`.
- Call `WorkspaceIndexer.index()`.

**Expected result:**
- `CParser.parse()` is NOT called for any file whose hash matches the cache.
- The `NavigationIndex` is populated from the cache data, not from re-parsing.

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-CACHE-003
**Scenario:** Stale file (hash mismatch) triggers re-index of that file only  
**Setup:**
- Cache contains two files: `src/foo.c` (hash `"aaa"`) and `src/bar.c` (hash `"bbb"`).
- Stub file system: `src/foo.c` now has hash `"ccc"` (changed); `src/bar.c` still has `"bbb"`.
- Spy on `CParser.parse()`.

**Expected result:**
- `CParser.parse()` is called exactly once, for `src/foo.c`.
- `CParser.parse()` is NOT called for `src/bar.c`.
- After indexing, `fileHashes["src/foo.c"]` is updated to `"ccc"` in the cache.

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-CACHE-004
**Scenario:** New file (not in cache) is indexed and added to cache  
**Setup:**
- Cache contains only `src/foo.c`.
- Stub file system also contains `src/new_module.c` (not in cache).
- Spy on `CParser.parse()`.

**Expected result:**
- `CParser.parse()` is called for `src/new_module.c`.
- After indexing, `fileHashes["src/new_module.c"]` appears in the updated cache.

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-CACHE-005
**Scenario:** Deleted file is removed from cache and index  
**Setup:**
- Cache contains `src/foo.c` and `src/deleted.c`.
- Stub file system: `src/deleted.c` no longer exists (file not found on read).
- Run `WorkspaceIndexer.index()`.

**Expected result:**
- `fileHashes["src/deleted.c"]` is absent from the updated cache.
- Any index records sourced from `src/deleted.c` are removed from `NavigationIndex`.

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-CACHE-006
**Scenario:** `version` field mismatch triggers full re-index  
**Setup:**
- Cache has `"version": "0"` (old format); current extension expects `"version": "1"`.
- Spy on `CParser.parse()`.
- Stub file system contains two files.

**Expected result:**
- `CParser.parse()` is called for ALL files (full re-index), not only changed ones.
- The rewritten cache has `"version": "1"`.

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-CACHE-007
**Scenario:** `FileSystemWatcher` triggers re-index on `.c`, `.h`, `.cpp` file changes  
**Setup:**
- Register a `FileSystemWatcher` stub that can fire `onDidChange`, `onDidCreate`, `onDidDelete`
  events programmatically.
- Spy on `WorkspaceIndexer._reindexFile()` (or equivalent single-file re-index method).
- Fire `onDidChange` for `src/foo.c`.

**Expected result:**
- `_reindexFile("src/foo.c")` is called within the debounce window.
- `NavigationIndex` records from `src/foo.c` are updated.

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-CACHE-008
**Scenario:** Debounced write: rapid saves produce only one cache write  
**Setup:**
- Use Jest fake timers.
- Fire `onDidChange` for `src/foo.c` five times in 100 ms intervals (all within a 500 ms window).
- Spy on `CacheManager.write()`.
- Advance timers by 600 ms (past the 500 ms debounce threshold).

**Expected result:**
- `CacheManager.write()` is called exactly once (debounced, not five times).

**Requirement:** REQ-VSCODE-001

---

### Test Case ID: TC-CACHE-009
**Scenario:** Cache write is atomic (temp file + rename)  
**Setup:**
- Spy on `fs.writeFile` and `fs.rename` (or `fs.promises` equivalents).
- Trigger a cache write.

**Expected result:**
- `fs.writeFile` is called with a path that is NOT the final cache path
  (e.g., ends with `.tmp` or a random suffix).
- `fs.rename` is subsequently called, moving the temp file to `.libjuno/navigation-cache.json`.
- `fs.writeFile` is NOT called directly with the final cache path name.

**Requirement:** REQ-VSCODE-001

---

## Section 17: Failure Handler Navigation Tests

These test cases verify end-to-end navigation for failure handler assignments (REQ-VSCODE-016),
including the `JUNO_FAILURE_HANDLER` macro form, which is now handled by the `JunoFailureHandler` alternation token (closing the gap previously identified in TC-P10-002).

**Test double approach:** Inject a `NavigationIndex` stub and a source-text getter stub into
`FailureHandlerResolver`. Use the same resolver infrastructure as the vtable resolution tests.

---

### Test Case ID: TC-FH-001
**Scenario:** Ctrl+Click on `_pfcnFailureHandler` assignment resolves to handler function definition  
**Setup:**
- Index state:
  ```typescript
  failureHandlerAssignments: {
    "JUNO_DS_HEAP_ROOT_T": [
      { functionName: "MyHeapFailureHandler", file: "src/init.c", line: 42 }
    ]
  }
  ```
- Stub source text at call site line: `    ptHeap->_pfcnFailureHandler = MyHeapFailureHandler;`
- LocalTypeInfo provides: `JUNO_DS_HEAP_ROOT_T *ptHeap`.
- Cursor: column on `_pfcnFailureHandler`.

**Expected result:**
```typescript
{ found: true, locations: [{ functionName: "MyHeapFailureHandler", file: "src/init.c", line: 42 }] }
```

**Requirement:** REQ-VSCODE-016

---

### Test Case ID: TC-FH-002
**Scenario:** `JUNO_FAILURE_HANDLER` macro form — visitFailureHandlerAssignment resolves assignment  
**Setup:**
- This test verifies that the `JunoFailureHandler` alternation token correctly handles the macro form (gap from TC-P10-002 is closed).
- Index state:
  ```typescript
  failureHandlerAssignments: {
    "JUNO_APP_ROOT_T": [
      { functionName: "pfcnFailureHandler", file: "examples/example_project/engine/src/engine_app.c", line: 111 }
    ]
  }
  ```
- Stub source line (from `engine_app.c`, line 111):
  `    ptEngineApp->tRoot.JUNO_FAILURE_HANDLER = pfcnFailureHandler;`
- The visitFailureHandlerAssignment visitor (via the `JunoFailureHandler` alternation token) correctly parses this form.
- LocalTypeInfo context: `ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp);`
  → root type resolved via derivation chain: `ENGINE_APP_T` → `JUNO_APP_ROOT_T`.

**Expected result:**
```typescript
{ found: true, locations: [{ functionName: "pfcnFailureHandler", file: "examples/.../engine_app.c", line: 111 }] }
```

**Note:** The `JunoFailureHandler` alternation token (`/JUNO_FAILURE_HANDLER\b|_pfcnFailureHandler\b/`) handles both the macro form and the expanded member name as the same token, closing the previously identified gap. The resolver traces `pfcnFailureHandler` through the call chain to the concrete handler definition.

**Requirement:** REQ-VSCODE-016

---

### Test Case ID: TC-FH-003
**Scenario:** Failure handler with multiple assignments across files shows QuickPick  
**Setup:**
- Index state:
  ```typescript
  failureHandlerAssignments: {
    "JUNO_APP_ROOT_T": [
      { functionName: "EngineFailureHandler", file: "engine/src/engine_app.c", line: 111 },
      { functionName: "SysManFailureHandler", file: "sys_manager/src/sys_manager_app.c", line: 88 }
    ]
  }
  ```
- Stub source line: `    ptApp->_pfcnFailureHandler = EngineFailureHandler;`
- Backward type: `JUNO_APP_ROOT_T *ptApp`.

**Expected result:**
- `FailureHandlerResolver.resolve()` returns `{ found: true, locations: [2 entries] }`.
- `JunoDefinitionProvider.provideDefinition()` returns `undefined` (navigation delegated to QuickPick).
- `vscode.window.showQuickPick` is called with two items.

**Requirement:** REQ-VSCODE-016

---

### Test Case ID: TC-FH-004
**Scenario:** Failure handler with no assignments shows error  
**Setup:**
- `failureHandlerAssignments` contains no entry for `"JUNO_LOG_ROOT_T"`.
- Stub source line: `    ptLogger->_pfcnFailureHandler = SomeHandler;`
- Backward type: `JUNO_LOG_ROOT_T *ptLogger`.

**Expected result:**
```typescript
{ found: false, errorMsg: "No failure handler registered for 'JUNO_LOG_ROOT_T'." }
```
- Status bar message is shown (TC-ERR-001 behavior applies).

**Requirement:** REQ-VSCODE-016

---

## Section 18: Multi-Implementation QuickPick Tests

These test cases verify the QuickPick presentation described in design Section 5.2 and 6.3
(REQ-VSCODE-006).

**Test double approach:** Spy on `vscode.window.showQuickPick`. Inject it into the
`JunoDefinitionProvider` or `QuickPickHelper`. Capture the items array passed to it.

---

### Test Case ID: TC-QP-001
**Scenario:** QuickPick items show function name as label  
**Setup:**
- `VtableResolver.resolve()` returns two locations:
  ```typescript
  locations: [
    { functionName: "OnStart", file: "engine/src/engine_app.c", line: 128 },
    { functionName: "OnStart", file: "sys_manager/src/sys_manager_app.c", line: 77 }
  ]
  ```
- Spy on `vscode.window.showQuickPick`.

**Expected result:**
- `showQuickPick` is called with items where every item has `label: "OnStart"`.

**Requirement:** REQ-VSCODE-006

---

### Test Case ID: TC-QP-002
**Scenario:** QuickPick items show `file:line` as description  
**Setup:** Same as TC-QP-001.

**Expected result:**
- Item 0 has `description: "engine_app.c:128"` (basename + colon + line number).
- Item 1 has `description: "sys_manager_app.c:77"`.

**Requirement:** REQ-VSCODE-006

---

### Test Case ID: TC-QP-003
**Scenario:** QuickPick items show workspace-relative path as detail  
**Setup:** Same as TC-QP-001. Workspace root: `/workspace`.

**Expected result:**
- Item 0 has `detail: "engine/src/engine_app.c"` (workspace-relative path).
- Item 1 has `detail: "sys_manager/src/sys_manager_app.c"`.
- Neither `detail` string is an absolute path starting with `/`.

**Requirement:** REQ-VSCODE-006

---

### Test Case ID: TC-QP-004
**Scenario:** Selecting a QuickPick item navigates to the correct file and line  
**Setup:**
- Stub `vscode.window.showQuickPick` to return a Promise resolving to item 0 (the first entry).
- Spy on `vscode.window.showTextDocument` and `vscode.workspace.openTextDocument`.
- Trigger `provideDefinition` with a two-location resolution result.

**Expected result:**
- `vscode.workspace.openTextDocument` is called with the URI of `engine/src/engine_app.c`.
- `vscode.window.showTextDocument` is called with a `selection` range starting at line 128
  (0-based: 127).

**Requirement:** REQ-VSCODE-006

---

### Test Case ID: TC-QP-005
**Scenario:** Cancelling QuickPick does not navigate anywhere  
**Setup:**
- Stub `vscode.window.showQuickPick` to return a Promise resolving to `undefined`
  (user pressed Escape).
- Spy on `vscode.window.showTextDocument`.
- Trigger `provideDefinition` with a two-location resolution result.

**Expected result:**
- `vscode.window.showTextDocument` is NOT called.
- No navigation occurs.
- No error or status bar message is shown for cancellation.

**Requirement:** REQ-VSCODE-006

---

## Summary Table

| Test Case ID | Section | Scenario | Requirement(s) |
|-------------|---------|----------|----------------|
| TC-P1-001 | visitStructDefinition | JUNO_LOG_ROOT_TAG, JUNO_MODULE_EMPTY | REQ-VSCODE-008 |
| TC-P1-002 | visitStructDefinition | JUNO_DS_HEAP_ROOT_TAG, extra members | REQ-VSCODE-008 |
| TC-P1-003 | visitStructDefinition | JUNO_SB_BROKER_ROOT_TAG | REQ-VSCODE-008 |
| TC-P1-004 | visitStructDefinition | Negative: JUNO_MODULE_DERIVE | REQ-VSCODE-008 |
| TC-P2-001 | visitStructDefinition | ENGINE_APP_TAG derives JUNO_APP_ROOT_T | REQ-VSCODE-009 |
| TC-P2-002 | visitStructDefinition | JUNO_SB_PIPE_TAG derives JUNO_DS_QUEUE_ROOT_T | REQ-VSCODE-009 |
| TC-P2-003 | visitStructDefinition | Negative: JUNO_MODULE_ROOT | REQ-VSCODE-009 |
| TC-P3-001 | visitStructDefinition | JUNO_POINTER_TAG, JUNO_TRAIT_ROOT | REQ-VSCODE-014 |
| TC-P3-002 | visitStructDefinition | Negative: JUNO_MODULE_ROOT | REQ-VSCODE-014 |
| TC-P3-003 | visitStructDefinition | Negative: JUNO_MODULE_DERIVE | REQ-VSCODE-014 |
| TC-P4-001 | visitStructDefinition | MY_POINTER_IMPL_TAG, JUNO_TRAIT_DERIVE (synthetic) | REQ-VSCODE-015 |
| TC-P4-002 | visitStructDefinition | Negative: JUNO_TRAIT_ROOT (synthetic) | REQ-VSCODE-015 |
| TC-P5-001 | visitStructDefinition | JUNO_DS_HEAP_API_T: [Insert, Heapify, Pop] | REQ-VSCODE-012 |
| TC-P5-002 | visitStructDefinition | JUNO_LOG_API_T: [LogDebug, LogInfo, LogWarning, LogError] | REQ-VSCODE-012 |
| TC-P5-003 | visitStructDefinition | JUNO_DS_HEAP_POINTER_API_T: [Compare, Swap] | REQ-VSCODE-012 |
| TC-P5-004 | visitStructDefinition | JUNO_POINTER_API_T: [Copy, Reset] | REQ-VSCODE-012 |
| TC-P5-005 | visitStructDefinition | JUNO_SB_BROKER_API_T: [Publish, RegisterSubscriber] | REQ-VSCODE-012 |
| TC-P6-001 | visitVtableDeclaration | Designated: tEngineAppApi (OnStart, OnProcess, OnExit) | REQ-VSCODE-010 |
| TC-P6-002 | visitVtableDeclaration | Negative: positional initializer → zero designated matches | REQ-VSCODE-010 |
| TC-P6-003 | visitVtableDeclaration | Negative: non-API-T type variable | REQ-VSCODE-010 |
| TC-P7-001 | visitVtableDeclaration | Direct assignment tHeapApi.Insert (synthetic) | REQ-VSCODE-011 |
| TC-P7-002 | visitVtableDeclaration | Disambiguation vs P6 (synthetic) | REQ-VSCODE-011 |
| TC-P8-001 | visitVtableDeclaration | Positional: JUNO_DS_HEAP_API_T, juno_heap.c | REQ-VSCODE-012 |
| TC-P8-002 | visitVtableDeclaration | Positional: JUNO_SB_BROKER_API_T, static functions | REQ-VSCODE-012 |
| TC-P8-003 | visitVtableDeclaration | Positional: JUNO_LOG_API_T, main.c | REQ-VSCODE-012 |
| TC-P8-004 | visitVtableDeclaration | Negative: designated initializer → no fallthrough to P8 | REQ-VSCODE-012 |
| TC-P9-001 | Chain-walk | Now, cursor confirmed | REQ-VSCODE-003 |
| TC-P9-002 | Chain-walk | LogInfo via indirect variable | REQ-VSCODE-003 |
| TC-P9-003 | Chain-walk | Negative: plain member access, no `(` | REQ-VSCODE-003 |
| TC-P9-004 | Chain-walk | Negative: cursor on whitespace | REQ-VSCODE-003 |
| TC-P9-005 | Chain-walk | Negative: free function call | REQ-VSCODE-003 |
| TC-P9-101 | Chain-walk | Simple pointer: ptTime->ptApi->Now | REQ-VSCODE-003 |
| TC-P9-102 | Chain-walk | Simple pointer: ptBroker->ptApi->Publish | REQ-VSCODE-003 |
| TC-P9-103 | Chain-walk | Array subscript: ptAppList[i]->ptApi->OnStart | REQ-VSCODE-003 |
| TC-P9-104 | Chain-walk | Array subscript (var index): ptAppList[iCounter]->ptApi->OnProcess | REQ-VSCODE-003 |
| TC-P9-105 | Chain-walk | Chained member: ptEngineApp->ptBroker->ptApi->RegisterSubscriber | REQ-VSCODE-003 |
| TC-P9-201 | Chain-walk | Dot-accessed: tReturn.ptApi->Copy | REQ-VSCODE-003 |
| TC-P9-202 | Chain-walk | Nested dot: tPtrResult.tOk.ptApi->Copy | REQ-VSCODE-003 |
| TC-P9-203 | Chain-walk | Dot-accessed: tItem.ptApi->Copy | REQ-VSCODE-003 |
| TC-P9-301 | Chain-walk | Indirect: ptLoggerApi->LogInfo | REQ-VSCODE-003 |
| TC-P9-302 | Chain-walk | Indirect: ptCmdPipeApi->Dequeue | REQ-VSCODE-003 |
| TC-P9-303 | Chain-walk | Indirect: ptApi->GetAt (generic name) | REQ-VSCODE-003 |
| TC-P9-401 | Chain-walk | Non-ptApi member: ptHeapPointerApi->Compare | REQ-VSCODE-003 |
| TC-P9-402 | Chain-walk | Non-ptApi member: ptHeapPointerApi->Swap | REQ-VSCODE-003 |
| TC-P9-501 | Chain-walk | Macro: JUNO_MODULE_GET_API()->Insert (synthetic) | REQ-VSCODE-003 |
| TC-P9-601 | Chain-walk | Fallback: unknown receiver, unique field | REQ-VSCODE-003 |
| TC-P9-602 | Chain-walk | Fallback: unknown receiver, shared field name | REQ-VSCODE-003, REQ-VSCODE-006 |
| TC-P9-603 | Chain-walk | Fallback: unknown field → not found | REQ-VSCODE-003, REQ-VSCODE-004 |
| TC-P10-001 | visitFailureHandlerAssignment | Direct: ptHeap->_pfcnFailureHandler | REQ-VSCODE-016 |
| TC-P10-002 | visitFailureHandlerAssignment | Macro form: JUNO_FAILURE_HANDLER (alternation token) | REQ-VSCODE-016 |
| TC-P10-003 | visitFailureHandlerAssignment | Negative: ptApi assignment | REQ-VSCODE-016 |
| TC-P11-001 | visitFunctionDefinition | Match: static LogInfo (varied return type) | REQ-VSCODE-003 |
| TC-P11-002 | visitFunctionDefinition | Match: non-static JunoDs_Heap_Insert | REQ-VSCODE-003 |
| TC-P11-003 | visitFunctionDefinition | Allman braces: static inline Verify (grammar handles natively) | REQ-VSCODE-003 |
| TC-P11-004 | visitFunctionDefinition | Match: void FailureHandler | REQ-VSCODE-003 |
| TC-P11-005 | visitFunctionDefinition | Match: static result-type Now | REQ-VSCODE-003 |
| TC-P11-006 | visitFunctionDefinition | No match: forward declaration (`;`) | REQ-VSCODE-003 |
| TC-P11-007 | visitFunctionDefinition | No match: header prototype (`;`) | REQ-VSCODE-003 |
| TC-RES-001 | E2E Resolution | LogInfo via indirect API pointer (Cat. 3) | REQ-VSCODE-003, REQ-VSCODE-005 |
| TC-RES-002 | E2E Resolution | OnStart via array subscript (Cat. 1 + QuickPick) | REQ-VSCODE-003, REQ-VSCODE-006 |
| TC-RES-003 | E2E Resolution | RegisterSubscriber via chained member (Cat. 1) | REQ-VSCODE-003, REQ-VSCODE-005 |
| TC-RES-004 | E2E Resolution | Compare via non-ptApi member (Cat. 4) | REQ-VSCODE-003, REQ-VSCODE-005 |
| TC-RES-005 | E2E Resolution | Copy via dot-accessed ptApi (Cat. 2) | REQ-VSCODE-003, REQ-VSCODE-005 |
| TC-RES-006 | E2E Resolution | No implementation found — error path | REQ-VSCODE-004, REQ-VSCODE-013 |
| TC-RES-007 | E2E Resolution | Multiple implementations — QuickPick | REQ-VSCODE-006 |
| TC-VSC-001 | VSCode | Extension activates on C file open | REQ-VSCODE-001 |
| TC-VSC-002 | VSCode | DefinitionProvider registered for `c` and `cpp` | REQ-VSCODE-001, REQ-VSCODE-007 |
| TC-VSC-003 | VSCode | F12 on vtable call site returns location | REQ-VSCODE-002, REQ-VSCODE-007 |
| TC-VSC-004 | VSCode | F12 on non-vtable call returns undefined (fallthrough) | REQ-VSCODE-007 |
| TC-VSC-005 | VSCode | `libjuno.goToImplementation` command registered | REQ-VSCODE-001 |
| TC-VSC-006 | VSCode | `libjuno.reindexWorkspace` clears cache and rebuilds | REQ-VSCODE-001 |
| TC-VSC-007 | VSCode | `provideDefinition` returns undefined without throw on unresolvable site | REQ-VSCODE-002, REQ-VSCODE-004 |
| TC-ERR-001 | Error UX | Status bar shown on failure (non-intrusive) | REQ-VSCODE-004, REQ-VSCODE-013 |
| TC-ERR-002 | Error UX | Status bar auto-clears after 5 seconds | REQ-VSCODE-013 |
| TC-ERR-003 | Error UX | Information message on repeated failure within 10 s | REQ-VSCODE-013 |
| TC-ERR-004 | Error UX | No modal dialog on resolution failure | REQ-VSCODE-013 |
| TC-ERR-005 | Error UX | Error message includes specific API type and field name | REQ-VSCODE-004, REQ-VSCODE-013 |
| TC-ERR-006 | Error UX | MCP returns proper error object, not HTTP error code | REQ-VSCODE-004 |
| TC-MCP-001 | MCP | Server starts on extension activation | REQ-VSCODE-017 |
| TC-MCP-002 | MCP | `resolve_vtable_call` registered with correct schema | REQ-VSCODE-018 |
| TC-MCP-003 | MCP | `resolve_failure_handler` registered with correct schema | REQ-VSCODE-019 |
| TC-MCP-004 | MCP | `resolve_vtable_call` valid input → found: true + locations | REQ-VSCODE-018 |
| TC-MCP-005 | MCP | `resolve_vtable_call` no-match → found: false + error | REQ-VSCODE-018 |
| TC-MCP-006 | MCP | `resolve_failure_handler` valid input → found: true | REQ-VSCODE-019 |
| TC-MCP-007 | MCP | `resolve_failure_handler` no handler → found: false | REQ-VSCODE-019 |
| TC-MCP-008 | MCP | `.libjuno/mcp.json` written on activation | REQ-VSCODE-017 |
| TC-MCP-009 | MCP | Server binds to 127.0.0.1 only | REQ-VSCODE-017, REQ-VSCODE-020 |
| TC-MCP-010 | MCP | Error responses use `isError: true`, not HTTP error codes | REQ-VSCODE-017 |
| TC-MCP-011 | MCP | MCP tools work headless (no VSCode UI) | REQ-VSCODE-017, REQ-VSCODE-020 |
| TC-MCP-012 | MCP | No platform-specific AI SDK imports in MCP source | REQ-VSCODE-020 |
| TC-CACHE-001 | Cache | Cache file created on first index | REQ-VSCODE-001 |
| TC-CACHE-002 | Cache | Cache loaded on activation when valid | REQ-VSCODE-001 |
| TC-CACHE-003 | Cache | Stale file triggers re-index of that file only | REQ-VSCODE-001 |
| TC-CACHE-004 | Cache | New file indexed and added to cache | REQ-VSCODE-001 |
| TC-CACHE-005 | Cache | Deleted file removed from cache and index | REQ-VSCODE-001 |
| TC-CACHE-006 | Cache | Version mismatch triggers full re-index | REQ-VSCODE-001 |
| TC-CACHE-007 | Cache | FileSystemWatcher triggers re-index on file change | REQ-VSCODE-001 |
| TC-CACHE-008 | Cache | Debounced write: rapid saves → single cache write | REQ-VSCODE-001 |
| TC-CACHE-009 | Cache | Cache write is atomic (temp file + rename) | REQ-VSCODE-001 |
| TC-FH-001 | Failure Handler | _pfcnFailureHandler assignment resolves to handler | REQ-VSCODE-016 |
| TC-FH-002 | Failure Handler | JUNO_FAILURE_HANDLER macro form resolves (gap fix) | REQ-VSCODE-016 |
| TC-FH-003 | Failure Handler | Multiple assignments → QuickPick shown | REQ-VSCODE-016 |
| TC-FH-004 | Failure Handler | No assignment → error shown | REQ-VSCODE-016 |
| TC-QP-001 | QuickPick | Items show function name as label | REQ-VSCODE-006 |
| TC-QP-002 | QuickPick | Items show file:line as description | REQ-VSCODE-006 |
| TC-QP-003 | QuickPick | Items show workspace-relative path as detail | REQ-VSCODE-006 |
| TC-QP-004 | QuickPick | Selecting item navigates to correct file and line | REQ-VSCODE-006 |
| TC-QP-005 | QuickPick | Cancelling QuickPick does not navigate | REQ-VSCODE-006 |
| TC-SYS-001 | System E2E | LogInfo via indirect API pointer — real file, chain-walk Cat. 3 | REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005 |
| TC-SYS-002 | System E2E | RegisterSubscriber via chained member access — real file, chain-walk Cat. 1 | REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005 |
| TC-SYS-003 | System E2E | Now via standard ptApi access — real file, chain-walk Cat. 1 | REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005 |
| TC-SYS-004 | System E2E | Publish via standard ptApi access — real file, chain-walk Cat. 1 | REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005 |
| TC-SYS-005 | System E2E | OnStart via array subscript — multiple locations, real file | REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-006 |
| TC-SYS-006 | System E2E | SubtractTime via ptApi — non-static library function, real file | REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005 |
| TC-SYS-007 | System E2E | Dequeue via dot-chained ptApi — positional vtable, real file | REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005, REQ-VSCODE-012 |
| TC-SYS-008 | System E2E | JUNO_FAILURE_HANDLER macro form — navigates to FailureHandler in main.c | REQ-VSCODE-016 |
| TC-SYS-009 | System E2E | OnProcess via array subscript with variable index — multiple locations, real file | REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-006 |
| TC-SYS-010 | System E2E | LogInfo cross-file resolution from system_manager_app.c — real file | REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005 |
| TC-SYS-011 | System E2E | Extension activation and index build with real example project | REQ-VSCODE-001 |
| TC-SYS-012 | System E2E | Non-vtable call passes through to default C language provider | REQ-VSCODE-007 |
| TC-SYS-013 | System E2E | Designated initializer indexing — engine_app.c vtable, all 3 fields via MCP | REQ-VSCODE-010 |
| TC-SYS-014 | System E2E | Positional initializer indexing — main.c log API, all 4 fields via MCP | REQ-VSCODE-012 |

---

## Section 20: Lexer Token Boundary Tests

These test cases verify that the Chevrotain lexer correctly tokenizes LibJuno macro identifiers
using `\b` word-boundary patterns and `longer_alt: Identifier` priority ordering. They confirm
that macro token names are never over-eagerly consumed when they appear as prefixes of longer
identifiers (e.g., `JUNO_MODULE_ROOT_T` must not be split into `JunoModuleRoot` + `_T`), and
that alternation tokens such as `JunoFailureHandler` match both the macro form
(`JUNO_FAILURE_HANDLER`) and the underlying member name (`_pfcnFailureHandler`).
All inputs are *(synthetic)*.

---

### Test Case ID: TC-LEX-001
**Scenario:** `JUNO_MODULE_ROOT` followed by `(` tokenizes as `JunoModuleRoot`, not `Identifier`
**Input text:** *(synthetic)*
```c
struct FOO_TAG JUNO_MODULE_ROOT(FOO_API_T, JUNO_MODULE_EMPTY);
```
**Expected result:**
- Lexer produces token sequence: `Struct`, `Identifier(FOO_TAG)`, `JunoModuleRoot`, `LParen`, `Identifier(FOO_API_T)`, `Comma`, `JunoModuleEmpty`, `RParen`, `Semicolon`
- Token type for `JUNO_MODULE_ROOT` is `JunoModuleRoot`, NOT `Identifier`
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-LEX-002
**Scenario:** `JUNO_MODULE_ROOT_T` (a plain type name) tokenizes as `Identifier`, NOT as `JunoModuleRoot` + leftover
**Input text:** *(synthetic)*
```c
JUNO_MODULE_ROOT_T *ptModule;
```
**Expected result:**
- Lexer produces: `Identifier(JUNO_MODULE_ROOT_T)`, `Star`, `Identifier(ptModule)`, `Semicolon`
- Token type for `JUNO_MODULE_ROOT_T` is `Identifier`, NOT `JunoModuleRoot`
- Validates that `/JUNO_MODULE_ROOT\b/` does NOT match `JUNO_MODULE_ROOT_T` because `_` is a word character
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-LEX-003
**Scenario:** Standalone `JUNO_MODULE` macro tokenizes as `JunoModule` when followed by `(`
**Input text:** *(synthetic)*
```c
JUNO_MODULE(FOO_T, FOO_ROOT_T, JUNO_MODULE_EMPTY);
```
**Expected result:**
- Lexer produces: `JunoModule`, `LParen`, `Identifier(FOO_T)`, `Comma`, `Identifier(FOO_ROOT_T)`, `Comma`, `JunoModuleEmpty`, `RParen`, `Semicolon`
- Token type for `JUNO_MODULE` is `JunoModule`, NOT `Identifier`
- Validates that `/JUNO_MODULE\b/` matches only the standalone form and not any longer prefix
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-LEX-004
**Scenario:** `JUNO_FAILURE_HANDLER` macro form tokenized as `JunoFailureHandler`
**Input text:** *(synthetic)*
```c
ptApp->tRoot.JUNO_FAILURE_HANDLER = MyHandler;
```
**Expected result:**
- Lexer produces: `Identifier(ptApp)`, `ArrowOp`, `Identifier(tRoot)`, `Dot`, `JunoFailureHandler`, `Assign`, `Identifier(MyHandler)`, `Semicolon`
- Token type for `JUNO_FAILURE_HANDLER` is `JunoFailureHandler`
**Requirement:** REQ-VSCODE-003, REQ-VSCODE-016

---

### Test Case ID: TC-LEX-005
**Scenario:** `_pfcnFailureHandler` alternation — underlying member name tokenized as `JunoFailureHandler`
**Input text:** *(synthetic)*
```c
ptApp->tRoot._pfcnFailureHandler = MyHandler;
```
**Expected result:**
- Lexer produces the same token sequence as TC-LEX-004: `Identifier(ptApp)`, `ArrowOp`, `Identifier(tRoot)`, `Dot`, `JunoFailureHandler`, `Assign`, `Identifier(MyHandler)`, `Semicolon`
- Token type for `_pfcnFailureHandler` is `JunoFailureHandler` (same token type via alternation)
**Requirement:** REQ-VSCODE-003, REQ-VSCODE-016

---

### Test Case ID: TC-LEX-006
**Scenario:** `JUNO_FAILURE_HANDLER_T` (the type, not the member) tokenizes as `Identifier`
**Input text:** *(synthetic)*
```c
JUNO_FAILURE_HANDLER_T pfcnHandler;
```
**Expected result:**
- Lexer produces: `Identifier(JUNO_FAILURE_HANDLER_T)`, `Identifier(pfcnHandler)`, `Semicolon`
- Token type for `JUNO_FAILURE_HANDLER_T` is `Identifier`, NOT `JunoFailureHandler`
- Validates that `\b` boundary prevents the type name from being consumed as the member-access token
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-LEX-007
**Scenario:** `JUNO_MODULE_SUPER` tokenized as `JunoModuleSuper` in member position
**Input text:** *(synthetic)*
```c
ptDerived->JUNO_MODULE_SUPER.ptApi->Foo(ptDerived);
```
**Expected result:**
- Lexer produces: `Identifier(ptDerived)`, `ArrowOp`, `JunoModuleSuper`, `Dot`, `Identifier(ptApi)`, `ArrowOp`, `Identifier(Foo)`, `LParen`, `Identifier(ptDerived)`, `RParen`, `Semicolon`
- Token type for `JUNO_MODULE_SUPER` is `JunoModuleSuper`, NOT `Identifier`
**Requirement:** REQ-VSCODE-003, REQ-VSCODE-009

---

### Test Case ID: TC-LEX-008
**Scenario:** `_pvFailureUserData` alternation — underlying member tokenized as `JunoFailureUserData`
**Input text:** *(synthetic)*
```c
ptApp->tRoot._pvFailureUserData = pvUserData;
```
**Expected result:**
- Lexer produces: `Identifier(ptApp)`, `ArrowOp`, `Identifier(tRoot)`, `Dot`, `JunoFailureUserData`, `Assign`, `Identifier(pvUserData)`, `Semicolon`
- Token type for `_pvFailureUserData` is `JunoFailureUserData`
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-LEX-009
**Scenario:** C keyword `struct` with `longer_alt` — `structure` tokenizes as `Identifier`, not `Struct` + suffix
**Input text:** *(synthetic)*
```c
int structure = 42;
```
**Expected result:**
- Lexer produces: `Int`, `Identifier(structure)`, `Assign`, `IntegerLiteral(42)`, `Semicolon`
- Token type for `structure` is `Identifier`, NOT `Struct`
- Validates `longer_alt: Identifier` prevents keyword over-consumption
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-LEX-010
**Scenario:** `HashDirective` captures entire preprocessor line as single token
**Input text:** *(synthetic)*
```c
#define JUNO_MODULE_ROOT(API_T, ...) struct { ... }
```
**Expected result:**
- Lexer produces a single `HashDirective` token whose image contains the full line text starting with `#`
- No additional tokens are emitted for this line
**Requirement:** REQ-VSCODE-003

---

## Section 21: Parser Error Recovery Tests

These test cases verify that the Chevrotain parser's error recovery (`{ recoveryEnabled: true }`
on `externalDeclaration`) limits parse failures to the local declaration where they occur.
When the parser cannot match a construct, it skips tokens forward to the next `;` or `}` at
brace depth 0, then resumes. A single malformed or unsupported construct (e.g., inline assembly,
a GCC attribute, or a struct missing its closing brace) must not prevent subsequent well-formed
declarations from being parsed and indexed.
All inputs are *(synthetic)*.

---

### Test Case ID: TC-ERR-PARSE-001
**Scenario:** Inline assembly block triggers error recovery; next function is still parsed
**Input text:** *(synthetic)*
```c
void BadFunc(void) {
    __asm__ volatile("nop");
}

static JUNO_STATUS_T GoodFunc(JUNO_FOO_ROOT_T *ptFoo) {
    return ptFoo->ptApi->Bar(ptFoo);
}
```
**Expected result:**
- `GoodFunc` appears in `functionDefinitions` of the parsed result
- The `__asm__` construct triggers error recovery but does not prevent subsequent parsing
- No uncaught exception is thrown
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-ERR-PARSE-002
**Scenario:** Malformed struct missing closing brace; subsequent struct still parsed
**Input text:** *(synthetic)*
```c
struct BROKEN_TAG {
    int x;
/* missing closing brace */

struct JUNO_FOO_ROOT_TAG JUNO_MODULE_ROOT(JUNO_FOO_API_T, JUNO_MODULE_EMPTY);
```
**Expected result:**
- `moduleRoots` contains `{ rootType: "JUNO_FOO_ROOT_T", apiType: "JUNO_FOO_API_T" }`
- Error recovery on the malformed struct does not prevent the subsequent `JUNO_MODULE_ROOT` declaration from being indexed
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-ERR-PARSE-003
**Scenario:** Unrecognized GCC attribute triggers error recovery; subsequent declaration still parsed
**Input text:** *(synthetic)*
```c
__attribute__((packed)) struct WeirdStruct { int x; };

static const JUNO_FOO_API_T tFooApi = { .Bar = MyBar };
```
**Expected result:**
- `tFooApi` vtable assignment is still extracted despite the `__attribute__` construct above it
- Error recovery skips the `__attribute__` declaration without aborting the parse
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-ERR-PARSE-004
**Scenario:** Empty file produces empty `ParsedFile` with no errors
**Input text:** *(synthetic)*
```c

```
**Expected result:**
- All arrays in `ParsedFile` (`moduleRoots`, `derivations`, `functionDefinitions`, etc.) are empty
- No parse errors are thrown
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-ERR-PARSE-005
**Scenario:** File containing only comments and whitespace produces empty `ParsedFile`
**Input text:** *(synthetic)*
```c
/* This file is intentionally blank */
// Nothing here
```
**Expected result:**
- All arrays in `ParsedFile` are empty
- No parse errors are thrown
**Requirement:** REQ-VSCODE-003

---

## Section 22: LocalTypeInfo Population Tests

These test cases verify the `visitLocalDeclaration` and `visitFunctionParameters` visitor
methods that populate `LocalTypeInfo`. Each function scope must accumulate its own
`TypeInfo` records (parameter list and local variable map) independently, with no
cross-contamination between sibling functions. `TypeInfo` fields — `typeName`, `isPointer`,
`isConst`, `isArray` — must be set correctly from the declaration syntax.
All inputs are *(synthetic)*.

---

### Test Case ID: TC-LOCAL-001
**Scenario:** Simple pointer parameter extraction
**Input text:** *(synthetic)*
```c
static JUNO_STATUS_T Init(JUNO_FOO_ROOT_T *ptFoo) {
    return JUNO_STATUS_SUCCESS;
}
```
**Expected result:**
- `functionParameters["Init"]` contains one entry: `{ name: "ptFoo", typeName: "JUNO_FOO_ROOT_T", isPointer: true, isConst: false, isArray: false }`
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-LOCAL-002
**Scenario:** Const pointer parameter extraction
**Input text:** *(synthetic)*
```c
static void Display(const JUNO_LOG_API_T *ptLogApi) {}
```
**Expected result:**
- `functionParameters["Display"]` contains one entry: `{ name: "ptLogApi", typeName: "JUNO_LOG_API_T", isPointer: true, isConst: true, isArray: false }`
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-LOCAL-003
**Scenario:** Local variable declaration inside function body
**Input text:** *(synthetic)*
```c
static JUNO_STATUS_T Process(JUNO_FOO_ROOT_T *ptFoo) {
    const JUNO_LOG_API_T *ptLogApi = ptFoo->ptLogger->ptApi;
    ptLogApi->LogInfo(ptFoo->ptLogger, "Processing");
    return JUNO_STATUS_SUCCESS;
}
```
**Expected result:**
- `localVariables["Process"]["ptLogApi"]` = `{ name: "ptLogApi", typeName: "JUNO_LOG_API_T", isPointer: true, isConst: true, isArray: false }`
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-LOCAL-004
**Scenario:** Array local variable declaration
**Input text:** *(synthetic)*
```c
static void RunAll(void) {
    static JUNO_APP_ROOT_T *ptAppList[4];
    ptAppList[0]->ptApi->OnStart(ptAppList[0]);
}
```
**Expected result:**
- `localVariables["RunAll"]["ptAppList"]` = `{ name: "ptAppList", typeName: "JUNO_APP_ROOT_T", isPointer: true, isConst: false, isArray: true }`
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-LOCAL-005
**Scenario:** Multiple parameters extraction
**Input text:** *(synthetic)*
```c
static JUNO_STATUS_T Copy(JUNO_POINTER_T tDest, JUNO_POINTER_T tSrc) {
    return JUNO_STATUS_SUCCESS;
}
```
**Expected result:**
- `functionParameters["Copy"]` contains 2 entries:
  - `{ name: "tDest", typeName: "JUNO_POINTER_T", isPointer: false, isConst: false, isArray: false }`
  - `{ name: "tSrc", typeName: "JUNO_POINTER_T", isPointer: false, isConst: false, isArray: false }`
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-LOCAL-006
**Scenario:** Stack-allocated (non-pointer) local variable
**Input text:** *(synthetic)*
```c
static JUNO_STATUS_T Transform(JUNO_FOO_ROOT_T *ptFoo) {
    JUNO_POINTER_T tResult;
    tResult.ptApi->Copy(tResult, tInput);
    return JUNO_STATUS_SUCCESS;
}
```
**Expected result:**
- `localVariables["Transform"]["tResult"]` = `{ name: "tResult", typeName: "JUNO_POINTER_T", isPointer: false, isConst: false, isArray: false }`
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-LOCAL-007
**Scenario:** Visitor correctly scopes variables to their enclosing function — no cross-contamination
**Input text:** *(synthetic)*
```c
static void FuncA(JUNO_FOO_ROOT_T *ptFoo) {
    int iCounterA = 0;
}
static void FuncB(JUNO_BAR_ROOT_T *ptBar) {
    int iCounterB = 0;
}
```
**Expected result:**
- `localVariables["FuncA"]` contains `iCounterA` but NOT `iCounterB`
- `localVariables["FuncB"]` contains `iCounterB` but NOT `iCounterA`
- `functionParameters["FuncA"]` contains `ptFoo` with `typeName: "JUNO_FOO_ROOT_T"`
- `functionParameters["FuncB"]` contains `ptBar` with `typeName: "JUNO_BAR_ROOT_T"`
**Requirement:** REQ-VSCODE-003

---

## Section 23: Preprocessor Directive Handling Tests

These test cases verify the `visitPreprocessorDirective` visitor method and the `HashDirective`
token. The lexer must consume each `#...` preprocessor line as a single `HashDirective` token,
and the parser must route that token through `visitPreprocessorDirective` so it never interferes
with adjacent C declarations. Well-formed LibJuno declarations that follow or are surrounded by
preprocessor directives (include guards, `#include`, `#define`, `#ifdef`/`#endif`) must be
indexed normally.
All inputs are *(synthetic)*.

---

### Test Case ID: TC-PP-001
**Scenario:** `#include` directive captured as single `HashDirective` token; following struct parsed normally
**Input text:** *(synthetic)*
```c
#include "juno/module.h"
struct JUNO_FOO_ROOT_TAG JUNO_MODULE_ROOT(JUNO_FOO_API_T, JUNO_MODULE_EMPTY);
```
**Expected result:**
- The `#include` line is consumed as a single `HashDirective` token
- `moduleRoots` contains `{ rootType: "JUNO_FOO_ROOT_T", apiType: "JUNO_FOO_API_T" }`
- No parse error is produced for the `#include` line
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-PP-002
**Scenario:** `#ifdef` / `#endif` include guards do not prevent parsing of guarded content
**Input text:** *(synthetic)*
```c
#ifndef JUNO_FOO_API_H
#define JUNO_FOO_API_H

struct JUNO_FOO_ROOT_TAG JUNO_MODULE_ROOT(JUNO_FOO_API_T, JUNO_MODULE_EMPTY);

#endif
```
**Expected result:**
- `moduleRoots` contains `{ rootType: "JUNO_FOO_ROOT_T", apiType: "JUNO_FOO_API_T" }`
- The `#ifndef`, `#define`, and `#endif` lines are each consumed as `HashDirective` tokens and do not interfere with the struct declaration
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-PP-003
**Scenario:** `#define` of a known LibJuno macro is consumed by the visitor without error
**Input text:** *(synthetic)*
```c
#define JUNO_MODULE_ROOT(API_T, ...) struct { ... }
```
**Expected result:**
- The entire `#define` line is consumed as a single `HashDirective` token
- `visitPreprocessorDirective` records this `#define` for `JUNO_MODULE_ROOT` without error
- No lexer or parser error is produced
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-PP-004
**Scenario:** Multiple `#include` directives followed by code — all parsed correctly
**Input text:** *(synthetic)*
```c
#include <stdint.h>
#include "juno/module.h"
#include "juno/status.h"

struct JUNO_BAR_IMPL_TAG JUNO_MODULE_DERIVE(JUNO_BAR_ROOT_T, JUNO_MODULE_EMPTY);
```
**Expected result:**
- `derivations` contains `{ derivedType: "JUNO_BAR_IMPL_T", rootType: "JUNO_BAR_ROOT_T" }`
- All three `#include` lines are consumed as individual `HashDirective` tokens
- No parse error is produced
**Requirement:** REQ-VSCODE-003

---

## Section 24: Standalone Macro Declaration Tests

These test cases verify the `visitJunoStandaloneDeclaration` visitor method, which handles
forward-declaration macros that appear as top-level statements — `JUNO_MODULE_DECLARE`,
`JUNO_MODULE_ROOT_DECLARE`, `JUNO_MODULE_DERIVE_DECLARE`, and `JUNO_MODULE_RESULT`. These
constructs expand to typedef or union declarations and must be matched by the `junoStandaloneDeclaration`
grammar rule without triggering error recovery. When multiple standalone declarations are
interspersed with struct definitions, all must be parsed cleanly in a single pass.
All inputs are *(synthetic)*.

---

### Test Case ID: TC-DECL-001
**Scenario:** `JUNO_MODULE_DECLARE` parsed as standalone declaration without error
**Input text:** *(synthetic)*
```c
JUNO_MODULE_DECLARE(JUNO_FOO_T);
```
**Expected result:**
- Parser matches the `junoStandaloneDeclaration` rule
- No parse error and no error recovery triggered
- The visitor records the forward-declared module union type `JUNO_FOO_T`
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-DECL-002
**Scenario:** `JUNO_MODULE_ROOT_DECLARE` parsed as standalone declaration without error
**Input text:** *(synthetic)*
```c
JUNO_MODULE_ROOT_DECLARE(JUNO_FOO_ROOT_T);
```
**Expected result:**
- Parser matches the `junoStandaloneDeclaration` rule
- No parse error and no error recovery triggered
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-DECL-003
**Scenario:** `JUNO_MODULE_DERIVE_DECLARE` parsed as standalone declaration without error
**Input text:** *(synthetic)*
```c
JUNO_MODULE_DERIVE_DECLARE(JUNO_FOO_IMPL_T);
```
**Expected result:**
- Parser matches the `junoStandaloneDeclaration` rule
- No parse error and no error recovery triggered
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-DECL-004
**Scenario:** `JUNO_MODULE_RESULT` with two arguments parsed as standalone declaration
**Input text:** *(synthetic)*
```c
JUNO_MODULE_RESULT(JUNO_FOO_RESULT_T, JUNO_FOO_ROOT_T);
```
**Expected result:**
- Parser matches the `junoStandaloneDeclaration` rule
- No parse error and no error recovery triggered
- The visitor records result type name `JUNO_FOO_RESULT_T` and payload type `JUNO_FOO_ROOT_T`
**Requirement:** REQ-VSCODE-003

---

### Test Case ID: TC-DECL-005
**Scenario:** Multiple standalone declarations interspersed with struct definitions — all parsed cleanly
**Input text:** *(synthetic)*
```c
JUNO_MODULE_ROOT_DECLARE(JUNO_FOO_ROOT_T);
JUNO_MODULE_DERIVE_DECLARE(JUNO_FOO_IMPL_T);

struct JUNO_FOO_ROOT_TAG JUNO_MODULE_ROOT(JUNO_FOO_API_T, JUNO_MODULE_EMPTY);

JUNO_MODULE_DECLARE(JUNO_FOO_T);
```
**Expected result:**
- `moduleRoots` contains `{ rootType: "JUNO_FOO_ROOT_T", apiType: "JUNO_FOO_API_T" }`
- All three standalone declarations (`JUNO_MODULE_ROOT_DECLARE`, `JUNO_MODULE_DERIVE_DECLARE`, `JUNO_MODULE_DECLARE`) are parsed without error
- No error recovery is triggered for any declaration
**Requirement:** REQ-VSCODE-003

---

## Requirements Coverage Matrix

| Requirement ID | Title | Test Case(s) |
|----------------|-------|--------------|
| REQ-VSCODE-001 | VSCode Extension | TC-VSC-001, TC-VSC-002, TC-VSC-005, TC-VSC-006, TC-CACHE-001 through TC-CACHE-009, TC-SYS-011 |
| REQ-VSCODE-002 | Vtable-Aware Go to Definition | TC-VSC-003, TC-VSC-007, TC-SYS-001 through TC-SYS-007, TC-SYS-009, TC-SYS-010, TC-SYS-012 |
| REQ-VSCODE-003 | LibJuno API Pattern Recognition | TC-P9-001 through TC-P9-603, TC-P11-001 through TC-P11-007, TC-RES-001 through TC-RES-007, TC-SYS-001 through TC-SYS-007, TC-SYS-009, TC-SYS-010, TC-SYS-013, TC-SYS-014, TC-LEX-001 through TC-LEX-010, TC-ERR-PARSE-001 through TC-ERR-PARSE-005, TC-LOCAL-001 through TC-LOCAL-007, TC-PP-001 through TC-PP-004, TC-DECL-001 through TC-DECL-005 |
| REQ-VSCODE-004 | Graceful Error on Missing Implementation | TC-P9-603, TC-RES-006, TC-VSC-007, TC-ERR-001, TC-ERR-005, TC-ERR-006 |
| REQ-VSCODE-005 | Single Implementation Navigation | TC-RES-001, TC-RES-003, TC-RES-004, TC-RES-005, TC-SYS-001, TC-SYS-002, TC-SYS-003, TC-SYS-004, TC-SYS-006, TC-SYS-007, TC-SYS-010 |
| REQ-VSCODE-006 | Multiple Implementation Selection | TC-P9-602, TC-RES-002, TC-RES-007, TC-QP-001 through TC-QP-005, TC-SYS-005, TC-SYS-009 |
| REQ-VSCODE-007 | Native Go to Definition Integration | TC-VSC-002, TC-VSC-003, TC-VSC-004, TC-SYS-012 |
| REQ-VSCODE-008 | Module Root API Discovery | TC-P1-001 through TC-P1-004 |
| REQ-VSCODE-009 | Module Derivation Chain Resolution | TC-P2-001 through TC-P2-003, TC-LEX-007 |
| REQ-VSCODE-010 | Designated Initializer Recognition | TC-P6-001 through TC-P6-003, TC-SYS-013 |
| REQ-VSCODE-011 | Direct Assignment Recognition | TC-P7-001, TC-P7-002 |
| REQ-VSCODE-012 | Positional Initializer Recognition | TC-P5-001 through TC-P5-005, TC-P8-001 through TC-P8-004, TC-SYS-007, TC-SYS-014 |
| REQ-VSCODE-013 | Informative Non-Intrusive Error | TC-RES-006, TC-ERR-001 through TC-ERR-005 |
| REQ-VSCODE-014 | Trait Root API Discovery | TC-P3-001 through TC-P3-003 |
| REQ-VSCODE-015 | Trait Derivation Chain Resolution | TC-P4-001, TC-P4-002 |
| REQ-VSCODE-016 | Failure Handler Navigation | TC-P10-001 through TC-P10-003, TC-FH-001 through TC-FH-004, TC-SYS-008, TC-LEX-004, TC-LEX-005, TC-LEX-006, TC-LEX-008 |
| REQ-VSCODE-017 | AI Agent Accessibility | TC-MCP-001, TC-MCP-008, TC-MCP-009, TC-MCP-010, TC-MCP-011 |
| REQ-VSCODE-018 | AI Vtable Resolution Access | TC-MCP-002, TC-MCP-004, TC-MCP-005 |
| REQ-VSCODE-019 | AI Failure Handler Resolution Access | TC-MCP-003, TC-MCP-006, TC-MCP-007 |
| REQ-VSCODE-020 | Platform-Agnostic AI Interface | TC-MCP-009, TC-MCP-011, TC-MCP-012 |

---

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
**Requirement:** REQ-VSCODE-012
