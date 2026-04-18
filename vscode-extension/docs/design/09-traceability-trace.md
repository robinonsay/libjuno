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

The trace view complements the existing Go to Definition feature (§5.1): Go to Definition navigates directly to the implementation, while the trace view surfaces the intermediate wiring step — the composition root vtable assignment — making the full dependency injection chain visible. This is especially useful for debugging DI configuration issues and understanding the wiring of large LibJuno-based systems.

### 11.2 Component: VtableTraceProvider

A new component added to the VSCode Integration Layer (§3.5). Responsibility: collect the full 3-node resolution trace and render it in a `vscode.WebviewPanel`.

**TypeScript interfaces:**

```typescript
interface TraceNode {
  type:   'call-site' | 'composition-root' | 'implementation';
  label:  string;   // e.g., "ptCmdPipeApi->Dequeue(...)"
  file:   string;   // workspace-relative path
  line:   number;
  detail: string;   // additional context line
}

interface VtableTrace {
  callSite:        TraceNode;
  compositionRoot: TraceNode;
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

STEP 3 — Build the composition root node (REQ-VSCODE-031)
  // Use the extended ConcreteLocation (with assignmentFile/assignmentLine from §4.1)
  location = result.locations[selectedIndex or 0]
  compositionRoot = {
    type:   'composition-root',
    label:  `.${fieldName} = ${location.functionName}`,
    file:   location.assignmentFile,
    line:   location.assignmentLine,
    detail: // read line from assignment file at assignmentLine
  }

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
  Show a WebviewPanel with HTML rendering of the 3 nodes in tree layout (see §11.3)
```

### 11.3 WebviewPanel Layout

The panel is opened via `vscode.window.createWebviewPanel` with `enableScripts: true`. It uses a self-contained HTML template with inline CSS and a nonce-based inline script — no external resources.

The tree layout uses CSS border and padding to create a visual connection between nodes:

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
      <a href="#" data-file="..." data-line="...">engine_app.c:45</a>
      <code>.Dequeue = JunoDs_BuffQueue_Dequeue</code>
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
