> Part of: [Software Development Plan](index.md) — Section 9: Technical Debt Register

# Section 9: Technical Debt Register

## Overview

This register catalogs known technical debt in the LibJuno VSCode extension as of Sprint 34. Items are categorised by type (source code, coverage, test infrastructure, documentation, requirements, architecture), assigned a stable ID, severity rating, effort estimate, and remediation guidance. The register is the authoritative record for debt accepted by PM decision and for open items that must be scheduled in future sprints. Items marked **Accepted** were explicitly deferred by PM decision and are documented for the record; they are not subject to re-opening unless the PM reverses the decision.

---

## Severity and Effort Legend

| Level | Severity meaning | Effort meaning |
|-------|-----------------|----------------|
| High | Causes silent failures, navigation errors, or CI instability | L = multi-sprint |
| Medium | Causes test fragility, incorrect classification, or reduced maintainability | M = one sprint |
| Low | Cosmetic, documentation-only, or no runtime impact | S = hours to a day |
| — | No impact | XS = minutes |

---

## Source Code Debt (TD-SRC)

### TD-SRC-001 — `apiCallSites` Dead Code Path

| Field | Value |
|-------|-------|
| **Category** | Source Code |
| **Location** | `src/parser/visitor.ts` (populates `ParsedFile.apiCallSites`); `src/indexer/workspaceIndexer.ts` (`mergeInto()` never reads it) |
| **Origin** | Sprint 3 — Phase 4 (Visitor: Call Sites) removed by PM decision |
| **Severity** | Medium |
| **Effort** | M |
| **Status** | Closed (Sprint 36) |
| **Resolution** | Dead code removed: `tryExtractCallSite()` deleted from `visitor.ts`; `ParsedFile.apiCallSites` field removed from `types.ts`; `ApiCallSiteRecord` interface removed. |

**Description:** `IndexBuildingVisitor` populates `ParsedFile.apiCallSites` during every CST traversal, but `mergeInto()` in `workspaceIndexer.ts` never reads or stores it, and neither resolver (Vtable nor FailureHandler) consumes it. The visitor logic remains in place but is untested and unmaintained. The data structure is allocated and written on every file parse without purpose.

**Remediation:** Either (a) implement Phase 4 to make the call-site data useful, or (b) remove the `apiCallSites` population code and the `ParsedFile.apiCallSites` field entirely to eliminate the dead path.

---

### TD-SRC-002 — `JUNO_MODULE_RESULT` Extracted but Not Consumed

| Field | Value |
|-------|-------|
| **Category** | Source Code |
| **Location** | `src/parser/visitor.ts` (extracts result type info); `src/indexer/workspaceIndexer.ts` (no consumer) |
| **Origin** | Sprint 2 — visitor extracts result type metadata; noted in design doc as "available for future type resolution but not currently used by the chain-walk algorithm" |
| **Severity** | Low |
| **Effort** | S |
| **Status** | Closed (Sprint 36) |
| **Resolution** | Verified no extraction code existed in source; field was never added to `ParsedFile`. Item was pre-resolved. |

**Description:** The visitor recognises `JUNO_MODULE_RESULT(NAME_T, OK_T)` macro invocations and notes the result type name and payload type. This data is parsed on every file but never stored in `NavigationIndex` and never consumed by any resolver. It is reserved for a future enhancement to result-type inference in complex call chains.

**Remediation:** Either (a) implement result-type inference in the chain-walk algorithm to use this data, or (b) remove the extraction logic if the feature is not planned.

---

### TD-SRC-003 — `(t as any).tokenType?.name` Unsafe Cast in Visitor

| Field | Value |
|-------|-------|
| **Category** | Source Code |
| **Location** | `src/parser/visitor.ts:197` |
| **Origin** | Sprint 1 — workaround for incomplete Chevrotain token type definitions |
| **Severity** | Medium |
| **Effort** | S |
| **Status** | Closed (Sprint 36) |
| **Resolution** | Replaced `(t as any).tokenType?.name` with `(t as IToken).tokenType.name`; `IToken` imported from `chevrotain`. |

**Description:** A single `as any` cast is used to access `tokenType.name` on a Chevrotain token where the TypeScript type definitions do not expose the property directly. This bypasses type checking and would silently return `undefined` if the token structure changes in a Chevrotain upgrade.

**Remediation:** Import and use the correct Chevrotain `IToken` interface property (e.g., `(t as IToken).tokenType.name`) or add a proper type guard.

---

### TD-SRC-004 — Heavy Type Assertion Usage in Visitor

| Field | Value |
|-------|-------|
| **Category** | Source Code |
| **Location** | `src/parser/visitor.ts` (~39 `as CstNode` / `as IToken` cast sites) |
| **Origin** | Sprint 1–2 — Chevrotain v11 CST typing is loose; casts required to navigate CST children |
| **Severity** | Low |
| **Effort** | L |
| **Status** | Closed (Sprint 36) |
| **Resolution** | Created `src/parser/cstHelpers.ts` with `getChild`, `getChildren`, `getToken`, `getTokens`, `getAll` typed helpers; migrated 35 cast sites in `visitor.ts`. 4 casts remain in dead-code `getPostfixSuffixes()` (reserved for future use). |

**Description:** The visitor makes approximately 39 type assertions (`as CstNode`, `as IToken`) throughout its CST traversal logic. While most are safe given the grammar structure, they are not checked at runtime — a grammar change or Chevrotain upgrade could cause silent `undefined` access without compile-time errors.

**Remediation:** Create narrow helper types and type-safe accessor functions for the most-used CST patterns (e.g., `getChild(node, key): CstNode`, `getToken(node, key): IToken`) that encapsulate the casts in one place and can be guarded with runtime checks.

---

### TD-SRC-005 — `activate()` Has No DI Seam for `WorkspaceIndexer`

| Field | Value |
|-------|-------|
| **Category** | Source Code |
| **Location** | `src/extension.ts` (`activate()` function); `src/__tests__/fileSystemEvents.test.ts` (uses `jest.mock('../indexer/workspaceIndexer')`) |
| **Origin** | Sprint 34 — discovered when writing TC-FSE-001–009 |
| **Severity** | Low |
| **Effort** | S |
| **Status** | Deferred |
| **Resolution** | Deferred to future sprint (PM decision, Sprint 36). |

**Description:** The `activate()` function constructs a `WorkspaceIndexer` directly with `new WorkspaceIndexer(...)`. There is no factory parameter or injectable seam through which tests can supply a pre-configured indexer without intercepting the module registry. The `fileSystemEvents.test.ts` activation tests work around this by using `jest.mock('../indexer/workspaceIndexer')` on an internal module — a pattern explicitly discouraged by the project's DI standards.

**Remediation:** Add an optional `indexerFactory` parameter to `activate()` (defaulting to `(root, excludes) => new WorkspaceIndexer(root, excludes)`) so tests can inject a real or stub indexer through the production DI boundary without module-level mocking.

---

### Source Code Debt Summary

| ID | Title | Category | Severity | Effort | Status |
|----|-------|----------|----------|--------|--------|
| TD-SRC-001 | `apiCallSites` Dead Code Path | Source Code | Medium | M | Closed (Sprint 36) |
| TD-SRC-002 | `JUNO_MODULE_RESULT` Extracted but Not Consumed | Source Code | Low | S | Closed (Sprint 36) |
| TD-SRC-003 | `(t as any).tokenType?.name` Unsafe Cast in Visitor | Source Code | Medium | S | Closed (Sprint 36) |
| TD-SRC-004 | Heavy Type Assertion Usage in Visitor | Source Code | Low | L | Closed (Sprint 36) |
| TD-SRC-005 | `activate()` Has No DI Seam for `WorkspaceIndexer` | Source Code | Low | S | Deferred |

---

## Coverage Debt (TD-COV)

### TD-COV-001 — `extension.ts` Branch Coverage 67.2%

| Field | Value |
|-------|-------|
| **Category** | Coverage |
| **Location** | `src/extension.ts:130–134` |
| **Origin** | Sprint 11 — activation tests added; command branch not exercised |
| **Severity** | High |
| **Effort** | S |
| **Status** | Open |

**Description:** `extension.ts` has 67.24% branch coverage — the lowest of any production source file and significantly below the 85% project threshold (Amendment A2). Lines 130–134 correspond to the `libjuno.showVtableTrace` command handler's editor-guard branch (the path taken when the command fires but no active text editor is present). This branch is never exercised by the current test suite.

**Remediation:** Add a test case to `extension-branches.test.ts` that fires the `showVtableTrace` command with no active editor and asserts that the command returns without error.

---

### TD-COV-002 — `visitor.ts` Branch Coverage 77.97%

| Field | Value |
|-------|-------|
| **Category** | Coverage |
| **Location** | `src/parser/visitor.ts` (many branches; see uncovered lines: 31, 38, 80, 124–149, 320–378, 384–386, 396, 453–471, 487, 510, 517–519, 540–542, 552, 557, 597, 607–609, 636, 649, 662–667, 681–700, 710, 731–732, 744–745, 756–757, 768–793, 809, 832, 838–844, 853–856, 864–867, 878, 905, 917, 924, 936, 946, 990, 997–1004, 1038, 1049–1057) |
| **Origin** | Sprints 1–3 — visitor coverage was never driven to 85%+ |
| **Severity** | High |
| **Effort** | L |
| **Status** | Open |

**Description:** `visitor.ts` is the largest and most complex production file (~1,125 lines) and has 77.97% branch coverage. The uncovered branches span dozens of CST traversal edge cases — error handling branches, alternative CST child orderings, rare macro forms, and C grammar edge cases. Gaps here mean some parsing paths are untested production code, increasing the risk of silent data loss when parsing unusual-but-valid C constructs.

**Remediation:** Systematically add targeted test cases to `visitor-branches.test.ts` for each uncovered branch range. Prioritise ranges that involve data emission (struct, vtable, function definition extraction) over pure traversal guards.

---

### TD-COV-003 — `vtableTraceProvider.ts` Branch Coverage 78.12%

| Field | Value |
|-------|-------|
| **Category** | Coverage |
| **Location** | `src/providers/vtableTraceProvider.ts:297–298` |
| **Origin** | Sprint 22 — VtableTraceProvider added; branch at 297–298 not exercised |
| **Severity** | Medium |
| **Effort** | S |
| **Status** | Open |

**Description:** Lines 297–298 correspond to an error-handling branch in the webview message handler (source-line read failure path). This path is not exercised by any current test.

**Remediation:** Add a test case that injects a `readFile` error into the webview handler and asserts graceful failure behaviour.

---

### TD-COV-004 — `workspaceIndexer.ts` Branch Coverage 83.46%

| Field | Value |
|-------|-------|
| **Category** | Coverage |
| **Location** | `src/indexer/workspaceIndexer.ts:269–274, 458` |
| **Origin** | Sprint 9 — cross-file deferred positional resolution path not fully covered |
| **Severity** | Medium |
| **Effort** | S |
| **Status** | Open |

**Description:** Lines 269–274 are in the `mergeInto()` deferred positional vtable handling path; line 458 is an edge case in `resolveCompositionRoots()`. Both represent low-frequency but critical paths — silent failures here cause vtable navigation to break for positional initialisers whose API struct is defined in a separate file.

**Remediation:** Add targeted tests for the `mergeInto()` deferred path when `apiStructFields` is not yet populated, and for the `resolveCompositionRoots()` edge case at line 458.

---

### TD-COV-005 — `mcpServer.ts` Branch Coverage 83.09%

| Field | Value |
|-------|-------|
| **Category** | Coverage |
| **Location** | `src/mcp/mcpServer.ts:136–137, 144–145, 155–157, 221–226, 259–264` |
| **Origin** | Sprint 10 — MCP error handling branches not exercised |
| **Severity** | Medium |
| **Effort** | S |
| **Status** | Open |

**Description:** The uncovered lines correspond to MCP protocol error-handling branches: malformed JSON-RPC responses, network error conditions, and tool-call validation paths. These branches are never triggered by the current test suite, leaving the server's error resilience untested.

**Remediation:** Add negative test cases to `mcpServer.test.ts` that inject malformed requests and network errors into the server.

---

### Coverage Debt Summary

| ID | Title | Category | Severity | Effort | Status |
|----|-------|----------|----------|--------|--------|
| TD-COV-001 | `extension.ts` Branch Coverage 67.2% | Coverage | High | S | Open |
| TD-COV-002 | `visitor.ts` Branch Coverage 77.97% | Coverage | High | L | Open |
| TD-COV-003 | `vtableTraceProvider.ts` Branch Coverage 78.12% | Coverage | Medium | S | Open |
| TD-COV-004 | `workspaceIndexer.ts` Branch Coverage 83.46% | Coverage | Medium | S | Open |
| TD-COV-005 | `mcpServer.ts` Branch Coverage 83.09% | Coverage | Medium | S | Open |

---

## Test Infrastructure Debt (TD-TST)

### TD-TST-001 — REG-17-007 Needs Custom Timeout

| Field | Value |
|-------|-------|
| **Category** | Test Infrastructure |
| **Location** | `src/__tests__/sprint17-regression.test.ts` (REG-17-007) |
| **Origin** | Sprint 17 — regression test for cross-file deferred vtable resolution |
| **Severity** | Medium |
| **Effort** | XS |
| **Status** | Open |

**Description:** REG-17-007 exercises the cross-file deferred positional vtable resolution path which involves real file I/O and a full workspace index cycle. Under parallel Jest execution the test times out with the default 5-second timeout; it passes reliably when run serially. The fix is a single `timeout` option on the test function.

**Remediation:** Add `{ timeout: 15000 }` as the third argument to the `it('REG-17-007: reindexFile() resolves positional vtable assignments...', async () => { ... }, { timeout: 15000 })` call.

---

### TD-TST-002 — Jest Worker Force-Exit Warning (StatusBarHelper Timer Leak)

| Field | Value |
|-------|-------|
| **Category** | Test Infrastructure |
| **Location** | `src/providers/statusBarHelper.ts` (`showError()` schedules `setTimeout(5000)`); test files that instantiate `StatusBarHelper` without `afterEach(() => statusBar.dispose())` |
| **Origin** | Sprint 12 — StatusBarHelper added; dispose() pattern not enforced |
| **Severity** | Medium |
| **Effort** | S |
| **Status** | Open |

**Description:** `StatusBarHelper.showError()` schedules a 5-second auto-clear timeout. Any test suite that creates a `StatusBarHelper` in `beforeEach` but omits `afterEach(() => statusBar.dispose())` leaks the active timer into the Jest worker process. On full-suite runs, Jest logs "A worker process has failed to exit gracefully" and force-kills the worker. This is a pre-existing issue visible on every `npm test` run but not affecting test results.

**Remediation:** Confirmed affected file: `providers/__tests__/junoDefinitionProvider.test.ts` — constructs `StatusBarHelper` in test setup but has no `afterEach(() => statusBar.dispose())` guard. Audit any other test file that constructs `StatusBarHelper` outside of `jest.useFakeTimers()` context. (Note: `statusBarHelper.test.ts` itself already uses `jest.useFakeTimers()` and calls `helper.dispose()`, so it is not affected.)

---

### TD-TST-003 — `jest.mock()` on Internal Module in `fileSystemEvents.test.ts`

| Field | Value |
|-------|-------|
| **Category** | Test Infrastructure |
| **Location** | `src/__tests__/fileSystemEvents.test.ts:41` (`jest.mock('../indexer/workspaceIndexer')`) |
| **Origin** | Sprint 34 — activation tests require a mock indexer; no DI seam available in `activate()` |
| **Severity** | Low |
| **Effort** | S |
| **Status** | Open |

**Description:** The activation-level tests (TC-FSE-001, TC-FSE-004, TC-FSE-005) mock the `workspaceIndexer` module at the Jest registry level because `activate()` provides no DI seam for the indexer. This pattern is explicitly discouraged by the project's DI standards (test doubles must be injected through the production DI boundary). The mock is functional but couples the tests to Jest's module system rather than to the production interface.

**Remediation:** Implement TD-SRC-005 (add `indexerFactory` parameter to `activate()`), then rewrite the activation tests to inject a stub indexer through the factory parameter instead of via `jest.mock`.

---

### Test Infrastructure Debt Summary

| ID | Title | Category | Severity | Effort | Status |
|----|-------|----------|----------|--------|--------|
| TD-TST-001 | REG-17-007 Needs Custom Timeout | Test Infrastructure | Medium | XS | Open |
| TD-TST-002 | Jest Worker Force-Exit Warning (StatusBarHelper Timer Leak) | Test Infrastructure | Medium | S | Open |
| TD-TST-003 | `jest.mock()` on Internal Module in `fileSystemEvents.test.ts` | Test Infrastructure | Low | S | Open |

---

## Documentation Debt (TD-DOC)

### TD-DOC-001 — `01-overview.md` Test Count Stale (622 vs 717)

| Field | Value |
|-------|-------|
| **Category** | Documentation |
| **Location** | `docs/sdp/01-overview.md` Section 2.2, summary row |
| **Origin** | Sprint 23 — test count last updated to 622; 95 tests added across Sprints 24–34 |
| **Severity** | Low |
| **Effort** | XS |
| **Status** | Open |

**Description:** The test status table in Section 2.2 shows "Total | **622** | All passing". Actual count as of Sprint 34 is **717**. The table is also missing 8 test files added after Sprint 14 (e.g., `fileSystemEvents.test.ts`, `sprint17-regression.test.ts`, `c11-grammar-conformance.test.ts`, `vtableTraceProvider.test.ts` etc.).

**Remediation:** Update Section 2.2 test count to 717, add missing test file rows, and update "All passing" status.

---

### TD-DOC-002 — `01-overview.md` Source File Line Counts Stale

| Field | Value |
|-------|-------|
| **Category** | Documentation |
| **Location** | `docs/sdp/01-overview.md` Section 2.1 source code status table |
| **Origin** | Sprint 14 — table last updated; all files have grown since |
| **Severity** | Low |
| **Effort** | S |
| **Status** | Open |

**Description:** Source file line counts in Section 2.1 are significantly stale. Notable gaps: `workspaceIndexer.ts` listed as 399 lines (actual: 643), `failureHandlerResolver.ts` listed as 162 (actual: 405), `mcpServer.ts` listed as 174 (actual: 339). Also, `providers/vtableTraceProvider.ts` (303 lines, added in Phase 22 / Sprint 22) is completely absent from the table.

**Remediation:** Re-run `wc -l` on all source files and update the table. Add the `vtableTraceProvider.ts` row.

---

### TD-DOC-003 — `01-overview.md` Phase WBS Stops at Phase 19 (Actual: Phase 30)

| Field | Value |
|-------|-------|
| **Category** | Documentation |
| **Location** | `docs/sdp/01-overview.md` Section 3 ("The project is organized into 16 small, focused phases") |
| **Origin** | Sprint 19 — WBS last updated; 11 additional phases completed since |
| **Severity** | High |
| **Effort** | M |
| **Status** | Open |

**Description:** Section 3 declares "The project is organized into 16 small, focused phases" and the phase table ends at Phase 19. As of Sprint 34, the project has **30 numbered phases (29 active; Phase 4 removed)**. Phases 20–30 — covering multi-impl UX, FAIL macro navigation, VtableTraceProvider, MCP JSON-RPC 2.0, composition root detection, MCP tool quality, and file system event handling — are entirely absent from the overview. A reader relying on this section would believe the project is far behind schedule and missing major features.

**Remediation:** Update the phase count claim, add a new row in the overview phase table for each of Phases 20–30, and update the WBS diagram or narrative accordingly.

---

### TD-DOC-004 — TC-P9-201/202/203 and TC-RES-005 Mislabeled in Test Case Docs

| Field | Value |
|-------|-------|
| **Category** | Documentation |
| **Location** | `docs/test-cases/` (specific test-cases doc sections) |
| **Origin** | Noted as "m4" finding in Sprint review; deferred from `07-appendix.md` |
| **Severity** | Low |
| **Effort** | XS |
| **Status** | Open (deferred from Appendix m4) |

**Description:** Test case IDs TC-P9-201, TC-P9-202, TC-P9-203, and TC-RES-005 are mislabeled in the test-cases documentation. Labels in the docs do not match the test ID scheme used in the actual test files. No test execution is affected; this is a documentation consistency issue.

**Remediation:** Locate the affected sections in the test-cases docs, correct the TC IDs to match the actual test file naming, and remove the m4 errata note from `07-appendix.md`.

---

### Documentation Debt Summary

| ID | Title | Category | Severity | Effort | Status |
|----|-------|----------|----------|--------|--------|
| TD-DOC-001 | `01-overview.md` Test Count Stale (622 vs 717) | Documentation | Low | XS | Open |
| TD-DOC-002 | `01-overview.md` Source File Line Counts Stale | Documentation | Low | S | Open |
| TD-DOC-003 | `01-overview.md` Phase WBS Stops at Phase 19 (Actual: Phase 30) | Documentation | High | M | Open |
| TD-DOC-004 | TC-P9-201/202/203 and TC-RES-005 Mislabeled in Test Case Docs | Documentation | Low | XS | Open (deferred from Appendix m4) |

---

## Requirements Debt (TD-REQ)

### TD-REQ-001 — Five Requirements Misclassified as "Demonstration" Instead of "Test"

| Field | Value |
|-------|-------|
| **Category** | Requirements |
| **Location** | `requirements/vscode/requirements.json` — REQ-VSCODE-006, REQ-VSCODE-007, REQ-VSCODE-016, REQ-VSCODE-018, REQ-VSCODE-019 |
| **Origin** | Noted as "O2" finding in Sprint review; deferred from `07-appendix.md` |
| **Severity** | Low |
| **Effort** | XS |
| **Status** | Open (deferred from Appendix O2) |

**Description:** Five requirements are classified as `verification_method: "Demonstration"` but each has one or more automated Jest tests with `@{"verify": [...]}` tags. The correct method is `"Test"`. The misclassification does not affect test execution or the traceability tool (which checks tag-to-requirement links, not verification methods), but it misrepresents the project's automated coverage level in documentation and audits.

- REQ-VSCODE-006 (Multiple Implementation Selection) — verified by TC-RES-002, TC-RES-007, TC-QP-001–005
- REQ-VSCODE-007 (Native Go to Definition Integration) — verified by TC-VSC-002, TC-VSC-003, TC-VSC-004
- REQ-VSCODE-016 (Failure Handler Navigation) — verified by TC-FH-001–006
- REQ-VSCODE-018 (AI Vtable Resolution Access) — verified by TC-MCP-002, TC-MCP-004, TC-MCP-005
- REQ-VSCODE-019 (AI Failure Handler Resolution Access) — verified by TC-MCP-003, TC-MCP-006, TC-MCP-007

**Remediation:** Update `verification_method` to `"Test"` for all five requirements in `requirements.json`.

---

### Requirements Debt Summary

| ID | Title | Category | Severity | Effort | Status |
|----|-------|----------|----------|--------|--------|
| TD-REQ-001 | Five Requirements Misclassified as "Demonstration" Instead of "Test" | Requirements | Low | XS | Open (deferred from Appendix O2) |

---

## Architecture Debt (TD-ARCH)

### TD-ARCH-001 — `reindexFile()` Must Mirror `fullIndex()` for All Deferred Data Paths

| Field | Value |
|-------|-------|
| **Category** | Architecture |
| **Location** | `src/indexer/workspaceIndexer.ts` (`reindexFile()` and `fullIndex()`) |
| **Origin** | Sprint 17 — discovered that `reindexFile()` silently dropped cross-file positional vtable initializers because `mergeInto()` was called without a `DeferredPositional[]` array |
| **Severity** | High |
| **Effort** | S |
| **Status** | Open (fixed for known paths; maintenance risk for future paths) |

**Description:** `mergeInto()` has an `if (deferred)` guard that silently drops `pendingPositionalVtables` when no deferred array is provided. This means any future deferred data path added to `mergeInto()` will also be silently dropped by `reindexFile()` unless the developer explicitly threads the array through. The Sprint 17 fix addressed the known `DeferredPositional` path, but the architecture creates an ongoing maintenance trap: there is no compile-time check or type-level guarantee that all deferred data paths are wired in both `fullIndex()` and `reindexFile()`.

**Remediation:** Refactor `mergeInto()` so that `deferred` is a required parameter (not optional), and provide an empty array at call sites that genuinely have no deferred records. This converts a silent data-loss failure mode into a compile-time argument error.

---

### TD-ARCH-002 — Chevrotain `gobbleMacroCallStatement` Uses Private Method Instead of Grammar Rule

| Field | Value |
|-------|-------|
| **Category** | Architecture |
| **Location** | `src/parser/parser.ts` (`gobbleMacroCallStatement` implementation) |
| **Origin** | Sprint 15 — a proper Chevrotain RULE caused stack overflow in `performSelfAnalysis()` |
| **Severity** | Medium |
| **Effort** | M |
| **Status** | Open |

**Description:** `gobbleMacroCallStatement` is implemented as a plain private TypeScript method with a `RECORDING_PHASE` guard rather than as a Chevrotain `RULE`. The reason: making it a RULE and calling `SUBRULE(macroBodyTokens)` caused `RangeError: Maximum call stack size exceeded` during `performSelfAnalysis()` because `macroBodyTokens` uses a `MANY` loop accepting any token, creating an infinite path expansion when reachable from `statement → compoundStatement → statement`. The workaround is functional but reduces grammar clarity — the method is invisible to Chevrotain's error recovery and diagnostic tooling.

**Remediation:** Restructure the grammar to break the left-recursive path (e.g., by gating `macroBodyTokens` with a specific token predicate rather than `any`) so that `gobbleMacroCallStatement` can be a proper Chevrotain RULE without triggering infinite self-analysis.

---

### TD-ARCH-003 — VSIX Packaging: Runtime Dependency Bundling Requires Manual Verification

| Field | Value |
|-------|-------|
| **Category** | Architecture |
| **Location** | `.vscodeignore`; VSIX packaging process |
| **Origin** | Sprint 22 — discovered that blanket `node_modules/**` in `.vscodeignore` strips production deps (e.g., `chevrotain`) from the VSIX |
| **Severity** | High |
| **Effort** | S |
| **Status** | Open |

**Description:** The `.vscodeignore` configuration, if set to `node_modules/**`, strips all `node_modules` including production runtime dependencies. The result: the VSIX installs cleanly but the extension silently fails to activate because `require('chevrotain')` throws `MODULE_NOT_FOUND`. This failure is not caught by CI because packaging is a manual deployment step. The correct `.vscodeignore` excludes only dev dependencies; `vsce package` handles this automatically if the blanket exclude is removed. There is no automated gate currently verifying that the packaged VSIX activates correctly before release.

**Remediation:** (1) Confirm the current `.vscodeignore` does NOT contain `node_modules/**`. (2) Add a post-packaging smoke test step to the release checklist that installs the VSIX and verifies extension activation. (3) Document the correct `.vscodeignore` pattern in the SDP release process section.

---

### TD-ARCH-004 — TypeScript `__mocks__/` Directory Excluded from Production Build

| Field | Value |
|-------|-------|
| **Category** | Architecture |
| **Location** | `tsconfig.json` (`exclude` array); `src/__mocks__/vscode.ts` |
| **Origin** | Sprint 22 — discovered that without explicit exclusion, `__mocks__/` compiles to `out/__mocks__/` and ships in the VSIX |
| **Severity** | Medium |
| **Effort** | XS |
| **Status** | Open (documented, requires verification each build) |

**Description:** Jest `__mocks__/` directories are test infrastructure and must not appear in the compiled `out/` directory or in the packaged VSIX. Without `"**/__mocks__/**"` in `tsconfig.json` `exclude`, the TypeScript compiler picks them up alongside production source files. The entry should be present but must be verified after any `tsconfig.json` restructuring.

**Remediation:** Verify that `"**/__mocks__/**"` is present in the `exclude` array of `tsconfig.json`. Add a CI check or pre-package step that confirms `out/__mocks__/` does not exist after compilation.

---

### Architecture Debt Summary

| ID | Title | Category | Severity | Effort | Status |
|----|-------|----------|----------|--------|--------|
| TD-ARCH-001 | `reindexFile()` Must Mirror `fullIndex()` for All Deferred Data Paths | Architecture | High | S | Open (fixed for known paths; maintenance risk for future paths) |
| TD-ARCH-002 | Chevrotain `gobbleMacroCallStatement` Uses Private Method Instead of Grammar Rule | Architecture | Medium | M | Open |
| TD-ARCH-003 | VSIX Packaging: Runtime Dependency Bundling Requires Manual Verification | Architecture | High | S | Open |
| TD-ARCH-004 | TypeScript `__mocks__/` Directory Excluded from Production Build | Architecture | Medium | XS | Open (documented, requires verification each build) |

---

## Full Register Summary

| ID | Title | Category | Severity | Effort | Status |
|----|-------|----------|----------|--------|--------|
| TD-SRC-001 | `apiCallSites` Dead Code Path | Source Code | Medium | M | Closed (Sprint 36) |
| TD-SRC-002 | `JUNO_MODULE_RESULT` Extracted but Not Consumed | Source Code | Low | S | Closed (Sprint 36) |
| TD-SRC-003 | `(t as any).tokenType?.name` Unsafe Cast in Visitor | Source Code | Medium | S | Closed (Sprint 36) |
| TD-SRC-004 | Heavy Type Assertion Usage in Visitor | Source Code | Low | L | Closed (Sprint 36) |
| TD-SRC-005 | `activate()` Has No DI Seam for `WorkspaceIndexer` | Source Code | Low | S | Deferred |
| TD-COV-001 | `extension.ts` Branch Coverage 67.2% | Coverage | High | S | Open |
| TD-COV-002 | `visitor.ts` Branch Coverage 77.97% | Coverage | High | L | Open |
| TD-COV-003 | `vtableTraceProvider.ts` Branch Coverage 78.12% | Coverage | Medium | S | Open |
| TD-COV-004 | `workspaceIndexer.ts` Branch Coverage 83.46% | Coverage | Medium | S | Open |
| TD-COV-005 | `mcpServer.ts` Branch Coverage 83.09% | Coverage | Medium | S | Open |
| TD-TST-001 | REG-17-007 Needs Custom Timeout | Test Infrastructure | Medium | XS | Open |
| TD-TST-002 | Jest Worker Force-Exit Warning (StatusBarHelper Timer Leak) | Test Infrastructure | Medium | S | Open |
| TD-TST-003 | `jest.mock()` on Internal Module in `fileSystemEvents.test.ts` | Test Infrastructure | Low | S | Open |
| TD-DOC-001 | `01-overview.md` Test Count Stale (622 vs 717) | Documentation | Low | XS | Open |
| TD-DOC-002 | `01-overview.md` Source File Line Counts Stale | Documentation | Low | S | Open |
| TD-DOC-003 | `01-overview.md` Phase WBS Stops at Phase 19 (Actual: Phase 30) | Documentation | High | M | Open |
| TD-DOC-004 | TC-P9-201/202/203 and TC-RES-005 Mislabeled in Test Case Docs | Documentation | Low | XS | Open (deferred from Appendix m4) |
| TD-REQ-001 | Five Requirements Misclassified as "Demonstration" Instead of "Test" | Requirements | Low | XS | Open (deferred from Appendix O2) |
| TD-ARCH-001 | `reindexFile()` Must Mirror `fullIndex()` for All Deferred Data Paths | Architecture | High | S | Open (fixed for known paths; maintenance risk for future paths) |
| TD-ARCH-002 | Chevrotain `gobbleMacroCallStatement` Uses Private Method Instead of Grammar Rule | Architecture | Medium | M | Open |
| TD-ARCH-003 | VSIX Packaging: Runtime Dependency Bundling Requires Manual Verification | Architecture | High | S | Open |
| TD-ARCH-004 | TypeScript `__mocks__/` Directory Excluded from Production Build | Architecture | Medium | XS | Open (documented, requires verification each build) |

---

**Last Updated:** Sprint 36 (2026-04-21)
