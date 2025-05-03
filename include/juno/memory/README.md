# Juno Memory Module

The Juno Memory Module provides a unified API for memory management in embedded systems. It supports both generic memory allocation and fixed-size block-based allocation. The design is tailored for embedded projects, ensuring flexibility, deterministic failure handling, and future extensibility.

## Getting Started

### Prerequisites
- Basic understanding of C programming
- LibJuno included in your build system (via CMake)

### Header Files Organization

The Juno Memory Module is organized into the following header files:

1. **memory_types.h** - Contains core type definitions
   - `JUNO_MEMORY_T` - Structure for memory allocation tracking
   - `JUNO_MEMORY_BLOCK_T` - Structure for block-based allocators
   - `JUNO_MEMORY_ALLOC_T` - Union for generic allocation types
   - Macros for memory block declaration (`JUNO_MEMORY_BLOCK`, `JUNO_MEMORY_BLOCK_METADATA`)
   - Reference counting macros (`JUNO_REF`, `JUNO_NEW_REF_FROM`, etc.)
   - Inline reference counting functions (`Juno_MemoryGetRef`, `Juno_MemoryPutRef`)

2. **memory.h** - Contains the main function declarations
   - `Juno_MemoryBlkInit` - Initialize a block allocator
   - `Juno_MemoryGet` - Allocate memory
   - `Juno_MemoryUpdate` - Resize memory allocation
   - `Juno_MemoryPut` - Free memory
   - `Juno_MemoryApi` - Get the API structure

3. **memory_api.h** - Contains the function pointer API structure
   - `JUNO_MEMORY_API_T` - Structure of function pointers for memory operations

These header files should be included :
```c
#include "juno/status.h"              // Required for status codes
#include "juno/memory/memory_types.h"  // Required for type definitions
#include "juno/memory/memory.h"        // Required for function declarations
```

If you're using the function pointer API, you should also include:
```c
#include "juno/memory/memory_api.h"   // Required for API structure
```

### Quick Start
Here's how to get started with the memory module in three steps:

1. **Include Headers**
   ```c
   #include "juno/memory/memory.h"
   #include "juno/memory/memory_types.h"
   #include "juno/status.h"  // For status return codes
   ```

2. **Declare Memory Blocks**
   ```c
   // Define your data structure
   typedef struct MY_DATA_TAG {
       int value;
       char text[32];
   } MY_DATA_T;
   
   // Create static arrays for memory blocks (10 blocks) and metadata
   JUNO_MEMORY_BLOCK(myMemoryPool, MY_DATA_T, 10);
   JUNO_MEMORY_BLOCK_METADATA(myMetadata, 10);
   ```

3. **Initialize and Use the Memory Manager**
   ```c
   #include <stdio.h>
   
   // Define a failure handler callback
   void MyFailureHandler(JUNO_STATUS_T tStatus, const char *pcCustomMessage, JUNO_USER_DATA_T *pvUserData) {
       (void)pvUserData;  // Unused in this example
       fprintf(stderr, "Memory Error (code %d): %s\n", tStatus, pcCustomMessage);
   }
   
   // Initialize the memory block manager
   JUNO_MEMORY_ALLOC_T memAlloc = {0};
   JUNO_STATUS_T status = Juno_MemoryBlkInit(
       &memAlloc.tBlock, 
       myMemoryPool, 
       myMetadata, 
       sizeof(MY_DATA_T), 
       10, 
       MyFailureHandler,   // Failure handler callback
       NULL                // No user data
   );
   
   if (status != JUNO_STATUS_SUCCESS) {
       // Failure handler will already have been called with the error
       return -1;
   }
   
   // Allocate memory
   JUNO_MEMORY_T memory = {0};
   status = Juno_MemoryGet(&memAlloc, &memory, sizeof(MY_DATA_T));
   if (status != JUNO_STATUS_SUCCESS) {
       // Failure handler will already have been called
       return -1;
   }
   
   // Use the memory
   MY_DATA_T* data = (MY_DATA_T*)memory.pvAddr;
   data->value = 42;
   
   // Free memory when done
   Juno_MemoryPut(&memAlloc, &memory);
   ```

---

## Overview

- **Generic Memory Allocation**  
  The generic API offers functions to:
  - **Allocate:** `Juno_MemoryGet` - Allocates a memory block
  - **Update:** `Juno_MemoryUpdate` - Updates an existing memory allocation (when using block allocation, it verifies that the new size fits within the fixed block size)
  - **Free:** `Juno_MemoryPut` - Releases memory back to the allocator

- **Block-Based Memory Allocation**  
  This implementation manages a pre-allocated, fixed-size memory pool. You declare the memory region and associated metadata with the provided macros:
  - `JUNO_MEMORY_BLOCK(name, type, length)` - Creates a static memory block array
  - `JUNO_MEMORY_BLOCK_METADATA(name, length)` - Creates the metadata array for tracking

  The block allocator is initialized using `Juno_MemoryBlkInit`, which sets up the memory block structure, free stack, block size, and an optional failure handler callback.

- **Reference Counting**  
  The module provides reference counting through:
  - `Juno_MemoryGetRef` - Increments reference count
  - `Juno_MemoryPutRef` - Decrements reference count
  - Convenience macros like `JUNO_REF()` and `JUNO_NEW_REF_FROM()`
  
  Reference counting prevents premature deallocation of shared memory resources.

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

- **JUNO_MEMORY_T**
  This structure represents an allocated memory block with:
  - `pvAddr` - Pointer to the allocated memory
  - `zSize` - Size of the allocation in bytes  
  - `iRefCount` - Reference count for tracking shared access

- **JUNO_MEMORY_BLOCK_T**  
  This structure holds details of block-based allocation such as:
  - `pvMemory` - Pointer to the memory area
  - `ptMetadata` - Pointer to the metadata array
  - `zTypeSize` - Block size in bytes
  - `zLength` - Total number of blocks
  - `zUsed` - Number of blocks currently allocated
  - `zFreed` - Number of blocks freed and available for reuse
  - A failure handler defined using the macro `DECLARE_FAILURE_HANDLER`

- **JUNO_MEMORY_ALLOC_T**  
  If `JUNO_CUSTOM_ALLOC` is not defined, this union is declared to accommodate multiple allocation types (currently block-based). If `JUNO_CUSTOM_ALLOC` is defined, you must provide your own implementation of allocation types.

## API Styles

LibJuno offers two ways to use its memory functionality:

1. **Function Pointer API**: Use `Juno_MemoryApi()` to get a struct of function pointers - enables more flexible designs
2. **Direct Function Calls**: Call functions like `Juno_MemoryGet()` directly - simple and straightforward

The function pointer API is preferred since it enables modularity through out your project.
If you need the performance and timing benefits of direct function calls, those are available
as well.


## How to Use

### 1. Declare Memory Blocks & Metadata

Define your memory blocks and metadata arrays using the supplied macros:

````c
#include "juno/memory/memory.h"
#include "juno/memory/memory_types.h"
#include "juno/status.h"

typedef struct MY_BLOCK_TAG {
    int id;
    char data[32];
} MY_BLOCK_T;

// Create static arrays for memory blocks and metadata
JUNO_MEMORY_BLOCK(myBlock, MY_BLOCK_T, 20);
JUNO_MEMORY_BLOCK_METADATA(myMetadata, 20);
````

### 2. Initialize the Block Allocator

Initialize the block-based memory allocator with its memory region, metadata, block size, and (optionally) a failure handler:

````c
#include <string.h>
#include <stdio.h>

// Callback function matching JUNO_FAILURE_HANDLER_T signature.
void MyFailureHandler(JUNO_STATUS_T tStatus, const char *pcCustomMessage, JUNO_USER_DATA_T *pvUserData) {
    (void)pvUserData;  // Unused in this example.
    fprintf(stderr, "Memory Error (code %d): %s\n", tStatus, pcCustomMessage);
}

int main(void) {
    JUNO_MEMORY_ALLOC_T memAlloc = {0};

    // Initialize block allocator; pass NULL for a failure handler if not needed.
    JUNO_STATUS_T status = Juno_MemoryBlkInit(
        &memAlloc.tBlock,
        myBlock,
        myMetadata,
        sizeof(MY_BLOCK_T),
        20,
        MyFailureHandler,    // Failure callback
        NULL     // No user data
    );
    // Failure handler is automatically called
    if (status) return -1;
    // Continue using the allocator...
    return 0;
}
````

### 3. Allocate, Update, and Free Memory Blocks

It is recommended to use the generic allocator interface (`JUNO_MEMORY_ALLOC_T`).

````c
#include <stdio.h>

// Failure handler callback
void MyFailureHandler(JUNO_STATUS_T tStatus, const char *pcCustomMessage, JUNO_USER_DATA_T *pvUserData) {
    (void)pvUserData;  // Unused in this example
    fprintf(stderr, "Memory Error (code %d): %s\n", tStatus, pcCustomMessage);
}

void useMemoryBlocks(JUNO_MEMORY_ALLOC_T *allocator) {
    // Now use the generic API functions
    JUNO_MEMORY_T memDesc = {0};
    JUNO_STATUS_T status = Juno_MemoryGet(allocator, &memDesc, sizeof(MY_BLOCK_T));
    
    if (status == JUNO_STATUS_SUCCESS && memDesc.pvAddr != NULL) {
        MY_BLOCK_T *pData = (MY_BLOCK_T *)memDesc.pvAddr;
        pData->id = 42;
        snprintf(pData->data, sizeof(pData->data), "Hello Juno!");

        printf("Allocated Block: ID %d, Data: %s\n", pData->id, pData->data);

        // Try updating the block; the update verifies that the new size does not exceed the fixed size.
        // Note: For block allocators, you can only update to a size <= the original block size
        status = Juno_MemoryUpdate(allocator, &memDesc, sizeof(MY_BLOCK_T));
        if (status != JUNO_STATUS_SUCCESS) {
            // Failure handler will already have been called
            return;
        }

        // Free the block when done with it
        status = Juno_MemoryPut(allocator, &memDesc);
        if (status != JUNO_STATUS_SUCCESS) {
            // Failure handler will already have been called
            return;
        }
    } else {
        // Failure handler will already have been called
        return;
    }
}

int main(void) {
    JUNO_MEMORY_ALLOC_T memAlloc = {0};
    
    // Initialize with a failure handler
    JUNO_STATUS_T status = Juno_MemoryBlkInit(
        &memAlloc.tBlock,
        myBlock,
        myMetadata,
        sizeof(MY_BLOCK_T),
        20,
        MyFailureHandler,    // Failure callback
        NULL                 // No user data
    );
    
    if (status != JUNO_STATUS_SUCCESS) {
        // Failure handler will already have been called
        return -1;
    }
    
    useMemoryBlocks(&memAlloc);
    return 0;
}
````

### 4. Using the Generic Memory API

You can initialize a `JUNO_MEMORY_ALLOC_T` directly using the block implementation. This allows you to use the generic allocator functions seamlessly:

````c
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

// Define a failure handler callback
void MyFailureHandler(JUNO_STATUS_T tStatus, const char *pcCustomMessage, JUNO_USER_DATA_T *pvUserData) {
    (void)pvUserData;  // Unused in this example
    fprintf(stderr, "Memory Error (code %d): %s\n", tStatus, pcCustomMessage);
}

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
        MyFailureHandler,    // Failure callback
        NULL                 // No user data
    );

    if (status != JUNO_STATUS_SUCCESS) {
        // Failure handler will already have been called
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
        // Note: For block allocators, you can only update to a size <= the original block size
        status = Juno_MemoryUpdate(&myAlloc, &memDesc, sizeof(MY_BLOCK_T));
        if (status != JUNO_STATUS_SUCCESS) {
            // Failure handler will already have been called
            return -1;
        }

        // Free the memory using the generic free API.
        status = Juno_MemoryPut(&myAlloc, &memDesc);
        if (status != JUNO_STATUS_SUCCESS) {
            // Failure handler will already have been called
            return -1;
        }
    } else {
        // Failure handler will already have been called
        return -1;
    }

    return 0;
}
````

### 5. Using Reference Counting

Reference counting allows multiple components to safely share memory without premature deallocation. This is essential in complex systems where different modules might need access to the same data.

#### Reference Counting Basics

1. **Initial Allocation**: When memory is allocated with `Juno_MemoryGet()`, its reference count is initialized to 1.
2. **Creating References**: Use `Juno_MemoryGetRef()` or the convenience macro `JUNO_NEW_REF_FROM()` to increment the reference count.
3. **Releasing References**: Use `Juno_MemoryPutRef()` to decrement the reference count when a component is done with the memory.
4. **Final Release**: When the last reference is released, the memory can be freed with `Juno_MemoryPut()`.

#### Visual Example of Reference Counting

```
                                  ┌─────────────────┐
                                  │ Memory Block    │
                                  │ pvAddr: 0x1000  │
 Initial Allocation               │ zSize: 128      │
 iRefCount = 1        ───────────►│ iRefCount: 1    │
                                  └─────────────────┘
                                          ▲
                                          │
                      ┌───────────────────┴───────────────────┐
                      │                                       │
            ┌─────────┴──────────┐               ┌────────────┴─────────┐
            │ Component A        │               │ Component B          │
            │ JUNO_NEW_REF_FROM()│               │ (Original reference) │
            └─────────┬──────────┘               └────────────┬─────────┘
                      │                                       │
                      ▼                                       ▼
             iRefCount becomes 2                    Memory is still usable
                      │                                       │
                      │                                       │
            ┌─────────┴──────────┐               ┌────────────┴─────────┐
            │ Juno_MemoryPutRef()│               │ Juno_MemoryPut()     │
            └─────────┬──────────┘               └────────────┬─────────┘
                      │                                       │
                      ▼                                       ▼
              Decrements count to 1            Memory is not freed because
                                              iRefCount > 0 (returns REF_IN_USE_ERROR)
```

#### Reference Counting Example Code

````c
#include <stdio.h>
#include <string.h>
#include "juno/memory/memory.h"
#include "juno/memory/memory_types.h"

// Define a data type to allocate
typedef struct MY_DATA_TAG {
    int id;
    char name[32];
} MY_DATA_T;

// Define a failure handler callback
void MyFailureHandler(JUNO_STATUS_T tStatus, const char *pcCustomMessage, JUNO_USER_DATA_T *pvUserData) {
    (void)pvUserData;  // Unused in this example
    fprintf(stderr, "Memory Error (code %d): %s\n", tStatus, pcCustomMessage);
}

// Create static memory arrays
JUNO_MEMORY_BLOCK(myMemoryPool, MY_DATA_T, 10);
JUNO_MEMORY_BLOCK_METADATA(myMetadata, 10);

// This function demonstrates shared access to memory
// It takes a reference to the memory and modifies the contents
void useSharedData(JUNO_MEMORY_T *ptMemory) {
    // Create a new reference to the shared memory
    JUNO_NEW_REF_FROM_PTR(ptMemory);
    
    // Access and modify the data through our reference
    MY_DATA_T *pSharedData = (MY_DATA_T *)JUNO_REF(ptMemory)->pvAddr;
    pSharedData->id += 10;  // Modify the ID
    strncpy(pSharedData->name, "Modified Resource", sizeof(pSharedData->name)-1);
    
    printf("In useSharedData: ID=%d, Name=%s\n", pSharedData->id, pSharedData->name);
    
    // Release our reference when done
    // Note: This decrements the reference count but doesn't free the memory
    // because the original reference in main() still exists
    Juno_MemoryPutRef(JUNO_REF(ptMemory));
}

int main(void) {
    // Initialize the memory block
    JUNO_MEMORY_ALLOC_T memAlloc = {0};
    JUNO_STATUS_T status = Juno_MemoryBlkInit(
        &memAlloc.tBlock, 
        myMemoryPool, 
        myMetadata, 
        sizeof(MY_DATA_T), 
        10, 
        MyFailureHandler,    // Failure handler callback
        NULL                 // No user data
    );
    
    if (status != JUNO_STATUS_SUCCESS) {
        // Failure handler will already have been called
        return -1;
    }
    
    // Allocate memory (reference count = 1)
    JUNO_MEMORY_T memory = {0};
    status = Juno_MemoryGet(&memAlloc, &memory, sizeof(MY_DATA_T));
    if (status != JUNO_STATUS_SUCCESS) {
        // Failure handler will already have been called
        return -1;
    }
    
    // Initialize the data
    MY_DATA_T *pData = (MY_DATA_T *)memory.pvAddr;
    pData->id = 42;
    strncpy(pData->name, "Shared Resource", sizeof(pData->name)-1);
    
    // Pass the memory to another component for use
    useSharedData(&memory);
    
    // We can still access the memory here because the reference count is still 1
    printf("Main function: ID=%d, Name=%s\n", pData->id, pData->name);
    
    // Final cleanup - this will actually free the memory
    // since this is the last reference
    status = Juno_MemoryPut(&memAlloc, &memory);
    if (status != JUNO_STATUS_SUCCESS) {
        // Failure handler will already have been called
        return -1;
    }
    
    return 0;
}
````

#### Important Rules for Reference Counting

1. **Always Match Get/Put**: For every `Juno_MemoryGetRef()` or `JUNO_NEW_REF_FROM()`, ensure there is a matching `Juno_MemoryPutRef()`.

2. **Check Return Values**: When calling `Juno_MemoryPut()`, check the return value. If it returns `JUNO_STATUS_REF_IN_USE_ERROR`, it means there are still outstanding references to the memory.

3. **Reference Count Debugging**: You can check the current reference count by examining the `iRefCount` field of the `JUNO_MEMORY_T` structure.

#### When to Use Reference Counting

Reference counting is particularly useful in these scenarios:

- **Shared Resources**: When multiple components need access to the same data
- **Asynchronous Operations**: When memory might be used by a callback or task after the initiator has completed
- **Data Structures**: When implementing data structures like linked lists or trees where nodes may be referenced from multiple places

### 6. Example: Using a Failure Handler

The failure handler enables deterministic error management. For example, the following code demonstrates a callback that logs errors to `stderr`:

````c
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
    // Initialize block allocator
    JUNO_MEMORY_ALLOC_T memAlloc = {0};
    JUNO_STATUS_T status = Juno_MemoryBlkInit(
        &memAlloc.tBlock,
        myBlock,
        myMetadata,
        sizeof(MY_BLOCK_T),
        10,
        myFailureHandler,  // Custom failure callback.
        NULL               // No additional user data.
    );

    if (status != JUNO_STATUS_SUCCESS) {
        return -1;
    }

    // Force an error scenario: attempt to allocate with a size larger than allowed.
    JUNO_MEMORY_T memDesc = {0};
    status = Juno_MemoryGet(&memAlloc, &memDesc, sizeof(MY_BLOCK_T) + 10);
    // This will trigger the failure handler with JUNO_STATUS_MEMALLOC_ERROR

    return 0;
}
````

### 7. Using the Function Pointer API

The Juno Memory Module exposes its operations through function pointer APIs, allowing downstream code to remain agnostic to the underlying allocation method. This design means that whether you're using the default block-based allocator or a fully custom implementation, the API remains the same.

The module provides a primary API structure:

- **Generic Memory API:**  
  Obtain this API using the function `Juno_MemoryApi()`. It provides function pointers for the generic operations:
  - `Get` – to allocate memory.
  - `Update` – to update memory size.
  - `Put` – to free memory.
  
Below is an example demonstrating how to use the generic API to allocate and free memory from a block-based allocator:

````c
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

// Define a failure handler callback
void MyFailureHandler(JUNO_STATUS_T tStatus, const char *pcCustomMessage, JUNO_USER_DATA_T *pvUserData) {
    (void)pvUserData;  // Unused in this example
    fprintf(stderr, "Memory API Error (code %d): %s\n", tStatus, pcCustomMessage);
}

// Declare static memory block and metadata using provided macros
JUNO_MEMORY_BLOCK(myBlock, MY_BLOCK_T, 20);
JUNO_MEMORY_BLOCK_METADATA(myMetadata, 20);

int main(void) {
    // Initialize the block-based allocator
    JUNO_MEMORY_ALLOC_T memAlloc = {0};
    JUNO_STATUS_T status = Juno_MemoryBlkInit(
        &memAlloc.tBlock,
        myBlock,
        myMetadata,
        sizeof(MY_BLOCK_T),
        20,
        MyFailureHandler,  // Failure handler callback
        NULL               // No user data
    );
    if (status != JUNO_STATUS_SUCCESS) {
        // The failure handler will already have been called
        return -1;
    }

    // Retrieve the generic memory API, which abstracts the underlying allocation method.
    const JUNO_MEMORY_API_T *pApi = Juno_MemoryApi();
    JUNO_MEMORY_T memDesc = {0};

    // Allocate a memory block using the generic Get API.
    status = pApi->Get(&memBlock, &memDesc, sizeof(MY_BLOCK_T));
    if (status != JUNO_STATUS_SUCCESS || memDesc.pvAddr == NULL) {
        // The failure handler will already have been called
        return -1;
    }

    // Use the allocated memory.
    MY_BLOCK_T *pBlock = (MY_BLOCK_T *)memDesc.pvAddr;
    pBlock->id = 100;
    strcpy(pBlock->data, "Using API function pointers");
    printf("Allocated Block: ID %d, Data: %s\n", pBlock->id, pBlock->data);

    // Update the memory block size using the API's Update function
    status = pApi->Update(&memBlock, &memDesc, sizeof(MY_BLOCK_T));
    if (status != JUNO_STATUS_SUCCESS) {
        // The failure handler will already have been called
        return -1;
    }

    // Free the allocated block using the generic Put API.
    status = pApi->Put(&memBlock, &memDesc);
    if (status != JUNO_STATUS_SUCCESS) {
        // The failure handler will already have been called
        return -1;
    }

    return 0;
}
````

In this example:

- We initialize a block-based allocator.
- We retrieve the generic API using `Juno_MemoryApi()`.
- We invoke the `Get`, `Update`, and `Put` functions through the API, which internally dispatch to the block-based implementation.

## Common Issues and Troubleshooting

| Issue | Possible Cause | Solution |
|-------|---------------|----------|
| `JUNO_STATUS_MEMALLOC_ERROR` | Memory pool is full | Increase the size of your memory block array |
| `JUNO_STATUS_MEMALLOC_ERROR` | Requested size exceeds block size | Ensure allocation size is <= the block size defined during initialization |
| `JUNO_STATUS_INVALID_SIZE_ERROR` | Attempted to allocate with size 0 | Specify a valid size > 0 for allocations |
| `JUNO_STATUS_MEMFREE_ERROR` | Attempting to free memory that was not allocated | Check for double-free issues in your code |
| `JUNO_STATUS_REF_IN_USE_ERROR` | Attempting to free memory that still has references | Ensure all references are properly released with `Juno_MemoryPutRef()` before calling `Juno_MemoryPut()` |
| `JUNO_STATUS_INVALID_REF_ERROR` | Attempting to decrement a reference count that is already zero | Check your reference counting logic |
| Reference count never reaches zero | Forgetting to call `Juno_MemoryPutRef()` | Audit your code to ensure every `GetRef` has a matching `PutRef` |
| Memory leak despite reference counting | Circular references | Redesign your data structure to avoid circular references |

## Custom Allocators

If you wish to use a completely custom memory allocation implementation (for example, to integrate with an external memory manager), you can define `JUNO_CUSTOM_ALLOC` and implement your own custom allocation type and functions that adhere to the generic API. This allows for seamless integration with third-party allocators or completely custom memory management mechanisms.

### How to Integrate Your Custom Allocator

1. **Define `JUNO_CUSTOM_ALLOC`:**  
   This prevents the default union type from being defined, allowing you to create your own type.

2. **Implement Your Custom Allocator Type:**  
   Define your own `JUNO_MEMORY_ALLOC_T` and any context information your allocator requires.

3. **Provide the Allocator Function Implementations:**  
   Implement the functions `Juno_MemoryGet`, `Juno_MemoryUpdate`, and `Juno_MemoryPut` exactly as they are prototyped in memory_api.h. These functions will provide your custom memory management and can use any underlying mechanism (e.g., POSIX malloc, realloc, free).

4. **Support Reference Counting:**  
   You must also implement the reference counting functions (`Juno_MemoryGetRef` and `Juno_MemoryPutRef`) to properly handle the `iRefCount` field in the `JUNO_MEMORY_T` structure.

5. **Implement Error Handling:**  
   Ensure your implementations return the appropriate error codes consistent with the standard implementation.

6. **Link Downstream Code Unchanged:**  
   Downstream dependencies link against these functions via the alloc.h API, or implement the function pointer API. They won't need to know whether your implementation is custom or the default block-based allocator.

By using this approach, you can seamlessly substitute the default memory manager with your custom implementation while maintaining API compatibility for all dependent code.

## Additional Notes

- **Memory Alignment:**  
  When declaring memory blocks with `JUNO_MEMORY_BLOCK`, ensure that the data types are properly aligned for your target platform.

- **Thread Safety:**  
  The current implementation is not thread-safe. If you need thread safety, you'll need to implement appropriate synchronization mechanisms in your custom allocator or around the API calls.

- **Error Handling:**  
  Always check return status codes from memory operations. The module uses the `JUNO_STATUS_T` enum defined in `juno/status.h` to indicate success or different types of failures.

- **Size Limitations:**  
  With block-based allocators, you cannot allocate or update to a size larger than the block size defined during initialization.

- **Zero Size Allocations:**  
  Attempting to allocate with a size of 0 will return `JUNO_STATUS_INVALID_SIZE_ERROR`.

- **Extensibility:**  
  The API is designed to be extensible. If you wish to integrate custom memory allocation strategies, define `JUNO_CUSTOM_ALLOC` and provide your own implementation for the allocation union and associated operations.

- **Custom Allocator Behavior (JUNO_CUSTOM_ALLOC):**  
  When `JUNO_CUSTOM_ALLOC` is defined, the default union `JUNO_MEMORY_ALLOC_T` (which includes both `tHdr` and `tBlock`) is not provided. In this case, you must implement your own memory allocation structure and functions that adhere to the generic API. This flexibility allows for seamless integration with third-party allocators or completely custom memory management mechanisms.
