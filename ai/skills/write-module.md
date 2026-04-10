# Skill: write-module

## Purpose

Scaffold a new LibJuno module with all required files following project
conventions: header, source, requirements, and test file — complete with
vtable pattern, Doxygen documentation, and traceability annotations.

## When to Use

- Creating a brand new module from scratch
- Adding a new data structure or capability to LibJuno

## Inputs Required

- **Module name** (e.g., `ringbuffer`, `bitset`)
- **Subsystem** (e.g., `ds`, `memory`, `crc`, `io`, or new)
- **Feature description**: what the module should do
- **Dependencies**: what existing modules it depends on (e.g., array API, pointer API)

## Instructions

### Coach Role

1. Read existing modules in the same subsystem to understand patterns:
   - Header structure and Doxygen group organization
   - Init function signature pattern
   - Vtable (API struct) layout
   - Source file structure
   - Test file structure
2. Read `ai/memory/architecture.md` for the module system pattern.
3. Ask the Program (ONE question at a time):
   - What operations should this module support?
   - What data does it manage?
   - What are the injected dependencies?
   - What error conditions should be handled?
   - What is the design rationale?
4. Draft a module plan:
   - Type definitions (root, derivation, API, result/option types)
   - Public API functions
   - Initial requirements
5. **Present the plan to the Program for review.**

### Player Role

6. After Program approval, generate files:

   **`include/juno/<subsystem>/<module>_api.h`**:
   - MIT License header
   - Include guards
   - `extern "C"` wrapper
   - Doxygen `@file`, `@brief`, `@details`, `@defgroup`
   - Forward declarations
   - Type definitions (root, derivation, API struct, module union)
   - Public function declarations with full Doxygen
   - Inline `Verify` function

   **`src/juno_<module>.c`**:
   - MIT License header
   - Includes
   - Static vtable instance
   - Static helper functions
   - Public function implementations
   - `// @{"req": ["REQ-MODULE-NNN"]}` tags on all functions

   **`requirements/<module>/requirements.json`**:
   - Following the JSON schema from `ai/memory/traceability.md`
   - Rationale from Program

   **`tests/test_<module>.c`**:
   - MIT License header
   - Test data type definition
   - Test double implementation (custom vtable)
   - Fixtures with `setUp`/`tearDown`
   - Test functions with `// @{"verify": ["REQ-MODULE-NNN"]}` tags
   - Section banner comments
   - `main()` with `RUN_TEST` calls

7. Submit all files to Coach for review.

### Coach Verification

8. Verify header follows the module root / derivation / vtable pattern.
9. Verify source wires vtable correctly and calls Verify at entry.
10. Verify all naming conventions (Hungarian notation, PascalCase, etc.).
11. Verify Doxygen is complete on all public elements.
12. Verify requirements.json is valid against schema.
13. Verify test doubles use vtable injection.
14. Verify all traceability tags are present and valid.
15. Verify no dynamic allocation.
16. **Present final output to Program for approval.**

## Constraints

- All files must follow conventions in `ai/memory/coding-standards.md`.
- Module must use the vtable/DI pattern from `ai/memory/architecture.md`.
- No dynamic allocation — ever.
- Rationale in requirements must come from the Program.
- Test doubles must use vtable injection, not linker mocking.
- The module should be added to the CMake build (update `src/` source list
  if not using `aux_source_directory`).

## Output Format

- `include/juno/<subsystem>/<module>_api.h`
- `src/juno_<module>.c`
- `requirements/<module>/requirements.json`
- `tests/test_<module>.c`
- Summary of types, functions, and requirements created

## Example Invocation

> Use skill: write-module
> Module: ringbuffer
> Subsystem: ds
> Description: Fixed-size circular buffer for streaming data
> Dependencies: array API, pointer API
