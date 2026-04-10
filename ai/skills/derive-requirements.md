# Skill: derive-requirements

## Purpose

Analyze existing source code and test files to derive requirements, generate or
update `requirements.json` files, and add traceability annotations (`@{"req": ...}`
and `@{"verify": ...}`) to source and test code.

## When to Use

- Bootstrapping requirements for a module that has code but no `requirements.json`
- Updating requirements after code changes
- Adding missing traceability tags to existing source/test functions

## Inputs Required

- **Module name** (e.g., `heap`, `memory`, `crc`)
- **Scope** (optional): specific files or the entire module

## Instructions

### Coach Role

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
4. **Present the proposed requirements to the Program (user) for review.**
5. Ask the Program for **rationale** for each requirement — do NOT invent rationale.
6. Ask the Program to confirm or modify the hierarchy (`uses`/`implements` links).

### Player Role

7. After Program approval, generate/update `requirements/<module>/requirements.json`.
8. Add `// @{"req": ["REQ-MODULE-NNN"]}` tags above implementing functions in source.
9. Add `// @{"verify": ["REQ-MODULE-NNN"]}` tags above test functions.
10. Submit all changes to Coach for review.

### Coach Verification

11. Verify every requirement has at least one code tag.
12. Verify every requirement with `verification_method: "Test"` has at least one
    test tag.
13. Verify all `uses`/`implements` links are valid.
14. **Present final output to Program for approval.**

## Constraints

- Never invent rationale — always ask the Program.
- Never assume a requirement exists unless it is directly evidenced by code behavior
  or test assertions.
- Follow the JSON schema defined in `ai/memory/traceability.md`.
- Follow all naming conventions in `ai/memory/coding-standards.md`.

## Output Format

- `requirements/<module>/requirements.json` — new or updated
- Modified source files with `@{"req": ...}` tags
- Modified test files with `@{"verify": ...}` tags
- Summary table of derived requirements with traceability status

## Example Invocation

> Use skill: derive-requirements
> Module: heap
