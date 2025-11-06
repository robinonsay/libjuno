<img src="https://raw.githubusercontent.com/robinonsay/libjuno/b710f2f363f589a5da475543f53a22d3d030cc26/assets/juno_logo_rect.svg" alt="drawing" width="400em"/>

![GitHub Actions Workflow Status](https://github.com/robinonsay/libjuno/actions/workflows/ctest.yml/badge.svg)
![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)
![GitHub Tag](https://img.shields.io/github/v/tag/robinonsay/libjuno)

# LibJuno
* [LibJuno GitHub](https://github.com/robinonsay/libjuno)

LibJuno is a lightweight C11 embedded systems micro-framework. It's designed to
provide developers with common capabilities and interfaces that are utilized
commonly in embedded systems development.

* LibJuno is a lightweight C11 library designed specifically for embedded systems.
* LibJuno enables embedded systems developers to utilize dependency injection within 
C11 in a memory-safe manner
* LibJuno provides essential functionalities like memory management
* LibJuno optimizes for memory safety, determinism and efficiency in constrained environments.
* LibJuno supports freestanding builds (no hosted standard library) for maximum portability when enabled via `-DJUNO_FREESTANDING=ON`.

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

## Using LibJuno
* By default, LibJuno builds a static library (`libjuno.a`). To also build a shared library, pass `-DJUNO_SHARED=ON`.

### Documentation
* [Dependency Injection](include/juno/README.md)
* [Memory Module](include/juno/memory/README.md)

## Dependencies
* LibJuno aims to minimize dependencies, including dependencies on the C standard library
* Dependencies of LibJuno are listed here
   * This does not include dependencies on compilers or build scripting/tooling

| Dependency Name | Rationale                                      |
|-----------------|------------------------------------------------|
|    <math.h>     | Math is required for math lib                  |

## Building and Testing
1. Configure (static library by default):
   - `cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo`
2. Build:
   - `cmake --build build -j`
3. Run unit tests (optional):
   - `cmake -S . -B build -DJUNO_TESTS=ON -DCMAKE_BUILD_TYPE=Debug`
   - `cmake --build build -j`
   - `ctest --test-dir build --output-on-failure`

### CMake Build Option

* `-DJUNO_TESTS=On` (Default `Off`): Enable unit testing
* `-DJUNO_COVERAGE=On` (Default `Off`): Compile Juno with code coverage
* `-DJUNO_DOCS=On` (Default `Off`): Enable doxygen docs
* `-DJUNO_PIC=On` (Default `On`): Compile Juno with Position Independent Code
* `-DJUNO_SHARED=On` (Default `Off`): Compile the juno shared library
* `-DJUNO_FREESTANDING=On` (Default `Off`): Build in freestanding mode (adds `-ffreestanding -nostdlib` and avoids hosted libc)
* `-DJUNO_ASAN=On` (Default `Off`): Enable AddressSanitizer (host debugging)
* `-DJUNO_UBSAN=On` (Default `Off`): Enable UndefinedBehaviorSanitizer (host debugging)
* `-DJUNO_EXAMPLES=On` (Default `Off`): Build examples in `examples/`

#### Sanitizers and host debug builds (optional)

For a quick host review with sanitizers and tests enabled:

```sh
cmake -S . -B build -DJUNO_TESTS=ON -DJUNO_ASAN=ON -DJUNO_UBSAN=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
```
## Inspiration for the Name
Juno is the name of my wonderful dog and
she has brought me so much comfort and stability throughout the years.
I wanted to honor her legacy by naming an open-source library after her.

