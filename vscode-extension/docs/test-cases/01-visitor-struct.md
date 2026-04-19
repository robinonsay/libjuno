> Part of: [Test Case Specification](index.md) — Sections 1-4: visitStructDefinition

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
