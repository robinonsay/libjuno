#ifndef JUNO_MEMORY_H
#define JUNO_MEMORY_H

#include <stddef.h>
#include <stdint.h>
#include "juno/status.h"
#include "stddef.h"
#include "juno/memory/memory_api.h"
#ifdef __cplusplus
extern "C" {
#endif

/// @brief Macro to declare a static memory block and its associated free stack.
/// @param name Name of the memory block.
/// @param type Data type of each block element.
/// @param length Number of elements in the memory block.
#define JUNO_MEMORY_BLOCK(name, type, length) \
static type name[length] = {};

/// @brief Macro to declare a static array for memory metadata.
/// @param name Name of the metadata array.
/// @param length Number of metadata entries.
#define JUNO_MEMORY_BLOCK_METADATA(name, length) \
static JUNO_MEMORY_BLOCK_METADATA_T name[length] = {};

#define JUNO_REF(name) REF##name
#define JUNO_NEW_REF(name) JUNO_MEMORY_T *JUNO_REF(name)

typedef struct JUNO_MEMORY_BLOCK_METADATA_TAG JUNO_MEMORY_BLOCK_METADATA_T;
typedef struct JUNO_MEMORY_BLOCK_TAG JUNO_MEMORY_BLOCK_T;
typedef struct JUNO_MEMORY_ALLOC_HDR_TAG JUNO_MEMORY_ALLOC_HDR_T;

#ifndef JUNO_CUSTOM_ALLOC
#endif

/// @brief Enumeration of memory allocation types.
typedef enum JUNO_MEMORY_ALLOC_TYPE_TAG
{
    JUNO_MEMORY_ALLOC_TYPE_RESERVED = 0, ///< Reserved allocation type.
    JUNO_MEMORY_ALLOC_TYPE_BLOCK    = 1, ///< Block-based memory allocation.
} JUNO_MEMORY_ALLOC_TYPE_T;

struct JUNO_MEMORY_BLOCK_METADATA_TAG
{
    uint8_t *ptFreeMem;
};

/// @brief Structure for memory allocation header.
/// Contains general information including the type of allocation.
struct JUNO_MEMORY_ALLOC_HDR_TAG
{
    JUNO_MEMORY_ALLOC_TYPE_T tType; ///< Type of memory allocation.
};





/// @brief Structure representing a block-based memory allocator.
/// Manages a fixed-size memory area along with associated free memory tracking.
struct JUNO_MEMORY_BLOCK_TAG
{
    JUNO_MEMORY_ALLOC_HDR_T tHdr;       ///< Header indicating the allocation type.
    uint8_t *pvMemory;                  ///< Pointer to the allocated memory area.
    JUNO_MEMORY_BLOCK_METADATA_T *ptMetadata; ///< Array of metadata for each block.
    size_t zTypeSize;                   ///< Size of each block element.
    size_t zLength;                     ///< Total number of blocks available.
    size_t zUsed;                       ///< Current count of allocated blocks.
    size_t zFreed;                      ///< Current count of freed blocks in the free stack.
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler;            ///< Macro to declare a failure handler.
    JUNO_USER_DATA_T *pvFailureUserData;
};

#ifndef JUNO_CUSTOM_ALLOC
/// @brief Union for a generic memory allocation.
/// Accommodates various allocation types, currently including block-based allocations.
union JUNO_MEMORY_ALLOC_TAG
{
    JUNO_MEMORY_ALLOC_HDR_T tHdr; ///< Common header for any allocation type.
    JUNO_MEMORY_BLOCK_T tBlock;   ///< Block-based allocation structure.
};
#endif

/// Get the reference to this juno memory
/// - This function will track the reference count to this memory
/// - The reference count is used to prevent freeing of used memory
/// - When using `JUNO_MEMORY_T` it is recommended to pass memory
///   around using this function to increment/decrement the reference count
/// @param ptMemory The memory to get the reference to
/// @return The reference to the memory
static inline JUNO_MEMORY_T * Juno_MemoryGetRef(JUNO_MEMORY_T *ptMemory)
{
    if(ptMemory->iRefCount)
    {
        ptMemory->iRefCount += 1;
    }
    return ptMemory;
}

/// Put the reference to this juno memory
/// - This function will track the reference count to this memory
/// - The reference count is used to prevent freeing of used memory
/// - When using `JUNO_MEMORY_T` it is recommended to pass memory
///   around using this function to increment /decrement the reference count
/// @param ptMemory The memory to put the reference away
/// @return The reference to the memory
static inline void Juno_MemoryPutRef(JUNO_MEMORY_T *ptMemory)
{
    if(ptMemory->iRefCount)
    {
        ptMemory->iRefCount -= 1;
    }
}
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

#ifdef __cplusplus
}
#endif
#endif

