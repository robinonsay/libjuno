> Part of: [Software Design Document](index.md) — Sections 10-11: RTM and Vtable Trace View

## 10. Requirements Traceability Matrix

| Requirement ID | Title | Design Element(s) |
|----------------|-------|-------------------|
| REQ-VSCODE-001 | VSCode Extension | Entire extension; `activate()` entry point; DefinitionProvider registration |
| REQ-VSCODE-002 | Vtable-Aware Go to Definition | `JunoDefinitionProvider`; Vtable Resolver (Section 3.3, 5.1) |
| REQ-VSCODE-003 | LibJuno API Pattern Recognition | C Parser (Section 3.1); Lexer token definitions (Section 3.1.1); Grammar productions including `postfixExpression` and `structOrUnionSpecifier` (Section 3.1.2); CST Visitor methods `visitStructDefinition`, `visitVtableDeclaration`, `visitFunctionDefinition`, `visitFailureHandlerAssignment` (Section 3.1.3); chain-walk resolution algorithm (Section 5.1); `apiMemberRegistry` in index and cache |
| REQ-VSCODE-004 | Graceful Error on Missing Implementation | Error Handling Design (Section 8); `VtableResolutionResult.found == false` path |
| REQ-VSCODE-005 | Single Implementation Navigation | Multiple Result Dispatch — single branch (Section 5.2) |
| REQ-VSCODE-006 | Multiple Implementation Selection | Multiple Result Dispatch — QuickPick branch (Section 5.2, 6.3) |
| REQ-VSCODE-007 | Native Go to Definition Integration | `vscode.languages.registerDefinitionProvider` (Section 6.1) |
| REQ-VSCODE-008 | Module Root API Discovery | `visitStructDefinition` — `JUNO_MODULE_ROOT` branch (Section 3.1.3); `junoModuleRootMacro` grammar production (Section 3.1.2); `moduleRoots` in index and cache |
| REQ-VSCODE-009 | Module Derivation Chain Resolution | `visitStructDefinition` — `JUNO_MODULE_DERIVE` branch (Section 3.1.3); `junoModuleDeriveMacro` grammar production (Section 3.1.2); `derivationChain` in index and cache; Step 5 (derivation chain) of resolution algorithm |
| REQ-VSCODE-010 | Designated Initializer Recognition | `visitVtableDeclaration` — designated initializer branch (Section 3.1.3); `designation` grammar production (Section 3.1.2); `vtableAssignments` population |
| REQ-VSCODE-011 | Direct Assignment Recognition | `visitVtableDeclaration` — direct assignment branch (Section 3.1.3); `expressionStatement` grammar production (Section 3.1.2); `vtableAssignments` population |
| REQ-VSCODE-012 | Positional Initializer Recognition | `visitStructDefinition` — API struct field extraction (Section 3.1.3); `visitVtableDeclaration` — positional initializer branch (Section 3.1.3); `apiStructFields` in index and cache; positional zip algorithm |
| REQ-VSCODE-013 | Informative Non-Intrusive Error | Status bar message; optional `showInformationMessage` (Section 8.1) |
| REQ-VSCODE-014 | Trait Root API Discovery | `visitStructDefinition` — `JUNO_TRAIT_ROOT` branch (Section 3.1.3); `junoTraitRootMacro` grammar production (Section 3.1.2); `traitRoots` in index and cache |
| REQ-VSCODE-015 | Trait Derivation Chain Resolution | `visitStructDefinition` — `JUNO_TRAIT_DERIVE` branch (Section 3.1.3); `junoTraitDeriveMacro` grammar production (Section 3.1.2); `derivationChain` shared with module derivations; Step 5 (derivation chain) of resolution algorithm |
| REQ-VSCODE-016 | Failure Handler Navigation | Failure Handler Resolver (Section 3.4, 5.3); `visitFailureHandlerAssignment` (Section 3.1.3); `JunoFailureHandler` lexer token (Section 3.1.1); `failureHandlerAssignments` in index and cache |
| REQ-VSCODE-017 | AI Agent Accessibility | MCP Server (Section 3.6, 7); `.libjuno/mcp.json` discovery file |
| REQ-VSCODE-018 | AI Vtable Resolution Access | MCP tool `resolve_vtable_call` (Section 7.2) |
| REQ-VSCODE-019 | AI Failure Handler Resolution Access | MCP tool `resolve_failure_handler` (Section 7.3) |
| REQ-VSCODE-020 | Platform-Agnostic AI Interface | MCP protocol selection rationale (Section 7.1); no platform-specific AI API used |
| REQ-VSCODE-021 | C and C++ File Type Support | File scan scope (Section 3.2); FileSystemWatcher glob (Section 9.3); configurable extension settings |
| REQ-VSCODE-022 | FAIL Macro Failure Handler Navigation | `FailureHandlerResolver` §5.3.1; `JunoDefinitionProvider` §6.2 |
| REQ-VSCODE-023 | JUNO_FAIL Direct Handler Resolution | §5.3.1 Step 2 — JUNO_FAIL branch; `functionDefinitions` index lookup |
| REQ-VSCODE-024 | JUNO_FAIL_MODULE Handler Resolution | §5.3.1 Step 2 — JUNO_FAIL_MODULE branch; derivation chain walk + `failureHandlerAssignments` lookup |
| REQ-VSCODE-025 | JUNO_FAIL_ROOT Handler Resolution | §5.3.1 Step 2 — JUNO_FAIL_ROOT branch; `failureHandlerAssignments` direct lookup (no derivation chain walk) |
| REQ-VSCODE-026 | JUNO_ASSERT_EXISTS_MODULE Handler Resolution | §5.3.1 Step 2 — same as JUNO_FAIL_MODULE branch; derivation chain walk + `failureHandlerAssignments` lookup |
| REQ-VSCODE-027 | Vtable Resolution Trace View | VtableTraceProvider (§11); WebviewPanel (§11.3); TraceNode/VtableTrace interfaces (§11.2) |
| REQ-VSCODE-028 | Trace View Activation via Keyboard | Keybinding: Ctrl+Shift+T (§11.5); `when` clause guard for C/C++ files |
| REQ-VSCODE-029 | Trace View Activation via Command Palette | Command: `libjuno.showVtableTrace` (§11.5) |
| REQ-VSCODE-030 | Trace View Call Site Node | TraceNode type='call-site' (§11.2 Step 2); WebviewPanel call-site div (§11.3) |
| REQ-VSCODE-031 | Trace View Composition Root Node | ConcreteLocation.assignmentFile/.assignmentLine (§4.1); TraceNode type='composition-root' (§11.2 Step 3) |
| REQ-VSCODE-032 | Trace View Implementation Node | FunctionDefinitionRecord.signature (§4.1); TraceNode type='implementation' (§11.2 Step 4) |

---

// @{"design": ["REQ-VSCODE-027", "REQ-VSCODE-028", "REQ-VSCODE-029", "REQ-VSCODE-030", "REQ-VSCODE-031", "REQ-VSCODE-032"]}
## 11. Vtable Resolution Trace View Design

### 11.1 Overview

The vtable resolution trace view provides a visual tree showing the full resolution chain from an API call site, through the composition root where the vtable was initialized, to the concrete implementation function. This satisfies REQ-VSCODE-027.

The trace view complements the existing Go to Definition feature (§5.1): Go to Definition navigates directly to the implementation, while the trace view surfaces the intermediate wiring steps — the composition root caller and, when available, the Init function body where the vtable pointer is wired — making the full dependency injection chain visible. This is especially useful for debugging DI configuration issues and understanding the wiring of large LibJuno-based systems.

### 11.2 Component: VtableTraceProvider

A new component added to the VSCode Integration Layer (§3.5). Responsibility: collect the full up-to-4-node resolution trace and render it in a `vscode.WebviewPanel`.

**TypeScript interfaces:**

```typescript
interface TraceNode {
  type:   'call-site' | 'composition-root' | 'init-impl' | 'implementation';
  label:  string;   // e.g., "ptCmdPipeApi->Dequeue(...)"
  file:   string;   // workspace-relative path
  line:   number;
  detail: string;   // additional context line
}

interface VtableTrace {
  callSite:        TraceNode;
  compositionRoot: TraceNode;
  initImpl?:       TraceNode;  // present when compRootFile is resolved
  implementation:  TraceNode;
}
```

**Data Collection Algorithm:**

```
STEP 1 — Resolve the vtable call using VtableResolver
  result = vtableResolver.resolve(file, line, column)
  IF result.found == false:
    Show error via StatusBarHelper (same as §8.1)
    RETURN

STEP 2 — Build the call site node (REQ-VSCODE-030)
  callSite = {
    type:   'call-site',
    label:  extractCallExpression(lineText),
    file:   currentFile,
    line:   cursorLine,
    detail: lineText.trim()
  }

STEP 3 — Build the composition root node (REQ-VSCODE-031, REQ-VSCODE-036, REQ-VSCODE-037)
  // When compRootFile is set, use it as the true composition root (caller of Init,
  // e.g. main.c). Otherwise fall back to initCallFile ?? assignmentFile as before.
  location = result.locations[selectedIndex or 0]
  compRootFile = location.compRootFile ?? location.initCallFile ?? location.assignmentFile ?? 'unknown'
  compRootLine = location.compRootLine ?? location.initCallLine ?? location.assignmentLine ?? 0
  compositionRoot = {
    type:   'composition-root',
    label:  location.functionName,
    file:   compRootFile,
    line:   compRootLine,
    detail: location.functionName
  }

STEP 3b — Build the initialization implementation node (REQ-VSCODE-037)
  // Only present when compRootFile is resolved AND initCallFile is known.
  // This node points to where the vtable pointer is wired inside the Init function body.
  IF location.compRootFile AND location.initCallFile:
    initImpl = {
      type:   'init-impl',
      label:  location.functionName,
      file:   location.initCallFile,
      line:   location.initCallLine ?? 0,
      detail: location.functionName
    }
  ELSE:
    initImpl = undefined

STEP 4 — Build the implementation node (REQ-VSCODE-032)
  // Use FunctionDefinitionRecord.signature (from §4.1) if available
  implementation = {
    type:   'implementation',
    label:  location.functionName,
    file:   location.file,
    line:   location.line,
    detail: location.signature ?? location.functionName
  }

STEP 5 — Display the WebviewPanel
  Show a WebviewPanel with HTML rendering of up to 4 nodes in tree layout (see §11.3)
```

### 11.3 WebviewPanel Layout

The panel is opened via `vscode.window.createWebviewPanel` with `enableScripts: true`. It uses a self-contained HTML template with inline CSS and a nonce-based inline script — no external resources.

The tree layout uses CSS border and padding to create a visual connection between nodes:

The tree layout renders up to 4 nodes. The `init-impl` node is only emitted when `compRootFile` is resolved on the `ConcreteLocation`.

```html
<div class="trace-tree">
  <div class="trace-node call-site">
    <span class="node-icon">📍</span>
    <span class="node-label">Call Site</span>
    <div class="node-detail">
      <a href="#" data-file="..." data-line="...">engine_app.c:223</a>
      <code>ptCmdPipeApi-&gt;Dequeue(...)</code>
    </div>
  </div>
  <div class="trace-connector">│</div>
  <div class="trace-node composition-root">
    <span class="node-icon">🔗</span>
    <span class="node-label">Composition Root</span>
    <div class="node-detail">
      <a href="#" data-file="..." data-line="...">main.c:12</a>
      <code>JunoSb_BrokerInit(&amp;tBroker, &amp;gtCmdPipeApi)</code>
    </div>
  </div>
  <!-- Optional: only present when compRootFile is resolved -->
  <div class="trace-connector">│</div>
  <div class="trace-node init-impl">
    <span class="node-icon">⚙</span>
    <span class="node-label">Initialization Implementation</span>
    <div class="node-detail">
      <a href="#" data-file="..." data-line="...">juno_sb_broker.c:45</a>
      <code>ptRoot-&gt;ptApi = ptApi</code>
    </div>
  </div>
  <div class="trace-connector">│</div>
  <div class="trace-node implementation">
    <span class="node-icon">⚡</span>
    <span class="node-label">Implementation</span>
    <div class="node-detail">
      <a href="#" data-file="..." data-line="...">juno_buff_queue.c:112</a>
      <code>JUNO_STATUS_T JunoDs_BuffQueue_Dequeue(...)</code>
    </div>
  </div>
</div>
```

File link clicks are communicated back to the extension host via `postMessage`. The extension host handles each click by calling `vscode.window.showTextDocument` to navigate to the referenced file and line.

### 11.4 Multiple Results Handling

When `result.locations.length > 1`, the WebviewPanel shows all results. Each location becomes a collapsible section with its own composition root → implementation subtree. The call site node is shared at the top.

### 11.5 Command Registration (REQ-VSCODE-028, REQ-VSCODE-029)

The following entries are added to `package.json` under `contributes`:

```json
{
  "commands": [
    {
      "command": "libjuno.showVtableTrace",
      "title": "LibJuno: Show Vtable Resolution Trace"
    }
  ],
  "keybindings": [
    {
      "command": "libjuno.showVtableTrace",
      "key": "ctrl+shift+t",
      "when": "editorTextFocus && resourceLangId =~ /^c/"
    }
  ],
  "menus": {
    "editor/context": [
      {
        "command": "libjuno.showVtableTrace",
        "when": "resourceLangId =~ /^c/",
        "group": "navigation"
      }
    ]
  }
}
```

**Activation gesture summary:**

| Gesture | Details |
|---------|---------|
| Command Palette | `libjuno.showVtableTrace` — "LibJuno: Show Vtable Resolution Trace" |
| Keyboard shortcut | `Ctrl+Shift+T` with `when: editorTextFocus && resourceLangId =~ /^c/` |
| Right-click context menu | "LibJuno: Show Vtable Resolution Trace" — group: navigation |

> **Note:** `Ctrl+Shift+Click` is a built-in VSCode gesture (multi-cursor) and is NOT used for trace view activation.

### 11.6 Security

The WebviewPanel uses `enableScripts: true` to handle file link clicks via `postMessage`. Security measures:

- All file paths and code text are **HTML-escaped** before insertion into the panel HTML to prevent XSS.
- The Content Security Policy restricts script sources to `nonce`-based inline scripts only:
  ```
  Content-Security-Policy: default-src 'none'; script-src 'nonce-${nonce}'; style-src 'unsafe-inline';
  ```
- No external resources (fonts, images, CDN scripts) are loaded.

---

### 11.8 Composition Root Detection — Robustness Proposals

#### 11.8.1 Current State and Known Defects

The current implementation locates composition roots in two sequential passes after full
workspace indexing:

**`resolveCompositionRoots()`** (REQ-VSCODE-036) — scans each indexed source file
line-by-line with a regex matching `&apiVarName`. It captures every match per variable per
file and stores results in `initCallIndex`. Only the first match per API variable name is
stamped onto `ConcreteLocation.initCallFile` / `initCallLine`.

**`resolveInitCallers()`** (REQ-VSCODE-037) — once the Init-function call site is known,
scans all `.c`/`.cpp` files with a regex of the form `\bInitFnName\s*\(`. It stops at the
first non-definition match and stores it in `ConcreteLocation.compRootFile` /
`compRootLine`.

**Known defects:**

1. Both regex passes match inside C-style block comments (`/* &gtMyApi */`), line comments
   (`// &gtMyApi`), and string literals (`printf("&gtMyApi")`), producing false positives
   that point to the wrong file and line.
2. `resolveInitCallers()` stops at the first match; in multi-translation-unit projects
   there can be multiple callers, and which file is "first" depends on the
   non-deterministic iteration order of `fileHashes`.
3. `resolveCompositionRoots()` similarly stamps only the first match per API variable onto
   `ConcreteLocation`, discarding all other call sites.
4. The post-parse full-file text scan is redundant: the Chevrotain lexer and visitor
   already process every token in every function body; a second raw-text scan adds latency
   and duplicates work.

---

#### 11.8.2 Option A — Visitor-based Call Site Tracking (Parser-native)

**Summary.** Extend the CST visitor to detect `&varName` unary-address-of expressions
inside function call argument lists during the parse pass. Record these as `InitCallRecord`
entries in `ParsedFile.initCallSites`. For Init-caller discovery, extend the visitor to
record every function invocation by name, building a `callGraph` in the index. Both pieces
of data then become pure index lookups — no post-parse text scanning is needed.

**Grammar nodes used.** The existing grammar already produces `postfixExpression` nodes
with call suffixes containing an `argumentExpressionList`. The `unaryExpression` production
handles the unary `&` operator. No grammar changes are required — only new visitor code
walking these already-existing nodes.

> **⚠ Implementation prerequisite:** The Chevrotain CST node shapes for
> `argumentExpressionList` and `unaryExpression` call-suffix children have NOT been
> empirically verified. Per team lessons-learned, CST key names must be confirmed via
> `Object.keys(node.children)` in a diagnostic test before writing visitor code.
> The pseudocode below uses conceptual names that may differ from actual Chevrotain output.
> A worker implementing Option A must run this diagnostic first and update the pseudocode
> accordingly.

**New visitor method pseudocode.**

Called from `walkExpressionStatement` whenever the postfix expression has a call suffix
with a non-empty argument list. For each argument the method checks for a leading `&`:

```
FUNCTION visitCallArguments(postfixNode, currentFn):
  args = getChildren(postfixNode, "argumentExpressionList")
  FOR EACH argExpr IN args:
    unary = drillToUnaryExpr(argExpr)
    IF unary has UnaryAmpersand token:
      innerIdent = extractPrimaryIdent(unary.operand)
      IF innerIdent IN knownApiVarNames:
        EMIT InitCallRecord {
          apiVarName: innerIdent,
          file:       this.filePath,
          line:       unary.location.startLine
        }
    calleeIdent = extractCalleeName(postfixNode)
    IF calleeIdent is non-empty:
      EMIT FunctionInvocationRecord {
        calleeName: calleeIdent,
        callerFn:   currentFn,
        file:       this.filePath,
        line:       postfixNode.location.startLine
      }
```

**New data structures.**

```typescript
// New record type (added to types.ts):
interface FunctionInvocationRecord {
  calleeName: string;   // e.g. "JunoDs_Heap_Init"
  callerFn:   string;   // enclosing function, e.g. "App_Init"
  file:       string;
  line:       number;
}

// New field on ParsedFile:
functionInvocations: FunctionInvocationRecord[];

// New field on NavigationIndex:
callGraph: Map<string, Array<{ callerFn: string; file: string; line: number }>>;
// calleeName → list of call sites
```

**Changes to resolution methods.**

Both methods become pure index lookups:

```
resolveCompositionRoots():
  // initCallSites already populated during parse — merged into initCallIndex
  FOR EACH loc IN all ConcreteLocations:
    sites = index.initCallIndex.get(loc.apiVarName) ?? []
    IF sites.length > 0:
      loc.initCallFile = sites[0].file
      loc.initCallLine = sites[0].line

resolveInitCallers():
  FOR EACH loc IN all ConcreteLocations:
    initFnName = findEnclosingFunction(index, loc.initCallFile, loc.initCallLine)
    IF NOT initFnName OR initFnName == 'main': CONTINUE
    callers = index.callGraph.get(initFnName) ?? []
    nonDef  = callers.filter(c => !isDefinitionLine(c, index))
    IF nonDef.length > 0:
      loc.compRootFile = nonDef[0].file
      loc.compRootLine = nonDef[0].line
```

**Tradeoff table.**

| Axis | Rating |
|------|--------|
| Accuracy | High — token-level; comments and strings are stripped by the lexer before the CST is built |
| Implementation effort | High — new visitor methods, new CST navigation for `argumentExpressionList` / `unaryExpression`, new `FunctionInvocationRecord` type, new `callGraph` index field, cache serialization |
| Index size impact | Medium — `callGraph` can be large (one entry per call site); `functionInvocations` adds to `ParsedFile` |
| Handles multiple callers | Yes — all callers collected naturally |

**Scope estimate: M (one sprint)**

The visitor has no existing code walking argument lists. The `unaryExpression` CST node
shape must be verified empirically before writing visitor code (per the lesson:
`Object.keys(node.children)` in a diagnostic test). The `callGraph` also requires new
cache serialization and invalidation logic.

---

#### 11.8.3 Option B — Pre-scan Comment Stripper (Minimal Change)

**Summary.** Add a helper that strips `/* ... */` block comments, `//` line comments, and
`"..."` / `'...'` string literals from a source line before applying the existing regex.
Apply it in both `resolveCompositionRoots()` and `resolveInitCallers()`. Also change
`resolveInitCallers()` to collect ALL matches rather than stopping at the first. Zero
grammar or CST changes are required.

**Stripper algorithm.**

The function carries an `inBlockComment` boolean across line boundaries:

```
FUNCTION stripNonCode(line: string, inBlockComment: boolean):
  result = ""
  i = 0
  WHILE i < line.length:
    IF inBlockComment:
      end = line.indexOf("*/", i)
      IF end == -1:
        RETURN { stripped: result, inBlockComment: true }
      i = end + 2
      inBlockComment = false
    ELSE IF line[i] == '/' AND line[i+1] == '/':
      BREAK                              // rest of line is comment
    ELSE IF line[i] == '/' AND line[i+1] == '*':
      inBlockComment = true
      i += 2
    ELSE IF line[i] == '"':
      i++
      WHILE i < line.length AND line[i] != '"':
        IF line[i] == '\\': i++          // skip escaped character
        i++
      i++                                // skip closing quote
    ELSE IF line[i] == "'":
      i++
      WHILE i < line.length AND line[i] != "'":
        IF line[i] == '\\': i++
        i++
      i++
    ELSE:
      result += line[i]
      i++
  RETURN { stripped: result, inBlockComment }
```

The scan loop threads `inBlockComment` across lines:

```
resolveCompositionRoots():
  FOR EACH relPath IN fileHashes:
    lines = readFileSync(absPath).split('\n')
    inBlock = false
    FOR i = 0 TO lines.length - 1:
      { stripped, inBlock } = stripNonCode(lines[i], inBlock)
      pattern.lastIndex = 0
      WHILE (m = pattern.exec(stripped)) != null:
        record { file: absPath, line: i + 1 }
```

**Changes to `resolveInitCallers()`.**

Remove `let found = false` / `break`. Collect all matches, filter definition lines:

```
resolveInitCallers():
  FOR EACH loc IN all ConcreteLocations:
    initFnName = findEnclosingFunction(...)
    IF NOT initFnName OR initFnName == 'main': CONTINUE
    callPattern = /\b<initFnName>\s*\(/g
    allCallers = []
    FOR EACH relPath IN fileHashes (.c/.cpp only):
      inBlock = false
      FOR i = 0 TO lines.length - 1:
        { stripped, inBlock } = stripNonCode(lines[i], inBlock)
        callPattern.lastIndex = 0
        IF callPattern.test(stripped):
          isDef = functionDefinitions.get(initFnName)
                    ?.some(d => d.file == absPath && d.line == i + 1)
          IF NOT isDef:
            allCallers.push({ file: absPath, line: i + 1 })
    IF allCallers.length > 0:
      loc.compRootFile = allCallers[0].file
      loc.compRootLine = allCallers[0].line
      loc.allCompRoots = allCallers
```

**Edge cases.**

| Edge case | Handling |
|-----------|----------|
| Block comment spanning multiple lines | `inBlockComment` boolean threaded across lines |
| Escaped quote inside string (`"\""`) | Backslash check advances `i` by one extra |
| Character literals (`'\n'`) | Same backslash-escape handling |
| C++ raw string literals (`R"(...)"`) | Not handled — acceptable for a C11 project |
| Line-continuation backslash at EOL | Not handled — a false negative if `&varName` is split across continuation lines (pathological in practice) |
| Unclosed string at end-of-line | When a `"` is opened but not closed before EOL (malformed source or macro expansion), the inner string loop exits at `i == line.length`. The remainder of the line beyond the opening `"` is treated as inside a string (false negative — match suppressed). This is acceptable: the C compiler would also reject such a line, making it a pathological input. |

**Data structure changes.**

```typescript
// Optional new field on ConcreteLocation (types.ts):
/** All callers of the Init function found during resolveInitCallers(). */
allCompRoots?: Array<{ file: string; line: number }>;
```

No changes to `ParsedFile`, `NavigationIndex`, grammar, or visitor.

**Tradeoff table.**

| Axis | Rating |
|------|--------|
| Accuracy | Medium-high — eliminates false positives from comments and strings; does not handle backslash line-continuation or C++ raw strings |
| Implementation effort | Low — self-contained ~40-line pure helper; no grammar or visitor changes |
| Index size impact | Negligible — `allCompRoots` is a small optional array |
| Handles multiple callers | Yes — `found`/`break` removal collects all callers |

**Scope estimate: S (hours)**

---

#### 11.8.4 Option C — Structural Index-based Cross-reference (No New Parsing)

**Summary.** Infer composition roots from already-indexed structural data. The
`localTypeInfo` per-file map records every local variable's and parameter's type. A
function (in a `.c` file) that declares a variable of the Init function's root type — and
is not the Init function itself — is a candidate composition root. No additional parsing or
text scanning is required.

**Core insight.** By LibJuno convention the Init function's first parameter is a pointer to
the root type (e.g. `JUNO_DS_HEAP_ROOT_T *ptRoot`). Any other `.c` function that holds a
variable of that root type is a plausible caller. This data is fully available in
`index.localTypeInfo` after the parse pass.

**Algorithm pseudocode.**

```
FUNCTION findCompositionRootCandidates(loc: ConcreteLocation, index: NavigationIndex):
  initFnDefs = index.functionDefinitions.get(loc.functionName) ?? []
  IF initFnDefs.length == 0: RETURN []

  initFileLti = index.localTypeInfo.get(initFnDefs[0].file)
  initParams  = initFileLti?.functionParameters.get(loc.functionName) ?? []
  rootType    = initParams[0]?.typeName ?? ""     // e.g. "JUNO_DS_HEAP_ROOT_T"
  IF NOT rootType: RETURN []

  candidates = []
  // NOTE: NavigationIndex.localTypeInfo is keyed by ABSOLUTE file path
  // (set via parsed.filePath which is the absolute path from indexFile()).
  // NavigationIndex.functionDefinitions[n].file is also absolute.
  // The comparison d.file == filePath is therefore a valid absolute-to-absolute match.
  // Do NOT use workspace-relative paths here — they would silently produce no matches.
  FOR EACH [filePath, lti] IN index.localTypeInfo:
    ext = path.extname(filePath)
    IF ext != '.c' AND ext != '.cpp': CONTINUE

    FOR EACH [fnName, params] IN lti.functionParameters:
      IF fnName == loc.functionName: CONTINUE     // skip Init fn itself
      IF params.some(p => p.typeName == rootType):
        matchDef = index.functionDefinitions.get(fnName)
                     ?.find(d => d.file == filePath)
        IF matchDef: candidates.push({ file: matchDef.file, line: matchDef.line })

    FOR EACH [fnName, varMap] IN lti.localVariables:
      IF fnName == loc.functionName: CONTINUE
      IF [...varMap.values()].some(v => v.typeName == rootType):
        matchDef = index.functionDefinitions.get(fnName)
                     ?.find(d => d.file == filePath)
        IF matchDef AND NOT candidates.some(c => c.file==filePath):
          candidates.push({ file: matchDef.file, line: matchDef.line })

  RETURN candidates
```

**Integration with `resolveInitCallers()`.**

```
resolveInitCallers():
  FOR EACH loc IN all ConcreteLocations:
    candidates = findCompositionRootCandidates(loc, index)
    IF candidates.length > 0:
      loc.compRootFile = candidates[0].file
      loc.compRootLine = candidates[0].line
      loc.allCompRoots = candidates
```

Option C targets only `resolveInitCallers()`. The `resolveCompositionRoots()` method still
benefits from Option B's comment stripper for `initCallFile`/`initCallLine` accuracy; the
two options are complementary.

**Data structure changes.**

```typescript
// Same optional field as Option B:
allCompRoots?: Array<{ file: string; line: number }>;
// No changes to ParsedFile, NavigationIndex, grammar, or visitor.
```

**Limitations.**

- Correctly finds functions that declare a root type as a local or parameter — the standard
  LibJuno pattern.
- Misses composition roots that use an anonymous or cast pointer rather than a named
  variable of the root type (uncommon in well-typed LibJuno code).
- May produce false positives if multiple unrelated modules share the same root type name
  (unlikely in a correctly namespaced project).
- Functions in `.h` files are excluded by the `ext` check — correct because LibJuno
  composition roots always reside in `.c` files.

**Tradeoff table.**

| Axis | Rating |
|------|--------|
| Accuracy | Medium — structural heuristic; correct for standard LibJuno patterns; may miss cast-pointer callers or over-report on shared root type names |
| Implementation effort | Low-Medium — pure Map iteration over already-indexed data; no grammar or visitor changes |
| Index size impact | None — uses only already-indexed data |
| Handles multiple callers | Yes — algorithm collects all matching functions |

**Scope estimate: S–M (hours to a short sprint)**

---

#### 11.8.5 Comparison Summary

| Criterion | Option A (Visitor) | Option B (Stripper) | Option C (Structural) |
|-----------|-------------------|--------------------|-----------------------|
| Accuracy | High | Medium-high | Medium |
| Implementation effort | High | Low | Low-Medium |
| Index size impact | Medium | Negligible | None |
| Handles multiple callers | Yes | Yes | Yes |
| Grammar / visitor changes | Yes | No | No |
| Scope | M (1 sprint) | S (hours) | S–M |
| False positive risk | Very low | Low | Medium |
| False negative risk | Very low | Low (line-continuation) | Low–Medium (cast pointers) |

---

#### 11.8.6 Recommendation

**Option B is recommended as the immediate fix**, with Option A as the longer-term
follow-on.

Rationale:

1. **Option B eliminates the primary failure mode** — regex matching inside comments and
   strings — with the smallest possible code change. The stripper is a pure function with
   no dependencies on the grammar, CST, or index structure. It can be unit-tested in
   complete isolation and merged in a single small PR.

2. **Option B also fixes the multiple-callers defect** in `resolveInitCallers()` by
   removing the `found`/`break` early-exit. This requires touching only a few lines of
   existing code.

3. **Option A provides the highest long-term accuracy** because it operates on tokens
   (comments and strings are already stripped by the Chevrotain lexer before the CST is
   built), eliminates the redundant post-parse text scan entirely, and naturally produces a
   `callGraph` that could serve future "find all callers" navigation features. It is
   appropriate for a dedicated sprint after Option B stabilises the existing behaviour.

4. **Option C is a viable structural fallback** once Option B or A is in place — useful
   when the text scan produces no results, or as a cross-check. It should not replace
   Option B outright because its precision depends on the completeness of `localTypeInfo`.

**Recommended sequencing:**

| Sprint | Action |
|--------|--------|
| Current | Implement Option B (comment stripper + collect-all-callers) |
| Next | Implement Option A (visitor call-site tracking + `callGraph`) |
| Post-A | Optionally add Option C as a structural fallback inside `resolveInitCallers()` |

---

### 11.9 Composition Root UI — Concept Proposals

**Problem statement:** The current trace view shows a single Composition Root node
(one caller of the Init function). In a real workspace, the same Init function may be
called from multiple places — unit test harnesses, application entry points, platform
variants, and so on. A developer debugging a DI wiring problem needs to see ALL of
those callers at a glance.

Three UI concepts are presented below. Each is independent; at most one would be
implemented.

---

#### Option A — All Callers List in the Existing WebviewPanel

**Description:** Extend the current WebviewPanel so that the Composition Root node
expands into a numbered list of all caller locations instead of a single file:line
link. The remainder of the panel (Init Impl, Implementation) remains a shared subtree
below the list because those nodes are properties of the vtable assignment, not of the
individual caller.

**ASCII mockup — panel layout:**

```
┌─────────────────────────────────────────────┐
│  Vtable Resolution Trace                    │
│                                             │
│  📍 Call Site                               │
│     engine_app.c:223                        │
│     ptCmdPipeApi->Dequeue(...)              │
│  │                                          │
│  🔗 Composition Roots  (3 callers)          │
│     1. main.c:12                            │
│        JunoSb_BrokerInit(&tBroker, ...)     │
│     2. test_broker.c:45                     │
│        JunoSb_BrokerInit(&tBroker, ...)     │
│     3. platform_init.c:88                   │
│        JunoSb_BrokerInit(&tBroker, ...)     │
│  │                                          │
│  ⚙  Initialization Implementation           │
│     juno_sb_broker.c:45                     │
│     ptRoot->ptApi = ptApi                   │
│  │                                          │
│  ⚡ Implementation                           │
│     juno_buff_queue.c:112                   │
│     JUNO_STATUS_T JunoDs_BuffQueue_Dequeue  │
└─────────────────────────────────────────────┘
```

**HTML structure change** — replace the single `<div class="trace-node composition-root">`
with a list variant:

```html
<div class="trace-node composition-root">
  <span class="node-icon">🔗</span>
  <span class="node-label">Composition Roots</span>
  <span class="node-count">(3 callers)</span>
  <ul class="caller-list">
    <li>
      <a href="#" data-file="main.c" data-line="12">main.c:12</a>
      <code>JunoSb_BrokerInit(&amp;tBroker, &amp;gtCmdPipeApi)</code>
    </li>
    <li>
      <a href="#" data-file="test_broker.c" data-line="45">test_broker.c:45</a>
      <code>JunoSb_BrokerInit(&amp;tBroker, &amp;gtTestApi)</code>
    </li>
    <li>
      <a href="#" data-file="platform_init.c" data-line="88">platform_init.c:88</a>
      <code>JunoSb_BrokerInit(&amp;tBroker, &amp;gtPlatformApi)</code>
    </li>
  </ul>
</div>
```

**Key TypeScript interface change** (data model only — no new VSCode API):

```typescript
// Replace the single TraceNode for composition root with a list.
interface VtableTrace {
    callSite:         TraceNode;
    compositionRoots: TraceNode[];   // was: compositionRoot: TraceNode
    initImpl?:        TraceNode;
    implementation:   TraceNode;
}
```

**Implementation scope:** S (small). Changes are confined to `vtableTraceProvider.ts`
and the callers that build `VtableTrace` objects. No new VSCode API is needed. The
caller-finding logic (looking up all call sites of the Init function in the index) is
the main new algorithmic work; the rendering change is minimal.

**Tradeoff table:**

| Dimension               | Rating | Notes                                                           |
|-------------------------|--------|-----------------------------------------------------------------|
| Discoverability         | High   | Callers appear inline; no extra gesture needed                  |
| Implementation effort   | Low    | HTML/CSS and data model change only; no new VSCode contribution |
| Native VSCode feel      | Low    | Custom webview UI, not standard VSCode components               |
| Shows all callers       | Yes    | Full list with clickable file:line links                        |

---

#### Option B — VSCode TreeView Sidebar Panel ("LibJuno DI Wiring")

**Description:** Register a `vscode.TreeDataProvider<DiWiringItem>` in a new
activity-bar sidebar view (`"view": "libjuno.diWiring"`). The tree is always visible
and shows the full workspace DI wiring — not triggered by cursor position. The root
nodes are Module Root types; expanding a node reveals the API type, vtable fields,
their implementations, and finally the list of composition root callers.

**ASCII mockup — sidebar tree:**

```
LIBJUNO DI WIRING
▾ JUNO_SB_BROKER_ROOT_T
  ▾ JUNO_SB_BROKER_API_T
    ▾ Publish
      ▸ JunoSb_Broker_Publish  [juno_sb_broker.c:78]
        Composition Roots
          • main.c:12
          • test_broker.c:45
    ▾ RegisterSubscriber
      ▸ JunoSb_Broker_RegisterSubscriber  [juno_sb_broker.c:95]
        Composition Roots
          • main.c:12
▾ JUNO_DS_BUFF_QUEUE_ROOT_T
  ▾ JUNO_DS_BUFF_QUEUE_API_T
    ▾ Enqueue
      ▸ JunoDs_BuffQueue_Enqueue  [juno_buff_queue.c:88]
        Composition Roots
          • engine_app.c:55
    ▾ Dequeue
      ▸ JunoDs_BuffQueue_Dequeue  [juno_buff_queue.c:112]
        Composition Roots
          • engine_app.c:55
          • test_queue.c:30
```

**TypeScript interface for tree items:**

```typescript
type DiWiringItemKind =
    | 'module-root' | 'api-type' | 'vtable-field'
    | 'implementation' | 'caller-group' | 'caller';

interface DiWiringItem extends vscode.TreeItem {
    kind:     DiWiringItemKind;
    file?:    string;
    line?:    number;
    children: DiWiringItem[];
}

class DiWiringProvider implements vscode.TreeDataProvider<DiWiringItem> {
    getChildren(element?: DiWiringItem): DiWiringItem[];
    getTreeItem(element: DiWiringItem): vscode.TreeItem;
}
```

**Implementation scope:** L (large). Requires a new `vscode.TreeDataProvider`
registration, a new `package.json` contribution (`views`, `viewsContainers`,
`activitybar`), and an always-on indexing pass to build the full wiring tree on
workspace open. The tree must refresh when files change (FileSystemWatcher events).

**Tradeoff table:**

| Dimension               | Rating  | Notes                                                                  |
|-------------------------|---------|------------------------------------------------------------------------|
| Discoverability         | High    | Always visible in the sidebar; no trigger gesture needed               |
| Implementation effort   | High    | New TreeDataProvider, new activity bar entry, watcher integration      |
| Native VSCode feel      | High    | Uses standard VSCode tree widget, icons, collapse/expand behaviour     |
| Shows all callers       | Yes     | Caller nodes are first-class tree leaves; unlimited depth              |

---

#### Option C — CodeLens on Init Functions + Standard References Panel

**Description:** Register a `vscode.CodeLensProvider` that scans each C file for
vtable assignment statements (the same `vtableAssignments` index already built by
the parser). Above each Init function definition that wires a vtable, the lens
displays a count badge. Clicking the lens delegates to the built-in
`vscode.commands.executeCommand('editor.action.showReferences', ...)` VSCode command,
which opens the standard References panel listing all call sites.

**What the developer sees in the editor:**

```c
// juno_sb_broker.c

         ← [3 composition roots — click to show callers]
JUNO_STATUS_T JunoSb_BrokerInit(
    JUNO_SB_BROKER_ROOT_T *ptRoot,
    const JUNO_SB_BROKER_API_T *ptApi,
    ...)
{
    ...
    ptRoot->ptApi = ptApi;   // vtable wire
    ...
}
```

Clicking the lens opens the built-in References panel:

```
REFERENCES — JunoSb_BrokerInit
  main.c
    12: JunoSb_BrokerInit(&tBroker, &gtCmdPipeApi, ...)
  test_broker.c
    45: JunoSb_BrokerInit(&tBroker, &gtTestApi, ...)
  platform_init.c
    88: JunoSb_BrokerInit(&tBroker, &gtPlatformApi, ...)
```

**TypeScript class sketch:**

```typescript
class DiWiringCodeLensProvider implements vscode.CodeLensProvider {
    provideCodeLenses(document: vscode.TextDocument): vscode.CodeLens[] {
        // Look up vtableAssignments for this file in the index.
        // For each Init function that contains an assignment, emit one CodeLens
        // at the function definition line with the caller count.
    }
    resolveCodeLens(lens: vscode.CodeLens): vscode.CodeLens {
        // Attach the showReferences command with pre-computed caller locations.
        lens.command = {
            title:   `${callerCount} composition roots — click to show callers`,
            command: 'editor.action.showReferences',
            arguments: [uri, position, callerLocations],
        };
        return lens;
    }
}
```

**Implementation scope:** M (medium). Requires a new `vscode.CodeLensProvider`
registration and a `package.json` `"codeLens"` contribution, plus a lookup from
the existing `vtableAssignments` index to the call sites of the enclosing Init
function. No new webview or sidebar is needed. The References panel is entirely
provided by VSCode.

**Tradeoff table:**

| Dimension               | Rating  | Notes                                                                   |
|-------------------------|---------|-------------------------------------------------------------------------|
| Discoverability         | Medium  | Visible only when the developer has the Init file open                  |
| Implementation effort   | Medium  | New provider + contribution; leverages built-in References panel        |
| Native VSCode feel      | High    | Lens and References panel are first-class VSCode patterns               |
| Shows all callers       | Yes     | Standard References panel lists every caller with file:line context     |

---

#### Recommendation

**Option A (extended WebviewPanel caller list) is recommended** for an initial
implementation targeting a developer who is actively debugging a DI wiring problem.

Rationale:

1. **Lowest friction.** The developer is already looking at the trace view panel;
   all callers appear in the same place without an extra gesture or a file switch.
   Options B and C both require the developer to navigate away from their current
   context.

2. **Smallest scope.** Option A is a contained change to `vtableTraceProvider.ts`
   with no new VSCode contributions. This limits risk of regressions and minimises
   review burden. Option B requires a fully new sidebar contribution and always-on
   indexing. Option C requires a new provider registration and a non-trivial
   reverse-lookup step.

3. **Consistent UX.** The trace view already uses a custom webview for the
   Call Site → Composition Root → Init Impl → Implementation chain. Adding a caller
   list inside that same panel keeps the mental model coherent — one panel, one
   chain, all relevant locations.

4. **Option C is a viable complement** (not a replacement) once Option A is in
   place: CodeLens on Init functions would let a developer discover callers without
   first triggering the trace view. The two options are not mutually exclusive and
   could be implemented in subsequent sprints.

---

### 11.7 Composition Root Detection — Fragility Analysis

This section documents known fragility points in `resolveCompositionRoots()` (line ~434)
and `resolveInitCallers()` (line ~492) in `workspaceIndexer.ts`. Both methods use
line-by-line regex scanning to locate vtable wiring calls and their callers. The
analysis is based on the implementation as of the `feature/vscode-extension` branch and
uses `examples/udp-threads/src/main.cpp` as the primary real-world reference.

#### 11.7.1 Fragility Table

| ID | Function | Approx. Line | Description | Severity |
|----|----------|-------------|-------------|----------|
| FR-01 | `resolveCompositionRoots` | 448 | `&varName` regex matches inside C comments and string literals | High |
| FR-02 | `resolveCompositionRoots` | 483 | Only `sites[0]` is stamped — all additional call sites for the same API var are silently discarded | High |
| FR-03 | `resolveCompositionRoots` | 448 | Requires literal `&` at the call site — misses address-of via intermediate pointer or cast | Medium |
| FR-04 | `resolveCompositionRoots` | 452 | Scans `.h`/`.hpp` files — forward declarations, Doxygen, and extern references in headers match the pattern before any `.c` caller | Medium |
| FR-05 | `resolveInitCallers` | 505 | `main` exclusion is bare string equality — platform wrappers (`SDL_main`, `WinMain`, RTOS task entry functions) are not excluded | Low |
| FR-06 | `resolveInitCallers` | 508 | Call-pattern regex matches the function name inside Doxygen comments and block comments | Medium |
| FR-07 | `resolveInitCallers` | 511 | `found = true; break` stops at the first caller in an arbitrary `Map` iteration order — caller selection is non-deterministic when multiple callers exist | High |
| FR-08 | `resolveInitCallers` | 522–524 | Definition-line skip compares `absPath` and line number only — an inline function defined in a header is only recorded once; the same inline body included in other TUs is not skipped | Medium |
| FR-09 | `resolveInitCallers` | 499–501 | `findEnclosingFunction` uses a highest-start-line heuristic with no end-line knowledge — a match between two consecutive function definitions is attributed to the wrong function | Medium |
| FR-10 | `resolveCompositionRoots` | 462–464 | `pattern.lastIndex = 0` reset is latently fragile — correctness of the per-file multi-match scan depends on the reset being present on every line iteration; if a future maintainer moves it to the per-file loop, matches are silently skipped for every line after the first match in a file. Current code is correct; this is a maintainability hazard. | Medium |

#### 11.7.2 Concrete Failure Scenarios

**FR-01 — Regex matches in comments and string literals (High)**

The pattern `&(varName)\b` is applied to raw source text with no comment stripping or
string tokenization.

```c
// Wires the API — see &gtMyLoggerApi for the rationale
void Subsystem_Init(LOGGER_API_T *ptApi) { ... }
```

The comment on line 1 contains `&gtMyLoggerApi`. `resolveCompositionRoots` records this
comment line as an init-call site. `loc.initCallFile` and `loc.initCallLine` are stamped
with the comment line. The trace view's Composition Root node navigates the developer to
an irrelevant comment. No warning or diagnostic is emitted.

A string literal produces the same defect:

```c
const char *pcHelp = "Pass &gtMyLoggerApi to enable logging";
```

---

**FR-02 — Multiple call sites: only the first is retained (High)**

In `udp-threads/main.cpp`, two brokers are initialized in sequence:

```c
tStatus = JunoSb_BrokerInit(&s_tBroker1, s_aptBroker1Registry, 2u, NULL, NULL);
tStatus = JunoSb_BrokerInit(&s_tBroker2, s_aptBroker2Registry, 2u, NULL, NULL);
```

The two API variable names are distinct, so FR-02 does not fire for this specific case.
The defect fires when two separate modules in a larger project define a file-scoped API
variable with the same name (e.g., `s_tSchApi` in both `sensor.c` and `actuator.c`).
`initCallIndex.get("s_tSchApi")` returns two sites; `sites[0]` is stamped on every
`ConcreteLocation` with `apiVarName === "s_tSchApi"` — including the `ConcreteLocation`
that actually corresponds to `actuator.c`. That location now shows `sensor.c`'s init-call
line. The user navigates to the wrong file.

---

**FR-03 — Address-of via intermediate pointer (Medium)**

```c
static const JUNO_SCH_API_T s_tSchApi = { SchExecute, NULL, NULL };

void Wiring_Init(JUNO_SCH_ROOT_T *ptSch)
{
    const JUNO_SCH_API_T *ptApi = &s_tSchApi;   /* & IS present, but on this line */
    JunoSch_Init(ptSch, ptApi, NULL, NULL);     /* no & on this line */
}
```

`resolveCompositionRoots` matches `&s_tSchApi` on the local-variable assignment line and
records that line as the init-call site. `findEnclosingFunction` returns `"Wiring_Init"`.
`resolveInitCallers` then scans for callers of `Wiring_Init` — which may be `main` (and
thus excluded by FR-05 guard), or another function. If `Wiring_Init` is called from
`main`, the caller-search fires; if not found, `loc.compRootFile` stays `undefined` and
the trace view falls back to the assignment file. The composition root node is either
missing or points to the wrong abstraction layer.

---

**FR-04 — Header file false positives (Medium)**

`C_FILE_EXTENSIONS` includes `.h` and `.hpp`. A library header typically contains:

```c
/* juno_sch_api.h */
/**
 * @brief Default scheduler API. Pass &gtDefaultSchApi to JunoSch_Init.
 */
extern const JUNO_SCH_API_T gtDefaultSchApi;
```

The Doxygen comment `Pass &gtDefaultSchApi` matches the regex. The header file is
likely scanned before any `.c` composition root because `Map` insertion order is
scan order. `loc.initCallFile` is set to the header path and `loc.initCallLine` to the
Doxygen comment line. The trace view Composition Root node navigates to a library header
comment. No real composition root is found.

---

**FR-05 — Platform entry-point wrappers not excluded (Low)**

```c
/* SDL2 application */
int SDL_main(int argc, char *argv[])
{
    MyModule_Init(&s_tModuleApi, NULL, NULL);
    ...
}
```

`findEnclosingFunction` returns `"SDL_main"`. The guard `initFnName === 'main'` does not
match. `resolveInitCallers` scans for callers of `SDL_main`. SDL's internal dispatch
function is in a precompiled library not indexed by the extension, so the search finds no
caller and `loc.compRootFile` remains `undefined` — the composition root node is missing.
In a project that does test `SDL_main` from a test harness, the test file is recorded as
the composition root instead, showing the wrong location.

---

**FR-06 — Call-pattern regex matches Doxygen and block comments (Medium)**

Given a wrapper function:

```c
/**
 * Calls SystemStartup() to wire all vtables before spawning threads.
 */
void PlatformInit(void)
{
    SystemStartup();
}
```

When `resolveInitCallers` searches for callers of `SystemStartup`, the call pattern
`\bSystemStartup\s*\(` matches the Doxygen comment on line 2 of any file that contains
that text. The definition-line skip (lines 522–524) only excludes the exact line where
`SystemStartup` is defined; the Doxygen mention is not a definition line, so it passes.
`loc.compRootFile` and `loc.compRootLine` are set to the comment line. The trace view
navigates to the comment instead of `PlatformInit`.

---

**FR-07 — Non-deterministic caller selection across multiple callers (High)**

```c
/* board_a.c */
void BoardA_Init(void) {
    MyModule_Init(&s_tModuleApi, NULL, NULL);
}

/* board_b.c */
void BoardB_Init(void) {
    MyModule_Init(&s_tModuleApi, NULL, NULL);
}
```

Both `BoardA_Init` and `BoardB_Init` call `MyModule_Init`. After stamping,
`loc.initCallFile` points to the first `&s_tModuleApi` match found in file-scan order.
`findEnclosingFunction` returns (say) `"BoardA_Init"`. `resolveInitCallers` then scans
for callers of `BoardA_Init` and stops at the first match. But `BoardB_Init` is also a
valid composition root and is never surfaced. The choice between the two callers depends
entirely on `Map` insertion order, which changes if the workspace is indexed in a
different order (e.g., after a cache invalidation). The result is both incomplete and
non-deterministic.

---

**FR-08 — Inline function in header: definition-line skip fails for other TUs (Medium)**

```c
/* utils.h — included in app_a.c and app_b.c */
static inline void Utils_Init(MY_API_T *ptRoot) {
    ptRoot->ptApi = &gtMyApi;    /* & present — this line matched as init-call site */
}
```

The parser records the definition of `Utils_Init` at the absolute path of `utils.h`,
line 2. `resolveCompositionRoots` matches `&gtMyApi` on line 2 of `utils.h` and records
`utils.h:2` as the init-call site. `findEnclosingFunction` returns `"Utils_Init"`.
`resolveInitCallers` then scans for callers of `Utils_Init`. It finds the inline body
again in `utils.h` at line 2 and checks whether it is a definition: `defs?.some(d => d.file === absPath && d.line === i + 1)`. Since `absPath` is `utils.h` and the stored
definition record matches, the skip fires correctly for `utils.h`. However, if
`resolveInitCallers` scans `app_a.c` and that file's preprocessed text contains the
inlined body (not directly visible as source lines in the `.c` file), the mismatch is
irrelevant. The real defect surfaces if the indexer scans `utils.h` standalone and also
encounters a call to `Utils_Init` in a comment or a string within `utils.h` itself — the
definition-skip line comparison uses the wrong file path for those occurrences.

---

**FR-09 — `findEnclosingFunction` misattributes lines between function definitions (Medium)**

```c
/* file_scope_init.c */

JUNO_STATUS_T JunoMod_Alpha_Init(JUNO_MOD_ALPHA_ROOT_T *ptRoot, ...)
{
    ptRoot->ptApi = &gtAlphaApi;   /* line 5 */
}

/* File-scope vtable — NOT inside any function */
static const JUNO_MOD_BETA_API_T gtBetaApi = { BetaFn1, BetaFn2 };  /* line 9 */

JUNO_STATUS_T JunoMod_Beta_Init(JUNO_MOD_BETA_ROOT_T *ptRoot, ...)
{
    ptRoot->ptApi = &gtBetaApi;   /* line 13 */
}
```

`resolveCompositionRoots` scans line 9 and matches `gtBetaApi` (no `&` here, so FR-03
applies). But if line 9 were `static const JUNO_MOD_BETA_API_T *p = &gtBetaApi;`,
`findEnclosingFunction` would be called with line 9. The highest definition start-line
at or below line 9 is `JunoMod_Alpha_Init` at line 3. `findEnclosingFunction` returns
`"JunoMod_Alpha_Init"` instead of `undefined` (no enclosing function). The composition
root chain is attributed to `Alpha`'s caller rather than to `Beta`'s caller — wrong
result, no diagnostic.

---

**FR-10 — `pattern.lastIndex` latent fragility across files (Medium)**

In `resolveCompositionRoots`, the combined regex is created once with flag `'g'` before
the file loop, and `pattern.lastIndex = 0` is reset at the top of the per-line loop:

```typescript
// line 447-448
const pattern = new RegExp(`&(${escaped.join('|')})\\b`, 'g');
...
for (let i = 0; i < lines.length; i++) {
    pattern.lastIndex = 0;          // defensive reset — see note below
    while ((m = pattern.exec(lines[i])) !== null) { ... }
}
```

**Current status: the code is correct.** When `exec` returns `null` (all matches on the
line exhausted), ECMAScript 2022 §22.2.7.2 step 12 resets `lastIndex` to 0 automatically
for global regexes. The explicit reset at the top of the per-line loop is therefore
defensive-best-practice, not strictly necessary — it guards against a caller invoking
`exec` without consuming to `null`. **Do not remove it.**

The latent hazard is a future refactor: if a maintainer moves the reset to the outer file
loop (to reduce loop overhead), `lastIndex` will carry over between files. On the second
and subsequent files, the first `exec` call may start mid-string and silently skip the
leading portion of the first matched line. Rated Medium (not High) because the current
code is not broken and the refactor that would introduce the bug requires an active change
to the reset placement.

#### 11.7.3 Missing Coverage Summary

The following C/C++ patterns are completely unhandled by the current implementation:

1. **Vtable wired via indirect pointer** — only `&apiVarName` at the call site is
   matched; any indirection (local pointer copy, cast, `memcpy`, struct member
   assignment) produces no composition root entry.

2. **Multiple wiring sites for the same API struct variable name** — only the first
   match is retained per `apiVarName`; a codebase that uses the same static variable
   name in multiple translation units (a common pattern for module-local API tables)
   will always show the same site regardless of which module is being navigated.

3. **Caller disambiguation when many functions call the same Init wrapper** — the first
   caller found in file-scan order is stored with no awareness of TU proximity, call
   graph depth, or the current navigation context. All other callers are discarded.

4. **Platform-specific entry points** — `WinMain`, `SDL_main`, `app_main`, RTOS task
   entry functions (e.g., `vTaskCode`, `osThreadFunc_t`). Only bare `"main"` is
   excluded; all platform variants are propagated to the caller-search step.

5. **Comment and string literal filtering** — no tokenization or comment stripping is
   applied before the regex scan. Any occurrence of `&apiVarName` in a `//` comment,
   `/* */` block, `#if 0` block, or string literal is treated as a live wiring call.

6. **Header file false positives** — `.h` and `.hpp` files are included in the scan
   scope by `C_FILE_EXTENSIONS`. Headers routinely contain the API variable name in
   Doxygen, `extern` declarations, and usage notes that will match the pattern before
   any `.c` caller is reached.

7. **`findEnclosingFunction` boundary errors** — the enclosing-function heuristic uses
   only the function's start line and has no knowledge of end lines. A match in a
   file-scope initializer between two function definitions is attributed to the preceding
   function. This misattribution propagates silently into `loc.compRootFile` and
   `loc.compRootLine`.
