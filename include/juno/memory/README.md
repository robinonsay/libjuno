# Juno Memory Module

Deterministic, static, block-based memory management for embedded systems. This module avoids dynamic allocation and favors predictable behavior and explicit error paths.

This document reflects the current APIs in `juno/memory/memory_api.h`, `juno/memory/pointer_api.h`, and `juno/memory/memory_block.h` and matches the unit tests in `tests/test_memory.c`.

## Overview

The memory module provides a fixed-size block allocator. You supply:
- A statically allocated memory array for elements of a single type
- A metadata array for the allocator's free list
- A pointer API (Copy/Reset) that defines how to copy and reset one element

The allocator exposes an API vtable with Get/Update/Put. Get returns a result containing both a status and a pointer payload.

The module integrates with LibJuno's module system:
- `JUNO_POINTER_T` uses `JUNO_TRAIT_ROOT` to carry its API pointer
- `JUNO_MEMORY_ALLOC_ROOT_T` uses `JUNO_MODULE_ROOT` with both the allocator API and pointer API
- `JUNO_MEMORY_ALLOC_BLOCK_T` uses `JUNO_MODULE_DERIVE` to extend the root with implementation fields
- Failure handlers can be attached to allocators for error reporting

No reference counting is implemented in the current design.

## Core types (public headers)

### `JUNO_MEMORY_BLOCK_METADATA_T`
Per-block metadata used by the allocator's free list. This is a simple public struct (currently contains a `uint8_t *ptFreeMem` field) used internally by the allocator. Treat it as allocator-managed storage: declare arrays with `JUNO_MEMORY_BLOCK_METADATA(...)` but avoid reading/modifying fields directly. The layout may evolve; consumers should not rely on specific members.

### `JUNO_POINTER_T`
Describes a block of memory the allocator returned (defined in `pointer_api.h`):
- `void *pvAddr;` — block address
- `size_t zSize;` — block size in bytes
- `size_t zAlignment;` — required alignment for the block
- `const JUNO_POINTER_API_T *ptApi;` — pointer to the API (via `JUNO_TRAIT_ROOT` macro)

### `JUNO_POINTER_API_T`
Functions that the allocator calls per element type (defined in `pointer_api.h`):
- `JUNO_STATUS_T (*Copy)(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);` — copy from source to destination
- `JUNO_STATUS_T (*Reset)(JUNO_POINTER_T tPointer);` — reset/zero-initialize the memory

### `JUNO_MEMORY_ALLOC_API_T`
Allocator vtable:
- `JUNO_RESULT_POINTER_T (*Get)(JUNO_MEMORY_ALLOC_ROOT_T *ptMem, size_t zSize);` — allocate a block
- `JUNO_STATUS_T (*Update)(JUNO_MEMORY_ALLOC_ROOT_T *ptMem, JUNO_POINTER_T *ptMemory, size_t zNewSize);` — update block size
- `JUNO_STATUS_T (*Put)(JUNO_MEMORY_ALLOC_ROOT_T *ptMem, JUNO_POINTER_T *ptMemory);` — free a block

### `JUNO_MEMORY_ALLOC_ROOT_T` and `JUNO_MEMORY_ALLOC_BLOCK_T`
The concrete block allocator type is `JUNO_MEMORY_ALLOC_BLOCK_T` (derived via `JUNO_MODULE_DERIVE`). It contains:
- `tRoot` member of type `JUNO_MEMORY_ALLOC_ROOT_T` carrying the API pointer and pointer API
- `pvMemory` — pointer to the allocated memory area (uint8_t*)
- `ptMetadata` — array of metadata for each block
- `zTypeSize` — size of each block element
- `zAlignment` — required alignment
- `zLength` — total number of blocks available
- `zUsed` — current count of allocated blocks (monotonically increases)
- `zFreed` — current count of freed blocks in the free stack

### `JUNO_VALUE_POINTER_T` and `JUNO_VALUE_POINTER_API_T`
Extended pointer types with value semantics (defined in `pointer_api.h`):
- `JUNO_VALUE_POINTER_T` derives from `JUNO_POINTER_T` 
- `JUNO_VALUE_POINTER_API_T` adds `Equals` comparison operation

## Declaring memory and metadata

Use the provided macros to declare storage:

```c
// For C: initializes to {0}; for C++: {}
JUNO_MEMORY_BLOCK(myBlock, MY_TYPE_T, 10);
JUNO_MEMORY_BLOCK_METADATA(myMeta, 10);
```

These create:
- `static MY_TYPE_T myBlock[10];`
- `static JUNO_MEMORY_BLOCK_METADATA_T myMeta[10];`

## Pointer API (Copy and Reset)

You must provide a `JUNO_POINTER_API_T` for your element type. For trivially copyable POD structs, Copy can assign and Reset can zero or reinitialize.

```c
#include <stdalign.h>
#include "juno/memory/memory_api.h"
#include "juno/memory/pointer_api.h"

typedef struct {
    int id;
    bool flag;
} MY_TYPE_T;

// Forward declare the API to use in checks
static const JUNO_POINTER_API_T gMyPtrApi;

static JUNO_STATUS_T MyCopy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc)
{
    JUNO_STATUS_T tStatus = JunoMemory_PointerCheckType(tDest, MY_TYPE_T, gMyPtrApi);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerCheckType(tSrc, MY_TYPE_T, gMyPtrApi);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    *(MY_TYPE_T *)tDest.pvAddr = *(MY_TYPE_T *)tSrc.pvAddr;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T MyReset(JUNO_POINTER_T tPointer)
{
    JUNO_STATUS_T tStatus = JunoMemory_PointerCheckType(tPointer, MY_TYPE_T, gMyPtrApi);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    *(MY_TYPE_T *)tPointer.pvAddr = (MY_TYPE_T){0};
    return JUNO_STATUS_SUCCESS;
}

static const JUNO_POINTER_API_T gMyPtrApi = { MyCopy, MyReset };
```

### Helpers provided by the API

- `JunoMemory_PointerCheckType(pointer, TYPE, tApi)` — validates size, alignment, and API matching
- `JunoMemory_PointerInit(api, TYPE, addr)` — constructs a typed pointer descriptor (rarely needed by users)
- `JUNO_ASSERT_POINTER_COPY(tDest, tSrc, tApi)` — validates preconditions for copy operations
- `JunoMemory_PointerApiVerify(ptPointerApi)` — verifies a pointer API vtable is complete
- `JunoMemory_PointerVerify(tPointer)` — verifies a pointer structure is valid
- `JunoMemory_AllocApiVerify(ptAllocApi)` — verifies an allocator API vtable is complete
- `JunoMemory_AllocVerify(ptAlloc)` — verifies an allocator root structure is valid

## Initializing the block allocator

Signature (from `memory_block.h`):

```c
JUNO_STATUS_T JunoMemory_BlockInit(
    JUNO_MEMORY_ALLOC_BLOCK_T *ptJunoMemoryBlock,
    const JUNO_POINTER_API_T *ptPointerApi,
    void *pvMemory,
    JUNO_MEMORY_BLOCK_METADATA_T *ptMetadata,
    size_t zTypeSize,
    size_t zAlignment,
    size_t zLength,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
);
```

Usage:

```c
#include "juno/memory/memory_block.h"

JUNO_MEMORY_ALLOC_BLOCK_T tMem = {0};
JUNO_STATUS_T tStatus = JunoMemory_BlockInit(
    &tMem,
    &gMyPtrApi,
    myBlock,
    myMeta,
    sizeof(MY_TYPE_T),
    alignof(MY_TYPE_T),
    10,
    /* failure handler */ NULL,
    /* user data */ NULL
);
if (tStatus != JUNO_STATUS_SUCCESS) { /* handle error */ }
```

Notes:
- `ptPointerApi` and `zAlignment` are required and validated.
- `pvMemory` must be aligned to `zAlignment`.
- Failure handler and user data are optional.
- The function validates parameters and sets up the allocator's internal state.

## Allocating, updating, and freeing

Obtain the allocator API via the root member (`tRoot`), then call through it. Get returns a result struct; check `tStatus` before using `tOk`.

```c
const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tRoot.ptApi;

// Allocate
JUNO_RESULT_POINTER_T r = ptApi->Get(&tMem.tRoot, sizeof(MY_TYPE_T));
if (r.tStatus != JUNO_STATUS_SUCCESS) { /* handle full/invalid size */ }
JUNO_POINTER_T tPtr = r.tOk;
MY_TYPE_T *p = (MY_TYPE_T *)tPtr.pvAddr;

// The returned pointer includes:
//   - tPtr.pvAddr: address of the allocated block
//   - tPtr.zSize: size set to zTypeSize from the allocator
//   - tPtr.zAlignment: alignment from the allocator
//   - tPtr.ptApi: pointer to the API used for Copy/Reset

// Update size (must not exceed element size)
JUNO_STATUS_T st = ptApi->Update(&tMem.tRoot, &tPtr, sizeof(MY_TYPE_T));
if (st != JUNO_STATUS_SUCCESS) { /* handle size too large */ }

// Free (pointer is cleared on success)
st = ptApi->Put(&tMem.tRoot, &tPtr);
if (st == JUNO_STATUS_SUCCESS) {
    // tPtr.pvAddr is now NULL, zSize and zAlignment are zeroed
}
```

### Typed convenience macros

`memory_block.h` provides small wrappers:

```c
// Get a block for a type; returns JUNO_RESULT_POINTER_T
JUNO_RESULT_POINTER_T r = JunoMemory_BlockGetT(&tMem, MY_TYPE_T);

// Put a previously acquired JUNO_POINTER_T*
JUNO_STATUS_T st = JunoMemory_BlockPutT(&tMem, &tPtr);
```

## Internal allocator behavior

The block allocator maintains:
- `zUsed`: Total number of blocks ever allocated (monotonically increases until pool is full)
- `zFreed`: Count of freed blocks available for reuse (decreases on Get, increases on Put)
- Freed blocks are tracked in the `ptMetadata` array as a stack (LIFO order)

Allocation strategy:
1. If freed blocks exist (`zFreed > 0`), reuse the most recently freed block
2. Otherwise, if room remains (`zUsed < zLength`), allocate a new block from the pool
3. When all blocks are allocated and none are freed, return `JUNO_STATUS_MEMALLOC_ERROR`

The allocator automatically calls `Reset` on newly allocated blocks. If `Reset` fails, the block is returned to the free list and the error is propagated.

## Common pitfalls

- Always check `tStatus` on results from `Get`; don't use `tOk` unless success.
- Request sizes must be > 0 and <= element size (`zTypeSize`).
- Freeing the same pointer twice is rejected with `JUNO_STATUS_MEMFREE_ERROR`.
- Passing an address outside the managed block range is rejected.
- Passing an unaligned address to `Put` is rejected.
- Ensure `pvMemory` is correctly aligned; pass `alignof(TYPE)` at init.
- After a successful `Put`, the pointer's `pvAddr` is set to NULL, `zSize` and `zAlignment` are zeroed.
- The `JunoMemory_PointerCheckType` macro now requires three arguments: `(pointer, TYPE, tApi)`.
- The `Copy` function signature takes `const JUNO_POINTER_T tSrc` (const second parameter).
- When the `Reset` operation fails during `Get`, the block is returned to the free list automatically.

## Minimal end-to-end example (compiles like the tests)

```c
#include <stdalign.h>
#include <stdbool.h>
#include "juno/memory/memory_api.h"
#include "juno/memory/memory_block.h"
#include "juno/memory/pointer_api.h"

typedef struct { int id; bool flag; } MY_TYPE_T;

JUNO_MEMORY_BLOCK(gMem, MY_TYPE_T, 10);
JUNO_MEMORY_BLOCK_METADATA(gMeta, 10);

// Forward declare the API to use in checks
static const JUNO_POINTER_API_T gPtrApi;

static JUNO_STATUS_T Copy(JUNO_POINTER_T d, const JUNO_POINTER_T s) {
    JUNO_STATUS_T st = JunoMemory_PointerCheckType(d, MY_TYPE_T, gPtrApi);
    JUNO_ASSERT_SUCCESS(st, return st);
    st = JunoMemory_PointerCheckType(s, MY_TYPE_T, gPtrApi);
    JUNO_ASSERT_SUCCESS(st, return st);
    *(MY_TYPE_T*)d.pvAddr = *(MY_TYPE_T*)s.pvAddr; 
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T Reset(JUNO_POINTER_T p) {
    JUNO_STATUS_T st = JunoMemory_PointerCheckType(p, MY_TYPE_T, gPtrApi);
    JUNO_ASSERT_SUCCESS(st, return st);
    *(MY_TYPE_T*)p.pvAddr = (MY_TYPE_T){0}; 
    return JUNO_STATUS_SUCCESS;
}

static const JUNO_POINTER_API_T gPtrApi = { Copy, Reset };

void example(void) {
    JUNO_MEMORY_ALLOC_BLOCK_T mem = {0};
    JUNO_STATUS_T st = JunoMemory_BlockInit(&mem, &gPtrApi, gMem, gMeta,
                                            sizeof(MY_TYPE_T), alignof(MY_TYPE_T), 10,
                                            NULL, NULL);
    if (st != JUNO_STATUS_SUCCESS) return;
    const JUNO_MEMORY_ALLOC_API_T *api = mem.tRoot.ptApi;

    JUNO_RESULT_POINTER_T r = api->Get(&mem.tRoot, sizeof(MY_TYPE_T));
    if (r.tStatus != JUNO_STATUS_SUCCESS) return;
    JUNO_POINTER_T ptr = r.tOk;
    ((MY_TYPE_T*)ptr.pvAddr)->id = 42;
    (void)api->Put(&mem.tRoot, &ptr);
}
```

## Freestanding notes

The module is suitable for freestanding builds. Avoid hosted headers in your Copy/Reset implementations; prefer simple assignment/zeroing for POD types. In freestanding mode, enable the CMake option `-DJUNO_FREESTANDING=ON` and ensure your `pvMemory` buffer is properly aligned for the element type.

## Error Handling

The memory module reports errors through status codes of type `JUNO_STATUS_T`:

- `JUNO_STATUS_SUCCESS`: Operation completed successfully
- `JUNO_STATUS_MEMALLOC_ERROR`: Failed to allocate memory (block is full or size exceeds element size)
- `JUNO_STATUS_MEMFREE_ERROR`: Failed to free memory (invalid address, double free, or unaligned address)
- `JUNO_STATUS_NULLPTR_ERROR`: Null pointer provided to function (handled by `JUNO_ASSERT_EXISTS`)
- `JUNO_STATUS_INVALID_TYPE_ERROR`: Invalid memory allocator type (API mismatch)
- `JUNO_STATUS_INVALID_SIZE_ERROR`: Invalid size parameter (e.g., zero size)
- `JUNO_STATUS_ERR`: General error (e.g., corrupt allocator counters, alignment issues, overflow)

## Best Practices

1. **Always check status codes**: Memory operations can fail, especially in resource-constrained environments.
2. **Define appropriate block sizes**: Size your memory blocks based on your application's needs to avoid waste.
3. **Implement failure handlers**: Use failure handlers to catch memory issues early during development.
4. **Initialize your data**: Even if memory is zeroed by Reset, initialize your data structures explicitly to reveal intent.
5. **Use typed macros**: Prefer `JunoMemory_BlockGetT` and `JunoMemory_BlockPutT` to reduce size mismatches.
6. **Forward declare APIs**: When using `JunoMemory_PointerCheckType` in Copy/Reset, forward declare the API constant.

## Future work

Reference counting is planned but not implemented in the current allocator. The macros `JUNO_REF` and `JUNO_NEW_REF` are reserved for a future reference-counting extension and should not be used yet. Status codes such as `JUNO_STATUS_INVALID_REF_ERROR` and `JUNO_STATUS_REF_IN_USE_ERROR` are defined globally but are not produced by the block allocator at this time.
