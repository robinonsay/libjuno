# Sprint 11 — Phase 14: VSCode Mocks & Definition Provider

**Sprint:** 11  
**Phases:** 14  
**Date:** 2026-04-17  
**Status:** COMPLETE  
**SDP Version:** 3.0

---

## 1. Sprint Startup Protocol Summary

### 1.1 Documents Reviewed

| Document | Key Points |
|----------|-----------|
| `ai/skills/software-lead.md` | Orchestration protocol, work item sizing, verifier assignments |
| `ai/memory/lessons-learned-software-lead.md` | 15 lessons applied (see §1.2) |
| `software-development-plan.md` (v3.0) | Phase 14 scope: mock + 10 test cases |
| `design/test-cases.md` | TC-VSC-001 through TC-VSC-007 specifications |
| `requirements/vscode-extension/requirements.json` | 21 requirements; REQ-VSCODE-001, 002, 004, 007, 013, 016 in scope |
| `design/design.md` | Architecture, DefinitionProvider bridge (§6) |
| `sprints/Sprint_10.md` | Previous sprint: 440 tests, Phase 13 complete |

### 1.2 Lessons Learned Applied

| Lesson | Application |
|--------|------------|
| Serialized fix-verify-test plan (2026-04-14) | WI-11.0 mock verified before tests |
| One work item = one phase (2026-04-17) | Each WI has its own verification gate |
| Verify worker output by running tests (2026-04-14) | Full suite run after each WI |
| Always use absolute paths (2026-04-17) | All agent briefs use absolute paths |
| Never do hands-on work (2026-04-14) | All work delegated to agents |

### 1.3 Current Baseline

| Metric | Value |
|--------|-------|
| Tests (start) | 440 passing, 0 failing, 15 suites |
| Phases complete | 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13 |
| Phase 4 | REMOVED (PM decision) |

---

## 2. Sprint Goal

Build the shared VSCode API mock (`__mocks__/vscode.ts`) and verify the `JunoDefinitionProvider` bridge — activation, definition resolution (vtable + failure handler), fall-through logic, command registration, FileSystemWatcher setup, and string-coupling behavior.

---

## 3. Work Breakdown

| # | Work Item | Description | Worker | Verifier(s) | Dependencies | Status |
|---|-----------|-------------|--------|-------------|--------------|--------|
| WI-11.0 | VSCode mock infrastructure | Build `src/__mocks__/vscode.ts`; update jest.config.js moduleNameMapper | software-developer | software-quality-engineer | None | ✅ Complete |
| WI-11.1+WI-11.2 | Definition provider tests | Create `providers/__tests__/junoDefinitionProvider.test.ts` with 10 test cases | software-test-engineer | software-quality-engineer, software-verification-engineer | WI-11.0 | ✅ Complete |

### Execution Order

```
WI-11.0 → tsc + full suite gate (mock doesn't break existing tests)
  ↓
WI-11.1+WI-11.2 → full suite + traceability gate
  ↓
Verifier review → rework (1 iteration)
  ↓
Final Quality Engineer → present to PM
```

---

## 4. Acceptance Criteria

1. ✅ `__mocks__/vscode.ts` implemented and loadable by Jest (module name mapping works)
2. ✅ TC-VSC-001 through TC-VSC-008 implemented and passing (10 test cases)
3. ✅ TC-VSC-NEG-001, TC-VSC-BND-001 implemented and passing
4. ✅ Provider string-coupling test passing (TC-VSC-008)
5. ✅ All 440 existing tests still pass (zero regressions)
6. ✅ `verify_traceability.py` passes (0 errors, 0 warnings)
7. ✅ `tsc --noEmit` clean

---

## 5. Test Cases

| TC ID | Scenario | Requirement |
|-------|----------|-------------|
| TC-VSC-001 | Extension `activate()` completes without throwing; subscriptions registered | REQ-VSCODE-001 |
| TC-VSC-002 | DefinitionProvider registered for `c` and `cpp` language IDs | REQ-VSCODE-001, REQ-VSCODE-007 |
| TC-VSC-003 | `provideDefinition()` — vtable call → returns Location with correct file and 0-based line | REQ-VSCODE-002, REQ-VSCODE-007 |
| TC-VSC-004 | `provideDefinition()` — failure handler match → returns Location | REQ-VSCODE-016 |
| TC-VSC-005 | `provideDefinition()` — no match from either resolver → returns undefined | REQ-VSCODE-007 |
| TC-VSC-006 | FileSystemWatcher registered with `**/*.{c,h,cpp,hpp,hh,cc}` glob | REQ-VSCODE-001 |
| TC-VSC-007 | Both `libjuno.goToImplementation` and `libjuno.reindexWorkspace` commands registered | REQ-VSCODE-001 |
| TC-VSC-008 | Both resolvers fail with real error → `setStatusBarMessage` called with `LibJuno` prefix | REQ-VSCODE-004, REQ-VSCODE-013 |
| TC-VSC-NEG-001 | Non-vtable line → undefined, `setStatusBarMessage` NOT called | REQ-VSCODE-007 |
| TC-VSC-BND-001 | `provideDefinition()` at line 0, col 0, empty line → no throw, returns undefined | REQ-VSCODE-002 |

---

## 6. Risk Register

| Risk | Likelihood | Impact | Mitigation | Outcome |
|------|-----------|--------|------------|---------|
| Mock misses a VSCode API used by `activate()` | High | High | Iterate on mock until TC-VSC-001 passes | Mock covered all 17 API surfaces; no iteration needed |
| Jest module name mapping misconfigured | Medium | Medium | Verify `moduleNameMapper` in jest.config.js | Mapping worked first try |
| Resolver injection not accessible for spying | Medium | Medium | Constructor DI confirmed in source | `jest.spyOn` on resolve methods worked cleanly |
| `activate()` triggers disk I/O via WorkspaceIndexer | High | High | Mock fs or handle gracefully | `/test-workspace` doesn't exist; extension's try-catch handles gracefully |
| MCP server binds real port during tests | Medium | Medium | Mock McpServer | `jest.mock('../../mcp/mcpServer')` prevents port binding |

---

## 7. Sprint Outcomes

### 7.1 Final Metrics

| Metric | Value |
|--------|-------|
| Tests (end) | 450 passing, 0 failing, 16 suites |
| New tests added | 10 |
| New files | 2 (mock + test) |
| Modified files | 1 (jest.config.js) |
| Source bugs found | 0 |
| Phases complete | 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 |
| Phase 4 | REMOVED (PM decision) |

### 7.2 Files Created/Modified

| File | Changes |
|------|---------|
| `src/__mocks__/vscode.ts` | NEW — VSCode API mock: Position, Range, Location, Uri classes; enums; languages/commands/window/workspace namespace stubs; createMockExtensionContext(); resetMocks() |
| `src/providers/__tests__/junoDefinitionProvider.test.ts` | NEW — 10 tests (TC-VSC-001 through TC-VSC-008, NEG-001, BND-001) |
| `jest.config.js` | Added `'^vscode$': '<rootDir>/src/__mocks__/vscode.ts'` to moduleNameMapper |

### 7.3 Verification History

| # | Agent | Role | Verdict | Key Findings |
|---|-------|------|---------|--------------|
| 1 | software-developer | WI-11.0 (mock) | Complete | All 17 VSCode API surfaces stubbed |
| 2 | software-test-engineer | WI-11.1+11.2 (tests) | Complete | 11 tests initially (1 removed in rework) |
| 3 | software-quality-engineer | Verifier | NEEDS CHANGES | TC-VSC-001 always-pass; TC-VSC-005b non-standard ID; TC-VSC-005 redundant with TC-VSC-007 |
| 4 | software-verification-engineer | Verifier | NEEDS CHANGES | TC-VSC-004/TC-VSC-007 mapping discrepancy (SDP vs test-cases.md); non-standard IDs |
| 5 | software-developer | Rework | Complete | Fixed TC-VSC-001 (added subscriptions assertion); removed redundant TC-VSC-005; renamed TC-VSC-005b→005; added cancellationToken reset |
| 6 | final-quality-engineer | Final gate | APPROVED | 7/7 AC met, 450 tests, traceability PASS |

### 7.4 Verifier Finding Adjudication

| Finding | Verdict | Rationale |
|---------|---------|-----------|
| TC-VSC-001 always-pass | **Accepted** — fixed | Added `expect(context.subscriptions.length).toBeGreaterThan(0)` |
| TC-VSC-005b non-standard ID | **Accepted** — fixed | Renamed to TC-VSC-005; removed redundant Group A TC-VSC-005 |
| TC-VSC-005 redundant with TC-VSC-007 | **Accepted** — removed | TC-VSC-007 is a strict superset |
| `cancellationToken` reset missing | **Accepted** — fixed | Added `cancellationToken.isCancellationRequested = false` to resetMocks() |
| TC-VSC-004 requirement mismatch (SDP vs test-cases.md) | **Rejected** | SDP Phase 14 is the controlling document; TC-VSC-004 = failure handler navigation per SDP |
| TC-VSC-007 requirement mismatch | **Rejected** | SDP Phase 14 maps TC-VSC-007 = "all commands registered" |
| TC-VSC-NEG-001/BND-001 non-standard ID | **Rejected** | Project-wide convention: NEG/BND suffixes used in every phase (TC-IDX-NEG-001, TC-LTI-BND-001, etc.) |
