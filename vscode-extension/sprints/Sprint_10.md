# Sprint 10 — Phase 13: MCP Server Tests

**Sprint:** 10  
**Phases:** 13  
**Date:** 2026-04-17  
**Status:** COMPLETE  
**SDP Version:** 3.0

---

## 1. Sprint Startup Protocol Summary

### 1.1 PM Decisions Applied

| Decision | Detail |
|----------|--------|
| Jest spies for resolvers | PM approved using Jest spies rather than real resolvers |
| TC-MCP-001, TC-MCP-008 deferred to Phase 14 | Require VSCode mock (activation context) |
| 14 test cases | PM approved set covering lifecycle, endpoints, security, error handling |

### 1.2 Lessons Learned Applied

| Lesson | Application |
|--------|------------|
| Serialized fix-verify-test plan (2026-04-14) | WI-10.0 source change verified before tests |
| One work item = one phase (2026-04-17) | Each WI has its own verification gate |
| Verify worker output by running tests (2026-04-14) | Full suite run after each WI |
| Always use absolute paths (2026-04-17) | All agent briefs use absolute paths |

### 1.3 Current Baseline

| Metric | Value |
|--------|-------|
| Tests (start) | 426 passing, 0 failing, 14 suites |
| Phases complete | 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12 |
| Phase 4 | REMOVED (PM decision) |

---

## 2. Sprint Goal

Test the embedded HTTP MCP server (`src/mcp/mcpServer.ts`): server lifecycle, all 3 HTTP endpoints, security (127.0.0.1-only binding), error handling, and edge cases.

---

## 3. Work Breakdown

| # | Work Item | Description | Worker | Verifier(s) | Dependencies | Status |
|---|-----------|-------------|--------|-------------|--------------|--------|
| WI-10.0 | Source change: expose bound port | Modify `start()` to return `Promise<number>`; update `extension.ts` call site; verify `tsc --noEmit` + full suite | software-developer | senior-software-engineer | None | ✅ Complete |
| WI-10.1 | MCP Server tests | Create `src/mcp/__tests__/mcpServer.test.ts` with 14 test cases | software-test-engineer | software-quality-engineer, software-verification-engineer | WI-10.0 | ✅ Complete |

### Execution Order

```
WI-10.0 → tsc + full suite gate
WI-10.1 → full suite + traceability gate
Final Quality Engineer → present to PM
```

---

## 4. Acceptance Criteria

1. `start()` exposes the OS-assigned bound port (returns `Promise<number>` or provides `getPort()`)
2. All 14 TC-MCP tests implemented and passing
3. Server confirmed to bind to `127.0.0.1` only (TC-MCP-009)
4. Port release after `stop()` verified (TC-MCP-016)
5. All 426 existing tests still pass (zero regressions)
6. `verify_traceability.py` passes
7. `tsc --noEmit` clean

---

## 5. Test Cases

| TC ID | Scenario | Requirement |
|-------|----------|-------------|
| TC-MCP-002 | `/resolve_vtable_call` accepts POST with correct parameters | REQ-VSCODE-018 |
| TC-MCP-003 | `/resolve_failure_handler` accepts POST with correct parameters | REQ-VSCODE-019 |
| TC-MCP-004 | `/resolve_vtable_call` valid input → HTTP 200, found: true | REQ-VSCODE-018 |
| TC-MCP-005 | `/resolve_vtable_call` no-match → HTTP 200, found: false | REQ-VSCODE-018 |
| TC-MCP-006 | `/resolve_failure_handler` valid input → HTTP 200, found: true | REQ-VSCODE-019 |
| TC-MCP-007 | `/resolve_failure_handler` no handler → HTTP 200, found: false | REQ-VSCODE-019 |
| TC-MCP-009 | Server binds to 127.0.0.1 only | REQ-VSCODE-017, REQ-VSCODE-020 |
| TC-MCP-010 | Application errors use HTTP 200 (not 4xx) | REQ-VSCODE-017 |
| TC-MCP-011 | Headless mode — no vscode.* API calls | REQ-VSCODE-017, REQ-VSCODE-020 |
| TC-MCP-012 | No platform-specific AI imports | REQ-VSCODE-020 |
| TC-MCP-013 | Unknown endpoint → HTTP 404 | REQ-VSCODE-017 |
| TC-MCP-014 | Malformed JSON body → HTTP 400 | REQ-VSCODE-017 |
| TC-MCP-015 | Large request body (>1MB) → rejected | REQ-VSCODE-017 |
| TC-MCP-016 | stop() releases port; re-bind succeeds | REQ-VSCODE-017 |

---

## 6. Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| `start()` return type change breaks `extension.ts` | Low | Low | Single call site; update to `await` or `.catch()` |
| EADDRINUSE between tests | Medium | Medium | Port 0 + afterEach stop() |
| Resolver spy setup complex | Low | Low | Resolvers injected via constructor; simple jest.fn() |

---

## 7. Sprint Outcomes

### 7.1 Final Metrics

| Metric | Value |
|--------|-------|
| Tests (end) | 440 passing, 0 failing, 15 suites |
| New tests added | 14 |
| Source changes | 2 files (mcpServer.ts, extension.ts) |
| Phases complete | 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13 |
| Phase 4 | REMOVED (PM decision) |

### 7.2 Files Modified

| File | Changes |
|------|---------|
| `src/mcp/mcpServer.ts` | `start()` returns `Promise<number>`; added `AddressInfo` import; Promise wraps listen with listening/error events |
| `src/extension.ts` | Updated call site to `.then()/.catch()` pattern; uses `actualPort` for discovery file |
| `src/mcp/__tests__/mcpServer.test.ts` | NEW — 14 tests (TC-MCP-002 through TC-MCP-016) |

### 7.3 Verification History

| # | Agent | Role | Verdict | Key Findings |
|---|-------|------|---------|--------------|
| 1 | software-developer | WI-10.0 (source change) | Complete | start() → Promise<number>, extension.ts .then()/.catch() |
| 2 | software-test-engineer | WI-10.1 (tests) | Complete | 14 tests, all pass, traceability tags present |
| 3 | final-quality-engineer | Final gate | APPROVED | 7/7 AC met, 440 tests, traceability PASS |
