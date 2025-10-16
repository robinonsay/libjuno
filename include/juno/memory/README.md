# Juno Memory Module

Deterministic, static, block-based memory management for embedded systems. This module avoids dynamic allocation and favors predictable behavior and explicit error paths.

This document reflects the current APIs in `juno/memory/memory_api.h` and `juno/memory/memory_block.h` and matches the unit tests in `tests/test_memory.c`.

## Overview

The memory module provides a fixed-size block allocator. You supply:
- A statically allocated memory array for elements of a single type
- A metadata array for the allocator’s free list
- A pointer API (Copy/Reset) that defines how to copy and reset one element

The allocator exposes an API vtable with Get/Update/Put. Get returns a result containing both a status and a pointer payload.

No reference counting is implemented in the current design.

## Core types (public headers)

- `JUNO_MEMORY_BLOCK_METADATA_T`
  - Per-block metadata used by the allocator’s free list. This is a simple public struct (currently contains a `uint8_t *ptFreeMem` field) used internally by the allocator. Treat it as allocator-managed storage: declare arrays with `JUNO_MEMORY_BLOCK_METADATA(...)` but avoid reading/modifying fields directly. The layout may evolve; consumers should not rely on specific members.

- `JUNO_POINTER_T`
  - Describes a block of memory the allocator returned:
    - `void *pvAddr;` — block address
    - `size_t zSize;` — block size in bytes
    - `size_t zAlignment;` — required alignment for the block
    - plus an internal `ptApi` member (via lite root) used by the pointer API

- `JUNO_POINTER_API_T`
  - Functions that the allocator calls per element type:
    - `JUNO_STATUS_T (*Copy)(JUNO_POINTER_T tDest, JUNO_POINTER_T tSrc);`
    - `JUNO_STATUS_T (*Reset)(JUNO_POINTER_T tPointer);`

- `JUNO_MEMORY_ALLOC_API_T`
  - Allocator vtable:
    - `JUNO_RESULT_POINTER_T (*Get)(JUNO_MEMORY_ALLOC_ROOT_T *ptMem, size_t zSize);`
    - `JUNO_STATUS_T (*Update)(JUNO_MEMORY_ALLOC_ROOT_T *ptMem, JUNO_POINTER_T *ptMemory, size_t zNewSize);`
    - `JUNO_STATUS_T (*Put)(JUNO_MEMORY_ALLOC_ROOT_T *ptMem, JUNO_POINTER_T *ptMemory);`

- `JUNO_MEMORY_ALLOC_ROOT_T` and `JUNO_MEMORY_ALLOC_BLOCK_T`
  - The concrete block allocator type is `JUNO_MEMORY_ALLOC_BLOCK_T` (derived). Its first member is a root (`tRoot`) that carries the API pointer and the pointer API.

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

typedef struct {
    int id;
    bool flag;
} MY_TYPE_T;

static JUNO_STATUS_T MyCopy(JUNO_POINTER_T tDest, JUNO_POINTER_T tSrc)
{
    JUNO_STATUS_T tStatus = JUNO_CHECK_POINTER_TYPE(tDest, MY_TYPE_T);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JUNO_CHECK_POINTER_TYPE(tSrc, MY_TYPE_T);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    *(MY_TYPE_T *)tDest.pvAddr = *(MY_TYPE_T *)tSrc.pvAddr;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T MyReset(JUNO_POINTER_T tPointer)
{
    JUNO_STATUS_T tStatus = JUNO_CHECK_POINTER_TYPE(tPointer, MY_TYPE_T);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    *(MY_TYPE_T *)tPointer.pvAddr = (MY_TYPE_T){0};
    return JUNO_STATUS_SUCCESS;
}

static const JUNO_POINTER_API_T gMyPtrApi = { MyCopy, MyReset };
```

Helpers provided by the API:
- `JUNO_CHECK_POINTER_TYPE(pointer, TYPE)` validates size and alignment
- `JunoMemory_PointerInit(api, TYPE, addr)` constructs a typed pointer descriptor (rarely needed by users)

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

## Allocating, updating, and freeing

Obtain the allocator API via the root member (`tRoot`), then call through it. Get returns a result struct; check `tStatus` before using `tOk`.

```c
const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tRoot.ptApi;

// Allocate
JUNO_RESULT_POINTER_T r = ptApi->Get(&tMem.tRoot, sizeof(MY_TYPE_T));
if (r.tStatus != JUNO_STATUS_SUCCESS) { /* handle full/invalid size */ }
JUNO_POINTER_T tPtr = r.tOk;
MY_TYPE_T *p = (MY_TYPE_T *)tPtr.pvAddr;

// Update size (must not exceed element size)
(void)ptApi->Update(&tMem.tRoot, &tPtr, sizeof(MY_TYPE_T));

// Free
(void)ptApi->Put(&tMem.tRoot, &tPtr);
```

### Typed convenience macros

`memory_block.h` provides small wrappers:

```c
// Get a block for a type; returns JUNO_RESULT_POINTER_T
JunoMemory_BlockGetT(&tMem, MY_TYPE_T);

// Put a previously acquired JUNO_POINTER_T*
JunoMemory_BlockPutT(&tMem, &tPtr);
```

## Common pitfalls

- Always check `tStatus` on results from `Get`; don’t use `tOk` unless success.
- Request sizes must be > 0 and <= element size (`zTypeSize`).
- Freeing the same pointer twice is rejected.
- Passing an address outside the managed block range is rejected.
- Ensure `pvMemory` is correctly aligned; pass `alignof(TYPE)` at init.

## Minimal end-to-end example (compiles like the tests)

```c
#include <stdalign.h>
#include <stdbool.h>
#include "juno/memory/memory_api.h"
#include "juno/memory/memory_block.h"

typedef struct { int id; bool flag; } MY_TYPE_T;

JUNO_MEMORY_BLOCK(gMem, MY_TYPE_T, 10);
JUNO_MEMORY_BLOCK_METADATA(gMeta, 10);

static JUNO_STATUS_T Copy(JUNO_POINTER_T d, JUNO_POINTER_T s) {
    JUNO_STATUS_T st = JUNO_CHECK_POINTER_TYPE(d, MY_TYPE_T);
    JUNO_ASSERT_SUCCESS(st, return st);
    st = JUNO_CHECK_POINTER_TYPE(s, MY_TYPE_T);
    JUNO_ASSERT_SUCCESS(st, return st);
    *(MY_TYPE_T*)d.pvAddr = *(MY_TYPE_T*)s.pvAddr; return JUNO_STATUS_SUCCESS;
}
static JUNO_STATUS_T Reset(JUNO_POINTER_T p) {
    JUNO_STATUS_T st = JUNO_CHECK_POINTER_TYPE(p, MY_TYPE_T);
    JUNO_ASSERT_SUCCESS(st, return st);
    *(MY_TYPE_T*)p.pvAddr = (MY_TYPE_T){0}; return JUNO_STATUS_SUCCESS;
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
- `JUNO_STATUS_MEMALLOC_ERROR`: Failed to allocate memory (block is full)
- `JUNO_STATUS_MEMFREE_ERROR`: Failed to free memory (invalid address or double free)
- `JUNO_STATUS_NULLPTR_ERROR`: Null pointer provided to function
- `JUNO_STATUS_INVALID_TYPE_ERROR`: Invalid memory allocator type
- `JUNO_STATUS_INVALID_SIZE_ERROR`: Invalid size parameter (e.g., zero size)

## Best Practices

1. **Always check status codes**: Memory operations can fail, especially in resource-constrained environments.
2. **Define appropriate block sizes**: Size your memory blocks based on your application's needs to avoid waste.
3. **Implement failure handlers**: Use failure handlers to catch memory issues early.
4. **Initialize your data**: Even if memory is zeroed, initialize your data structures explicitly to intent-reveal.
## Future work

Reference counting is planned but not implemented in the current allocator. The macros `JUNO_REF` and `JUNO_NEW_REF` are reserved for a future reference-counting extension and should not be used yet. Status codes such as `JUNO_STATUS_INVALID_REF_ERROR` and `JUNO_STATUS_REF_IN_USE_ERROR` are defined globally but are not produced by the block allocator at this time.
