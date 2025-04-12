#ifndef JUNO_MEMORY_DIRECT_H
#define JUNO_MEMORY_DIRECT_H

#include "juno/memory/memory_types.h"
#include "juno/status.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Initializes a memory block for allocation.
/// Sets up a memory block with an associated free stack for managing fixed-size allocations.
/// @param ptMemBlk Pointer to the memory block structure to initialize.
/// @param pvMemory Pointer to the contiguous memory used for allocations.
/// @param pvMemoryFreeStack Pointer to an array of pointers used as the free stack.
/// @param zTypeSize Size in bytes of each element in the block.
/// @param zLength Total number of possible allocations.
/// @param pfcnFailureHandler Callback function to handle failures.
/// @param pvUserData User data passed to the failure handler.
/// @return JUNO_STATUS_T Status of the initialization.
JUNO_STATUS_T Juno_MemoryBlkInit(
    JUNO_MEMORY_BLOCK_T *ptMemBlk,
    void *pvMemory,
    JUNO_MEMORY_BLOCK_METADATA_T *pvMetadata,
    size_t zTypeSize,
    size_t zLength,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvUserData
);

/// @brief Allocates a block of memory.
/// Retrieves an available fixed-size memory block from the free stack or creates a new allocation if available.
/// @param ptMemBlk Pointer to the memory block structure.
/// @param pvRetAddr Address where the pointer to the allocated block will be stored.
/// @return JUNO_STATUS_T Status of the allocation attempt.
JUNO_STATUS_T Juno_MemoryBlkGet(JUNO_MEMORY_BLOCK_T *ptMemBlk, JUNO_MEMORY_T *pvRetAddr);

/// @brief Updates the current memory allocation to a new size (realloc).
/// @param ptMem Pointer to the memory allocator.
/// @param ptMemory The memory to update with a new size.
/// @param zNewSize The new size of the memory.
/// @return JUNO_STATUS_T Status of the allocation.
JUNO_STATUS_T Juno_MemoryBlkUpdate(JUNO_MEMORY_BLOCK_T *ptMem, JUNO_MEMORY_T *ptMemory, size_t zNewSize);

/// @brief Frees a previously allocated block of memory.
/// Returns a memory block back to the free stack so it can be reused.
/// @param ptMemBlk Pointer to the memory block structure.
/// @param pvAddr Pointer to the allocated block to be freed.
/// @return JUNO_STATUS_T Status of the free operation.
JUNO_STATUS_T Juno_MemoryBlkPut(JUNO_MEMORY_BLOCK_T *ptMemBlk, JUNO_MEMORY_T *pvAddr);
#ifdef __cplusplus
}
#endif
#endif
