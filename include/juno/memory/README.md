# Juno Memory Module

The Juno Memory Module provides a unified API for memory management in embedded systems. It supports both generic memory allocation and fixed-size block-based allocation. The design is tailored for embedded projects, ensuring flexibility, deterministic failure handling, and future extensibility.

---

## Overview

- **Generic Memory Allocation**  
  The generic API offers functions to:
  - **Allocate:** `Juno_MemoryGet`
  - **Update:** `Juno_MemoryUpdate` (when using block allocation, it checks that the new size fits within the fixed block size)
  - **Free:** `Juno_MemoryPut`

- **Block-Based Memory Allocation**  
  This implementation manages a pre-allocated, fixed-size memory pool. You declare the memory region and associated metadata with the provided macros:
  - `JUNO_MEMORY_BLOCK(name, type, length)`  
  - `JUNO_MEMORY_BLOCK_METADATA(name, length)`

  The block allocator is initialized using `Juno_MemoryBlkInit`, which sets up the memory block structure, free stack, block size, and an optional failure handler callback.

- **Failure Handling**  
  The module employs a failure callback (of type `JUNO_FAILURE_HANDLER_T` as defined in `juno/status.h`) to handle errors deterministically in allocation, update, or free operations. This callback receives:
  - The error status.
  - A custom message.
  - Optional user data.

- **Custom Allocators (JUNO_CUSTOM_ALLOC)**  
  By default, the memory module declares the union `JUNO_MEMORY_ALLOC_T` which contains both a generic header (`tHdr`) and a block-based allocator (`tBlock`).  
  If the macro `JUNO_CUSTOM_ALLOC` is defined, the module expects the developer to provide their own definitions for memory allocation types. This allows for integration with external/custom memory managers without being tied to the default union and its implementations.

---

## Data Structures

The key structures and macros are defined in memory_types.h:

- **JUNO_MEMORY_BLOCK_T**  
  This structure holds details of block-based allocation such as:
  - Pointer to the memory area.
  - Pointer to the metadata array.
  - Block size (`zTypeSize`), total number of blocks (`zLength`), blocks used (`zUsed`) and blocks freed (`zFreed`).
  - A failure handler defined using the macro `DECLARE_FAILURE_HANDLER`.

- **JUNO_MEMORY_ALLOC_T**  
  If `JUNO_CUSTOM_ALLOC` is not defined, this union is declared to accommodate multiple allocation types (currently block-based). If `JUNO_CUSTOM_ALLOC` is defined, you must provide your own implementation of allocation types.

---

## How to Use

### 1. Declare Memory Blocks & Metadata

Define your memory blocks and metadata arrays using the supplied macros:

````cpp
#include "juno/memory/memory.h"
#include "juno/memory/memory_types.h"
#include "juno/status.h"

typedef struct MY_BLOCK_TAG {
    int id;
    char data[32];
} MY_BLOCK_T;

JUNO_MEMORY_BLOCK(myBlock, MY_BLOCK_T, 20);
JUNO_MEMORY_BLOCK_METADATA(myMetadata, 20);
````

### 2. Initialize the Block Allocator

Initialize the block-based memory allocator with its memory region, metadata, block size, and (optionally) a failure handler. For example:

````cpp
#include <string.h>
#include <stdio.h>

int main(void) {
    JUNO_MEMORY_BLOCK_T memBlock = {0};

    // Initialize block allocator; pass NULL for a failure handler if not needed.
    JUNO_STATUS_T status = Juno_MemoryBlkInit(
        &memBlock,
        myBlock,
        myMetadata,
        sizeof(MY_BLOCK_T),
        20,
        NULL,    // No failure callback
        NULL     // No user data
    );

    if (status != JUNO_STATUS_SUCCESS) {
        fprintf(stderr, "Memory block initialization failed!\n");
        return -1;
    }

    // Continue using the allocator...
    return 0;
}
````

### 3. Allocate, Update, and Free Memory Blocks

Use the block-specific functions to manage memory allocations:

````cpp
#include <stdio.h>

void useMemoryBlocks(JUNO_MEMORY_BLOCK_T *memBlock) {
    JUNO_MEMORY_T memDesc = {0};
    JUNO_STATUS_T status = Juno_MemoryBlkGet(memBlock, &memDesc);
    if (status == JUNO_STATUS_SUCCESS && memDesc.pvAddr != NULL) {
        MY_BLOCK_T *pData = (MY_BLOCK_T *)memDesc.pvAddr;
        pData->id = 42;
        snprintf(pData->data, sizeof(pData->data), "Hello Juno!");

        printf("Allocated Block: ID %d, Data: %s\n", pData->id, pData->data);

        // Try updating the block; the update verifies that the new size does not exceed the fixed size.
        status = Juno_MemoryBlkUpdate(memBlock, &memDesc, sizeof(MY_BLOCK_T));
        if (status != JUNO_STATUS_SUCCESS) {
            fprintf(stderr, "Block update failed!\n");
        }

        // Free the block.
        status = Juno_MemoryBlkPut(memBlock, &memDesc);
        if (status != JUNO_STATUS_SUCCESS) {
            fprintf(stderr, "Block free failed!\n");
        }
    } else {
        fprintf(stderr, "Block allocation failed!\n");
    }
}
````

### 4. Using the Generic Memory API

You can initialize a `JUNO_MEMORY_ALLOC_T` directly using the block implementation. This allows you to use the generic allocator functions seamlessly:

````cpp
#include <stdio.h>
#include <string.h>
#include "juno/memory/memory.h"
#include "juno/memory/memory_types.h"
#include "juno/memory/alloc.h"
#include "juno/status.h"

// Define a custom type for the memory block.
typedef struct MY_BLOCK_TAG {
    int id;
    char data[32];
} MY_BLOCK_T;

// Declare memory block and metadata arrays.
JUNO_MEMORY_BLOCK(myBlock, MY_BLOCK_T, 20);
JUNO_MEMORY_BLOCK_METADATA(myMetadata, 20);

int main(void) {
    // Initialize the generic allocator with the block implementation.
    JUNO_MEMORY_ALLOC_T myAlloc = {0};

    // Use the tBlock member to initialize the block allocator.
    JUNO_STATUS_T status = Juno_MemoryBlkInit(
        &myAlloc.tBlock,     // Initialize using the tBlock member of the union
        myBlock,
        myMetadata,
        sizeof(MY_BLOCK_T),
        20,
        NULL,    // No failure callback.
        NULL     // No user data.
    );

    if (status != JUNO_STATUS_SUCCESS) {
        fprintf(stderr, "Memory block initialization failed!\n");
        return -1;
    }

    // Allocate memory using the generic API.
    JUNO_MEMORY_T memDesc = {0};
    status = Juno_MemoryGet(&myAlloc, &memDesc, sizeof(MY_BLOCK_T));
    if (status == JUNO_STATUS_SUCCESS && memDesc.pvAddr != NULL) {
        MY_BLOCK_T *pData = (MY_BLOCK_T *)memDesc.pvAddr;
        pData->id = 7;
        snprintf(pData->data, sizeof(pData->data), "Generic Alloc!");

        printf("Allocated Block: ID %d, Data: %s\n", pData->id, pData->data);

        // Optionally update memory using the generic update API.
        status = Juno_MemoryUpdate(&myAlloc, &memDesc, sizeof(MY_BLOCK_T));
        if (status != JUNO_STATUS_SUCCESS) {
            fprintf(stderr, "Memory update failed!\n");
        }

        // Free the memory using the generic free API.
        status = Juno_MemoryPut(&myAlloc, &memDesc);
        if (status != JUNO_STATUS_SUCCESS) {
            fprintf(stderr, "Memory free failed!\n");
        }
    } else {
        fprintf(stderr, "Memory allocation failed!\n");
    }

    return 0;
}
````

### 5. Example: Using a Failure Handler

The failure handler enables deterministic error management. For example, the following code demonstrates a callback that logs errors to `stderr`:

````cpp
#include <stdio.h>
#include "juno/status.h"

// Callback function matching JUNO_FAILURE_HANDLER_T signature.
void myFailureHandler(JUNO_STATUS_T tStatus, const char *pcCustomMessage, JUNO_USER_DATA_T *pvUserData) {
    (void)pvUserData;  // Unused in this example.
    fprintf(stderr, "Memory Error (code %d): %s\n", tStatus, pcCustomMessage);
}

typedef struct MY_BLOCK_TAG {
    int id;
    char data[32];
} MY_BLOCK_T;

JUNO_MEMORY_BLOCK(myBlock, MY_BLOCK_T, 10);
JUNO_MEMORY_BLOCK_METADATA(myMetadata, 10);

int main(void) {
    JUNO_MEMORY_BLOCK_T memBlock = {0};

    // Initialize block allocator with a failure handler.
    JUNO_STATUS_T status = Juno_MemoryBlkInit(
        &memBlock,
        myBlock,
        myMetadata,
        sizeof(MY_BLOCK_T),
        10,
        myFailureHandler,  // Custom failure callback.
        NULL               // No additional user data.
    );

    if (status != JUNO_STATUS_SUCCESS) {
        fprintf(stderr, "Memory block initialization failed!\n");
        return -1;
    }

    // Force an error scenario: attempt to update memory with a size larger than allowed.
    JUNO_MEMORY_T memDesc = {0};
    status = Juno_MemoryBlkGet(&memBlock, &memDesc);
    if (status == JUNO_STATUS_SUCCESS) {
        // Intentionally request a size that exceeds the fixed block size.
        status = Juno_MemoryBlkUpdate(&memBlock, &memDesc, sizeof(MY_BLOCK_T) + 10);
        // If the size is too large, the failure handler will be invoked.
    }

    // Free the block if it was allocated.
    Juno_MemoryBlkPut(&memBlock, &memDesc);
    return 0;
}
````

### Using the API

The Juno Memory Module exposes its operations through function pointer APIs, allowing downstream code to remain agnostic to the underlying allocation method. This design means that whether you're using the default block-based allocator or a fully custom implementation, the API remains the same.

The module provides two primary API structures:

- **Generic Memory API:**  
  Obtain this API using the function `Juno_MemoryApi()`. It provides function pointers for the generic operations:
  - `Get` – to allocate memory.
  - `Update` – to update (or reallocate) memory.
  - `Put` – to free memory.

- **Block-Based Memory API:**  
  Accessed via `Juno_MemoryBlkApi()`, this API primarily offers an initialization function (`Init`) for block-based allocators.

Below is an example demonstrating how to use the generic API to allocate, update, and free memory from a block-based allocator:

````cpp
#include <stdio.h>
#include <string.h>
#include "juno/memory/memory.h"
#include "juno/memory/memory_types.h"
#include "juno/memory/alloc.h"
#include "juno/memory/memory_api.h"
#include "juno/status.h"

// Define a custom block type
typedef struct MY_BLOCK_TAG {
    int id;
    char data[32];
} MY_BLOCK_T;

// Declare static memory block and metadata using provided macros
JUNO_MEMORY_BLOCK(myBlock, MY_BLOCK_T, 20);
JUNO_MEMORY_BLOCK_METADATA(myMetadata, 20);

int main(void) {
    // Initialize the block-based allocator
    JUNO_MEMORY_ALLOC_T memBlock = {0};
    JUNO_STATUS_T status = Juno_MemoryBlkInit(
        &memBlock.tBlock,
        myBlock,
        myMetadata,
        sizeof(MY_BLOCK_T),
        20,
        NULL,   // No failure handler in this example
        NULL
    );
    if (status != JUNO_STATUS_SUCCESS) {
        fprintf(stderr, "Block allocator initialization failed\n");
        return -1;
    }

    // Retrieve the generic memory API, which abstracts the underlying allocation method.
    const JUNO_MEMORY_API_T *pApi = Juno_MemoryApi();
    JUNO_MEMORY_T memDesc = {0};

    // Allocate a memory block using the generic Get API.
    status = pApi->Get(&memBlock, &memDesc, sizeof(MY_BLOCK_T));
    if (status != JUNO_STATUS_SUCCESS || memDesc.pvAddr == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return -1;
    }

    // Use the allocated memory.
    MY_BLOCK_T *pBlock = (MY_BLOCK_T *)memDesc.pvAddr;
    pBlock->id = 100;
    strcpy(pBlock->data, "Using API function pointers");
    printf("Allocated Block: ID %d, Data: %s\n", pBlock->id, pBlock->data);

    // Update the memory block (for block-based allocation this validates that the new size fits).
    status = pApi->Update(&memBlock, &memDesc, sizeof(MY_BLOCK_T));
    if (status != JUNO_STATUS_SUCCESS) {
        fprintf(stderr, "Memory update failed\n");
    }

    // Free the allocated block using the generic Put API.
    status = pApi->Put(&memBlock, &memDesc);
    if (status != JUNO_STATUS_SUCCESS) {
        fprintf(stderr, "Memory free failed\n");
        return -1;
    }

    return 0;
}
````

In this example:

- We initialize a block-based allocator.
- We retrieve the generic API using `Juno_MemoryApi()`.
- We invoke the `Get`, `Update`, and `Put` functions through the API, which internally dispatch to the block-based implementation (or any other implementation if replaced).

Downstream consumers of the API can call these functions without knowing whether the underlying allocator is block-based or custom—promoting modularity and future extensibility.

## Custom Allocators

If you wish to use a completely custom memory allocation implementation (for example, to integrate with an external memory manager), you can simply not include the default block-based implementation. Instead, implement your own custom allocation type and provide implementations for the allocator functions defined in alloc.h. Downstream consumers will use the same API regardless of the underlying implementation.

Below is an example of a custom allocator that uses POSIX malloc, realloc, and free:

````cpp
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "juno/status.h"

// Developer-defined custom allocator type.
// This type can store any context information needed by your allocator.
typedef struct JUNO_MEMORY_ALLOC_T {
    int dummy;  // Example context field.
} JUNO_MEMORY_ALLOC_T;
#define JUNO_CUSTOM_ALLOC
#include "juno/memory/alloc.h"
// The memory descriptor type is defined in the API as JUNO_MEMORY_T.
// Here, we will use it directly which contains:
//   void *pvAddr;   // Pointer to the allocated memory.
//   size_t zSize;   // Size of the allocation.

// Custom implementation of the generic memory allocation functions.
// Downstream code will call these functions exactly as defined in alloc.h.

JUNO_STATUS_T Juno_MemoryGet(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *ptMemory, size_t zSize) {
    // The ptMem parameter can be cast to your custom allocator type if needed.
    (void)ptMem;  // Unused in this simple example.
    ptMemory->pvAddr = malloc(zSize);
    if (ptMemory->pvAddr == NULL) {
        return JUNO_STATUS_MEMALLOC_ERROR;
    }
    ptMemory->zSize = zSize;
    return JUNO_STATUS_SUCCESS;
}

JUNO_STATUS_T Juno_MemoryUpdate(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *ptMemory, size_t zNewSize) {
    (void)ptMem;  // Unused in this simple example.
    void *pNew = realloc(ptMemory->pvAddr, zNewSize);
    if (pNew == NULL) {
        return JUNO_STATUS_MEMALLOC_ERROR;
    }
    ptMemory->pvAddr = pNew;
    ptMemory->zSize = zNewSize;
    return JUNO_STATUS_SUCCESS;
}

JUNO_STATUS_T Juno_MemoryPut(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *ptMemory) {
    (void)ptMem;  // Unused in this simple example.
    free(ptMemory->pvAddr);
    ptMemory->pvAddr = NULL;
    ptMemory->zSize = 0;
    return JUNO_STATUS_SUCCESS;
}

int main(void) {
    // Create an instance of your custom allocator.
    MY_CUSTOM_ALLOC_T myAllocStruct = { 0 };
    // We emulate a JUNO_MEMORY_ALLOC_T by casting the address of our custom allocator.
    JUNO_MEMORY_ALLOC_T *pAlloc = (JUNO_MEMORY_ALLOC_T *)&myAllocStruct;
    JUNO_MEMORY_T memDesc = { 0 };

    // Allocate 128 bytes of memory.
    if (Juno_MemoryGet(pAlloc, &memDesc, 128) != JUNO_STATUS_SUCCESS) {
        fprintf(stderr, "Custom Juno_MemoryGet failed.\n");
        return -1;
    }
    // Use the memory (store a sample string).
    strcpy((char *)memDesc.pvAddr, "Hello from Custom Allocator!");
    printf("%s\n", (char *)memDesc.pvAddr);

    // Update allocation to 256 bytes.
    if (Juno_MemoryUpdate(pAlloc, &memDesc, 256) != JUNO_STATUS_SUCCESS) {
        fprintf(stderr, "Custom Juno_MemoryUpdate failed.\n");
        return -1;
    }

    // Free the allocation.
    if (Juno_MemoryPut(pAlloc, &memDesc) != JUNO_STATUS_SUCCESS) {
        fprintf(stderr, "Custom Juno_MemoryPut failed.\n");
        return -1;
    }
    return 0;
}
````

### How to Integrate Your Custom Allocator

1. **Implement Your Custom Allocator Type:**  
   Define your own `JUNO_MEMORY_ALLOC_T` and any context information your allocator requires.

2. **Provide the Allocator Function Implementations:**  
   Implement the functions `Juno_MemoryGet`, `Juno_MemoryUpdate`, and `Juno_MemoryPut` exactly as they are prototyped in alloc.h. These functions will provide your custom memory management and can use any underlying mechanism (e.g., POSIX malloc, realloc, free).

3. **Link Downstream Code Unchanged:**  
   Downstream dependencies link against these functions via the alloc.h API, or implelement the function pointer API. They won’t need to know whether your implementation is custom or the default block-based allocator.

By using this approach, you can seamlessly substitute the default memory manager with your custom implementation while maintaining API compatibility for all dependent code.

## Additional Notes

- **Memory Alignment:**  
  When declaring memory blocks with `JUNO_MEMORY_BLOCK`, ensure that the data types are properly aligned for your target platform.

- **Extensibility:**  
  The API is designed to be extensible. If you wish to integrate custom memory allocation strategies, define `JUNO_CUSTOM_ALLOC` and provide your own implementation for the allocation union and associated operations.

- **Custom Allocator Behavior (JUNO_CUSTOM_ALLOC):**  
  When `JUNO_CUSTOM_ALLOC` is defined, the default union `JUNO_MEMORY_ALLOC_T` (which includes both `tHdr` and `tBlock`) is not provided. In this case, you must implement your own memory allocation structure and functions that adhere to the generic API. This flexibility allows for seamless integration with third-party allocators or completely custom memory management mechanisms.
