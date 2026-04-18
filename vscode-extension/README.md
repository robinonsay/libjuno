# LibJuno VSCode Extension

Vtable-aware Go to Definition for LibJuno embedded C projects.

---

## Table of Contents

### User Guide

1. [Overview](#overview)
2. [Installation](#installation)
3. [How It Works](#how-it-works)
4. [Supported Patterns](#supported-patterns)
5. [Using Go to Definition](#using-go-to-definition)
6. [Re-indexing](#re-indexing)
7. [MCP Server for AI Agents](#mcp-server-for-ai-agents)
8. [Configuration](#configuration)
9. [Troubleshooting](#troubleshooting)

### Developer Guide

10. [Prerequisites](#prerequisites)
11. [Getting Started](#getting-started)
12. [Project Structure](#project-structure)
13. [Building](#building)
14. [Running and Debugging](#running-and-debugging)
15. [Testing](#testing)
16. [Mutation Testing](#mutation-testing)
17. [Packaging](#packaging)
18. [Extension Configuration](#extension-configuration)
19. [Architecture Overview](#architecture-overview)

---

## Overview

LibJuno uses vtable-based dependency injection (DI) — function calls dispatched
through `ptApi->Foo(...)` — that standard IDE tooling cannot resolve to concrete
implementations. This extension bridges that gap by:

- Building a workspace-wide navigation index from a Chevrotain-based C parser.
- Wiring that index into VSCode's native **Go to Definition** system (F12 /
  Ctrl+Click).
- Providing failure-handler navigation (`_pfcnFailureHandler` /
  `JUNO_FAILURE_HANDLER` assignments).
- Exposing an embedded **MCP (Model Context Protocol) HTTP server** so AI agents
  (GitHub Copilot, Claude, etc.) can query the same resolution data.

**Entry in `package.json`**

| Field | Value |
|---|---|
| Name | `libjuno` |
| Display name | LibJuno |
| Version | 0.1.0 |
| VSCode engine | ^1.85.0 |
| Activation | `onLanguage:c`, `onLanguage:cpp` |

---

## Installation

Install the packaged `.vsix` file from the command line:

```bash
code --install-extension libjuno-<version>.vsix
```

Or via the Extensions panel: open **Extensions** (`Ctrl+Shift+X`), click the
`···` overflow menu, select **Install from VSIX…**, and choose the `.vsix` file.

The extension activates automatically the first time any C or C++ file is opened
in the workspace. No restart is required.

---

## How It Works

LibJuno uses vtable-based dependency injection. Function calls look like:

```c
ptModule->ptApi->Method(ptModule, arg);
```

Standard IDE "Go to Definition" (F12) cannot resolve these because the function
pointer is assigned at runtime through a vtable struct — the IDE sees only a
pointer dereference, not a concrete symbol.

This extension solves that by:

1. Parsing every C/C++ file in the workspace with a Chevrotain-based C parser.
2. Building an in-memory index of vtable struct assignments and function
   definitions.
3. At navigation time, tracing the vtable assignment chain from the call site to
   the concrete function implementation and returning the source location to
   VSCode.

On first activation the extension indexes all `.c`, `.h`, `.cpp`, `.hpp`, `.hh`,
and `.cc` files in the workspace. A persistent cache written to
`.libjuno/navigation-cache.json` in the workspace root speeds up subsequent
activations so that only changed files need to be re-parsed.

---

## Supported Patterns

The extension resolves the following call patterns when F12 / Ctrl+Click is used.
Place the cursor on the **function name** (the word after the last `->`) and
then invoke Go to Definition.

```c
// 1. Indirect API pointer — place cursor on "RegisterSubscriber"
ptEngineApp->ptBroker->ptApi->RegisterSubscriber(ptBroker, &pipe);

// 2. Direct API pointer — place cursor on "LogInfo"
const JUNO_LOG_API_T *ptLoggerApi = ptLogger->ptApi;
ptLoggerApi->LogInfo(ptLogger, "message");

// 3. Dot-accessed API — place cursor on "Copy"
tResult.ptApi->Copy(ptSrc, ptDst);

// 4. Named API member — place cursor on "Compare"
ptHeap->ptHeapPointerApi->Compare(ptA, ptB);

// 5. Failure handler — place cursor on the handler function name
ptModule->tRoot._pfcnFailureHandler = MyFailureHandler;
ptModule->tRoot.JUNO_FAILURE_HANDLER = MyFailureHandler;
```

The extension does **not** resolve:

- **Direct function calls** such as `EngineApp_Init(...)` — VSCode's built-in
  C/C++ tooling (e.g., the C/C++ extension or clangd) already handles these.
- **Macro calls** such as `JUNO_ASSERT_SUCCESS(...)` — macro expansion is out of
  scope.
- Functions defined outside the workspace.

---

## Using Go to Definition

1. Open any C/C++ file in a LibJuno project.
2. Place the cursor on a vtable-dispatched function name (e.g., `LogInfo` in
   `ptLoggerApi->LogInfo(...)`).
3. Press **F12** or **Ctrl+Click** (**Cmd+Click** on macOS).
4. If one implementation is found, VSCode navigates directly to it.
5. If multiple implementations exist, a picker appears — select the desired one.

Alternatively, use the Command Palette:

```
Ctrl+Shift+P → LibJuno: Go to Implementation
```

---

## Re-indexing

The extension watches for file changes and re-indexes modified files automatically.
After bulk operations (renaming directories, adding many new source files) a
manual re-index may be needed:

```
Ctrl+Shift+P → LibJuno: Re-index Workspace
```

The status bar shows **LibJuno: Indexed X files** once the index is current. If
the count looks wrong, run a manual re-index.

---

## MCP Server for AI Agents

The extension runs an embedded MCP (Model Context Protocol) HTTP server, which
allows AI agents (GitHub Copilot, Claude, etc.) to query vtable and failure
handler resolution programmatically.

| Item | Detail |
|---|---|
| Default port | `6543` |
| Discovery file | `.libjuno/mcp.json` in the workspace root |
| Disable | Set `libjuno.mcpServerPort` to `0` |

Configure the port in VS Code settings:

```json
"libjuno.mcpServerPort": 6543
```

---

## Configuration

| Setting | Default | Description |
|---|---|---|
| `libjuno.excludedDirectories` | `["build", "deps", ".libjuno"]` | Directories skipped during indexing. Add `vendor`, `third_party`, or similar as needed. |
| `libjuno.mcpServerPort` | `6543` | Port for the embedded MCP server. Set to `0` to disable. |

---

## Troubleshooting

### F12 / Ctrl+Click doesn't navigate to the implementation

**1. Is the cursor on a vtable-dispatched call?**
The extension only resolves calls through vtable function pointers such as
`ptApi->Method(...)`. Direct function calls like `EngineApp_Init(...)` are
handled by VSCode's built-in C/C++ tooling, not this extension. See
[Supported Patterns](#supported-patterns) for the full list of resolvable forms.

**2. Is the workspace indexed?**
Check the status bar — it should show **LibJuno: Indexed X files**. If it is
missing or shows 0, run **LibJuno: Re-index Workspace** from the Command Palette.

**3. Is the implementation file in the workspace?**
The extension can only resolve to functions defined in files within the workspace
root. Functions in external libraries or outside the workspace root will not be
found.

**4. Is the vtable assignment visible?**
The extension needs to find where the vtable struct is populated — for example:

```c
static const JUNO_APP_API_T tApi = { .OnStart = OnStart, .OnStop = OnStop };
```

If this assignment is in a file that has not been indexed (e.g., it was added
after the last indexing run), resolution will fail. Run a manual re-index.

**5. Check the Output panel.**
Open the Output panel (`Ctrl+Shift+U`), select **LibJuno** from the channel
dropdown, and look for error messages or indexing diagnostics.

**6. Is the file type supported?**
The extension indexes `.c`, `.h`, `.cpp`, `.hpp`, `.hh`, and `.cc` files. Other
file types are ignored.

---

## Prerequisites

| Tool | Minimum version | Notes |
|---|---|---|
| Node.js | 18+ | Bundled inside VSCode; required on the host for development |
| npm | 9+ | Comes with Node.js |
| TypeScript | ^5.3.0 | Installed as a dev dependency — no global install needed |
| VS Code | ^1.85.0 | Required to run or debug the extension |

Optional, for packaging:

```
npm install -g @vscode/vsce
```

---

## Getting Started

```bash
# 1. Navigate to the extension directory
cd vscode-extension

# 2. Install all dependencies
npm install

# 3. Compile TypeScript to the out/ directory
npm run compile
```

After compilation the extension can be launched from the Run and Debug panel (see
[Running and Debugging](#running-and-debugging)).

---

## Project Structure

```
vscode-extension/
├── src/
│   ├── extension.ts                  — Extension entry point (activate / deactivate)
│   ├── cache/
│   │   └── cacheManager.ts           — JSON navigation cache persistence
│   ├── indexer/
│   │   ├── navigationIndex.ts        — In-memory Map-based navigation data store
│   │   └── workspaceIndexer.ts       — Workspace scanning, hashing, parse coordination
│   ├── mcp/
│   │   └── mcpServer.ts              — Embedded HTTP MCP server for AI agents
│   ├── parser/
│   │   ├── lexer.ts                  — Chevrotain lexer (C token definitions)
│   │   ├── parser.ts                 — Chevrotain CstParser (C grammar rules)
│   │   ├── types.ts                  — TypeScript types for parsed records
│   │   ├── visitor.ts                — CST visitor that extracts index data
│   │   └── __tests__/                — Parser unit tests
│   ├── providers/
│   │   ├── junoDefinitionProvider.ts — VSCode DefinitionProvider integration
│   │   ├── quickPickHelper.ts        — Multi-implementation picker UI
│   │   └── statusBarHelper.ts        — Status bar indexing progress
│   └── resolver/
│       ├── vtableResolver.ts         — Vtable call → concrete impl resolution
│       ├── failureHandlerResolver.ts — Failure handler assignment resolution
│       └── resolverUtils.ts          — Shared resolution utilities
├── jest-esm-to-cjs.cjs               — Custom Jest transformer for Chevrotain ESM bundle
├── jest.config.js                    — Jest configuration
├── stryker.config.json               — Stryker mutation testing configuration
├── tsconfig.json                     — TypeScript compiler options
├── package.json                      — npm scripts, dependencies, VS Code manifest
├── .vscodeignore                     — Files excluded from the packaged .vsix
└── .gitignore                        — Files excluded from version control
```

Generated at build time:

```
out/          — Compiled JavaScript (CommonJS, ES2020 target); excluded from git
coverage/     — Jest coverage reports; excluded from git
reports/      — Stryker HTML mutation reports; excluded from git
.stryker-tmp/ — Stryker working directory; excluded from git
*.vsix        — Packaged extension archive; excluded from git
```

---

## Building

All build commands are run from the `vscode-extension/` directory.

### One-shot compile

```bash
npm run compile
```

Invokes `tsc` with the options in `tsconfig.json`:
- **Target:** ES2020
- **Module system:** CommonJS
- **Output directory:** `out/`
- **Source maps:** included alongside each `.js` file (`sourceMap: true`)
- **Strict mode:** enabled

### Watch mode

```bash
npm run watch
```

Runs `tsc -w` and recompiles automatically on every file save. Use this during
active development, especially alongside the extension host debugger.

---

## Running and Debugging

The recommended way to run the extension during development is through the VS Code
**Extension Development Host**:

1. Open the `libjuno` workspace root in VS Code.
2. Press **F5** (or open the Run and Debug panel and select **Run Extension**).
3. A new VS Code window opens with the extension loaded from `out/extension.js`.
4. Open a folder that contains LibJuno C source files.

> **Tip:** Keep `npm run watch` running in a terminal while debugging so that
> TypeScript changes are compiled automatically without needing to restart the
> task.

Breakpoints set in `.ts` source files work because source maps are emitted to
`out/`.

### Logging

The extension writes diagnostic messages to the **Output** panel under the channel
`LibJuno`. Look there for indexing progress, parse errors, and MCP server startup
messages.

---

## Testing

Tests are written with **Jest** and **ts-jest** and live in `src/**/__tests__/`
directories, matched by the `**/*.test.ts` glob.

### Run all tests

```bash
npm test
```

### Chevrotain ESM workaround

Chevrotain v11+ ships as an **ESM-only** package. Jest runs in CommonJS mode by
default, so a direct `import 'chevrotain'` would fail at test time.

Two mechanisms work together to solve this:

1. **`moduleNameMapper`** in `jest.config.js` redirects `chevrotain` imports to
   the self-contained bundle at
   `node_modules/chevrotain/lib/chevrotain.mjs`.
2. **`jest-esm-to-cjs.cjs`** — a minimal custom Jest transformer that strips the
   trailing `export { ... };` block from the bundle and replaces it with
   `exports.Name = Name;` assignments, making it loadable as CommonJS at test
   time.

The `transform` section of `jest.config.js` applies `ts-jest` to `.ts`/`.tsx`
files and `jest-esm-to-cjs.cjs` to `.mjs` files. The `transformIgnorePatterns`
entry allows the `chevrotain` package directory to pass through the transformer
pipeline (Jest's default is to ignore all of `node_modules`).

These settings are opaque; **do not modify them** unless you are upgrading
Chevrotain or Jest.

### Test environment notes

- `testEnvironment: 'node'` — tests run in a pure Node.js environment, not jsdom.
- VS Code API types (`@types/vscode`) are available but the VS Code runtime is
  **not** available in tests. Components that call VS Code APIs must be isolated
  behind the providers layer and excluded from unit test scope.

---

## Mutation Testing

[Stryker Mutator](https://stryker-mutator.io/) is configured to measure the
effectiveness of the parser test suite by introducing deliberate code mutations
and checking whether tests catch them.

### Run mutation tests

```bash
npm run test:mutation
```

### What gets mutated

Only the core parsing components are mutated (see `stryker.config.json`):

| File | Reason |
|---|---|
| `src/parser/lexer.ts` | Token definitions — incorrect token boundaries would be caught by parser tests |
| `src/parser/parser.ts` | Grammar rules — structural changes to the C grammar |
| `src/parser/visitor.ts` | CST-to-index extraction — incorrect field extraction or skipped nodes |

### Thresholds

| Level | Score |
|---|---|
| High | ≥ 80% |
| Low | ≥ 75% |
| Break (CI failure) | < 75% |

Scores below 75% cause `stryker run` to exit with a non-zero code.

### Reports

Stryker writes an HTML report to the `reports/` directory and a summary to
stdout. Open `reports/mutation/mutation.html` in a browser to inspect which
mutants survived.

### Stryker configuration highlights

- **Checker:** `typescript` — type-invalid mutants are filtered out before test
  runs, keeping the feedback loop fast.
- **Concurrency:** 8 workers.
- **`ignoreStatic: true`** — mutations to static (module-level) initialisation
  are skipped; they are difficult to test in isolation and rarely hide bugs.
- **Timeout:** 60 000 ms per test run.

---

## Packaging

The extension is packaged into a `.vsix` archive using
[`@vscode/vsce`](https://github.com/microsoft/vscode-vsce).

### Install `vsce` (once)

```bash
npm install -g @vscode/vsce
```

### Package

```bash
npm run package
```

This runs `vsce package` in the `vscode-extension/` directory and produces a
`libjuno-<version>.vsix` file.

### What is included in the package

The `.vscodeignore` file excludes the following from the `.vsix`:

| Excluded | Reason |
|---|---|
| `src/**` | TypeScript sources — only compiled JS is shipped |
| `out/**/*.map` | Source maps — not needed by end users |
| `node_modules/**` | Dependencies are bundled by vsce if needed, or excluded |
| `.vscode/**` | Editor settings |
| `tsconfig.json`, `jest.config.js` | Developer tooling |
| `**/*.test.ts`, `**/*.test.js` | Test files |

### Install locally

```bash
code --install-extension libjuno-0.1.0.vsix
```

---

## Extension Configuration

The extension contributes the following VS Code settings under the `libjuno`
namespace:

| Setting | Type | Default | Description |
|---|---|---|---|
| `libjuno.excludedDirectories` | `string[]` | `["build", "deps", ".libjuno"]` | Directories to exclude from workspace indexing. Add `vendor`, `third_party`, or similar as needed. |
| `libjuno.mcpServerPort` | `number` | `6543` | Port the embedded MCP HTTP server listens on. Change if there is a conflict with another local service. |

### Contributed commands

| Command ID | Title | Typical invocation |
|---|---|---|
| `libjuno.goToImplementation` | LibJuno: Go to Implementation | Command Palette or keybinding |
| `libjuno.reindexWorkspace` | LibJuno: Re-index Workspace | Command Palette after adding new source files |

### Cache location

The navigation cache is written to `.libjuno/navigation-cache.json` in the
workspace root. This directory is listed under `libjuno.excludedDirectories` by
default so that the cache file itself is not re-indexed.

---

## Architecture Overview

The extension has seven components arranged in a bottom-up dependency stack:

```
┌──────────────────────────────────────────────────────────┐
│  Extension Activation (extension.ts)                     │
│  ├── WorkspaceIndexer ──► CacheManager                   │
│  │      └── parseFileWithDefs() ──► Chevrotain Parser    │
│  │             ├── Lexer (lexer.ts)                      │
│  │             ├── CParser (parser.ts)                   │
│  │             └── IndexBuildingVisitor (visitor.ts)     │
│  │                    └──► NavigationIndex (CRUD)        │
│  ├── VtableResolver ◄── NavigationIndex ──► FHResolver   │
│  ├── JunoDefinitionProvider ◄── Resolvers                │
│  │      ├── StatusBarHelper                              │
│  │      └── QuickPickHelper                              │
│  └── McpServer ◄── Resolvers                             │
└──────────────────────────────────────────────────────────┘
```

### Component responsibilities

| # | Component | Files | Responsibility |
|---|---|---|---|
| 1 | **C Parser** | `parser/lexer.ts`, `parser/parser.ts`, `parser/visitor.ts` | Tokenises C source, builds a CST, and walks it to extract structured records (struct definitions, vtable assignments, function definitions, failure handler assignments). |
| 2 | **Navigation Index** | `indexer/navigationIndex.ts` | In-memory `Map`-based store for all parsed records. Provides CRUD operations used by the indexer and resolvers. |
| 3 | **Workspace Indexer** | `indexer/workspaceIndexer.ts` | Scans the workspace for C/H files, computes content hashes to detect changes, coordinates parse calls, and manages incremental updates via VS Code `FileSystemWatcher`. |
| 4 | **Cache Manager** | `cache/cacheManager.ts` | Serialises the navigation index to JSON and writes it atomically to `.libjuno/navigation-cache.json`. Deserialises on startup to avoid a full re-scan. |
| 5 | **Resolvers** | `resolver/vtableResolver.ts`, `resolver/failureHandlerResolver.ts`, `resolver/resolverUtils.ts` | Traverse the vtable assignment chain from a call site to locate the concrete function implementation. Shared utilities live in `resolverUtils.ts`. |
| 6 | **VSCode Integration** | `providers/junoDefinitionProvider.ts`, `providers/quickPickHelper.ts`, `providers/statusBarHelper.ts` | Implements `vscode.DefinitionProvider`, presents a QuickPick when multiple implementations exist, and shows indexing progress in the status bar. |
| 7 | **MCP Server** | `mcp/mcpServer.ts` | Runs a vanilla Node.js HTTP server on `libjuno.mcpServerPort` (default 6543). Exposes vtable and failure-handler resolution as JSON-RPC-style MCP tools consumable by GitHub Copilot, Claude, and other MCP-compatible AI agents. |

### Data flow: Go to Definition (F12)

```
User presses F12 on a call site
  → JunoDefinitionProvider.provideDefinition()
  → VtableResolver.resolve(document, position)
      → NavigationIndex.lookup(...)
  → Single result   → return Location directly
  → Multiple results → QuickPickHelper shows picker → user selects → navigate
  → No result       → show informative error message (non-intrusive)
```

### Activation sequence

1. Extension activates on the first C or C++ file opened (`onLanguage:c` / `onLanguage:cpp`).
2. Reads `libjuno.excludedDirectories` and `libjuno.mcpServerPort` from workspace settings.
3. Displays a progress notification while loading the cache (`CacheManager`) or running a full index (`WorkspaceIndexer`).
4. Shows the indexed file count in the status bar.
5. Registers `JunoDefinitionProvider` and the two contributed commands.
6. Starts the MCP server.
