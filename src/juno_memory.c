#include "juno/macros.h"
#include "juno/memory/memory.h"
#include "juno/memory/memory_api.h"
#include "juno/memory/memory_types.h"
#include "juno/status.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief Validates the essential members of a memory block structure.
 *
 * Checks that the pointer to the memory block, its memory area, the free stack,
 * length of the block, and size of each element are valid.
 *
 * @param ptMemBlk Pointer to the memory block structure.
 * @return JUNO_STATUS_T Status indicating success or failure.
 */
static JUNO_STATUS_T Juno_MemoryBlkValidate(JUNO_MEMORY_BLOCK_T *ptMemBlk)
{
    // Ensure that the memory block structure and its key members exist.
    ASSERT_EXISTS((ptMemBlk && ptMemBlk->pvMemory && ptMemBlk->pvMemoryFreeStack && ptMemBlk->zLength && ptMemBlk->zTypeSize));
    return JUNO_STATUS_SUCCESS;
}

/**
 * @brief Initializes a block-based memory allocator.
 *
 * Sets up the memory structure with a pointer to the memory area, free stack, and
 * initializes counters for used and freed blocks. Also clears the memory.
 *
 * @param ptMemBlk Pointer to the memory block structure to initialize.
 * @param pvMemory Pointer to the contiguous memory area.
 * @param pvMemoryFreeStack Pointer to the free stack array.
 * @param zTypeSize Size of each block element.
 * @param zLength Total number of blocks.
 * @param pfcnFailureHandler Failure handler callback.
 * @param pvUserData Pointer to user data for the failure handler.
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
)
{
    // Validate input pointer
    ASSERT_EXISTS(ptMemBlk);
    // Set the allocation type to block allocation
    ptMemBlk->tHdr.tType = JUNO_MEMORY_ALLOC_TYPE_BLOCK;
    // Initialize memory area and free stack pointer
    ptMemBlk->pvMemory = (uint8_t *) pvMemory;
    ptMemBlk->pvMemoryFreeStack = pvMemoryFreeStack;
    // Set type size and total number of blocks
    ptMemBlk->zTypeSize = zTypeSize;
    ptMemBlk->zLength = zLength;
    // No block is in use yet
    ptMemBlk->zUsed = 0;
    // Set the failure handler and its user data
    ptMemBlk->pfcnFailureHandler = pfcnFailureHandler;
    ptMemBlk->pvUserData = pvUserData;
    // Initially, no freed blocks are available
    ptMemBlk->zFreed = 0;
    // Clear the memory area
    memset(pvMemory, 0, zTypeSize * zLength);
    return Juno_MemoryBlkValidate(ptMemBlk);
}

/**
 * @brief Retrieves a free block of memory.
 *
 * If no block has been freed, the next block from the memory area is prepared.
 * Then, the latest free block is returned and removed from the free stack.
 *
 * @param ptMemBlk Pointer to the memory block structure.
 * @param pvRetAddr Pointer where the address of the allocated block is stored.
 * @return JUNO_STATUS_T Status of the allocation attempt.
 */
JUNO_STATUS_T Juno_MemoryBlkGet(JUNO_MEMORY_BLOCK_T *ptMemBlk, void **pvRetAddr)
{
    // Validate the memory block structure
    JUNO_STATUS_T tStatus = Juno_MemoryBlkValidate(ptMemBlk);
    ASSERT_SUCCESS(tStatus, return tStatus);
    
    // Check if there is room for allocation (either new or from freed list)
    if(!(ptMemBlk->zUsed < ptMemBlk->zLength || ptMemBlk->zFreed))
    {
        tStatus = JUNO_STATUS_MEMALLOC_ERROR;
        // Log error through the failure handler
        FAIL(tStatus, ptMemBlk->pfcnFailureHandler, ptMemBlk->pvUserData,
            "Failed to allocate block memory. Memory is full: %lu", ptMemBlk->zLength
        );
        return tStatus;
    }
    
    // If no freed block is available, allocate a new block
    if(ptMemBlk->zFreed == 0)
    {
        // Compute the offset for the next available block
        size_t zNextFreeBlock = ptMemBlk->zUsed * ptMemBlk->zTypeSize;
        // Place the new block on the free stack
        ptMemBlk->pvMemoryFreeStack[ptMemBlk->zFreed] = &ptMemBlk->pvMemory[zNextFreeBlock];
        ptMemBlk->zFreed += 1;
        // Increment the used block counter
        ptMemBlk->zUsed += 1;
    }
    // Retrieve the latest free block and update free stack
    *pvRetAddr = ptMemBlk->pvMemoryFreeStack[ptMemBlk->zFreed-1];
    ptMemBlk->zFreed -= 1;
    ptMemBlk->pvMemoryFreeStack[ptMemBlk->zFreed] = NULL;
    return tStatus;
}

/**
 * @brief Frees a previously allocated block.
 *
 * Validates the address and ensures it hasn't already been freed. If the block to
 * be freed is the last allocated block, the used counter is decremented. Otherwise,
 * the block is added back to the free stack.
 *
 * @param ptMemBlk Pointer to the memory block structure.
 * @param pvAddr Pointer to the block to free.
 * @return JUNO_STATUS_T Status of the free operation.
 */
JUNO_STATUS_T Juno_MemoryBlkPut(JUNO_MEMORY_BLOCK_T *ptMemBlk, void *pvAddr)
{
    // Validate the memory block structure
    JUNO_STATUS_T tStatus = Juno_MemoryBlkValidate(ptMemBlk);
    ASSERT_SUCCESS(tStatus, return tStatus);
    
    // Calculate start and end addresses for the memory block area
    void *pvStartAddr = ptMemBlk->pvMemory;
    void *pvEndAddr = &ptMemBlk->pvMemory[ptMemBlk->zTypeSize * ptMemBlk->zLength];
    
    // Check if the address is outside the managed memory range or no block is in use
    if(pvAddr < pvStartAddr || pvEndAddr < pvAddr || ptMemBlk->zUsed == 0)
    {
        tStatus = JUNO_STATUS_MEMFREE_ERROR;
        // Log error if invalid address detected
        FAIL(tStatus, ptMemBlk->pfcnFailureHandler, ptMemBlk->pvUserData,
            "Failed to free block memory. Invalid Address"
        );
        return tStatus;
    }
    
    // Ensure the block has not already been freed
    for(size_t i = 0; i < ptMemBlk->zFreed; ++i)
    {
        if(ptMemBlk->pvMemoryFreeStack[i] == pvAddr)
        {
            tStatus = JUNO_STATUS_MEMFREE_ERROR;
            // Log error for duplicate free attempt
            FAIL(tStatus, ptMemBlk->pfcnFailureHandler, ptMemBlk->pvUserData,
                "Failed to free block memory. Memory already freed"
            );
            return tStatus;           
        }
    }
    
    // Clear the block memory
    memset(pvAddr, 0, ptMemBlk->zTypeSize);
    // Check if the block being freed is the last allocated block
    void *pvEndOfBlk = &ptMemBlk->pvMemory[(ptMemBlk->zUsed - 1) * ptMemBlk->zTypeSize];
    if(pvEndOfBlk == pvAddr)
    {
        // Simply decrement used counter without adding to free stack.
        ptMemBlk->zUsed -= 1;
    }
    else
    {
        // Otherwise, add the block back to the free stack.
        ptMemBlk->pvMemoryFreeStack[ptMemBlk->zFreed] = pvAddr;
        ptMemBlk->zFreed += 1;
    }
    return tStatus;
}

/**
 * @brief Generic function for memory allocation.
 *
 * Delegates allocation based on the allocation type defined in the generic union.
 *
 * @param ptMem Pointer to the memory allocation union.
 * @param ptMemory Pointer to the memory descriptor where allocated details are stored.
 * @return JUNO_STATUS_T Status of the allocation.
 */
JUNO_STATUS_T Juno_MemoryGet(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *ptMemory)
{
    ASSERT_EXISTS(ptMem);
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    // Switch based on allocation type stored in the header
    switch (ptMem->tHdr.tType)
    {
        case JUNO_MEMORY_ALLOC_TYPE_BLOCK:
        {
            // Delegate to block allocation getter
            tStatus = Juno_MemoryBlkGet(&ptMem->tBlock, &ptMemory->pvAddr);
            break;
        }
        default:
        {
            tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        }
    }
    return tStatus;
}

/**
 * @brief Generic function for freeing memory.
 *
 * Frees memory based on the allocation type and then clears the memory descriptor.
 *
 * @param ptMem Pointer to the memory allocation union.
 * @param ptMemory Pointer to the memory descriptor to free.
 * @return JUNO_STATUS_T Status of the free operation.
 */
JUNO_STATUS_T Juno_MemoryPut(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *ptMemory)
{
    ASSERT_EXISTS(ptMem);
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    // Switch based on allocation type stored in the header
    switch (ptMem->tHdr.tType)
    {
        case JUNO_MEMORY_ALLOC_TYPE_BLOCK:
        {
            // Delegate to block free function
            tStatus = Juno_MemoryBlkPut(&ptMem->tBlock, ptMemory->pvAddr);
            // Clear the memory descriptor fields
            ptMemory->pvAddr = NULL;
            ptMemory->zSize = 0;
            break;
        }
        default:
        {
            tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        }
    }
    return tStatus;
}

// Define static API structures to expose functions returning pointers to these APIs.
static const JUNO_MEMORY_BLOCK_API_T tJuno_MemoryBlkApi =
{
    .Init = Juno_MemoryBlkInit,
    .Get = Juno_MemoryBlkGet,
    .Put = Juno_MemoryBlkPut
};

static const JUNO_MEMORY_API_T tJuno_MemoryApi =
{
    .Get = Juno_MemoryGet,
    .Put = Juno_MemoryPut
};

/**
 * @brief Returns the API structure for block-based memory operations.
 *
 * @return Pointer to a constant JUNO_MEMORY_BLOCK_API_T structure.
 */
const JUNO_MEMORY_BLOCK_API_T * Juno_MemoryBlkApi(void)
{
    return &tJuno_MemoryBlkApi;
}

/**
 * @brief Returns the generic memory API structure.
 *
 * @return Pointer to a constant JUNO_MEMORY_API_T structure.
 */
const JUNO_MEMORY_API_T * Juno_MemoryApi(void)
{
    return &tJuno_MemoryApi;
}
