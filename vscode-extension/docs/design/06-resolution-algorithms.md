> Part of: [Software Design Document](index.md) — Section 5: Resolution Algorithms

// @{"design": ["REQ-VSCODE-002", "REQ-VSCODE-004", "REQ-VSCODE-005", "REQ-VSCODE-006", "REQ-VSCODE-009", "REQ-VSCODE-015", "REQ-VSCODE-016", "REQ-VSCODE-022", "REQ-VSCODE-023", "REQ-VSCODE-024", "REQ-VSCODE-025", "REQ-VSCODE-026"]}
## 5. Resolution Algorithms

### 5.1 Vtable Call Resolution

Triggered when the user places the cursor on a vtable call site and activates Go to Definition.

**Input:** cursor position `(file, line, column)`

**Algorithm:**

```
STEP 1 — Re-parse the current file (or retrieve cached CST)
  Parse the file using the Chevrotain parser to obtain the CST.
  In practice, the CST may be cached from the most recent index run. If the file
  has been modified since the last index, re-parse it.

STEP 2 — Locate the CST node at the cursor position
  Walk the CST to find the deepest node spanning (line, column).
  The target node should be an Identifier within a postfixExpression.
  If the node is not part of a postfixExpression with a function call suffix:
    RETURN { found: false, errorMsg: "Cursor is not on a LibJuno API call site." }

STEP 3 — Extract the postfixExpression chain
  Walk up from the cursor node to the enclosing postfixExpression.
  The postfixExpression has the form:
    primaryExpr (suffix)*
  where each suffix is: [expr], (args), .id, ->id, ++, or --

  Identify which suffix contains the cursor. It should be a '->identifier' suffix
  where 'identifier' is the field name being called, followed by a '(args)' suffix.

  fieldName = the identifier in the '->identifier' suffix under the cursor.

STEP 4 — Resolve the receiver type by walking the chain
  Starting from the primaryExpression (leftmost element), resolve the type at each step:

  4a. primaryExpression:
    - If Identifier: look up in localTypeInfo for the containing file and function
      (local variables first, then function parameters). The type comes from
      the TypeInfo entry produced by visitLocalDeclaration / visitFunctionParameters.
    - If junoModuleGetApiMacro (JUNO_MODULE_GET_API(expr, TYPE)):
        rootType = TYPE (explicit in the macro); skip to Step 5 (derivation chain).
    - If '(' expression ')': recursively resolve the inner expression type.

  4b. For each suffix in order (left to right up to the cursor suffix):
    - '->member' or '.member':
        The member token may be an Identifier, a JunoModuleSuper token, a
        JunoFailureHandler token, or a JunoFailureUserData token (these
        are all valid in the memberIdentifier grammar position).

        If the member is JunoModuleSuper:
          currentType = the root type embedded as the first member of
          currentType's struct. This is semantically equivalent to
          accessing the literal member 'tRoot', consistent with:
            #define JUNO_MODULE_SUPER tRoot

        If the member is JunoFailureHandler / JunoFailureUserData:
          This is a failure handler member access (handled by §5.3).

        Otherwise (Identifier):
          '->member': currentType must be a pointer-to-struct.
          Look up 'member' in the struct's definition from the index
          (apiMemberRegistry or struct member data from visitStructDefinition).
          currentType = declared type of 'member'.
          '.member': same as '->' but for non-pointer struct access.

    - '[expr]': array subscript — strip one pointer/array level from currentType.
    - '(args)': function call — result type is the return type of the function.
        (For vtable dispatch this is typically the final suffix and return type
        resolution is not needed.)

  4c. When we reach the '->fieldName' suffix under the cursor:
    - currentType at this point should be an '_API_T *' (API struct pointer).
    - apiType = currentType (stripped of pointer).
    - If apiType IS a registered API type: GOTO STEP 6 (look up implementations).
    - If currentType is NOT a registered API type:
        The suffix immediately before '->fieldName' is likely '->ptApi' or '.ptApi'.
        In that case the type before 'ptApi' was a root/derived type.
        rootType = currentType (the type resolved just before the ptApi access).
        GOTO STEP 5 (derivation chain resolution).

STEP 5 — Fallback: apiMemberRegistry and field-name search
  If Step 4 could not determine apiType (type resolution failed at some intermediate step):

  5a. Check apiMemberRegistry for the member immediately before '->fieldName':
    memberName = the identifier in the suffix immediately before '->fieldName'
    IF memberName is in index.apiMemberRegistry:
      apiType = index.apiMemberRegistry[memberName]
      GOTO STEP 6.

  5b. Field-name search across all API types (uniform fallback):
    candidates = all apiTypes in index.apiStructFields that contain fieldName.
    IF candidates.length == 0:
      RETURN { found: false, errorMsg: "No API type contains field '${fieldName}'." }
    IF candidates.length == 1:
      apiType = candidates[0]; GOTO STEP 6.
    IF candidates.length > 1:
      allLocations = []
      FOR EACH candidate IN candidates:
        locs = index.vtableAssignments.get(candidate)?.get(fieldName) ?? []
        allLocations.push(...locs)
      RETURN { found: allLocations.length > 0, locations: allLocations }

STEP 5 (from 4c) — Resolve derivation chain to the root type
  current = rootType
  WHILE index.derivationChain.has(current):
    current = index.derivationChain.get(current)
  rootType = current
  // rootType is now the topmost root type with an API mapping

  apiType = index.moduleRoots.get(rootType) ?? index.traitRoots.get(rootType)
  IF apiType is undefined:
    RETURN { found: false, errorMsg: "No API type registered for root '${rootType}'." }

STEP 6 — Look up concrete implementations
  fieldMap = index.vtableAssignments.get(apiType)
  IF fieldMap is undefined:
    RETURN { found: false, errorMsg: "No vtable assignments found for '${apiType}'." }
  locations = fieldMap.get(fieldName)
  IF locations is undefined or empty:
    RETURN { found: false, errorMsg: "No implementation found for '${apiType}::${fieldName}'." }

STEP 7 — Return results
  RETURN { found: true, locations: locations }
```

**Key improvements over the regex-based approach:**

- **No backward regex scans:** Local variable types come from `LocalTypeInfo`, populated deterministically by the parser visitor. The 200-line backward scan heuristic is eliminated.
- **No strategy ordering:** The chain-walk algorithm handles ALL call patterns uniformly. `ptTime->ptApi->Now(...)`, `ptAppList[i]->ptApi->OnStart(...)`, `ptEngineApp->ptBroker->ptApi->RegisterSubscriber(...)`, `tReturn.ptApi->Copy(...)`, and `ptLoggerApi->LogInfo(...)` are all different shapes of `postfixExpression` chains resolved by the same algorithm.
- **No Allman brace workarounds:** The grammar handles both K&R and Allman brace styles natively.
- **No macro form gaps:** `JUNO_FAILURE_HANDLER` is a first-class lexer token, so `var->member.JUNO_FAILURE_HANDLER = func;` is parsed correctly without dual patterns.

**Call pattern coverage — mapping the 20 patterns from prior catalog to the chain-walk algorithm:**

| Old Category | Example | Chain-Walk Path |
|-------------|---------|----------------|
| Category 1 — Simple `->ptApi->Field` | `ptTime->ptApi->Now(ptTime)` | Identifier → `->ptApi` (root type) → `->Now` (field): rootType from localTypeInfo[ptTime]; walk derivation chain to apiType |
| Category 1 — Array subscript | `ptAppList[i]->ptApi->OnStart(...)` | Identifier → `[i]` (strip array) → `->ptApi` → `->OnStart` |
| Category 1 — Array subscript counter | `ptAppList[iCounter]->ptApi->OnProcess(...)` | Same as above |
| Category 1 — Chained member | `ptEngineApp->ptBroker->ptApi->RegisterSubscriber(...)` | Identifier → `->ptBroker` (member type) → `->ptApi` → `->RegisterSubscriber` |
| Category 1 — Chained member | `ptSystemManagerApp->ptTime->ptApi->Now(...)` | Identifier → `->ptTime` → `->ptApi` → `->Now` |
| Category 1 — Chained member | `ptStack->ptStackArray->ptApi->GetAt(...)` | Identifier → `->ptStackArray` → `->ptApi` → `->GetAt` |
| Category 1 — Simple pointer | `ptRecvQueue->ptApi->Enqueue(...)` | Identifier → `->ptApi` → `->Enqueue` |
| Category 1 — Simple pointer | `ptBuffer->ptApi->SetAt(...)` | Identifier → `->ptApi` → `->SetAt` |
| Category 2 — Nested dot | `tPtrResult.tOk.ptApi->Copy(...)` | Identifier → `.tOk` → `.ptApi` → `->Copy` |
| Category 2 — Stack value | `tReturn.ptApi->Copy(tReturn, tResult.tOk)` | Identifier → `.ptApi` → `->Copy` |
| Category 2 — Stack value | `tItem.ptApi->Copy(...)` | Identifier → `.ptApi` → `->Copy` |
| Category 2 — Nested dot | `tResult.tOk.ptApi->Reset(...)` | Identifier → `.tOk` → `.ptApi` → `->Reset` |
| Category 2 — Stack value | `tIndexPointer.ptApi->Copy(...)` | Identifier → `.ptApi` → `->Copy` |
| Category 2 — Stack value | `tIndexPointer.ptApi->Reset(...)` | Identifier → `.ptApi` → `->Reset` |
| Category 2 — Stack value | `tMemory.ptApi->Reset(...)` | Identifier → `.ptApi` → `->Reset` |
| Category 3 — Direct API ptr | `ptLoggerApi->LogInfo(...)` | Identifier → `->LogInfo`: localTypeInfo[ptLoggerApi] = `JUNO_LOG_API_T *`; apiType direct from type |
| Category 3 — Direct API ptr | `ptCmdPipeApi->Dequeue(...)` | Identifier → `->Dequeue`: localTypeInfo resolves to `JUNO_DS_QUEUE_API_T *` |
| Category 3 — Direct API ptr | `ptArrayApi->GetAt(...)` | Identifier → `->GetAt`: localTypeInfo resolves to `JUNO_DS_ARRAY_API_T *` |
| Category 4 — Named API member | `ptHeap->ptHeapPointerApi->Compare(...)` | Identifier → `->ptHeapPointerApi` (apiMemberRegistry lookup) → `->Compare` |
| Category 5 — Macro-based | `JUNO_MODULE_GET_API(ptModule, ROOT_T)->Field(...)` | `junoModuleGetApiMacro` primary expression → rootType = ROOT_T explicit |

All 20 patterns are handled by the single chain-walk algorithm without strategy enumeration.

### 5.2 Multiple Result Dispatch (VSCode Integration)

After the Vtable Resolver returns:

```
IF result.found == false:
  Show status bar message (see Section 8)
  RETURN (no navigation)

IF result.locations.length == 1:
  // REQ-VSCODE-005: direct navigation
  Navigate to result.locations[0] via vscode.window.showTextDocument

IF result.locations.length > 1:
  // REQ-VSCODE-006: selection list
  Show vscode.window.showQuickPick with items:
    label: functionName
    description: file:line
    detail: relative file path
  ON selection: Navigate to selected location
```

### 5.3 Failure Handler Resolution

Triggered when the cursor is on a line containing `_pfcnFailureHandler` or `JUNO_FAILURE_HANDLER`.

**Algorithm:**

```
STEP 1 — Re-parse the current file (or retrieve cached CST)
  Same as Vtable Resolution Step 1.

STEP 2 — Locate the CST node at the cursor position
  Walk the CST to find the deepest node spanning (line, column).
  The target node should be a JunoFailureHandler token appearing as the
  final member access in an assignment expression. Because the lexer's
  alternation pattern (/JUNO_FAILURE_HANDLER\b|_pfcnFailureHandler\b/)
  matches both the macro name and the underlying member name as the same
  token type, the algorithm handles both code styles uniformly.

STEP 3 — Extract the assignment
  Walk up from the cursor node to the enclosing assignmentExpression.
  The LHS is a postfixExpression ending with a JunoFailureHandler token
  in a memberIdentifier position (via '->' or '.' access). The chained
  form '->member.JUNO_FAILURE_HANDLER' and the direct member form
  '->_pfcnFailureHandler' both produce the same token type.
  The RHS is the function name identifier.

  variableName = first identifier of the LHS postfixExpression.
  functionName = RHS identifier.

STEP 4 — Resolve the root type of variableName
  Use the chain-walk algorithm (same as Vtable Resolution Step 4):
  look up variableName in LocalTypeInfo (local variables, then function
  parameters). Walk the derivation chain if the resolved type is a
  derived type.
  rootType = resolved root type.

STEP 5 — Look up concrete handler(s)
  locations = index.failureHandlerAssignments.get(rootType)
  IF empty:
    RETURN { found: false, errorMsg: "No failure handler registered for '${rootType}'." }

STEP 6 — Dispatch (same single/multiple logic as Section 5.2)
```

// @{"design": ["REQ-VSCODE-022", "REQ-VSCODE-023", "REQ-VSCODE-024", "REQ-VSCODE-025", "REQ-VSCODE-026"]}
### 5.3.1 FAIL Macro Call Site Resolution

This subsection extends the `FailureHandlerResolver` to handle the four FAIL/ASSERT macro call sites. Recognition and resolution happen **at query time** inside the resolver — no new lexer tokens or index record types are required, because these macros parse as ordinary `Identifier '(' argumentExpressionList ')'` expressions whose arguments are already present as tokens in the line text.

```
STEP 0 — Check for FAIL macro call site
  Inspect the line text at the cursor position for one of the following patterns:
    /\bJUNO_FAIL\s*\(/
    /\bJUNO_FAIL_MODULE\s*\(/
    /\bJUNO_FAIL_ROOT\s*\(/
    /\bJUNO_ASSERT_EXISTS_MODULE\s*\(/
  IF a pattern matches:
    Record the macro name and proceed with FAIL macro resolution (Steps 1–2 below).
  ELSE:
    Fall through to the existing §5.3 algorithm (cursor on _pfcnFailureHandler member).

STEP 1 — Extract macro arguments from line text
  Scan the line text starting from the opening '(' of the macro call.
  Use balanced-parenthesis tracking to handle nested expressions within arguments:
    depth = 0
    FOR each character from the opening '(':
      IF '(': depth++
      IF ')': depth--; IF depth == 0: end of argument list
      IF ',' AND depth == 1: argument boundary
  Collect each argument as a trimmed substring.

  Extract the 2nd argument (index 1, 0-indexed) for all four macro forms:
    JUNO_FAIL(tStatus, pfcnHandler, pvUserData, msg)         → arg[1] = pfcnHandler
    JUNO_FAIL_MODULE(tStatus, ptMod, msg)                   → arg[1] = ptMod
    JUNO_FAIL_ROOT(tStatus, ptRootMod, msg)                 → arg[1] = ptRootMod
    JUNO_ASSERT_EXISTS_MODULE(ptr, ptMod, str)              → arg[1] = ptMod

  Strip surrounding whitespace and cast expressions (e.g. '(TYPE *)ptr' → 'ptr').
  extractedArg = the bare identifier name.

STEP 2 — Resolve based on macro type

  IF macro is JUNO_FAIL:
    handlerName = extractedArg  // a function pointer variable or function name
    locations = index.functionDefinitions.get(handlerName)
    IF locations is defined and non-empty:
      RETURN { found: true, locations: locations.map(fd => ({
        functionName: fd.functionName, file: fd.file, line: fd.line
      }))}
    ELSE:
      RETURN { found: false, errorMsg: "No definition found for failure handler '${handlerName}'." }

  IF macro is JUNO_FAIL_MODULE or JUNO_ASSERT_EXISTS_MODULE:
    // The second argument is a pointer to a derived or root module struct.
    modulePtrName = extractedArg
    Resolve modulePtrName's declared type using localTypeInfo
    (local variables first, then function parameters — same lookup as §5.3 Step 4).
    Walk the derivation chain to the root type
    (same WHILE loop as §5.1 Step 5):
      current = resolvedType
      WHILE index.derivationChain.has(current):
        current = index.derivationChain.get(current)
      rootType = current
    locations = index.failureHandlerAssignments.get(rootType)
    IF empty:
      RETURN { found: false, errorMsg: "No failure handler registered for '${rootType}'." }
    ELSE:
      RETURN { found: true, locations: locations }

  IF macro is JUNO_FAIL_ROOT:
    // The second argument is already a root (not derived) module pointer.
    rootPtrName = extractedArg
    Resolve rootPtrName's declared type using localTypeInfo (no derivation chain walk).
    rootType = resolvedType
    locations = index.failureHandlerAssignments.get(rootType)
    IF empty:
      RETURN { found: false, errorMsg: "No failure handler registered for '${rootType}'." }
    ELSE:
      RETURN { found: true, locations: locations }
```

**Design rationale:** Handling FAIL macro recognition at query time (in the resolver) rather than at parse/index time (in the CST visitor) avoids adding new record types and index structures. The macro arguments are available directly in the line text; the type information needed for module pointer resolution is already populated in `localTypeInfo` by `visitLocalDeclaration` and `visitFunctionParameters`. This approach keeps the indexer and data model unchanged.

---
