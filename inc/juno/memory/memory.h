#ifndef JUNO_MEMORY_DIRECT_H
#define JUNO_MEMORY_DIRECT_H

#include "juno/memory/memory_types.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes a memory block for allocation.
 *
 * Sets up a memory block with an associated free stack for managing fixed-size allocations.
 *
 * @param ptMemBlk Pointer to the memory block structure to initialize.
 * @param pvMemory Pointer to the contiguous memory used for allocations.
 * @param pvMemoryFreeStack Pointer to an array of pointers used as the free stack.
 * @param zTypeSize Size in bytes of each element in the block.
 * @param zLength Total number of possible allocations.
 * @param pfcnFailureHandler Callback function to handle failures.
 * @param pvUserData User data passed to the failure handler.
 *
 * @return JUNO_STATUS_T Status of the initialization.
 */
JUNO_STATUS_T Juno_MemoryBlkInit(
    JUNO_MEMORY_BLOCK_T *ptMemBlk,
    void *pvMemory,
    uint8_t **pvMemoryFreeStack,
    size_t zTypeSize,
    size_t zLength,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvUserData
);

/**
 * @brief Allocates a block of memory.
 *
 * Retrieves an available fixed-size memory block from the free stack or creates a new allocation if available.
 *
 * @param ptMemBlk Pointer to the memory block structure.
 * @param pvRetAddr Address where the pointer to the allocated block will be stored.
 *
 * @return JUNO_STATUS_T Status of the allocation attempt.
 */
JUNO_STATUS_T Juno_MemoryBlkGet(JUNO_MEMORY_BLOCK_T *ptMemBlk, void **pvRetAddr);

/**
 * @brief Frees a previously allocated block of memory.
 *
 * Returns a memory block back to the free stack so it can be reused.
 *
 * @param ptMemBlk Pointer to the memory block structure.
 * @param pvAddr Pointer to the allocated block to be freed.
 *
 * @return JUNO_STATUS_T Status of the free operation.
 */
JUNO_STATUS_T Juno_MemoryBlkPut(JUNO_MEMORY_BLOCK_T *ptMemBlk, void *pvAddr);

/**
 * @brief Generic memory allocation function.
 *
 * Allocates memory by using the appropriate allocation method based on the allocation type.
 *
 * @param ptMem Pointer to the memory allocation structure.
 * @param ptMemory Pointer to a memory descriptor where allocation details will be stored.
 *
 * @return JUNO_STATUS_T Status of the allocation.
 */
JUNO_STATUS_T Juno_MemoryGet(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *ptMemory);

/**
 * @brief Generic memory free function.
 *
 * Releases memory back to the allocator and clears the memory descriptor.
 *
 * @param ptMem Pointer to the memory allocation structure.
 * @param ptMemory Pointer to the memory descriptor to free.
 *
 * @return JUNO_STATUS_T Status of the free operation.
 */
JUNO_STATUS_T Juno_MemoryPut(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *ptMemory);

#ifdef __cplusplus
}
#endif
#endif
