# Skill: write-tests

## Purpose

Generate tests for a module or component from approved requirements and the
public interface defined in the design. This skill supports any language —
C, Python, JavaScript/TypeScript, or others — and applies dependency
injection (DI) double patterns so every test is isolated and repeatable.

## When to Use

- Writing tests for a new module or component
- Adding coverage for untested requirements
- Adding edge-case or error-path tests to an existing test file

## Inputs Required

- **Module / component name** (e.g., `heap`, `user_service`, `eventBus`)
- **Language** (e.g., C, Python, JavaScript/TypeScript)
- **Test framework** (if known; otherwise infer from the project)
- **Requirements to cover** (optional): specific IDs, or "all untested"
- **Scope** (optional): specific functions or behaviors to target
- **Project conventions** (if any): naming rules, fixture patterns, banned
  APIs, or framework-specific style

## Instructions

> **Software Lead**: See `ai/skills/software-lead.md` → Write Tests for planning and verification steps.

### Software Developer Role

1. Read all provided context before writing a single test:
   - Requirements file — every "shall" statement and its verification method
   - Public interface (header, class definition, type exports) — the contract
     under test
   - Implementation source — to understand error paths and edge cases
   - Any existing test file — to match established fixture, double, and
     assertion patterns
   - Project coding standards provided by the Software Lead

2. Identify gaps: requirements whose `verification_method` is "Test" but
   lack a `@{"verify": ...}` tag (or equivalent) in the test file.

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

5. Submit all test files to the Software Lead for review.

## Constraints

- DO NOT write implementation code or requirements.
- DO NOT use linker-level or module-level patching when constructor injection
  is possible.
- DO NOT invent requirements — only test what is documented.
- Follow project-specific constraints provided by the Software Lead.
- Traceability tags go above each individual test function/block, not file-level.

## Output Format

- New or modified test file(s)
- Summary table: REQ ID → Test Function / Block → Scenario

## Example Invocations

> Use skill: write-tests
> Module: heap
> Language: C (LibJuno)
> Cover: all untested requirements

> Use skill: write-tests
> Module: user_service
> Language: Python
> Framework: pytest
> Cover: all requirements

> Use skill: write-tests
> Module: eventBus
> Language: TypeScript
> Framework: Jest
> Cover: REQ-EVENTBUS-001, REQ-EVENTBUS-002
