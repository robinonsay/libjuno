# Skill: write-tests

## Purpose

Generate Unity test functions for a given module, including proper
`@{"verify": ["REQ-MODULE-NNN"]}` traceability tags and test doubles
following the project's vtable injection pattern.

## When to Use

- Writing tests for a new module
- Adding test coverage for untested requirements
- Adding edge-case or error-path tests to an existing test file

## Inputs Required

- **Module name** (e.g., `heap`, `memory`, `crc`)
- **Requirements to cover** (optional): specific REQ IDs, or "all untested"
- **Scope** (optional): specific functions or behaviors to test

## Instructions

> **Software Lead**: See `ai/skills/software-lead.md` → Write Tests for planning and verification steps.

### Software Developer Role

1. After Project manager approval, write test functions following project conventions:
   - `static void test_<module>_<scenario>(void)` naming
   - `// @{"verify": ["REQ-MODULE-NNN"]}` tag above each test function
   - Section banner comments: `/* === Test Cases: <Category> === */`
   - Unity assertions: `TEST_ASSERT_EQUAL`, `TEST_ASSERT_TRUE_MESSAGE`, etc.
   - Global static fixtures with `setUp`/`tearDown` for cleanup
2. Write test doubles as needed:
   - Custom API structs implementing the module's vtable
   - Injectable failure flags (e.g., `bFailInsert`, `bFailCompare`)
   - Callback functions matching the API function pointer signatures
3. Register all tests in `main()` with `RUN_TEST(test_<name>)`.
4. Submit to Software Lead for review.

## Constraints

- Follow Unity assertion patterns — no custom assertion frameworks.
- Test doubles must use vtable injection, not linker-level mocking.
- No dynamic allocation in tests (use static/stack buffers).
- Follow all conventions in `ai/memory/coding-standards.md`.
- Tags go above each individual test function, not file-level.

## Output Format

- New or modified `tests/test_<module>.c` file
- Summary table: REQ ID → Test Function → Scenario

## Example Invocation

> Use skill: write-tests
> Module: heap
> Cover: all untested requirements
