# Software Development Plan — LibJuno VSCode Extension

**Document Version:** 3.0  
**Date:** 2025-07-22  
**Project:** LibJuno VSCode Extension — Vtable Go-to-Definition, Failure Handler Navigation, MCP Server  
**Status:** Phases 1–3, 5–14 complete, Phase 4 removed, Phases 15–16 pending

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
| `parser/lexer.ts` | 401 | Tested | 72 lexer tests passing |
| `parser/parser.ts` | ~1050 | Tested | 2 bugs fixed (Sprint 1) |
| `parser/visitor.ts` | ~1076 | Partially tested | 1 bug fixed (Sprint 1); struct/vtable init, functions, failure handlers, local type info tested |
| `parser/types.ts` | 413 | N/A | Type definitions only |
| `indexer/navigationIndex.ts` | 101 | Tested | createEmptyIndex, clearIndex, removeFileRecords — 7 tests (Sprint 3) |
| `indexer/workspaceIndexer.ts` | 399 | Not tested | Full/incremental indexing, file scanning, cache coordination |
| `resolver/vtableResolver.ts` | 244 | Tested | 18 tests passing (TC-RES-001–011, NEG-001) — Sprint 4; chain-walk resolution with 3 regex strategies (`macroRe`, `arrayRe`, `generalRe`) + field-name fallback |
| `resolver/failureHandlerResolver.ts` | 162 | Tested | 13 tests passing (TC-FH-001–004, 005a/b, 006, 007, NEG-001–003, BND-001) — Sprint 5 |
| `resolver/resolverUtils.ts` | 109 | Tested | 11 tests passing (TC-UTIL-001–006, NEG-001–002, BND-001, PRI-001) — Sprint 4 |
| `cache/cacheManager.ts` | 250 | Tested | 7 tests; 1 source bug fixed (null guards in cacheToIndex) — Sprint 7 |
| `providers/junoDefinitionProvider.ts` | 90 | Tested | 10 tests passing (TC-VSC-001–008, NEG-001, BND-001) — Sprint 11 |
| `providers/quickPickHelper.ts` | 34 | Not tested | QuickPick UI display |
| `providers/statusBarHelper.ts` | 54 | Not tested | Status bar messages |
| `mcp/mcpServer.ts` | 174 | Tested | 14 tests passing (TC-MCP-002–007, 009–016) — Sprint 10; `start()` returns `Promise<number>` |
| `extension.ts` | 208 | Tested | Activation tested via mock — Sprint 11 |

**M3 Source change completed (Sprint 10):** `McpServer.start()` now returns `Promise<number>` (the bound port). WI-13.0 is complete.

### 2.2 Test Status

| Test File | Tests | Status |
|-----------|-------|--------|
| `parser/__tests__/lexer.test.ts` | 72 | All passing |
| `parser/__tests__/parser-grammar.test.ts` | 148 | All passing |
| `parser/__tests__/visitor-structs.test.ts` | 17 | All passing |
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
| `indexer/__tests__/workspaceIndexer.test.ts` | 17 | All passing (TC-WI-001–009, TC-CACHE-003–005, TC-FILE-001, NEG-001, BND-001) — Sprints 8–9 |
| `mcp/__tests__/mcpServer.test.ts` | 14 | All passing (TC-MCP-002–007, 009–016) — Sprint 10 |
| `providers/__tests__/junoDefinitionProvider.test.ts` | 10 | All passing (TC-VSC-001–008, NEG-001, BND-001) — Sprint 11 |
| **Total** | **450** | **All passing** |

### 2.3 Bugs Found and Fixed (Sprint 1)

| Bug | Root Cause | Fix |
|-----|-----------|-----|
| Bug A | `visitor.ts` used `"Identifier2"` key instead of `"Identifier"` in `walkStructOrUnionSpecifier` | Changed to `tok(c, "Identifier")` |
| Bug B | `parser.ts` `declarationSpecifiers` used greedy `AT_LEAST_ONE` causing Identifier consumption | Restructured to OR + MANY with GATE on LA(2) |
| Bug C | `visitor.ts` `walkExpressionStatement` called `drillToPostfix(lhsCond)` instead of `drillToPostfix(ae)` | Changed argument to `ae` |

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
Phase 11 ─── WorkspaceIndexer Core                  [PENDING]                 Sprint 9
Phase 12 ─── File Discovery & Deferred Resolution   [PENDING]                 Sprint 9
Phase 13 ─── MCP Server                             [COMPLETE]                Sprint 10 ✅
Phase 14 ─── VSCode Mocks & Definition Provider     [COMPLETE]                Sprint 11 ✅
Phase 15 ─── Error UX, QuickPick & StatusBar        [PENDING]                 Sprint 12
Phase 16 ─── End-to-End Smoke & Final Quality       [PENDING]                 Sprint 13
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
| 11 | WorkspaceIndexer Core | WorkspaceIndexer | 9 | TC-WI-001–006 |
| 12 | File Discovery & Deferred Resolution | WorkspaceIndexer (file scan, deferred positional, multi-module FH) | 9 | TC-FILE-001, TC-WI-007–008 |
| 13 | MCP Server | McpServer | 10 ✅ | TC-MCP-002–007, 009–016 |
| 14 | VSCode Mocks & Definition Provider | JunoDefinitionProvider, vscode mock | 11 ✅ | TC-VSC-001–008, NEG-001, BND-001 |
| 15 | Error UX, QuickPick & StatusBar | StatusBarHelper, QuickPickHelper | 12 | TC-ERR-001–006, TC-QP-001–005 |
| 16 | End-to-End Smoke & Final Quality | Full stack | 13 | Smoke tests |

---

## 4. Detailed Phase Plans

### Phase 1: Parser Foundation ✅ COMPLETE

**Sprint:** 1 (complete)  
**Goal:** Establish a fully tested Chevrotain lexer, grammar parser, and CST visitor for the C constructs used by LibJuno.  
**Prerequisites:** None.

**Outcomes:**
- 4 test files, 98 tests, 0 failures
- 3 source bugs found and fixed (Bugs A, B, C — see Section 2.3)
- `visitor-vtable.test.ts` includes TC-P6-001 (designated initializer single field) and TC-P6-002 (designated initializer multi-field) — counted as Phase 1 deliverables

**Acceptance Criteria:** ✅ All met.

---

### Phase 2: Visitor — Vtable Init Patterns ✅ COMPLETE

**Sprint:** 2 (complete)  
**Goal:** Achieve full test coverage for all vtable initialization patterns (designated, direct, positional) excluding call sites and local type info.  
**Prerequisites:** Phase 1 complete.

#### 2.1 Scope

| Work Item | TC IDs | Description | Complexity |
|-----------|--------|-------------|------------|
| WI-2.1 | TC-P6-003 | Designated initializer — remaining patterns not covered in Phase 1 | Low |
| WI-2.2 | TC-P7-001 to TC-P7-002 | Direct assignment extraction | Low |
| WI-2.3 | TC-P8-001 to TC-P8-004 | Positional initializer extraction (API struct field zip) | Medium |
| WI-2.4 | TC-P10-001 to TC-P10-003 | Failure handler assignments — gap check vs. Phase 1 tests | Low |
| WI-2.5 | TC-P11-001 to TC-P11-007 | Function definitions — gap check vs. Phase 1 tests | Low |
| WI-2.6 | TC-P6-NEG-001 | Negative: empty initializer list `= { }` → no vtable assignment emitted | Low |
| WI-2.7 | TC-P7-NEG-001 | Negative: assignment to non-function-pointer field → not recorded as vtable assignment | Low |
| WI-2.8 | TC-P8-BND-001 | Boundary: positional initializer with exactly one field → field zip produces single pair | Low |

#### 2.2 Test Approach

Tests extend `visitor-vtable.test.ts` for WI-2.1 through WI-2.3. A gap-check pass compares existing `visitor-functions.test.ts` and `visitor-vtable.test.ts` against the full TC-P10/P11 and TC-P6 lists and adds any missing cases. All tests inject synthetic C source directly into `parseFileWithDefs()` — no file system access.

#### 2.3 Discovery Checkpoint

After writing TC-P8-001 (first positional init test): dump `vtableAssignments` and `apiStructFields` output. Verify the field zip produces the correct `(fieldName, functionName)` pairs before writing TC-P8-002 through TC-P8-004.

#### 2.4 Debug Budget

**30%** — Rationale: designated/direct patterns are simple; positional init requires cross-referencing `apiStructFields` order, which is an untested path and has medium bug probability. Phase 1 actuals were 40%; 30% is a calibrated estimate for the simpler patterns in this phase.

#### 2.5 Acceptance Criteria

- [ ] TC-P6-003 implemented and passing
- [ ] TC-P7-001, TC-P7-002 implemented and passing
- [ ] TC-P8-001 through TC-P8-004 implemented and passing
- [ ] TC-P6-NEG-001, TC-P7-NEG-001, TC-P8-BND-001 implemented and passing (negative/boundary — v2.1 Amendment C1)
- [ ] TC-P10 and TC-P11 gap check complete — all cases have coverage
- [ ] No regressions in Phase 1 tests (98 tests still passing)
- [ ] Any source bugs documented in lessons-learned

#### 2.6 Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Positional init field zip order wrong | Medium | Medium | Discovery checkpoint after TC-P8-001; inspect `apiStructFields` before continuing |
| `visitor-functions` gap check reveals untested patterns | Low | Low | Budget one session for gap analysis before writing new tests |
| Regression in existing 98 tests | Low | High | Run full suite before and after each test file edit |

---

### Phase 3: Visitor — Local Type Info Extraction ✅ COMPLETE

**Sprint:** 3 (complete)  
**Goal:** Verify that `visitLocalDeclaration` and `visitFunctionParameters` emit correct variable and parameter type records into `localTypeInfo` — the data that `resolverUtils.lookupVariableType()` actually consumes.  
**Prerequisites:** Phase 2 complete.

#### 3.1 Scope

| Work Item | TC IDs | Description | Complexity |
|-----------|--------|-------------|------------|
| WI-3.1 | TC-LTI-001 | `visitLocalDeclaration` — simple typed local variable (e.g., `JUNO_TIME_T *ptTime`) | Low |
| WI-3.2 | TC-LTI-002 | `visitLocalDeclaration` — pointer-to-struct local variable | Low |
| WI-3.3 | TC-LTI-003 | `visitFunctionParameters` — single typed parameter | Low |
| WI-3.4 | TC-LTI-004 | `visitFunctionParameters` — multiple parameters, correct index mapping | Medium |
| WI-3.5 | TC-LTI-005 | `localTypeInfo` function scope — variables from different functions do not collide | Medium |
| WI-3.6 | TC-LTI-NEG-001 | Negative: function with no local variables → `localVariables` Map empty for that scope | Low |
| WI-3.7 | TC-LTI-NEG-002 | Negative: function with no parameters → `functionParameters` Map empty for that scope | Low |
| WI-3.8 | TC-LTI-BND-001 | Boundary: variable with deeply nested pointer type (e.g., `JUNO_FOO_T **ppFoo`) → type extracted correctly | Low |

#### 3.2 Test Approach

Create `visitor-localtypeinfo.test.ts` (new file). Each test parses a synthetic C function body and inspects the `localTypeInfo` field of the returned `ParsedFile`. Tests verify that `localVariables` and `functionParameters` Maps contain the correct `(variableName → typeName)` entries. No resolver invocation — this phase tests only the visitor output.

#### 3.3 Discovery Checkpoint

After TC-LTI-001: dump the full `localTypeInfo` structure to understand the exact Map nesting (e.g., function name → {localVariables, functionParameters}). Confirm the key format matches what `resolverUtils.ts` expects before writing further tests. Record the exact format in lessons-learned for use in Phases 6 and 7.

#### 3.4 Debug Budget

**35%** — Rationale: `localTypeInfo` is the highest-probability bug source in the entire project. It is populated by the visitor, consumed by the resolver, and has never been tested. Any mismatch in key format between visitor and resolver will break Phase 7. Investing debug time here prevents cascading failures upstream.

#### 3.5 Acceptance Criteria

- [ ] TC-LTI-001 through TC-LTI-005 implemented and passing
- [ ] TC-LTI-NEG-001, TC-LTI-NEG-002, TC-LTI-BND-001 implemented and passing (negative/boundary — v2.1 Amendment C1)
- [ ] `localVariables` Map keys and values confirmed to match `resolverUtils.lookupVariableType()` expected format
- [ ] `functionParameters` Map keys and values confirmed correct
- [ ] `localTypeInfo` key format recorded in lessons-learned for Phases 6 and 7
- [ ] No regressions in Phases 1–2

#### 3.6 Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| `localTypeInfo` key format doesn't match resolver expectations | High | High | Discovery checkpoint after TC-LTI-001; fix visitor or document mismatch before Phase 7 |
| Multi-function scoping causes variable collision | Medium | Medium | TC-LTI-005 specifically targets this; checkpoint output before writing |
| Visitor doesn't populate `localTypeInfo` at all | Medium | High | Run diagnostic first: inspect raw visitor output on a simple C file |

#### 3.7 Outcomes (Sprint 3)

- 8 tests in `visitor-localtypeinfo.test.ts`, all passing
- `localTypeInfo` format confirmed: `localVariables: Map<funcName, Map<varName, TypeInfo>>`, `functionParameters: Map<funcName, TypeInfo[]>`
- Format matches `resolverUtils.lookupVariableType()` expectations — no mismatch
- Behavioral notes: visitor omits localVariables entry for functions with no locals; void-param functions get empty functionParameters array; double-pointer extracts base typename

---

### Phase 4: Visitor — Call Sites & Completeness — REMOVED

**Status:** Removed per PM decision (Sprint 3).  
**Rationale:** `apiCallSites` is a dead-code data path — populated by the visitor but not consumed by any downstream component. Removing this phase reduces complexity and prioritizes the working product.  
**Impact:** None — no tests, no source code changes. The visitor continues to populate `apiCallSites` but it is not tested or maintained.

---

### Phase 5: Navigation Index CRUD ✅ COMPLETE

**Sprint:** 3 (complete)  
**Goal:** Verify all NavigationIndex CRUD operations — creation, population, retrieval, and removal by file — across all 9 Map types.  
**Prerequisites:** Phase 4 complete.

#### 5.1 Scope

| Work Item | TC IDs | Description | Complexity |
|-----------|--------|-------------|------------|
| WI-5.1 | TC-IDX-001 | `createEmptyIndex()` — all Maps initialized, correct keys present | Low |
| WI-5.2 | TC-IDX-002 | `mergeInto()` — add records from a ParsedFile; verify all Maps populated | Medium |
| WI-5.3 | TC-IDX-003 | `clearIndex()` — all Maps empty after clear | Low |
| WI-5.4 | TC-IDX-004 | `removeFileRecords()` — removes all records associated with the given file path where supported | Medium |
| WI-5.5 | TC-IDX-005 | `removeFileRecords()` — stale entries in flat maps: `moduleRoots`, `traitRoots`, `derivationChain`, `apiStructFields`, `apiMemberRegistry` are NOT pruned (documented behavior — see M4) | Medium |
| WI-5.6 | TC-IDX-NEG-001 | Negative: `removeFileRecords()` with a file path not in the index → no-op, no error thrown | Low |
| WI-5.7 | TC-IDX-BND-001 | Boundary: `mergeInto()` with a ParsedFile containing zero records (all Maps empty) → index unchanged | Low |

> **M4 Note:** Source inspection confirms `removeFileRecords` does not prune `moduleRoots`, `traitRoots`, `derivationChain`, `apiStructFields`, or `apiMemberRegistry` on file delete. TC-IDX-005 explicitly documents and verifies this as known behavior: a file delete without re-index leaves stale entries in these 5 maps. This is not a bug fix in this phase.

#### 5.2 Test Approach

All tests use `createEmptyIndex()` directly and construct synthetic `ParsedFile` objects in-memory. No parser invocation, no file system. Tests verify Map contents by key equality.

#### 5.3 Discovery Checkpoint

After TC-IDX-002: confirm the exact Map-key format used by `mergeInto()` for each of the 9 Map types. Record these formats in lessons-learned — they determine the key format that Phases 7 and 8 must use when constructing test NavigationIndex instances.

#### 5.4 Debug Budget

**15%** — Rationale: NavigationIndex is pure data structure manipulation with no I/O. The stale-map behavior (TC-IDX-005) is documented, not fixed.

#### 5.5 Acceptance Criteria

- [ ] TC-IDX-001 through TC-IDX-005 implemented and passing
- [ ] TC-IDX-NEG-001, TC-IDX-BND-001 implemented and passing (negative/boundary — v2.1 Amendment C1)
- [ ] All 9 Map types covered by at least one test
- [ ] Stale-map behavior documented in TC-IDX-005 and in lessons-learned
- [ ] NavigationIndex Map key formats recorded in lessons-learned for use by Phases 7 and 8
- [ ] No regressions in Phases 1–4

#### 5.6 Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| `mergeInto()` format has undocumented nested Map nesting | Medium | Medium | Discovery checkpoint after TC-IDX-002; document format |
| `removeFileRecords` partially prunes some flat maps (contradicting M4) | Low | Low | TC-IDX-005 verifies current behavior; document any discrepancy |

#### 5.7 Outcomes (Sprint 3)

- 7 tests in `navigationIndex.test.ts`, all passing
- All 9 Map types covered: moduleRoots, traitRoots, derivationChain, apiStructFields, vtableAssignments, failureHandlerAssignments, apiMemberRegistry, functionDefinitions, localTypeInfo
- `removeFileRecords()` confirmed to filter file-bearing Maps and leave flat Maps untouched (M4 stale behavior)
- Unknown file path is a no-op (TC-IDX-NEG-001)

---

### Phase 6: Resolver Utilities

**Sprint:** 4  
**Goal:** Verify all four `resolverUtils` functions at unit level against synthetic NavigationIndex state.  
**Prerequisites:** Phase 5 complete (NavigationIndex Map key formats confirmed).

#### 6.1 Scope

| Work Item | TC IDs | Description | Complexity |
|-----------|--------|-------------|------------|
| WI-6.1 | TC-UTIL-001 | `findEnclosingFunction()` — cursor inside function body → correct function name | Low |
| WI-6.2 | TC-UTIL-002 | `findEnclosingFunction()` — cursor outside any function → undefined/null | Low |
| WI-6.3 | TC-UTIL-003 | `lookupVariableType()` — variable found in `localVariables` → correct type | Low |
| WI-6.4 | TC-UTIL-004 | `lookupVariableType()` — variable found in `functionParameters` (not in locals) → correct type | Low |
| WI-6.5 | TC-UTIL-005 | `walkToRootType()` — multi-hop derivation chain → correct root type; cycle detection guard | Medium |
| WI-6.6 | TC-UTIL-006 | `parseIntermediates()` — single member, multi-member, dot-accessor, empty string input | Low |
| WI-6.7 | TC-UTIL-NEG-001 | Negative: `lookupVariableType()` with unknown variable name → returns undefined | Low |
| WI-6.8 | TC-UTIL-NEG-002 | Negative: `walkToRootType()` with type not in derivation chain → returns input type unchanged | Low |
| WI-6.9 | TC-UTIL-BND-001 | Boundary: `findEnclosingFunction()` with cursor at exact function start line → correct function | Low |

#### 6.2 Test Approach

Tests in `resolverUtils.test.ts` construct a minimal NavigationIndex with pre-populated entries relevant to each utility. No parser invocation. `walkToRootType()` test uses a synthetic 3-level derivation chain and a separate input with an artificial 2-node cycle to verify cycle detection doesn't infinite-loop.

#### 6.3 Discovery Checkpoint

After TC-UTIL-003: confirm `lookupVariableType()` searches `localVariables` first, then `functionParameters`. If the search order differs from Phase 3's `localTypeInfo` format, this surfaces the contract mismatch before it reaches Phase 7.

#### 6.4 Debug Budget

**20%** — Rationale: pure-logic code with no I/O. The `walkToRootType()` cycle detection is the highest-risk element; all others are straightforward lookups.

#### 6.5 Acceptance Criteria

- [ ] TC-UTIL-001 through TC-UTIL-006 implemented and passing
- [ ] TC-UTIL-NEG-001, TC-UTIL-NEG-002, TC-UTIL-BND-001 implemented and passing (negative/boundary — v2.1 Amendment C1)
- [ ] Cycle detection in `walkToRootType()` verified not to infinite-loop
- [ ] `parseIntermediates()` verified for single, multi, dot, and empty inputs (TC-UTIL-006)
- [ ] `lookupVariableType()` search order confirmed and documented
- [ ] No regressions in Phases 1–5

#### 6.6 Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| `lookupVariableType` key format doesn't match Phase 3 `localTypeInfo` | Medium | High | Discovery checkpoint; fix visitor or utility before Phase 7 |
| `walkToRootType` cycle detection missing or broken | Low | High | TC-UTIL-005 explicitly tests cycle; budget source fix if needed |

---

### Phase 7: VtableResolver

**Sprint:** 5  
**Goal:** Verify that `VtableResolver` correctly resolves all 6 chain-walk patterns to concrete function locations using injected NavigationIndex stubs, including all 3 regex strategy boundary conditions.  
**Prerequisites:** Phases 5 and 6 complete (index format and resolver utilities verified).

#### 7.1 Scope

| Work Item | TC IDs | Description | Complexity |
|-----------|--------|-------------|------------|
| WI-7.1 | TC-RES-001 | Resolution — Category 1: indirect API pointer (`ptMod->ptApi->Field`) | Medium |
| WI-7.2 | TC-RES-002 | Resolution — Category 2: dot-accessed API (`.ptApi->Field`) | Medium |
| WI-7.3 | TC-RES-003 | Resolution — Category 3: direct API pointer | Medium |
| WI-7.4 | TC-RES-004 | Resolution — Category 4: named API member | Medium |
| WI-7.5 | TC-RES-005 | Resolution — Category 5: macro-based (`JUNO_MODULE_SUPER` in lineText — triggers field-name fallback, not any regex) | High |
| WI-7.6 | TC-RES-006 | Resolution — error path: unresolved → `found: false` with message | Low |
| WI-7.7 | TC-RES-007 | Resolution — multi-match: multiple implementations → QuickPick list | Medium |
| WI-7.8 | TC-RES-008 | `macroRe` boundary — cursor at match start (inclusive) | Low |
| WI-7.9 | TC-RES-009 | `macroRe` boundary — cursor at match end (inclusive) | Low |
| WI-7.10 | TC-RES-010 | `arrayRe` boundary — cursor one past match end (no match expected) | Low |
| WI-7.11 | TC-RES-011 | `generalRe` boundary — cursor at match start, at match end, one past end | Low |
| WI-7.12 | TC-RES-NEG-001 | Negative: line contains a vtable-like pattern but `derivationChain` has no matching root type → `found: false` (resolve attempt with index miss) | Low |

> **M1 Note:** The design document (§5.1) describes a CST-based chain-walk. The actual implementation uses 3 line-text regex strategies: `macroRe`, `arrayRe`, `generalRe`. TC-RES tests target the **regex implementation**, not the design's CST algorithm. TC-RES-005 specifically tests `JUNO_MODULE_SUPER` — this pattern is NOT handled by any of the three regexes; it triggers a field-name fallback path. TC-RES-008 through TC-RES-011 verify regex column-range boundary conditions.

#### 7.2 Test Approach

Tests in `vtableResolver.test.ts` use synthetic NavigationIndex instances (no parser, no file system). Each test constructs a NavigationIndex with `vtableAssignments` and `derivationChain` entries corresponding to the chain-walk category under test, then calls `VtableResolver.resolve()` with a synthetic `lineText` and `column` value.

#### 7.3 Discovery Checkpoint

After TC-RES-001: log `macroRe.exec(lineText)` output in the test. Confirm match indices match expected column ranges. If the column-range check `(column >= m.index && column < m.index + m[0].length)` has off-by-one, fix the source before continuing.

#### 7.4 Debug Budget

**25%** — Rationale: regex boundary conditions are the primary risk. TC-RES-008 through TC-RES-011 explicitly probe these boundaries and are expected to surface at least one off-by-one issue.

#### 7.5 Acceptance Criteria

- [ ] TC-RES-001 through TC-RES-011 implemented and passing
- [ ] All 3 regex strategies (`macroRe`, `arrayRe`, `generalRe`) have boundary tests
- [ ] `JUNO_MODULE_SUPER` field-name fallback path verified (TC-RES-005)
- [ ] Multi-match (QuickPick) path verified (TC-RES-007)
- [ ] No regressions in Phases 1–6

#### 7.6 Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Regex column-range check has off-by-one | High | High | TC-RES-008 through TC-RES-011 boundary tests; fix source |
| `JUNO_MODULE_SUPER` fallback not implemented | Low | Medium | TC-RES-005 explicitly tests it; diagnose before writing |
| `derivationChain` format mismatch from Phase 5 | Medium | High | Use exact key format confirmed in Phase 5 |

---

### Phase 8: FailureHandlerResolver

**Sprint:** 6  
**Goal:** Verify that `FailureHandlerResolver` correctly resolves failure handler assignments, handles the macro form, returns multi-match lists, and properly handles edge cases including column guards and fallthrough behavior.  
**Prerequisites:** Phase 7 complete.

#### 8.1 Scope

| Work Item | TC IDs | Description | Complexity |
|-----------|--------|-------------|------------|
| WI-8.1 | TC-FH-001 | Assignment form — `_pfcnFailureHandler = OnFailure` → resolves to OnFailure location | Medium |
| WI-8.2 | TC-FH-002 | Macro form — `JUNO_FAILURE_HANDLER(ptMod, OnFailure)` → resolves to OnFailure location | Medium |
| WI-8.3 | TC-FH-003 | Multi-match — multiple handlers for same root type → returns list | Medium |
| WI-8.4 | TC-FH-004 | Error path — no handler found → `found: false` | Low |
| WI-8.5 | TC-FH-005a | Column guard — cursor at column 0 on `_pfcnFailureHandler` assignment line (LHS variable) → document expected behavior (currently triggers resolution — no column guard) | Low |
| WI-8.6 | TC-FH-005b | Column guard — cursor within RHS function name on same line → activates resolution | Low |
| WI-8.7 | TC-FH-006 | Fallthrough — assignment regex matches but RHS function not in index → returns all handlers for root type (silent fallthrough, not an error) | Medium |
| WI-8.8 | TC-FH-NEG-001 | Negative: line contains `_pfcnFailureHandler` as a substring inside a comment → should NOT trigger resolution | Low |

> **m1 Note (column guard):** `FailureHandlerResolver` currently has no column guard — any cursor on a line with `_pfcnFailureHandler` triggers resolution regardless of column. TC-FH-005a documents current behavior at column 0 explicitly. TC-FH-005b confirms resolution activates on the RHS. The distinction defines the user-visible cursor-position contract.

> **O4 Note (fallthrough):** TC-FH-006 tests the case where the assignment regex matches but the RHS function name is not in the NavigationIndex. Expected behavior: return all known handlers for that root type (silent fallthrough). This behavior must be explicitly documented in the test.

#### 8.2 Test Approach

Tests in `failureHandlerResolver.test.ts` use synthetic NavigationIndex instances. Each test pre-populates the relevant index structure, then calls `FailureHandlerResolver.resolve()` with a synthetic line and column. For TC-FH-005a and TC-FH-005b, the same C line is used with column varied.

#### 8.3 Discovery Checkpoint

After TC-FH-001: confirm which NavigationIndex structure `FailureHandlerResolver` reads. This determines how to pre-populate the index for subsequent tests.

#### 8.4 Debug Budget

**20%** — Rationale: FailureHandlerResolver is structurally simpler than VtableResolver (one pattern, not 6). The new TC-FH-005/006 edge cases are novel but bounded in scope.

#### 8.5 Acceptance Criteria

- [x] TC-FH-001 through TC-FH-006 (including TC-FH-005a and TC-FH-005b) implemented and passing
- [x] Column 0 cursor behavior on LHS documented in test comments and lessons-learned
- [x] Fallthrough behavior documented in test comments (TC-FH-006)
- [x] No regressions in Phases 1–7

#### 8.6 Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Column guard absent causes false positives for users | Medium | Low | TC-FH-005a documents current behavior; escalate to PM if guard is needed |
| Fallthrough silently swallows real indexing errors | Low | Medium | TC-FH-006 verifies expected fallthrough output is non-empty for known root types |

#### 8.7 Outcomes (Sprint 5)

- 13 tests in `failureHandlerResolver.test.ts`, all passing
- All test cases from SDP §8.1 (TC-FH-001 through TC-FH-006, TC-FH-NEG-001) implemented
- Additional coverage tests added per verifier feedback: TC-FH-007 (PRIMARY_VAR_RE branch), TC-FH-NEG-002 (Step 0 early exit), TC-FH-NEG-003 (typeInfo undefined), TC-FH-BND-001 (multi-hop derivation chain in Step 2)
- Column guard behavior documented: no column guard exists — any cursor on a handler line triggers resolution (TC-FH-005a/b)
- Fallthrough behavior documented: assignment regex match with unindexed RHS → returns all handlers for root type (TC-FH-006)
- Comment false positive documented: presence regex matches inside comments, but both resolution steps fail gracefully (TC-FH-NEG-001)
- No source bugs found — all code paths behave as expected

---

### Phase 9: Visitor → Index → Resolver Integration ✅ COMPLETE

**Sprint:** 6 (complete)  
**Goal:** Prove the full data pipeline from real visitor output → WorkspaceIndexer mergeInto → NavigationIndex → VtableResolver end-to-end — no stubs.  
**Prerequisites:** Phases 5, 6, 7, and 8 complete; `localTypeInfo` format confirmed in Phase 3.

#### 9.1 Scope

| Work Item | TC IDs | Description | Complexity |
|-----------|--------|-------------|------------|
| WI-9.1 | TC-INT-001 | Parse a known C source with the real visitor; inject resulting `localTypeInfo` into VtableResolver; verify resolution returns the expected concrete location | High |
| WI-9.2 | TC-INT-002 | Parse C source across two in-memory sources; merge both into NavigationIndex; verify full chain resolution end-to-end | High |
| WI-9.3 | TC-INT-003 | Parse C source containing a failure handler assignment; merge via real `WorkspaceIndexer.mergeInto()`; verify `FailureHandlerResolver.resolve()` returns expected concrete location | High |
| WI-9.4 | TC-INT-004 | Multi-hop derivation — parse C source with 3-level derivation chain (root → derived → leaf); verify VtableResolver resolves through all hops to the concrete function | High |
| WI-9.5 | TC-INT-005 | Two modules in same file — parse a single C source defining two independent modules; merge into index; verify both modules resolve independently without cross-contamination | Medium |
| WI-9.6 | TC-INT-006 | No vtable patterns — parse a C source with no vtable init patterns (plain functions only); verify resolver returns `found: false` without error | Low |

> **Scope change (v2.1 — Amendment B2):** Integration tests expanded from 3 to 6. TC-INT-004 validates the multi-hop `walkToRootType` path end-to-end (previously only unit-tested in Phase 6). TC-INT-005 catches index key collision bugs when multiple modules share a file. TC-INT-006 confirms graceful degradation on non-LibJuno C code.

#### 9.2 Test Approach

Tests in `integration.test.ts` (at `src/__tests__/`) follow the "test like you fly" principle (PM decision, Sprint 6): synthetic C source files are written to a temporary directory on disk, indexed through the real `WorkspaceIndexer.reindexFile()` public API (disk I/O → Chevrotain parse → mergeInto), and resolved through real `VtableResolver` / `FailureHandlerResolver` instances. No mocks, no manual index population. See Sprint_6.md section 6 for design rationale.

This phase bridges the Phase 2–3 contract gap: if `localTypeInfo` Map key format doesn't match what the resolver expects, it surfaces here before the higher-level phases.

#### 9.3 Discovery Checkpoint

After TC-INT-001: log the full `NavigationIndex` state after mergeInto. Confirm all required Maps are populated and key formats match what Phase 7's TC-RES tests used. If a format mismatch is found, fix the visitor or resolver before writing TC-INT-002.

#### 9.4 Debug Budget

**40%** — Rationale: this is the highest-risk phase in the project. It is the first test that exercises the full data pipeline without any stubs. Format mismatches between visitor output and resolver expectations will all surface here.

#### 9.5 Acceptance Criteria

- [x] TC-INT-001 implemented and passing (end-to-end single-file VtableResolver resolution)
- [x] TC-INT-002 implemented and passing (end-to-end two-source chain resolution)
- [x] TC-INT-003 implemented and passing (end-to-end FailureHandlerResolver pipeline)
- [x] TC-INT-004 implemented and passing (multi-hop derivation chain resolution)
- [x] TC-INT-005 implemented and passing (two modules in same file, no cross-contamination)
- [x] TC-INT-006 implemented and passing (no vtable patterns → `found: false`)
- [x] Any format mismatches between Phase 3 visitor output and Phase 7/8 resolver input found and fixed
- [x] Navigation pipeline data path documented in lessons-learned as a reference
- [x] No regressions in Phases 1–8

#### 9.6 Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| `localTypeInfo` format mismatch between visitor and resolver | High | High | Phase 3 discovery checkpoint mitigates; Phase 9 catches any remainder |
| `mergeInto` doesn't populate all Maps correctly | Medium | High | TC-IDX-002 (Phase 5) partially covered; TC-INT-001 is the full smoke |
| Parser grammar doesn't handle the chosen synthetic C patterns | Low | Medium | Use C patterns already tested in Phases 1–4 |

---

### Phase 10: CacheManager

**Sprint:** 8  
**Goal:** Verify that the CacheManager correctly serializes and deserializes the NavigationIndex to/from JSON, handles all failure modes gracefully, and writes atomically.  
**Prerequisites:** Phase 9 complete (NavigationIndex format confirmed end-to-end).

#### 10.1 Scope

| Work Item | TC IDs | Description | Complexity |
|-----------|--------|-------------|------------|
| WI-10.1 | TC-CACHE-001 | `indexToCache → JSON.stringify → JSON.parse → cacheToIndex` roundtrip preserves all 9 Map types | High |
| WI-10.2 | TC-CACHE-002 | `loadCache` — cache version mismatch → returns null (triggers full re-index) | Low |
| WI-10.3 | TC-CACHE-003 | `loadCache` — stale file (file mtime newer than cache) → affected file flagged for re-index | Medium |
| WI-10.4 | TC-CACHE-004 | `loadCache` — new file added to workspace (not in cache) → triggers full index | Medium |
| WI-10.5 | TC-CACHE-005 | `loadCache` — deleted file (in cache but not on disk) → delete-without-reindex: stale entries remain in `moduleRoots`, `traitRoots`, `derivationChain`, `apiStructFields`, `apiMemberRegistry` (documented — see M4) | Medium |
| WI-10.6 | TC-CACHE-006 | `saveCache` — writes successfully to output path | Low |
| WI-10.7 | TC-CACHE-008 | Debounced write — rapid file-save events → single cache write emitted | Medium |
| WI-10.8a | TC-CACHE-009 | Atomic write — writes to temp file first, then renames; atomicity validated for single-caller (concurrent writes prevented by debouncing per TC-CACHE-008) | Low |
| WI-10.9 | TC-CACHE-010 | Version matches but `vtableAssignments` field is null or wrong type → `loadCache` or `cacheToIndex` must not throw; falls back to full re-index | Medium |
| WI-10.10 | TC-CACHE-NEG-001 | Negative: `loadCache` from non-existent file path → returns null gracefully, no throw | Low |
| WI-10.11 | TC-CACHE-BND-001 | Boundary: roundtrip with an index containing one Map type populated and all others empty → all Maps preserved correctly | Low |

> **M4 Note (TC-CACHE-005):** Maps NOT pruned on file delete: `moduleRoots`, `traitRoots`, `derivationChain`, `apiStructFields`, `apiMemberRegistry`. Verified as known limitation.

> **M8 Note (TC-CACHE-010):** When cache version matches but structured fields are corrupted, the system must not throw. Fallback to full re-index is required.

> **m3 Note (TC-CACHE-009):** Atomicity validated for single-caller only. Concurrent-write safety provided by debouncing (TC-CACHE-008).

#### 10.2 Test Approach

`cacheManager.test.ts` injects a file-system abstraction (stubs for `fs.readFileSync`, `fs.writeFileSync`, `fs.renameSync`, `fs.statSync`) to avoid real disk I/O. The roundtrip test (TC-CACHE-001) uses an in-memory NavigationIndex populated with data in all 9 Map types. Corruption tests construct malformed JSON/object payloads and verify the fallback.

#### 10.3 Discovery Checkpoint

After TC-CACHE-001: confirm all 9 Map types survive the roundtrip. Nested Maps (`vtableAssignments: Map<string, Map<string, ConcreteLocation[]>>`) are the highest risk. If any Map type is lost, fix `indexToCache`/`cacheToIndex` before continuing.

#### 10.4 Debug Budget

**25%** — Rationale: Map-to-Object serialization with 9 types including nested Maps, plus corruption handling. File system mocking adds infrastructure risk.

#### 10.5 Acceptance Criteria

- [ ] TC-CACHE-001 through TC-CACHE-010 implemented and passing
- [ ] All 9 Map types verified to survive roundtrip serialization
- [ ] Stale flat-map entries documented in TC-CACHE-005 (aligned with TC-IDX-005)
- [ ] Cache corruption (TC-CACHE-010) handled without uncaught exception
- [ ] Atomic write verified via temp-file rename (TC-CACHE-009)
- [ ] No regressions in Phases 1–9

#### 10.6 Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Nested Map serialization loses inner Map data | High | High | TC-CACHE-001 catches this; fix `indexToCache` before continuing |
| `cacheToIndex` throws on corrupted data | High | High | TC-CACHE-010 catches this; add null checks to source |
| File system mock fragility | Medium | Medium | Keep mock minimal; use thin abstraction |

---

### Phase 11: WorkspaceIndexer Core

**Sprint:** 9  
**Goal:** Verify that `WorkspaceIndexer` correctly performs full indexing, incremental re-indexing, cache loading, and file removal against synthetic file system content.  
**Prerequisites:** Phase 10 complete (CacheManager verified).

#### 11.1 Scope

| Work Item | TC IDs | Description | Complexity |
|-----------|--------|-------------|------------|
| WI-11.1 | TC-WI-001 | `fullIndex()` — indexes 2+ synthetic C files; NavigationIndex populated correctly | High |
| WI-11.2 | TC-WI-002 | `loadFromCache()` — cache present and valid → uses cache, skips re-parse | Medium |
| WI-11.3 | TC-WI-003 | `loadFromCache()` — cache absent or invalid → falls back to `fullIndex()` | Medium |
| WI-11.4 | TC-WI-004 | `reindexFile()` — single file changed → only that file re-parsed | Medium |
| WI-11.4a | TC-WI-004a | FileSystemWatcher change event → triggers `reindexFile()` for changed file (moved from Phase 10 — see Finding 2) | Medium |
| WI-11.5 | TC-WI-005 | `removeFile()` — file deleted → records removed from index | Low |
| WI-11.6 | TC-WI-006 | `mergeInto()` multi-file — records from 2 files merged correctly without key collision | Medium |
| WI-11.7 | TC-WI-NEG-001 | Negative: `reindexFile()` on a file that was never indexed → indexes as new, no error thrown | Low |
| WI-11.8 | TC-WI-BND-001 | Boundary: `fullIndex()` on an empty workspace (no C files) → empty index, no error | Low |

#### 11.2 Test Approach

Tests in `workspaceIndexer.test.ts` use either a temporary directory or a stubbed `fs` module. Spy on `parseFileWithDefs()` to verify which files are parsed. Pre-populate a CacheManager stub to test the cache-hit path without real disk I/O.

#### 11.3 Discovery Checkpoint

After TC-WI-001: inspect the NavigationIndex state after indexing 2 files. Compare against the integration test (Phase 9) format. If the indexer's `mergeInto()` produces a different format than the direct-visitor test, surface the discrepancy here.

#### 11.4 Debug Budget

**25%** — Rationale: file system interaction (even stubbed) adds infrastructure risk. The `fullIndex` → `mergeInto` path exercises all prior phases together.

#### 11.5 Acceptance Criteria

- [ ] TC-WI-001 through TC-WI-006 implemented and passing
- [ ] TC-WI-NEG-001, TC-WI-BND-001 implemented and passing (negative/boundary — v2.1 Amendment C1)
- [ ] `parseFileWithDefs()` spy confirms only changed files are re-parsed in incremental mode
- [ ] No regressions in Phases 1–10

#### 11.6 Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Multi-file merge causes Map key collision | Medium | Medium | TC-WI-006 explicitly tests; fix `mergeInto` if collision detected |
| FileSystemWatcher mock overly complex | Medium | Medium | Defer watcher-specific tests to Phase 12 if needed |

---

### Phase 12: File Discovery & Deferred Resolution

**Sprint:** 9  
**Goal:** Verify that the workspace file scanner discovers all required file extensions, and that the deferred positional vtable initializer cross-file resolution works correctly (including the multi-module failure handler heuristic).  
**Prerequisites:** Phase 11 complete.

#### 12.1 Scope

| Work Item | TC IDs | Description | Complexity |
|-----------|--------|-------------|------------|
| WI-12.1 | TC-FILE-001 | File scanner discovers files with all 6 mandatory extensions: `.c`, `.h`, `.cpp`, `.hpp`, `.hh`, `.cc` | Low |
| WI-12.2 | TC-WI-007 | Deferred positional initializer — API struct in file A, positional initializer in file B → resolved after both files indexed | High |
| WI-12.3 | TC-WI-008 | Multi-module failure handler — synthetic C file with two `Init` functions assigning failure handlers to different module roots → both stored under correct root type keys | High |

> **M5 Note (TC-FILE-001):** Verifies REQ-VSCODE-021. Previously listed as "covered by extension.ts file-type glob" without a proper TC ID.

> **m8 Note (TC-WI-008):** Tests `resolveFailureHandlerRootType()` disambiguation across multiple module roots.

#### 12.2 Test Approach

TC-FILE-001 uses a temporary directory with one file per extension type; verifies `scanFiles()` returns all 6. TC-WI-007 uses synthetic string sources for both files. TC-WI-008 constructs a synthetic two-Init C source and verifies the resulting index maps each handler to its correct root type.

#### 12.3 Discovery Checkpoint

After TC-WI-007's first assertion (after file A indexed): dump the deferred queue state to confirm the deferred entry is held. Verify that after file B is merged, the deferred entry resolves to the expected location.

#### 12.4 Debug Budget

**35%** — Rationale: cross-file deferred resolution is the highest-risk WorkspaceIndexer path. TC-WI-008 has never been tested.

#### 12.5 Acceptance Criteria

- [ ] TC-FILE-001 implemented and passing (all 6 file extensions discovered)
- [ ] TC-WI-007 implemented and passing (deferred positional resolves cross-file)
- [ ] TC-WI-008 implemented and passing (multi-module FH root type disambiguation)
- [ ] No regressions in Phases 1–11

#### 12.6 Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Deferred positional queue ordering sensitive to indexing order | High | High | TC-WI-007 tests both orderings |
| `resolveFailureHandlerRootType()` assigns both handlers to same root | Medium | High | TC-WI-008 explicitly tests; fix source if incorrect |
| File extension glob doesn't include `.hh` or `.cc` | Low | Medium | TC-FILE-001 catches this |

---

### Phase 13: MCP Server

**Sprint:** 10  
**Goal:** Verify the embedded HTTP MCP server: start/stop lifecycle, all 3 endpoints, error handling, security (127.0.0.1-only binding), and port cleanup.  
**Prerequisites:** Phase 12 complete; **WI-13.0 source change required** before any tests can be written.

**M3 Source Change Prerequisite:**
Before writing any TC-MCP-\* tests, modify `mcpServer.ts` so the bound port is observable from test code. Recommended: change `start()` return type from `void` to `Promise<number>`. This is required work item WI-13.0.

#### 13.1 Scope

| Work Item | TC IDs | Description | Complexity |
|-----------|--------|-------------|------------|
| WI-13.0 | — | **Source change (prerequisite):** expose bound port from `start()` return value or `getPort()` | Low |
| WI-13.1 | TC-MCP-001 | Server starts on port 0 (OS-assigned), `start()` returns actual bound port | Low |
| WI-13.2 | TC-MCP-002 | Endpoint `/resolve` — valid request → correct resolution response | High |
| WI-13.3 | TC-MCP-003 | Endpoint `/resolve` — unresolved → `found: false` JSON response | Medium |
| WI-13.4 | TC-MCP-004 | Endpoint `/schema` — returns valid JSON schema | Low |
| WI-13.5 | TC-MCP-005 | Endpoint 404 — unknown path → 404 with error body | Low |
| WI-13.6 | TC-MCP-006 | Endpoint `/resolve` — malformed JSON body → 400 status | Low |
| WI-13.7 | TC-MCP-007 | Security: server bound to 127.0.0.1 only (not 0.0.0.0) | Medium |
| WI-13.8 | TC-MCP-008 | Headless mode: server starts without VSCode dependency | Low |
| WI-13.9 | TC-MCP-009 | `/resolve` with `file`, `line`, `column` parameters — all three consumed correctly | Medium |
| WI-13.10 | TC-MCP-010 | `/resolve` — multi-match result (array of locations returned) | Medium |
| WI-13.11 | TC-MCP-011 | `/resolve` — schema-only request (no resolution invoked) | Low |
| WI-13.12 | TC-MCP-012 | Large request body — server handles gracefully without memory error | Low |
| WI-13.13 | TC-MCP-013 | Start then stop server — port released; re-binding the same port succeeds without EADDRINUSE | Medium |
| WI-13.14 | TC-MCP-014 | `file` path outside workspace root → resolver returns `found: false` without filesystem access | Medium |

> **m7 Note (TC-MCP-013):** After `server.stop()`, test re-binds on the same port to verify release.

> **O3 Note (TC-MCP-014):** Path traversal guard: a `file` parameter escaping the workspace root must not trigger filesystem access.

#### 13.2 Test Approach

Tests in `mcpServer.test.ts` start the server in-process on port 0, send HTTP requests via Node.js `http.request`, and verify JSON responses. No VSCode dependency. Each test uses a fresh server instance; `afterEach` calls `stop()`. TC-MCP-013 re-binds after stop.

#### 13.3 Discovery Checkpoint

After TC-MCP-001 and TC-MCP-007: confirm server binds to `127.0.0.1` and port is returned. If WI-13.0 isn't complete, no other tests can proceed.

#### 13.4 Debug Budget

**20%** — Rationale: pure HTTP server. Source change is small. Port management is main risk.

#### 13.5 Acceptance Criteria

- [ ] WI-13.0 complete: `start()` exposes bound port
- [ ] TC-MCP-001 through TC-MCP-014 implemented and passing
- [ ] Server confirmed to bind to `127.0.0.1` only (TC-MCP-007)
- [ ] Port release after stop verified (TC-MCP-013)
- [ ] Workspace path guard verified (TC-MCP-014)
- [ ] No regressions in Phases 1–12

#### 13.6 Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| `EADDRINUSE` between test runs due to port leak | Medium | Medium | Use port 0; `afterEach` calls `stop()` |
| `start()` return type change breaks existing call sites | Low | Low | Check all call sites before modifying |
| Path traversal in `file` parameter not guarded | Medium | High | TC-MCP-014 catches this; add workspace-root guard |

---

### Phase 14: VSCode Mocks & Definition Provider

**Sprint:** 11  
**Goal:** Build the shared VSCode API mock and verify the `JunoDefinitionProvider` bridge, including fall-through logic and string-coupling behavior.  
**Prerequisites:** Phases 7 and 8 complete (resolvers verified). Phase 14 can proceed in parallel with Phase 13 — they are architecturally independent (`JunoDefinitionProvider` does not call any MCP API; `McpServer` does not call any VSCode API).

#### 14.1 Scope

| Work Item | TC IDs | Description | Complexity |
|-----------|--------|-------------|------------|
| WI-14.0 | — | Build `__mocks__/vscode.ts` — stub all required VSCode APIs | High |
| WI-14.1 | TC-VSC-001 | Extension `activate()` completes without throwing with mock | Medium |
| WI-14.2 | TC-VSC-002 | DefinitionProvider registered with `vscode.languages.registerDefinitionProvider` on activation | Low |
| WI-14.3 | TC-VSC-003 | `provideDefinition()` — matched vtable call → navigates to implementation | High |
| WI-14.4 | TC-VSC-004 | `provideDefinition()` — matched failure handler → navigates | High |
| WI-14.5 | TC-VSC-005 | `provideDefinition()` — no match → returns undefined | Low |
| WI-14.6 | TC-VSC-006 | FileSystemWatcher registered on activation → change events trigger re-index | Medium |
| WI-14.7 | TC-VSC-007 | All expected commands registered on activation | Low |
| WI-14.8 | TC-VSC-008 | Both resolvers return `found: false` → provider surfaces correct status bar message (string coupling test) | Medium |
| WI-14.9 | TC-VSC-NEG-001 | Negative: `provideDefinition()` called on a non-C file (e.g., `.txt`) → returns undefined, no resolver invoked | Low |
| WI-14.10 | TC-VSC-BND-001 | Boundary: `provideDefinition()` at line 0, column 0 → does not throw, returns undefined or valid result | Low |

> **m2 Note (TC-VSC-008):** Catches string drift between `junoDefinitionProvider.ts` hard-coded strings and resolver error messages.

#### 14.2 Test Approach

`junoDefinitionProvider.test.ts` imports `__mocks__/vscode.ts` via Jest module mapping. Tests call `activate()` with a mock `ExtensionContext`, then invoke `provideDefinition()` with synthetic inputs. Resolvers are injected or replaced by Jest spies.

**VSCode mock stubs required:** `vscode.languages.registerDefinitionProvider` (spy), `vscode.commands.registerCommand` (spy), `vscode.window.showQuickPick` (configurable), `vscode.window.showTextDocument` (spy), `vscode.window.createStatusBarItem` (stub), `vscode.window.setStatusBarMessage` (spy), `vscode.workspace.createFileSystemWatcher` (stub), `vscode.workspace.workspaceFolders` (configurable), `vscode.Uri.file` (identity transform).

#### 14.3 Discovery Checkpoint

After WI-14.0 and TC-VSC-001: confirm `activate()` runs without throwing. Iterate on mock until activation is clean before writing further tests.

#### 14.4 Debug Budget

**35%** — Rationale: VSCode API mocking is the highest-risk infrastructure work. First-time mock setup always requires iteration.

#### 14.5 Acceptance Criteria

- [x] `__mocks__/vscode.ts` implemented and loadable by Jest
- [x] TC-VSC-001 through TC-VSC-008 implemented and passing
- [x] TC-VSC-NEG-001, TC-VSC-BND-001 implemented and passing (negative/boundary — v2.1 Amendment C1)
- [x] Provider string-coupling test passing (TC-VSC-008)
- [x] No regressions in Phases 1–13

#### 14.6 Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Mock misses a VSCode API used by `activate()` | High | High | Iterate on mock until TC-VSC-001 passes |
| Resolver injection point not accessible for test spying | Medium | Medium | May require refactoring provider constructor for DI |

#### 14.7 Outcomes (Sprint 11)

- 10 tests in `junoDefinitionProvider.test.ts`, all passing
- `__mocks__/vscode.ts` mock covers all 17 VSCode API surfaces used by the extension
- `McpServer` mocked via `jest.mock()` to prevent port binding during activation tests
- No source bugs found — JunoDefinitionProvider, extension.ts, and all providers behave as designed
- Resolver DI confirmed accessible — `JunoDefinitionProvider` constructor accepts resolvers directly; `jest.spyOn` on resolve methods works cleanly

---

### Phase 15: Error UX, QuickPick & StatusBar

**Sprint:** 12  
**Goal:** Verify the user-facing error UX (status bar messages, auto-clear), QuickPick presentation, and status bar helper behavior.  
**Prerequisites:** Phase 14 complete (VSCode mock established).

#### 15.1 Scope

| Work Item | TC IDs | Description | Complexity |
|-----------|--------|-------------|------------|
| WI-15.1 | TC-ERR-001 | Single failure → status bar message shown, auto-clears after timeout | Medium |
| WI-15.2 | TC-ERR-002 | Repeated failures → each shows message (no silent suppression) | Low |
| WI-15.3 | TC-ERR-003 | No modal dialog shown on failure | Low |
| WI-15.4 | TC-ERR-004 | Error message cleared after successful resolution | Low |
| WI-15.5 | TC-ERR-005 | Concurrent resolution requests handled gracefully | Medium |
| WI-15.6 | TC-ERR-006 | Maps to REQ-VSCODE-004 AND REQ-VSCODE-013 (dual traceability) | Low |
| WI-15.7 | TC-QP-001 | QuickPick shown with correct labels for multiple implementations | Medium |
| WI-15.8 | TC-QP-002 | QuickPick item descriptions contain correct file + line | Low |
| WI-15.9 | TC-QP-003 | QuickPick selection navigates to correct location | Medium |
| WI-15.10 | TC-QP-004 | QuickPick cancel → no navigation | Low |
| WI-15.11 | TC-QP-005 | QuickPick with single item → navigates directly without picker | Low |

#### 15.2 Test Approach

Tests in `statusBarHelper.test.ts` and `quickPickHelper.test.ts` reuse the Phase 14 VSCode mock. Auto-clear tests use `jest.useFakeTimers()` and `jest.advanceTimersByTime()`. QuickPick tests configure the `showQuickPick` spy return value.

#### 15.3 Discovery Checkpoint

After TC-ERR-001: confirm auto-clear timer is observable via `jest.useFakeTimers()`. If `setTimeout` is not intercepted, use `jest.runAllTimers()` as fallback.

#### 15.4 Debug Budget

**20%** — Rationale: thin wrappers over VSCode APIs; Phase 14 mock ready.

#### 15.5 Acceptance Criteria

- [ ] TC-ERR-001 through TC-ERR-006 implemented and passing
- [ ] TC-QP-001 through TC-QP-005 implemented and passing
- [ ] Auto-clear timer behavior verified via fake timers
- [ ] No regressions in Phases 1–14

#### 15.6 Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| `setTimeout` not intercepted by `jest.useFakeTimers()` | Low | Medium | Use `jest.runAllTimers()` as fallback |
| QuickPick item format doesn't match expectation | Low | Low | Inspect `showQuickPick` call args in TC-QP-001 first |

---

### Phase 16: End-to-End Smoke & Final Quality

**Sprint:** 13  
**Goal:** Run capstone smoke tests exercising the full extension stack with a real LibJuno C file and close all quality gates.  
**Prerequisites:** All Phases 1–15 complete.

#### 16.1 Scope

| Work Item | TC IDs | Description | Complexity |
|-----------|--------|-------------|------------|
| WI-16.1 | — | Run full test suite (all phases); confirm 0 failures | Low |
| WI-16.2a | — | End-to-end smoke #1: index a real LibJuno C file with vtable init patterns (e.g., `src/juno_time.c`); trigger `provideDefinition()`; verify navigation | High |
| WI-16.2b | — | End-to-end smoke #2: index a real LibJuno C file with failure handler assignments; verify FH resolution navigates correctly | High |
| WI-16.2c | — | End-to-end smoke #3: index a real LibJuno header-only file (e.g., a module API header); verify struct/function extraction without resolution (no vtable patterns) → graceful `found: false` | Medium |
| WI-16.3 | — | Cache smoke: save cache → clear index → load cache → verify restored | Medium |
| WI-16.4 | — | Jest coverage gate: `jest --coverage` meets ≥90% line coverage and ≥85% branch coverage on all production source files | Medium |
| WI-16.5 | — | TypeScript strict mode: `tsc --noEmit` exits clean | Low |
| WI-16.6 | — | Requirements-traceability coverage gate: verify 100% of requirements in `requirements.json` have at least one linked test case (`// @{"verify": [...]}` tag), and every test file has valid traceability tags linking back to requirement IDs. Generate a traceability matrix report. | Medium |
| WI-16.7 | — | Final quality engineer review: documentation up to date, lessons-learned complete | Low |

> **Scope change (v2.1 — Amendment B3):** E2E smoke expanded from 1 real file to 3 real LibJuno files covering the three main code patterns: vtable init, failure handler assignment, and header-only. This ensures the parser handles production code diversity, not just synthetic test patterns.

> **Scope change (v2.1 — Amendment A1):** Coverage gate raised from ≥80% line coverage to ≥90% line coverage and ≥85% branch coverage.

#### 16.2 Test Approach

WI-16.2a/b/c each use a real LibJuno source file from the repository. WI-16.2a selects a file with vtable init patterns (e.g., `src/juno_time.c`), WI-16.2b selects a file with failure handler assignments, and WI-16.2c selects a header-only file. Each follows the full path: `parseFileWithDefs()` → index → resolve (or verify graceful `found: false`) → verify `ConcreteLocation`. WI-16.3 calls `saveCache()` on a populated index, `clearIndex()`, then `loadCache()`, verifying the restored index matches. WI-16.6 performs requirements-traceability coverage analysis: enumerates all requirements from `requirements.json`, verifies each has at least one test case with a valid `// @{"verify": [...]}` tag, checks that all test `verify` tags reference valid requirement IDs, and generates a traceability matrix report.

#### 16.3 Debug Budget

**20%** — Rationale: all components individually verified. Bugs here indicate seam issues not caught by Phase 9.

#### 16.4 Acceptance Criteria

- [ ] Full test suite: 0 failures across all 16 phases
- [ ] End-to-end smoke passes on 3 real LibJuno C files (vtable init, failure handler, header-only)
- [ ] Cache roundtrip smoke passes
- [ ] Jest line coverage ≥90% on production source files
- [ ] Jest branch coverage ≥85% on production source files
- [ ] Requirements-traceability coverage: 100% of requirements have at least one linked test case (v2.1 Amendment A3-revised)
- [ ] `tsc --noEmit` exits clean
- [ ] All documentation (design.md, test-cases.md, this plan) up to date
- [ ] All 21 requirements have at least one test case
- [ ] Extension loads and resolves a real vtable call in VSCode

#### 16.5 Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Real LibJuno C file exposes grammar bug | Medium | High | Select file types already exercised in Phases 1–4; 3 files increase detection probability |
| Coverage below 90% line / 85% branch threshold | Medium | Medium | Per-sprint coverage checkpoints (starting Sprint 2) flag modules early; add targeted tests |
| Requirements-traceability gap (requirement with no linked test) | Medium | High | Per-sprint traceability audit catches gaps early; fix before final sprint |
| `tsc --noEmit` fails on source changes from earlier phases | Low | Medium | Run `tsc --noEmit` after each phase's source fixes |

---

## 5. Sprint Schedule

### Sprint Cadence

Each sprint represents one orchestration cycle: plan → delegate → verify → fix → gate. Sprint length is adaptive based on work item complexity (not fixed calendar time).

### Schedule

| Sprint | Phase(s) | Focus | Key Deliverables | Debug Budget |
|--------|---------|-------|-----------------|-------------|
| 1 | Phase 1 ✅ | Parser Foundation | 4 test files, 98 tests, 3 bug fixes | 40% (actual) |
| 2 | Phase 2 ✅ | Visitor: Vtable Init Patterns | TC-P6/P7/P8 tests; TC-P10/P11 gap fill | 30% |
| 3 | Phases 3+5 ✅ | Visitor: Local Type Info + Navigation Index CRUD | TC-LTI-001–005; TC-IDX-001–005 | 35% |
| 4 | Phase 6 | Resolver Utilities | TC-UTIL-001–006 | 20% |
| 5 | Phase 7 | VtableResolver | TC-RES-001–011; regex boundary tests | 25% |
| 6 | Phase 8 | FailureHandlerResolver | TC-FH-001–006 including column guard and fallthrough edge cases | 20% |
| 7 | Phase 9 | Integration Seam | TC-INT-001–006; full pipeline smoke (no stubs) | 40% |
| 7 | Phase 10 ✅ | CacheManager | TC-CACHE-001, 002, 006, 009, 010, NEG-001, BND-001; 1 source fix | 25% |
| 9 | Phases 11+12 | WorkspaceIndexer Core + File Discovery | TC-WI-001–008; TC-FILE-001 | 25–35% |
| 10 | Phase 13 | MCP Server | TC-MCP-001–014; WI-13.0 source change | 20% |
| 11 | Phase 14 | VSCode Mocks & Definition Provider | TC-VSC-001–008; `__mocks__/vscode.ts` | 35% |
| 12 | Phase 15 | Error UX, QuickPick & StatusBar | TC-ERR-001–006; TC-QP-001–005 | 20% |
| 13 | Phase 16 | End-to-End Smoke & Final Quality | Full suite; coverage gate; `tsc --noEmit`; real C file smoke | 20% |

**Total: 13 sprints, 15 active phases (16 numbered; Phase 4 removed)**

### Sprint Entry Criteria

Before starting any sprint:
1. Previous sprint's full test suite passes (0 failures)
2. All source bugs from previous sprint are fixed and verified
3. Lessons learned from previous sprint are documented in `ai/memory/lessons-learned-software-developer.md`
4. PM has approved the sprint plan

### Sprint Exit Criteria

A sprint is complete when:
1. All planned test cases are implemented
2. All tests pass (including all prior sprints' tests)
3. Any source bugs discovered are fixed, verified, and documented
4. ESLint/TSLint clean (when configured)
5. **Requirements-traceability HARD gate (v2.1 Amendment A3-revised):** Starting Sprint 2, every requirement in `requirements.json` that has test cases written during or before the current sprint must have at least one test with a valid `// @{"verify": ["REQ-..."]}'` tag. Every `verify` tag in test files must reference a valid requirement ID. Sprint CANNOT exit until this passes. This ensures test adequacy — tests that pass but don't trace to requirements provide no verifiable coverage.
6. **Coverage checkpoint (v2.1 Amendment A2):** Starting Sprint 2, no production source module with tests below 85% line coverage. Flagged modules must be addressed or have a documented remediation plan.
7. Final quality engineer has approved the sprint output
8. PM has approved the sprint deliverables

---

## 6. Debugging & Discovery Strategy

### 6.1 Lessons from Sprint 1

Sprint 1 consumed approximately **40% of effort on debugging**, significantly higher than initially planned. Three key lessons emerged:

1. **Bugs cascade upward.** A parser bug (Bug B) caused all struct extraction to fail, which blocked all vtable tests and all function tests. Testing bottom-up (lexer → parser → visitor) was the right strategy but the initial plan tried to parallelize test writing before verifying the lower layers.

2. **Runtime behavior diverges from design assumptions.** The Chevrotain CST key naming convention (`CONSUME2(Token)` stores under key `"Token"` not `"Token2"`) was not documented and caused Bug A. Only empirical verification (dumping actual CST output) revealed the issue.

3. **Hidden bugs appear after fixing earlier bugs.** Bug C was invisible until Bugs A and B were fixed. The failing tests after A+B were initially assumed to be test expectation issues, but diagnostic analysis revealed a third source bug.

### 6.2 Debugging Process

Every sprint follows this diagnosis-first pattern:

```
1. Write tests for the simplest case first
2. If tests fail:
   a. Diagnose: is it a test expectation issue or a source bug?
   b. If source bug: fix source → verify fix → rewrite test if needed
   c. If test expectation: fix test → re-run
3. Checkpoint: run all tests before writing more
4. Write next batch of tests
5. Repeat
```

The key discipline is: **never batch test-writing without intermediate verification.** Write 3–5 tests, run them, diagnose any failures, fix them, then write the next 3–5.

### 6.3 Discovery Checkpoints

| Phase | Checkpoint | What to Inspect | When |
|-------|-----------|----------------|------|
| 2 | After TC-P8-001 | `vtableAssignments` and `apiStructFields` field zip order | After first positional init test |
| 3 | After TC-LTI-001 | `localTypeInfo` Map nesting and key format | After first localTypeInfo test |
| 4 | After TC-P9-001/002 | `apiCallSites` structure and token content | After first call site tests |
| 5 | After TC-IDX-002 | NavigationIndex Map key formats for all 9 Map types | After first mergeInto test |
| 6 | After TC-UTIL-003 | `lookupVariableType` search order matches Phase 3 format | After first lookup test |
| 7 | After TC-RES-001 | Regex match output and column range check | After first resolver test |
| 8 | After TC-FH-001 | Which NavigationIndex structure FHResolver reads | After first FH test |
| 9 | After TC-INT-001 | Full NavigationIndex state post-mergeInto; format matches Phase 7 stubs | After first integration test |
| 10 | After TC-CACHE-001 | All 9 Map types survive roundtrip | After first cache test |
| 11 | After TC-WI-001 | NavigationIndex state after fullIndex; compare to Phase 9 format | After first indexer test |
| 12 | Midway TC-WI-007 | Deferred queue state after file A indexed, before file B | After first half of deferred test |
| 13 | After TC-MCP-001/007 | Bound port returned; binding address is 127.0.0.1 | After first MCP tests |
| 14 | After WI-14.0 + TC-VSC-001 | `activate()` runs without throwing under mock | After mock is built |
| 15 | After TC-ERR-001 | Auto-clear timer intercepted by fake timers correctly | After first status bar test |

### 6.4 Debug Budget Summary

| Sprint | Phase(s) | Budget | Rationale |
|--------|---------|--------|-----------|
| 1 | 1 | 40% (actual) | Undiscovered parser bugs; Chevrotain internals unknown |
| 2 | 2 | 30% | Positional init zip untested; calibrated from Sprint 1 actuals |
| 3 | 3 | 35% | `localTypeInfo` is highest-probability bug source; entire resolver depends on it |
| 4 | 4 | 20% | Representative sample only; visitor traversal well-understood after Phase 3 |
| 5 | 5+6 | 15–20% | Pure data structure and logic; no I/O |
| 6 | 7 | 25% | Regex boundary conditions are primary risk |
| 7 | 8 | 20% | Structurally simpler than VtableResolver; new edge cases bounded |
| 8 | 9 | 40% | First full-pipeline test; format mismatches expected |
| 9 | 10 | 25% | Nested Map serialization; file system mock fragility |
| 10 | 11+12 | 25–35% | Deferred cross-file resolution is highest-risk indexer path |
| 11 | 13 | 20% | Pure HTTP; no VSCode; port management is main risk |
| 12 | 14 | 35% | VSCode mock first-time setup; always requires iteration |
| 13 | 15 | 20% | Thin wrappers; Phase 14 mock reused |
| 14 | 16 | 20% | All components individually verified; smoke only |

---

## 7. Test Case Traceability

### Test Cases by Requirement

| Requirement | TC IDs | Phase(s) |
|-------------|--------|---------|
| REQ-VSCODE-001 | TC-VSC-001, TC-VSC-005, TC-VSC-006, TC-CACHE-001–010 | 10, 14 |
| REQ-VSCODE-002 | TC-VSC-003, TC-VSC-007 | 14 |
| REQ-VSCODE-003 | TC-P9-001–005, TC-P9-101/201/301/401/501/601, TC-P11-001–007, TC-RES-001–005 | 2, 4, 7 |
| REQ-VSCODE-004 | TC-P9-601, TC-RES-006, TC-ERR-001, TC-ERR-005, TC-ERR-006 | 4, 7, 15 |
| REQ-VSCODE-005 | TC-RES-001, TC-RES-003, TC-RES-004, TC-RES-005 | 7 |
| REQ-VSCODE-006 | TC-P9-601, TC-RES-002, TC-RES-007, TC-QP-001–005 | 4, 7, 15 |
| REQ-VSCODE-007 | TC-VSC-002, TC-VSC-003, TC-VSC-004 | 14 |
| REQ-VSCODE-008 | TC-P1-001–004 | 1 ✅ |
| REQ-VSCODE-009 | TC-P2-001–003 | 1 ✅ |
| REQ-VSCODE-010 | TC-P6-001–003 | 1 ✅ (P6-001/002), 2 (P6-003) |
| REQ-VSCODE-011 | TC-P7-001–002 | 2 |
| REQ-VSCODE-012 | TC-P5-001–005, TC-P6-002, TC-P8-001–004 | 1 ✅ (partial), 2 |
| REQ-VSCODE-013 | TC-ERR-001–005, TC-ERR-006 | 15 |
| REQ-VSCODE-014 | TC-P3-001–003 | 1 ✅ |
| REQ-VSCODE-015 | TC-P4-001–002 | 1 ✅ |
| REQ-VSCODE-016 | TC-P10-001–003, TC-FH-001–006 | 1 ✅ (partial), 2, 8 |
| REQ-VSCODE-017 | TC-MCP-001, TC-MCP-008–011 | 13 |
| REQ-VSCODE-018 | TC-MCP-002, TC-MCP-004, TC-MCP-005 | 13 |
| REQ-VSCODE-019 | TC-MCP-003, TC-MCP-006, TC-MCP-007 | 13 |
| REQ-VSCODE-020 | TC-MCP-009, TC-MCP-011, TC-MCP-012 | 13 |
| REQ-VSCODE-021 | TC-FILE-001 | 12 |

> **TC-ERR-006 dual traceability:** Maps to both REQ-VSCODE-004 (resolution failure UX) AND REQ-VSCODE-013 (error handling behavior).

> **O2 Note:** REQ-VSCODE-006, -007, -016, -018, -019 are currently classified as verification_method "Demonstration" in requirements.json. These should be updated to "Test" in a future requirements maintenance pass.

### New TC IDs Added in Version 2.0

| TC ID Range | Phase | Focus Area |
|-------------|-------|-----------|
| TC-LTI-001 through TC-LTI-005 | 3 | localTypeInfo visitor extraction (NEW) |
| TC-IDX-001 through TC-IDX-005 | 5 | NavigationIndex CRUD (NEW) |
| TC-UTIL-001 through TC-UTIL-006 | 6 | resolverUtils functions including `parseIntermediates` (NEW) |
| TC-RES-008 through TC-RES-011 | 7 | VtableResolver regex boundary conditions (NEW) |
| TC-FH-005a, TC-FH-005b, TC-FH-006 | 8 | FailureHandlerResolver edge cases (NEW) |
| TC-INT-001 through TC-INT-003 | 9 | Integration seam tests (VtableResolver + FailureHandlerResolver pipelines) (NEW) |
| TC-CACHE-010 | 10 | Cache corruption with matching version (NEW) |
| TC-WI-001 through TC-WI-008 | 11–12 | WorkspaceIndexer core and deferred resolution (NEW) |
| TC-FILE-001 | 12 | File extension discovery for REQ-VSCODE-021 (NEW) |
| TC-MCP-013 through TC-MCP-014 | 13 | MCP stop/port release; workspace path guard (NEW) |
| TC-VSC-008 | 14 | Provider string-coupling test (NEW) |

### Documentation Errata

> **m4 Note:** TC-P9-201/202/203 and TC-RES-005 are mislabeled in the current `test-cases.md`. Correct in a future maintenance pass. No impact on test execution.

### Test Count Estimates by Phase

| Phase | Estimated Test Count | Test Files |
|-------|---------------------|-----------|
| 1 ✅ | 98 | 4 files |
| 2 | 15–20 | visitor-vtable.test.ts (expanded) |
| 3 | 5–8 | visitor-localtypeinfo.test.ts (NEW) |
| 4 | 10–15 | visitor-callsites.test.ts (NEW) |
| 5 | 5–6 | navigationIndex.test.ts (NEW) |
| 6 | 5–7 | resolverUtils.test.ts (NEW) |
| 7 | 11–12 | vtableResolver.test.ts (NEW) |
| 8 | 7–8 | failureHandlerResolver.test.ts (NEW) |
| 9 | 3–4 | integration.test.ts (NEW) |
| 10 | 10–12 | cacheManager.test.ts (NEW) |
| 11 | 7–9 | workspaceIndexer.test.ts (NEW) |
| 12 | 3–5 | workspaceIndexer.test.ts (continued) |
| 13 | 14–16 | mcpServer.test.ts (NEW) |
| 14 | 8–10 | junoDefinitionProvider.test.ts (NEW) |
| 15 | 11–13 | statusBarHelper.test.ts (NEW), quickPickHelper.test.ts (NEW) |
| 16 | 3–5 | integration smoke (ad hoc) |
| **Total** | **213–255** | **17–20 files** |

---

## 8. Definition of Done

### Per Work Item
- [ ] All specified test cases implemented
- [ ] All tests pass (including all prior work items)
- [ ] `npm test` passes with 0 failures (mandatory gate — v2.1 Amendment D1)
- [ ] Source code changes (if any) reviewed by verifier agent
- [ ] No unbounded global state accumulation introduced
- [ ] No `any` type assertions added to production code
- [ ] Naming conventions followed

### Per Sprint
- [ ] Full test suite green (0 failures)
- [ ] Any source bugs documented in `ai/memory/lessons-learned-software-developer.md`
- [ ] ESLint/TSLint clean (when configured)
- [ ] **Requirements-traceability gate (HARD — v2.1 Amendment A3-revised):** Audit all test files for valid `// @{"verify": [...]}` tags. Every requirement with tests written must have at least one linked test case. Every `verify` tag must reference a valid requirement ID in `requirements.json`. Sprint cannot exit until this passes. Applies to every sprint starting Sprint 2 — including retroactive validation of Sprint 1 test files.
- [ ] **Coverage checkpoint (v2.1 Amendment A2):** Starting Sprint 2, run `jest --coverage`. Flag any production source module below 85% line coverage. Flagged modules must be addressed in the current sprint or have a documented plan for the next sprint.
- [ ] **Double test suite run (v2.1 Amendment D2):** If the sprint modifies any production source code, the full test suite must be run TWICE: (1) immediately after the source change but before writing new tests, and (2) after all new tests are written. Both run counts and results must be recorded in the sprint exit report.
- [ ] Sprint deliverables reviewed by final quality engineer
- [ ] PM approval

### Per Phase
- [ ] All work items in the phase complete
- [ ] Test count targets met
- [ ] Acceptance criteria for the phase checked off
- [ ] Cumulative test suite stable

### Project Complete
- [ ] All 16 phases done
- [ ] All 21 requirements have at least one test case
- [ ] Full test suite passes (280+ tests)
- [ ] Jest line coverage ≥90% across production source files (v2.1 Amendment A1)
- [ ] Jest branch coverage ≥85% across production source files (v2.1 Amendment A1)
- [ ] Requirements-traceability coverage: 100% of requirements have at least one linked test case; all `verify` tags reference valid requirement IDs (v2.1 Amendment A3-revised)
- [ ] `tsc --noEmit` exits clean (TypeScript strict mode)
- [ ] Extension loads and resolves a real vtable call in VSCode
- [ ] Documentation (design.md, test-cases.md, this plan) are up to date
- [ ] Cache survives a restart cycle (save → close → reopen → load)

---

## 9. Appendix: File Structure (Target State)

```
vscode-extension/
├── src/
│   ├── parser/
│   │   ├── lexer.ts
│   │   ├── parser.ts
│   │   ├── visitor.ts
│   │   ├── types.ts
│   │   └── __tests__/
│   │       ├── lexer.test.ts                       (Phase 1 ✅)
│   │       ├── visitor-structs.test.ts              (Phase 1 ✅)
│   │       ├── visitor-vtable.test.ts               (Phases 1–2)
│   │       ├── visitor-functions.test.ts            (Phases 1–2)
│   │       ├── visitor-localtypeinfo.test.ts        (Phase 3 — NEW)
│   │       └── visitor-callsites.test.ts            (Phase 4 — NEW)
│   ├── indexer/
│   │   ├── navigationIndex.ts
│   │   ├── workspaceIndexer.ts
│   │   └── __tests__/
│   │       ├── navigationIndex.test.ts              (Phase 5 — NEW)
│   │       └── workspaceIndexer.test.ts             (Phases 11–12 — NEW)
│   ├── resolver/
│   │   ├── vtableResolver.ts
│   │   ├── failureHandlerResolver.ts
│   │   ├── resolverUtils.ts
│   │   └── __tests__/
│   │       ├── resolverUtils.test.ts                (Phase 6 — NEW)
│   │       ├── vtableResolver.test.ts               (Phase 7 — NEW)
│   │       └── failureHandlerResolver.test.ts       (Phase 8 — NEW)
│   ├── cache/
│   │   ├── cacheManager.ts
│   │   └── __tests__/
│   │       └── cacheManager.test.ts                (Phase 10 — NEW)
│   ├── providers/
│   │   ├── junoDefinitionProvider.ts
│   │   ├── quickPickHelper.ts
│   │   ├── statusBarHelper.ts
│   │   └── __tests__/
│   │       ├── junoDefinitionProvider.test.ts       (Phase 14 — NEW)
│   │       ├── quickPickHelper.test.ts              (Phase 15 — NEW)
│   │       └── statusBarHelper.test.ts             (Phase 15 — NEW)
│   ├── mcp/
│   │   ├── mcpServer.ts
│   │   └── __tests__/
│   │       └── mcpServer.test.ts                   (Phase 13 — NEW)
│   ├── __tests__/
│   │   └── integration.test.ts                     (Phase 9 — NEW)
│   ├── __mocks__/
│   │   └── vscode.ts                               (Phase 14 — NEW)
│   └── extension.ts
├── design/
│   ├── design.md
│   └── test-cases.md
├── software-development-plan.md                    (this file)
├── jest.config.js
├── package.json
└── tsconfig.json
```

---

## 10. Review Findings Log

This section records all findings from the three review engineers (Systems Engineer, Quality Engineer, Senior Engineer) that reviewed version 1.0 of this plan. Each finding is listed with its severity, summary, and disposition.

### Review Findings Summary

| ID | Severity | Finding | Disposition | Incorporated In |
|----|---------|---------|------------|----------------|
| C1 | Critical | `apiCallSites` is dead-code data — populated by visitor but never consumed by indexer or resolver | Incorporated | Section 1.3 (design question), Phase 3 (new), Phase 4 (TC-P9 reduced to sample) |
| M1 | Major | Design §5.1 describes CST-based chain-walk; actual code uses 3 regex strategies (`macroRe`, `arrayRe`, `generalRe`) | Incorporated | Phase 7 scope rewritten; TC-RES-008 through TC-RES-011 added |
| M2 | Major | No cross-seam integration test between visitor output, indexer mergeInto, and resolver | Incorporated | Phase 9 added (TC-INT-001, TC-INT-002) |
| M3 | Major | `McpServer.start()` returns void; bound port not observable for port-0 tests | Incorporated | Phase 13 WI-13.0 source change listed as prerequisite |
| M4 | Major | `removeFileRecords` leaves stale entries in 5 flat maps; TC-CACHE-005 acceptance criteria incomplete | Incorporated | Phase 5 TC-IDX-005; Phase 10 TC-CACHE-005 updated with stale-map documentation |
| M5 | Major | REQ-VSCODE-021 has zero test cases | Incorporated | Phase 12 TC-FILE-001; Section 7 traceability table updated |
| M6 | Major | WorkspaceIndexer, NavigationIndex CRUD, and resolverUtils have no TC-\* IDs | Incorporated | Phases 5, 6, 11–12: TC-IDX-001–005, TC-UTIL-001–005, TC-WI-001–008 |
| M7 | Major | Phase 2a debug budget too low at 20%; Sprint 1 actuals were 40% | Incorporated | Phase 2 debug budget revised to 30% |
| M8 | Major | Cache corruption with matching version not tested | Incorporated | Phase 10 TC-CACHE-010 |
| m1 | Minor | `FailureHandlerResolver` has no column guard — any cursor triggers resolution | Incorporated | Phase 8 TC-FH-005a (column 0, LHS) and TC-FH-005b (RHS) |
| m2 | Minor | `junoDefinitionProvider.ts` hard-codes strings matching resolver error messages; drift risk | Incorporated | Phase 14 TC-VSC-008 |
| m3 | Minor | TC-CACHE-009 atomicity — concurrent write scenario undocumented | Incorporated | Phase 10 TC-CACHE-009: documented as single-caller only; concurrency handled by debouncing |
| m4 | Minor | TC-P9-201/202/203 and TC-RES-005 mislabeled in test-cases.md | Deferred | Section 7 Documentation Errata note |
| m5 | Minor | No Jest coverage threshold defined | Incorporated | Section 8 DoD; Phase 16 WI-16.4 (≥80% line coverage gate) |
| m6 | Minor | DoD "no dynamic memory allocation" inapplicable to TypeScript | Incorporated | Section 8 DoD rewritten with TypeScript-relevant constraints |
| m7 | Minor | No MCP stop/port release test | Incorporated | Phase 13 TC-MCP-013 |
| m8 | Minor | `resolveFailureHandlerRootType()` multi-module heuristic untested | Incorporated | Phase 12 TC-WI-008 |
| O1 | Observation | `visitor-vtable.test.ts` already has TC-P6-001/002; Phase 1 scope should include them | Incorporated | Section 2.2 test table and Phase 1 summary updated |
| O2 | Observation | Six requirements under-classified as "Demonstration" instead of "Test" | Deferred | Section 7 note; requirements.json update deferred to maintenance pass |
| O3 | Observation | MCP `file` parameter path not validated against workspace root | Incorporated | Phase 13 TC-MCP-014 |
| O4 | Observation | Failure handler silent fallthrough behavior undocumented | Incorporated | Phase 8 TC-FH-006 |

### Deferred Findings

| ID | Finding | Reason Deferred | Owner |
|----|---------|----------------|-------|
| O2 | requirements.json verification_method fields for REQ-VSCODE-006/-007/-016/-018/-019 should be "Test" | Low-priority documentation fix; no impact on test execution. Deferred to next requirements maintenance pass. | PM |
| m4 | TC-P9-201/202/203 and TC-RES-005 labels in test-cases.md incorrect | Cosmetic fix; no test execution impact. Deferred to next test-cases.md maintenance pass. | PM |

### Rejected Findings

None. All findings were either incorporated or deferred with documented rationale.

### Version 2.0 Verification Pass

The following additional findings from the Systems Engineer's v2.0 review were incorporated:

| ID | Severity | Finding | Disposition |
|----|---------|---------|------------|
| V2-1 | Error | `FailureHandlerResolver` pipeline not included in Phase 9 integration tests | Incorporated: TC-INT-003 added to Phase 9 |
| V2-2 | Error | TC-CACHE-007 (FSW → re-index) tests WorkspaceIndexer behavior, not CacheManager | Incorporated: moved to Phase 11 as TC-WI-004a |
| V2-3 | Warning | `parseIntermediates()` has no dedicated TC in Phase 6 | Incorporated: TC-UTIL-006 added to Phase 6 |
| V2-4 | Warning | Phase 14 has false prerequisite on Phase 13 (architecturally independent) | Incorporated: Phase 14 prerequisite changed to Phases 7+8; can parallel Phase 13 |
