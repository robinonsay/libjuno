#ifndef JUNO_MEMORY_API_H
#define JUNO_MEMORY_API_H
#include "juno/status.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "juno/memory/memory_types.h"

typedef struct JUNO_MEMORY_BLOCK_API_TAG JUNO_MEMORY_BLOCK_API_T;
typedef struct JUNO_MEMORY_API_TAG JUNO_MEMORY_API_T;

/// @brief API for generic memory allocation operations.
/// 
/// This structure holds pointers to functions that implement operations for
/// generic memory allocation including allocation, update, and free.
struct JUNO_MEMORY_API_TAG
{
    /// @brief Allocates memory using the specified memory allocation method.
    /// 
    /// @param ptMem Pointer to the memory allocation structure.
    /// @param pvRetAddr Pointer to a memory descriptor where allocation details will be stored.
    /// @param zSize Size of the memory block to allocate in bytes.
    /// @return JUNO_STATUS_T Status of the allocation operation.
    JUNO_STATUS_T (*Get)(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *pvRetAddr, size_t zSize);

    /// @brief Updates an existing memory allocation to a new size.
    /// 
    /// @param ptMem Pointer to the memory allocation structure.
    /// @param ptMemory Pointer to the memory descriptor to update.
    /// @param zNewSize The new size for the memory block.
    /// @return JUNO_STATUS_T Status of the update operation.
    JUNO_STATUS_T (*Update)(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *ptMemory, size_t zNewSize);

    /// @brief Frees an allocated memory block.
    /// 
    /// @param ptMem Pointer to the memory allocation structure.
    /// @param pvAddr Pointer to the memory block to free.
    /// @return JUNO_STATUS_T Status of the free operation.
    JUNO_STATUS_T (*Put)(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *pvAddr);
};

/// @brief API for block-based memory management operations.
/// 
/// This structure holds the initialization function for block-based memory management.
struct JUNO_MEMORY_BLOCK_API_TAG
{
    /// @brief Initializes a memory block for block-based allocation.
    /// 
    /// This function sets up a memory block with its associated free stack to support
    /// block-based memory allocations.
    /// 
    /// @param ptMemBlk Pointer to the memory block structure to initialize.
    /// @param pvMemory Pointer to the contiguous memory used for allocations.
    /// @param ptMetadata Pointer to the array of metadata for managing memory blocks.
    /// @param zTypeSize Size in bytes of each block element.
    /// @param zLength Total number of blocks available.
    /// @param pfcnFailureHandler Callback function to handle allocation failures.
    /// @param pvUserData User data passed to the failure handler.
    /// @return JUNO_STATUS_T Status of the initialization operation.
    JUNO_STATUS_T (*Init)(
        JUNO_MEMORY_BLOCK_T *ptMemBlk,
        void *pvMemory,
        JUNO_MEMORY_BLOCK_METADATA_T *ptMetadata,
        size_t zTypeSize,
        size_t zLength,
        JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
        JUNO_USER_DATA_T *pvUserData
    );
};

/// @brief Retrieves the block-based memory API structure.
/// @return Pointer to the memory block API structure.
const JUNO_MEMORY_BLOCK_API_T * Juno_MemoryBlkApi(void);

/// @brief Retrieves the generic memory API structure.
/// @return Pointer to the generic memory API structre.
const JUNO_MEMORY_API_T * Juno_MemoryApi(void);

#ifdef __cplusplus
}
#endif
#endif


