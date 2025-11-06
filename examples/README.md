# LibJuno Examples

This directory contains minimal, self-contained examples demonstrating LibJuno's core capabilities.

## Quick Start

Build all examples with:
```sh
cmake -S .. -B ../build -DJUNO_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build ../build -j
```

Executables will be in `../build/examples/`.

## Available Examples

### 1. Minimal Memory Example (`minimal_memory_example.c`)

**What it demonstrates**: Block-based memory allocator with type-safe pointer API

**Key concepts**:
- Defining a custom type (`USER_DATA_T`)
- Implementing `JUNO_POINTER_API_T` (Copy and Reset functions)
- Initializing a block allocator with static memory
- Allocating, using, and freeing memory blocks
- Type safety through pointer verification

**Learn**: How LibJuno achieves memory safety without dynamic allocation

**Run**:
```sh
../build/examples/minimal_memory_example
```

---

### 2. Minimal Queue Example (`minimal_queue_example.c`)

**What it demonstrates**: Type-safe queue implementation using array and pointer APIs

**Key concepts**:
- Implementing `JUNO_POINTER_API_T` for a custom type
- Implementing `JUNO_DS_ARRAY_API_T` to provide indexed access
- Using the queue API (Push, Pop, Peek)
- Module derivation with `JUNO_MODULE_DERIVE`
- How data structures compose with pointer APIs

**Learn**: How LibJuno data structures work with your types

**Documentation**: See `MINIMAL_QUEUE_EXAMPLE.md` for detailed walkthrough

**Run**:
```sh
../build/examples/minimal_queue_example
```

---

### 3. State Machine Example (`example_state_machine.c`)

**What it demonstrates**: State machine implementation using LibJuno's SM API

**Key concepts**:
- Defining states with `JUNO_MODULE_DERIVE`
- Implementing state actions and transitions
- Using the state machine API
- Module unions with multiple state types

**Learn**: How to implement state machines in LibJuno

**Use case**: Traffic light controller simulation

**Run**:
```sh
../build/examples/example_state_machine
```

---

### 4. Complete Tutorial Project (`example_project/`)

**What it demonstrates**: Full application using multiple LibJuno subsystems

**Key concepts**:
- Logging API implementation and injection
- Time API implementation
- Memory management in a real application
- Dependency injection patterns
- Module composition

**Learn**: How to structure a complete embedded application with LibJuno

**Documentation**: See `example_project/LibJuno_Tutorial.md` for complete walkthrough

**Build**:
```sh
cd example_project
# See tutorial for build instructions
```

---

## Example Selection Guide

| Goal | Start Here |
|------|-----------|
| Learn memory management | `minimal_memory_example.c` |
| Learn data structures | `minimal_queue_example.c` |
| Learn state machines | `example_state_machine.c` |
| Build a complete app | `example_project/` |
| Understand DI patterns | Tutorial + any example |

## Using Examples as Templates

Examples are designed to be:
- **Copy-paste friendly**: Take code snippets directly into your project
- **Minimal**: Only demonstrate the specific feature
- **Self-contained**: No hidden dependencies or setup

### Code Generation Scripts

For production code, use the generators in `../scripts/`:
- `create_array.py`: Generate array and pointer API boilerplate
- `create_impl.py`: Generate module implementations
- `create_app.py`: Generate application skeleton
- `create_msg.py`: Generate message types

## Example Code Style

All examples follow these patterns:
1. **Forward declarations** of APIs to allow static initialization
2. **Implementation functions** with verification and error handling
3. **API vtables** as static const structures
4. **Initialization** showing dependency injection
5. **Usage** demonstrating the API

## Notes

- Examples may use **hosted features** (printf, etc.) for clarity
- Core library supports **freestanding builds** - examples are for learning
- For production embedded code, replace stdio/stdlib with your platform's APIs
- See `../include/juno/README.md` for module system architecture

## Additional Resources

- [Main README](../README.md): Build instructions and CMake options
- [Module System Guide](../include/juno/README.md): Architecture and DI concepts
- [Memory Module Guide](../include/juno/memory/README.md): Detailed memory management
- [API Documentation](../docs/): Generated Doxygen docs (build with `-DJUNO_DOCS=ON`)

## Questions?

Check the [LibJuno GitHub Issues](https://github.com/robinonsay/libjuno/issues) or consult inline documentation in headers.
