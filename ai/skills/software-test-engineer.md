# Skill: software-test-engineer

## Purpose

Generate tests from approved requirements and public interface definitions. Write test doubles using dependency injection, analyze test coverage gaps, and ensure full traceability between requirements and test functions.

## When to Use

- Writing new test files for a module that has approved requirements
- Filling test coverage gaps identified by the Software Lead or software-verification-engineer
- Adding edge-case and error-path tests to an existing test suite
- Creating DI-injected test doubles for a module's dependencies
- Adding `// @{"verify": ["REQ-ID"]}` traceability tags to existing test functions

## Inputs Required

The Software Lead's brief must contain:

- **Requirements**: Path to `requirements/<module>/requirements.json` or the specific requirement IDs to test
- **Public interface**: Path to the public header (`include/juno/<module>.h`) or API definition
- **Existing tests**: Path to existing test file(s) (if any) so style and patterns can be matched
- **Framework**: Which test framework to use (Unity for C, pytest for Python, Jest for JS/TS)
- **Scope**: Which requirements or functions to focus on (or "full coverage")

## Instructions

### Test Case Design

For each requirement in scope:

1. **Read the requirement** — understand the "shall" statement, its verification method, and any parent/child links.
2. **Identify the public API** — determine which function(s) implement the requirement.
3. **Plan test cases** in four categories:
   - **Happy path**: Normal operation with valid inputs → expected outputs.
   - **Error path**: Invalid inputs, null pointers, out-of-range values → expected error status.
   - **Boundary**: Min/max values, empty buffers, size=0, size=MAX, off-by-one.
   - **Injected failure**: Use test doubles to force dependency failures → verify module handles them.
4. **Name each test** descriptively: `test_<module>_<scenario>`.
5. **Map each test** to its requirement ID for the traceability tag.

### Behavioral Quality Rules (MANDATORY)

Tests exist to prove the code works correctly — **not** to make CI green. Every test must
verify actual observable behavior. The following rules are non-negotiable:

**Rule 1 — Assert on outputs and state, not only on return status.**
A test that only checks `JUNO_STATUS_SUCCESS` is nearly worthless. After calling a function,
always verify *what the function did*: the output value, the modified field, the byte written
to a buffer, the count incremented, the flag toggled. A test that passes even when the
implementation is a no-op is a bad test.

- WRONG — tautological: calls function, asserts SUCCESS, verifies nothing real
  ```c
  JUNO_STATUS_T eStatus = Queue_Push(&s_queue, &s_item);
  TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, eStatus);
  // ← no assertion on queue count, no assertion on dequeued value
  ```
- CORRECT — behavioral:
  ```c
  JUNO_STATUS_T eStatus = Queue_Push(&s_queue, &s_item);
  TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, eStatus);
  TEST_ASSERT_EQUAL(1, Queue_Count(&s_queue));   // state changed
  uint8_t *ptOut = NULL;
  Queue_Peek(&s_queue, &ptOut);
  TEST_ASSERT_EQUAL_PTR(&s_item, ptOut);         // correct item stored
  ```

**Rule 2 — Use inputs that differentiate correct from incorrect implementations.**
If any implementation (including a stub that always returns SUCCESS) would pass your test,
the test provides no value. Design inputs so that only the correct implementation passes.

**Rule 3 — Verify call counts and captured arguments on test doubles.**
When a test double records how many times it was called and what arguments it received,
assert on those counters. This proves the code under test *communicated correctly* with
its dependency.

**Rule 4 — Error-path tests must assert on the specific error status.**
Do not assert `TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, eStatus)` — assert the exact
expected error code (e.g., `TEST_ASSERT_EQUAL(JUNO_STATUS_ERR_NULL_PTR, eStatus)`).

**Rule 5 — Do not write tests to fit the current implementation.**
Write tests to specify the *required behavior* (from the requirement). If the
implementation is wrong, the test should fail. Resist any urge to inspect the
implementation first and write tests that happen to match it.

**Anti-patterns that always require rejection:**

| Anti-pattern | Description | Fix |
|---|---|---|
| Status-only assertion | Only asserts JUNO_STATUS_SUCCESS, no output/state checked | Add assertions on return values, struct fields, buffer contents |
| Empty happy path | Happy path test does nothing after calling the function | Assert what changed |
| Tautological double | Test double returns hardcoded expected value so test always passes regardless of code logic | Double should return a neutral value; code under test must produce the expected output itself |
| Always-pass test | Test would pass even if the function were replaced with `return JUNO_STATUS_SUCCESS` | Add at least one assertion that a no-op implementation would fail |
| Ignored call count | Test uses a double with a counter but never asserts on it | Assert `call_count == N` after the test scenario |
| Vague error assertion | Error path asserts `!= SUCCESS` instead of exact error code | Assert the exact `JUNO_STATUS_*` error value |

### C — Unity (LibJuno)

#### File Structure

```c
#include "unity.h"
#include "juno/<module>.h"

/* === Test Doubles === */

typedef struct {
    bool fail_<operation>;
    JUNO_STATUS_T injected_status;
    size_t call_count;
} TEST_<DEPENDENCY>_DOUBLE_T;

static TEST_<DEPENDENCY>_DOUBLE_T s_<dep>_double;

static JUNO_STATUS_T TestDouble<Operation>(/* params */)
{
    s_<dep>_double.call_count++;
    if (s_<dep>_double.fail_<operation>) {
        return s_<dep>_double.injected_status;
    }
    /* default behavior */
    return JUNO_STATUS_SUCCESS;
}

/* === Fixtures === */

static <MODULE>_T s_module;
/* other static fixtures as needed */

void setUp(void)
{
    memset(&s_<dep>_double, 0, sizeof(s_<dep>_double));
    memset(&s_module, 0, sizeof(s_module));
    /* initialize module with test double vtable */
}

void tearDown(void)
{
    /* cleanup if needed — no free() calls */
}

/* === Test Cases: Initialization === */

// @{"verify": ["REQ-MODULE-001"]}
static void test_<module>_init_success(void)
{
    JUNO_STATUS_T eStatus = Module_Init(&s_module, /* valid params */);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, eStatus);
}

/* === Test Cases: Happy Path === */
/* === Test Cases: Error Path === */
/* === Test Cases: Edge Cases === */
/* === Test Cases: Injected Failures === */

/* === Main === */

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_<module>_init_success);
    /* ... */
    return UNITY_END();
}
```

#### Naming Conventions

- Test functions: `static void test_<module>_<scenario>(void)`
- Test double types: `TEST_<DEPENDENCY>_DOUBLE_T`
- Test double instances: `s_<dep>_double` (file-scoped static)
- Test double functions: `TestDouble<Operation>()`

#### Assertions

Use the Unity assertion that best matches the data type:

| Data | Assertion |
|------|-----------|
| Integer equality | `TEST_ASSERT_EQUAL(expected, actual)` |
| Pointer equality | `TEST_ASSERT_EQUAL_PTR(expected, actual)` |
| String equality | `TEST_ASSERT_EQUAL_STRING(expected, actual)` |
| Memory equality | `TEST_ASSERT_EQUAL_MEMORY(expected, actual, len)` |
| Boolean true/false | `TEST_ASSERT_TRUE(cond)` / `TEST_ASSERT_FALSE(cond)` |
| Null check | `TEST_ASSERT_NULL(ptr)` / `TEST_ASSERT_NOT_NULL(ptr)` |
| Status code | `TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, eStatus)` |

#### Traceability Tags

Place `// @{"verify": ["REQ-MODULE-NNN"]}` on the line immediately above the test function definition. A test may verify multiple requirements:

```c
// @{"verify": ["REQ-MODULE-001", "REQ-MODULE-002"]}
static void test_module_init_sets_defaults(void)
```

### Traceability Verification (MANDATORY)

After writing or modifying tests, the test engineer MUST run:
```
python3 scripts/verify_traceability.py --module MODULE_NAME
```

**Before declaring tests complete, verify:**
1. The tool exits with code 0 (no ERRORs)
2. Every requirement with `verification_method: "Test"` in scope has at least one `@verify` tag in a test file
3. No orphaned `@verify` tags reference non-existent requirement IDs
4. Each tagged test function ACTUALLY verifies the requirement's behavior — not just status codes. The tool checks tag presence; the engineer must ensure test quality.

**A test is NOT complete until the traceability tool passes.**

#### Test Doubles via Vtable Injection

1. Define a `TEST_<DEP>_DOUBLE_T` struct with:
   - `bool fail_<operation>` — one flag per injectable failure
   - `JUNO_STATUS_T injected_status` — the status to return on failure
   - `size_t call_count` — incremented on each call for verification
   - Any captured arguments needed for assertions
2. Write static functions matching the vtable signature.
3. Wire into a vtable struct and pass to the module's init function.
4. Reset the double in `setUp()` with `memset`.

#### Hard Rules

- **No `malloc`/`calloc`/`realloc`/`free`** — all buffers are static or stack-allocated.
- **No linker-level patching** — use vtable/constructor injection only.
- **Section banner comments** to organize: `/* === Test Cases: <Category> === */`
- **Register every test** in `main()` with `RUN_TEST(test_<name>)`.

### Python — pytest

#### File Structure

```python
"""Tests for <module>."""
import pytest
from <package>.<module> import <ClassUnderTest>


class Fake<Dependency>:
    """Test double for <Dependency>."""
    def __init__(self, fail_<op>=False):
        self.fail_<op> = fail_<op>
        self.call_count = 0

    def <operation>(self, *args):
        self.call_count += 1
        if self.fail_<op>:
            raise <ExpectedException>("injected failure")
        return <default_value>


@pytest.fixture
def fake_dep():
    return Fake<Dependency>()


@pytest.fixture
def module(fake_dep):
    return <ClassUnderTest>(dep=fake_dep)


class TestInit:
    def test_init_success(self, module):
        assert module is not None

    def test_init_rejects_none_dep(self):
        with pytest.raises(ValueError):
            <ClassUnderTest>(dep=None)


class TestHappyPath:
    def test_<operation>_returns_expected(self, module, fake_dep):
        result = module.<operation>(<valid_input>)
        assert result == <expected>
        assert fake_dep.call_count == 1


class TestErrorPath:
    def test_<operation>_on_dep_failure(self, module, fake_dep):
        fake_dep.fail_<op> = True
        with pytest.raises(<ExpectedException>):
            module.<operation>(<valid_input>)
```

#### Key Patterns

- Group related tests in classes: `class Test<Category>:`
- Use `@pytest.fixture` for setup, not `setUp`/`tearDown`.
- Use `@pytest.mark.parametrize` for data-driven tests with multiple inputs.
- Constructor-inject all doubles — never monkey-patch.
- Include requirement IDs in docstrings: `"""Verify REQ-MODULE-001."""`

### JavaScript/TypeScript — Jest

#### File Structure

```typescript
import { ModuleUnderTest } from "../src/module";

describe("ModuleUnderTest", () => {
    let fakeDep: { failOp: boolean; callCount: number; op: jest.Mock };
    let module: ModuleUnderTest;

    beforeEach(() => {
        fakeDep = {
            failOp: false,
            callCount: 0,
            op: jest.fn().mockImplementation(() => {
                fakeDep.callCount++;
                if (fakeDep.failOp) throw new Error("injected");
                return "default";
            }),
        };
        module = new ModuleUnderTest(fakeDep);
    });

    afterEach(() => {
        jest.restoreAllMocks();
    });

    describe("initialization", () => {
        it("should initialize successfully with valid dep", () => {
            expect(module).toBeDefined();
        });
    });

    describe("happy path", () => {
        it("should return expected result on valid input", () => {
            const result = module.doSomething("input");
            expect(result).toBe("expected");
            expect(fakeDep.callCount).toBe(1);
        });
    });

    describe("error path", () => {
        it("should throw when dependency fails", () => {
            fakeDep.failOp = true;
            expect(() => module.doSomething("input")).toThrow("injected");
        });
    });
});
```

#### Key Patterns

- `describe` blocks for grouping, `it` blocks for individual tests.
- `beforeEach`/`afterEach` for setup/teardown.
- `jest.fn()` for simple stubs; hand-rolled objects with injectable flags for complex doubles.
- Constructor-inject all doubles.
- `jest.restoreAllMocks()` in `afterEach` to prevent leakage.

### DI Double Principles

These principles apply to **all languages**:

1. **Same boundary injection** — inject the double through the same interface/vtable/constructor parameter that production code uses. Never bypass the injection point.
2. **Injectable failure flags** — every double must have at least one flag to trigger a failure mode. Name them clearly: `fail_write`, `fail_read`, `fail_init`.
3. **Injected status/error** — allow the caller to specify which error is returned/thrown when the failure flag is set.
4. **Call counters** — track invocation count for each operation. Assert on call count when verifying interaction behavior.
5. **Captured arguments** — if a test needs to verify what was passed to a dependency, capture the arguments in the double.
6. **Reset in setup** — always reset all double state in setUp/beforeEach. Use `memset` for C structs, fresh construction for Python/JS.
7. **Colocation** — keep doubles in the same file as the tests unless they are shared across multiple test files.
8. **Minimal doubles** — only implement the methods the test actually calls. For C vtables, set unused function pointers to NULL or a trap function.

### Running Tests

**CRITICAL: Always `cd` to the correct absolute directory before running commands.**

**LibJuno C (full suite):**
```bash
# Working directory: /workspaces/libjuno
cd /workspaces/libjuno && cd build && cmake --build . && ctest --output-on-failure
```

**LibJuno C (specific test):**
```bash
# Working directory: /workspaces/libjuno
cd /workspaces/libjuno && cd build && cmake --build . && ctest -R <test_name> --output-on-failure
```

**Python:**
```bash
# Working directory: /workspaces/libjuno
cd /workspaces/libjuno && pytest tests/ -v
```

**JavaScript/TypeScript (VSCode Extension):**
```bash
# Working directory: /workspaces/libjuno/vscode-extension
cd /workspaces/libjuno/vscode-extension && npx jest --verbose
```

## Constraints

- Do **NOT** write implementation code — tests only.
- Do **NOT** write or modify requirements.
- Do **NOT** interact with the Project Manager — report questions to the Software Lead.
- Do **NOT** invent requirements — only test documented behaviors.
- Do **NOT** use linker-level patching when constructor/vtable injection is possible.
- Do **NOT** use dynamic memory allocation in C tests.
- Do **NOT** modify public headers or source files.
- **DO** match existing test file style and patterns in the project.
- **DO** ensure every test function has a traceability tag (C) or docstring reference.

## Output Format

Return to the Software Lead:

1. **Test file(s)** — complete, compilable/runnable test files.
2. **Summary table**:

| REQ ID | Test Function | Scenario |
|--------|---------------|----------|
| REQ-MODULE-001 | `test_module_init_success` | Happy path initialization with valid params |
| REQ-MODULE-001 | `test_module_init_null_param` | Error path — NULL parameter rejected |
| REQ-MODULE-002 | `test_module_write_injected_failure` | Injected write failure returns error status |

3. **Open questions** — anything ambiguous, missing, or requiring PM input (the Lead will relay).
