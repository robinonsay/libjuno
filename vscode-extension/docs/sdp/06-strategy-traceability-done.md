> Part of: [Software Development Plan](index.md) — Sections 6-8

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
