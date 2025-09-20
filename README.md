<img src="https://raw.githubusercontent.com/robinonsay/libjuno/b710f2f363f589a5da475543f53a22d3d030cc26/assets/juno_logo_rect.svg" alt="drawing" width="400em"/>

![GitHub Actions Workflow Status](https://github.com/robinonsay/libjuno/actions/workflows/ctest.yml/badge.svg)
![License: MIT](https://img.shields.io/badge/License-MIT%202.0-blue.svg)
![GitHub Tag](https://img.shields.io/github/v/tag/robinonsay/libjuno)

# LibJuno
* [LibJuno GitHub](https://github.com/robinonsay/libjuno)
* LibJuno is a lightweight C99 library designed specifically for embedded systems.
* LibJuno enables embedded systems developers to utilize dependency injection within
  C99 in a memory-safe manner
* LibJuno provides essential functionalities like memory management and more all without dynamic memory allocation!
* LibJuno optimizes for memory safety, determinism and efficiency in constrained environments.
* LibJuno is compiled without the standard library to maximize portability.

## Using LibJuno
* By default, LibJuno will compile both a shared and static library

### Documentation
* [Dependency Injection](include/juno/README.md)
* [Memory Module](include/juno/memory/README.md)

## Dependencies
* LibJuno is aims to minimze dependencies, including dependencies on the C standard library
* Dependencies of LibJuno are listed here
   * This does not include dependencies on compilers or build scripting/tooling

| Dependency Name | Rationale                                      |
|-----------------|------------------------------------------------|
|    <math.h>     | Math is required for math lib                  |

## Building and Testing
1. Generate build files:
   - `cmake -B build .`
3. Compile the project:
   - `make -C build`
4. Run unit-tests:
   - `cmake -B build . -DJUNO_TESTS=ON`
   - `make -C build`
   - `cd build`
   - `ctest`

### CMake Build Option

* `-DJUNO_TESTS=On` (Default `Off`): Enable unit testing
* `-DJUNO_COVERAGE=On` (Default `Off`): Compile Juno with code coverage
* `-DJUNO_DOCS=On` (Default `Off`): Enable doxygen docs
* `-DJUNO_PIC=On` (Default `On`): Compile Juno with Position Independent Code
* `-DJUNO_SHARED=On` (Default `Off`): Compile the juno shared library

## Current Modules
- **Memory Management**: Provides block-based allocation, deallocation, and memory tracking.
- **String Operations**: Handles string initialization, manipulation, concatenation, and cleanup.

## Future Modules
- Filesystem interactions
- Networking support
- Additional utility libraries for embedded applications

## Inspiration for the Name
Juno is the name of my wonderful dog and
she has brought me so much comfort and stability throughout the years.
I wanted to honor her legacy by naming an open-source library after her.

