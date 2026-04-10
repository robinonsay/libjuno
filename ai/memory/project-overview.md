# Project Overview — LibJuno

## Summary

LibJuno is a lightweight C11 embedded systems micro-framework that provides common
capabilities and interfaces for embedded systems development. It is designed to be
freestanding-compatible, use zero dynamic memory allocation, and support dependency
injection as its core architectural paradigm.

## Repository

- **Name**: libjuno
- **Owner**: robinonsay
- **License**: MIT
- **Version**: 1.0.1

## Key Characteristics

| Property               | Value                                          |
|------------------------|------------------------------------------------|
| Language               | C11 (with C++11 wrappers for some components)  |
| Build System           | CMake (≥ 3.10)                                 |
| Test Framework         | Unity (ThrowTheSwitch)                         |
| Documentation          | Doxygen with doxygen-awesome-css theme          |
| Memory Model           | No dynamic allocation; all memory caller-owned |
| Freestanding Support   | Yes (`-DJUNO_FREESTANDING=ON`)                 |
| Library Type           | Static (default), optional shared              |

## Core Philosophy

1. **Memory Safety** — No `malloc`/`free`. No heap-allocated memory. All memory is
   injected by the caller.
2. **Software Scalability** — Maintainable as the codebase grows via modular design.
3. **Shareability** — Small components are easy to extract and reuse across codebases.
4. **Transparency** — Every capability is explicit about its dependencies.

## Module Categories

| Directory       | Purpose                                                  |
|-----------------|----------------------------------------------------------|
| `ds/`           | Data structures (heap, queue, stack, map, array, buffer) |
| `memory/`       | Memory management (pointer API, block allocator)         |
| `crc/`          | CRC algorithms (CCITT, CCITT-32, Kermit)                 |
| `math/`         | Math utilities                                           |
| `time/`         | Time abstractions                                        |
| `io/`           | I/O abstractions                                         |
| `sm/`           | State machine                                            |
| `sb/`           | String builder                                           |
| `sch/`          | Scheduler                                                |
| `mp/`           | Message passing                                          |
| `app/`          | Application framework                                    |
| `log/`          | Logging                                                  |
| `hash/`         | Hashing utilities                                        |

## Methodology

- **Agile** — Code and tests are the single source of truth.
- **Traceability** — Convention-based, using `requirements.json` files and
  inline code/test annotations.
- **Documentation** — Generated from code via Doxygen (API) and a custom Python
  tool (SRS, SDD, RTM in AsciiDoc/HTML/PDF).
