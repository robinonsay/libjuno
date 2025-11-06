# LibJuno Module System & Dependency Injection

This document explains the core architecture and module system that underlies LibJuno's design.

## Overview

LibJuno is built around a lightweight **module system** that enables **dependency injection (DI)** in C11. This system allows developers to write decoupled, testable, and portable embedded software without relying on dynamic memory allocation or complex frameworks.

## Core Concepts

### 1. Modules

A **module** in LibJuno consists of:

- **Module Root** (`JUNO_MODULE_ROOT`): Common, freestanding members shared by all implementations
  - Contains the API vtable pointer (`ptApi`)
  - Contains failure handler callback and user data
  - Should avoid hosted dependencies for portability

- **Derived Implementations** (`JUNO_MODULE_DERIVE`): Concrete implementations that embed the root
  - First member is always the root (accessible via `JUNO_MODULE_SUPER`)
  - Add implementation-specific state and dependencies
  - Enable safe up-casting to the root type

- **Module Union** (`JUNO_MODULE`): Union of root and all derivations
  - Enables safe type punning and polymorphism
  - All variants share the same API interface

### 2. API Vtables

APIs are defined as function pointer tables (vtables) that operate on the module root type:

```c
typedef struct MY_API_T {
    JUNO_STATUS_T (*Operation)(MY_ROOT_T *ptModule, ...);
    JUNO_RESULT_T (*Query)(const MY_ROOT_T *ptModule, ...);
} MY_API_T;
```

This pattern enables:
- **Runtime polymorphism**: Different implementations behind the same interface
- **Dependency injection**: Modules receive their dependencies as API pointers
- **Testability**: Mock implementations can be injected for testing

### 3. Dependency Injection

Instead of modules creating their own dependencies, they receive them:

```c
// Module receives time and logging APIs
JUNO_STATUS_T MyModule_Init(
    MY_MODULE_T *ptModule,
    const JUNO_TIME_ROOT_T *ptTime,      // Injected dependency
    const JUNO_LOG_ROOT_T *ptLog         // Injected dependency
);
```

Benefits:
- **No hidden dependencies**: All requirements are explicit in function signatures
- **Easy to test**: Inject mocks/stubs instead of real implementations
- **Flexible**: Swap implementations at runtime or compile time
- **Portable**: No global state or singleton patterns

## Module System Directory Structure

```
include/juno/
├── module.h           # Core module system macros
├── status.h           # Status codes and error handling
├── types.h            # Common type definitions
├── macros.h           # Utility macros
├── memory/            # Memory management APIs
│   ├── memory_api.h   # Memory allocator API
│   ├── pointer_api.h  # Pointer and type-safe memory access
│   └── memory_block.h # Block allocator implementation
├── ds/                # Data structures (queues, stacks, buffers, maps)
├── crc/               # CRC algorithms
├── time/              # Time management API
├── log/               # Logging API
├── sm/                # State machine API
├── sch/               # Scheduler API
├── sb/                # Service bus/broker API
├── io/                # I/O abstractions (SPI, I2C, UART, etc.)
├── hash/              # Hashing functions
└── math/              # Mathematical utilities
```

## Example: Using the Module System

### Define a Module

```c
// Forward declarations
typedef struct MY_API_T MY_API_T;
typedef struct MY_ROOT_T MY_ROOT_T;
typedef struct MY_IMPL_T MY_IMPL_T;

// API vtable
typedef struct MY_API_T {
    JUNO_STATUS_T (*DoWork)(MY_ROOT_T *ptModule);
} MY_API_T;

// Root: common interface
typedef struct MY_ROOT_T JUNO_MODULE_ROOT(MY_API_T,
    int iCommonField;
) MY_ROOT_T;

// Derived implementation
typedef struct MY_IMPL_T JUNO_MODULE_DERIVE(MY_ROOT_T,
    int iPrivateState;
    const JUNO_LOG_ROOT_T *ptLog;  // Injected dependency
) MY_IMPL_T;

// Module union
union MY_MODULE_T JUNO_MODULE(MY_API_T, MY_ROOT_T,
    MY_IMPL_T tImpl;
);
```

### Initialize and Use

```c
// Implementation
static JUNO_STATUS_T MyImpl_DoWork(MY_ROOT_T *ptModule) {
    MY_IMPL_T *ptImpl = (MY_IMPL_T *)ptModule;
    // Access private state and dependencies
    ptImpl->iPrivateState++;
    ptImpl->ptLog->ptApi->Info(ptImpl->ptLog, "Work done: %d", ptImpl->iPrivateState);
    return JUNO_STATUS_SUCCESS;
}

static const MY_API_T gMyApi = {
    .DoWork = MyImpl_DoWork
};

// Initialize with dependency injection
JUNO_STATUS_T MyModule_Init(MY_IMPL_T *ptImpl, const JUNO_LOG_ROOT_T *ptLog) {
    ptImpl->JUNO_MODULE_SUPER.ptApi = &gMyApi;
    ptImpl->JUNO_MODULE_SUPER.JUNO_FAILURE_HANDLER = NULL;
    ptImpl->JUNO_MODULE_SUPER.JUNO_FAILURE_USER_DATA = NULL;
    ptImpl->iPrivateState = 0;
    ptImpl->ptLog = ptLog;  // Inject dependency
    return JUNO_STATUS_SUCCESS;
}

// Use polymorphically
void UseModule(MY_ROOT_T *ptModule) {
    ptModule->ptApi->DoWork(ptModule);
}
```

## Traits vs Modules

LibJuno distinguishes between **modules** and **traits**:

- **Modules** (`JUNO_MODULE_ROOT`): Have failure handlers and full module infrastructure
- **Traits** (`JUNO_TRAIT_ROOT`): Lightweight, only carry the API pointer and data

Use traits for simple value types (like `JUNO_POINTER_T`) that don't need failure handling.

## Memory Safety

LibJuno's approach to memory safety:

1. **No dynamic allocation in library code**: All memory is provided by the caller
2. **Type-safe pointer API**: `JUNO_POINTER_T` carries size, alignment, and API information
3. **Explicit memory management**: Block allocators with static pools
4. **Reference semantics**: Clear ownership through API contracts

See `memory/README.md` for detailed memory management documentation.

## Status and Error Handling

All fallible operations return `JUNO_STATUS_T` (or a `JUNO_RESULT_T` variant):

```c
JUNO_STATUS_T tStatus = ptModule->ptApi->Operation(ptModule, ...);
if (tStatus != JUNO_STATUS_SUCCESS) {
    // Handle error
}
```

Modules can optionally register failure handlers that are called on error conditions for logging/diagnostics without affecting control flow.

## Freestanding Compatibility

LibJuno is designed to support **freestanding builds** (no hosted C standard library):

- Module roots avoid hosted types
- Build with `-DJUNO_FREESTANDING=ON` to enable freestanding mode
- Adds `-ffreestanding -nostdlib` flags
- Users provide their own printf/time implementations when needed

Note: Some examples and tests use hosted features for convenience, but the core library supports true freestanding operation.

## Documentation and Examples

- **Memory module**: See `memory/README.md` for detailed memory allocator documentation
- **Tutorial**: See `examples/example_project/LibJuno_Tutorial.md` for a complete walkthrough
- **Examples**: See `examples/README.md` for guides to minimal examples
- **API reference**: Build Doxygen documentation with `-DJUNO_DOCS=ON`

## Design Philosophy

LibJuno prioritizes:

1. **Memory Safety**: Type-safe APIs, no dynamic allocation, explicit bounds
2. **Software Scalability**: Modular design, clear dependencies, easy to extend
3. **Shareability**: Small, self-contained components easy to reuse across projects
4. **Transparency**: All dependencies explicit, no hidden global state
5. **Portability**: Freestanding support, minimal assumptions about target platform

The library **does not** prescribe:
- A specific RTOS or bare-metal environment
- A centralized executive or main loop
- Specific hardware abstractions

Instead, LibJuno provides tools that developers can compose to fit their specific requirements.

## Getting Started

1. Read the [main README](../../README.md) for build instructions
2. Review the [LibJuno Tutorial](../../examples/example_project/LibJuno_Tutorial.md)
3. Explore minimal examples in `examples/`
4. Consult inline Doxygen documentation in headers
5. Use template generators in `scripts/` to bootstrap your own modules

---

For questions or contributions, see the [GitHub repository](https://github.com/robinonsay/libjuno).
