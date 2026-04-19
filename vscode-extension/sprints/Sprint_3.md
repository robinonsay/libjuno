# Sprint 3 — Cleanup + Local Type Info + Navigation Index

**Sprint:** 3  
**Phases:** 3 (Local Type Info), 5 (Navigation Index CRUD), plus traceability cleanup  
**Date:** 2026-04-17  
**Status:** IN PROGRESS  
**SDP Version:** 3.0 (mutation testing removed per PM directive)

---

## 1. Sprint Startup Protocol Summary

### 1.1 PM Decisions Applied

| Decision | Detail |
|----------|--------|
| Remove mutation testing | Mutation testing dropped in favor of requirements verification and traceability as the test efficacy metric. Stryker artifacts to be removed. |
| Remove `apiCallSites` | Dead-code data path: visitor populates `apiCallSites` but `mergeInto()` never ingests it and no resolver reads it. PM directive: remove for now; can be re-added if needed. Phase 4 eliminated. |
| Advance past Sprint 2 | Sprint 2 functional work is COMPLETE (all TCs pass). Sprint 2B (mutation hardening) is OBSOLETE. |

### 1.2 Lessons Learned Applied

| Lesson | Application |
|--------|------------|
| One work item = one phase (2026-04-17) | Each work item is independently gated |
| Fix failing tests BEFORE new ones (2026-04-16) | Traceability cleanup (Track A) before new test work |
| Verify with test execution (2026-04-14) | Gate after every work item |
| Do not do hands-on work (2026-04-14) | All coding delegated to worker agents |
| ≤10 tests per agent brief (2026-04-16) | Phase 3: 5+3 tests, Phase 5: 5+2 tests |

### 1.3 Current Baseline

| Metric | Value |
|--------|-------|
| Tests | 345 passing, 0 failing, 7 suites |
| Phases complete | Phase 1, Phase 2 |
| @req tags | 21/21 (100%) |
| @verify tags | 11/11 test-method reqs (100%) |

### 1.4 Known Debt (from Verification Engineer audit)

| # | Issue | Resolution in this sprint |
|---|-------|--------------------------|
| 1 | TC-P10/P11 IDs reversed in `visitor-functions.test.ts` | WI-3.1 |
| 2 | `parser-grammar.test.ts` missing `@verify` tags | WI-3.2 |
| 3 | `vtableResolver.test.ts` missing TC-RES identifiers | WI-3.3 |
| 4 | SDP Section 2 stale | WI-3.4 |
| 5 | Stryker artifacts in repo | WI-3.5 |

---

## 2. Sprint Goal

1. Clear all documentation and traceability debt
2. Remove mutation testing artifacts and dead-code data paths
3. Implement Phase 3 (Visitor Local Type Info) — 8 test cases
4. Implement Phase 5 (Navigation Index CRUD) — 7 test cases
5. Document `localTypeInfo` and NavigationIndex Map key formats for downstream phases

---

## 3. Work Breakdown Structure

### Track A — Cleanup (parallel, no dependencies between items)

| WI | Description | Agent | Files | AC |
|----|------------|-------|-------|-----|
| WI-3.1 | Fix TC-P10/P11 ID swap in `visitor-functions.test.ts` | `junior-software-developer` | `visitor-functions.test.ts` | 1. TC-P10 → failure handlers (matching test-cases.md Section 10). 2. TC-P11 → function defs (matching test-cases.md Section 11). 3. All 345 tests pass. |
| WI-3.2 | Add `@verify` tags to `parser-grammar.test.ts` | `junior-software-developer` | `parser-grammar.test.ts` | 1. `@{"verify": ["REQ-VSCODE-003"]}` tag added at file level. 2. All tests pass. |
| WI-3.3 | Add TC-RES IDs to `vtableResolver.test.ts` test names | `junior-software-developer` | `vtableResolver.test.ts` | 1. Each test maps to a TC-RES ID per test-cases.md. 2. All tests pass. |
| WI-3.4 | Update SDP Section 2: fix stale data, remove mutation gate references, mark Phases 1-2 complete, mark Phase 4 removed | `software-developer` | `software-development-plan.md` | 1. Test status table accurate. 2. Phase 2 marked COMPLETE. 3. Mutation testing removed from exit criteria. 4. Phase 4 marked REMOVED. |
| WI-3.5 | Remove Stryker artifacts: `.stryker-tmp/`, `reports/mutation/` | `junior-software-developer` | Multiple cleanup | 1. `.stryker-tmp/` deleted. 2. `reports/mutation/` deleted. 3. `npm test` passes. |

### Track B — Phase 3: Visitor Local Type Info (serial)

| WI | Description | Agent | Files | AC |
|----|------------|-------|-------|-----|
| WI-3.6 | Write `visitor-localtypeinfo.test.ts`: TC-LTI-001 through TC-LTI-005 | `software-test-engineer` | New: `parser/__tests__/visitor-localtypeinfo.test.ts` | 1. 5 tests passing. 2. `@{"verify": ["REQ-VSCODE-003", "REQ-VSCODE-009", "REQ-VSCODE-015"]}` tags. 3. Discovery checkpoint: dump `localTypeInfo` structure after TC-LTI-001. 4. No regressions. |
| **GATE-3.6** | Verify TC-LTI-001 key format matches `resolverUtils.lookupVariableType()` | Lead | — | Format documented |
| WI-3.7 | Add TC-LTI-NEG-001, TC-LTI-NEG-002, TC-LTI-BND-001 to same file | `software-test-engineer` | `visitor-localtypeinfo.test.ts` | 1. 3 more tests passing. 2. Total: 8. 3. No regressions. |

### Track C — Phase 5: Navigation Index CRUD (serial)

| WI | Description | Agent | Files | AC |
|----|------------|-------|-------|-----|
| WI-3.8 | Write `navigationIndex.test.ts`: TC-IDX-001 through TC-IDX-005 | `software-test-engineer` | New: `indexer/__tests__/navigationIndex.test.ts` | 1. 5 tests passing. 2. `@{"verify": ["REQ-VSCODE-001"]}` tag. 3. Discovery checkpoint: confirm Map key formats. 4. No regressions. |
| **GATE-3.8** | Verify NavigationIndex Map key formats documented | Lead | — | Key formats recorded |
| WI-3.9 | Add TC-IDX-NEG-001, TC-IDX-BND-001 to same file | `software-test-engineer` | `navigationIndex.test.ts` | 1. 2 more tests passing. 2. Total: 7. 3. No regressions. |

### Final Gate

| WI | Description | Agent | AC |
|----|------------|-------|-----|
| WI-3.10 | Final quality gate | `final-quality-engineer` | 1. All tests pass. 2. SDP updated. 3. Traceability tags complete. 4. No regressions. 5. Phases 3 and 5 AC met. |

---

## 4. Dependency Graph

```
Track A (Cleanup)              Track B (Phase 3)           Track C (Phase 5)
WI-3.1 ─┐                     WI-3.6                      WI-3.8
WI-3.2 ─┤ (all parallel)        │                            │
WI-3.3 ─┤                     GATE-3.6                    GATE-3.8
WI-3.4 ─┤                       │                            │
WI-3.5 ─┘                     WI-3.7                      WI-3.9
                                 │                            │
                                 └──────────┬─────────────────┘
                                            │
                                       WI-3.10 (Final QE)
```

Tracks A, B, C are independent (different files). Track A items are parallelizable.
Tracks B and C are internally serial (gate between happy-path and negative/boundary).

---

## 5. Sprint Exit Criteria

- [ ] All tests pass (estimated ~360+ after new tests)
- [ ] SDP Section 2 accurate, mutation references removed, Phase 4 marked REMOVED
- [ ] TC-P10/P11 numbering corrected in `visitor-functions.test.ts`
- [ ] All test files have `@verify` tags
- [ ] Phase 3 complete — 8 TCs implemented (TC-LTI-001 through TC-LTI-BND-001)
- [ ] Phase 5 complete — 7 TCs implemented (TC-IDX-001 through TC-IDX-BND-001)
- [ ] `localTypeInfo` key format documented for Phase 6 use
- [ ] NavigationIndex Map key formats documented for Phases 6-8 use
- [ ] Stryker artifacts removed
- [ ] Final QE approved
