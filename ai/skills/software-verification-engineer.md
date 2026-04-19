# Skill: software-verification-engineer

## Purpose

Audit traceability completeness, requirements coverage, test coverage, and annotation validity. This is the IV&V (Independent Verification & Validation) role.

## When to Use

- After requirements are authored or derived
- After code is written with traceability tags
- After tests are written with verify tags
- Before documentation generation (pre-flight check)
- When the Software Lead needs a traceability health check

## Inputs Required

- Files to audit (source, test, requirements.json, design docs)
- Acceptance criteria from the Software Lead's work breakdown
- Scope (which modules, which requirement IDs)

## Instructions

### Traceability Audit

Perform a complete cross-reference between requirements, source code, tests, and design documents.

**Step 1 — Collect all requirements:**
1. For each module in scope, read `requirements/<module>/requirements.json`.
2. Parse the JSON and extract every requirement object.
3. Build a master list of all requirement IDs, keyed by module.
4. Validate each ID matches the pattern `^REQ-[A-Z]+-[0-9]{3}$`.
5. Check for duplicate IDs across all modules — flag any duplicates as ERROR.
6. Validate required fields exist: `id`, `title`, `description`, `rationale`, `verification_method`.
7. Validate `verification_method` is one of: `Test`, `Inspection`, `Analysis`, `Demonstration`.

**Step 2 — Scan source code for `@req` tags:**
1. Search all `.c` and `.h` files under `src/` and `include/` for the pattern `@{"req":`.
2. Extract the requirement IDs from each tag (e.g., `// @{"req": ["REQ-HEAP-001", "REQ-HEAP-002"]}`).
3. Build a map: requirement ID → list of source file locations where it is tagged.
4. Check for IDs that appear in tags but do NOT exist in any `requirements.json` — flag as ERROR (orphaned code tag).

**Step 3 — Scan test files for `@verify` tags:**
1. Search all `.c` and `.cpp` files under `tests/` for the pattern `@{"verify":`.
2. Extract the requirement IDs from each tag.
3. Build a map: requirement ID → list of test file locations where it is tagged.
4. Check for IDs that appear in tags but do NOT exist in any `requirements.json` — flag as ERROR (orphaned test tag).

**Step 4 — Scan design docs for `@design` tags:**
1. Search all `.adoc` files under `docs/sdd/` for the pattern `@{"design":`.
2. Extract the requirement IDs from each tag.
3. Build a map: requirement ID → list of design file locations where it is tagged.
4. Check for IDs that appear in tags but do NOT exist in any `requirements.json` — flag as ERROR (orphaned design tag).

**Step 5 — Cross-reference and detect gaps:**
1. For each requirement ID in the master list:
   - If it has NO source code tag → flag as WARNING (untraced to code).
   - If its `verification_method` is `"Test"` and it has NO test tag → flag as ERROR (missing test coverage).
   - If it has NO design tag → flag as WARNING (undesigned requirement).
2. Compute coverage percentages:
   - Code coverage: (requirements with at least one `@req` tag) / (total requirements) × 100
   - Test coverage: (testable requirements with at least one `@verify` tag) / (requirements with verification_method "Test") × 100
   - Design coverage: (requirements with at least one `@design` tag) / (total requirements) × 100

### Requirements Coverage Check

Verify that every requirement has appropriate implementation and verification coverage.

1. Load the master requirement list from Step 1 above.
2. For each requirement:
   - **Code tag present?** — Check the `@req` tag map from Step 2. A requirement without a code tag means code may exist but is untagged, or implementation is missing.
   - **Test tag present (if testable)?** — Check the `@verify` tag map from Step 3. Only requirements with `verification_method: "Test"` are required to have test tags.
   - **Design tag present?** — Check the `@design` tag map from Step 4.
3. Flag missing coverage per the severity rules:
   - Missing test tag for `verification_method: "Test"` → ERROR
   - Missing code tag → WARNING
   - Missing design tag → WARNING

### Link Integrity Check

Validate all `uses` and `implements` links resolve correctly and form a consistent hierarchy.

**Step 1 — Validate `uses` links:**
1. For each requirement that has a `"uses"` array:
   - For each referenced ID in the array, verify it exists in some module's `requirements.json`.
   - If the referenced ID does not exist → flag as ERROR (broken `uses` link).
2. `uses` points UP — the referenced requirement should be at a higher level (typically a SYS-level or parent module requirement).

**Step 2 — Validate `implements` links:**
1. For each requirement that has an `"implements"` array:
   - For each referenced ID in the array, verify it exists in some module's `requirements.json`.
   - If the referenced ID does not exist → flag as ERROR (broken `implements` link).
2. `implements` points DOWN — the referenced requirement should be at a more detailed level.

**Step 3 — Check bidirectional consistency:**
1. If requirement A lists B in its `"implements"` array, then B should list A in its `"uses"` array.
2. If requirement B lists A in its `"uses"` array, then A should list B in its `"implements"` array.
3. Flag mismatches as WARNING (inconsistent bidirectional links).

**Step 4 — Detect circular dependencies:**
1. Build a directed graph from all `uses` and `implements` relationships.
2. Perform a cycle detection (DFS with back-edge detection or topological sort).
3. If any cycle is found → flag as ERROR (circular dependency) and report the cycle path.

### Test Coverage Assessment

Verify that test files exercise adequate scenarios through proper DI boundaries.

**Step 1 — Check DI boundary usage:**
1. For each test file in scope, examine how test doubles are injected.
2. Acceptable patterns:
   - Custom vtable struct with test function pointers, assigned to the module's `ptApi` field.
   - Constructor injection — passing test dependencies through `Init` functions.
3. Unacceptable patterns (flag as ERROR):
   - `__attribute__((weak))` overrides of production functions.
   - `LD_PRELOAD` or dynamic linker tricks.
   - Direct modification of static/private members bypassing the API.

**Step 2 — Assess test scenario coverage:**
1. For each requirement with `verification_method: "Test"`, examine the corresponding test functions:
   - **Happy path**: At least one test calls the function with valid inputs and asserts the expected successful outcome. Look for `TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, ...)` or equivalent.
   - **Error paths**: At least one test exercises invalid inputs (NULL pointers, zero sizes, invalid states) and asserts the correct error status is returned. Look for `TEST_ASSERT_EQUAL(JUNO_STATUS_ERR_NULL_PTR, ...)` or similar error assertions.
   - **Boundary conditions**: At least one test exercises edge values (empty buffer, single element, max capacity, off-by-one indices) where applicable to the requirement.
2. Flag missing scenario types:
   - Missing happy path test → ERROR
   - Missing error path test → ERROR (for requirements whose described behavior includes error conditions)
   - Missing boundary test → WARNING (where boundary conditions are applicable)

**Step 3 — Assess test behavioral quality:**
Examine each test function for the following defects. These indicate tests that are optimized
to pass rather than to verify correct behavior.

1. **Status-only assertion (ERROR)**: The test asserts `JUNO_STATUS_SUCCESS` but makes no
   assertion on output values, state changes, buffer contents, or observable side-effects.
   A passing status alone does not prove the function did anything correct.

2. **Always-pass test (ERROR)**: The test would still pass if the function under test were
   replaced with an empty stub `{ return JUNO_STATUS_SUCCESS; }`. Look for tests that call
   a mutating function (push, write, encode, compute) and then never query the result.

3. **Tautological double (ERROR)**: The test double is configured to return a specific
   expected value, and the test then asserts on that very value — meaning the code under
   test adds no value to the chain. The code under test must be the one producing the
   asserted output, not the double.

4. **Vague error-path assertion (ERROR)**: Error-path test asserts
   `!= JUNO_STATUS_SUCCESS` instead of the exact expected error code. The exact error
   status is part of the behavioral contract and must be specified precisely.

5. **Ignored call count (WARNING)**: A test double captures a `call_count` but no
   assertion on that counter exists in the test. If call counts are tracked they must
   be verified.

6. **No output assertion (ERROR)**: A function writes to an output parameter or buffer,
   but the test never reads or asserts on the written value.

7. **No state-change assertion (ERROR)**: A mutating function (push, insert, enqueue,
   write) is called but no subsequent query (count, peek, read, get) asserts that the
   module's observable state changed correctly.

Flag each finding by test function name and line number. Apply severity as listed above.

### Automated Traceability Tool (MANDATORY)

Before performing manual cross-referencing, run the automated traceability verification tool:

```
python3 scripts/verify_traceability.py
```

Or for a specific module:

```
python3 scripts/verify_traceability.py --module MODULE_NAME
```

1. The tool exits with code 0 (PASS) or 1 (FAIL). A FAIL result means ERRORs exist — the verifier MUST include all ERROR items from the tool output in the verification report.

2. The tool checks: missing test coverage tags, orphaned tags, broken uses/implements links, bidirectional link inconsistencies, and schema validation of requirements.json files.

3. The tool output does NOT replace manual verification. After running the tool, the verifier must still perform the manual checks (especially test behavioral quality assessment, which the tool cannot do).

4. **Verdict impact**: If the tool exits with code 1, the verifier's verdict MUST be NEEDS CHANGES, regardless of manual findings.

## Severity Classification

| Severity | Meaning | Trigger |
|----------|---------|---------|
| **ERROR** | Blocking | Broken `uses`/`implements` links, orphaned tags, duplicate REQ IDs, missing test coverage for `Test` verification-method requirements, invalid JSON schema, circular dependencies, linker-level test doubles, missing happy-path tests, status-only assertions with no behavioral check, always-pass tests, tautological doubles, vague error-path assertions, missing output assertions, missing state-change assertions |
| **WARNING** | Non-blocking | Untraced requirements (no code tag), undesigned requirements (no design tag), inconsistent bidirectional links, missing boundary-condition tests, ignored test-double call counts |
| **INFO** | Informational | Coverage percentages, total counts, tag statistics |

## Verdict Criteria

- **APPROVED**: Zero ERRORs AND the `scripts/verify_traceability.py` tool exits with code 0 (no ERRORs). Warnings are acceptable if the Software Lead's acceptance criteria do not require their resolution.
- **NEEDS CHANGES**: Any ERROR is present. The report must list every ERROR with its file location, requirement ID, and specific issue so the responsible worker can fix it.

## Output Format

Produce a structured audit report:

```
## Traceability Audit Report

### Summary
- **Modules audited**: <list of module names>
- **Total requirements**: N
- **Traced to code**: N/N (XX%)
- **Traced to tests**: N/N (XX%) [of requirements with verification_method: Test]
- **Traced to design**: N/N (XX%)
- **Link integrity**: N uses links checked (N valid, N broken), N implements links checked (N valid, N broken)
- **Circular dependencies**: None detected / <cycle path>

### Errors
1. [ERROR] <file:line or REQ-ID> — <description>
2. [ERROR] ...

### Warnings
1. [WARNING] <file:line or REQ-ID> — <description>
2. [WARNING] ...

### Info
- Total code tags found: N across M files
- Total test tags found: N across M files
- Total design tags found: N across M files
- <additional observations>

### Verdict: APPROVED / NEEDS CHANGES
<brief justification referencing error count and critical findings>
```
