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

### Coach Role

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
7. **Present the test plan to the Program for review** before writing code.

### Player Role

8. After Program approval, write test functions following project conventions:
   - `static void test_<module>_<scenario>(void)` naming
   - `// @{"verify": ["REQ-MODULE-NNN"]}` tag above each test function
   - Section banner comments: `/* === Test Cases: <Category> === */`
   - Unity assertions: `TEST_ASSERT_EQUAL`, `TEST_ASSERT_TRUE_MESSAGE`, etc.
   - Global static fixtures with `setUp`/`tearDown` for cleanup
9. Write test doubles as needed:
   - Custom API structs implementing the module's vtable
   - Injectable failure flags (e.g., `bFailInsert`, `bFailCompare`)
   - Callback functions matching the API function pointer signatures
10. Register all tests in `main()` with `RUN_TEST(test_<name>)`.
11. Submit to Coach for review.

### Coach Verification

12. Verify every test function has a valid `@{"verify": ...}` tag.
13. Verify all referenced REQ IDs exist in `requirements.json`.
14. Verify test doubles follow the project's vtable injection pattern.
15. Verify Hungarian notation and naming conventions are followed.
16. Verify the test compiles (check includes, types, function signatures).
17. **Present final output to Program for approval.**

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
