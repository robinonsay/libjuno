# Architecture — LibJuno

## Architectural Pattern: Dependency Injection via Vtables

LibJuno implements a C-native dependency injection (DI) system using vtables
and struct embedding. This enables polymorphism, testability, and modularity
without dynamic allocation or C++ features.

## Module System

### Module Root

Every module has a root struct defined via `JUNO_MODULE_ROOT(...)`:

```c
struct MY_MODULE_ROOT_TAG {
    const MY_MODULE_API_T *ptApi;          // Vtable pointer
    JUNO_FAILURE_HANDLER_T _pfcnFailureHandler;  // Optional diagnostic callback
    void *_pvFailureUserData;              // User data for failure handler
};
```

### Module Derivation

Concrete implementations embed the root as their first member via
`JUNO_MODULE_DERIVE(...)`:

```c
struct MY_MODULE_IMPL_TAG {
    MY_MODULE_ROOT_T tRoot;     // JUNO_MODULE_SUPER — must be first
    // Implementation-specific state...
    void *pvBuffer;
    size_t zCapacity;
};
```

This enables safe upcasting from any derivation to its root.

### Module Union

A union of root + derivations via `JUNO_MODULE(...)`:

```c
union MY_MODULE_T {
    MY_MODULE_ROOT_T tRoot;
    MY_MODULE_IMPL_A_T tImplA;
    MY_MODULE_IMPL_B_T tImplB;
};
```

### Vtable (API Struct)

Function pointer tables for polymorphic dispatch:

```c
typedef struct MY_MODULE_API_TAG {
    JUNO_STATUS_T (*DoSomething)(MY_MODULE_ROOT_T *ptRoot, ...);
    JUNO_RESULT_T (*GetValue)(MY_MODULE_ROOT_T *ptRoot, ...);
} MY_MODULE_API_T;
```

Dispatch: `ptModule->ptApi->DoSomething(ptModule, ...)`

### Trait Root

Lightweight interfaces without failure handler via `JUNO_TRAIT_ROOT(...)`.
Used for small capabilities like `JUNO_POINTER_T`.

## Initialization Pattern

Every module follows this pattern:

1. **Caller allocates all storage** (stack, static, or their own memory pool).
2. **`Init` function** wires the vtable, stores dependencies, calls `Verify`.
3. **`Verify` function** validates all pointers and dependencies are non-NULL.
4. All public functions call `Verify` at entry.

```c
JUNO_STATUS_T JunoDs_Heap_Init(
    JUNO_DS_HEAP_ROOT_T *ptHeap,
    JUNO_DS_ARRAY_ROOT_T *ptArray,       // Injected dependency
    JUNO_POINTER_API_T *ptPointerApi,    // Injected trait
    JUNO_FAILURE_HANDLER_T pfcnHandler,  // Optional
    void *pvUserData                     // Optional
);
```

## Pointer System

`JUNO_POINTER_T` is a fat pointer: `{ptApi, pvAddr, zSize, zAlignment}`.
It provides type-safe memory access with Copy/Reset operations, enabling
data structures to operate on arbitrary types without `void *` arithmetic.

## Result / Option Types

Inspired by Rust:

- `JUNO_MODULE_RESULT(NAME_T, OK_T)` → `{JUNO_STATUS_T tStatus; OK_T tOk;}`
- `JUNO_MODULE_OPTION(NAME_T, SOME_T)` → `{bool bIsSome; SOME_T tSome;}`
- Extract with `JUNO_OK(result)` and `JUNO_SOME(option)`.

## Directory Layout

```
include/juno/         ← Public API headers
  module.h            ← Module system primitives
  status.h            ← Status codes
  types.h             ← Common result types
  macros.h            ← Assertion and utility macros
  ds/                 ← Data structure APIs
  memory/             ← Memory and pointer APIs
  crc/                ← CRC algorithm APIs
  ...                 ← Other subsystems
src/                  ← Implementation files
tests/                ← Unity test files
scripts/              ← Code generators and tooling
templates/            ← Boilerplate templates for new modules
requirements/         ← Per-module requirements.json files
```

## Test Architecture

- **Framework**: Unity (ThrowTheSwitch)
- **Pattern**: `setUp()` / `tearDown()` with global static fixtures
- **Test doubles**: Hand-crafted via vtable injection (no mock framework)
  - Injectable failure flags (e.g., `bFailCompare`) for error path testing
  - Custom API structs implementing the module's vtable interface
- **CMake**: Each test file compiles to a separate executable, registered via `add_test`
