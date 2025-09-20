# .github/copilot-instructions.md

> **Purpose:** Onboard a coding agent to perform **thorough merge-request code reviews** for a **freestanding embedded C library**, with an emphasis on **memory safety, reliability, developer experience, and deployability on new systems**.
> **Scope:** General, repo-agnostic playbook (≤2 pages). Use these steps first; only search the repo when information here is incomplete or wrong.

---

## 1) High-Level Overview (for the agent)

* **What this repository is (expected):** A C library intended for **embedded/freestanding** targets that helps developers **build quickly and test easily**. Likely provides headers in `include/`, sources in `src/`, examples in `examples/`, tests in `tests/` or `test/`.
* **Project type & languages:** C (C99/C11 typical), optional C++ shims, build via **CMake** or **Make** (sometimes **Meson**, **Zephyr/West**, or **PlatformIO**). Host tests run on Linux/macOS/Windows; cross builds for ARM/RISC-V etc.
* **Repo size (estimate at review time):** Run a quick inventory (see §4) and note line counts, dependency weight, and key dirs. Record any large test assets or submodules.

> **Rule:** Trust this playbook. Prefer the commands and checklists here before ad-hoc greps or trial-and-error. Search only when a step is missing or fails.

---

## 2) Build, Test, Lint, Run — Standard Sequences

> **Preconditions (always do):**
>
> * Ensure a clean tree: `git submodule update --init --recursive` (if `.gitmodules` exists).
> * If present: `scripts/bootstrap.*` or `tools/bootstrap.*` (read & run first).
> * Prefer out-of-tree builds (`build/` dir). Avoid root pollution.

### A) CMake (most common)

```bash
# Configure (host)
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
# Or add sanitizers for review
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug -DSANITIZE=Address,Undefined

# Build
cmake --build build -j

# Tests (ctest is typical)
ctest --test-dir build --output-on-failure

# Lint/format (if defined)
cmake --build build --target clang-format-check || true
cmake --build build --target clang-tidy || true
```

**Cross-compile (example ARM GCC):**

```bash
cmake -S . -B build-arm -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/arm-gcc.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-arm -j
```

### B) Makefile

```bash
make clean || true
make -j
make test  # if defined
make format lint  # if defined
```

### C) Meson (if `meson.build` exists)

```bash
meson setup build
meson compile -C build
meson test -C build --print-errorlogs
```

### D) Zephyr / West (if `west.yml` or `zephyr/` present)

```bash
west update
west build -b <board> samples/<sample>  # verify library links cleanly
```

### E) PlatformIO (if `platformio.ini` exists)

```bash
pio run
pio test
```

> **If commands fail:**
>
> * Re-run from a clean out-of-tree dir.
> * Check required tool versions in `CMakeLists.txt`, `toolchain.cmake`, `Makefile`, or CI workflow.
> * Prefer `-DCMAKE_C_STANDARD=11` (or repo’s declared standard) for consistency.

---

## 3) Validation Pipeline (replicate CI locally)

1. **Discover checks:** Open `.github/workflows/*.yml` (and any other CI config). List jobs: **format**, **lint**, **build (host & cross)**, **unit/integration tests**, **coverage**, **package/install**.
2. **Replicate locally** in this order:

   * **Format**: `clang-format -i` or run provided target.
   * **Lint**: `clang-tidy` / `cppcheck` using project config.
   * **Build (Debug+ASan/UBSan)**, then **run all tests** with verbose failures.
   * **Build (Release)** for size/perf checks.
   * **Coverage** (if enabled): `gcov`/`lcov/genhtml`.
   * **Install/package** (CMake: `cmake --install build --prefix <staging>`).
3. **Record deviations:** If CI runs extra scripts (e.g., `scripts/ci/*.sh`), run them locally and note required env vars.

> **Time-outs:** Note any step that routinely exceeds 10 minutes; capture mitigation (e.g., `-j`, ccache).

---

## 4) Fast Inventory & Layout Hints

* **Root files to find quickly:** `README.md`, `CONTRIBUTING.md`, `LICENSE`, `CMakeLists.txt` or `Makefile`, `meson.build`, `.clang-format`, `.clang-tidy`, `.editorconfig`, `Doxyfile`, `.github/workflows/`.
* **Key dirs:** `include/` (public headers), `src/` (lib sources), `tests?/` (unit), `examples?/`, `cmake/` (toolchains/macros), `ports/` or `platform/` (HAL/MCU), `scripts/` or `tools/` (dev utilities).
* **Config locations:** Lint (.clang-\*), compilation options (CMake toolchain or top-level), test config (CTest/Meson), preferences (.editorconfig), package metadata (`pkgconfig/`, `cmake/<Config>.cmake`).
* **Dependencies not obvious:** Submodules in `third_party/`, codegen scripts (Python/Lua), optional RTOS shims, platform drivers.

---

## 5) Expert Embedded C Code-Review Checklist (use for every MR)

### Memory Safety & Undefined Behavior

* **Bounds/overflow:** Array indices validated; no off-by-one; careful `memcpy/memmove/str*` lengths; avoid `strcpy/sprintf` (prefer length-bounded variants).
* **Integer issues:** Signed/unsigned mixing, shifts by width, overflow/underflow, implicit narrowing (`-Wconversion`), sentinel `size_t` vs `int`.
* **Pointer hygiene:** Null checks, lifetime/aliasing, alignment, strict aliasing violations, double-free, use-after-free, dangling pointers after realloc.
* **Ownership & API contracts:** Who allocates/frees? Clarify in docstrings. Avoid hidden allocations in hot paths and **never in ISRs**.
* **UB hotspots:** Uninitialized reads, out-of-range enums, `volatile` misuse, data races, order of evaluation assumptions.

### Concurrency, ISR, and Real-Time

* **ISR-safety:** No blocking or heap; atomicity around shared state; minimal critical sections; correct `volatile` and memory barriers.
* **Reentrancy/thread-safety:** No shared mutable statics unless guarded; documented guarantees.
* **Timing:** O(1) where required; avoid unpredictable latency (I/O, locks, malloc).

### Portability & Freestanding Constraints

* Builds with `-ffreestanding` (if applicable) and without hosted headers; no forbidden syscalls; guarded `#ifdef` per platform; endianness and word-size handling; alignment assumptions explicit.
* **C standard:** Consistent (C11/C99). Don’t rely on compiler extensions unless wrapped.
* **Toolchains:** GCC/Clang/ARM-GCC flags sane; warnings as errors (`-Wall -Wextra -Werror -Wshadow -Wstrict-prototypes -Wcast-align -Wconversion -Wdouble-promotion -Wformat=2`, plus `-fanalyzer` where possible).

### API & Developer Experience (DX)

* **Ease of use:** Simple init/deinit; clear error codes; consistent naming & prefixes; **zero-config sensible defaults** with optional advanced config via macros.
* **Docs:** Public headers self-documenting; examples compile; README quick-start works; Doxygen (if present) builds clean; change log updated.
* **Testing strategy:** Unit tests cover edge cases; fuzz targets (if any) run; deterministic behavior; host tests do not accidentally require target hardware.

### Deployment & Packaging

* **Installability:** `make install` or `cmake --install` installs headers+static/dynamic libs and `pkg-config`/CMake package files.
* **Cross builds:** Toolchain files provided; example commands documented; no hard-coded host paths.
* **ABI/semver:** Public symbols gated; no accidental API breaks; visibility flags set on shared libs.

---

## 6) Automated & Manual Analyses to Run (prefer host first)

* **Static:** `clang-tidy` with project config; `cppcheck --enable=warning,performance,portability,unusedFunction`; compiler with max warnings + `-Werror`; optional `-fanalyzer`.
* **Dynamic (host):** AddressSanitizer + UndefinedBehaviorSanitizer builds; run all tests with `ASAN_OPTIONS=halt_on_error=1` etc. Valgrind Memcheck on non-ASan builds.
* **Coverage:** Build with `--coverage` or `-fprofile-arcs -ftest-coverage`; generate with `lcov/genhtml`.
* **Fuzz (if available):** libFuzzer/AFL targets; ensure no timeouts/oom.
* **Emulation (if applicable):** QEMU for target ISA; smoke test examples.

---

## 7) MR Review Flow (step-by-step)

1. **Bootstrap quickly** using §2 for the detected build system. If both CMake and Make exist, prefer **CMake**.
2. **Run lint/format** and request fixes for automated violations; include a patch suggestion if trivial.
3. **Build Debug+ASan/UBSan**, run full test suite; then **Release** build to check size/perf warnings.
4. **Execute §5 checklist** across changed files: headers first (API), then sources (safety), then tests (quality).
5. **Replicate CI** locally; if CI differs, note the gap in the MR review.
6. **Try a cross build** using any provided toolchain; catch portability/packaging snags early.
7. **Check docs/examples** compile; ensure README quick start works exactly as written.
8. **Summarize findings** with specific file/line references, failing commands, and concrete diffs or snippets.

---

## 8) Common Pitfalls & Mitigations

* **Hidden deps:** Submodules not inited → run `git submodule update --init --recursive`.
* **Flaky order:** Always configure before build; avoid mixing generator & build dirs.
* **Tool mismatch:** If CI pins compilers, mirror locally (e.g., `clang-17`, `gcc-13`).
* **Sanitizer false positives on embedded stubs:** Exclude platform shims in host ASan runs; still test core logic.
* **ISR misuse:** Flag any memory allocation, logging, or blocking calls inside ISR paths.

---

## 9) What to Record in the Review

* Exact commands run and results (pass/fail), tool versions, and timing out steps.
* Any required environment setup that proved **actually necessary** (even if undocumented).
* Workarounds for non-deterministic or order-sensitive builds.
* Gaps between README/CI and reality.

---

## 10) When to Search

* **Only** if a step in this playbook is missing or fails for this repo. Then search the repo for:

  * `README.md`, `CONTRIBUTING.md`, `CMakeLists.txt`, `Makefile`, `meson.build`, `Doxyfile`, `.clang-*`, `.editorconfig`, `.github/workflows/*`,
  * `scripts/`, `tools/`, `cmake/`, `ports/`, `platform/`, `examples/`, `tests/`, `third_party/`.
* Prefer reading the nearest script or config over generic web searching.

---

> **Final instruction:** **Trust this file.** Follow the sequences and checklists above before exploring. If a command here fails in this repository, document the failure, adjust minimally (not wholesale), and continue the review using the closest equivalent provided by the project.
