# Skill: write-code

## Purpose

Implement production-quality source code from approved requirements and an
approved design document. This skill supports any language — C, Python,
JavaScript, or others — and is not restricted to LibJuno conventions. All
implementations must be modular and use dependency injection (DI) to keep
components independently testable.

## When to Use

- Writing implementation files when the design has been approved
- Completing a partial implementation against a set of requirements
- Updating existing source code to satisfy new or changed requirements

## Inputs Required

- **Module / component name** (e.g., `ringbuffer`, `user_service`, `eventBus`)
- **Language** (e.g., C, Python, JavaScript/TypeScript)
- **Requirements**: requirements file or document describing what the code must do
- **Design document**: approved design describing structure, interfaces, and algorithms
- **Dependencies**: existing modules or libraries whose APIs are consumed
- **Project conventions** (if any): coding standards, style guides, or framework
  patterns specific to this project

## Instructions

> **Software Lead**: See `ai/skills/software-lead.md` → Write Code for planning and verification steps.

### Iterative Development Philosophy: Code a Little, Test a Little

This skill operates as **one half of a developer + test pair**. You work
directly with a test agent through internal code→test sub-cycles. The Software
Lead reviews the pair's output only after the full increment is complete and
all tests are passing.

**Your role in the pair:**
- You implement one function at a time within the assigned increment scope.
- After each function, you hand off to the test agent immediately — before
  writing the next function.
- You wait for the test agent's results. If tests fail due to a code defect,
  you fix it and the test agent re-runs. Repeat until green.
- Once all functions in the increment are implemented and all tests pass, the
  pair co-authors a Pair Summary Report and submits it to the Lead.

**The pair's internal cycle:**

    You write one function
           │
           ▼
    Test agent writes tests for that function
           │
           ▼
    Run full test suite
           │
      Pass? ──Yes──► write next function in increment (repeat)
           │
          No
           │
      Test bug? ──Yes──► test agent fixes, re-runs
           │
          No (code bug) ──► you fix, test agent re-runs

**Escalation rule:**
Resolve sub-cycle failures within the pair. Only escalate to the Lead when
an ambiguity requires a design decision not addressed in the brief.

**When the increment is done:**
All assigned functions are implemented, all tests pass. Co-author the Pair
Summary Report with the test agent (format defined in
`ai/skills/software-lead.md` → Iterative Development → Pair Summary Report
Format). Submit to the Lead and stop — do not begin new work until the Lead
approves and assigns the next increment.

**Do not implement beyond the assigned increment scope.** The Lead controls
sequencing and may parallelize the next increment differently.

### Software Developer Role

1. Read all provided context files before writing a single line of code:
   - Requirements file — every "shall" statement that must be satisfied
   - Design document — approved interfaces, data structures, algorithms,
     and dependency relationships
   - Any project-specific coding standards or conventions provided by the
     Software Lead
   - Existing interfaces or modules this code must integrate with

2. Apply the correct implementation pattern for the target language:

   **C (LibJuno or other C projects)**:
   - Follow conventions from `ai/memory/coding-standards.md` and
     `ai/memory/architecture.md` when working within LibJuno
   - For non-LibJuno C, apply any project standards provided by the Software Lead
   - Use header/source file pairs; inject dependencies through struct function
     pointers or callback parameters
   - Add `// @{"req": ["REQ-ID"]}` traceability tags on implementing functions
     when the project uses the LibJuno traceability system

   **Python**:
   - Prefer classes or modules with explicit constructor injection of dependencies
   - Depend on abstractions (abstract base classes / protocols) rather than
     concrete implementations so dependencies can be swapped in tests
   - Avoid module-level global state; pass state explicitly
   - Follow PEP 8 style and any project-specific style guide
   - Add traceability comments above functions when the project uses them

   **JavaScript / TypeScript**:
   - Use constructor injection or factory functions for dependencies
   - Depend on interfaces/types rather than concrete classes
   - Avoid singleton globals; export factory functions or classes
   - Follow the project's existing style (ESM vs. CommonJS, async patterns, etc.)
   - Add traceability comments above functions when the project uses them

3. **Dependency injection principles (all languages)**:
   - Never hard-code calls to concrete dependencies inside a module — receive
     them as constructor parameters, function arguments, or injected interfaces
   - Expose a clear, minimal public interface; keep implementation details private
   - Design so that any dependency can be replaced with a test double without
     modifying the module under test

4. Update the project's build or package configuration if the new file is not
   yet included (e.g., `CMakeLists.txt`, `package.json`, `setup.py`).

5. Submit all files to the Software Lead for review.

## Constraints

- DO NOT implement more than the scoped increment — if the Software Lead
  specified one function, implement one function and stop.
- DO NOT write tests — that is the `write-tests` skill. Tests follow immediately
  after the Software Lead reviews this increment's code.
- DO NOT write requirements — that is the `write-requirements` skill.
- DO NOT invent design decisions — use only the approved design document; ask
  the Software Lead if anything is ambiguous.
- Rationale must come from the requirements file or the Project Manager via
  the Software Lead — never fabricated.
- Follow project-specific constraints provided by the Software Lead (e.g., no
  dynamic allocation for LibJuno, specific runtime versions, banned APIs).

## Output Format

- All implementation files for the **current increment only**
- Summary per increment:
  - Functions implemented
  - Requirement IDs covered (if applicable)
  - Any open questions for the Software Lead
- Explicitly state: **"Increment complete — ready for tests."**

## Example Invocations

> Use skill: write-code
> Module: ringbuffer
> Language: C (LibJuno)
> Requirements: requirements/ringbuffer/requirements.json
> Design: docs/designs/ringbuffer_design.md

> Use skill: write-code
> Module: user_service
> Language: Python
> Requirements: docs/requirements/user_service.md
> Design: docs/designs/user_service_design.md

> Use skill: write-code
> Module: eventBus
> Language: TypeScript
> Requirements: docs/requirements/event_bus.md
> Design: docs/designs/event_bus_design.md
