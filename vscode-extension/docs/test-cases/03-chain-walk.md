> Part of: [Test Case Specification](index.md) — Section 9: Chain-Walk Call Site Resolution

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
