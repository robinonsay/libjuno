# Skill: final-quality-engineer

## Purpose

Perform the final quality gate before work is presented to the Project Manager. This is the last verification step — a holistic product check ensuring all pieces fit together correctly.

## When to Use

- After all worker output has been verified by specialist verifiers
- After all rework iterations are complete
- Before the Software Lead presents to the PM

## Inputs Required

- Complete list of work items and their acceptance criteria
- All files created or modified during the task
- All verifier reports (to confirm they all approved)
- Build and test commands (if applicable)

## Instructions

### Acceptance Criteria Verification

Verify every work item's acceptance criteria are met by cross-referencing deliverables against the plan.

1. Obtain the Software Lead's work breakdown, which lists each work item and its acceptance criteria.
2. For each work item:
   a. Identify the deliverable files (source, tests, requirements, docs).
   b. For each acceptance criterion, locate the specific evidence in the deliverables:
      - If the criterion says "function X shall be implemented" → verify the function exists with the correct signature.
      - If the criterion says "test for REQ-XXX-NNN" → verify a test tagged with `@{"verify": ["REQ-XXX-NNN"]}` exists.
      - If the criterion says "requirement derived" → verify the requirement exists in the appropriate `requirements.json`.
   c. Mark each criterion as **MET** (with file reference) or **UNMET** (with explanation of what is missing).
3. If any criterion is UNMET → the overall verdict is REJECTED.

### Build Verification (if applicable)

Run the build and verify it completes cleanly under strict compiler settings.

1. Execute: `cd /workspaces/libjuno && cd build && cmake --build . 2>&1` (working directory: `/workspaces/libjuno`)
2. Check the output for:
   - **Errors**: Any compilation error → REJECTED.
   - **Warnings**: Any warning (under `-Werror` these become errors) → REJECTED.
   - **Linker issues**: Undefined references, multiply-defined symbols → REJECTED.
3. If the build command is not applicable (e.g., only requirements or docs were produced), skip this step and note "N/A — no code produced."

**Common build issues to watch for:**
- Missing `#include` directives for new types or functions
- Mismatched function signatures between `.h` and `.c` files
- Struct layout changes that break existing code
- New source files not added to `CMakeLists.txt`
- Macro redefinitions or conflicts between headers

### Test Suite Verification (if applicable)

Run the full test suite and verify zero failures.

1. Execute: `cd /workspaces/libjuno && cd build && ctest --output-on-failure` (working directory: `/workspaces/libjuno`)
2. Parse the output:
   - Record total tests, passed, failed, skipped.
   - For any failure, capture the test name and failure output.
3. If any test fails → REJECTED. Include the failing test name and output.
4. If no tests were modified or added, still run the full suite to check for regressions.

### VSCode Extension Test Verification (if applicable)

Run the VSCode extension test suite if extension code was modified.

1. Execute: `cd /workspaces/libjuno/vscode-extension && npm test` (working directory: `/workspaces/libjuno/vscode-extension`)
2. Parse the output:
   - Record total tests, passed, failed.
   - For any failure, capture the test name and failure output.
3. If any test fails → REJECTED. Include the failing test name and output.

**Common regression patterns to watch for:**
- A new type or struct change broke an existing test's assumptions about struct layout.
- A vtable signature change caused existing tests to pass wrong function pointers.
- A new header inclusion introduced a macro conflict affecting existing code.
- Buffer size constants changed, causing existing boundary tests to fail.
- Init function parameter changes broke existing test setup code.

### Traceability Verification (MANDATORY)

**Step 0 — Run automated traceability verification tool:**
1. Execute: `cd /workspaces/libjuno && python3 scripts/verify_traceability.py` (working directory: `/workspaces/libjuno`)
2. If exit code is 1 (FAIL) → the overall verdict is REJECTED. Include all ERROR lines in the findings.
3. If exit code is 0 (PASS) → proceed to manual spot-checks below.
4. The tool checks: tag coverage, orphaned references, link integrity, schema validity.
5. NOTE: The tool does NOT assess test behavioral quality — the final QE must verify that tagged tests actually exercise requirement behavior, not just return-status checks.

Spot-check that traceability annotations are present and correct.

1. Pick a representative sample of requirements from the scope (at least 3, or all if fewer than 5).
2. For each sampled requirement:
   a. Search source files for `@{"req": ["<REQ-ID>"]}` — verify at least one tag exists.
   b. If `verification_method` is `"Test"`, search test files for `@{"verify": ["<REQ-ID>"]}` — verify at least one tag exists.
   c. Search design docs for `@{"design": ["<REQ-ID>"]}` — verify at least one tag exists.
3. This is a spot-check, not a full audit. The Software Verification Engineer performs the comprehensive audit. Flag obvious gaps only.

### Cross-Item Consistency

Check that parallel work items do not conflict with each other.

1. If multiple files were created or modified:
   a. **Duplicate definitions**: Search for duplicate `typedef`, `struct`, function, or macro names across all new/modified files.
   b. **Incompatible vtable changes**: If a vtable (`_API_T`) struct was modified, verify all callers and test doubles match the new layout. Check that no other work item depends on the old vtable layout.
   c. **Conflicting requirement IDs**: Verify no two work items assigned the same REQ-ID to different requirements.
   d. **Broken cross-references**: If module A's code references module B's types, verify module B's headers export those types correctly.
   e. **Include guard conflicts**: Verify no two new headers use the same include guard macro.
2. If only a single work item was produced, note "Single work item — no cross-item conflicts possible."

**Common cross-item conflict patterns:**
- Two workers both add a requirement with the same ID (e.g., both use REQ-MODULE-005).
- One worker changes a vtable layout while another worker writes tests against the old layout.
- One worker adds a type name that collides with a type from another worker's module.
- One worker modifies a shared header that another worker's code depends on.

### Documentation Accuracy (if applicable)

Verify that produced documentation matches the actual code.

1. For each documentation file produced:
   a. **Function signatures**: Compare documented function signatures against the actual `.h` file declarations. Every parameter name, type, and return type must match exactly.
   b. **Struct layouts**: Compare documented struct members against actual definitions. Member names, types, and order must match.
   c. **Behavioral descriptions**: Verify that described behaviors (error handling, return values, preconditions) are consistent with the implementation.
   d. **Removed symbols**: Search for references to symbols that were renamed or removed — flag any stale references.
2. If no documentation was produced, skip this step and note "N/A — no docs produced."

## Verdict Criteria

- **APPROVED**: ALL checks pass. Zero blocking issues of any kind. Every acceptance criterion is MET. Build is clean. Tests pass. No regressions. No cross-item conflicts. `scripts/verify_traceability.py` exits with code 0.
- **REJECTED**: ANY check fails. Every blocking issue must be listed individually with:
  - File and line number (where applicable)
  - Severity (BLOCKING)
  - Clear description of the issue
  - Which acceptance criterion it violates (if applicable)

## Output Format

```
## Final Quality Assessment

- **Overall Verdict**: APPROVED / REJECTED
- **Acceptance Criteria**: X/Y met (list any unmet)
- **Test Suite**: X passed, Y failed, Z skipped
- **Build Status**: Clean / Warnings / Errors
- **Traceability**: Complete / Gaps (list gaps)
- **Cross-Item Consistency**: No conflicts / Issues (list issues)
- **Regressions**: None detected / Issues (list)
- **Key Observations**: <notable findings, commendations, or concerns>

### Detailed Findings
(If REJECTED, list each issue with file:line, severity, and description)

1. [BLOCKING] <file:line> — <description>
2. [BLOCKING] ...
```
