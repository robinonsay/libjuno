#ifndef JUNO_MEMORY_DIRECT_H
#define JUNO_MEMORY_DIRECT_H

#include "juno/memory/memory_api.h"
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
/// @param pvMetadata Pointer to an array for block metadata tracking.
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

/// @brief Generic memory allocation function.
/// Allocates memory by using the appropriate allocation method based on the allocation type.
/// @param ptMem Pointer to the memory allocation structure.
/// @param ptMemory Pointer to a memory descriptor where allocation details will be stored.
/// @return JUNO_STATUS_T Status of the allocation.
JUNO_STATUS_T Juno_MemoryGet(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *ptMemory, size_t zSize);

/// @brief Updates the current memory allocation to a new size (realloc).
/// @param ptMem Pointer to the memory allocator.
/// @param ptMemory The memory to update with a new size.
/// @param zNewSize The new size of the memory.
/// @return JUNO_STATUS_T Status of the allocation.
JUNO_STATUS_T Juno_MemoryUpdate(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *ptMemory, size_t zNewSize);

/// @brief Generic memory free function.
/// Releases memory back to the allocator and clears the memory descriptor.
/// @param ptMem Pointer to the memory allocation structure.
/// @param ptMemory Pointer to the memory descriptor to free.
/// @return JUNO_STATUS_T Status of the free operation.
JUNO_STATUS_T Juno_MemoryPut(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *ptMemory);


/// @brief Retrieves the generic memory API structure.
/// @return Pointer to the generic memory API structre.
const JUNO_MEMORY_API_T * Juno_MemoryApi(void);



#ifdef __cplusplus
}
#endif
#endif
