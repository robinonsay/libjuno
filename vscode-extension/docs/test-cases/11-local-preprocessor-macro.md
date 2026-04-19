> Part of: [Test Case Specification](index.md) — Sections 22-24: LocalTypeInfo, Preprocessor, and Macro Tests

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
