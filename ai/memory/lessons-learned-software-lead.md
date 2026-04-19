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
3. Read `vscode-extension/docs/design/index.md` (then relevant section file), and `vscode-extension/docs/test-cases/index.md` (then relevant section file).
4. Read `vscode-extension/docs/sdp/index.md` (then the relevant phase file for the current sprint).
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

### 2026-04-18 — If subagent delegation is rate-limited, switch to explicit manual fallback gates
- When runSubagent is blocked by weekly limits, continue sprint execution manually instead of stalling.
- Keep the orchestration structure: per-work-item change, targeted test gate, then full-suite gate.
- Remove any temporary diagnostic files immediately so verification scope remains clean.

### 2026-04-18 — Chevrotain v12 EOF sentinel: always use `tokenMatcher(t, EOF)`
- `t.tokenType === undefined` is WRONG. Chevrotain's `this.LA(i)` past the token vector returns either JS `undefined` (TypeError on `.tokenType`) or a sentinel token with a valid `tokenType` (infinite loop).
- The correct check is `tokenMatcher(t, EOF)` after importing `EOF` from `"chevrotain"`.
- This bug was NOT caught by 623 tests because all test inputs were well-formed C. Found only via senior engineer code review.
- Lesson: Always run verification on new parser lookahead methods that scan with `LA(i)`.

### 2026-04-18 — `reindexFile()` must mirror `fullIndex()` for all deferred data paths
- `mergeInto()` has an `if (deferred)` guard that silently drops `pendingPositionalVtables` when no array is provided.
- `reindexFile()` (incremental, on file-save) originally didn't pass `DeferredPositional[]`, so cross-file positional vtable initializers were silently dropped.
- Any new deferred data path added to `mergeInto()` must also be threaded through `reindexFile()`.

### 2026-04-18 — Agent failures happen; retry with a different agent or smaller scope
- software-developer agent returned no response on a large SDP edit brief.
- Re-spawned as junior-software-developer with a focused brief — succeeded.
- For large mechanical edits, prefer junior-software-developer with exact string specifications.

### 2026-04-18 — VSIX packaging: always verify runtime deps are bundled
- `.vscodeignore` with `node_modules/**` strips ALL dependencies from the VSIX, including production deps like `chevrotain`.
- Extension installs fine but silently crashes on activation when `require('chevrotain')` fails.
- Fix: Remove blanket `node_modules/**` from `.vscodeignore`; `vsce package` automatically excludes devDependencies.
- Always verify: `ls <installed-extension-path>/node_modules/<runtime-dep>/` after packaging.
- Symptom: "command not found" in Command Palette = extension never activated = check Extension Host log.

### 2026-04-18 — tsconfig.json must exclude __mocks__ to prevent mock contamination in out/
- Jest `__mocks__/` directories are test infrastructure, not production code.
- Without `"**/__mocks__/**"` in tsconfig `exclude`, mocks compile to `out/__mocks__/` and ship in the VSIX.
- Add `"**/__mocks__/**"` to tsconfig `exclude` alongside `"**/*.test.ts"`.

### 2026-04-18 — After any source change, recompile AND repackage AND reinstall before user testing
- Compiled `out/` can be stale if a worker modifies `.ts` but doesn't run `npm run compile`.
- The VSIX can be stale if `out/` was rebuilt but `vsce package` wasn't re-run.
- The installed extension can be stale if the VSIX was rebuilt but not reinstalled.
- Full chain: `npm run compile` → `vsce package` → `code --install-extension` → Reload Window.

### 2026-04-18 — Every plan presented to the PM must include a worker/verifier assignment table
- For every work item, show: WI | Deliverable | Worker | Verifier(s) | Notes.
- Applies to all phases — initial sprint plan, revised plans, and per-phase plans.
- PM explicitly requested this after the initial sprint plan omitted assignments (2026-04-18).
- No exceptions: even single-WI phases need the table.

### 2026-04-18 — Worker agents may not update tests for DI constructor changes
- WI-18.1 (source changes adding `log` param) completed but didn't update mock or tests.
- WI-18.2 (test updates) was planned as sequential but the first worker should have been briefed to at minimum check if tests still pass.
- Always include "run npm test and fix any failures" in worker briefs, even for source-only work items.

### 2026-04-18 — New REQ with `uses` needs reciprocal `implements` update in parent
- Added REQ-VSCODE-033 with `uses: ["REQ-VSCODE-003"]` but forgot to add "REQ-VSCODE-033" to REQ-VSCODE-003's `implements` array.
- `verify_traceability.py` flagged this as a bidirectional inconsistency warning.
- Protocol: whenever a new REQ adds a `uses` link, also update the parent REQ's `implements` list in the same commit.

### 2026-04-18 — `verify_traceability.py` needs `--root` for sub-project requirement trees
- Running the script from `/workspaces/libjuno` looks at `/workspaces/libjuno/requirements/`, which does NOT contain VSCode extension requirements.
- VSCode extension requirements live under `/workspaces/libjuno/vscode-extension/requirements/vscode/requirements.json`.
- Correct invocation: `python3 scripts/verify_traceability.py --root /workspaces/libjuno/vscode-extension`.
- Without `--root`, the script reports spurious "orphaned @verify tag" errors for every VSCode REQ ID.

### 2026-04-18 — Grammar conformance tests: tag the grammar-level REQ, not just the user-visible REQ
- The `c11-grammar-conformance.test.ts` file initially tagged only REQ-VSCODE-005 (user-visible go-to-def behavior).
- REQ-VSCODE-033 (parser C11 conformance) is the direct verification target for these tests.
- Correct: tag BOTH — the grammar REQ (direct) AND the user-visible REQ (indirect outcome).

### 2026-04-18 — C11 §6.5.3 unary-expression recursion target matters for vtable go-to-def
- Root cause of the go-to-def bug: `unaryExpression` recursed to `unaryExpression` for `& * + - ~ !`, violating C11 §6.5.3 which requires recursion to `cast-expression` for these six operators (only `++` and `--` recurse to `unary-expression`).
- Symptom: any function body with `return *(T *) pv;` style expression caused parser to bail mid-body; function definition never emitted; vtable resolver fell back to assignment line.
- Lesson: when fixing parser/grammar bugs, always cite the C11 §/production number in the fix comment so future audits have normative reference.

### 2026-04-18 — Requirements atomicity: UX behavior and implementation mechanism need separate requirements
- Sprint 20: REQ-VSCODE-035 initially bundled "editor shall display underline" (UX behavior) with "provider shall return exactly one location" (implementation mechanism).
- Systems engineer correctly flagged non-atomicity — two independently testable behaviors cannot share one requirement.
- Fix: REQ-VSCODE-034 owns the "underline for all call sites" observable behavior; REQ-VSCODE-035 owns the "return exactly one location for multi-impl" implementation constraint.

### 2026-04-18 — REQ hierarchy: new child requirement's `uses` parent must match semantic level
- Sprint 20: REQ-VSCODE-035 was initially given `uses: ["REQ-VSCODE-005"]` (single-impl navigation) even though it primarily governs multi-impl behavior.
- Systems engineer flagged: REQ-VSCODE-005 is a sibling, not a parent — REQ-VSCODE-007 (Native Go to Definition Integration) is the correct parent for provider-mechanism requirements.
- Rule: when choosing the `uses` parent, match the semantic scope. Provider mechanism reqs (how the extension integrates with VSCode) trace to REQ-VSCODE-007; user-visible navigation reqs trace to REQ-VSCODE-005/006.

### 2026-04-18 — Ambiguous PM UX feedback: describe-behavior ≠ remove-behavior
- Sprint 20 PM said "the window appears before I click" — interpreted as "remove the peek widget."
- Correct interpretation: PM was describing what they observed; the peek widget WAS their desired UX.
- Rule: when PM describes a UX behavior as a symptom, clarify design intent before removing the feature. Ask: "Do you want the peek window at all, or just want it triggered differently?"
- Lesson: description of behavior ≠ request to remove behavior.

### 2026-04-18 — VSCode multi-definition peek: return ALL locations; peek widget IS the desired UX
- Returning `Location[]` with 2+ entries from `provideDefinition` triggers VSCode's native multi-definition peek widget on Ctrl-hold — this is correct, intentional UX.
- Single-impl: return all 1 location → VSCode navigates directly. Multi-impl: return all N locations → VSCode shows native peek widget for selection.
- **NEVER truncate to slice(0,1)** — doing so suppresses the peek and breaks multi-implementation navigation (Sprint 20 regression, fixed in Sprint 21).
- Do NOT use `QuickPick` in the provider — VSCode's native peek is the selection mechanism.

### 2026-04-18 — TC-ID string presence ≠ test coverage; audit must check behaviors
- A gap audit that checks for literal TC-ID strings in test files massively overcounts missing tests.
- Many test files use internal naming (TC-LTI, TC-WI, GRAM-*, TC-RES-BRANCH) that covers the documented TC-LOCAL, TC-P9, TC-SYS, TC-DECL-001–004 behaviors exactly.
- Correct audit: for each documented TC-ID, check whether the described BEHAVIOR is tested — not whether the string appears.
- Practical shortcut: read the test file's header comment and `describe` blocks; they usually state which spec sections they cover.

### 2026-04-18 — Documented behavior with no source implementation cannot be tested green
- TC-CACHE-008 (debounced cache write) was documented for sprints but `reindexFile()` never called `saveToCache()` at all.
- Writing the test alone would always fail (0 calls, not 1). Source change was required first.
- Protocol: when a documented TC describes behavior not found in the source, implement the behavior AND the test in the same work item. Do not attempt to write the test first.

### 2026-04-18 — README stale version strings require a full-file scan, not just the header
- The Overview table was correctly updated to 0.1.7, but the "Install locally" example still had `libjuno-0.1.0.vsix`.
- Quality engineer caught it. Rule: after any version bump, search the entire README for the old version string (`grep "0\.1\.0"`) before closing the work item.

### 2026-04-18 — Discovery file URL ≠ endpoint implementation; both must be in sync
- The MCP discovery file correctly advertised `/mcp` but the server only handled `/resolve_vtable_call` and `/resolve_failure_handler` — the `/mcp` path returned 404.
- Agents got 404 and showed zero tools. Root cause was invisible in unit tests because the tests only hit the REST endpoints directly, never simulating what an MCP client actually sends.
- Rule: whenever a component writes a URL that external clients will call (discovery files, config files, documentation), verify that URL is actually handled by the server with the right protocol.

### 2026-04-18 — Structural detection beats naming-convention heuristics for feature detection
- Sprint 25: initial plan used `*Init*` function name to find composition roots — PM correctly rejected as "weak."
- Better rule: detect the STRUCTURAL INVARIANT (API struct variable's address passed as `&varName` in any call) rather than any naming convention.
- Lesson: when designing detection/resolution logic, ask "what structural property is always true?" not "what naming pattern is common?"

### 2026-04-18 — New NavigationIndex fields require updates in 4 places: createEmptyIndex, clearIndex, removeFileRecords, and test mocks
- Adding `initCallIndex` to NavigationIndex required edits in `navigationIndex.ts` (3 functions) AND in existing test mocks that construct `NavigationIndex` objects directly.
- The test agent correctly found and fixed 2 pre-existing test mocks (`vtableTraceProvider.test.ts`, `workspaceIndexer.test.ts`) that failed to compile after the new field was added.
- Rule: when adding a field to a shared interface like NavigationIndex, grep for all test files that construct the interface directly and update them in the same work item.

### 2026-04-19 — Always verify LibJuno API shape from actual header files before designing against it
- Sprint 30: broker API had 4 critical mismatches (Dequeue doesn't exist on broker, RegisterSubscriber takes 2 not 3 args, Publish needs JUNO_POINTER_T not raw pointer, empty queue returns JUNO_STATUS_OOB_ERROR not JUNO_STATUS_EMPTY).
- The senior engineer caught all four by reading `broker_api.h` and `juno_buff_queue.c` directly.
- Rule: any time a design doc describes API calls on a LibJuno module, the senior engineer verifier MUST check the actual header file before approving. Do not trust doc or design memory alone.

### 2026-04-19 — Design docs for example projects require explicit verification of API usage patterns
- Sprint 30: worker designed application algorithms based on assumed API shape; all four Broker/Pipe API calls were wrong.
- Fix required restructuring dequeue, publish, and register-subscriber pseudocode in 04-applications.md.
- Rule: when spawning a design worker, brief them to explicitly list ALL API calls they will use and their signatures — then spawn a senior engineer to cross-check those signatures against headers before approving.

### 2026-04-18 — Two JSON-RPC edge cases senior SE will always flag: notifications and missing params
- Notifications (no `id` field) must return HTTP 202 with no body — NOT a MethodNotFound error. A specific check for one notification name is insufficient; the guard must cover ALL methods with no `id`.
- `tools/call` must validate `arguments` fields before passing to the handler, returning `-32602 InvalidParams` on missing/wrong-typed fields. Silently casting `undefined` as `string` is a latent crash.
- Both issues are caught by code review, not unit tests (unit tests typically supply well-formed inputs). Always ask: "what happens if `id` is absent?" and "what happens if required `arguments` fields are missing?" when implementing any JSON-RPC handler.
