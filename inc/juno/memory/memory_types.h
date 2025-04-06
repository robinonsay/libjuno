#ifndef JUNO_MEMORY_H
#define JUNO_MEMORY_H

#include "juno/macros.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Macro to define a free memory stack name.
 *
 * Usage: MEMORY_FREE_STACK(myStack) expands to pzmyStackFreeStack.
 */
#define MEMORY_FREE_STACK(name) pz##name##FreeStack

/**
 * @brief Macro to declare a static memory block and its associated free stack.
 *
 * @param name Name of the memory block.
 * @param type Data type of each block element.
 * @param length Number of elements in the memory block.
 */
#define MEMORY_BLOCK(name, type, length) \
static type name[length] = {0}; \
static uint8_t* MEMORY_FREE_STACK(name)[length] = {0}

/**
 * @brief Opaque type representing a memory block structure.
 */
typedef struct JUNO_MEMORY_BLOCK_TAG JUNO_MEMORY_BLOCK_T;

/**
 * @brief Opaque type representing a memory allocation header.
 */
typedef struct JUNO_MEMORY_ALLOC_HDR_TAG JUNO_MEMORY_ALLOC_HDR_T;

/**
 * @brief Structure describing an allocated memory segment.
 *
 * Contains details such as the pointer to the allocated address and its size.
 */
typedef struct JUNO_MEMORY_TAG JUNO_MEMORY_T;

/**
 * @brief Union representing a generic memory allocation.
 */
typedef union JUNO_MEMORY_ALLOC_TAG JUNO_MEMORY_ALLOC_T;

/**
 * @brief Enumeration of memory allocation types.
 */
typedef enum JUNO_MEMORY_ALLOC_TYPE_TAG
{
    JUNO_MEMORY_ALLOC_TYPE_RESERVED = 0, /**< Reserved allocation type. */
    JUNO_MEMORY_ALLOC_TYPE_BLOCK    = 1, /**< Block-based memory allocation. */
} JUNO_MEMORY_ALLOC_TYPE_T;

/**
 * @brief Structure for memory allocation header.
 *
 * Contains general information including the type of allocation.
 */
struct JUNO_MEMORY_ALLOC_HDR_TAG
{
    JUNO_MEMORY_ALLOC_TYPE_T tType; /**< Type of memory allocation. */
};

/**
 * @brief Structure for an allocated memory segment.
 *
 * Describes the allocated memory with a pointer to the start and its size.
 */
struct JUNO_MEMORY_TAG
{
    void *pvAddr; /**< Pointer to the allocated memory. */
    size_t zSize; /**< Size of the allocated memory, in bytes. */
};

/**
 * @brief Structure representing a block-based memory allocator.
 *
 * Manages a fixed-size memory area along with associated free memory tracking.
 */
struct JUNO_MEMORY_BLOCK_TAG
{
    JUNO_MEMORY_ALLOC_HDR_T tHdr;      /**< Header indicating the allocation type. */
    uint8_t *pvMemory;                 /**< Pointer to the allocated memory area. */
    uint8_t **pvMemoryFreeStack;       /**< Array of pointers serving as a free stack. */
    size_t zTypeSize;                  /**< Size of each block element. */
    size_t zLength;                    /**< Total number of blocks available. */
    size_t zUsed;                      /**< Current count of allocated blocks. */
    size_t zFreed;                     /**< Current count of freed blocks in the free stack. */
    DECLARE_FAILURE_HANDLER;           /**< Macro to declare a failure handler. */
};

/**
 * @brief Union for a generic memory allocation.
 *
 * Accommodates various allocation types, currently including block-based allocations.
 */
union JUNO_MEMORY_ALLOC_TAG
{
    JUNO_MEMORY_ALLOC_HDR_T tHdr; /**< Common header for any allocation type. */
    JUNO_MEMORY_BLOCK_T tBlock;   /**< Block-based allocation structure. */
};

#ifdef __cplusplus
}
#endif
#endif

