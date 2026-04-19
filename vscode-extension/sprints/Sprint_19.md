# Sprint 19 — Phase 19: Go-to-Definition: C11 Parser Grammar Fix

**Sprint:** 19
**Phase:** 19 (Go-to-Definition: C11 Parser Grammar Fix)
**Date:** 2026-04-18
**Status:** COMPLETE ✅
**SDP Version:** 4.1

---

## 1. Sprint Startup Protocol Summary

### 1.1 Documents Reviewed

| Document | Key Points |
|----------|-----------|
| `ai/skills/software-lead.md` | Orchestration protocol, work item sizing, verifier assignments |
| `ai/memory/lessons-learned-software-lead.md` | Applied: delegate all work, every plan must show worker/verifier assignment table |
| `software-development-plan.md` (v4.1) | All 18 phases complete; Sprint 19 is bug fix for Go-to-Definition |
| `design/design.md` | §2.1 Chevrotain parser, §3 Architecture, §5.2 VtableResolver |
| `requirements/vscode-extension/requirements.json` | REQ-VSCODE-005 (vtable navigation), REQ-VSCODE-003 (pattern recognition) |

### 1.2 Lessons Learned Applied

| Lesson | Application |
|--------|------------|
| Never do hands-on work; delegate everything (2026-04-14) | All diagnosis and fixes delegated to worker agents |
| Every plan must include worker/verifier assignment table (2026-04-18) | Applied to all phase plans |
| Verify worker output by running tests (2026-04-14) | Full npm test run after every code-changing work item |
| Fix all failing tests BEFORE writing new ones (2026-04-16) | Source fix (Phase 2) completed before regression tests were declared GREEN |

### 1.3 Baseline (Sprint Start)

| Metric | Value |
|--------|-------|
| Tests (start) | 622 passing, 0 failing, 23 suites |
| Bug | ctrl-click `Reset` in `tIndexPointer.ptApi->Reset(tIndexPointer)` navigates to line 53 (vtable initializer entry) instead of line 88 (function definition) in `engine_cmd_msg.c` |
| Phases complete | 1–3, 5–18 |

---

## 2. Sprint Goal

Fix Go-to-Definition navigating to vtable struct initializer line instead of function definition line. Specifically: `tIndexPointer.ptApi->Reset(tIndexPointer)` in `engine_cmd_msg.c` jumped to line 53 (the positional initializer in `gtEngineCmdMsgPointerApi`) instead of line 88 (the actual `EngineCmdMsg_Reset` function definition). Implement a robust, long-term three-layer fix: (1) correct the grammar rule, (2) audit all C11 expression/statement rules for similar deviations, (3) add a permanent parser conformance test suite.

---

## 3. Root Cause Analysis

One root cause identified:

| # | Root Cause | Affected File | Description |
|---|-----------|---------------|-------------|
| RC-1 | `unaryExpression` grammar rule violated C11 §6.5.3 | `src/parser/parser.ts` | The rule recursed back into `unaryExpression` for ALL unary operators (`& * + - ~ ! ++ --`). Per C11, only `++`/`--` take a `unary-expression` operand; `& * + - ~ !` must recurse to `cast-expression`. This caused parse failure on any function body containing `*(T *) ptr = ...` (cast-dereference). Affected files emitted zero `FunctionDefinitionRecord` entries. The indexer then fell back to the vtable initializer line (`r.line` = 53) in `resolveDeferred()`, since `resolveDefinitionLocation()` returned `line: 0`. |

C11 grammar audit (WI-2.2) confirmed no additional deviations in postfix, cast, compound literal, designated initializer, assignment, or compound statement rules.

---

## 4. Work Breakdown

| # | Work Item | Description | Worker | Verifier(s) | Status |
|---|-----------|-------------|--------|-------------|--------|
| **WI-0.1** | Repro fixture | Create `test/fixtures/go-to-def-bug/fixture_api.h` and `fixture_impl.c` — positional vtable init + function defs with bug-inducing body patterns | junior-software-developer | software-quality-engineer | ✅ Complete |
| **WI-0.2** | Root cause analysis | Read parser.ts, visitor.ts, workspaceIndexer.ts; trace why line 53 is returned; identify fix | software-developer | senior-software-engineer | ✅ Complete |
| **WI-0.3** | Baseline confirmation | Run npm test, confirm 622/622 green | junior-software-developer | senior-software-engineer | ✅ Complete |
| **WI-1.1** | TC-WI-019 parser unit test | `go-to-def-bug.test.ts` — assert `FunctionDefinitionRecord` emitted at correct lines for fixture | software-test-engineer | software-verification-engineer, senior-software-engineer | ✅ Complete (RED) |
| **WI-1.2** | TC-WI-020 E2E test | `go-to-def-bug-e2e.test.ts` — E2E resolution via WorkspaceIndexer for fixture (line 48) AND real `engine_cmd_msg.c` (line 88) | software-test-engineer | software-verification-engineer, senior-software-engineer | ✅ Complete (RED) |
| **WI-1.3** | C11 conformance skeleton | `c11-grammar-conformance.test.ts` — 21 cases covering C11 expression/statement patterns; 9 RED before fix | software-test-engineer | software-verification-engineer, senior-software-engineer | ✅ Complete (partially RED) |
| **WI-2.1** | Fix unaryExpression | Split `++`/`--` (→ `unaryExpression`) from `& * + - ~ !` (→ `castExpression`) in parser.ts per C11 §6.5.3 | software-developer | senior-software-engineer, software-systems-engineer | ✅ Complete |
| **WI-2.2** | C11 grammar audit | Audit all expression/statement rules in parser.ts; no additional deviations found | software-developer | senior-software-engineer, software-systems-engineer, software-quality-engineer | ✅ Complete |
| **WI-2.3** | TSDoc on modified functions | Inline C11 §6.5.3 citation on unaryExpression RULE | junior-software-developer | software-quality-engineer | ✅ Complete |
| **WI-3.1** | Real-file E2E | Confirm TC-WI-020b passes: `EngineCmdMsg_Reset` resolves to line 88 in real `engine_cmd_msg.c` | software-developer | senior-software-engineer | ✅ Complete |
| **WI-4.1** | Requirements update | Tighten REQ-VSCODE-005 (explicit "function definition line"); add REQ-VSCODE-033 (C11 parser conformance); update REQ-VSCODE-003 reciprocal `implements` link | software-requirements-engineer | software-systems-engineer, software-verification-engineer | ✅ Complete |
| **WI-4.2** | Insert @req tags | Add `@{"req": ["REQ-VSCODE-005", "REQ-VSCODE-033"]}` on modified parser.ts lines | junior-software-developer | software-verification-engineer | ✅ Complete |
| **WI-4.3** | Insert @verify tags | Add `@{"verify": [...]}` tags on all new test functions in 3 new test files | junior-software-developer | software-verification-engineer | ✅ Complete |
| **WI-4.4** | Rebuild + SDP + cleanup | `npm run compile`, `vsce package` → 0.1.4; SDP Sprint 19 entry; remove diagnostic scratch files | junior-software-developer | software-quality-engineer | ✅ Complete |
| **FQ** | Full quality check | Full suite 645/645 green; compile clean; traceability passes; VSIX 0.1.4 built; no stray files | final-quality-engineer | — | ✅ APPROVED |

### Execution Order

```
Phase 0 — Diagnostic (WI-0.1 ∥ WI-0.2 → WI-0.3)
  Gate A: PM reviews root cause before Phase 1
Phase 1 — RED tests (serial)
  Gate B: confirm 9 tests RED, 636 GREEN, 0 regressions
Phase 2 — Source fix (serial: WI-2.1 → WI-2.2 → WI-2.3)
  Gate C: 645/645 GREEN
Phase 3 — E2E verification
Phase 4 — Traceability + docs (WI-4.1 → WI-4.2 ∥ WI-4.3 ∥ WI-4.4)
Phase 5 — Final quality gate
```

---

## 5. Acceptance Criteria

| # | Criterion | Status |
|---|-----------|--------|
| 1 | `EngineCmdMsg_Reset` resolves to line 88 (function definition), not line 53 (vtable initializer) | ✅ Met |
| 2 | All 622 baseline tests continue to pass | ✅ Met |
| 3 | TC-WI-019, TC-WI-020a, TC-WI-020b all GREEN after fix | ✅ Met |
| 4 | C11 conformance suite: all 21 cases GREEN after fix | ✅ Met |
| 5 | REQ-VSCODE-005 tightened to explicitly require navigation to function definition line | ✅ Met |
| 6 | REQ-VSCODE-033 added for C11 parser conformance | ✅ Met |
| 7 | `python3 scripts/verify_traceability.py` passes (sprint scope) | ✅ Met |
| 8 | `libjuno-0.1.4.vsix` built with chevrotain runtime bundled | ✅ Met |

---

## 6. Final Metrics

| Metric | Start | End |
|--------|-------|-----|
| Test suites | 23 | 26 |
| Test count | 622 | 645 |
| New tests | — | 23 |
| Go-to-def: `Reset` in engine_cmd_msg.c | Line 53 (WRONG) | Line 88 (CORRECT) |
| `npm run compile` | Clean | Clean |
| VSIX version | 0.1.2 | 0.1.4 |
| Phases complete | 1–3, 5–18 | 1–3, 5–19 |

---

## 7. Technical Design Decisions

### 7.1 C11 §6.5.3 Grammar Fix

**Problem:** The `unaryExpression` Chevrotain RULE had a single OR branch for all prefix unary operators (`& * + - ~ ! ++ --`), recursing into `unaryExpression` for all of them. Per C11 §6.5.3, the grammar is:
```
unary-expression:
    ++ unary-expression
    -- unary-expression
    unary-operator cast-expression    ← & * + - ~ !
    ...
```
Because `& * + - ~ !` require a `cast-expression` operand (not another `unary-expression`), expressions like `*(ENGINE_CMD_MSG_T *) tPointer.pvAddr` could not be parsed: after consuming `*`, the parser tried to match another `unaryExpression`, but `(ENGINE_CMD_MSG_T *)` is a cast prefix, not a valid `unaryExpression` start.

**Solution:** Split into two OR alternatives:
- `++` / `--` → SUBRULE `unaryExpression` (unchanged)
- `& * + - ~ !` → SUBRULE `castExpression` (new, C11-compliant)

Required bumping the Chevrotain OR ordinal from OR3 to OR4 for the `sizeof` inner branch to preserve unique ordinals within the RULE.

### 7.2 Three-Layer Fix Strategy

Rather than patching only the immediate failure, a three-layer approach was chosen for long-term robustness:
- Layer 1: Fix the single grammar rule (immediate, targeted)
- Layer 2: Audit all other expression/statement rules against C11 spec (no additional deviations found)
- Layer 3: Add `c11-grammar-conformance.test.ts` as a permanent regression guard (21 C11 patterns that must survive future parser changes)

### 7.3 REQ-VSCODE-033 (New Requirement)

The C11 conformance test suite needed a requirements anchor. REQ-VSCODE-033 was added with `verification_method: "Test"` and `uses: ["REQ-VSCODE-003"]` (pattern recognition). REQ-VSCODE-003's `implements` array was updated to include the new REQ for bidirectional consistency, required by `verify_traceability.py`.

---

## 8. Verification Summary

### Worker Agents Spawned

| # | Agent Type | Task | Files Changed | Iterations |
|---|-----------|------|---------------|------------|
| 1 | junior-software-developer | WI-0.1: Repro fixture | fixture_api.h, fixture_impl.c | 1 |
| 2 | software-developer | WI-0.2: Root cause analysis | (read-only) | 1 |
| 3 | junior-software-developer | WI-0.3: Baseline run | (command only) | 1 |
| 4 | software-test-engineer | WI-1.1/1.2/1.3: RED regression + conformance tests | go-to-def-bug.test.ts, go-to-def-bug-e2e.test.ts, c11-grammar-conformance.test.ts | 1 |
| 5 | software-developer | WI-2.1: unaryExpression fix | parser.ts | 1 |
| 6 | software-developer | WI-2.2: C11 grammar audit | (read-only, no changes) | 1 |
| 7 | junior-software-developer | WI-2.3: TSDoc/inline comments | parser.ts | 1 |
| 8 | software-developer | WI-3.1: Real-file E2E confirmation | (test execution) | 1 |
| 9 | software-requirements-engineer | WI-4.1: Requirements update | requirements.json | 2 (rework: add REQ-VSCODE-003 reciprocal link) |
| 10 | junior-software-developer | WI-4.2: @req tags | parser.ts | 1 |
| 11 | junior-software-developer | WI-4.3: @verify tags | c11-grammar-conformance.test.ts | 1 |
| 12 | junior-software-developer | WI-4.4: Compile + package + SDP | package.json, software-development-plan.md, libjuno-0.1.4.vsix | 1 |

### Verifier Agents Spawned

| # | Agent Type | Scope | Verdict | Key Findings |
|---|-----------|-------|---------|---------------|
| 1 | software-quality-engineer | WI-0.1 fixture | APPROVED | Fixture shape mirrors engine_cmd_msg.c; bug-inducing patterns present |
| 2 | senior-software-engineer | WI-0.2 root cause | APPROVED | Root cause correctly identified; fix strategy sound |
| 3 | senior-software-engineer | Gate B (RED tests) | APPROVED | 9 RED tests match expected failure modes; 636 baseline green; zero regressions |
| 4 | software-verification-engineer | Gate B (RED tests) | APPROVED | Traceability tags present on new tests; REQ-VSCODE-005 referenced |
| 5 | senior-software-engineer | WI-2.1 grammar fix | APPROVED | C11 §6.5.3 recursion targets correct; OR ordinals unique; 645/645 green |
| 6 | software-systems-engineer | WI-2.1 grammar fix | APPROVED | Architecture preserved; no DI or integration changes |
| 7 | senior-software-engineer | WI-2.2 audit | APPROVED | All rules traced to C11 §6.5/§6.8; no additional deviations |
| 8 | software-systems-engineer | WI-2.2 audit | APPROVED | LibJuno macro extensions intentional and documented |
| 9 | software-quality-engineer | WI-2.2 audit + WI-2.3 docs | APPROVED | Inline C11 § citations clear and consistent with file convention |
| 10 | senior-software-engineer | Gate C + WI-3.1 | APPROVED | 645/645 green; TC-WI-020b locks Reset → line 88 in real engine_cmd_msg.c |
| 11 | software-systems-engineer | WI-4.1 requirements | APPROVED (after rework) | Bidirectional implements link added to REQ-VSCODE-003 |
| 12 | software-verification-engineer | WI-4.1 requirements | APPROVED | JSON valid; REQ-VSCODE-033 ID unique; rationale present |
| 13 | software-verification-engineer | WI-4.2/4.3 tags | APPROVED | All modified source lines tagged; all new test functions tagged; REQ IDs resolve |
| 14 | software-quality-engineer | WI-4.4 packaging + SDP | APPROVED | VSIX 0.1.4 built; chevrotain bundled; no node_modules/** in .vscodeignore; no __mocks__ in VSIX; SDP Sprint 19 entry added |
| 15 | final-quality-engineer | Phase 5 final gate | APPROVED | All criteria met; sprint scope traceability clean |

---

## 9. Files Created/Modified This Sprint

### Modified Files

| File | Changes |
|------|---------|
| `src/parser/parser.ts` | unaryExpression split per C11 §6.5.3; file-level + function-level @req tags; inline C11 § comment |
| `requirements/vscode/requirements.json` | REQ-VSCODE-005 tightened; REQ-VSCODE-033 added; REQ-VSCODE-003 reciprocal implements link |
| `src/parser/__tests__/c11-grammar-conformance.test.ts` | @verify tags extended to include REQ-VSCODE-033 |
| `package.json` | version bumped 0.1.2 → 0.1.4 |
| `software-development-plan.md` | Sprint 19 phase entry added |

### New Files

| File | Contents |
|------|---------|
| `src/parser/__tests__/go-to-def-bug.test.ts` | TC-WI-019 parser unit test |
| `src/__tests__/go-to-def-bug-e2e.test.ts` | TC-WI-020 E2E test (fixture + real file) |
| `src/parser/__tests__/c11-grammar-conformance.test.ts` | 21-case C11 conformance suite |
| `test/fixtures/go-to-def-bug/fixture_api.h` | repro fixture header |
| `test/fixtures/go-to-def-bug/fixture_impl.c` | repro fixture implementation |
| `libjuno-0.1.4.vsix` | packaged extension |

---

## 10. Lessons Learned

| Lesson | Details |
|--------|---------|
| Every plan presented to PM must include worker/verifier assignment table | PM explicitly requested this after initial sprint plan omitted assignments. All columns: WI, Deliverable, Worker, Verifier(s), Notes. No exceptions. |
| C11 §6.5.3: unary-operator takes cast-expression, not unary-expression | `& * + - ~ !` must recurse to castExpression; only `++`/`--` recurse to unaryExpression. Parser grammar bugs of this class silently drop entire files from the function-definition index. |
| Reciprocal `implements` update required when adding new REQ with `uses` | verify_traceability.py enforces bidirectional consistency. When adding REQ-B with `uses: ["REQ-A"]`, also add REQ-B to REQ-A's `implements` array. |
| Grammar-level REQ must be tagged on conformance tests (not just user-visible REQ) | Tests in c11-grammar-conformance.test.ts directly verify REQ-VSCODE-033 (grammar conformance) and should carry that tag alongside REQ-VSCODE-005 (user-visible outcome). |

---

## 11. Commit

```
:bug: Fix Go-to-Definition navigating to vtable initializer instead of function definition

- Parser: fix unaryExpression per C11 §6.5.3 — & * + - ~ ! recurse to
  castExpression, not unaryExpression (affected all cast-deref patterns)
- Parser: C11 §6.5/§6.8 grammar audit — no additional deviations found
- Tests: TC-WI-019 parser unit test, TC-WI-020 E2E (fixture + real file)
- Tests: 21-case c11-grammar-conformance.test.ts permanant regression guard
- Requirements: REQ-VSCODE-005 tightened; REQ-VSCODE-033 added
- Traceability: @req/@verify tags on modified source and new tests
- VSIX: 0.1.4

Fixes: EngineCmdMsg_Reset ctrl-click navigation lands on line 88 (function
definition), not line 53 (positional vtable initializer).

Test count: 622 → 645 (26 suites)
```
