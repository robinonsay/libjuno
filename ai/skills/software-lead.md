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

1. Read the module's `requirements.json` to identify requirements and their
   verification methods.
2. Read the module's public API header to understand the interface.
3. Read the module's source to understand implementation details and error paths.
4. Read any existing test file for the module to understand:
   - Test double patterns (custom vtables, injectable failure flags)
   - Fixture setup (`setUp`/`tearDown` functions)
   - Assertion patterns used
5. Identify **gaps**: requirements with `verification_method: "Test"` that lack
   `@{"verify": ...}` tags in the test file.
6. Plan test cases:
   - Happy path for each API function
   - Error paths (NULL inputs, full capacity, invalid types, etc.)
   - Boundary conditions
   - Injected failure scenarios (via vtable test doubles)
7. **Present the test plan to the Project Manager for review** before writing code.

### Write Tests — Software Lead Verification

1. Verify every test function has a valid `@{"verify": ...}` tag.
2. Verify all referenced REQ IDs exist in `requirements.json`.
3. Verify test doubles follow the project's vtable injection pattern.
4. Verify Hungarian notation and naming conventions are followed.
5. Verify the test compiles (check includes, types, function signatures).
6. **Present final output to Project Manager for approval.**
