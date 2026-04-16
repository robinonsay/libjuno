# LibJuno VSCode Extension — Developer Guide

Vtable-aware Go to Definition for LibJuno embedded C projects.

---

## Table of Contents

1. [Overview](#overview)
2. [Prerequisites](#prerequisites)
3. [Getting Started](#getting-started)
4. [Project Structure](#project-structure)
5. [Building](#building)
6. [Running and Debugging](#running-and-debugging)
7. [Testing](#testing)
8. [Mutation Testing](#mutation-testing)
9. [Packaging](#packaging)
10. [Extension Configuration](#extension-configuration)
11. [Architecture Overview](#architecture-overview)

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
