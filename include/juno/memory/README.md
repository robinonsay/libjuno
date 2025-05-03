# Juno Memory Module

The Juno Memory Module provides a deterministic, efficient memory management solution for embedded systems, designed to operate without dynamic memory allocation. This module is a core component of the LibJuno library, focused on memory safety and deterministic behavior in resource-constrained environments.

## Overview

The memory module implements a block-based memory allocation system with reference counting support, allowing for safe and controlled memory management. It is particularly suitable for embedded systems where dynamic memory allocation is undesirable due to fragmentation concerns, indeterministic behavior, or resource limitations.

## Memory Block Visualization

To help understand how memory blocks work in Juno, here's a visualization of the memory structure:

```
┌─────────────────────────────────────────┐
│            JUNO_MEMORY_BLOCK            │
│ ┌─────────┬─────────┬─────────┬───────┐ │
│ │ Block 0 │ Block 1 │ Block 2 │  ...  │ │
│ └─────────┴─────────┴─────────┴───────┘ │
└─────────────────────────────────────────┘
               Memory Array

┌─────────────────────────────────────────┐
│        JUNO_MEMORY_BLOCK_METADATA       │
│ ┌─────────┬─────────┬─────────┬───────┐ │
│ │ Meta 0  │ Meta 1  │ Meta 2  │  ...  │ │
│ └─────────┴─────────┴─────────┴───────┘ │
└─────────────────────────────────────────┘
              Metadata Array

┌───────────────────────────┐
│    JUNO_MEMORY_BLOCK_T    │
│                           │
│ - Type header             │
│ - Pointer to memory array │
│ - Pointer to metadata     │
│ - Type size               │
│ - Length                  │
│ - Used blocks counter     │
│ - Freed blocks counter    │
│ - Failure handler         │
└───────────────────────────┘
     Block Control Structure
```

When calling `Juno_MemoryBlkInit()`:
1. You pass in a pre-allocated memory array (`JUNO_MEMORY_BLOCK`)
2. You pass in a pre-allocated metadata array (`JUNO_MEMORY_BLOCK_METADATA`)
3. The function initializes the control structure that tracks:
   - Which blocks are in use
   - Which blocks were freed and can be reused
   - How to handle errors

## Getting Started

This quick-start guide will help you understand and use the Juno Memory Module in a few simple steps:

### Step 1: Declare Memory Blocks

First, declare a memory block and its metadata for your data type:

```c
// Define your data structure
typedef struct {
    int id;
    float value;
} MY_DATA_T;

// Declare a memory block with space for 10 MY_DATA_T objects
JUNO_MEMORY_BLOCK(gptMyMemoryBlock, MY_DATA_T, 10);
JUNO_MEMORY_BLOCK_METADATA(gptMyMemoryMetadata, 10);
```

### Step 2: Initialize the Memory Allocator

Create and initialize a memory allocator to manage the block:

```c
// Create the memory allocator structure
JUNO_MEMORY_ALLOC_T tMemAlloc = {0};

// Initialize the block allocator
JUNO_STATUS_T tStatus = Juno_MemoryBlkInit(
    &tMemAlloc.tBlock,        // Pointer to memory block structure
    gptMyMemoryBlock,            // Memory block array
    gptMyMemoryMetadata,         // Metadata array
    sizeof(MY_DATA_T),        // Size of each element
    10,                       // Number of elements
    pfcnErrorHandler,         // Optional error handler function
    NULL                      // Optional user data for error handler
);

// Always check the status
if(tStatus != JUNO_STATUS_SUCCESS) {
    // Handle initialization error
    return -1;
}
```

### Step 3: Allocate Memory

Allocate memory for your data type:

```c
// Declare a memory descriptor
JUNO_MEMORY_T tMemory = {0};

// Allocate memory
tStatus = Juno_MemoryGet(&tMemAlloc, &tMemory, sizeof(MY_DATA_T));
if(tStatus != JUNO_STATUS_SUCCESS) {
    // Handle allocation error
    return -1;
}

// Use the allocated memory
MY_DATA_T *ptData = (MY_DATA_T*)tMemory.pvAddr;
ptData->id = 1;
ptData->value = 3.14f;
```

### Step 4: Share Memory with Reference Counting

Use reference counting to safely share memory:

```c
// Create a reference to the memory
JUNO_MEMORY_T *ptMemoryRef = Juno_MemoryGetRef(&tMemory);

// Or using the helper macros for cleaner code
JUNO_NEW_REF(memoryRef) = Juno_MemoryGetRef(&tMemory);

// Now you can use the reference elsewhere
MY_DATA_T *ptSharedData = (MY_DATA_T *)JUNO_REF(memoryRef)->pvAddr;

// Check reference count
printf("Reference count: %zu\n", tMemory.iRefCount); // Should be 2
```

### Step 5: Release Memory

Release memory when you're done with it:

```c
// First, release any references
Juno_MemoryPutRef(JUNO_REF(memoryRef));

// Check that the reference was released
printf("Reference count: %zu\n", tMemory.iRefCount); // Should be 1

// Now free the original memory
tStatus = Juno_MemoryPut(&tMemAlloc, &tMemory);
if(tStatus != JUNO_STATUS_SUCCESS) {
    // Handle error (could be JUNO_STATUS_REF_IN_USE_ERROR if references exist)
    printf("Error: Cannot free memory with active references\n");
}
```

### Step 6: Using the Memory API (Alternative Approach)

For a more dynamic approach, you can use the Memory API interface:

```c
// Get the memory API
const JUNO_MEMORY_API_T *ptMemApi = Juno_MemoryApi();

// Use the API for operations
JUNO_MEMORY_T tMemory = {0};
tStatus = ptMemApi->Get(&tMemAlloc, &tMemory, sizeof(MY_DATA_T));

// ... use the memory ...

// Release using the API
tStatus = ptMemApi->Put(&tMemAlloc, &tMemory);
```

### Common Pitfalls to Avoid

1. **Not checking status codes**: Always check return values for errors.
2. **Freeing memory with active references**: This will fail with `JUNO_STATUS_REF_IN_USE_ERROR`.
3. **Not initializing memory structures**: Always initialize `JUNO_MEMORY_T` with `{}` before passing to functions.
4. **Using the wrong size**: Always pass the correct size for your data type.

## Features

- **Block-based memory allocation**: Pre-allocated memory blocks for deterministic behavior
- **Reference counting**: Prevents premature deallocation of shared memory resources
- **Type-safe allocation**: Memory blocks are typed for better memory safety
- **Failure handling**: Comprehensive error reporting through configurable failure handlers
- **No heap fragmentation**: All memory is pre-allocated, eliminating fragmentation concerns
- **Minimal dependencies**: Designed to work without the standard C library when necessary

## Core Concepts

### Memory Blocks

Memory blocks are the foundation of Juno's memory management. These are statically allocated arrays of a specific type and size, declared using macros:

```c
// Declare a memory block for up to 100 elements of type MY_STRUCT_T
JUNO_MEMORY_BLOCK(gptMyMemoryBlock, MY_STRUCT_T, 100);

// Declare metadata for tracking freed memory blocks
JUNO_MEMORY_BLOCK_METADATA(gptMyMemoryMetadata, 100);
```

### Memory Allocation

Memory is allocated from a pre-defined block using the `Juno_MemoryGet` function or through the memory API:

```c
// Allocate memory
JUNO_MEMORY_T tMemory = {0};
JUNO_STATUS_T tStatus = Juno_MemoryGet(&tMemAlloc, &tMemory, sizeof(MY_STRUCT_T));
```

### Reference Counting

The module provides reference counting to safely share memory between different components:

```c
// Get a reference to existing memory
JUNO_MEMORY_T *ptMemoryRef = Juno_MemoryGetRef(&tExistingMemory);

// Release a reference when done
Juno_MemoryPutRef(ptMemoryRef);
```

### Memory Deallocation

Memory is returned to the allocator using the `Juno_MemoryPut` function when it's no longer needed:

```c
// Free memory
JUNO_STATUS_T tStatus = Juno_MemoryPut(&tMemAlloc, &tMemory);
```

## Data Types

### JUNO_MEMORY_T

The fundamental structure representing an allocated memory segment:

```c
struct JUNO_MEMORY_TAG
{
    void *pvAddr;      // Pointer to the allocated memory
    size_t zSize;      // Size of the allocated memory in bytes
    size_t iRefCount;  // Reference count for this memory
};
```

### JUNO_MEMORY_BLOCK_T

Structure representing a block-based memory allocator:

```c
struct JUNO_MEMORY_BLOCK_TAG
{
    JUNO_MEMORY_ALLOC_HDR_T tHdr;               // Allocation type header
    uint8_t *pvMemory;                         // Pointer to the memory area
    JUNO_MEMORY_BLOCK_METADATA_T *ptMetadata;  // Metadata for block tracking
    size_t zTypeSize;                          // Size of each block element
    size_t zLength;                            // Total number of blocks
    size_t zUsed;                              // Count of allocated blocks
    size_t zFreed;                             // Count of freed blocks
    // Failure handler function pointer and user data
};
```

### JUNO_MEMORY_ALLOC_T

Union for a generic memory allocation that currently supports block-based allocation:

```c
union JUNO_MEMORY_ALLOC_TAG
{
    JUNO_MEMORY_ALLOC_HDR_T tHdr;  // Common header
    JUNO_MEMORY_BLOCK_T tBlock;    // Block-based allocation structure
};
```

## API Reference

### Initialization

```c
JUNO_STATUS_T Juno_MemoryBlkInit(
    JUNO_MEMORY_BLOCK_T *ptMemBlk,
    void *pvMemory,
    JUNO_MEMORY_BLOCK_METADATA_T *pvMetadata,
    size_t zTypeSize,
    size_t zLength,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvUserData
);
```

Initializes a memory block for allocation. Parameters:
- `ptMemBlk`: Pointer to the memory block structure to initialize
- `pvMemory`: Pointer to the pre-allocated memory used for allocations
- `pvMetadata`: Pointer to the metadata array for tracking memory blocks
- `zTypeSize`: Size in bytes of each element in the block
- `zLength`: Total number of elements in the block
- `pfcnFailureHandler`: Optional callback function to handle failures
- `pvUserData`: Optional user data passed to the failure handler

### Memory Operations

```c
JUNO_STATUS_T Juno_MemoryGet(
    JUNO_MEMORY_ALLOC_T *ptMem,
    JUNO_MEMORY_T *ptMemory,
    size_t zSize
);
```

Allocates memory from the specified allocator. Parameters:
- `ptMem`: Pointer to the memory allocation structure
- `ptMemory`: Pointer to store allocation details
- `zSize`: Size of memory to allocate in bytes

```c
JUNO_STATUS_T Juno_MemoryUpdate(
    JUNO_MEMORY_ALLOC_T *ptMem,
    JUNO_MEMORY_T *ptMemory,
    size_t zNewSize
);
```

Updates an existing memory allocation to a new size. Parameters:
- `ptMem`: Pointer to the memory allocator
- `ptMemory`: The memory to update
- `zNewSize`: The new size for the memory

```c
JUNO_STATUS_T Juno_MemoryPut(
    JUNO_MEMORY_ALLOC_T *ptMem,
    JUNO_MEMORY_T *ptMemory
);
```

Releases memory back to the allocator. Parameters:
- `ptMem`: Pointer to the memory allocation structure
- `ptMemory`: Pointer to the memory to free

### Reference Management

```c
JUNO_MEMORY_T* Juno_MemoryGetRef(JUNO_MEMORY_T *ptMemory);
```

Gets a reference to memory, incrementing its reference count. Parameters:
- `ptMemory`: The memory to reference
- Returns: The same memory pointer with increased reference count

```c
void Juno_MemoryPutRef(JUNO_MEMORY_T *ptMemory);
```

Releases a reference to memory, decrementing its reference count. Parameters:
- `ptMemory`: The memory reference to release

### API Access

```c
const JUNO_MEMORY_API_T* Juno_MemoryApi(void);
```

Retrieves the memory API structure for function pointer-based access.

## Usage Example

The following example demonstrates how to use the Juno Memory Module to implement a simple single linked list with reference counting:

```c
#include "juno/memory/memory.h"
#include "juno/memory/memory_api.h"
#include "juno/memory/memory_types.h"
#include "juno/status.h"
#include <stdio.h>

// Define linked list node structure
typedef struct SINGLE_LINKED_LIST_NODE_TAG {
    struct SINGLE_LINKED_LIST_NODE_TAG *ptNext;
    JUNO_MEMORY_T tMemory;
    int iData;
} SINGLE_LINKED_LIST_NODE_T;

// Define linked list structure
typedef struct {
    SINGLE_LINKED_LIST_NODE_T *ptHead;
    size_t zLength;
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler;
    JUNO_USER_DATA_T *pvUserData;
} SINGLE_LINKED_LIST_T;

// Declare memory blocks for our linked list nodes (max 100)
JUNO_MEMORY_BLOCK(nodeMemory, SINGLE_LINKED_LIST_NODE_T, 100);
JUNO_MEMORY_BLOCK_METADATA(nodeMetadata, 100);

// Custom failure handler
void FailureHandler(JUNO_STATUS_T tStatus, const char *pcMsg, JUNO_USER_DATA_T *pvUserData) {
    printf("Error: %s\n", pcMsg);
}

int main() {
    // Get the memory API
    const JUNO_MEMORY_API_T *ptMemApi = Juno_MemoryApi();
    
    // Initialize the memory allocator
    JUNO_MEMORY_ALLOC_T tMemAlloc = {};
    JUNO_STATUS_T tStatus = Juno_MemoryBlkInit(
        &tMemAlloc.tBlock,
        nodeMemory,
        nodeMetadata,
        sizeof(SINGLE_LINKED_LIST_NODE_T),
        100,
        FailureHandler,
        NULL
    );
    
    if(tStatus != JUNO_STATUS_SUCCESS) {
        return -1;
    }
    
    // Create a linked list
    SINGLE_LINKED_LIST_T tList = {
        .pfcnFailureHandler = FailureHandler
    };
    
    // Add some nodes
    for(int i = 0; i < 5; i++) {
        // Allocate memory for new node
        JUNO_MEMORY_T tNodeMem = {};
        tStatus = ptMemApi->Get(&tMemAlloc, &tNodeMem, sizeof(SINGLE_LINKED_LIST_NODE_T));
        if(tStatus != JUNO_STATUS_SUCCESS) {
            break;
        }
        
        // Cast memory to node type and initialize
        SINGLE_LINKED_LIST_NODE_T *ptNewNode = (SINGLE_LINKED_LIST_NODE_T*)tNodeMem.pvAddr;
        ptNewNode->iData = i;
        ptNewNode->tMemory = tNodeMem;
        
        // Insert at head
        ptNewNode->ptNext = tList.ptHead;
        tList.ptHead = ptNewNode;
        tList.zLength++;
    }
    
    // Create a second list that shares nodes by reference
    SINGLE_LINKED_LIST_T tRefList = {
        .pfcnFailureHandler = FailureHandler
    };
    
    // Share the first node by reference
    if(tList.ptHead) {
        JUNO_NEW_REF(nodeRef) = Juno_MemoryGetRef(&tList.ptHead->tMemory);
        tRefList.ptHead = (SINGLE_LINKED_LIST_NODE_T*)JUNO_REF(nodeRef)->pvAddr;
        tRefList.zLength = tList.zLength;
    }
    
    // Print both lists
    printf("Original list:\n");
    SINGLE_LINKED_LIST_NODE_T *ptCurrentNode = tList.ptHead;
    while(ptCurrentNode) {
        printf("Node data: %d (ref count: %lu)\n", 
               ptCurrentNode->iData, 
               ptCurrentNode->tMemory.iRefCount);
        ptCurrentNode = ptCurrentNode->ptNext;
    }
    
    // Clean up - this will fail for the first node due to reference counting
    printf("\nAttempting to free the original list:\n");
    while(tList.ptHead) {
        SINGLE_LINKED_LIST_NODE_T *ptTemp = tList.ptHead;
        tList.ptHead = tList.ptHead->ptNext;
        
        tStatus = ptMemApi->Put(&tMemAlloc, &ptTemp->tMemory);
        if(tStatus == JUNO_STATUS_REF_IN_USE_ERROR) {
            printf("Node still has references, can't free yet\n");
        }
    }
    
    // Release reference and try again
    printf("\nReleasing reference and freeing memory:\n");
    if(tRefList.ptHead) {
        Juno_MemoryPutRef(&tRefList.ptHead->tMemory);
        tStatus = ptMemApi->Put(&tMemAlloc, &tRefList.ptHead->tMemory);
        if(tStatus == JUNO_STATUS_SUCCESS) {
            printf("Memory successfully freed\n");
        }
    }
    
    return 0;
}
```

## Error Handling

The memory module reports errors through status codes of type `JUNO_STATUS_T`:

- `JUNO_STATUS_SUCCESS`: Operation completed successfully
- `JUNO_STATUS_MEMALLOC_ERROR`: Failed to allocate memory (block is full)
- `JUNO_STATUS_MEMFREE_ERROR`: Failed to free memory (invalid address or double free)
- `JUNO_STATUS_NULLPTR_ERROR`: Null pointer provided to function
- `JUNO_STATUS_INVALID_REF_ERROR`: Invalid memory reference
- `JUNO_STATUS_REF_IN_USE_ERROR`: Memory cannot be freed because references exist
- `JUNO_STATUS_INVALID_TYPE_ERROR`: Invalid memory allocator type
- `JUNO_STATUS_INVALID_SIZE_ERROR`: Invalid size parameter (e.g., zero size)

## Best Practices

1. **Always check status codes**: Memory operations can fail, especially in resource-constrained environments.
2. **Use reference counting**: When sharing memory between components, use the reference counting API.
3. **Define appropriate block sizes**: Size your memory blocks based on your application's needs to avoid waste.
4. **Implement failure handlers**: Use failure handlers to catch memory issues early.
5. **Clear returned memory**: Although the allocator zeros memory blocks, initialize your data structures explicitly.

## Understanding Reference Counting Macros

The reference counting macros (`JUNO_REF` and `JUNO_NEW_REF`) can be confusing at first. Here's a detailed explanation of how they work:

### JUNO_NEW_REF Macro

The `JUNO_NEW_REF` macro creates a new pointer to hold a reference to a memory object:

```c
// This expands to: JUNO_MEMORY_T *REFmyMemoryRef
JUNO_NEW_REF(myMemoryRef) = Juno_MemoryGetRef(&originalMemory);
```

This is equivalent to:

```c
JUNO_MEMORY_T *REFmyMemoryRef = Juno_MemoryGetRef(&originalMemory);
```

### JUNO_REF Macro

The `JUNO_REF` macro is used to access the reference you created with `JUNO_NEW_REF`:

```c
// This expands to: REFmyMemoryRef
MY_DATA_T *data = (MY_DATA_T *)JUNO_REF(myMemoryRef)->pvAddr;
```

This is equivalent to:

```c
MY_DATA_T *data = (MY_DATA_T *)REFmyMemoryRef->pvAddr;
```

### Example Usage Pattern

Here's a complete example of using these macros:

```c
// Original memory allocation
JUNO_MEMORY_T tMemory = {0};
tStatus = Juno_MemoryGet(&tMemAlloc, &tMemory, sizeof(MY_DATA_T));

// Create a named reference using the macro
JUNO_NEW_REF(dataRef) = Juno_MemoryGetRef(&tMemory);
// Now REFdataRef points to the same memory as tMemory, and the reference count is 2

// Access the reference using the convenience macro
MY_DATA_T *ptSharedData = (MY_DATA_T *)JUNO_REF(dataRef)->pvAddr;
// This is equivalent to: MY_DATA_T *ptSharedData = (MY_DATA_T *)REFdataRef->pvAddr;

// Release the reference when done
Juno_MemoryPutRef(JUNO_REF(dataRef));
// This is equivalent to: Juno_MemoryPutRef(REFdataRef);
```

### When to Use Reference Counting

Use reference counting when:
1. Sharing memory between multiple components
2. Passing memory to a function that does not need ownership
3. Storing memory in a data structure outside the original allocation scope

Reference counting prevents memory from being freed while it's still in use, avoiding a common class of memory errors.
