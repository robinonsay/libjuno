> Part of: [Software Design Document](index.md) — Section 3.1.3 CST Visitor, 3.2-3.7 Components

#### 3.1.3 CST Visitor

The `IndexBuildingVisitor` extends Chevrotain's generated CST visitor base class. It walks the CST produced by the parser and populates the `ParsedFile` output record. Seven visitor methods replace the previous regex-based pattern system.

---

**1. `visitStructDefinition(ctx)`** — Replaces P1, P2, P3, P4, P5

Invoked for each `structOrUnionSpecifier` CST node. Dispatches based on which child node is present:

- **`struct TAG JUNO_MODULE_ROOT(API_T, ...)`** (ctx contains `junoModuleRootMacro`):
  - rootType = TAG with `_TAG` → `_T` suffix substitution
  - apiType = first `Identifier` argument of the macro
  - Emit `ModuleRootRecord` → add to `moduleRoots`

- **`struct TAG JUNO_MODULE_DERIVE(ROOT_T, ...)`** (ctx contains `junoModuleDeriveMacro`):
  - derivedType = TAG with `_TAG` → `_T`
  - rootType = first `Identifier` argument
  - Emit `DerivationRecord` → add to `derivationChain`

- **`struct TAG JUNO_TRAIT_ROOT(API_T, ...)`** (ctx contains `junoTraitRootMacro`):
  - Same semantics as `JUNO_MODULE_ROOT`
  - Emit `TraitRootRecord` → add to `traitRoots`

- **`struct TAG JUNO_TRAIT_DERIVE(ROOT_T, ...)`** (ctx contains `junoTraitDeriveMacro`):
  - Same semantics as `JUNO_MODULE_DERIVE`
  - Emit `DerivationRecord` → add to `derivationChain`

- **`struct TAG { ... }` where TAG ends in `_API_TAG`**:
  - apiType = TAG with `_API_TAG` → `_API_T`
  - Walk `structDeclarationList` to extract function pointer fields in document order:
    - For each `structDeclarator` whose `directDeclarator` has the form `'(' '*' Identifier ')' '(' parameterTypeList ')'`:
      - field name = the `Identifier` inside the parenthesized group
  - Emit `ApiStructRecord` → add to `apiStructFields`

- **For ALL struct bodies** (any TAG): walk member declarations and record members whose declared type ends in `_API_T`:
  - memberName → API type → add to `apiMemberRegistry`
  - This covers `ROOT_TAG`, `IMPL_TAG`, and all other struct kinds in a single pass.

- **`JUNO_MODULE_SUPER` resolution:** When the visitor encounters a `JunoModuleSuper` token as a member name in any struct body, it treats it as equivalent to the literal member `tRoot`. This is consistent with the macro definition `#define JUNO_MODULE_SUPER tRoot`. The chain-walk algorithm (§5.1, Step 4b) applies the same equivalence at query time.

---

**2. `visitVtableDeclaration(ctx)`** — Replaces P6, P7, P8

Handles three vtable assignment forms:

- **Designated initializer (P6 equivalent):** Matches a `declaration` of the form `(static)? const _API_T varName = { designation? initializer ... };`.
  - The `initializerList` contains `designation` nodes (`.Field = FuncName`).
  - For each designator: field = designator identifier, functionName = initializer identifier.
  - Emit `VtableAssignmentRecord` for each pair → add to `vtableAssignments`.

- **Positional initializer (P8 equivalent):** Same declaration shape, but `initializerList` contains no `designation` nodes (all initializers are bare expressions).
  - Look up the field order from `apiStructFields` for the declared API type.
  - If the field order is available (API struct was parsed earlier in the same file): zip field names with initializer expressions in document order → emit `VtableAssignmentRecord` for each pair.
  - If not yet available (API struct defined in another file): defer — record the positional initializer and retry after all files in the workspace have been indexed (cross-file deferred resolution, same as before but handled inline for same-file cases).

- **Direct assignment (P7 equivalent):** Matches `expressionStatement` nodes (within function bodies) of the form `identifier '.' identifier '=' identifier ';'`.
  - When the first identifier's declared type (from `LocalTypeInfo`) is a known API type → emit `VtableAssignmentRecord`.
  - This visitor method is also invoked from `visitCompoundStatement` rather than from the top level, since direct assignments appear inside function bodies.

---

**3. `visitFunctionDefinition(ctx)`** — Replaces P11

Invoked for each `functionDefinition` CST node.
- functionName = `Identifier` from the innermost `directDeclarator` in the `declarator`.
- isStatic = `true` if `declarationSpecifiers` contains a `Static` token.
- file and line = from the CST node's token position.
- Emit `FunctionDefinitionRecord` → add to `functionDefinitions`.

Because the grammar handles both K&R and Allman brace styles natively, no two-pass workaround is needed. Because `functionDefinition` is a distinct production from `declaration` (which ends with `;`), forward declarations are automatically excluded — they parse as `declaration`, not `functionDefinition`.

---

**4. `visitFailureHandlerAssignment(ctx)`** — Replaces P10a, P10b

Invoked for `expressionStatement` nodes where the `assignmentExpression` LHS is a `postfixExpression` ending with a `JunoFailureHandler` or `JunoFailureUserData` token in a member access position.

Because the lexer's alternation patterns match both the macro name (`JUNO_FAILURE_HANDLER`) and the underlying member name (`_pfcnFailureHandler`) as the same `JunoFailureHandler` token type, the visitor handles both code styles uniformly — no dual-pattern workaround is needed. Whether the source contains:
```c
ptEngineApp->tRoot.JUNO_FAILURE_HANDLER = pfcnFailureHandler;
```
or:
```c
ptEngineApp->tRoot._pfcnFailureHandler = pfcnFailureHandler;
```
the grammar sees the same `JunoFailureHandler` token in the `memberIdentifier` position.

- variableName = first token of the LHS `postfixExpression`.
- functionName = identifier on the RHS of the assignment.
- Emit `FailureHandlerRecord`; rootType is resolved at index-merge time via `LocalTypeInfo`.

---

**5. `visitLocalDeclaration(ctx)`** — NEW (no regex equivalent)

Invoked for `declaration` nodes appearing inside `compoundStatement` bodies (i.e., within function bodies).
- Extracts: variable name, declared type, pointer depth, `isConst`, `isArray` flags.
- Builds a `Map<string, TypeInfo>` for the enclosing function scope.
- Stored in `LocalTypeInfo.localVariables[functionName]`.
- Replaces the 200-line backward regex scan: type information for all local variables is available deterministically from the CST.

---

**6. `visitFunctionParameters(ctx)`** — NEW (no regex equivalent)

Invoked for each `functionDefinition` node's `parameterList`.
- Extracts: parameter name, declared type, pointer depth, `isConst` flags for each parameter.
- Stored in `LocalTypeInfo.functionParameters[functionName]`.
- Used by the query-time chain-walk resolver (§5.1) to determine the types of parameters such as `ptHeap`, `tReturn`, and `ptLoggerApi`.

---

**7. `visitPreprocessorDirective(ctx)`** — NEW

Invoked for each `preprocessorDirective` CST node.
- **`#define NAME value`:** Records the macro definition for known LibJuno macros. Opaque macros (not in the LibJuno macro token list) are ignored.
- **`#ifdef`/`#ifndef`/`#endif`:** Tracks nesting depth for future conditional compilation awareness. For the current design, branches are not selectively parsed — all branches are parsed and merged, with `#if`/`#endif` nesting tracked for informational purposes.
- **`#include "path"` / `#include <path>`:** Records the included path for future cross-file navigation support.

---

**8. `visitJunoStandaloneDeclaration(ctx)`** — NEW

Invoked for `junoStandaloneDeclaration` CST nodes.
- **`JUNO_MODULE_DECLARE(NAME_T)`:** Records a forward-declared module union type. Not directly used for navigation but prevents parse errors from macro invocations at file scope.
- **`JUNO_MODULE_ROOT_DECLARE(NAME_T)` / `JUNO_MODULE_DERIVE_DECLARE(NAME_T)`:** Same — forward-declaration bookkeeping.
- **`JUNO_MODULE_RESULT(NAME_T, OK_T)`:** Records a result type definition. The visitor notes the result type name and its payload type. This data is available for future type resolution but is not currently used by the chain-walk algorithm.



---

### 3.2 Workspace Indexer

Responsible for scanning all C and H files in the workspace, invoking the Chevrotain Parser on each, and populating the Navigation Index. Also coordinates with the Cache Manager to load, validate, and save the JSON cache.

**Responsibilities:**
- On activation: load cache; re-index files whose hash has changed; index new files.
- On `FileSystemWatcher` events: re-index the changed file, update cache.
- Provide the populated `NavigationIndex` to the Vtable Resolver, Failure Handler Resolver, and MCP Server.

**File scan scope:** All `*.c`, `*.h`, `*.cpp`, `*.hpp`, `*.hh`, and `*.cc` files in the workspace, excluding `build/`, `deps/`, and `.libjuno/` directories (configurable via extension settings).

**Indexing algorithm:**

```
FOR EACH file in workspace (*.c, *.h, *.cpp), excluding excluded dirs:
  hash = sha256(fileContent)
  IF hash == cache.fileHashes[filePath]:
    SKIP (use cached data)
  ELSE:
    parsedFile = ChevrotainParser.parse(filePath, fileContent)
    MERGE parsedFile.moduleRoots        INTO index.moduleRoots
    MERGE parsedFile.traitRoots         INTO index.traitRoots
    MERGE parsedFile.derivations        INTO index.derivationChain
    MERGE parsedFile.apiStructDefs      INTO index.apiStructFields
    MERGE parsedFile.vtableAssignments  INTO index.vtableAssignments
    MERGE parsedFile.failureHandlers    INTO index.failureHandlerAssignments
    MERGE parsedFile.functionDefs       INTO index.functionDefinitions
    MERGE parsedFile.apiMemberRegistry  INTO index.apiMemberRegistry
    STORE parsedFile.localTypeInfo      INTO index.localTypeInfo[filePath]
    cache.fileHashes[filePath] = hash
END FOR

RESOLVE positional initializers with missing field orders (cross-file deferred records):
  For any positional initializer whose API struct was defined in another file,
  now that all files have been parsed, the field order in apiStructFields is available.
  Retry zipping deferred records.
  If field order still unavailable after full workspace scan: log warning, skip.

SAVE cache to disk
```

**`reindexFile(filePath)` — Incremental Indexing Algorithm:**

Both the `onDidChange` and `onDidCreate` handlers delegate to `reindexFile()`. This procedure re-parses a single file and merges its updated records into the index:

```
reindexFile(filePath):
  fileContent = readFile(filePath)
  IF readFile fails (ENOENT or permission error):
    LOG warning to Output Channel; RETURN
  parsedFile = ChevrotainParser.parse(filePath, fileContent)
  IF parsedFile.errors.length > 0:
    LOG warning to Output Channel ("Parse errors in <filePath>: <errors>"); RETURN
  removeFileRecords(index, filePath)          // clear all prior records for this file
  deferred = []
  mergeInto(index, parsedFile, deferred)
  STORE parsedFile.localTypeInfo INTO index.localTypeInfo[filePath]
  resolveDeferred(index, deferred)
  resolveCompositionRoots(index)
  resolveInitCallers(index)
  cache.fileHashes[filePath] = sha256(fileContent)
  scheduleSave()
```

> **`removeFileRecords` contract:** Clears all index maps for the given `filePath`, including `moduleRoots`, `traitRoots`, `derivationChain`, `apiStructFields`, `vtableAssignments`, `failureHandlerAssignments`, `functionDefinitions`, `apiMemberRegistry`, and `localTypeInfo[filePath]`. Also clears `initCallIndex` records originating from `filePath`.

**File System Event Handlers:**

```
WHEN FileSystemWatcher.onDidCreate fires:
  IF file path is in an excluded directory (build/, deps/, .libjuno/): RETURN
  reindexFile(filePath)

WHEN FileSystemWatcher.onDidChange fires:
  reindexFile(filePath)

WHEN FileSystemWatcher.onDidDelete fires:
  removeFileRecords(index, filePath)
  delete cache.fileHashes[filePath]
  scheduleSave()
```

> **Note on excluded-directory guard:** The `onDidCreate` handler checks the excluded-directory list because newly created files may appear in excluded paths (e.g., a build system writing to `build/`). The `onDidChange` handler does not need this guard because the `FileSystemWatcher` glob (`**/*.{c,h,cpp,hpp,hh,cc}`) excludes those directories at the platform level — only files that were already indexed can trigger `onDidChange`.

> **`onDidCreate` edge case:** If a file is created and immediately deleted before `readFile` executes, `reindexFile` catches the `ENOENT` error, logs a warning, and returns without modifying the index or cache.

> **Single-pass note:** Function definitions are extracted by `visitFunctionDefinition` in the same parse pass as vtable assignments. The previous "second pass for Pattern P11" is eliminated. The `apiMemberRegistry` is also populated inline by `visitStructDefinition` for all struct bodies encountered during the same parse pass.

For each file, the parser also extracts local declarations and function parameters into a per-file type map (`LocalTypeInfo`). This data is stored in the cache under `localTypeInfo` and used at query time for expression type resolution in the chain-walk algorithm (§5.1).

### 3.3 Vtable Resolver

Given a cursor position (file, line, column) in a C source file, resolves the API call under the cursor to one or more concrete function locations.

**Input:** `{ file: string, line: number, column: number }`  
**Output:** `VtableResolutionResult`

```typescript
interface VtableResolutionResult {
  found:     boolean;
  locations: ConcreteLocation[];
  errorMsg?: string;
}

interface ConcreteLocation {
  functionName:    string;
  file:            string;
  line:            number;      // Line of the function **definition**.
                               // Resolved in the same parse pass as vtable assignments (see Section 3.2).
  assignmentFile?: string;     // File where the vtable assignment occurs (composition root)
  assignmentLine?: number;     // Line of the vtable assignment (composition root)
}
```

Resolution algorithm is detailed in Section 5.1.

### 3.4 Failure Handler Resolver

Given a cursor position on a line containing `->_pfcnFailureHandler` or a failure handler call site, resolves the concrete handler function.

**Input:** `{ file: string, line: number, column: number }`  
**Output:** `VtableResolutionResult` (same type, reused)

Resolution algorithm is detailed in Section 5.3.

### 3.5 VSCode Integration Layer

Provides the user-facing features: DefinitionProvider registration, QuickPick for multiple results, commands, status bar messages, and the vtable resolution trace WebviewPanel (VtableTraceProvider).

**Registered providers and commands:**

| Item | Type | Trigger |
|------|------|---------|
| `JunoDefinitionProvider` | `vscode.DefinitionProvider` | F12, Ctrl+Click on C/C++ files |
| `VtableTraceProvider` | WebviewPanel | `libjuno.showVtableTrace` command |
| `libjuno.goToImplementation` | Command | Command Palette |
| `libjuno.reindexWorkspace` | Command | Command Palette |
| `libjuno.showVtableTrace` | Command | Command Palette; Ctrl+Shift+T; Right-click context menu |

### 3.6 MCP Server

An HTTP server embedded in the extension process, implementing the Model Context Protocol. Exposes two tools to AI agent platforms. Described in detail in Section 7.

### 3.7 Cache Manager

Handles reading and writing `.libjuno/navigation-cache.json`. Detects staleness by comparing file hashes. Described in detail in Section 9.

---
