#ifndef JUNO_MEMORY_API_H
#define JUNO_MEMORY_API_H
#include "juno/status.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif


typedef struct JUNO_MEMORY_API_TAG JUNO_MEMORY_API_T;
typedef union JUNO_MEMORY_ALLOC_TAG JUNO_MEMORY_ALLOC_T;
typedef struct JUNO_MEMORY_TAG JUNO_MEMORY_T;
/// @brief Structure for an allocated memory segment.
/// Describes the allocated memory with a pointer to the start and its size.
struct JUNO_MEMORY_TAG
{
    /// Pointer to the allocated memory.
    void *pvAddr;
    /// Size of the allocated memory, in bytes.
    size_t zSize;
    /// The reference count for this memory
    size_t iRefCount;
};

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



#ifdef __cplusplus
}
#endif
#endif


