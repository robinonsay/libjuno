# Sprint 17 — Phase 18: Parser & Indexer: Production Source Compatibility

**Sprint:** 17
**Phase:** 18 (Parser & Indexer: Production Source Compatibility)
**Date:** 2026-04-18
**Status:** COMPLETE ✅
**SDP Version:** 4.1

---

## 1. Sprint Startup Protocol Summary

### 1.1 Documents Reviewed

| Document | Key Points |
|----------|-----------|
| `ai/skills/software-lead.md` | Orchestration protocol, work item sizing, verifier assignments |
| `ai/memory/lessons-learned-software-lead.md` | Applied: delegate all work, never hands-on, verify by running tests |
| `software-development-plan.md` (v4.0) | All 17 phases complete; no Phase 18 yet |
| `design/design.md` | §2.1 Chevrotain parser, §3 Architecture, §5.2 VtableResolver |
| `requirements/vscode-extension/requirements.json` | REQ-VSCODE-002 (Vtable Navigation), REQ-VSCODE-003 (Pattern Recognition) |

### 1.2 Lessons Learned Applied

| Lesson | Application |
|--------|------------|
| Never do hands-on work; delegate everything (2026-04-14) | All fixes delegated to worker agents |
| Verify worker output by running tests (2026-04-14) | Tests run after every worker completion |
| Fix all failing tests BEFORE writing new ones (2026-04-16) | Source fixes (WI-17.2, 17.2b, 17.2c) completed before regression tests (WI-17.4) |

### 1.3 Baseline (Sprint Start)

| Metric | Value |
|--------|-------|
| Tests (start) | 603 passing, 0 failing, 22 suites |
| Production source files parsing clean | engine_app.c, juno_broker.c: parse errors; juno_buff_queue.c: parse errors |
| Phases complete | 1–3, 5–17 |

---

## 2. Sprint Goal

Fix vtable call resolution failures on production C source files. Two specific calls were reported broken:
1. `ptCmdPipeApi->Dequeue(...)` in `engine_app.c:223` — not resolving to implementation
2. `ptRecvQueue->ptApi->Enqueue(...)` in `juno_broker.c:68` — not resolving to implementation

---

## 3. Root Cause Analysis

Diagnostic analysis (WI-17.1) of 3 production source files identified 3 root causes:

| # | Root Cause | Affected File | Description |
|---|-----------|---------------|-------------|
| RC-1 | `{0}` compound literal in macro arg | `juno_buff_queue.c` | `JUNO_ERR_RESULT(JUNO_STATUS_ERR, {0})` — braced `{0}` has no parser path in `primaryExpression`. The entire file failed to parse, so no vtable data was extracted. |
| RC-2 | `(identifier) &&` mis-parsed as C cast | `engine_app.c` | `JUNO_ASSERT_EXISTS_MODULE(ptFoo && ptBar, ...)` — the `(ptFoo` prefix triggers `castExpression` BACKTRACK, and `&&` after `)` is not a valid unaryExpression start, causing parse failure. |
| RC-3 | `reindexFile()` drops deferred vtables | `workspaceIndexer.ts` | `reindexFile()` called `indexFile()` without a `DeferredPositional[]` array and never called `resolveDeferred()`. Cross-file positional vtable initializers were silently dropped during incremental reindexing. |

Additional fix found during verification:

| # | Root Cause | Affected File | Description |
|---|-----------|---------------|-------------|
| RC-4 | EOF sentinel check incorrect | `parser.ts` | `t.tokenType === undefined` is not the correct EOF check for Chevrotain v12. On malformed input (unclosed parens), this causes either TypeError or infinite loop. Fixed to `tokenMatcher(t, EOF)`. |
| RC-5 | FloatingLiteral missing pattern | `lexer.ts` | `[0-9]+[eE][+-]?[0-9]+[fFlL]?` pattern missing for `500E3` form. |

---

## 4. Work Breakdown

| # | Work Item | Description | Worker | Verifier(s) | Status |
|---|-----------|-------------|--------|-------------|--------|
| **WI-17.1** | Diagnostic analysis | Create diagnostic test file; parse 3 production files; identify root causes | Lead (diagnostic) | — | ✅ Complete |
| **WI-17.2** | Fix A: Parser fixes | `looksLikeCast()` fast lookahead for castExpression; braced initializer as primaryExpression alternative; initializer GATE; FloatingLiteral pattern | software-developer | senior-software-engineer | ✅ Complete |
| **WI-17.2b** | Fix B: reindexFile deferred resolution | Pass `DeferredPositional[]` to `indexFile()`, call `resolveDeferred()` after | software-developer | senior-software-engineer | ✅ Complete |
| **WI-17.2c** | Fix C: EOF sentinel | Replace `t.tokenType === undefined` with `tokenMatcher(t, EOF)` in 4 locations | software-developer | — | ✅ Complete |
| **WI-17.4** | Regression tests | 15 regression tests covering all 5 fixes + end-to-end vtable resolution | software-test-engineer | — | ✅ Complete |
| **WI-17.5** | Documentation | Sprint 17 document + SDP update | software-developer | — | ✅ Complete |

### Execution Order

```
Phase 1 — Diagnostic:
  WI-17.1 (diagnostic file, root cause identification)
  ── Result: 3 root causes identified (parser×2, indexer×1) ──   ✅

Phase 2 — Source Fixes (serialized):
  WI-17.2 (parser: looksLikeCast, braced init, initializer GATE, FloatingLiteral)
  WI-17.2b (indexer: reindexFile deferred resolution)
  ── gate: 3 files parse 0 errors, vtable resolution works ──   ✅ PASSED

Phase 3 — Verification + Fix:
  Senior engineer review: NEEDS CHANGES (EOF sentinel bug)
  WI-17.2c (EOF sentinel fix)
  ── gate: All fixes verified, 623 tests pass ──                 ✅ PASSED

Phase 4 — Regression Tests:
  WI-17.4 (15 regression tests)
  Diagnostic file cleanup (diagnostic-sprint17.test.ts removed)
  ── gate: 618 tests pass ──                                     ✅ PASSED

Phase 5 — Documentation:
  WI-17.5 (this document + SDP update)
```

---

## 5. Acceptance Criteria

| # | Criterion | Status |
|---|-----------|--------|
| 1 | `juno_buff_queue.c` parses with 0 Chevrotain errors | ✅ Met |
| 2 | `engine_app.c` parses with 0 Chevrotain errors | ✅ Met |
| 3 | `juno_broker.c` parses with 0 Chevrotain errors | ✅ Met |
| 4 | `ptRecvQueue->ptApi->Enqueue(...)` at broker:68 resolves to `JunoDs_QueuePush` | ✅ Met |
| 5 | `ptCmdPipeApi->Dequeue(...)` at engine_app:223 resolves to `JunoDs_QueuePop` | ✅ Met |
| 6 | No regressions in existing tests | ✅ Met |
| 7 | Regression tests cover all 5 root cause fixes | ✅ Met |
| 8 | EOF sentinel safe for malformed input | ✅ Met |

---

## 6. Final Metrics

| Metric | Start | End |
|--------|-------|-----|
| Test suites | 22 | 23 |
| Test count | 603 | 618 |
| New tests | — | 15 |
| Production source parse errors | 3 files failing | 0 errors |
| Vtable resolution (broker Enqueue) | FAIL | PASS |
| Vtable resolution (engine Dequeue) | FAIL | PASS |
| `tsc --noEmit` | Clean | Clean |
| Phases complete | 1–3, 5–17 | 1–3, 5–18 |

---

## 7. Technical Design Decisions

### 7.1 `looksLikeCast()` Fast Lookahead

**Problem:** `(identifier)` expressions were ambiguously parsed. `castExpression` used a `BACKTRACK(castTypePrefix)` that succeeded for `(JUNO_TYPE_T *)ptr` but also for `(ptFoo)` in `(ptFoo && ptBar)`, where `&&` cannot start a unaryExpression.

**Solution:** Added `looksLikeCast()` that scans past the matching `)` and checks if the following token can start a cast operand. If the next token is `;`, `,`, `)`, `&&`, `||`, `?`, `:`, `]`, `}`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `<<`, `>>` — it's NOT a cast. The GATE rejects the cast alternative before the expensive BACKTRACK runs.

### 7.2 Braced Initializer in `primaryExpression`

**Problem:** `{0}` appearing as a macro argument (e.g., `JUNO_ERR_RESULT(STATUS, {0})`) had no parser path. Braces were only handled in the `initializer` rule.

**Solution:** Added `LBrace initializerList Comma? RBrace` as a new alternative in `primaryExpression`. The `initializer` rule got a GATE (`!tokenMatcher(LA(1), LBrace)`) on its `assignmentExpression` alternative to prevent ambiguity.

### 7.3 `reindexFile()` Deferred Resolution

**Problem:** `fullIndex()` passes `DeferredPositional[]` to `indexFile()` and calls `resolveDeferred()` after all files. But `reindexFile()` — used on file-save events — called `indexFile()` without a deferred array. The `mergeInto()` function has an `if (deferred)` guard that silently drops `pendingPositionalVtables` when no array is provided.

**Solution:** Added 3 lines to `reindexFile()`:
```typescript
const deferred: DeferredPositional[] = [];
await this.indexFile(filePath, deferred);
this.resolveDeferred(deferred);
```

### 7.4 EOF Sentinel Fix

**Problem:** `t.tokenType === undefined` is not correct for Chevrotain v12 EOF detection. `this.LA(i)` past the token vector may return `undefined` (causing TypeError on property access) or a sentinel token with a valid `tokenType` (causing infinite scan loop).

**Solution:** Replaced with `tokenMatcher(t, EOF)` in all 4 locations: `looksLikeCast()` (2 checks), `isMacroCallWithKeywordArg()` (1 check), `gobbleMacroCallStatement()` (1 check).

---

## 8. Verification Summary

### Worker Agents Spawned

| # | Agent Type | Task | Files Changed | Iterations |
|---|-----------|------|---------------|------------|
| 1 | software-developer | WI-17.2: Parser fixes | parser.ts, lexer.ts | 1 |
| 2 | software-developer | WI-17.2b: reindexFile deferred resolution | workspaceIndexer.ts | 1 |
| 3 | software-developer | WI-17.2c: EOF sentinel fix | parser.ts | 1 |
| 4 | software-test-engineer | WI-17.4: Regression tests | sprint17-regression.test.ts | 1 |

### Verifier Agents Spawned

| # | Agent Type | Scope | Verdict | Key Findings |
|---|-----------|-------|---------|---------------|
| 1 | senior-software-engineer | WI-17.2 + WI-17.2b | NEEDS CHANGES → APPROVED (after WI-17.2c) | Found EOF sentinel bug in `looksLikeCast()` — `t.tokenType === undefined` unsafe for Chevrotain v12. Also noted missing negative-path test for `looksLikeCast()` (info, not blocking). |

---

## 9. Files Created/Modified This Sprint

### Modified Files

| File | Changes |
|------|---------|
| `src/parser/parser.ts` | `looksLikeCast()` fast lookahead; braced initializer in `primaryExpression`; GATE in `initializer`; EOF sentinel fix (4 locations); `import { EOF }` |
| `src/parser/lexer.ts` | FloatingLiteral pattern extended with `[0-9]+[eE][+-]?[0-9]+[fFlL]?` |
| `src/indexer/workspaceIndexer.ts` | `reindexFile()` passes `DeferredPositional[]` and calls `resolveDeferred()` |

### New Files

| File | Contents |
|------|---------|
| `src/__tests__/sprint17-regression.test.ts` | 15 regression tests: REG-17-001 through REG-17-010b |

### Removed Files

| File | Reason |
|------|--------|
| `src/__tests__/diagnostic-sprint17.test.ts` | Temporary diagnostic file — removed after root causes identified and fixed |

---

## 10. Lessons Learned

| Lesson | Details |
|--------|---------|
| Chevrotain EOF sentinel: use `tokenMatcher(t, EOF)` | `t.tokenType === undefined` is wrong for Chevrotain v12. `this.LA(i)` past token vector returns either JS `undefined` (TypeError on `.tokenType`) or a sentinel token (infinite loop). Always use `tokenMatcher(t, EOF)`. |
| `reindexFile()` must mirror `fullIndex()` deferred resolution | Any new data path in `mergeInto()` that uses deferred arrays must also be threaded through `reindexFile()`. Silent drops from `if (deferred)` guards are hard to detect. |
| Senior engineer verifiers catch latent bugs | The EOF sentinel issue was not caught by 623 tests because all tests use well-formed C input. The senior engineer identified it via code review. Always run verification on new parser lookahead code. |

---

## 11. Commit

```
:bug: Fix vtable resolution on production C source files

- Parser: looksLikeCast() prevents (identifier)&& false cast parse
- Parser: braced initializer {0} in primaryExpression for macro args
- Parser: initializer GATE prevents brace ambiguity
- Lexer: FloatingLiteral digits-exp-suffix pattern (500E3)
- Indexer: reindexFile() resolves deferred positional vtables
- Parser: EOF sentinel fix (tokenMatcher(t, EOF)) in 4 locations
- Tests: 15 regression tests (REG-17-001 through REG-17-010b)

Fixes: ptCmdPipeApi->Dequeue() and ptRecvQueue->ptApi->Enqueue()
resolution on engine_app.c and juno_broker.c.

Test count: 603 → 618 (23 suites)
```
