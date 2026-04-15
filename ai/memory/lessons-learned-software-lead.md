# Lessons Learned — Software Lead

## How to Use This File

- **Read** this file before starting any task
- **Append** a new entry when a mistake or insight is discovered during work or verification
- **Keep entries concise**: what went wrong, root cause, and corrective action
- **Do not delete** old entries — they form institutional knowledge
- **Date** each entry for chronological tracking

## Lessons

### 2026-04-14 — Do not do hands-on work; delegate everything
**What happened:** The Software Lead started diagnosing parser bugs directly (running ts-node commands, reading CST output, tracing grammar logic) instead of delegating to worker agents.
**Root cause:** Instinct to "just fix it quickly" rather than trusting the team structure. Blurred the line between orchestrator and worker.
**Corrective action:** The Software Lead must NEVER write code, run diagnostic commands, or perform technical analysis. All hands-on work — including bug diagnosis — must be delegated to worker or verifier agents. The Lead's job is to plan, delegate, verify, and present.

### 2026-04-14 — Break work into smallest testable increments with verification gates
**What happened:** The first plan spawned 4 large test-writing agents in parallel. All 4 produced tests that failed because of underlying source bugs that hadn't been caught first. This wasted all 4 agents' work.
**Root cause:** Monolithic work items with no intermediate verification. Tests were written against broken source code with no check that the parser/visitor actually worked before writing tests for it.
**Corrective action:**
1. Always run existing tests and verify source code compiles/works BEFORE writing new tests.
2. Sequence dependent work: fix source → verify fix → then write tests against verified code.
3. Insert verification gates between every dependent step. The next worker agent should only start after the previous agent's output has been verified.
4. Prefer many small work items (1 fix, 1 test file) over few large ones (4 test files at once).

### 2026-04-14 — Verify worker output with test execution, not just code review
**What happened:** Worker agents produced test files that looked correct but failed at runtime because they made incorrect assumptions about the source code's CST structure and API surface.
**Root cause:** Tests were written based on design docs and type definitions without verifying actual runtime behavior. No agent ran the tests before declaring them complete.
**Corrective action:** After every worker produces code, the Lead must run the relevant tests (or have a verifier run them) before considering the work item complete. "Code compiles" and "tests pass" are both required gates.

### 2026-04-14 — Chevrotain CST key naming is not intuitive — always verify empirically
**What happened:** The visitor.ts code used `"Identifier2"` to access the second CONSUME of Identifier in the structOrUnionSpecifier rule, but Chevrotain actually stores it under `"Identifier"`. This caused all struct extraction to silently return empty arrays.
**Root cause:** Chevrotain's CST key naming convention (`CONSUME2(X)` → key `"X"` not `"X2"`) was assumed incorrectly.
**Corrective action:** When fixing Chevrotain visitor code, always instruct the developer to dump the actual CST keys first (via a small diagnostic test) before writing visitor logic. Include this instruction in every brief involving Chevrotain CST visitor code.

### 2026-04-14 — Plan template: serialized fix-verify-fix-verify with test gates
**What happened:** The PM approved a plan structure that serializes dependent work with verification gates between each step. This pattern should be reused.
**Plan template for bug-fix + test work:**
1. **Phase 1 — Source fixes (serialized):** For each source bug: (a) spawn worker to fix, (b) spawn verifier to review, (c) Lead runs test gate. Only proceed to next fix after gate passes.
2. **Phase 2 — Test alignment (serialized, one file at a time):** Only starts after ALL source fixes verified. For each test file: (a) spawn test engineer to fix expectations against verified source, (b) spawn quality engineer to verify test quality, (c) Lead runs test gate. Only proceed to next test file after gate passes.
3. **Phase 3 — Final gate:** Spawn `final-quality-engineer` for holistic check (full test suite, compilation, consistency).
**Key rules:** No parallel work on dependent items. Verification after every single work item. Test execution is a mandatory gate (not just code review). Source bugs fixed before any test work. Test files fixed one at a time, not batched.

### 2026-04-14 — Use test-engineer as diagnostic agent before committing to Phase 2
**What happened:** The plan assumed 7 remaining failures were test expectation issues (Phase 2 test alignment). Spawned a test engineer to diagnose before fixing. The engineer discovered all 7 failures shared ONE source bug (`drillToPostfix(lhsCond)` instead of `drillToPostfix(ae)`) — a Phase 1 issue, not Phase 2.
**Root cause:** The plan assumed all source bugs were found during initial diagnosis. A third bug was hiding because its symptoms only appeared after Bugs A and B were fixed.
**Corrective action:** Always spawn a diagnostic agent when failures remain after planned fixes. Don't assume all bugs are found. Diagnosis before fix-attempts saves wasted work. The diagnostic brief should ask: "(A) wrong test expectations, (B) source bug in visitor, or (C) source bug in parser?" and require concrete evidence (ts-node output) for each answer.
