/*
    MIT License

    Copyright (c) Year Robin A. Onsay

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.
*/
#ifndef JUNO_MEMORY_H
#define JUNO_MEMORY_H

#include <stddef.h>
#include <stdint.h>
#include "juno/status.h"
#include "stddef.h"

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
typedef struct JUNO_MEMORY_TAG JUNO_MEMORY_T;

#ifndef JUNO_CUSTOM_ALLOC
typedef union JUNO_MEMORY_ALLOC_TAG JUNO_MEMORY_ALLOC_T;
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

#ifdef __cplusplus
}
#endif
#endif

