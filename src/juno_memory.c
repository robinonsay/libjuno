#include "juno/macros.h"
#include "juno/memory/memory.h"
#include "juno/memory/memory_api.h"
#include "juno/memory/memory_types.h"
#include "juno/status.h"
#include <stddef.h>
#include <stdint.h>

static JUNO_STATUS_T Juno_MemoryBlkValidate(JUNO_MEMORY_BLOCK_T *ptMemBlk)
{
    // Ensure that the memory block structure and its key members exist.
    ASSERT_EXISTS((ptMemBlk && ptMemBlk->pvMemory && ptMemBlk->pvMemoryFreeStack && ptMemBlk->zLength && ptMemBlk->zTypeSize));
    return JUNO_STATUS_SUCCESS;
}

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
    Juno_Memset(pvMemory, 0, zTypeSize * zLength);
    return Juno_MemoryBlkValidate(ptMemBlk);
}

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
            "Failed to allocate block memory. Memory is full"
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
    Juno_Memset(pvAddr, 0, ptMemBlk->zTypeSize);
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


void Juno_Memset(void *pMem, const uint8_t iVal, size_t zSize)
{
    uint8_t *pcMem = pMem;
    for(size_t i = 0; i < zSize; i++)
    {
        pcMem[i] = iVal;
    }
}

void Juno_Memcpy(void *pDest, const void *pSrc, size_t zSize)
{
    uint8_t *pcDest = pDest;
    const uint8_t *pcSrc = pSrc;
    for(size_t i = 0; i < zSize; i++)
    {
        pcDest[i] = pcSrc[i];
    }
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

const JUNO_MEMORY_BLOCK_API_T * Juno_MemoryBlkApi(void)
{
    return &tJuno_MemoryBlkApi;
}

const JUNO_MEMORY_API_T * Juno_MemoryApi(void)
{
    return &tJuno_MemoryApi;
}
