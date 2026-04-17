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

### 2026-04-15 — MANDATORY Sprint Startup Protocol: re-read all documents before planning
**PM Directive:** At the beginning of every sprint, BEFORE presenting the sprint plan to the PM, the Software Lead MUST:
1. Re-read the software-lead agent mode instructions and skill file (`ai/skills/software-lead.md`)
2. Read the appropriate worker and verifier skill files for the sprint's work
3. Read the relevant requirements (`requirements/vscode-extension/requirements.json` or as appropriate)
4. Read the software design document (`vscode-extension/design/design.md`)
5. Read the test cases document (`vscode-extension/design/test-cases.md`)
6. Read the software development plan (`vscode-extension/software-development-plan.md`) — especially the current sprint's phase(s)
7. Summarize each document's key points relevant to the sprint
8. Only THEN create and communicate the sprint plan to the PM

**Why:** Context is lost between conversations. Without re-reading, the Lead operates on stale or incomplete mental models, leading to plans that miss design constraints, skip requirements, or contradict prior decisions. This protocol ensures every sprint starts from verified ground truth.
**This is non-negotiable — it must happen every sprint without exception.**

### 2026-04-16 — Mutant-killing briefs must be tiny: ONE describe block, ≤10 tests per agent
**What happened:** Delegated "kill all surviving mutants in parser.ts" and "kill all surviving mutants in visitor.ts" as single agent tasks. Both agents hit their context window limits — one returned no output, the other returned truncated output. ~80 minutes wasted.
**Root cause:** The briefs were massive: they included full mutant analysis (100+ surviving mutants each), the entire production file structure, and asked agents to write 50–80+ tests in one invocation. This exceeds what a single agent can hold in context.
**Corrective action:**
1. **Each mutant-killing agent brief must target ONE specific code region** (e.g., "lines 375-392 of parser.ts" or "walkMacroBodyForApiMembers in visitor.ts") and produce **at most 10 tests**.
2. **Brief size limit:** ≤50 lines of context-setting text. Include only the 20–30 lines of source code the agent needs, not the full file.
3. **Provide the exact test file and insertion point** — don't ask the agent to figure out file structure.
4. **Split by function or line range**, not by file. A 1000-line file needs 5–8 agents targeting different functions, not 1 agent targeting the whole file.
5. **Never include full mutant lists in briefs.** Summarize: "5 mutants survive on lines 375-383 (GATE depth tracking). Write tests that exercise nested parentheses in JUNO macro bodies."
6. **Run tests after each small batch**, verify, then proceed to next batch. Don't batch all mutant-killing into one step.

### 2026-04-16 — Fix failing tests BEFORE writing new ones
**What happened:** The existing `visitor-mutant-killing.test.ts` file (written by a prior agent) has 11 failing tests out of 80. Instead of fixing those first, I tried to write more tests. The failing tests indicate misunderstandings about the source code behavior — fixing them first would have revealed the correct behavior to inform new test writing.
**Root cause:** Skipped the "verify existing baseline" step from the Sprint 1 lesson ("Verify worker output with test execution, not just code review").
**Corrective action:** Before any new mutant-killing tests, fix the 11 failing tests first. A failing test suite masks regressions and wastes mutation testing cycles.

### 2026-04-16 — Do NOT skip plan creation when PM asks for a plan
**What happened:** PM said "Create and document a new sprint plan for me to approve." Instead of creating the plan, the Lead started doing hands-on diagnostic work — reading source code, running debug tests, analyzing Chevrotain parser internals — directly violating the 2026-04-14 lesson ("Lead must NEVER write code or run diagnostic commands") AND ignoring the PM's explicit instruction.
**Root cause:** The Lead had already gathered diagnostic context in prior turns and defaulted to "keep investigating" instead of stopping to plan. The habit of doing hands-on work overrode the PM's clear directive.
**Corrective action:** When the PM gives an explicit instruction, execute THAT instruction immediately. Do not continue prior work. Planning comes before execution — always. If the PM says "create a plan," the very next action must be creating that plan document, not more analysis.

### 2026-04-17 — Mutation-killing phases must be split into single-function work items, not batched by file region
**What happened:** Phase 3 (parser.ts mutants) and Phase 4 (visitor.ts mutants) were each planned as a single phase with multiple serial batches inside. A worker spawned for this work stalled — likely because even a "phase" containing 3-6 serial batches is too coarse-grained for the orchestration loop. The Lead was waiting on a single large phase to complete instead of getting incremental progress.
**Root cause:** Phases 3 and 4 were still too monolithic. Each contained 3 and 6 serial work items respectively, but were treated as single phases. This meant: (1) a stall in one batch blocked the entire phase, (2) the Lead had no intermediate checkpoints to detect problems early, (3) re-planning required restarting the entire phase.
**Corrective action:**
1. **One work item = one phase.** Each mutant-killing batch (e.g., "kill mutants in macroBodyTokens depth tracking") should be its own independent phase with its own gate.
2. **Never batch multiple serial work items into one phase.** If WI-3a depends on GATE-3a before WI-3b can start, then 3a and 3b are separate phases, not sub-items of one phase.
3. **Execute one agent at a time for dependent work.** Spawn agent → verify → gate → next agent. No queuing multiple agents in sequence within a single orchestration step.
4. **Phases 3 and 4 should be re-split into ~9 independent micro-phases** (one per target function/region), each completable in a single agent invocation.

### 2026-04-17 — Always use absolute paths in `cd` commands; read `directory-map.md` before running terminal commands
**What happened:** Multiple agents (including the Lead) ran commands from the wrong working directory. Common failures: running `npm test` from project root (fails — must be in `vscode-extension/`), running `cd build && cmake --build .` from `vscode-extension/` (fails — must be in project root), running `python3 scripts/verify_traceability.py` from `vscode-extension/` (fails).
**Root cause:** Skill files used relative paths like `cd build` without specifying the required starting directory. Agents lost track of their current working directory across multiple commands.
**Corrective action:**
1. **Read `ai/memory/directory-map.md`** before running any terminal command — it maps every command to its required absolute directory.
2. **Always prefix commands with an absolute `cd`**: e.g., `cd /workspaces/libjuno && cd build && cmake --build .` or `cd /workspaces/libjuno/vscode-extension && npm test`.
3. **Include the working directory in every agent brief** that involves running terminal commands.
4. **Two project roots exist**: `/workspaces/libjuno` (C library, Python scripts) and `/workspaces/libjuno/vscode-extension` (TypeScript extension).
