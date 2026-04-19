> Part of: [Test Case Specification](index.md) — Sections 20-21: Lexer Token Boundary and Parser Error Recovery Tests

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
