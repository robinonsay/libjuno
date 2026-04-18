# Sprint 15 — Phase 17: Parser Production Header Compatibility

**Sprint:** 15  
**Phase:** 17 (Parser: Production Header Compatibility)  
**Date:** 2026-04-18  
**Status:** COMPLETE ✅  
**SDP Version:** 3.0

---

## 1. Sprint Startup Protocol Summary

### 1.1 Documents Reviewed

| Document | Key Points |
|----------|-----------|
| `ai/skills/software-lead.md` | Orchestration protocol, work item sizing, verifier assignments |
| `ai/memory/lessons-learned-software-lead.md` | Applied: one WI = one phase, fix before test, never hands-on, verify by running tests, use absolute paths, serialized fix-verify-test plan |
| `software-development-plan.md` (v3.0) | Phase 17 scope: fix 6/29 production header parse failures |
| `design/design.md` | §2.1 Chevrotain parser, §3 Architecture — parser component |
| `requirements/vscode-extension/requirements.json` | REQ-VSCODE-003 (Pattern Recognition), REQ-VSCODE-001 (Extension) |

### 1.2 Lessons Learned Applied

| Lesson | Application |
|--------|------------|
| One work item = one phase (2026-04-17) | Each fix is its own serialized step with gate |
| Fix all failing tests BEFORE writing new ones (2026-04-16) | Fix source (WI-15.1, WI-15.2) → gate → then write tests (WI-15.4, WI-15.5) |
| Never do hands-on work; delegate everything (2026-04-14) | All fixes delegated to worker agents |
| Verify worker output by running tests (2026-04-14) | WI-15.3 is explicit test-run gate between source fixes and test writing |
| Always use absolute paths (2026-04-17) | All agent briefs use absolute paths |

### 1.3 Baseline (Sprint Start)

| Metric | Value |
|--------|-------|
| Tests (start) | 592 passing, 0 failing, 22 suites |
| Headers parsing clean | 23/29 (6 failing) |
| Phases complete | 1, 2, 3, 5–14 |
| Phase 4 | REMOVED (PM decision) |

---

## 2. Sprint Goal

Fix all 6 production headers that fail Chevrotain parsing, achieving 29/29 clean parses with 0 errors. The 6 failing headers are: `array_api.h`, `queue_api.h`, `memory_api.h`, `pointer_api.h`, `broker_api.h`, `sm_api.h`.

---

## 3. Root Cause Analysis

Diagnostic analysis (read-only agent) of the 6 failing headers identified two root causes:

### 3.1 Primary: `JUNO_ASSERT_SUCCESS(status, return expr)` (12 occurrences, all 6 headers)

The parser treated `JUNO_ASSERT_SUCCESS(...)` as a function call via `expressionStatement → postfixExpression → argumentExpressionList`. The `argumentExpressionList` expects expressions, but `return` is a C keyword that cannot start any expression — it can only appear in a `jumpStatement`.

**Affected functions across all 6 headers:**

| Header | Failing Function(s) |
|--------|---------------------|
| `array_api.h` | `JunoDs_ArrayVerify`, `JunoDs_ArrayVerifyIndex` |
| `queue_api.h` | `JunoDs_QueueVerify` |
| `memory_api.h` | `JunoMemory_AllocVerify` |
| `pointer_api.h` | `JunoMemory_PointerVerify` |
| `broker_api.h` | `JunoSb_BrokerVerify`, `JunoSb_PipeVerify` |
| `sm_api.h` | `JunoSm_Init`, `JunoSm_GetCurrentState`, `JunoSm_TransitionState` |

### 3.2 Secondary: `JUNO_MODULE_ROOT(void, ...)` (1 occurrence, sm_api.h only)

The `junoModuleRootMacro` parser rule used `this.CONSUME(Identifier)` for the first argument (the API type). When the argument is `void`, the lexer produces a `Void` keyword token, not an `Identifier`. The rule failed.

### 3.3 Constructs that already worked (no fix needed)

- `JUNO_ASSERT_EXISTS(expr)` — parsed as function call
- `JUNO_FAIL_ROOT(status, mod, "msg")` — parsed as function call with 3 expression args
- Cast expressions `(TYPE *) ptr` — handled by `castExpression` with `BACKTRACK`
- Arrow/dot member access — handled by `postfixExpression`

---

## 4. Work Breakdown

| # | Work Item | Description | Worker | Verifier(s) | Status |
|---|-----------|-------------|--------|-------------|--------|
| **WI-15.0** | Diagnostic analysis | Read-only analysis of 6 failing headers; identify root causes | junior-software-developer | — | ✅ Complete |
| **WI-15.1** | Fix A: macroCallStatement | Detect `Identifier(` with keyword args via GATE lookahead; gobble via balanced-paren consumption | software-developer | senior-software-engineer | ✅ Complete |
| **WI-15.2** | Fix B: void keyword in macro | Expand `junoModuleRootMacro` OR to accept `Void`; visitor fallback extraction | software-developer | senior-software-engineer | ✅ Complete |
| **WI-15.3** | Verification gate: 29/29 | Run bulk-headers test + ts-node diagnostic; confirm 0 errors on all 29 headers | Lead (gate) | — | ✅ Complete |
| **WI-15.4** | Grammar tests for macroCallStatement | TC-MACRO-STMT-001–006, NEG-001, BND-001 (8 tests) | software-test-engineer | software-verification-engineer | ✅ Complete |
| **WI-15.5** | Visitor test for void arg | TC-MODROOT-VOID-001 (1 test) | software-test-engineer | software-verification-engineer | ✅ Complete |
| **WI-15.6** | Bulk-headers zero-error assertion | TC-BULK-004 — explicit 0-error assertion per header | software-developer | software-quality-engineer | ✅ Complete |
| **WI-15.7** | SDP update | Phase 17 section, test counts, Sprint 15 row | junior-software-developer | software-verification-engineer | ✅ Complete |
| **WI-15.8** | Verifier warning remediation | TC-MACRO-STMT-009 mutation guard + TC-BULK-004 vacuous-pass guard | software-test-engineer | — | ✅ Complete |

### Execution Order

```
Phase 1 — Diagnostic:
  WI-15.0 (read-only analysis)
  ── Result: 2 root causes identified ──                         ✅

Phase 2 — Source Fixes (serialized — both edit parser.ts):
  WI-15.1 (macroCallStatement) → 23/29 → 28/29
  WI-15.2 (void macro arg) → 28/29 → 29/29
  ── gate WI-15.3: 29/29 headers, 592 tests pass, tsc clean ──  ✅ PASSED

Phase 3 — Tests (parallel — no shared files):
  WI-15.4 (grammar tests) + WI-15.5 (visitor test)
  ── gate: 601 tests pass ──                                     ✅ PASSED

Phase 4 — Bulk-headers assertion + SDP:
  WI-15.6 (TC-BULK-004) → 602 tests
  ── gate: 602 tests pass ──                                     ✅ PASSED

Phase 5 — Verification + Remediation:
  Senior software engineer: APPROVED (0 errors, 0 warnings)
  Software verification engineer: APPROVED (3 warnings)
  WI-15.8 (address warnings) → 603 tests
  Temp file cleanup (count-errors-temp.test.ts removed)
  Final quality engineer: APPROVED (7/7 criteria, 4 SDP count fixes)
  ── Final gate: 603 tests, 22 suites, tsc clean ──              ✅ PASSED

Phase 6 — Documentation:
  WI-15.7 (SDP update) + SDP count corrections
  Lessons-learned updated
  Git committed
```

---

## 5. Acceptance Criteria

| # | Criterion | Status |
|---|-----------|--------|
| 1 | All 29 production headers parse with 0 Chevrotain errors | ✅ Met |
| 2 | All existing tests pass (no regressions) — was 592, now 603 | ✅ Met |
| 3 | New grammar tests cover `JUNO_ASSERT_SUCCESS(status, return expr)` | ✅ Met |
| 4 | New grammar test covers `JUNO_MODULE_ROOT(void, ...)` | ✅ Met |
| 5 | Bulk-headers test explicitly asserts 0 parse errors per header | ✅ Met |
| 6 | SDP updated with Phase 17 outcome | ✅ Met |
| 7 | TypeScript compiles cleanly | ✅ Met |

---

## 6. Final Metrics

| Metric | Start | End |
|--------|-------|-----|
| Test suites | 22 | 22 |
| Test count | 592 | 603 |
| New tests | — | 11 |
| Headers parsing clean | 23/29 | 29/29 |
| `tsc --noEmit` | Clean | Clean |
| Phases complete | 1–3, 5–14 | 1–3, 5–14, 17 |

---

## 7. Technical Design Decisions

### 7.1 Plain Method vs Chevrotain RULE for Token Gobbling

**Problem:** Creating `macroCallStatement = this.RULE(...)` that called `SUBRULE(this.macroBodyTokens)` caused `RangeError: Maximum call stack size exceeded` in Chevrotain's `performSelfAnalysis()`. The `macroBodyTokens` MANY loop accepts any token, creating infinite path expansion when reachable from `statement → compoundStatement → statement`.

**Solution:** Use a plain TypeScript private method (`gobbleMacroCallStatement()`) with an early return during `RECORDING_PHASE`. The GATE (`isMacroCallWithKeywordArg()`) provides the lookahead predicate. Chevrotain never needs to analyze the alternative body.

### 7.2 GATE Lookahead Implementation

The `isMacroCallWithKeywordArg()` method scans ahead from LA(3) with paren-depth tracking:
- Starts at `depth = 1` (LA(2) is the opening `LParen`)
- Scans for `Return`, `Break`, `Continue`, `Goto` tokens at any nesting depth
- Returns `true` immediately on keyword detection
- Returns `false` on matching RParen (depth drops to 0) or EOF

**False positive immunity:** `tokenMatcher(t, Return)` matches the `Return` token type, not string content. Identifiers like `return_value` tokenize as `Identifier` and are invisible to the GATE.

### 7.3 Visitor Void Token Extraction

The visitor's `dispatchMacro` method was updated to fall back to the `Void` CST key:
```typescript
const apiType = (tok(m.children, "Identifier") ?? tok(m.children, "Void"))?.image ?? "";
```
This correctly extracts `apiType: "void"` for `JUNO_MODULE_ROOT(void, ...)`.

---

## 8. Verification Summary

### Worker Agents Spawned

| # | Agent Type | Task | Files Changed | Iterations |
|---|-----------|------|---------------|------------|
| 1 | junior-software-developer | Diagnose 6 failing headers | — (read-only) | 1 |
| 2 | software-developer | WI-15.1: macroCallStatement | parser.ts | 1 |
| 3 | software-developer | WI-15.2: void keyword in macro | parser.ts, visitor.ts | 1 |
| 4 | software-test-engineer | WI-15.4: grammar tests | parser-grammar.test.ts | 1 |
| 5 | software-test-engineer | WI-15.5: visitor test | visitor-structs.test.ts | 1 |
| 6 | software-developer | WI-15.6: bulk-headers assertion | bulk-headers.test.ts | 1 |
| 7 | junior-software-developer | WI-15.7: SDP update | software-development-plan.md | 1 |
| 8 | software-test-engineer | WI-15.8: mutation guard + vacuous-pass guard | parser-grammar.test.ts, bulk-headers.test.ts | 1 |

### Verifier Agents Spawned

| # | Agent Type | Scope | Verdict | Key Findings |
|---|-----------|-------|---------|---------------|
| 1 | senior-software-engineer | WI-15.1 + WI-15.2 parser fixes | APPROVED | 0 errors, 0 warnings. EOF handling correct, GATE priority verified, no false positive risk |
| 2 | software-verification-engineer | WI-15.4 + WI-15.5 + WI-15.6 tests | APPROVED | 3 warnings: (1) parseFileWithDefs doesn't expose errors — mutation guard needed, (2) TC-BULK-004 vacuous pass risk, (3) REQ-VSCODE-001 tag convention mismatch |
| 3 | final-quality-engineer | Full sprint output | APPROVED | 7/7 criteria met. 4 minor SDP count inaccuracies flagged and corrected |

### Warnings Addressed

| Warning | Source | Resolution |
|---------|--------|-----------|
| No zero-error assertion in macro tests | Verification engineer | Added TC-MACRO-STMT-009 (CLexer/CParser direct test) |
| TC-BULK-004 vacuous pass if empty dir | Verification engineer | Added `expect(headers.length).toBeGreaterThanOrEqual(29)` |
| SDP counts off by 1 | Final QE | Corrected: 156→157, 602→603, test range corrected |

---

## 9. Files Created/Modified This Sprint

### Modified Files

| File | Changes |
|------|---------|
| `src/parser/parser.ts` | `isMacroCallWithKeywordArg()` lookahead method; `gobbleMacroCallStatement()` token gobbler; GATE in `statement` rule; `junoModuleRootMacro` accepts `Void` |
| `src/parser/visitor.ts` | `apiType` extraction falls back to `Void` token key |
| `src/parser/__tests__/parser-grammar.test.ts` | +9 tests: TC-MACRO-STMT-001–006, NEG-001, BND-001, 009 |
| `src/parser/__tests__/visitor-structs.test.ts` | +1 test: TC-MODROOT-VOID-001 |
| `src/__tests__/bulk-headers.test.ts` | +1 test: TC-BULK-004 (29 headers × 0 errors); count guard added |
| `software-development-plan.md` | Phase 17 section, Sprint 15 row, test counts updated |
| `ai/memory/lessons-learned-software-lead.md` | 2 new lessons: Chevrotain RULE vs plain method; worker temp file cleanup |

---

## 10. Lessons Learned

| Lesson | Details |
|--------|---------|
| Chevrotain RULE vs plain method for token gobbling | Creating a RULE that calls macroBodyTokens from within the recursive statement chain causes `performSelfAnalysis` stack overflow. Use plain private methods with `RECORDING_PHASE` guard instead. |
| Worker agents may leave temp diagnostic files | A worker left `count-errors-temp.test.ts` inflating test counts. Always scan for unexpected test files after worker execution. |
| GATE lookahead with depth-tracked keyword detection | Scanning for specific keyword tokens at any paren depth is safe for macro detection — false positives impossible because identifiers tokenize differently from keywords. |

---

## 11. Commit

```
:bug: Fix 6/29 production header parse failures — 29/29 clean

Sprint 15 / Phase 17: Parser Production Header Compatibility

Fixes:
- macroCallStatement: handle macro calls with keyword args (return/break/
  continue/goto) via balanced-paren token gobbling with GATE lookahead
- junoModuleRootMacro: accept Void keyword as first argument
- visitor: extract apiType from Void token fallback

Tests (11 new, 603 total):
- TC-MACRO-STMT-001–006, NEG-001, BND-001, 009
- TC-MODROOT-VOID-001
- TC-BULK-004 (29 headers × 0 errors assertion)
```

Commit hash: `7d2cf50f`
