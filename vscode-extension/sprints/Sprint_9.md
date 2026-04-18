# Sprint 9 — Source Bug Fixes (4 Bugs from Sprint 8)

**Sprint:** 9  
**Phases:** Bug Fix Sprint (cross-cutting)  
**Date:** 2026-04-17  
**Status:** COMPLETE  
**SDP Version:** 3.0

---

## 1. Sprint Startup Protocol Summary

### 1.1 PM Decisions Applied

| Decision | Detail |
|----------|--------|
| Fix all 4 bugs | PM directed: "We should absolutely fix these bugs in the next sprint" |
| Serialized execution | Each bug fixed and verified before moving to the next |
| Tests after all fixes | New/updated tests written after all source fixes pass baseline |

### 1.2 Lessons Learned Applied

| Lesson | Application |
|--------|------------|
| Serialized fix-verify-test plan (2026-04-14) | Phase 1: serial source fixes with gate after each; Phase 2: tests |
| Never do hands-on work (2026-04-14) | All coding delegated to worker agents |
| One work item = one phase (2026-04-17) | Each bug is its own work item with independent verification |
| Fix all failing tests BEFORE writing new ones (2026-04-16) | Each fix verified against full test suite before next |
| Always use absolute paths (2026-04-17) | All agent briefs use absolute paths |

### 1.3 Current Baseline

| Metric | Value |
|--------|-------|
| Tests (start) | 423 passing, 0 failing, 14 suites |
| Phases complete | 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12 |
| Phase 4 | REMOVED (PM decision) |

---

## 2. Sprint Goal

Fix all 4 source bugs discovered during Sprint 8:

1. **Bug 1** — `visitor.ts` `extractPositionalVtable()`: cross-file deferred positional producer not implemented
2. **Bug 2** — `workspaceIndexer.ts` `resolveFailureHandlerRootType()`: multi-module FH disambiguation fails
3. **Bug 3** — `workspaceIndexer.ts` `resolveDeferred()`: duplicate-check compares wrong line values (dormant until Bug 1 fixed)
4. **Bug 4** — `workspaceIndexer.ts` `hashText()` vs `hashFile()`: BOM-prefixed files produce different hashes

---

## 3. Work Breakdown

| # | Work Item | Description | Worker | Verifier(s) | Dependencies | Status |
|---|-----------|-------------|--------|-------------|--------------|--------|
| WI-9.1 | Fix Bug 1: Cross-file deferred positional producer | Add `pendingPositionalVtables` to `ParsedFile`; update `extractPositionalVtable()` to push deferred entries; update `mergeInto()` to consume them | software-developer | senior-software-engineer | None | ✅ Complete |
| WI-9.2 | Fix Bug 3: `resolveDeferred()` duplicate-check | Fix comparison to use `loc.line` instead of `d.lines[i]` | software-developer | senior-software-engineer | WI-9.1 | ✅ Complete |
| WI-9.3 | Fix Bug 2: Multi-module FH disambiguation | Scope `resolveFailureHandlerRootType()` to the function containing the FH assignment | software-developer | senior-software-engineer | None | ✅ Complete |
| WI-9.4 | Fix Bug 4: Hash method mismatch | Unify `hashText()` and `hashFile()` encoding | software-developer | senior-software-engineer | None | ✅ Complete |
| WI-9.5 | Write/update tests for all 4 fixes | Add cross-file positional test, multi-module FH test, hash BOM test; update existing test comments | software-test-engineer | software-quality-engineer, software-verification-engineer | WI-9.1–9.4 | ✅ Complete |

### Execution Order

```
WI-9.1 (Bug 1) → full test suite gate
WI-9.2 (Bug 3) → full test suite gate
WI-9.3 (Bug 2) → full test suite gate
WI-9.4 (Bug 4) → full test suite gate
WI-9.5 (Tests) → full test suite + traceability gate
Final Quality Engineer → present to PM
```

---

## 4. Acceptance Criteria

1. `extractPositionalVtable()` pushes deferred entries into `ParsedFile.pendingPositionalVtables` when API struct not found in same file
2. `mergeInto()` consumes `pendingPositionalVtables` and populates the `deferred` array for cross-file resolution
3. Cross-file positional vtable initializers resolve correctly after `fullIndex()`
4. `resolveDeferred()` duplicate-check compares `loc.line` (definition line), not `d.lines[i]` (assignment line)
5. `resolveFailureHandlerRootType()` correctly disambiguates when multiple root types exist in one file
6. `hashText()` and `hashFile()` produce identical hashes for same file content (including BOM)
7. All 423 existing tests still pass (zero regressions)
8. New/updated tests verify each fix with traceability tags
9. `verify_traceability.py` passes

---

## 5. Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Bug 1 fix requires `ParsedFile` interface change | Medium | Medium | New optional field — won't break existing consumers |
| Bug 2 fix requires FH assignment line → function mapping | Medium | Medium | `FailureHandlerRecord.line` + `localTypeInfo.functionParameters` keys provide scope |
| Bug 4 fix changes hash output → invalidates existing caches | Low | Low | Caches regenerate automatically on next startup |

---

## 6. Sprint Outcomes

### 6.1 Final Metrics

| Metric | Value |
|--------|-------|
| Tests (end) | 426 passing, 0 failing, 14 suites |
| New tests added | 3 |
| Bugs fixed | 4 |
| Phases complete | 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12 |
| Phase 4 | REMOVED (PM decision) |

### 6.2 Files Modified

| File | Changes |
|------|---------|
| `src/parser/types.ts` | Added `PendingPositionalVtable` interface, added field to `ParsedFile` |
| `src/parser/visitor.ts` | Updated `extractPositionalVtable()` to push deferred entries; updated constructor |
| `src/indexer/workspaceIndexer.ts` | Fixed `resolveDeferred()` duplicate-check, `resolveFailureHandlerRootType()` scoping, `hashFile()` encoding; added `findContainingFunction()` helper; imports and `mergeInto()` updated |
| `src/indexer/__tests__/workspaceIndexer.test.ts` | 3 new tests (TC-WI-007b, TC-WI-008b, TC-WI-009); updated comments and header |

### 6.3 Verification History

| # | Agent | Role | Verdict | Key Findings |
|---|-------|------|---------|--------------|
| 1 | software-developer | WI-9.1 (Bug 1 fix) | Complete | Added PendingPositionalVtable, updated visitor + mergeInto |
| 2 | software-developer | WI-9.3 (Bug 2 fix) | Complete | Added findContainingFunction(), scoped FH resolution |
| 3 | software-test-engineer | WI-9.5 (Tests) | Complete | 3 new tests, comment updates, all pass |
| 4 | final-quality-engineer | Final gate | APPROVED | 9/9 acceptance criteria met, 426 tests pass, traceability PASS |
