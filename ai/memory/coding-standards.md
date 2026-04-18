# Coding Standards — LibJuno

## Language Standard

- **C11** (`-std=c11`), freestanding-compatible
- **C++11** (`-std=c++11`) for C++ wrapper headers and tests only
- No RTTI (`-fno-rtti`), no exceptions (`-fno-exceptions`) in C++ code

## Compiler Flags (enforced)

```
-Wall -Wextra -Werror -pedantic -Wshadow -Wcast-align -Wundef
-Wswitch -Wswitch-default -Wmissing-field-initializers
-fno-common -fno-strict-aliasing
```

All warnings are treated as errors (`-Werror`).

## Naming Conventions

| Element            | Convention                 | Example                          |
|--------------------|----------------------------|----------------------------------|
| Types / Structs    | `SCREAMING_SNAKE_CASE_T`   | `JUNO_DS_HEAP_ROOT_T`           |
| Struct tags        | `SCREAMING_SNAKE_CASE_TAG` | `JUNO_DS_HEAP_ROOT_TAG`         |
| Public functions   | `PascalCase` with prefix   | `JunoDs_Heap_Init`              |
| Static functions   | `PascalCase` (shorter)     | `Verify`, `Juno_MemoryBlkGet`   |
| Macros             | `SCREAMING_SNAKE_CASE`     | `JUNO_ASSERT_EXISTS`            |
| Private members    | Leading underscore          | `_pfcnFailureHandler`           |

## Hungarian Notation (Variables)

| Prefix  | Meaning               | Example          |
|---------|-----------------------|------------------|
| `t`     | Struct / type value   | `tStatus`        |
| `pt`    | Pointer to type       | `ptHeap`         |
| `z`     | `size_t`              | `zLength`        |
| `i`     | Index / integer       | `iIndex`         |
| `b`     | `bool`                | `bFlag`          |
| `pv`    | `void *`              | `pvMemory`       |
| `pc`    | `char *`              | `pcMessage`      |
| `pfcn`  | Function pointer      | `pfcnCompare`    |

## Error Handling

- All fallible functions return `JUNO_STATUS_T` (int32_t, 0 = success).
- Use `JUNO_MODULE_RESULT(NAME_T, OK_T)` for functions returning a value + status.
- Use `JUNO_MODULE_OPTION(NAME_T, SOME_T)` for optional return values.
- Use `JUNO_ASSERT_EXISTS`, `JUNO_ASSERT_SUCCESS`, `JUNO_ASSERT_OK`,
  `JUNO_ASSERT_SOME` macros for early-return error propagation.
- Failure handlers are diagnostic only — they never alter control flow.

## Documentation

- **Doxygen** comments on all public API elements:
  - Files: `@file`, `@brief`, `@details`, `@defgroup`
  - Functions: `@brief`, `@param`, `@return`, `@note`
  - Structs/members: `/** ... */` or `/// ...`
  - Groups: `@ingroup`, `@{`, `@}`
- Embed usage examples in `@details` using `@code{.c}` blocks.
- Include complexity notes and invariant descriptions where applicable.

## File Structure

- MIT License header at the top of every file (block comment).
- `#ifndef` / `#define` include guards using `JUNO_<PATH>_H` pattern.
- `#ifdef __cplusplus extern "C" {` wrappers in all public C headers.

## Forbidden Practices

- **No `malloc`, `calloc`, `realloc`, `free`** — zero dynamic allocation.
- **No heap-allocated memory** of any kind within the library.
- **No global mutable state** — all state is caller-owned and injected.
- **No `goto`** unless for structured cleanup in deeply nested error paths.
- **No platform-specific headers** in freestanding-compatible code.
- **No silent error swallowing** — all errors must propagate via status codes.
