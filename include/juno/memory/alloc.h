#ifndef JUNO_MEMORY_ALLOC_H
#define JUNO_MEMORY_ALLOC_H
#ifndef JUNO_MEMORY_CUSTOM_ALLOC
#include "juno/memory/memory_types.h"
#endif
#include "juno/status.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

