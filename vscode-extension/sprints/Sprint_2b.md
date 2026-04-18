# Sprint 2B — Production Fixes + Mutation Hardening (Revised)

**Sprint:** 2B (continuation of Sprint 2)  
**Phase:** 2 (Visitor — Vtable Init Patterns)  
**Date:** 2026-04-17 (replanned)  
**Status:** AWAITING PM APPROVAL  
**SDP Version:** 2.1

---

## 1. Sprint Startup Protocol Summary

Documents read and summarized before replan:

### 1.1 Software Lead Skill File (`ai/skills/software-lead.md`)
- Work items must produce 1–3 files max, be completable by a single agent, have explicit acceptance criteria
- Parallelization safe only when workers edit different files with no shared dependencies
- Must serialize when Worker A's output is Worker B's input
- Test verification requires both `software-quality-engineer` and `software-verification-engineer`
- Code verification requires `software-quality-engineer` + `senior-software-engineer`

### 1.2 Lessons Learned Applied
| Lesson (Date) | Application |
|----------------|------------|
| One work item = one phase (2026-04-17) | **Primary driver of replan.** Every mutant-killing batch is its own numbered phase with its own gate. No sub-items within phases. |
| ≤10 tests per agent brief (2026-04-16) | Every mutant-killing phase capped at ≤10 tests |
| Briefs must be tiny: ≤50 lines context (2026-04-16) | Each phase brief includes only the ~20-30 lines of source the agent needs |
| Fix failing tests BEFORE new ones (2026-04-16) | Phase 2 (failing test fixes) completed before mutation work |
| Fix source BEFORE tests (2026-04-14) | Phase 1 (parser fix) completed before Phase 2 |
| Verify with test execution (2026-04-14) | GATE after every single phase |
| Do not do hands-on work (2026-04-14) | All coding delegated to worker agents; Lead only orchestrates |
| Do NOT skip plan creation (2026-04-16) | This replan document created before any execution |

### 1.3 Requirements (Sprint 2B-Relevant)
- **REQ-VSCODE-003**: LibJuno API Pattern Recognition — `apiMemberRegistry` population
- **REQ-VSCODE-008**: Module Root API Discovery — `walkMacroBodyForApiMembers`
- **REQ-VSCODE-016**: Failure Handler Navigation — `tryExtractFailureHandler`

### 1.4 Software Design
- Chevrotain parser + CST visitor produces `ParsedFile` with `vtableAssignments`, `failureHandlerAssigns`, and `localTypeInfo`
- `macroBodyTokens` parser rule consumes all tokens between balanced parentheses in JUNO macro bodies
- `walkMacroBodyForApiMembers` scans macro body tokens for `_API_T` pattern to populate `apiMemberRegistry`
- `tryExtractFailureHandler` requires RHS of assignment to be a plain function name

### 1.5 SDP Phase 2 Exit Criteria
- Mutation testing ≥75% on all tested production files (HARD gate)
- Coverage checkpoint ≥85% line coverage

---

## 2. Current State

| Metric | Value |
|--------|-------|
| Tests | 430 pass, 0 fail, 7 suites |
| Mutation: lexer.ts | 100.0% ✅ |
| Mutation: parser.ts | ~73.4% ❌ (need ≥75%) — 127 survived, 6 NoCov |
| Mutation: visitor.ts | ~50.1% ❌ (need ≥75%) — 123 survived, 129 NoCov |

> **Note:** Mutation scores are from the last Stryker run (before 44 new tests were added). Actual scores may be higher. Phase 3 re-baselines before targeted work begins.

### 2.1 Completed Work (from original Sprint 2B plan)

| Phase | Description | Status |
|-------|-------------|--------|
| Phase 1 | Fix `macroBodyTokens` CST recording in parser.ts | ✅ COMPLETE |
| Phase 2 | Fix 11 failing tests in visitor-mutant-killing.test.ts | ✅ COMPLETE |

---

## 3. Replan Rationale

The original plan batched multiple serial work items inside Phases 3 and 4 (3 sub-items in Phase 3, 6 sub-items in Phase 4). Per lesson learned 2026-04-17:

> "One work item = one phase. Each mutant-killing batch should be its own independent phase with its own gate. Never batch multiple serial work items into one phase."

This replan:
1. Promotes every work item to its own numbered phase
2. Adds a re-baseline phase (Phase 3) to get current mutation scores before targeting mutants
3. Adds intermediate Stryker checkpoints after each track completes
4. Separates parser.ts and visitor.ts into parallel tracks (different test files)

---

## 4. Work Breakdown Structure

### Phase 3 — Re-baseline Mutation Scores

| # | Work Item | Agent | Acceptance Criteria |
|---|-----------|-------|---------------------|
| WI-3 | Run `npx stryker run`, save report, analyze surviving mutants per function/region | `software-test-engineer` | 1. Stryker completes without error. 2. Report saved to `reports/mutation/`. 3. Per-function mutant survival counts produced for parser.ts and visitor.ts. 4. Identifies which planned phases can be skipped if scores already ≥75%. |
| **GATE-3** | Review mutant analysis | Lead | Mutation scores captured. Remaining phases adjusted if scores improved. |

> **Decision point:** If parser.ts is already ≥75% after re-baseline, skip Phases 4–6. If visitor.ts is already ≥75%, skip Phases 8–13. Proceed to Phase 15 directly for any file already at target.

---

### Track A — parser.ts Mutation Hardening

> These phases are serial (each depends on prior gate). Modifies `parser-grammar.test.ts`.

#### Phase 4 — Kill mutants: `macroBodyTokens` depth tracking (parser.ts L375-383)

| # | Work Item | Agent | Files | Acceptance Criteria |
|---|-----------|-------|-------|---------------------|
| WI-4 | Write ≤8 tests targeting GATE logic, depth++/-- mutations in `macroBodyTokens` | `software-test-engineer` | `parser-grammar.test.ts` | 1. ≤8 new tests added. 2. Tests exercise nested parentheses in macro bodies at depths 0, 1, 2+. 3. All tests pass. 4. No regressions. |
| V-4 | Verify test quality | `software-quality-engineer` | `parser-grammar.test.ts` | Standards compliance, assertion quality |
| **GATE-4** | Run `npx jest --no-coverage` | Lead | All tests pass, 0 regressions |

#### Phase 5 — Kill mutants: `declarationSpecifiers` GATE predicates (parser.ts L85-107)

| # | Work Item | Agent | Files | Acceptance Criteria |
|---|-----------|-------|-------|---------------------|
| WI-5 | Write ≤8 tests targeting GATE predicate branches in `declarationSpecifiers` | `software-test-engineer` | `parser-grammar.test.ts` | 1. ≤8 new tests added. 2. Tests exercise each LA(2) branch: Identifier+Star, Identifier+Const, Identifier+Identifier, Identifier+LParen+Star. 3. All tests pass. |
| V-5 | Verify test quality | `software-quality-engineer` | `parser-grammar.test.ts` | Standards compliance, assertion quality |
| **GATE-5** | Run `npx jest --no-coverage` | Lead | All tests pass, 0 regressions |

#### Phase 6 — Kill mutants: `specifierQualifierList` GATE predicates (parser.ts L225-247)

| # | Work Item | Agent | Files | Acceptance Criteria |
|---|-----------|-------|-------|---------------------|
| WI-6 | Write ≤8 tests targeting GATE predicate branches in `specifierQualifierList` | `software-test-engineer` | `parser-grammar.test.ts` | 1. ≤8 new tests added. 2. Tests exercise type-qualifier + type-specifier combinations through specifierQualifierList. 3. All tests pass. |
| V-6 | Verify test quality | `software-quality-engineer` | `parser-grammar.test.ts` | Standards compliance, assertion quality |
| **GATE-6** | Run `npx jest --no-coverage` | Lead | All tests pass, 0 regressions |

#### Phase 7 — parser.ts Mutation Checkpoint

| # | Work Item | Agent | Acceptance Criteria |
|---|-----------|-------|---------------------|
| WI-7 | Run Stryker on parser.ts, report score | Lead (or `software-test-engineer`) | 1. parser.ts mutation score ≥75%. 2. If <75%, identify remaining gaps for additional micro-phases. |
| **GATE-7** | parser.ts ≥75% | Lead | If YES → Track A complete. If NO → Lead creates additional targeted phases and re-enters Track A. |

---

### Track B — visitor.ts Mutation Hardening

> These phases are serial (each depends on prior gate). Modifies `visitor-mutant-killing.test.ts`.  
> **Track B can run in parallel with Track A** since they modify different test files.

#### Phase 8 — Kill mutants: helper functions (visitor.ts L22-147)

| # | Work Item | Agent | Files | Acceptance Criteria |
|---|-----------|-------|-------|---------------------|
| WI-8 | Write ≤10 tests targeting `tok()`, `child()`, `tagToType()`, `extractDeclaratorInfo()`, `drillToPostfix()` | `software-test-engineer` | `visitor-mutant-killing.test.ts` | 1. ≤10 new tests. 2. Tests exercise edge cases: missing tokens, empty children, all tag-to-type mappings, declarator with/without pointer stars. 3. All tests pass. |
| V-8 | Verify test quality | `software-quality-engineer` | `visitor-mutant-killing.test.ts` | Standards compliance, assertion quality |
| **GATE-8** | Run `npx jest --no-coverage` | Lead | All tests pass, 0 regressions |

#### Phase 9 — Kill mutants: `getPostfixSuffixes` (visitor.ts L168-206)

| # | Work Item | Agent | Files | Acceptance Criteria |
|---|-----------|-------|-------|---------------------|
| WI-9 | Write ≤10 tests targeting JunoFailureHandler/UserData token detection, postfix suffix extraction | `software-test-engineer` | `visitor-mutant-killing.test.ts` | 1. ≤10 new tests. 2. Tests exercise: arrow vs dot access, `_pfcnFailureHandler` detection, `JUNO_FAILURE_USER_DATA` detection, nested suffix chains. 3. All tests pass. |
| V-9 | Verify test quality | `software-quality-engineer` | `visitor-mutant-killing.test.ts` | Standards compliance, assertion quality |
| **GATE-9** | Run `npx jest --no-coverage` | Lead | All tests pass, 0 regressions |

#### Phase 10 — Kill mutants: `walkStatement` / `walkCompoundStatement` (visitor.ts L403-436)

| # | Work Item | Agent | Files | Acceptance Criteria |
|---|-----------|-------|-------|---------------------|
| WI-10 | Write ≤10 tests targeting control flow branches: if/else, for, while, do-while, switch, return, compound nesting | `software-test-engineer` | `visitor-mutant-killing.test.ts` | 1. ≤10 new tests. 2. Tests exercise each statement branch to kill boundary mutants. 3. All tests pass. |
| V-10 | Verify test quality | `software-quality-engineer` | `visitor-mutant-killing.test.ts` | Standards compliance, assertion quality |
| **GATE-10** | Run `npx jest --no-coverage` | Lead | All tests pass, 0 regressions |

#### Phase 11 — Kill mutants: `walkMacroBodyForApiMembers` (visitor.ts L743-761)

| # | Work Item | Agent | Files | Acceptance Criteria |
|---|-----------|-------|-------|---------------------|
| WI-11 | Write ≤10 tests targeting `_API_T` pattern matching, `apiMemberRegistry` population, token scanning in macro bodies | `software-test-engineer` | `visitor-mutant-killing.test.ts` | 1. ≤10 new tests. 2. Tests exercise: macro body with 0, 1, N `_API_T` members; non-matching tokens; pointer-star detection. 3. All tests pass. |
| V-11 | Verify test quality | `software-quality-engineer` | `visitor-mutant-killing.test.ts` | Standards compliance, assertion quality |
| **GATE-11** | Run `npx jest --no-coverage` | Lead | All tests pass, 0 regressions |

#### Phase 12 — Kill mutants: `walkApiStructBody` (visitor.ts L778-861)

| # | Work Item | Agent | Files | Acceptance Criteria |
|---|-----------|-------|-------|---------------------|
| WI-12 | Write ≤10 tests targeting API struct field extraction, function-pointer member detection, field-name extraction | `software-test-engineer` | `visitor-mutant-killing.test.ts` | 1. ≤10 new tests. 2. Tests exercise: struct with 0, 1, N fields; function-pointer vs data members; nested struct references. 3. All tests pass. |
| V-12 | Verify test quality | `software-quality-engineer` | `visitor-mutant-killing.test.ts` | Standards compliance, assertion quality |
| **GATE-12** | Run `npx jest --no-coverage` | Lead | All tests pass, 0 regressions |

#### Phase 13 — Kill mutants: `walkExpressionStatement` (visitor.ts L868-1042)

| # | Work Item | Agent | Files | Acceptance Criteria |
|---|-----------|-------|-------|---------------------|
| WI-13 | Write ≤10 tests targeting expression statement parsing: assignment detection, vtable assignment extraction, failure handler extraction, call-site detection | `software-test-engineer` | `visitor-mutant-killing.test.ts` | 1. ≤10 new tests. 2. Tests exercise: simple assignment, chained member access, function call detection, expression with no assignment. 3. All tests pass. |
| V-13 | Verify test quality | `software-quality-engineer` | `visitor-mutant-killing.test.ts` | Standards compliance, assertion quality |
| **GATE-13** | Run `npx jest --no-coverage` | Lead | All tests pass, 0 regressions |

#### Phase 14 — visitor.ts Mutation Checkpoint

| # | Work Item | Agent | Acceptance Criteria |
|---|-----------|-------|---------------------|
| WI-14 | Run Stryker on visitor.ts, report score | Lead (or `software-test-engineer`) | 1. visitor.ts mutation score ≥75%. 2. If <75%, identify remaining gaps for additional micro-phases. |
| **GATE-14** | visitor.ts ≥75% | Lead | If YES → Track B complete. If NO → Lead creates additional targeted phases and re-enters Track B. |

---

### Phase 15 — Full Mutation Test Run

| # | Work Item | Agent | Acceptance Criteria |
|---|-----------|-------|---------------------|
| WI-15 | Run full `npx stryker run`, save report | Lead | 1. Stryker completes without error. 2. Report saved to `reports/mutation/`. |
| **GATE-15** | All files ≥75% | Lead | lexer.ts ≥75%, parser.ts ≥75%, visitor.ts ≥75% |

### Phase 16 — Final Quality Gate

| # | Work Item | Agent | Acceptance Criteria |
|---|-----------|-------|---------------------|
| WI-16 | Final product verification | `final-quality-engineer` | 1. All tests pass (0 failures). 2. Mutation score ≥75% on all 3 files. 3. No regressions. 4. All acceptance criteria from every phase met. 5. Mutation report saved. |
| **GATE-16** | Final QE APPROVED | Lead | Sprint complete → present to PM |

---

## 5. Dependency Graph

```
                    Phase 3: Re-baseline (Stryker run + analysis)
                                    │
                    ┌───────────────┴───────────────┐
                    ▼                               ▼
          Track A (parser.ts)             Track B (visitor.ts)
          File: parser-grammar.test.ts    File: visitor-mutant-killing.test.ts
                    │                               │
              Phase 4                         Phase 8
              (macroBodyTokens)               (helper functions)
                    │                               │
                GATE-4                          GATE-8
                    │                               │
              Phase 5                         Phase 9
              (declarationSpecifiers)         (getPostfixSuffixes)
                    │                               │
                GATE-5                          GATE-9
                    │                               │
              Phase 6                         Phase 10
              (specifierQualifierList)        (walkStatement)
                    │                               │
                GATE-6                          GATE-10
                    │                               │
              Phase 7                         Phase 11
              (parser.ts checkpoint)          (walkMacroBodyForApiMembers)
                    │                               │
                GATE-7                          GATE-11
                    │                               │
                    │                         Phase 12
                    │                         (walkApiStructBody)
                    │                               │
                    │                           GATE-12
                    │                               │
                    │                         Phase 13
                    │                         (walkExpressionStatement)
                    │                               │
                    │                           GATE-13
                    │                               │
                    │                         Phase 14
                    │                         (visitor.ts checkpoint)
                    │                               │
                    │                           GATE-14
                    │                               │
                    └───────────┬───────────────────┘
                                ▼
                    Phase 15: Full Stryker run
                                │
                            GATE-15
                                │
                                ▼
                    Phase 16: Final Quality Gate
                                │
                            GATE-16
                                │
                                ▼
                        Present to PM
```

### Parallelization

- **Track A (Phases 4–7)** and **Track B (Phases 8–14)** can execute in parallel — they modify different test files (`parser-grammar.test.ts` vs `visitor-mutant-killing.test.ts`)
- **Within each track**, phases are serial — each depends on its predecessor's gate passing
- **Phase 15** requires both tracks to complete
- **Phase 16** requires Phase 15's gate

---

## 6. Phase Size Summary

| Phase | Target | Tests Written | File Modified | Agent |
|-------|--------|---------------|---------------|-------|
| 3 | Re-baseline | 0 | (reports only) | `software-test-engineer` |
| 4 | macroBodyTokens (parser L375-383) | ≤8 | parser-grammar.test.ts | `software-test-engineer` |
| 5 | declarationSpecifiers (parser L85-107) | ≤8 | parser-grammar.test.ts | `software-test-engineer` |
| 6 | specifierQualifierList (parser L225-247) | ≤8 | parser-grammar.test.ts | `software-test-engineer` |
| 7 | parser.ts checkpoint | 0 | (Stryker run) | Lead |
| 8 | helpers (visitor L22-147) | ≤10 | visitor-mutant-killing.test.ts | `software-test-engineer` |
| 9 | getPostfixSuffixes (visitor L168-206) | ≤10 | visitor-mutant-killing.test.ts | `software-test-engineer` |
| 10 | walkStatement (visitor L403-436) | ≤10 | visitor-mutant-killing.test.ts | `software-test-engineer` |
| 11 | walkMacroBodyForApiMembers (visitor L743-761) | ≤10 | visitor-mutant-killing.test.ts | `software-test-engineer` |
| 12 | walkApiStructBody (visitor L778-861) | ≤10 | visitor-mutant-killing.test.ts | `software-test-engineer` |
| 13 | walkExpressionStatement (visitor L868-1042) | ≤10 | visitor-mutant-killing.test.ts | `software-test-engineer` |
| 14 | visitor.ts checkpoint | 0 | (Stryker run) | Lead |
| 15 | Full Stryker run | 0 | (report) | Lead |
| 16 | Final quality gate | 0 | — | `final-quality-engineer` |

**Total new tests budget:** ≤84 (24 parser + 60 visitor)  
**Total phases:** 14 (Phases 3–16), each with exactly 1 work item and 1 gate

---

## 7. Adaptive Replanning Rules

1. **After Phase 3 (re-baseline):** If a file is already ≥75%, skip its entire track. If a file is close (e.g., 74%), reduce the number of phases for that track.
2. **After Phase 7 (parser checkpoint):** If parser.ts is still <75%, the Lead creates 1–3 additional micro-phases targeting the remaining surviving mutants before proceeding to Phase 15.
3. **After Phase 14 (visitor checkpoint):** If visitor.ts is still <75%, the Lead creates 1–3 additional micro-phases targeting the remaining surviving mutants before proceeding to Phase 15.
4. **Each additional micro-phase** follows the same pattern: 1 WI (≤10 tests, one function/region), 1 verifier, 1 gate.

---

## 8. Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Re-baseline shows scores already ≥75% | Low-Medium | Positive | Skip unnecessary track(s), proceed to final gate |
| Individual phase doesn't kill enough mutants | Medium | Low | Checkpoint phases (7, 14) catch gaps; adaptive replanning adds targeted phases |
| Equivalent mutants make ≥75% structurally unreachable | Low | Medium | Report to PM with evidence; request threshold waiver |
| Agent context overflow on brief | Low | Low | Brief size capped at ≤50 lines context, ≤10 tests, one code region |
| Tests pass but don't kill target mutants | Medium | Low | Stryker checkpoint after each track verifies actual kill count |

---

## 9. Sprint Exit Criteria

- [ ] All tests pass (0 failures)
- [ ] Mutation score ≥75% on `lexer.ts`
- [ ] Mutation score ≥75% on `parser.ts`
- [ ] Mutation score ≥75% on `visitor.ts`
- [ ] `final-quality-engineer` issues APPROVED verdict
- [ ] New mutation report saved to `reports/mutation/`
- [ ] No regressions in any previously-passing tests
