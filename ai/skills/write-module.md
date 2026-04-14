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

> **Software Lead**: See `ai/skills/software-lead.md` → Write Module for planning and verification steps.

### Software Developer Role

1. After Project manager approval, generate files:

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
   - Rationale from Project manager

   **`tests/test_<module>.c`**:
   - MIT License header
   - Test data type definition
   - Test double implementation (custom vtable)
   - Fixtures with `setUp`/`tearDown`
   - Test functions with `// @{"verify": ["REQ-MODULE-NNN"]}` tags
   - Section banner comments
   - `main()` with `RUN_TEST` calls

2. Submit all files to Software Lead for review.

## Constraints

- All files must follow conventions in `ai/memory/coding-standards.md`.
- Module must use the vtable/DI pattern from `ai/memory/architecture.md`.
- No dynamic allocation — ever.
- Rationale in requirements must come from the Project manager.
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
