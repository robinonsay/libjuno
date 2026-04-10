# Constraints — LibJuno

## Hard Technical Constraints

### Memory

- **ZERO dynamic memory allocation** — no `malloc`, `calloc`, `realloc`, `free`.
- **ZERO heap-allocated memory** within the library.
- All memory is **caller-owned and injected** via init functions.
- Memory pools (block allocators) operate on caller-provided buffers.

### Language & Portability

- **C11 standard** — all code must compile with `-std=c11 -pedantic`.
- **Freestanding-compatible** — core library must compile with
  `-nostdlib -ffreestanding` (no hosted C standard library dependency).
- No platform-specific headers or system calls in library code.
- C++ wrappers must use C++11 with `-fno-rtti -fno-exceptions`.

### Compiler Strictness

- **All warnings are errors** (`-Werror`).
- Full warning set enabled: `-Wall -Wextra -pedantic -Wshadow -Wcast-align
  -Wundef -Wswitch -Wswitch-default -Wmissing-field-initializers`.
- `-fno-common -fno-strict-aliasing` enforced.

### Determinism

- All operations must have **predictable, deterministic behavior**.
- No floating-point in core paths unless explicitly documented.
- Error paths must be explicit — no silent failures, no undefined behavior.
- Failure handlers are diagnostic-only and never alter control flow.

## Architectural Constraints

- Every public API function must **verify its preconditions** at entry
  (NULL checks, dependency validation).
- All modules must use the **module root / derivation / vtable** pattern.
- Dependencies must be **injected**, never globally referenced.
- No global mutable state.

## Traceability Constraints

- Every module must have a `requirements.json` in `requirements/<module>/`.
- Source code implementing requirements must be tagged:
  `// @{"req": ["REQ-MODULE-001"]}`
- Test functions verifying requirements must be tagged:
  `// @{"verify": ["REQ-MODULE-001"]}`
- Tags must be at the **function level** (above the function definition).
- Requirements use a **two-tier hierarchy**: high-level → detailed.
- Relationships: `"uses"` points up (to parent), `"implements"` points down (to child).
- Verification methods: `Test`, `Inspection`, `Analysis`, `Demonstration`.

## Build Constraints

- CMake ≥ 3.10.
- Must support both hosted and freestanding builds from the same source.
- Static library by default; shared library optional (`-DJUNO_SHARED=ON`).
- Sanitizer support: ASAN (`-DJUNO_ASAN=ON`), UBSAN (`-DJUNO_UBSAN=ON`).
- Code coverage support via `--coverage` (`-DJUNO_COVERAGE=ON`).
