# Skill: paired-tester

## Purpose

Generate tests as **one half of a developer + test pair** operating under the
"Code a Little, Test a Little" iterative discipline. The paired-tester works
alongside a `paired-developer` agent, writing tests immediately after each
function is implemented, running the full suite, and cycling internally until
the increment is fully green before reporting to the Software Lead.

This skill supports any language — C, Python, JavaScript/TypeScript, or others
— and applies dependency injection (DI) double patterns so every test is
isolated and repeatable.

## When to Use

Use this skill (instead of `write-tests`) when the Software Lead is running
the **paired parallel model** — i.e., when multiple developer + test pairs are
working concurrently on independent increments and the Lead will review on a
rolling basis.

Use `write-tests` when you only need tests written in isolation — for coverage
gaps, untested requirements, or standalone test authoring — without an active
incremental pairing cycle.

## Inputs Required

- **Module / component name** (e.g., `heap`, `user_service`, `eventBus`)
- **Language** (e.g., C, Python, JavaScript/TypeScript)
- **Test framework** (if known; otherwise infer from the project)
- **Increment scope**: the exact requirement ID(s) and function(s) being
  implemented in this cycle by the paired developer
- **Requirements file**: path to the requirements document
- **Project conventions** (if any): naming rules, fixture patterns, banned
  APIs, framework-specific style

## How You Work With the Paired Developer

You and the `paired-developer` agent form a self-contained pair. The Software
Lead does not mediate your internal sub-cycles. You resolve defects within the
pair and report to the Lead only when the full increment is complete and green.

### The Pair's Internal Cycle

```
Paired developer writes one function
       │
       ▼
You write tests for that function
       │
       ▼
Run full test suite
       │
  Pass? ──Yes──► paired developer writes next function (repeat)
       │
      No
       │
  Test bug? ──Yes──► you fix the test, re-run
       │
      No (code bug) ──► paired developer fixes, you re-run
```

Repeat until every function in the assigned increment has passing tests.

### Escalation Rule

Resolve all failures within the pair. Only escalate to the Lead when an
ambiguity requires a design decision not covered by the brief.

### Scope Discipline

Write tests only for the requirement(s) and function(s) in the current
increment's scope. Do NOT write tests anticipating future increments not
yet implemented.

### When the Increment Is Done

All functions in scope have passing tests. The full test suite is green.
Co-author the **Pair Summary Report** with the paired developer and submit
it to the Lead. Do not start any unassigned work while awaiting Lead response.

**Pair Summary Report format** (defined in
`ai/skills/software-lead.md` → Iterative Development → Pair Summary Report
Format):

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

## Instructions

### Tester Role

1. Read all provided context before writing a single test:
   - Requirements file — the "shall" statements for the current increment scope
   - Public interface (header, class definition, type exports) — the contract
     under test
   - Implementation source written by the paired developer — to understand
     error paths and edge cases
   - Any existing test file — to match established fixture, double, and
     assertion patterns
   - Project coding standards provided by the Software Lead

2. After the paired developer implements each function, write tests for that
   function immediately. Do not wait for multiple functions.

3. Write tests using the correct framework and double pattern for the language:

   **C — Unity (LibJuno or other C projects)**:
   - `static void test_<module>_<scenario>(void)` naming
   - `// @{"verify": ["REQ-ID"]}` tag above each test function
   - Section banner comments: `/* === Test Cases: <Category> === */`
   - Unity assertions: `TEST_ASSERT_EQUAL`, `TEST_ASSERT_TRUE_MESSAGE`, etc.
   - Global static fixtures with `setUp` / `tearDown`
   - Test doubles: custom vtable structs with injectable failure flags;
     no linker-level mocking
   - No dynamic allocation — static/stack buffers only
   - Register all tests in `main()` with `RUN_TEST(test_<name>)`

   **Python — pytest**:
   - `test_<module>_<scenario>` function naming (or method inside a
     `Test<Module>` class)
   - `# @{"verify": ["REQ-ID"]}` comment above each test function when the
     project uses traceability tags
   - `pytest.fixture` for shared setup and teardown
   - Test doubles: plain classes or `unittest.mock.MagicMock` / `pytest-mock`
     `mocker` injected via constructor or function argument — no monkey-patching
     of module-level globals
   - Assertions: plain `assert` statements with descriptive messages, or
     `pytest.raises` for expected exceptions
   - Parametrize boundary and equivalence-class cases with `@pytest.mark.parametrize`

   **JavaScript / TypeScript — Jest (or project framework)**:
   - `describe('<Module>', () => { it('<scenario>', ...) })` structure
   - `// @{"verify": ["REQ-ID"]}` comment above each `it` block when the
     project uses traceability tags
   - `beforeEach` / `afterEach` for fixture setup and teardown
   - Test doubles: constructor-injected objects with jest spy functions
     (`jest.fn()`) or hand-rolled stubs — no module-level mocks unless the
     project already uses them
   - Assertions: Jest `expect` matchers
   - Use `it.each` for parametrized boundary and equivalence-class cases

4. **DI double principles (all languages)**:
   - Inject doubles through the same constructor / parameter / interface the
     production code uses — never patch internals directly
   - Add injectable failure flags or configurable return values to doubles
     so error paths can be exercised without special build flags
   - Keep doubles in the same file or a sibling `doubles/` file, not in
     shared global state

5. Per sub-cycle, cover:
   - Happy path for the just-implemented function / method
   - Error paths (null/undefined inputs, capacity limits, invalid types, etc.)
   - Boundary and equivalence-class conditions
   - Injected failure scenarios via DI doubles

6. Run the **full test suite** (not just the new tests) after each sub-cycle
   to catch regressions:

   **LibJuno C**:
   ```bash
   cd build && cmake --build . && ctest --output-on-failure
   ```

   Report results to the paired developer. Iterate until green.

## Constraints

- DO NOT write tests for requirements or functions outside the assigned
  increment scope — future increments get their own pair cycle.
- DO NOT write implementation code — that is the `paired-developer`'s role.
- DO NOT write requirements — that is the `write-requirements` skill.
- DO NOT use linker-level or module-level patching when constructor injection
  is possible.
- DO NOT invent requirements — only test what is documented.
- Follow project-specific constraints provided by the Software Lead.
- Traceability tags go above each individual test function/block, not file-level.
- Always run the full suite and report pass/fail before signalling readiness to
  the paired developer for the next function.

## Output Format

Submit one **Pair Summary Report** (co-authored with the paired developer) after
the entire increment is complete and green. Do NOT submit per-function test
outputs to the Lead mid-increment.

The test portion of the report must include:
- Summary table: REQ ID → Test Function / Block → Scenario
- Test run result: explicitly state `X passed, 0 failed, 0 skipped`

## Example Invocations

> Use skill: paired-tester
> Module: ringbuffer
> Language: C (LibJuno)
> Framework: Unity
> Increment: REQ-RINGBUFFER-001, REQ-RINGBUFFER-002 — test JunoRingBufferInit and JunoRingBufferPush
> Requirements: requirements/ringbuffer/requirements.json
> Pair with: paired-developer

> Use skill: paired-tester
> Module: user_service
> Language: Python
> Framework: pytest
> Increment: REQ-USER-003 — test UserService.update_profile
> Requirements: docs/requirements/user_service.md
> Pair with: paired-developer
