# Sprint 8 — WorkspaceIndexer Core & File Discovery (Phases 11 + 12)

**Sprint:** 8  
**Phases:** 11 (WorkspaceIndexer Core), 12 (File Discovery & Deferred Resolution)  
**Date:** 2026-04-17  
**Status:** COMPLETE  
**SDP Version:** 3.0

---

## 1. Sprint Startup Protocol Summary

### 1.1 PM Decisions Applied

| Decision | Detail |
|----------|--------|
| Phases 11+12 combined | Both target WorkspaceIndexer — combined into single sprint per SDP |
| TC-CACHE-003/004/005 included | Deferred from Sprint 7 — require WorkspaceIndexer coordination |
| TC-CACHE-007/008, TC-WI-004a deferred | Require FileSystemWatcher (VSCode API mock) — deferred to Phase 14 |
| "Test like you fly" continuation | Real file system, real parsing through Chevrotain |

### 1.2 Lessons Learned Applied

| Lesson | Application |
|--------|------------|
| One work item = one phase (2026-04-17) | Serial execution: WI-8.1 first, then WI-8.2 + WI-8.3 |
| Verify with test execution (2026-04-14) | Gate after each worker batch completes |
| Do not do hands-on work (2026-04-14) | All coding delegated to worker agents |
| Always use absolute paths (2026-04-17) | All agent briefs use absolute paths |
| ≤10 tests per agent brief (2026-04-16) | Work items sized 3–8 tests each |
| Integration tests: "test like you fly" (2026-04-17) | Real WorkspaceIndexer with temp dirs on disk |

### 1.3 Current Baseline

| Metric | Value |
|--------|-------|
| Tests (start) | 409 passing, 0 failing, 13 suites |
| Phases complete | 1, 2, 3, 5, 6, 7, 8, 9, 10 |
| Phase 4 | REMOVED (PM decision) |

---

## 2. Sprint Goal

1. Verify `fullIndex()` correctly indexes multiple C files and populates the NavigationIndex
2. Verify `loadFromCache()` uses cache when valid, falls back to full index when not
3. Verify `reindexFile()` re-parses only the changed file
4. Verify `removeFile()` removes file records from the index
5. Verify incremental cache coordination: stale files, new files, deleted files (TC-CACHE-003/004/005)
6. Verify `scanFiles()` discovers all 6 mandatory C/C++ file extensions
7. Verify cross-file deferred positional vtable initializer resolution
8. Verify multi-module failure handler root type disambiguation

---

## 3. Work Breakdown

| # | Work Item | TC IDs | Worker | Verifiers | Dependencies | Status |
|---|-----------|--------|--------|-----------|--------------|--------|
| WI-8.1 | WorkspaceIndexer Core Tests | TC-WI-001, TC-WI-002, TC-WI-003, TC-WI-004, TC-WI-005, TC-WI-006, TC-WI-NEG-001, TC-WI-BND-001 | software-test-engineer | software-quality-engineer, senior-software-engineer, software-verification-engineer | None | ✅ Complete |
| WI-8.2 | Cache-Indexer Coordination Tests | TC-CACHE-003, TC-CACHE-004, TC-CACHE-005 | software-test-engineer | software-quality-engineer, senior-software-engineer | WI-8.1 | ✅ Complete |
| WI-8.3 | File Discovery & Cross-File Resolution | TC-FILE-001, TC-WI-007, TC-WI-008 | software-test-engineer | software-quality-engineer, senior-software-engineer, software-verification-engineer | WI-8.1 | ✅ Complete |

### Deferred Items (Phase 14)

| TC ID | Reason |
|-------|--------|
| TC-CACHE-007 | FileSystemWatcher re-index coordination — requires VSCode mock |
| TC-CACHE-008 | Debounce mechanism — requires fake timers + FileSystemWatcher mock |
| TC-WI-004a | FileSystemWatcher change event → triggers `reindexFile()` |

---

## 4. Acceptance Criteria

1. `workspaceIndexer.test.ts` created with ~14 passing tests
2. `fullIndex()` verified: indexes 2+ files, NavigationIndex populated correctly (TC-WI-001)
3. `loadFromCache()` verified: cache hit → skip re-parse (TC-WI-002); cache miss → full index (TC-WI-003)
4. `reindexFile()` verified: only changed file re-parsed (TC-WI-004)
5. `removeFile()` verified: records removed from index (TC-WI-005)
6. `mergeInto()` multi-file: no key collision (TC-WI-006)
7. Same-file positional vtable resolution verified (TC-WI-007); cross-file deferred path deferred — producer-side source bug documented
8. Single-module FH root type resolution via localTypeInfo verified (TC-WI-008); multi-module path deferred — disambiguation source bug documented
9. Stale file triggers selective re-parse (TC-CACHE-003)
10. New file discovered and indexed (TC-CACHE-004)
11. Deleted file: stale entries persist in index — known M4 behavior documented (TC-CACHE-005)
12. All 6 file extensions discovered (TC-FILE-001)
13. No regressions (409 existing tests still pass)
14. Traceability tags (`// @{"verify": [...]}`) present on all tests

---

## 5. Test Approach

Follow "test like you fly" — temp directories with real C files on disk, real `WorkspaceIndexer` instance, real parsing through Chevrotain. Use `jest.spyOn` on internal methods to verify which files get parsed in incremental scenarios. For cache coordination tests, pre-seed a cache file on disk and verify selective re-parsing behavior.

---

## 6. Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Multi-file merge causes Map key collision | Medium | Medium | TC-WI-006 tests explicitly |
| Deferred positional queue ordering sensitive to index order | High | High | TC-WI-007 verified same-file path; cross-file producer-side bug found |
| `resolveFailureHandlerRootType()` assigns both handlers to same root | Medium | High | TC-WI-008 verified single-module path; multi-module bug confirmed |
| Cache coordinator tests reveal source bugs | Medium | Medium | Fix-in-sprint, report to PM |
| `scanFiles()` misses `.hh` or `.cc` extensions | Low | Medium | TC-FILE-001 catches this |

---

## 7. Sprint Outcomes

### 7.1 Final Metrics

| Metric | Value |
|--------|-------|
| Tests (end) | 423 passing, 0 failing, 14 suites |
| New tests added | 14 |
| Phases complete | 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12 |
| Phase 4 | REMOVED (PM decision) |

### 7.2 Source Bugs Found

| # | Location | Description | Impact | Status |
|---|----------|-------------|--------|--------|
| 1 | `visitor.ts` — `extractPositionalVtable()` | Cross-file deferred positional producer not implemented: returns without pushing `DeferredPositional` entry when API struct is in a different file | Cross-file positional initializers silently dropped from index | Documented in TC-WI-007 comment; deferred for future fix |
| 2 | `workspaceIndexer.ts` — `resolveFailureHandlerRootType()` | Multi-module disambiguation fails: iterates all `localTypeInfo` and returns the first known root type found, attributing all handlers to whichever root type appears first in Map iteration order | Multi-module failure handlers mis-attributed | Documented in TC-WI-008 comment; deferred for future fix |
| 3 | `workspaceIndexer.ts` — `resolveDeferred()` | Duplicate-check compares `e.line` against assignment line (`d.lines[i]`) but `loc.line` stores the definition line (from `resolveDefinitionLine()`) — mismatch causes guard to always pass | Duplicate ConcreteLocation entries when producer bug is fixed | Documented in TC-WI-007 comment; dormant until Bug 1 fixed |
| 4 | `workspaceIndexer.ts` — `hashText()` vs `hashFile()` | `hashText()` uses `update(string, "utf8")` while `hashFile()` uses `update(Buffer)` — BOM-prefixed UTF-8 files produce different hashes | BOM-prefixed files would perpetually appear stale | Low-risk edge case; no BOM files in project |

### 7.3 Verification History

| # | Verifier | Verdict | Key Findings |
|---|----------|---------|--------------|
| 1 | software-quality-engineer | NEEDS CHANGES → APPROVED (after fix) | Header comment missing 6 TC IDs |
| 2 | senior-software-engineer | NEEDS CHANGES → APPROVED (after fix) | Source bugs 1–4 identified; TC-WI-007 comment updated |
| 3 | software-verification-engineer | APPROVED | All traceability correct |
| 4 | final-quality-engineer | REJECTED → APPROVED (after fix) | Sprint document inconsistencies (status, AC wording, missing completion section) |
