# LibJuno — Copilot review playbook (repo-specific)

Purpose: Enable thorough MR reviews for LibJuno, a lightweight C11 library for embedded/freestanding targets, with emphasis on memory safety, determinism, developer experience, and deployability.

Trust this file first. Search the repo only when a step is unclear or fails.

---

## 1) What LibJuno is (and is not)

- Lightweight C11 library designed for embedded systems; can build freestanding (no libc) and emphasizes memory safety and determinism.
- Core themes:
  - A small module system with dependency-injection (DI) via API vtables and macros (see `include/juno/README.md`).
  - Deterministic memory utilities: block-based allocator with reference counting (no malloc required) and helpers (see `include/juno/memory/README.md`).
  - Utility algorithms and containers (CRC variants, buffers/queues/stacks, maps, time, etc.).
- Tests use the Unity framework vendored in `deps/unity/`.

---

## 2) Build, test, docs — how to run here

Preconditions
- Use out-of-tree builds (`build/`). Initialize submodules if added in future.
- C standard is enforced as C11; warnings are enabled and treated as errors.

Common CMake flows
- Configure (static lib by default):
  - Debug + tests: `cmake -S . -B build -DJUNO_TESTS=ON -DCMAKE_BUILD_TYPE=Debug`
  - Release: `cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo`
- Sanitizers (host review): add any of
  - `-DJUNO_ASAN=ON`, `-DJUNO_UBSAN=ON`
- Freestanding (no hosted libc):
  - `-DJUNO_FREESTANDING=ON` (adds `-ffreestanding -nostdlib`; ensure no hosted headers in changes)
- Coverage (host):
  - `-DJUNO_COVERAGE=ON` (links `gcov`, enables coverage flags)
- Examples: `-DJUNO_EXAMPLES=ON` to build samples in `examples/`.
- Shared library: `-DJUNO_SHARED=ON` to also build a shared artifact.

Build and test
- Build: `cmake --build build -j`
- Run tests (when `JUNO_TESTS=ON`):
  - `ctest --test-dir build --output-on-failure`
  - Tests are auto-discovered from `tests/` and link `unity` and `m`.

Docs
- Doxygen: configure with `-DJUNO_DOCS=ON`, then `cmake --build build --target docs`.

Install/package
- Install headers and libs: `cmake --install build --prefix <staging>`
- CMake package export: `cmake/junoConfig.cmake.in` is provided; verify `find_package(juno)` consumers as part of packaging checks.

---

## 3) Directory map (quick mental model)

- `include/juno/` — public headers (module system, memory, status, APIs). See `include/juno/README.md` and `include/juno/memory/README.md`.
- `src/` — implementation: CRCs (`juno_ccitt*.c`, `juno_kermit.c`, `juno_zip.c`), buffers/queues/stacks, memory block/heap, maps, time, etc.
- `tests/` — unit tests compiled when `JUNO_TESTS=ON`; `deps/unity` vendored test framework.
- `examples/` — minimal usage and DI demos; gated by `JUNO_EXAMPLES`.
- `docs/` + `Doxyfile.in` — documentation assets and template.
- `cmake/` — packaging config.
- `scripts/` — helper generators (templates, docs, versioning).

---

## 4) Validation pipeline for this repo

Run locally in this order and record results:
1) Configure + build (Debug) with sanitizers (when host review):
   - `-DJUNO_TESTS=ON -DJUNO_ASAN=ON -DJUNO_UBSAN=ON -DCMAKE_BUILD_TYPE=Debug`
2) Run all tests via `ctest --output-on-failure`.
3) Build Release (`RelWithDebInfo`) and ensure no warnings (warnings are `-Werror`).
4) Optional: Coverage (`-DJUNO_COVERAGE=ON`) and generate reports (`lcov/genhtml`).
5) Docs build (`-DJUNO_DOCS=ON`, target `docs`).
6) Install/package sanity: `cmake --install` into a staging prefix; smoke check `junoConfig.cmake` and includes.

Notes
- Compilers: GCC/Clang expected. Project sets strict warnings (e.g., `-Wall -Wextra -Werror -Wshadow -Wcast-align -Wundef -Wswitch -Wmissing-field-initializers -fno-common -fno-strict-aliasing`).
- For freestanding builds, ensure added code avoids hosted headers and syscalls.

---

## 5) LibJuno-focused code review checklist

Memory safety & UB
- Block allocator: verify bounds, reuse of freed blocks, and reference counting semantics. Ensure:
  - `JunoMemory_BlockInit` parameters are validated; type sizes match usage.
  - `Get/Put/Update` respect element size and block limits; reject double-free; `Put` fails when refs > 1.
  - All return codes (`JUNO_STATUS_T`) are checked at call sites.
- Avoid libc when `JUNO_FREESTANDING=ON`; no hidden dynamic allocation in hot paths.
- Integer and pointer hygiene: no narrowing, no invalid shifts, alignments explicit; observe strict aliasing rules.

Module system & DI (see `include/juno/README.md`)
- Every module sets `ptApi` to a valid vtable during init; failure handler (`JUNO_FAILURE_HANDLER`) and user data wired.
- Derived types overlay base via `JUNO_MODULE/JUNO_MODULE_DERIVE`; ensure member access uses the correct view and alignment.
- API contracts are clear: inputs validated, null checks present, and failure paths call the failure handler with actionable messages.

Concurrency/real-time
- No blocking I/O or heap in ISR-sensitive paths; minimal critical sections; volatile and barriers when required by shared state.

Portability & build flags
- Code builds clean with the project’s warning set; no reliance on compiler-specific extensions unless guarded.
- When `JUNO_SHARED=ON`, confirm symbol visibility and no accidental ABI leaks from private headers.

Tests & examples
- Unit tests cover edge cases for allocators (empty/full pool, refcounting, invalid free), containers (under/overflow), and CRC correctness against vectors.
- Examples compile with `JUNO_EXAMPLES=ON` and match documented usage in READMEs.

Docs & DX
- Public headers are self-documenting; Doxygen groups render without warnings.
- Namespaces/prefixes consistent (`Juno*`, `JUNO_*`); error codes and macro usage are discoverable from headers.

---

## 6) Common pitfalls seen in this repo

- Freestanding mode requires avoiding hosted headers even in helpers/tests; gate usage with `#if !JUNO_FREESTANDING` where necessary.
- Tests link `m` (math); ensure new tests include only headers available on host.
- Watch for off-by-one in buffer/queue/stack implementations and mixing of signed/unsigned indices.
- When updating install/package, validate `cmake/junoConfig.cmake.in` and the exported targets; run a small consumer CMake to `find_package(juno)`.

---

## 7) What to record in MR reviews

- Exact configure/build/test commands and outcomes (pass/fail), compiler versions, sanitizer results.
- Any deviations from README or CI, including steps required to build docs or examples.
- Specific file/line issues with concrete suggestions or diffs (headers first, then sources, then tests).
- Packaging/install verification notes (what installed, include paths, and find_package smoke test results).

---

## 8) When to search the repo

- If something is unclear or fails, consult: `README.md`, `include/juno/README.md`, `include/juno/memory/README.md`, top-level `CMakeLists.txt`, `tests/CMakeLists.txt`, `cmake/junoConfig.cmake.in`, `docs/`, `examples/`, `scripts/`.

Final instruction: Prefer these sequences and checklists for LibJuno. If a command fails, document the failure, adjust minimally, and continue the review using the closest equivalent in this repository.
