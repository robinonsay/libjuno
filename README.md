<img src="assets/juno_logo_rect.svg" alt="drawing" width="400em"/>

![GitHub Actions Workflow Status](https://github.com/robinonsay/libjuno/actions/workflows/ctest.yml/badge.svg)
![License: Apache 2.0](https://img.shields.io/badge/License-Apache%202.0-blue.svg)

# LibJuno
* LibJuno is a lightweight C99 library designed specifically for embedded systems.
* LibJuno focuses on providing essential functionalities like memory management, data structures, string operations and more all without dynamic memory allocation!
* LibJuno optimizes for memory safety, determinism and efficiency in constrained environments.
* LibJuno is compiled without the standard library to maximize portability.

## Using LibJuno
* By default, LibJuno will compile both a shared and static library

## Dependencies
* LibJuno is aims to minimze dependencies, including dependencies on the C standard library
* Dependencies of LibJuno are listed here
   * This does not include dependencies on compilers or build scripting/tooling

| Dependency Name | Rationale                                      |
|-----------------|------------------------------------------------|
| `<string.h>`    | `string.h` is used for `memset` and `memcpy`.  |

## Building and Testing
1. Create and navigate to a build directory:
   - mkdir build && cd build
2. Generate build files:
   - cmake ..
3. Compile the project:
   - make
4. Run unit-tests:
   - ctest

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
