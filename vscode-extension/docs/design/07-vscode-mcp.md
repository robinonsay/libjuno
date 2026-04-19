> Part of: [Software Design Document](index.md) — Sections 6-7: VSCode Integration and MCP Server

// @{"design": ["REQ-VSCODE-005", "REQ-VSCODE-006", "REQ-VSCODE-007", "REQ-VSCODE-013"]}
## 6. VSCode Integration Details

### 6.1 DefinitionProvider Registration

The extension registers a `vscode.DefinitionProvider` for C and C++ files in `extension.ts`'s `activate` function:

```typescript
vscode.languages.registerDefinitionProvider(
  [{ language: 'c' }, { language: 'cpp' }],
  new JunoDefinitionProvider(index, vtableResolver, failureHandlerResolver)
);
```

### 6.2 JunoDefinitionProvider Logic

```typescript
class JunoDefinitionProvider implements vscode.DefinitionProvider {
  provideDefinition(
    document: vscode.TextDocument,
    position: vscode.Position,
    token: vscode.CancellationToken
  ): vscode.ProviderResult<vscode.Definition | vscode.LocationLink[]> {
    // 1. Check if line matches a FAIL macro call site pattern (§5.3.1 Step 0):
    //      /\bJUNO_FAIL\s*\(/, /\bJUNO_FAIL_MODULE\s*\(/, etc.
    //    If yes: invoke FailureHandlerResolver in FAIL macro mode → goto 4.
    // 2. Check if line matches a LibJuno vtable call site pattern (ptApi->Field)
    //    If vtable call: invoke VtableResolver → goto 4.
    // 3. Check if line matches a failure handler assignment (JUNO_FAILURE_HANDLER / _pfcnFailureHandler)
    //    If yes: invoke FailureHandlerResolver in standard §5.3 mode → goto 4.
    // 4. If result.found and single: return LocationLink[]
    // 5. If result.found and multiple: show QuickPick, return undefined
    //    (QuickPick handles navigation imperatively)
    // 6. If not found: show status bar error (Section 8), return undefined
  }
}
```

The provider returns `undefined` (not an error) in the multiple-results and not-found cases; navigation is handled imperatively to allow QuickPick use.

### 6.3 QuickPick Display (REQ-VSCODE-006)

When multiple implementations are found, a QuickPick is displayed with:
- **label:** function name (e.g., `JunoDs_Heap_Insert`)
- **description:** `file.c:27`
- **detail:** workspace-relative file path

Selecting an item opens the file at the specified line.

### 6.4 VSCode Commands

| Command ID | Title | Behavior |
|------------|-------|----------|
| `libjuno.goToImplementation` | LibJuno: Go to Implementation | Runs resolver on current cursor position |
| `libjuno.reindexWorkspace` | LibJuno: Re-index Workspace | Clears cache, runs full workspace scan |

---

// @{"design": ["REQ-VSCODE-017", "REQ-VSCODE-018", "REQ-VSCODE-019", "REQ-VSCODE-020"]}
## 7. MCP Server Design

### 7.1 Overview

The extension starts an HTTP MCP server on a local port (default 6543, configurable) bound to `127.0.0.1`. The port is advertised in a `.libjuno/mcp.json` file so AI platforms can discover it.

MCP is chosen as the AI interface mechanism (REQ-VSCODE-020) because it is platform-agnostic: GitHub Copilot, Claude (via Claude Desktop), and other platforms all support MCP tool servers. The extension does not implement any platform-specific API beyond MCP.

### 7.2 MCP Tool: `resolve_vtable_call`

**Satisfies:** REQ-VSCODE-018

**Description:** Given a C source file path and cursor position, resolves the LibJuno vtable API call to its concrete implementation function(s).

**Input Schema:**
```json
{
  "type": "object",
  "required": ["file", "line", "column"],
  "properties": {
    "file": {
      "type": "string",
      "description": "Absolute or workspace-relative path to the C source file."
    },
    "line": {
      "type": "integer",
      "description": "1-based line number of the API call site."
    },
    "column": {
      "type": "integer",
      "description": "1-based column number of the cursor within the line."
    }
  }
}
```

**Output Schema:**
```json
{
  "type": "object",
  "properties": {
    "found": { "type": "boolean" },
    "locations": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "functionName": { "type": "string" },
          "file":         { "type": "string" },
          "line":         { "type": "integer" }
        }
      }
    },
    "error": { "type": "string" }
  }
}
```

**Example request:**
```json
{ "file": "src/main.c", "line": 42, "column": 15 }
```

**Example response (found):**
```json
{
  "found": true,
  "locations": [
    { "functionName": "JunoDs_Heap_Insert", "file": "src/juno_heap.c", "line": 27 }
  ]
}
```

**Example response (not found):**
```json
{
  "found": false,
  "locations": [],
  "error": "No implementation found for 'JUNO_DS_HEAP_API_T::Insert'."
}
```

### 7.3 MCP Tool: `resolve_failure_handler`

**Satisfies:** REQ-VSCODE-019

**Description:** Given a C source file path and cursor position on a failure handler assignment or call, resolves the concrete handler function(s).

**Input Schema:** Same as `resolve_vtable_call`.

**Output Schema:** Same as `resolve_vtable_call`.

### 7.4 MCP Discovery File

The extension writes `.libjuno/mcp.json` to the workspace root on activation:

```json
{
  "mcpServers": {
    "libjuno": {
      "url": "http://127.0.0.1:6543/mcp"
    }
  }
}
```

This is the format recognized by Claude Desktop and similar platforms. For platforms using the VSCode MCP host API (e.g., GitHub Copilot), the extension also registers the server via `vscode.lm.registerMcpServer` (if available in the host version).

---
