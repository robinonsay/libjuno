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

# Core Philosophy
LibJuno prioritizes the following:
1. Memory Safety -- Memory needs to be accessed safely.
This means **no dynamic memory allocation** and **no heap allocated memory** within this library.
2. Software Scalability -- Software should be maintainable as the codebase grows
3. Shareability -- Small software components should be easy to share from one codebase to another
4. Transparency -- Capabilities need to be transparent about the dependencies they have

In order to implement this philosophy LibJuno heavily utilizes the Dependency Injection paradigm.
This enables these software systems to be scalable and easier to test. Additionally, LibJuno injects
memory use instead of allocating it. This enables developers to safely access their memory.

Finally, LibJuno aims to make few assumptions about developer's intended use-case. LibJuno understands
that developers and software architects are the experts of their system, not this library. The intent
is for this micro-framework to fit within developers software systems, not for a software system to conform
to this library. If you can tolerate function pointer use then this library is right for you.


# Tutorial

See [the LibJuno Tutorial](examples/example_project/LibJuno\ Tutorial.md) for a tutorial
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

## Current Modules
- **Memory**: Block-based allocator with typed pointer API (no malloc required). See `include/juno/memory/` and `src/juno_memory_block.c`.
- **CRCs**: CCITT, CCITT32, Kermit, ZIP. See `include/juno/crc/crc.h` and `src/juno_ccitt*.c`, `src/juno_kermit.c`, `src/juno_zip.c`.
- **Data Structures**: Buffers, queues, stacks, heap, map. See `include/juno/ds/*` and corresponding sources in `src/`.
- **Time Utilities**: Basic time helpers. See `include/juno/time/time_api.h` and `src/juno_time.c`.
- **BinHex/ARC helpers**: Utility encoders/decoders. See `src/juno_binhex.c`, `src/juno_arc.c`.

## Future Modules
- Filesystem interactions
- Networking support
- Additional utility libraries for embedded applications

## Inspiration for the Name
Juno is the name of my wonderful dog and
she has brought me so much comfort and stability throughout the years.
I wanted to honor her legacy by naming an open-source library after her.

