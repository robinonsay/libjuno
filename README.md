<img src="https://raw.githubusercontent.com/robinonsay/libjuno/b710f2f363f589a5da475543f53a22d3d030cc26/assets/juno_logo_rect.svg" alt="drawing" width="400em"/>

![GitHub Actions Workflow Status](https://github.com/robinonsay/libjuno/actions/workflows/ctest.yml/badge.svg)
![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)
![GitHub Tag](https://img.shields.io/github/v/tag/robinonsay/libjuno)

# LibJuno
* [LibJuno GitHub](https://github.com/robinonsay/libjuno)

LibJuno is a lightweight C11 embedded systems micro-framework designed to provide developers with common capabilities and interfaces commonly used in embedded systems development.

**Key Features:**
* **Dependency Injection in C11**: Enable modular, testable embedded software with explicit dependencies
* **Memory Safety**: Static memory allocation with type-safe pointer APIs - no dynamic allocation
* **Freestanding Support**: Can build without hosted C standard library (`-DJUNO_FREESTANDING=ON`)
* **Deterministic**: Predictable behavior and explicit error paths for real-time systems
* **Modular**: Use the whole library or cherry-pick individual components
* **Portable**: C11 standard with minimal platform assumptions

# The "Library of Everything" Problem
Many developers try to write the "Library of Everything". This is a library
that promises to solve all the problems that every developer has and does absolutely everything. It
claims to meet the needs of everyone and will lead to a glorious software future. It sounds great on paper
but is fundamentally flawed. Library developers have no idea what the user's requirements are and the
assumptions made about the user's requirements will be wrong for many users. It's impossible to write a library
that can meet every developer's needs. When library developers write the "Library of Everything" they often prescribe
developers to a specific architecture and run-time. A "Library of Everything" can be spotted when they dictate
a "central executive" and convoluted abstraction layers with specific implementations designed with a specific system
in mind. This pigeon holes them to the framework and does not scale when a developer has different requirements
than what the library assumed. 


LibJuno makes the assumption that it doesn't know how you're going to use it. It's designed to be
easy to change and adapt to your specific requirements. That's why LibJuno doesn't implement a run-time
and heavily utilizes dependency injection. Developers know their system better than anyone.
This library attempts to impower users with a set of tools they can choose to use
or leave on the table and create a solution that meets their requirements. LibJuno differs from other frameworks
because this library makes it easy for developers to make that choice.

# Core Philosophy
LibJuno prioritizes the following:
1. Memory Safety -- Memory needs to be accessed safely.
   This means **no dynamic memory allocation** and **no heap allocated memory** within this library.
2. Software Scalability -- Software should be maintainable as the codebase grows.
3. Shareability -- Small software components should be easy to share from one codebase to another.
4. Transparency -- Capabilities need to be transparent about the dependencies they have.

In order to implement this philosophy LibJuno heavily utilizes the Dependency Injection paradigm.
This enables these software systems to be scalable and easier to test. Additionally, LibJuno injects
memory use instead of allocating it. This enables developers to safely access their memory.

Finally, LibJuno aims to make few assumptions about developer's intended use-case. LibJuno understands
that developers and software architects are the experts of their system, not this library. The intent
is for this micro-framework to fit within developers software systems, not for a software system to conform
to this library. Developers can flexibly use the whole library or a single function with little project
overhead.

## Memory Safety, Pointers, and Arrays
At first, the LibJuno Pointer and Array systems may seem complex, or
boilerplate heavy. It's worth asking the question "Why not *just* use
`void *` and take a size?". The answer is memory safety. `void *`'s are
inherently unsafe. Many memory bugs and segfaults are a result of
complex pointer math and type erasure. LibJuno does not know your
type. It doesn't know what size, alignment, copy requirements your
type has. It doesn't know if you need a linked list, static array,
or some secret third data structure. Instead LibJuno says "Tell
me how to copy and reset your type and how to access your data". With
the answer to that question LibJuno can provide many tools at your disposal.
The alternative is either compromise on memory safety, or implement
the same queue function for every single type. The trade this library
makes is a little boilerplate for a lot of functionality. The nice
thing about LibJuno is that if you don't like the implementation,
you don't need to use it. You can always roll your own implementation
if that's what your project requires.

# Templates and Scripts
LibJuno provides many boilerplate generators in the `scripts` directory.
This makes it easy and effortless to generate structures and implementations
for arrays, pointers, apps, messages, and more.

# Tutorial

See [the LibJuno Tutorial](examples/example_project/LibJuno_Tutorial.md) for a tutorial
on how to use LibJuno. This is a complete toy-project that is used to explain and demonstrate
many concepts and core capabilities within LibJuno.

# Version and API Stability

**Current Version**: 1.0.0

LibJuno follows semantic versioning. While we strive for API stability and aim to minimize breaking changes, we prioritize correctness and safety. Future releases will follow semantic versioning practices:
- **Patch releases** (1.0.x): Bug fixes, no API changes
- **Minor releases** (1.x.0): New features, backward compatible
- **Major releases** (2.0.0): May include breaking changes when necessary

The public API includes all headers in `include/juno/`. Internal implementation details in `src/` are subject to change.

# Using LibJuno
* By default, LibJuno builds a static library (`libjuno.a`). To also build a shared library, pass `-DJUNO_SHARED=ON`.

## Documentation
* [Module System & Dependency Injection](include/juno/README.md)
* [Memory Module](include/juno/memory/README.md)
* [Examples Guide](examples/README.md)

## Dependencies
LibJuno is designed to minimize external dependencies for maximum portability.

**Core Library**: No external dependencies (can build freestanding with `-DJUNO_FREESTANDING=ON`)

**Testing Only**: Unity test framework (included in `deps/unity/`)

**Note**: Examples and tutorials may use standard library features (printf, etc.) for demonstration purposes, but the core library supports true freestanding operation where you provide your own I/O implementations.

## Building and Testing
1. Configure (static library by default):
   - `cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo`
2. Build:
   - `cmake --build build -j`
3. Run unit tests (optional):
   - `cmake -S . -B build -DJUNO_TESTS=ON -DCMAKE_BUILD_TYPE=Debug`
   - `cmake --build build -j`
   - `ctest --test-dir build --output-on-failure`

### CMake Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `JUNO_TESTS` | `OFF` | Enable unit testing with Unity framework |
| `JUNO_COVERAGE` | `OFF` | Compile with code coverage instrumentation |
| `JUNO_DOCS` | `OFF` | Generate Doxygen API documentation |
| `JUNO_PIC` | `ON` | Compile with Position Independent Code |
| `JUNO_SHARED` | `OFF` | Build shared library in addition to static |
| `JUNO_FREESTANDING` | `OFF` | **Freestanding mode**: Adds `-ffreestanding -nostdlib` flags for bare-metal targets |
| `JUNO_ASAN` | `OFF` | Enable AddressSanitizer (host debugging only) |
| `JUNO_UBSAN` | `OFF` | Enable UndefinedBehaviorSanitizer (host debugging only) |
| `JUNO_EXAMPLES` | `OFF` | Build example programs (requires hosted environment) |

#### Freestanding Mode

LibJuno is designed to support **true freestanding builds** for bare-metal and RTOS environments:

```sh
cmake -S . -B build -DJUNO_FREESTANDING=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j
```

In freestanding mode:
- No standard library headers are used (except freestanding ones like `<stdint.h>`, `<stdbool.h>`, `<stddef.h>`)
- Links with `-nostdlib`
- Core library APIs remain fully functional
- You provide platform-specific implementations (time, logging, I/O) via dependency injection

**Note**: Tests and examples require a hosted environment and cannot be built in freestanding mode.

#### Development and Testing Workflow

For comprehensive testing with sanitizers:

```sh
# Configure with tests and sanitizers
cmake -S . -B build -DJUNO_TESTS=ON -DJUNO_ASAN=ON -DJUNO_UBSAN=ON -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build -j

# Run tests
ctest --test-dir build --output-on-failure
```

For code coverage analysis:

```sh
cmake -S . -B build -DJUNO_TESTS=ON -DJUNO_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build
# Generate coverage report with lcov/genhtml
```

## Installation

Install headers and library to a system or staging prefix:

```sh
cmake --install build --prefix /path/to/install
```

For CMake-based projects, LibJuno provides package configuration files:

```cmake
find_package(juno REQUIRED)
target_link_libraries(your_target PRIVATE juno::juno)
```

## Contributing

Contributions are welcome! Please:
1. Ensure all tests pass with sanitizers enabled
2. Verify freestanding compatibility for core library changes
3. Update documentation for API changes
4. Follow existing code style and conventions

## Inspiration for the Name
Juno is the name of my wonderful dog and she has brought me so much comfort and stability throughout the years. I wanted to honor her legacy by naming an open-source library after her.

