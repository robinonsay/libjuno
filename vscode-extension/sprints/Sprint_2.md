# Sprint 2 — Visitor: Vtable Init Patterns

**Sprint:** 2  
**Phase:** 2 (Visitor — Vtable Init Patterns)  
**Date:** 2026-04-15  
**Status:** Planning  
**SDP Version:** 2.1

---

## 1. Sprint Startup Protocol Summary

Documents read and summarized before plan creation:

### 1.1 Software Lead Skill File (`ai/skills/software-lead.md`)
- Work items must produce 1–3 files max, be completable by a single agent, have explicit acceptance criteria
- Parallelization safe only when workers edit different files with no shared dependencies
- Must serialize when Worker A's output is Worker B's input or when two workers edit the same file
- Test verification requires both `software-quality-engineer` and `software-verification-engineer`
- Code verification requires `software-quality-engineer` + `senior-software-engineer`

### 1.2 Lessons Learned (`ai/memory/lessons-learned-software-lead.md`)
- **Do not do hands-on work** — delegate everything to workers/verifiers
- **Break work into smallest testable increments** with verification gates between each
- **Verify worker output with test execution**, not just code review
- **Chevrotain CST keys** — always verify empirically before writing visitor code
- **Serialize dependent work** — fix source → verify → then write tests
- **Diagnostic agent first** when failures remain after planned fixes
- **Sprint Startup Protocol is MANDATORY** — re-read all docs before planning

### 1.3 Requirements (Sprint 2-Relevant)
- **REQ-VSCODE-010**: Designated initializer recognition → TC-P6-003
- **REQ-VSCODE-011**: Direct assignment recognition → TC-P7-001, TC-P7-002
- **REQ-VSCODE-012**: Positional initializer recognition → TC-P8-001 through TC-P8-004
- **REQ-VSCODE-016**: Failure handler navigation → TC-P10-001 through TC-P10-003
- **REQ-VSCODE-003**: LibJuno API pattern recognition → TC-P11-001 through TC-P11-007

### 1.4 Software Design (`vscode-extension/design/design.md`)
- Chevrotain parser + CST visitor produces `ParsedFile` with `vtableAssignments`, `failureHandlerAssigns`, and `localTypeInfo`
- Three vtable init patterns: designated (Section 6), direct assignment (Section 7), positional (Section 8)
- Positional initializer requires field-order zip with `apiStructFields` from `visitStructDefinition`
- `VtableAssignmentRecord`: `{ apiType, field, functionName, file, line }`

### 1.5 Test Cases (`vscode-extension/design/test-cases.md`)
- **TC-P6-003**: Negative — `const` variable of non-API type must not be captured
- **TC-P7-001**: Direct assignment, heap API field; **TC-P7-002**: Negative — designated init not double-counted
- **TC-P8-001**: Positional init zips `["Insert","Heapify","Pop"]`; **TC-P8-002**: Broker API 2 fields; **TC-P8-003**: Log API 4 fields; **TC-P8-004**: Negative — designated takes precedence
- **TC-P10-001/002/003**: Failure handler assignment, macro form, negative
- **TC-P11-001–007**: Function definitions — static, non-static, inline, void, non-standard return, forward decl negative, header prototype negative

### 1.6 SDP Phase 2 Plan (`software-development-plan.md` §4 Phase 2)
- **Debug budget**: 30%
- **Discovery checkpoint**: After TC-P8-001, dump `vtableAssignments` and `apiStructFields` output
- **v2.1 Amendments applied**: TC-P6-NEG-001, TC-P7-NEG-001, TC-P8-BND-001 (negative/boundary tests)
- **Sprint Exit Criteria (new in v2.1)**: Mutation testing ≥75% on all tested production files (HARD gate); Coverage checkpoint ≥85% line coverage per tested module

---

## 2. Pre-Sprint Gap Analysis

### 2.1 Sprint 1 Baseline
- **98 tests passing** across 4 test files (confirmed by `npm test` run)
- 0 failures, 4 suites

### 2.2 Existing Test Coverage vs. SDP Phase 2 Scope

The following Sprint 2 test cases **already exist and pass** from Sprint 1 work:

| SDP Work Item | TC IDs | Status | File |
|---------------|--------|--------|------|
| WI-2.1 | TC-P6-003 | ✅ Implemented | `visitor-vtable.test.ts` |
| WI-2.2 | TC-P7-001, TC-P7-002, TC-P7-003 | ✅ Implemented | `visitor-vtable.test.ts` |
| WI-2.3 | TC-P8-001, TC-P8-002, TC-P8-002b, TC-P8-003, TC-P8-004 | ✅ Implemented | `visitor-vtable.test.ts` |
| WI-2.4 | TC-P10-001 through TC-P10-005 | ✅ Implemented | `visitor-functions.test.ts` |
| WI-2.5 | TC-P11-001 through TC-P11-007 | ✅ Implemented | `visitor-functions.test.ts` |
| WI-2.6 | TC-P6-NEG-001 | ✅ Implemented | `visitor-vtable.test.ts` |
| WI-2.7 | TC-P7-NEG-001 | ✅ Implemented | `visitor-vtable.test.ts` |
| WI-2.8 | TC-P8-BND-001 | ❌ **NOT implemented** | — |

**Additional tests found** (not in SDP but valuable):
- `TC-P6-META`: vtableAssignment record carries correct file path and line number
- `TC-P7-003`: Dereference assignment doesn't produce vtable records
- `TC-P8-002b`: Broker API positional init (2 fields)
- `TC-P8-NEG-001`: Designated init doesn't invoke positional path

### 2.3 Sprint 2 Remaining Work

| # | Work Item | Description | Type |
|---|-----------|-------------|------|
| 1 | TC-P8-BND-001 | Write boundary test: single-field positional initializer | Test gap fill |
| 2 | Stryker setup | Install Stryker, create config, configure for TypeScript/Jest | Infrastructure |
| 3 | Mutation testing | Run Stryker on `parser/lexer.ts`, `parser/parser.ts`, `parser/visitor.ts` — must reach ≥75% per module | Hard gate |
| 4 | Coverage checkpoint | Run `jest --coverage`, verify ≥85% line coverage per tested module | Quality gate |
| 5 | Mutation gap fill | If any module < 75% mutation score, write additional tests to kill surviving mutants | Conditional |

---

## 3. Dependency Graph

```
                    ┌─────────────────────┐
                    │  Pre-Sprint Gate     │
                    │  98/98 tests pass    │
                    └──────────┬──────────┘
                               │
              ┌────────────────┼────────────────┐
              │                │                 │
              ▼                ▼                 ▼
     ┌────────────────┐ ┌──────────────┐ ┌──────────────────┐
     │ WI-S2.1        │ │ WI-S2.2      │ │ WI-S2.3          │
     │ TC-P8-BND-001  │ │ Stryker      │ │ Coverage          │
     │ (test engineer)│ │ Setup        │ │ Checkpoint        │
     │                │ │ (developer)  │ │ (junior dev)      │
     └───────┬────────┘ └──────┬───────┘ └────────┬─────────┘
             │                 │                   │
              ┌────────────────┼───────────────────┘
              │   VERIFY BATCH 1 (3 verifiers)
              ▼
     ┌─────────────────────────────────────────────┐
     │ V-S2.1: quality-engineer (TC-P8-BND-001)    │
     │ V-S2.2: systems-engineer (Stryker config)   │
     │ V-S2.3: verification-engineer (coverage)    │
     └──────────────────────┬──────────────────────┘
                            │
                            ▼
                  ┌──────────────────┐
                  │ WI-S2.4          │
                  │ Run Mutation     │
                  │ Testing          │
                  │ (test engineer)  │
                  └────────┬─────────┘
                           │
                           ▼
                  ┌──────────────────┐
                  │ V-S2.4           │
                  │ senior-engineer  │
                  │ (mutation report)│
                  └────────┬─────────┘
                           │
                  ┌────────┴─────────┐
                  │                  │
            ≥75% all modules    < 75% any module
                  │                  │
                  ▼                  ▼
          ┌──────────────┐  ┌──────────────────┐
          │ WI-S2.5      │  │ WI-S2.5          │
          │ Final QE     │  │ Kill surviving   │
          │ Gate          │  │ mutants (loop)   │
          └──────────────┘  └──────────────────┘
```

---

## 4. Execution Plan

### Step 0: Pre-Sprint Gate ✅

**Action:** Confirm Sprint 1 baseline.  
**Result:** 98/98 tests passing. 0 failures. 4 suites. Confirmed.

---

### Step 1: Parallel Worker Batch (3 workers)

Spawn 3 workers simultaneously. These touch different files/concerns with no dependencies.

#### Worker WI-S2.1 — Test Engineer
**Task:** Write TC-P8-BND-001 boundary test  
**Agent:** `software-test-engineer`  
**File to modify:** `src/parser/__tests__/visitor-vtable.test.ts`  
**Brief:**
```
Task: Write TC-P8-BND-001 — boundary test for single-field positional initializer
File to modify: vscode-extension/src/parser/__tests__/visitor-vtable.test.ts
  Add test inside the existing "Positional Initializer (Section 8)" describe block.

Test specification:
  - Define a single-field API struct: `struct FOO_API_TAG { JUNO_STATUS_T (*DoThing)(...); };`
  - Define a positional initializer: `static const FOO_API_T tFooApi = { FooDoThing };`
  - Both struct and init in same source string (same pattern as TC-P8-002)
  - Assert vtableAssignments has exactly 1 record:
    { apiType: "FOO_API_T", field: "DoThing", functionName: "FooDoThing" }
  - This validates the field-zip works correctly with a single field (boundary)

Acceptance criteria:
  1. Test named "TC-P8-BND-001: single-field positional initializer produces one vtable record"
  2. Test passes with `npm test`
  3. All 98 prior tests still pass (0 regressions)
  4. Test uses parseFile() from "../visitor" (same import as existing tests)
  5. Test placed inside existing describe("Positional Initializer (Section 8)") block

Context files to read:
  - vscode-extension/src/parser/__tests__/visitor-vtable.test.ts (existing patterns)
  - vscode-extension/src/parser/types.ts (VtableAssignmentRecord interface)
  - vscode-extension/design/test-cases.md (TC-P8 section for format reference)

Constraints:
  - Do NOT modify any production source files
  - Use synthetic C source strings (no file system access)
  - Follow existing test naming and assertion patterns exactly

Lessons-learned file: ai/memory/lessons-learned-software-test-engineer.md
```

#### Worker WI-S2.2 — Software Developer
**Task:** Set up Stryker mutation testing infrastructure  
**Agent:** `software-developer`  
**Files to modify:** `package.json`, new file `stryker.config.json`  
**Brief:**
```
Task: Set up Stryker mutation testing for the VSCode extension project
Files to create/modify:
  - vscode-extension/stryker.config.json (NEW)
  - vscode-extension/package.json (add devDependency + npm script)

Specification:
  1. Add @stryker-mutator/core, @stryker-mutator/jest-runner, 
     @stryker-mutator/typescript-checker as devDependencies
  2. Create stryker.config.json targeting:
     - mutate: ["src/parser/lexer.ts", "src/parser/parser.ts", "src/parser/visitor.ts"]
       (all production source files that currently have tests)
     - testRunner: "jest"
     - checkers: ["typescript"]
     - reporters: ["clear-text", "html"]
     - thresholds: { high: 80, low: 75, break: 75 }
       (break: 75 means Stryker fails if score < 75%)
     - timeoutMS: 60000
     - concurrency: 2 (conservative for stability)
  3. Add npm script: "test:mutation": "stryker run"
  4. Do NOT run Stryker yet — just set up the config

Acceptance criteria:
  1. stryker.config.json is valid JSON and references correct file paths
  2. package.json has all 3 Stryker devDependencies
  3. package.json has "test:mutation" script
  4. `npm install` succeeds after changes
  5. No existing test or source files modified

Context files to read:
  - vscode-extension/package.json (existing dependencies)
  - vscode-extension/jest.config.js (Jest configuration)
  - vscode-extension/tsconfig.json (TypeScript configuration)

PM Rationale: Mutation testing is a HARD per-sprint gate starting Sprint 2.
  The PM directive is: "mutation testing should be a hard gate that occurs
  after a sprint to make sure the tests are valid." Score must be ≥75%
  per production module.

Lessons-learned file: ai/memory/lessons-learned-software-developer.md
```

#### Worker WI-S2.3 — Junior Software Developer
**Task:** Run coverage checkpoint and produce report  
**Agent:** `junior-software-developer`  
**Files to create:** None (report only)  
**Brief:**
```
Task: Run Jest coverage checkpoint and report per-module line coverage
Files to create/modify: NONE — report only (return results in your response)

Steps:
  1. cd vscode-extension
  2. Run: npx jest --coverage --coverageReporters=text 2>&1
  3. Report the per-file line coverage for ALL production source files
  4. Flag any production source file with tests that has < 85% line coverage
  5. Report which files currently have NO tests (expected — not flagged)

Expected output format:
  File                              | Lines % | Status
  parser/lexer.ts                   | XX%     | PASS/FAIL/NO TESTS
  parser/parser.ts                  | XX%     | PASS/FAIL/NO TESTS
  parser/visitor.ts                 | XX%     | PASS/FAIL/NO TESTS
  ...

Acceptance criteria:
  1. Coverage report includes all production source files
  2. Each tested file's line coverage percentage is reported
  3. Files below 85% are flagged as FAIL
  4. Files with no tests are marked NO TESTS (not flagged)

Constraints:
  - Do NOT modify any files
  - Do NOT install any packages
  - Just run the coverage command and report results

Lessons-learned file: ai/memory/lessons-learned-junior-software-developer.md
```

---

### Step 2: Verify Batch 1 (3 verifiers — one per worker)

Spawn 3 verifiers simultaneously, one for each worker's output.

#### Verifier V-S2.1 — Software Quality Engineer
**Checks:** WI-S2.1 output (TC-P8-BND-001 test)  
**Agent:** `software-quality-engineer`  
**Criteria:**
- Test follows existing naming convention (`TC-P8-BND-001: ...`)
- Test uses `parseFile()` import from `"../visitor"`
- Assertions use `toMatchObject()` with exact `VtableAssignmentRecord` property names from `types.ts`
- Test placed in correct describe block
- No `any` type assertions
- `npm test` passes with 0 failures (run it)

#### Verifier V-S2.2 — Software Systems Engineer
**Checks:** WI-S2.2 output (Stryker setup)  
**Agent:** `software-systems-engineer`  
**Criteria:**
- `stryker.config.json` is valid JSON
- All 3 Stryker packages in `devDependencies` with compatible versions
- `mutate` array targets exactly the 3 parser production files
- `break: 75` threshold configured (hard gate)
- `"test:mutation"` script exists in `package.json`
- `npm install` succeeds without errors
- No production source files or test files were modified

#### Verifier V-S2.3 — Software Verification Engineer
**Checks:** WI-S2.3 output (coverage report)  
**Agent:** `software-verification-engineer`  
**Criteria:**
- Coverage report covers all 14 production source files
- Tested modules (lexer.ts, parser.ts, visitor.ts) flagged if < 85% line coverage
- Untested modules correctly marked as NO TESTS
- Report format is clear and actionable

---

### Step 3: Worker Batch 2 (1 worker — depends on Steps 1+2)

This step depends on Stryker being set up (WI-S2.2 verified) and all tests passing (WI-S2.1 verified).

#### Worker WI-S2.4 — Software Test Engineer
**Task:** Run Stryker mutation testing and report results  
**Agent:** `software-test-engineer`  
**Brief:**
```
Task: Run Stryker mutation testing and report per-module mutation scores
Files to modify: NONE (unless mutant-killing tests needed)

Steps:
  1. cd vscode-extension
  2. Run: npm install (to install Stryker dependencies from WI-S2.2)
  3. Run: npm run test:mutation 2>&1
  4. Report per-module mutation score:
     - parser/lexer.ts:   XX% (PASS if ≥75%, FAIL if <75%)
     - parser/parser.ts:  XX% (PASS if ≥75%, FAIL if <75%)
     - parser/visitor.ts: XX% (PASS if ≥75%, FAIL if <75%)
  5. If ANY module is FAIL:
     a. List the top 5 surviving mutants per failing module
     b. For each survivor, describe the mutation and suggest a test to kill it
     c. Write the tests directly into the appropriate test file
     d. Re-run mutation testing to confirm improvement
  6. If ALL modules PASS: report final scores

Acceptance criteria:
  1. Mutation testing runs successfully to completion
  2. Per-module mutation scores reported
  3. If any module < 75%: additional tests written and mutation score re-checked
  4. All prior tests still pass (0 regressions)
  5. Final mutation score ≥75% per module

Context files to read:
  - vscode-extension/stryker.config.json (Stryker configuration)
  - vscode-extension/src/parser/__tests__/*.test.ts (existing tests)
  - vscode-extension/src/parser/lexer.ts (production code)
  - vscode-extension/src/parser/parser.ts (production code)
  - vscode-extension/src/parser/visitor.ts (production code)

Constraints:
  - Only modify test files — do NOT modify production source
  - Each new test must follow existing naming conventions
  - Run npm test after any test file changes to verify 0 regressions

Lessons-learned file: ai/memory/lessons-learned-software-test-engineer.md
```

---

### Step 4: Verify Batch 2 (1 verifier)

#### Verifier V-S2.4 — Senior Software Engineer
**Checks:** WI-S2.4 output (mutation testing results)  
**Agent:** `senior-software-engineer`  
**Criteria:**
- Mutation testing ran to completion without infrastructure errors
- Per-module scores reported accurately
- If additional tests were written:
  - Tests actually kill the targeted mutants (not just testing unrelated behavior)
  - Tests follow existing patterns and naming
  - No production source modifications
  - `npm test` passes with 0 failures
- Final mutation score ≥75% per module (HARD gate)

---

### Step 5: Conditional — Mutation Gap Fix Loop

**Trigger:** V-S2.4 reports any module still < 75% after WI-S2.4's fixes.

If triggered:
1. Spawn `software-test-engineer` to write targeted mutant-killing tests
2. Spawn `software-quality-engineer` to verify new tests
3. Re-run mutation testing
4. Repeat until all modules ≥75%

**This loop blocks sprint exit.** The mutation gate is non-negotiable per PM directive.

---

### Step 6: Final Quality Gate

#### Worker WI-S2.5 — Final Quality Engineer
**Agent:** `final-quality-engineer`  
**Checks:**
- All SDP Phase 2 acceptance criteria met (see Section 5 below)
- Full test suite passes (`npm test` — 99+ tests, 0 failures)
- TC-P8-BND-001 exists and passes
- Mutation score ≥75% per parser module (lexer, parser, visitor)
- Coverage checkpoint complete (≥85% on tested modules or documented plan)
- No regressions from Sprint 1 (98 original tests still pass)
- No production source files modified (test-only sprint)
- Stryker infrastructure properly configured and functional

---

## 5. Acceptance Criteria (from SDP Phase 2)

- [ ] TC-P6-003 implemented and passing *(pre-existing)*
- [ ] TC-P7-001, TC-P7-002 implemented and passing *(pre-existing)*
- [ ] TC-P8-001 through TC-P8-004 implemented and passing *(pre-existing)*
- [ ] TC-P6-NEG-001, TC-P7-NEG-001, TC-P8-BND-001 implemented and passing
- [ ] TC-P10 and TC-P11 gap check complete — all cases have coverage *(pre-existing)*
- [ ] No regressions in Phase 1 tests (98 tests still passing)
- [ ] Any source bugs documented in lessons-learned
- [ ] **Mutation score ≥75%** on `lexer.ts`, `parser.ts`, `visitor.ts` (HARD gate)
- [ ] **Line coverage ≥85%** on tested modules (or documented remediation plan)

---

## 6. Sprint Exit Checklist

- [ ] All SDP Phase 2 test cases implemented
- [ ] All tests pass (including Sprint 1 tests)
- [ ] `npm test` — 0 failures
- [ ] Mutation testing HARD gate passed (≥75% per module)
- [ ] Coverage checkpoint complete (≥85% on tested modules)
- [ ] No production source code modified (no double test run required)
- [ ] Final quality engineer approved
- [ ] PM approved

---

## 7. Parallelization Summary

| Step | Workers/Verifiers | Agents | Parallel? | Blocked By |
|------|-------------------|--------|-----------|------------|
| 0 | Pre-sprint gate | Lead | — | — |
| 1 | 3 workers | test-engineer, developer, junior-dev | ✅ All parallel | Step 0 |
| 2 | 3 verifiers | quality-engineer, systems-engineer, verification-engineer | ✅ All parallel | Step 1 |
| 3 | 1 worker | test-engineer | Serial | Step 2 |
| 4 | 1 verifier | senior-engineer | Serial | Step 3 |
| 5 | Loop (conditional) | test-engineer + quality-engineer | Serial per iteration | Step 4 (only if <75%) |
| 6 | 1 verifier | final-quality-engineer | Serial | Step 4 or 5 |

**Total agents: 3 workers + 3 verifiers + 1 worker + 1 verifier + 1 final QE = 9 agents minimum**  
**Parallelization ratio:** Step 1 runs 3 workers simultaneously; Step 2 runs 3 verifiers simultaneously.

---

## 8. Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Stryker installation fails on macOS | Low | Medium | Fall back to alternative mutation framework; document in lessons-learned |
| Mutation score < 75% on `parser.ts` (1050 lines, complex grammar) | High | High | Budget Step 5 loop; prioritize killing mutants in hot paths (declarationSpecifiers, GATE logic) |
| Mutation score < 75% on `visitor.ts` (1076 lines) | Medium | High | Focus on untested visitor methods; write targeted chain-walk tests |
| Stryker timeout on large files | Medium | Medium | Increase `timeoutMS`; reduce `concurrency` if needed |
| Coverage < 85% on visitor.ts | Medium | Low | Document remediation plan for Sprint 3 (localTypeInfo tests will raise coverage) |

---

## 9. Lessons Applied

| Lesson | How Applied |
|--------|------------|
| "Break work into smallest testable increments" | Only 1 new test to write (TC-P8-BND-001); rest is infrastructure and gates |
| "Verify worker output with test execution" | Every verifier step includes `npm test` as a mandatory check |
| "Do not do hands-on work; delegate everything" | All execution delegated to worker/verifier agents |
| "Sprint Startup Protocol" | Section 1 documents all files read and key points summarized |
