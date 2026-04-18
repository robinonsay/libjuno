> Part of: [Software Development Plan](index.md) — Sections 1-3

# Software Development Plan — LibJuno VSCode Extension

**Document Version:** 4.1  
**Date:** 2026-04-18  
**Project:** LibJuno VSCode Extension — Vtable Go-to-Definition, Failure Handler Navigation, MCP Server  
**Status:** All phases complete (Phase 4 removed by PM decision)

---

## 1. Project Summary

The LibJuno VSCode Extension provides "Go to Definition" (F12/Ctrl+Click) resolution for
LibJuno's vtable-based dispatch pattern. When a developer clicks on a virtual function call
like `ptTime->ptApi->Now(ptTime)`, the extension resolves the call through the vtable
assignment chain and navigates to the concrete function implementation.

The extension also provides:
- Failure handler navigation (`_pfcnFailureHandler` / `JUNO_FAILURE_HANDLER` assignments)
- An embedded HTTP MCP server for AI agent integration
- A persistent JSON navigation cache for fast startup

### 1.1 Technology Stack

| Component | Technology |
|-----------|-----------|
| Language | TypeScript (ES2020, CommonJS, strict) |
| C Parser | Chevrotain ^11.0.3 (lexer + CstParser + CST visitor) |
| Extension API | VSCode ^1.85.0 (DefinitionProvider, commands, FileSystemWatcher) |
| Test Framework | Jest ^29.0.0 + ts-jest |
| MCP Server | Node.js `http` module (vanilla, no framework) |

### 1.2 Architecture Overview

```
┌──────────────────────────────────────────────────────────┐
│  Extension Activation (extension.ts)                     │
│  ├── WorkspaceIndexer ──► CacheManager                   │
│  │      └── parseFileWithDefs() ──► Chevrotain Parser    │
│  │             ├── Lexer (lexer.ts)                      │
│  │             ├── CParser (parser.ts)                   │
│  │             └── IndexBuildingVisitor (visitor.ts)      │
│  │                    └──► NavigationIndex (CRUD)         │
│  ├── VtableResolver ◄── NavigationIndex ──► FHResolver   │
│  ├── JunoDefinitionProvider ◄── Resolvers                │
│  │      ├── StatusBarHelper                              │
│  │      └── QuickPickHelper                              │
│  └── McpServer ◄── Resolvers                             │
└──────────────────────────────────────────────────────────┘
```

Seven components, bottom-up dependency order:
1. **C Parser** (lexer → grammar → visitor) — parses C files into structured records
2. **Navigation Index** — in-memory Map-based store of all parsed records
3. **Workspace Indexer** — scans workspace, hashes files, coordinates parsing and cache
4. **Cache Manager** — JSON serialization/deserialization, atomic writes
5. **Resolvers** (VtableResolver, FailureHandlerResolver, resolverUtils) — resolve call sites to implementation locations
6. **VSCode Integration** (DefinitionProvider, QuickPick, StatusBar) — bridges resolvers to VSCode UI
7. **MCP Server** — HTTP API exposing resolution tools for AI agents

### 1.3 Known Design Questions

**`apiCallSites` — REMOVED (PM Decision, Sprint 3)**

The `IndexBuildingVisitor` populates `ParsedFile.apiCallSites` during CST traversal. However,
`WorkspaceIndexer.mergeInto()` does not read or store `apiCallSites`, and neither resolver
consumes it. **Per PM decision (Sprint 3), this dead-code data path is accepted as-is.**
Phase 4 (Visitor: Call Sites & Completeness) has been removed from the development plan.
The visitor logic remains in place but is not tested or maintained.

---

## 2. Current Status

### 2.1 Source Code Status

All 15 source files compile cleanly. No known compilation errors.

| File | Lines | Status | Notes |
|------|-------|--------|-------|
| `parser/lexer.ts` | ~408 | Tested | 72 lexer tests passing; FloatingLiteral fix (Sprint 17) |
| `parser/parser.ts` | ~1230 | Tested | 5 fixes (Sprints 1, 15, 17) |
| `parser/visitor.ts` | ~1076 | Partially tested | 1 bug fixed (Sprint 1); struct/vtable init, functions, failure handlers, local type info tested |
| `parser/types.ts` | 413 | N/A | Type definitions only |
| `indexer/navigationIndex.ts` | 101 | Tested | createEmptyIndex, clearIndex, removeFileRecords — 7 tests (Sprint 3) |
| `indexer/workspaceIndexer.ts` | 399 | Tested | 17 tests passing; reindexFile deferred fix (Sprint 17); resolveDefinitionLocation fix (Sprint 18) |
| `resolver/vtableResolver.ts` | 244 | Tested | 18 tests passing (TC-RES-001–011, NEG-001) — Sprint 4; chain-walk resolution with 3 regex strategies (`macroRe`, `arrayRe`, `generalRe`) + field-name fallback |
| `resolver/failureHandlerResolver.ts` | 162 | Tested | 13 tests passing (TC-FH-001–004, 005a/b, 006, 007, NEG-001–003, BND-001) — Sprint 5 |
| `resolver/resolverUtils.ts` | 109 | Tested | 11 tests passing (TC-UTIL-001–006, NEG-001–002, BND-001, PRI-001) — Sprint 4 |
| `cache/cacheManager.ts` | 250 | Tested | 7 tests; 1 source bug fixed (null guards in cacheToIndex) — Sprint 7 |
| `providers/junoDefinitionProvider.ts` | 90 | Tested | 10 tests passing (TC-VSC-001–008, NEG-001, BND-001) — Sprint 11 |
| `providers/quickPickHelper.ts` | 34 | Tested | QuickPick UI display |
| `providers/statusBarHelper.ts` | 54 | Tested | Status bar messages |
| `mcp/mcpServer.ts` | 174 | Tested | 14 tests passing (TC-MCP-002–007, 009–016) — Sprint 10; `start()` returns `Promise<number>` |
| `extension.ts` | 208 | Tested | Activation tested via mock — Sprint 11 |

**M3 Source change completed (Sprint 10):** `McpServer.start()` now returns `Promise<number>` (the bound port). WI-13.0 is complete.

### 2.2 Test Status

| Test File | Tests | Status |
|-----------|-------|--------|
| `parser/__tests__/lexer.test.ts` | 72 | All passing |
| `parser/__tests__/parser-grammar.test.ts` | 157 | All passing |
| `parser/__tests__/visitor-structs.test.ts` | 18 | All passing |
| `parser/__tests__/visitor-vtable.test.ts` | 5 | All passing (includes TC-P6-001, TC-P6-002) |
| `parser/__tests__/visitor-functions.test.ts` | 12 | All passing (TC-P11 function defs, TC-P10 failure handlers) |
| `parser/__tests__/visitor-localtypeinfo.test.ts` | 8 | All passing (TC-LTI-001–005, NEG-001, NEG-002, BND-001) — Sprint 3 |
| `resolver/__tests__/resolverUtils.test.ts` | 11 | All passing (TC-UTIL-001–006, NEG-001–002, BND-001, PRI-001) — Sprint 4 |
| `resolver/__tests__/vtableResolver.test.ts` | 18 | All passing (TC-RES-001a/b, TC-RES-002–005, TC-RES-006a–d, TC-RES-007–011, TC-RES-NEG-001) — Sprint 4 |
| `resolver/__tests__/failureHandlerResolver.test.ts` | 13 | All passing (TC-FH-001–004, 005a/b, 006, 007, NEG-001–003, BND-001) — Sprint 5 |
| `indexer/__tests__/fileExtensions.test.ts` | 85 | All passing |
| `indexer/__tests__/navigationIndex.test.ts` | 7 | All passing (TC-IDX-001–005, NEG-001, BND-001) — Sprint 3 |
| `src/__tests__/integration.test.ts` | 6 | All passing (TC-INT-001–006) — Sprint 6 |
| `cache/__tests__/cacheManager.test.ts` | 7 | All passing (TC-CACHE-001, 002, 006, 009, 010, NEG-001, BND-001) — Sprint 7 |
| `indexer/__tests__/workspaceIndexer.test.ts` | 30 | All passing (includes TC-WI-015–018 cross-file definition resolution regression tests — Sprint 18) |
| `mcp/__tests__/mcpServer.test.ts` | 14 | All passing (TC-MCP-002–007, 009–016) — Sprint 10 |
| `providers/__tests__/junoDefinitionProvider.test.ts` | 10 | All passing (TC-VSC-001–008, NEG-001, BND-001) — Sprint 11 |
| `src/__tests__/bulk-headers.test.ts` | 4 | All passing (TC-BULK-004) — Sprint 15 |
| `src/__tests__/sprint17-regression.test.ts` | 15 | All passing (REG-17-001–010b) — Sprint 17 |
| `providers/__tests__/statusBarHelper.test.ts` | 3 | All passing (TC-ERR-001/002/003) — Sprint 14 |
| `providers/__tests__/quickPickHelper.test.ts` | 5 | All passing (TC-QP-001/002/003/004/005) — Sprint 14 |
| `src/__tests__/e2e-smoke.test.ts` | 3 | All passing (TC-E2E-SMOKE-001/002/003) — Sprint 14 |
| `src/__tests__/extension-branches.test.ts` | varies | All passing — Sprint 14 |
| `parser/__tests__/visitor-branches.test.ts` | varies | All passing — Sprint 14 |
| **Total** | **622** | **All passing** |

### 2.3 Bugs Found and Fixed (Sprint 1)

| Bug | Root Cause | Fix |
|-----|-----------|-----|
| Bug A | `visitor.ts` used `"Identifier2"` key instead of `"Identifier"` in `walkStructOrUnionSpecifier` | Changed to `tok(c, "Identifier")` |
| Bug B | `parser.ts` `declarationSpecifiers` used greedy `AT_LEAST_ONE` causing Identifier consumption | Restructured to OR + MANY with GATE on LA(2) |
| Bug C | `visitor.ts` `walkExpressionStatement` called `drillToPostfix(lhsCond)` instead of `drillToPostfix(ae)` | Changed argument to `ae` |

### 2.4 Bugs Found and Fixed (Sprint 18)

| Bug | Root Cause | Fix |
|-----|-----------|-----|
| Bug D | `resolveDefinitionLine()` returned only a line number; when the function definition was in a different file than the vtable assignment, `ConcreteLocation.file` used the assignment file while `ConcreteLocation.line` came from the definition file — a file/line mismatch causing Go-to-Definition to navigate to the wrong location | Refactored to `resolveDefinitionLocation()` returning `{ file, line }`; updated `mergeInto()` and `resolveDeferred()` to use the resolved definition file; added same-file preference in index fallback; fallback to assignment location when definition not found |

### 2.5 Bugs Found and Fixed (Sprint 19 — Go-to-Definition C11 Conformance)

**Bug description:** Go-to-Definition on a vtable field (e.g., the `Reset` entry in an API struct initializer such as `.Reset = EngineCmdMsg_Reset`) navigated to the vtable assignment line or a forward declaration instead of the function definition line (line 88 of `engine_cmd_msg.c` for `EngineCmdMsg_Reset`). The defect manifested across every vtable initializer whose function body contained a unary operator immediately followed by a cast expression (for example, `return *(JUNO_STATUS_T *) pvUserData;` or `ptVtable->pfcnFn = & (FN_T) { ... };`).

**Root cause (three-layer analysis):**
1. **Layer 1 — Grammar (parser.ts):** The `unaryExpression` Chevrotain rule recursed to `unaryExpression` for every unary operator, violating C11 §6.5.3. The standard requires `++` and `--` to recurse to `unary-expression`, but `& * + - ~ !` must recurse to `cast-expression`. With the wrong recursion target, any unary operator followed by a cast-expression operand caused the parser to bail out mid-body.
2. **Layer 2 — Visitor:** Not defective; visitor would have handled the CST correctly had the parser emitted it.
3. **Layer 3 — Indexer:** When the parser bailed, the wrapping function definition was never emitted, so `functionDefinitions` held no record and the vtable resolver fell back to the assignment location.

**Three-layer fix summary:**

| Bug | Root Cause | Fix |
|-----|-----------|-----|
| Bug E | `unaryExpression` rule recursed to `unaryExpression` for `& * + - ~ !`, diverging from C11 §6.5.3 (which requires recursion to `cast-expression` for these six operators). Any function body containing, for example, `return *(T *) pv;` caused the parser to bail mid-body, dropping the function definition record and breaking Go-to-Definition to the definition line. | Split the first alternative of `unaryExpression` in `src/parser/parser.ts` into two alternatives: `++\|--` recurses to `unaryExpression`; `& * + - ~ !` recurses to `castExpression`. Preserved `sizeof` and `postfixExpression` branches. Renumbered inner `OR` ordinals (`OR2`/`OR3`/`OR4`) to satisfy Chevrotain's self-analysis. Added C11 §6.5.3 production citation as an inline comment. |

**Test coverage:**
- `src/parser/__tests__/c11-grammar-conformance.test.ts` — 21 canonical C11 expression/statement pattern tests (TC-CONF-001–021), permanent regression guard against future grammar drift. All tagged `@{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}`.
- `src/parser/__tests__/go-to-def-bug.test.ts` — TC-WI-019: focused parser-level reproduction against the controlled fixture under `test/fixtures/go-to-def-bug/`.
- `src/__tests__/go-to-def-bug-e2e.test.ts` — TC-WI-020a (fixture E2E through `WorkspaceIndexer`) and TC-WI-020b (real `examples/example_project/engine/src/engine_cmd_msg.c` — locks `EngineCmdMsg_Reset` resolving to line 88, not the forward-decl at 31 nor the vtable initializer entry at 53).
- Gate C result: **645/645 green** (622 baseline + 23 new).

**Requirements delta:**
- `REQ-VSCODE-005` tightened to specify navigation to the function definition line, explicitly excluding vtable assignment, vtable initializer entry, and forward-declaration lines.
- `REQ-VSCODE-033` added — "C11 Expression Grammar Conformance" requiring the parser to implement C11 §6.5.3 with the correct recursion targets for each unary operator.

---

## 3. High-Level Work Breakdown Structure

The project is organized into **16 small, focused phases**. Each phase targets one well-defined architectural layer or concern, is scoped to at most one sprint, and has explicit acceptance criteria before moving to the next phase.

```
Phase  1 ─── Parser Foundation                     [COMPLETE]                Sprint 1  ✅
Phase  2 ─── Visitor: Vtable Init Patterns          [COMPLETE]                Sprint 2  ✅
Phase  3 ─── Visitor: Local Type Info Extraction    [COMPLETE]                Sprint 3  ✅
Phase  4 ─── Visitor: Call Sites & Completeness     [REMOVED — PM Decision]
Phase  5 ─── Navigation Index CRUD                  [COMPLETE]                Sprint 3  ✅
Phase  6 ─── Resolver Utilities                     [COMPLETE]                Sprint 4  ✅
Phase  7 ─── VtableResolver                         [COMPLETE]                Sprint 4  ✅
Phase  8 ─── FailureHandlerResolver                 [COMPLETE]                Sprint 5  ✅
Phase  9 ─── Visitor → Index → Resolver Integration [COMPLETE]                Sprint 6  ✅
Phase 10 ─── CacheManager                           [COMPLETE]                Sprint 7  ✅
Phase 11 ─── WorkspaceIndexer Core                  [COMPLETE]                Sprint 9  ✅
Phase 12 ─── File Discovery & Deferred Resolution   [COMPLETE]                Sprint 9  ✅
Phase 13 ─── MCP Server                             [COMPLETE]                Sprint 10 ✅
Phase 14 ─── VSCode Mocks & Definition Provider     [COMPLETE]                Sprint 11 ✅
Phase 15 ─── Error UX, QuickPick & StatusBar        [COMPLETE]                Sprint 12 ✅
Phase 16 ─── End-to-End Smoke & Final Quality       [COMPLETE]                Sprint 14 ✅
Phase 17 ─── Parser: Production Header Compatibility [COMPLETE]               Sprint 15 ✅
Phase 18 ─── Parser & Indexer: Production Source Compatibility [COMPLETE]     Sprint 17 ✅
Phase 19 ─── Parser: C11 §6.5.3 Unary Expression Conformance   [COMPLETE]     Sprint 19 ✅
```

> **Note:** Phases 11+12 share Sprint 9. These are small enough to combine in execution but remain separate phases for tracking and traceability purposes.

### Phase Overview

| Phase | Focus | Components Under Test | Sprint | Test Sections |
|-------|-------|----------------------|--------|---------------|
| 1 | Parser Foundation | Lexer, Parser, Visitor (structs, vtable patterns P6, functions, failure handlers) | 1 ✅ | TC-P1–P5, TC-P6-001/002, TC-P10–P11 (partial) |
| 2 | Visitor: Vtable Init Patterns | Visitor (designated init, direct assign, positional init) | 2 ✅ | TC-P6-003, TC-P7, TC-P8 |
| 3 | Visitor: Local Type Info | Visitor (localVariables, functionParameters) | 3 ✅ | TC-LTI-001–005 |
| 4 | Visitor: Call Sites & Completeness | REMOVED — PM Decision | — | — |
| 5 | Navigation Index CRUD | NavigationIndex | 3 ✅ | TC-IDX-001–005 |
| 6 | Resolver Utilities | resolverUtils | 4 ✅ | TC-UTIL-001–006 |
| 7 | VtableResolver | VtableResolver | 4 ✅ | TC-RES-001–011 |
| 8 | FailureHandlerResolver | FailureHandlerResolver | 5 ✅ | TC-FH-001–006 |
| 9 | Visitor → Index → Resolver Integration | Full data pipeline (no stubs) | 6 ✅ | TC-INT-001–006 |
| 10 | CacheManager | CacheManager | 7 ✅ | TC-CACHE-001–010 |
| 11 | WorkspaceIndexer Core | WorkspaceIndexer | 9 ✅ | TC-WI-001–006 |
| 12 | File Discovery & Deferred Resolution | WorkspaceIndexer (file scan, deferred positional, multi-module FH) | 9 ✅ | TC-FILE-001, TC-WI-007–008 |
| 13 | MCP Server | McpServer | 10 ✅ | TC-MCP-002–007, 009–016 |
| 14 | VSCode Mocks & Definition Provider | JunoDefinitionProvider, vscode mock | 11 ✅ | TC-VSC-001–008, NEG-001, BND-001 |
| 15 | Error UX, QuickPick & StatusBar | StatusBarHelper, QuickPickHelper | 12 ✅ | TC-ERR-001–006, TC-QP-001–005 |
| 16 | End-to-End Smoke & Final Quality | Full stack | 14 ✅ | Smoke tests |
| 17 | Parser: Production Header Compatibility | Parser (macroCallStatement, void macro arg) | 15 ✅ | TC-MACRO-STMT-001–006, NEG-001, BND-001, TC-MODROOT-VOID-001, TC-BULK-004 |
| 18 | Parser & Indexer: Production Source Compatibility | Parser (looksLikeCast, braced init, EOF sentinel), WorkspaceIndexer (reindexFile deferred), Lexer (FloatingLiteral) | 17 ✅ | REG-17-001–010b |
| 19 | Parser: C11 §6.5.3 Unary Expression Conformance | Parser (unaryExpression recursion targets) | 19 ✅ | TC-CONF-001–022, TC-WI-019, TC-WI-020a/b |

---
