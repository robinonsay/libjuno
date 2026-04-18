> Part of: [Test Case Specification](index.md) — Section 12: End-to-End Resolution Tests

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
