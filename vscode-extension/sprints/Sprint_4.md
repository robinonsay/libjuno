# Sprint 4 — Resolver Utilities + VtableResolver + Traceability Fix

**Sprint:** 4  
**Phases:** 6 (Resolver Utilities), 7 (VtableResolver)  
**Date:** 2026-04-17  
**Status:** IN PROGRESS  
**SDP Version:** 3.0

---

## 1. Sprint Startup Protocol Summary

### 1.1 PM Decisions Applied

| Decision | Detail |
|----------|--------|
| Combine Phases 6 + 7 | Phase 6 (resolverUtils, 4 functions, 109 LOC) is small enough to combine with Phase 7 (VtableResolver, 244 LOC, 6 existing tests) in one sprint. Sprint 3 proved multi-phase sprints work. |
| Fix traceability script | PM identified `verify_traceability.py` reports 11 false-positive errors because it doesn't scan `src/` for `@verify` tags. Fix in this sprint. |

### 1.2 Lessons Learned Applied

| Lesson | Application |
|--------|------------|
| One work item = one phase (2026-04-17) | Each WI is independently gated |
| Verify with test execution (2026-04-14) | Gate after every work item |
| Do not do hands-on work (2026-04-14) | All coding delegated to worker agents |
| ≤10 tests per agent brief (2026-04-16) | Phase 6: 9 tests across 4 WIs; Phase 7: ~10 new tests across 4 WIs |
| Always use absolute paths (2026-04-17) | All agent briefs use absolute paths |

### 1.3 Current Baseline

| Metric | Value |
|--------|-------|
| Tests | 360 passing, 0 failing, 9 suites |
| Phases complete | 1, 2, 3, 5 |
| Phase 4 | REMOVED (PM decision) |
| @verify traceability | Tags exist for all 11 test-method requirements; script has false-positive bug |

---

## 2. Sprint Goal

1. Implement Phase 6 (resolverUtils) — 9 test cases in new file
2. Implement Phase 7 (VtableResolver) — ~10 new test cases extending existing file
3. Fix `verify_traceability.py` to eliminate false-positive traceability errors
4. Add file header docblock to vtableResolver.test.ts (pre-existing tech debt)
5. Document `lookupVariableType()` search order and regex match indices for downstream phases

---

## 3. Work Breakdown Structure

### Track A — Phase 6: Resolver Utilities (new file)

| WI | TC IDs | Description | Agent | Verifiers |
|----|--------|-------------|-------|-----------|
| WI-4.1 | TC-UTIL-001, TC-UTIL-002, TC-UTIL-BND-001 | `findEnclosingFunction()` — inside body, outside, at exact start line | software-test-engineer | SQE, SSE |
| WI-4.2 | TC-UTIL-003, TC-UTIL-004, TC-UTIL-NEG-001 | `lookupVariableType()` — found in locals, fallback to params, unknown var | software-test-engineer | SQE, SSE |
| WI-4.3 | TC-UTIL-005, TC-UTIL-NEG-002 | `walkToRootType()` — 3-hop chain, cycle detection, type not in chain | software-test-engineer | SQE, SSE |
| WI-4.4 | TC-UTIL-006 | `parseIntermediates()` — single, multi, dot, empty | software-test-engineer | SQE, SSE |

**Gate 4A**: All 9 resolverUtils tests pass. Discovery checkpoint confirms `lookupVariableType()` search order.

### Track B — Phase 7: VtableResolver (extend existing file)

| WI | TC IDs | Description | Agent | Verifiers |
|----|--------|-------------|-------|-----------|
| WI-4.5 | TC-RES-002, TC-RES-003 | Strategy 2 (array subscript) + Strategy 3 (general chain) resolution | software-test-engineer | SQE, SSE |
| WI-4.6 | TC-RES-004, TC-RES-005 | Named API member + field-name fallback (JUNO_MODULE_SUPER) | software-test-engineer | SQE, SSE |
| WI-4.7 | TC-RES-007, TC-RES-NEG-001 | Multi-match (QuickPick list) + derivation chain miss | software-test-engineer | SQE, SSE |
| WI-4.8 | TC-RES-008–011 | Regex boundary conditions: macroRe, arrayRe, generalRe | software-test-engineer | SQE, SSE |

**Gate 4B**: ~16 total vtableResolver tests pass (6 existing + ~10 new). Regex match indices confirmed.

### Track C — Cleanup

| WI | Description | Agent |
|----|-------------|-------|
| WI-4.9 | Add file header docblock to vtableResolver.test.ts | junior-software-developer |

### Track D — Traceability Fix

| WI | Description | Agent |
|----|-------------|-------|
| WI-4.10 | Fix `verify_traceability.py` line ~590: add `"src"` to @verify scan directories | junior-software-developer |

---

## 4. Dependency Graph

```
Track A (WI-4.1..4.4) ──► Gate 4A ──┐
                                      ├──► Verifiers ──► Final QE
Track B (WI-4.5..4.8) ──► Gate 4B ──┘
Track C (WI-4.9) ──────────────────────►
Track D (WI-4.10) ─────────────────────►
```

- Tracks A, B, C, D are all independent (different files)
- Gate 4A: resolverUtils.test.ts passes
- Gate 4B: vtableResolver.test.ts passes
- Final QE: full suite + traceability script passes

---

## 5. Exit Criteria

- [ ] `resolverUtils.test.ts`: 9 tests, all passing
- [ ] `vtableResolver.test.ts`: ~16 tests total, all passing
- [ ] Full suite: ~370+ tests, 0 failures
- [ ] `python3 scripts/verify_traceability.py --root vscode-extension/` passes with 0 errors
- [ ] Cycle detection in `walkToRootType()` verified
- [ ] All 3 regex strategies have boundary tests
- [ ] `fieldNameFallback` path tested (TC-RES-005)
- [ ] vtableResolver.test.ts has file header docblock

---

## 6. Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Regex column-range off-by-one | High | High | TC-RES-008–011 boundary tests; source fix if needed |
| `walkToRootType` cycle detection broken | Low | High | TC-UTIL-005 tests 2-node cycle |
| `JUNO_MODULE_SUPER` fallback not implemented | Low | Medium | TC-RES-005 tests it; diagnose first |
