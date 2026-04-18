# Lessons Learned — Software Lead
*Read before every task. Append new entries concisely.*

### 2026-04-17 — Always verify `loadCache` happy path, not just failure modes
- If all active tests only check null/error returns from `loadCache`, a regression to `return null` would pass.
- The senior-software-engineer verifier caught this: TC-CACHE-006 needed `loadCache` success assertions.

### 2026-04-17 — Source bugs found during test writing are valid sprint deliverables
- TC-CACHE-010 revealed that `cacheToIndex` lacked null guards on all 9 `Object.entries()` calls.
- Skipping the test AND fixing the source in the same sprint is the correct workflow.
- Report the bug scope accurately (all 9 fields, not just the one that triggered it).

### 2026-04-14 — Never do hands-on work; delegate everything
- Lead must NEVER write code, run diagnostic commands, or perform technical analysis.
- All hands-on work (incl. bug diagnosis) goes to worker/verifier agents.

### 2026-04-14 — Break work into smallest testable increments with verification gates
- Verify source compiles and existing tests pass BEFORE writing new tests.
- Sequence: fix source → verify → then write tests. Never parallelize dependent work.
- Insert a verification gate between every dependent step.
- Prefer many small work items (1 fix, 1 test file) over large batches.

### 2026-04-14 — Verify worker output by running tests, not just code review
- "Code compiles" AND "tests pass" are both required gates after every worker.

### 2026-04-14 — Chevrotain CST key naming — always verify empirically
- `CONSUME2(X)` → `children["X"][1]`, NOT `children["X2"]`.
- Always instruct developer to dump actual CST keys before writing visitor logic.

### 2026-04-14 — Serialized fix-verify-test plan template
- Phase 1: Serial source fixes — gate after each (spawn worker → verifier → run tests → next).
- Phase 2: Serial test-file fixes, one at a time — only after ALL source bugs verified.
- Phase 3: Final quality gate (full suite, compilation, consistency).
- No parallel work on dependent items. Test execution is a mandatory gate.

### 2026-04-14 — Spawn a diagnostic agent before assuming remaining failures are Phase 2
- When failures remain after planned fixes, diagnose first — don't assume all source bugs are found.
- Brief must ask: "(A) wrong test expectations, (B) visitor bug, (C) parser bug?" — require concrete ts-node evidence.

### 2026-04-15 — MANDATORY Sprint Startup Protocol
Before presenting any sprint plan, Lead MUST:
1. Re-read `ai/skills/software-lead.md` and relevant worker/verifier skill files.
2. Read `requirements/vscode-extension/requirements.json` (or applicable requirements).
3. Read `vscode-extension/design/design.md` and `vscode-extension/design/test-cases.md`.
4. Read `vscode-extension/software-development-plan.md` (current sprint phases).
5. Summarize each document's key points. THEN create and present the sprint plan.
This is non-negotiable every sprint without exception.

### 2026-04-16 — Mutant-killing briefs: ONE code region, ≤10 tests per agent
- Target ONE function/line range per agent; produce at most 10 tests.
- Brief text: ≤50 lines; include only the 20–30 source lines the agent needs.
- Provide the exact test file and insertion point.
- Never include full mutant lists; summarize the target region in ≤2 lines.
- Run tests after each small batch before proceeding.

### 2026-04-16 — Fix all failing tests BEFORE writing new ones
- A failing baseline masks regressions and wastes mutation cycles.

### 2026-04-16 — When PM gives a direct instruction, execute it immediately
- If PM says "create a plan," the very next action is the plan — not more analysis.

### 2026-04-17 — One work item = one phase; never batch serial items into one phase
- Each mutant-killing target (one function/region) is its own independent phase with its own gate.
- Spawn one agent → verify → gate → next agent. No queuing multiple serial agents in one step.

### 2026-04-17 — Always use absolute paths; read directory-map.md before terminal commands
- Read `ai/memory/directory-map.md` before any terminal command.
- Two roots: `/workspaces/libjuno` (C/Python), `/workspaces/libjuno/vscode-extension` (TS).
- Always prefix: `cd /workspaces/libjuno/vscode-extension && npm test`.

### 2026-04-17 — Integration tests: "test like you fly" pipeline reference
- Full pipeline: temp .c files on disk → WorkspaceIndexer.reindexFile() → Chevrotain parse → mergeInto → NavigationIndex → VtableResolver/FailureHandlerResolver.resolve().
- No format mismatches found between visitor output and resolver input (Sprint 6, Phase 9).
- Indexing order matters: JUNO_MODULE_ROOT file must be indexed before files with vtable assignments or failure handler assignments.
- For FailureHandlerResolver, assignment form (ASSIGNMENT_RE) resolves via functionDefinitions directly; Step 2 (failureHandlerAssignments) requires rootType resolution during mergeInto.

### 2026-04-18 — StatusBarHelper timer leak requires afterEach dispose
- When tests create `new StatusBarHelper()` in `beforeEach`, `showError()` schedules a real `setTimeout(5s)`.
- If no `afterEach(() => statusBar.dispose())` is added, Jest warns about leaked timers and force-exits workers.
- Always pair StatusBarHelper creation with disposal in test teardown.

### 2026-04-18 — Parallel WI execution works well for independent source+test splits
- WI-13.4 (JDP tests) and WI-13.5 (MCP tests) ran in parallel successfully — no shared files.
- WI-13.1+WI-13.2 (source changes) must complete before WI-13.4 (test changes) — sequential dependency.
- Audit/report work items (WI-13.6) can always run in parallel with test updates.

### 2026-04-18 — Chevrotain RULE vs plain method for token gobbling
- Creating a new `RULE("macroCallStatement")` that calls `SUBRULE(macroBodyTokens)` caused `RangeError: Maximum call stack size exceeded` in `performSelfAnalysis()`.
- Root cause: `macroBodyTokens` MANY loop accepts any token, creating infinite path expansion when reachable from `statement → compoundStatement → statement`.
- Fix: Use a plain private method with `RECORDING_PHASE` guard instead of a Chevrotain RULE. The GATE provides the lookahead predicate.

### 2026-04-18 — Worker agents may leave temp diagnostic files; always clean up
- A worker left `count-errors-temp.test.ts` in the test directory, inflating test counts.
- Always scan for unexpected test files after worker execution and remove temp artifacts before final gate.

### 2026-04-18 — When SDP header/phase tables are already updated, verify before re-editing
- The SDP was already partially updated by a prior sprint. The junior-software-developer correctly detected this and reported "no changes needed."
- Always read the file first to check current state before spawning editors.
- Sprint Schedule table and phase section headers can drift independently — verifier caught Sprint 13→14 mismatch.

### 2026-04-18 — Agent failures happen; retry with a different agent or smaller scope
- software-developer agent returned no response on a large SDP edit brief.
- Re-spawned as junior-software-developer with a focused brief — succeeded.
- For large mechanical edits, prefer junior-software-developer with exact string specifications.
