<img src="assets/juno_logo_rect.svg" alt="drawing" width="400em"/>
[![GitHub Actions Workflow Status](https://github.com/robinonsay/libjuno/actions/workflows/ci.yml/badge.svg)](https://github.com/robinonsay/libjuno/actions/workflows/ci.yml)
[![License: Apache 2.0](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
# LibJuno
The Juno library aims to make C99 development faster and more memory safe

## Summary
LibJuno is a lightweight C standard library designed specifically for embedded systems. It focuses on providing essential functionalities like memory management and string operations while ensuring safety and efficiency in constrained environments.

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

