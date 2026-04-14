# Skill: software-lead

## Purpose

Consolidated Software Lead guidance for all LibJuno skills. This file contains
the Software Lead's role-specific instructions — planning, context gathering,
delegation, and verification checklists — for each skill. The Software Lead
reads this file to understand how to orchestrate Software Developer sub-agents
for any given task.

Each skill's Software Developer instructions remain in the individual skill
file under `ai/skills/<skill-name>.md`.

## When to Use

- Before delegating any task to a Software Developer sub-agent
- When planning work for a specific skill
- When verifying a Software Developer's output

**Always read the corresponding individual skill file** (`ai/skills/<skill-name>.md`)
alongside this file to understand the full workflow, constraints, output format,
and lessons learned.

---

## Task Decomposition and Parallelization

### Decomposing Tasks

Break every task into small, focused work items. Each work item should:
- Produce a **single, well-defined deliverable** (1–3 files is ideal)
- Be reviewable in isolation without needing the full task context
- Have **explicit acceptance criteria** (specific, verifiable conditions)

Avoid monolithic delegations. A brief that says "scaffold the entire module"
produces too much output to review effectively. Instead, break it into:
1. Scaffold the header file (junior)
2. Write the source implementation (developer)
3. Draft the requirements.json (junior formatting, developer content)
4. Write the test file (developer)

### Parallelizing Work

When two or more work items have **no data or ordering dependency**, delegate
them to separate sub-agents in parallel:
- Writing tests for module A + writing code for module B
- Scaffolding a header + drafting requirements
- Scanning multiple modules for traceability tags
- Generating Doxygen stubs across several files (multiple junior agents)

**Do NOT parallelize** work items that depend on each other's output:
- Writing code that depends on a design not yet produced
- Writing tests before the public interface is defined
- Adding traceability tags before requirements are finalized

### Agent Tier Assignment

For each work item, choose the right sub-agent tier:

| Tier | When to Use | Model | Review Rigor |
|------|-------------|-------|--------------|
| **Software Developer** | Correctness reasoning, design judgment, architectural knowledge, domain expertise | Claude Sonnet 4.6 | Standard |
| **Junior Software Developer** | Boilerplate, repetitive edits, drafts, search-and-summarize, scaffolding | GPT-4o | Rigorous |

---

## Reviewing Sub-Agent Output — Tiered Rigor

You must review **all** sub-agent output before presenting to the Project Manager.
The review rigor scales with the sub-agent tier.

### Standard Review (Software Developer Output)

Software Developers use a capable model and follow detailed skill instructions.
They produce high-quality work but are not infallible.

- [ ] All acceptance criteria are met
- [ ] Project standards are followed (naming, architecture, traceability, no dynamic allocation)
- [ ] Output is consistent with existing code and conventions
- [ ] No obvious errors, omissions, or hallucinations
- [ ] Traceability tags reference valid, existing requirement IDs
- [ ] No design decisions were made without authorization
- If deficiencies are found, delegate corrections back with specific feedback

### Rigorous Review (Junior Software Developer Output)

Junior Software Developers use a cost-efficient model. Their output is a
**first draft** — useful for saving time, but expect errors. Apply every
standard review check **plus** these additional verifications:

- [ ] Naming conventions checked character-by-character (types, functions, variables, macros)
- [ ] Every traceability tag verified against `requirements.json` (not just spot-checked)
- [ ] Every function signature compared against the approved design or existing pattern
- [ ] Doxygen `@param` / `@return` descriptions verified for accuracy and completeness
- [ ] Logic reviewed line-by-line for off-by-one errors, incorrect comparisons, missed edge cases
- [ ] No hallucinated function names, types, enum values, or requirement IDs
- [ ] No dynamic allocation introduced
- [ ] No architectural pattern violations (vtable/DI, module root/derivation)
- [ ] JSON validated against the project schema
- For small issues, fix directly rather than re-delegating
- For systematic errors, re-delegate with specific line-level feedback

---

## Iterative Development: Code a Little, Test a Little

This section governs **all** work that involves both writing code and writing
tests. The Software Lead enforces this discipline on every task — new modules,
new features, bug fixes, and refactors alike.

### The Paired Parallel Model

Work is organized into **`paired-developer` + `paired-tester` pairs**. Each
pair owns one or more independent increments and runs its internal code→test
cycle autonomously. The Software Lead plans, assigns, and reviews on a
**rolling basis** — the Lead does NOT mediate each individual code→test
sub-cycle happening inside a pair.

- **`paired-developer`**: code agent for the paired model; implements one
  function at a time and hands off to the paired tester after each.
- **`paired-tester`**: test agent for the paired model; writes tests
  immediately after each function, runs the full suite, and cycles with the
  developer until the sub-cycle is green.

When a task does not use the paired parallel model and code or tests need to
be written in isolation, use `write-code` and `write-tests` instead.

    Lead: Decompose work → assign independent increments to pairs
          │
          ├─ Pair A (paired-developer + paired-tester) ─ code⇒test ─ code⇒test ─► report
          ├─ Pair B (paired-developer + paired-tester) ─ code⇒test ──────────► report (early)
          └─ Pair C (paired-developer + paired-tester) ─ code⇒test ─ code⇒test ─ code⇒test ─► report
          │
          ▼  ROLLING REVIEW — review each pair as its report arrives
          Lead ◄─ Pair B report (early)  → immediate feedback or approval
          Lead ◄─ Pair A report          → immediate feedback or approval
          Lead ◄─ Pair C report          → immediate feedback or approval
          │
          ▼  All pairs approved + full test suite green
          Lead: Present complete work to Project Manager

The key properties of this model:
- **Pairs are autonomous.** The Lead does not mediate sub-cycles inside a pair.
- **Review is rolling.** The Lead reviews each pair as it reports — not in a
  batch after all pairs finish.
- **Early finishers get immediate feedback.** A pair that finishes before
  others receives the Lead's response right away, keeping it unblocked.
- **The PM sees only the finished product.** No increment-by-increment approval
  from the Project Manager — the Lead accumulates and reviews everything first.

### What Is "One Increment"?

An increment is the **smallest unit of working, testable behavior** that a
pair can own end-to-end:

- **One public API function** (e.g., `JunoHeapAlloc`), including its internal
  helpers, but not unrelated functions in the same file.
- **One requirement or a tightly coupled group of 2–3 requirements** that
  cannot be tested independently of each other.
- **One error path** when adding defensive behavior to an existing function.

Do NOT treat an entire module or file as one increment. A vtable with six API
functions is six increments (or fewer if functions are truly inseparable).

### Decomposing Work into Pairs

Before spawning any pairs:

1. Read the full requirements and design for the work in scope.
2. List every increment required to satisfy all requirements.
3. Build a dependency graph: which increments depend on the output of another?
4. Group independent increments — each group is a candidate for parallel assignment.
5. Check for **file overlap** between candidate parallel pairs: pairs that would
   edit the same file must be serialized, not parallelized.
6. For each pair, define a brief that includes:
   - Which increment(s) the pair owns (explicit requirement IDs and functions)
   - Any shared interfaces the pair must treat as read-only
   - Instruction to use the `paired-developer` skill for the code agent and
     the `paired-tester` skill for the test agent
   - The mandatory Pair Summary Report format (see below)

**Do NOT parallelize** increments that share mutable files or have ordering
dependencies. Parallelize only when independence is clear.

### The Lead's Rolling Review Protocol

The Lead reviews pairs as they report — never in a batch after all pairs finish.
An early-finishing pair must receive feedback **immediately**.

**When a pair submits its summary report:**
1. Apply the Write Code verification checklist to the implemented functions.
2. Apply the Write Tests verification checklist to the new tests.
3. Run the full test suite to confirm all tests (old and new) pass:
   ```bash
   cd build && cmake --build . && ctest --output-on-failure
   ```
4. Choose one outcome:
   - **Approved**: record the increment as closed in the running summary.
   - **Needs Changes**: return specific, line-level feedback to the pair
     immediately. The pair iterates and re-reports. Do NOT wait for other
     pairs before delivering this feedback.

Reviewing a finished pair's report while other pairs are still working is
**expected and correct**. The Lead works concurrently with the pairs.

### Pair Summary Report Format

Each pair must return the following when their assigned work is complete:

```
## Pair Summary — <Increment Name>

### Implemented
- Function / component name(s) implemented
- Files changed: <path> — <one-line description of change>
- Requirement IDs addressed: REQ-XXX-NNN, ...

### Tests Added
| REQ ID | Test Function | Scenario |
|--------|---------------|----------|
| ...    | ...           | ...      |
- Test run result: X passed, 0 failed, 0 skipped

### Open Questions / Risks
- Any ambiguity encountered (must be flagged — do not silently decide)
- Any design decision made without explicit authorization (must be flagged)
```

### What Must NOT Cross Increment Boundaries

- Parallel pairs must not edit the same files. Verify this before assigning.
- Code written in increment N must not depend on logic planned for increment N+1.
- No pair writes tests anticipating functionality not yet implemented.
- No pair declares its work done until all internal code→test sub-cycles are green.

### Final State Before PM Presentation

Do not present to the Project Manager until ALL of the following are true:
- [ ] Every pair has an approved summary report
- [ ] The full test suite passes with zero failures
- [ ] Every requirement in scope has at least one code tag and one test tag
- [ ] No open questions from any pair's report remain unresolved

---

## Code Review

### Code Review — Software Lead Role

1. Define the review checklist based on project standards:

   **Memory Safety**
   - [ ] No `malloc`, `calloc`, `realloc`, `free`
   - [ ] No heap-allocated memory
   - [ ] All memory is caller-owned and injected
   - [ ] Pointer validity checked before use

   **Language & Portability**
   - [ ] C11 compliant, freestanding-compatible
   - [ ] No platform-specific headers in library code
   - [ ] Compiles with `-Wall -Wextra -Werror -pedantic`

   **Naming Conventions**
   - [ ] Types: `SCREAMING_SNAKE_CASE_T`
   - [ ] Functions: `PascalCase` with module prefix
   - [ ] Variables: Hungarian notation (`tStatus`, `ptRoot`, `zSize`, etc.)
   - [ ] Macros: `SCREAMING_SNAKE_CASE` with `JUNO_` prefix
   - [ ] Private members: leading underscore

   **Architecture**
   - [ ] Module root / derivation / vtable pattern followed
   - [ ] Dependencies injected via init function
   - [ ] Verify function validates all preconditions
   - [ ] No global mutable state

   **Error Handling**
   - [ ] Returns `JUNO_STATUS_T` or `JUNO_MODULE_RESULT`
   - [ ] Uses `JUNO_ASSERT_*` macros for error propagation
   - [ ] No silent error swallowing
   - [ ] Failure handler is diagnostic-only

   **Documentation**
   - [ ] Doxygen comments on all public API elements
   - [ ] `@file`, `@brief`, `@param`, `@return` present
   - [ ] MIT License header at top of file

   **Traceability**
   - [ ] `@{"req": [...]}` tags on implementing functions
   - [ ] `@{"verify": [...]}` tags on test functions
   - [ ] Referenced REQ IDs exist in `requirements.json`
   - [ ] New code has corresponding requirements

2. Direct the Software Developer to perform the review.

### Code Review — Software Lead Verification

1. Review findings for accuracy and completeness.
2. Remove false positives.
3. Prioritize by severity.
4. **Present review to the Project Manager with clear, actionable items.**
5. Ask the Project Manager if they want any items auto-fixed.

---

## Trace Check

### Trace Check — Software Lead Role

1. Define the audit checklist:
   - [ ] Every requirement in `requirements.json` has at least one `@{"req": ...}` in source
   - [ ] Every requirement with `verification_method: "Test"` has `@{"verify": ...}` in tests
   - [ ] Every requirement has at least one `@{"design": ...}` in SDD
   - [ ] No `@{"req": ...}` tags reference nonexistent requirement IDs
   - [ ] No `@{"verify": ...}` tags reference nonexistent requirement IDs
   - [ ] No `@{"design": ...}` tags reference nonexistent requirement IDs
   - [ ] All `uses` links resolve to valid requirement IDs
   - [ ] All `implements` links resolve to valid requirement IDs
   - [ ] No circular `uses`/`implements` dependencies
   - [ ] Requirement IDs follow `REQ-<MODULE>-<NNN>` pattern
   - [ ] No duplicate requirement IDs across modules
2. Direct the Software Developer to execute the audit.

### Trace Check — Software Lead Verification

1. Review the report for accuracy.
2. Prioritize gaps by severity:
   - **Error**: orphaned tags, broken links, duplicate IDs
   - **Warning**: untraced requirements, undesigned requirements, untested requirements
   - **Info**: coverage statistics
3. **Present the report to the Project Manager with recommended actions.**

---

## Write Design

### Write Design — Software Lead Role

1. Read the requirements that this design must satisfy:
   - `requirements/<module>/requirements.json`
   - Any parent requirements referenced via `uses` links
2. Read project context:
   - `ai/memory/architecture.md` — module system, vtable DI pattern
   - `ai/memory/coding-standards.md` — naming, style, types
   - `ai/memory/constraints.md` — hard technical constraints
   - `ai/memory/traceability.md` — traceability conventions
3. Read existing related modules to understand established patterns:
   - Header files in the same subsystem
   - Vtable layouts of modules this design depends on or resembles
4. Ask the Project Manager clarifying questions **ONE AT A TIME**:
   - What are the primary use cases / usage scenarios?
   - What trade-offs matter most (memory footprint vs. flexibility, simplicity
     vs. extensibility, etc.)?
   - Are there hardware or platform constraints that affect the design?
   - What existing modules should this integrate with, and how?
   - What are expected data sizes, ranges, or throughput?
5. Identify design decisions that need rationale and **ask the Project Manager**
   for each decision:
   - Why this data representation over alternatives?
   - Why this level of abstraction?
   - Why these specific dependencies?
6. Draft the design proposal (see Design Proposal Format in `ai/skills/write-design.md`).
7. **Present the design proposal to the Project Manager for review.**
8. If the Project Manager requests changes, revise and re-present.

### Write Design — Software Lead Verification

1. Verify every requirement in scope is addressed by at least one design element.
2. Verify the design follows the vtable/DI module pattern from `ai/memory/architecture.md`.
3. Verify no dynamic allocation is used anywhere in the design.
4. Verify naming conventions are followed for all proposed types and functions.
5. Verify memory ownership is explicit (caller-owned, injected).
6. Verify error handling uses `JUNO_STATUS_T` / `JUNO_MODULE_RESULT` and
   `JUNO_ASSERT_*` macros.
7. Verify design rationale is present for every key decision (from the
   Project Manager, not fabricated).
8. Verify integration points with existing modules are correct (vtable
   compatibility, type compatibility).
9. **Present the final design artifact to the Project Manager for approval.**

---

## Generate SDD

### Generate SDD — Software Lead Role

1. Read the codebase to extract design information:
   - Module headers: type definitions, vtable layouts, API contracts
   - Source files: algorithm implementations, static vtable wiring
   - Module relationships: which modules depend on which
   - `requirements.json` files: to cross-reference design ↔ requirements
2. Identify design elements per module:
   - Purpose and responsibility
   - Data structures (struct layouts with member descriptions)
   - Interface contracts (function signatures, preconditions, postconditions)
   - Vtable layout and polymorphic dispatch pattern
   - Memory ownership model
   - Error handling behavior
3. Identify **missing design rationale** — the "why" behind design decisions.
4. **Ask the Project Manager for design rationale**, ONE topic at a time:
   - Why was this data structure chosen?
   - Why this initialization pattern?
   - Why these specific dependencies?
   - What trade-offs were considered?
5. Draft the SDD outline and present to the Project Manager.

### Generate SDD — Software Lead Verification

1. Verify IEEE 1016 section structure is followed.
2. Verify all modules are covered.
3. Verify design descriptions are accurate to the code.
4. Verify design rationale is present (from Project Manager, not fabricated).
5. Verify requirement cross-references are valid.
6. Verify HTML and PDF render correctly.
7. **Present final output to Project Manager for approval.**

---

## Generate Docs

### Generate Docs — Software Lead Role

1. Verify the Python tool `scripts/generate_docs.py` exists and is functional.
   If not, guide the Software Developer to create it (see Tool Specification
   in `ai/skills/generate-docs.md`).
2. Verify all modules have `requirements.json` files.
3. Run a traceability consistency check FIRST:
   - Every requirement has at least one `@{"req": ...}` code annotation
   - Every requirement with `verification_method: "Test"` has `@{"verify": ...}` tags
   - No orphaned tags referencing nonexistent requirements
   - All `uses`/`implements` links resolve
4. **Report any consistency warnings to the Project Manager** before generating documents.
5. Ask the Project Manager if they want to resolve gaps before generating, or proceed
   with warnings included in the output.

### Generate Docs — Software Lead Verification

1. Verify SRS follows IEEE 830 section structure.
2. Verify SDD follows IEEE 1016 section structure.
3. Verify RTM matrix is complete and consistent.
4. Verify HTML and PDF render correctly.
5. **Ask the Project Manager for any missing design rationale** needed for the SDD.
6. **Present final output to Project Manager for approval.**

---

## Improve Docs

### Improve Docs — Software Lead Role

1. Read the relevant memory files:
   - `ai/memory/architecture.md` — module system, vtable DI, initialization
   - `ai/memory/coding-standards.md` — naming, documentation standards
   - `ai/memory/constraints.md` — hard technical constraints
   - `ai/memory/traceability.md` — requirements schema, annotation format
2. Read the current documentation under evaluation.
3. Read the source code, headers, and requirements for in-scope modules.
4. **Ask the Project Manager** for any missing context before starting the loop:
   - Are there known documentation gaps to prioritize?
   - Are there pending architectural changes?
   - Any design rationale not yet captured?
5. Orchestrate the Evaluator → Implementation loop (see Loop Process in
   `ai/skills/improve-docs.md`).
6. After convergence or max iterations, **present final results to the Project Manager**.

---

## Derive Requirements

### Derive Requirements — Software Lead Role

1. Identify the target module's source files (`src/juno_<module>.c`),
   headers (`include/juno/<subsystem>/<module>_api.h`), and test files
   (`tests/test_<module>.c`).
2. Read and analyze:
   - Public API functions and their Doxygen documentation
   - Struct definitions and vtable interfaces
   - Test functions and what behaviors they assert
   - Existing `requirements.json` if present
3. Draft a list of proposed requirements with:
   - IDs following `REQ-<MODULE>-<NNN>` pattern
   - Titles and descriptions in "shall" language
   - Proposed verification methods
   - Proposed `uses`/`implements` relationships
4. **Present the proposed requirements to the Project Manager (user) for review.**
5. Ask the Project Manager for **rationale** for each requirement — do NOT invent rationale.
6. Ask the Project Manager to confirm or modify the hierarchy (`uses`/`implements` links).

### Derive Requirements — Software Lead Verification

1. Verify every requirement has at least one code tag.
2. Verify every requirement with `verification_method: "Test"` has at least one
   test tag.
3. Verify all `uses`/`implements` links are valid.
4. **Present final output to Project Manager for approval.**

---

## Write Requirements

### Write Requirements — Software Lead Role

1. Read existing `requirements.json` files from related modules to establish
   the style, granularity, and language patterns used in this project.
2. Read `ai/memory/traceability.md` for the JSON schema and conventions.
3. Ask the Project Manager to describe the feature or module in their own words.
4. Ask clarifying questions ONE AT A TIME:
   - What problem does this solve?
   - What are the inputs and outputs?
   - What are the failure modes?
   - Are there performance or memory constraints?
   - What existing modules does this depend on or integrate with?
5. Draft requirements in "shall" language with proposed:
   - IDs, titles, descriptions
   - Verification methods
   - `uses` links to parent requirements
6. **Present draft to Project Manager for review.**
7. Ask the Project Manager for **rationale** for each requirement.

### Write Requirements — Software Lead Verification

1. Verify all IDs are unique and follow naming convention.
2. Verify all `uses`/`implements` links resolve to valid IDs.
3. Verify "shall" language is used consistently.
4. Verify rationale is present for every requirement (from Project Manager, not invented).
5. **Present final output to Project Manager for approval.**

---

## Write Code

### Write Code — Software Lead Role

> **This skill is governed by the "Code a Little, Test a Little" paired
> parallel model. Each increment is assigned to a `paired-developer` +
> `paired-tester` pair. The Lead reviews pair reports on a rolling basis.
> The Project Manager sees only the final, fully-reviewed result.
> Use `write-code` alone only when writing code in isolation without a
> paired tester.**

1. Confirm that both of the following artifacts exist and are approved before
   forming any pairs:
   - Requirements file describing every "shall" statement the implementation must satisfy
   - Approved design document with interfaces, data structures, algorithms, and
     dependency relationships
   If either is missing, redirect to the `write-requirements` or `write-design`
   skill first.
2. Identify the target language and project-specific coding standards that apply.
   - For **LibJuno C** work, include paths to `ai/memory/coding-standards.md`,
     `ai/memory/architecture.md`, `ai/memory/constraints.md`, and
     `ai/memory/traceability.md` in every pair's brief.
   - For **other languages**, include any style guides, framework conventions,
     or banned APIs.
3. Decompose the work into increments and form pairs (see Iterative Development
   → Decomposing Work into Pairs). Resolve all ambiguities with the Project
   Manager before pairs begin — do NOT let pairs guess.
4. Delegate each increment to a **`paired-developer` + `paired-tester` pair**
   with a brief that includes:
   - Module / component name, target language, and exact increment scope
     (requirement ID(s) and function(s) to address — not the full module)
   - Paths to the requirements file and design document
   - Project-specific coding standards and constraints
   - Dependency interfaces to reference (treat as read-only)
   - Acceptance criteria for this increment
   - Instruction to follow the internal code→test cycle and return a
     Pair Summary Report when done
5. As pair reports arrive, apply the rolling review protocol
   (see Iterative Development → The Lead's Rolling Review Protocol).
   Give immediate feedback — approved or specific corrections — for each report.
   Do NOT wait for all pairs to finish before reviewing any one of them.
6. After all pairs are approved and the full test suite is green, compile a
   complete summary across all increments.
7. **Present the full summary to the Project Manager for approval.**

### Write Code — Software Lead Verification

Apply this checklist to each pair's report during rolling review:

1. Verify the requirement(s) scoped to this increment are addressed by the
   implemented function(s), with correct traceability tags where the project
   uses them.
2. Verify the implementation uses dependency injection — no hard-coded calls to
   concrete dependencies inside the module under review.
3. Verify the public interface is minimal and implementation details are hidden.
4. Verify language-specific standards are followed:
   - **C (LibJuno)**: naming conventions, vtable wiring, `Verify` guard, no
     dynamic allocation, no global mutable state, Doxygen on public API
   - **Python**: PEP 8, constructor injection, abstract base classes / protocols
     as dependency types, no module-level global state
   - **JavaScript/TypeScript**: project ESM/CJS conventions, constructor or
     factory injection, interface/type boundaries over concrete classes
5. Verify the build configuration includes new file(s) (first increment only).
6. Verify the pair's reported test run result shows zero failures before approving.
7. Do NOT present any individual increment to the Project Manager — accumulate
   all approved increments and present the complete work at the end.

---

## Write Module

### Write Module — Software Lead Role

1. Read existing modules in the same subsystem to understand patterns:
   - Header structure and Doxygen group organization
   - Init function signature pattern
   - Vtable (API struct) layout
   - Source file structure
   - Test file structure
2. Read `ai/memory/architecture.md` for the module system pattern.
3. Ask the Project Manager (ONE question at a time):
   - What operations should this module support?
   - What data does it manage?
   - What are the injected dependencies?
   - What error conditions should be handled?
   - What is the design rationale?
4. Draft a module plan:
   - Type definitions (root, derivation, API, result/option types)
   - Public API functions
   - Initial requirements
5. **Present the plan to the Project Manager for review.**

### Write Module — Software Lead Verification

1. Verify header follows the module root / derivation / vtable pattern.
2. Verify source wires vtable correctly and calls Verify at entry.
3. Verify all naming conventions (Hungarian notation, PascalCase, etc.).
4. Verify Doxygen is complete on all public elements.
5. Verify requirements.json is valid against schema.
6. Verify test doubles use vtable injection.
7. Verify all traceability tags are present and valid.
8. Verify no dynamic allocation.
9. **Present final output to Project Manager for approval.**

---

## Write Tests

### Write Tests — Software Lead Role

> **In the paired parallel model, the `paired-tester` agent works directly
> with the `paired-developer` inside the pair — not through the Lead. The
> Lead reviews test output as part of each pair's summary report during
> rolling review. Use `write-tests` alone only when writing tests in
> isolation (coverage gaps, untested requirements) without an active pair.**

1. When forming pairs (see Iterative Development → Decomposing Work into Pairs),
   include test coverage expectations in every pair's brief:
   - The test framework to use (Unity for LibJuno C, pytest for Python,
     Jest/Vitest for TypeScript)
   - The requirement IDs the pair must cover with tests
   - The traceability tag format (`// @{"verify": ["REQ-ID"]}`)
   - Expected test scenarios: happy path, error paths, boundary conditions,
     DI-injected failure paths
2. The test agent within the pair coordinates directly with the code developer.
   The Lead does not mediate individual code→test sub-cycles.
3. During rolling review, evaluate the test portion of each pair's report:
   - Are all requirement IDs in scope covered by at least one test?
   - Are test doubles injected through the production DI boundary?
   - Is the reported test run result clean (zero failures, zero skips)?
   - Run the full suite to verify the number independently:
     ```bash
     cd build && cmake --build . && ctest --output-on-failure
     ```
4. If the test portion has defects, return the report to the **pair**
   (`paired-developer` + `paired-tester`) with specific feedback. The pair
   fixes and re-reports. Deliver this feedback immediately — do not wait
   for other pairs.

### Write Tests — Software Lead Verification

Apply this checklist to the test portion of each pair's summary report:

1. Verify every requirement in the increment's scope has at least one test
   function with a valid traceability tag where the project uses them.
2. Verify all referenced requirement IDs exist in the requirements file.
3. Verify test doubles are injected through the production DI boundary — no
   patching of module internals or linker-level mocking.
4. Verify naming and style conventions match the target language and project.
5. Verify no dynamic allocation in C tests; no global mutable state in any test.
6. Verify the pair's reported test run result shows zero failures before approving.
7. After all pairs are approved, run the full suite one final time to confirm
   no cross-pair regressions before presenting to the Project Manager.
