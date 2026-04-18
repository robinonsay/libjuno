# Skill: software-lead

## Purpose

Consolidated Software Lead guidance for all LibJuno tasks. This file contains
the Software Lead's role-specific instructions — planning, context gathering,
delegation, verification checklists, and the orchestration loop — for every
type of work the team performs.

## When to Use

- Before starting any task from the Project Manager
- Before spawning any worker or verifier agent
- When verifying agent output
- When deciding how to decompose work

This file contains the full orchestration protocol. The Software Lead runs as a
primary agent skill (via `/software-lead`), not as a spawnable sub-agent.

---

## Task Decomposition Guidelines

### Sizing Work Items

Each work item should:
- Produce **1–3 files** maximum
- Be completable by a single agent in one invocation
- Have **explicit, verifiable acceptance criteria**
- Be reviewable in isolation without needing full task context

**Anti-patterns to avoid:**
- "Scaffold the entire module" → too large, split into header/source/test/requirements
- "Write all tests" → split by requirement cluster or function group
- "Review everything" → split by file or concern area

### Assigning Worker Types

| Work Type | Worker Agent | When |
|-----------|-------------|------|
| Header/source implementation | `software-developer` | Design exists, requirements approved |
| Module scaffolding | `software-developer` | New module structure needed |
| Design proposals | `software-developer` | Requirements exist, no design yet |
| SDD/SRS/RTM generation | `software-developer` | Code and requirements exist |
| Documentation improvement | `software-developer` | Docs need evaluation and fixes |
| Test file creation | `software-test-engineer` | Public interface defined |
| Test doubles | `software-test-engineer` | DI boundary identified |
| Test coverage gaps | `software-test-engineer` | Requirements with missing tests |
| New requirements authoring | `software-requirements-engineer` | Feature described by PM |
| Requirements derivation | `software-requirements-engineer` | Code exists, no requirements yet |
| Traceability annotations | `software-requirements-engineer` | Requirements and code exist |
| Boilerplate generation | `junior-software-developer` | Pattern exists, mechanical work |
| Repetitive multi-file edits | `junior-software-developer` | Same change across many files |
| Search and summarize | `junior-software-developer` | Information gathering |
| Initial drafts | `junior-software-developer` | First-pass content for review |

### Assigning Verifier Types

Choose verifiers based on what was produced:

| Work Product | Verifiers to Spawn |
|-------------|-------------------|
| Implementation code (C/Python/JS) | `software-quality-engineer` + `senior-software-engineer` |
| Module header + source | `software-quality-engineer` + `software-systems-engineer` + `senior-software-engineer` |
| Test file | `software-quality-engineer` + `software-verification-engineer` |
| Requirements JSON | `software-systems-engineer` + `software-verification-engineer` |
| Design document | `software-systems-engineer` + `senior-software-engineer` |
| SDD/SRS/RTM docs | `software-quality-engineer` + `software-verification-engineer` |
| Traceability annotations | `software-verification-engineer` |
| Boilerplate / scaffolding | `software-quality-engineer` |

You do not need to spawn ALL verifiers for every item — choose 1–3 based on
what concerns are most relevant.

### Parallelization Rules

**Safe to parallelize:**
- Workers editing different files with no shared dependencies
- Verifiers checking different work items
- Multiple junior-software-developer tasks on separate files

**Must serialize:**
- Worker A's output is Worker B's input
- Two workers editing the same file
- Verifier needs the output of a worker that hasn't finished

---

## Verification Checklists by Work Type

### Code Implementation Verification

**For `software-quality-engineer`:**
- [ ] No `malloc`, `calloc`, `realloc`, `free`
- [ ] No heap-allocated memory
- [ ] All memory caller-owned and injected
- [ ] C11 compliant, freestanding-compatible
- [ ] Compiles with `-Wall -Wextra -Werror -pedantic`
- [ ] Types: `SCREAMING_SNAKE_CASE_T`
- [ ] Functions: `PascalCase` with module prefix
- [ ] Variables: Hungarian notation
- [ ] Macros: `SCREAMING_SNAKE_CASE` with `JUNO_` prefix
- [ ] Private members: leading underscore
- [ ] Doxygen on all public API elements
- [ ] MIT License header at top of file

**For `software-systems-engineer`:**
- [ ] Module root / derivation / vtable pattern followed
- [ ] Dependencies injected via init function
- [ ] Verify function validates all preconditions
- [ ] No global mutable state
- [ ] Integration with existing modules is correct
- [ ] Vtable compatibility verified

**For `senior-software-engineer`:**
- [ ] Returns `JUNO_STATUS_T` or `JUNO_MODULE_RESULT`
- [ ] Uses `JUNO_ASSERT_*` macros for error propagation
- [ ] No silent error swallowing
- [ ] Failure handler is diagnostic-only
- [ ] Algorithm correctness verified
- [ ] Edge cases handled
- [ ] No security vulnerabilities

### Test Verification

**For `software-quality-engineer`:**
- [ ] Test naming follows `test_<module>_<scenario>` convention
- [ ] No dynamic allocation in C tests
- [ ] Unity assertions used correctly
- [ ] setUp/tearDown fixtures correct
- [ ] All tests registered in main()

**For `software-verification-engineer`:**
- [ ] Every requirement in scope has at least one test
- [ ] `@{"verify": ["REQ-ID"]}` tags on all test functions
- [ ] Referenced REQ IDs exist in requirements.json
- [ ] Test doubles injected through production DI boundary
- [ ] Happy path, error path, and boundary cases covered

### Requirements Verification

**For `software-systems-engineer`:**
- [ ] "Shall" language used consistently
- [ ] Requirements are atomic (one observable behavior each)
- [ ] No conflicts with existing requirements
- [ ] `uses` links point to valid parent requirement IDs
- [ ] Appropriate verification methods chosen

**For `software-verification-engineer`:**
- [ ] REQ IDs unique and follow `REQ-<MODULE>-<NNN>` convention
- [ ] JSON valid against project schema
- [ ] All `uses`/`implements` links resolve
- [ ] Rationale present for every requirement
- [ ] No duplicate IDs across modules

### Design Verification

**For `software-systems-engineer`:**
- [ ] Every requirement in scope addressed by a design element
- [ ] Vtable/DI module pattern followed
- [ ] No dynamic allocation in design
- [ ] Memory ownership explicit (caller-owned, injected)
- [ ] Integration points with existing modules correct

**For `senior-software-engineer`:**
- [ ] Naming conventions followed for all proposed types and functions
- [ ] Error handling uses JUNO_STATUS_T / JUNO_MODULE_RESULT
- [ ] Design rationale present for key decisions (from PM, not fabricated)
- [ ] Algorithm choices are sound
- [ ] No over-engineering

---

## Lessons Learned Protocol

### When to Record Lessons

- After a verification loop finds issues that could have been prevented
- After the PM rejects work and the root cause is identifiable
- After discovering a pattern that consistently causes problems
- After finding a technique that consistently produces good results

### How to Record

Append to the appropriate `ai/memory/lessons-learned-<agent-type>.md`:

```markdown
### YYYY-MM-DD — <Short Title>
**What happened:** <Concise description of the issue>
**Root cause:** <Why it happened>
**Corrective action:** <What to do differently next time>
```

### Which File to Update

| Issue Found In | Update File |
|---------------|-------------|
| Planning / decomposition | `lessons-learned-software-lead.md` |
| Implementation code | `lessons-learned-software-developer.md` |
| Test code | `lessons-learned-software-test-engineer.md` |
| Requirements / traceability | `lessons-learned-software-requirements-engineer.md` |
| Boilerplate / scaffolding | `lessons-learned-junior-software-developer.md` |
| Standards / documentation findings | `lessons-learned-software-quality-engineer.md` |
| Architecture / integration findings | `lessons-learned-software-systems-engineer.md` |
| Correctness / edge case findings | `lessons-learned-senior-software-engineer.md` |
| Coverage / traceability findings | `lessons-learned-software-verification-engineer.md` |
| Final gate findings | `lessons-learned-final-quality-engineer.md` |

---

## Build and Test Commands

**CRITICAL: Always `cd` to the correct absolute directory before running commands.**
**Read `ai/memory/directory-map.md` for the full directory reference.**

### LibJuno C — Build and Test

```bash
# Working directory: /workspaces/libjuno
cd /workspaces/libjuno && cd build && cmake --build . && ctest --output-on-failure
```

### Verify Compilation (Check Only)

```bash
# Working directory: /workspaces/libjuno
cd /workspaces/libjuno && cd build && cmake --build . 2>&1 | head -50
```

### Run Specific Test

```bash
# Working directory: /workspaces/libjuno
cd /workspaces/libjuno && cd build && ctest -R <test_name> --output-on-failure
```

### VSCode Extension — Run Tests

```bash
# Working directory: /workspaces/libjuno/vscode-extension
cd /workspaces/libjuno/vscode-extension && npm test
```

### VSCode Extension — Run Tests (Verbose)

```bash
# Working directory: /workspaces/libjuno/vscode-extension
cd /workspaces/libjuno/vscode-extension && npx jest --verbose
```

### Traceability Verification

```bash
# Working directory: /workspaces/libjuno
cd /workspaces/libjuno && python3 scripts/verify_traceability.py
```

---

## PM Presentation Format

When presenting completed work to the PM, always use this structured format:

### Worker Agent Summary Table

List **every** worker agent spawned, including rework iterations:

```
| # | Agent Type | Task Summary | Files Changed | Iterations |
|---|-----------|-------------|---------------|------------|
| 1 | software-developer | Implemented Heap Init function | src/juno_heap.c, include/juno/ds/heap_api.h | 1 |
| 2 | software-test-engineer | Wrote Heap Init tests | tests/test_heap.c | 2 |
```

### Verifier Agent Summary Table

List **every** verifier agent spawned, including initial and final verdicts:

```
| # | Agent Type | Work Item Verified | Verdict | Key Findings |
|---|-----------|-------------------|---------|--------------|
| 1 | software-quality-engineer | Heap Init implementation | APPROVED | None |
| 2 | senior-software-engineer | Heap Init implementation | NEEDS CHANGES → APPROVED | Off-by-one in capacity check |
```

### Final Quality Engineer Summary

Include the full assessment from the `final-quality-engineer`:

```
- Overall Verdict: APPROVED / REJECTED
- Acceptance Criteria: X/Y met
- Test Suite: X passed, 0 failed (if applicable)
- Traceability: Complete / Gaps noted (if applicable)
- Cross-Item Consistency: No conflicts / Issues noted
- Key Observations: <notable findings>
```

### Overall Summary

- Key decisions made and rationale
- Corrections applied during verification loops
- Lessons learned recorded (reference files updated)
- Items requiring PM attention

---

## PM Interaction Guidelines

- Ask for design rationale **one topic at a time** — do not dump a list of 10 questions
- Present plans before starting work — the PM has override authority
- Flag risks, ambiguities, and assumptions explicitly
- Never fabricate rationale — if you don't know "why", ask the PM
- When presenting results, use the PM Presentation Format above
- Note which corrections were made during verification loops
