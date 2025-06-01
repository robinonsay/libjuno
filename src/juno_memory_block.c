#include "juno/macros.h"
#include "juno/memory/memory_block.h"
#include "juno/memory/memory_api.h"
#include "juno/status.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static inline JUNO_STATUS_T Juno_MemoryBlkValidate(JUNO_MEMORY_BLOCK_T *ptMemBlk)
{
    // Ensure that the memory block structure and its key members exist.
    ASSERT_EXISTS((ptMemBlk && ptMemBlk->pvMemory && ptMemBlk->ptMetadata && ptMemBlk->zLength && ptMemBlk->zTypeSize));
    return JUNO_STATUS_SUCCESS;
}

static inline JUNO_STATUS_T Juno_MemoryDecrementRef(JUNO_MEMORY_T *ptMemory)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    // Check if there are references 
    if(!ptMemory->iRefCount)
    {
        // No references to memory, this is an invalid reference
        tStatus = JUNO_STATUS_INVALID_REF_ERROR;
        return tStatus;
    }
    // Decrement the reference count
    ptMemory->iRefCount -= 1;
    return tStatus;
}

JUNO_STATUS_T Juno_MemoryBlkInit(
    JUNO_MEMORY_BLOCK_T *ptMemBlk,
    void *pvMemory,
    JUNO_MEMORY_BLOCK_METADATA_T *ptMetadata,
    size_t zTypeSize,
    size_t zLength,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
)
{
    // Validate input pointer
    ASSERT_EXISTS(ptMemBlk);
    // Set the allocation type to block allocation
    ptMemBlk->tHdr.tType = JUNO_MEMORY_ALLOC_TYPE_BLOCK;
    // Initialize memory area and free stack pointer
    ptMemBlk->pvMemory = (uint8_t *) pvMemory;
    ptMemBlk->ptMetadata = ptMetadata;
    // Set type size and total number of blocks
    ptMemBlk->zTypeSize = zTypeSize;
    ptMemBlk->zLength = zLength;
    // No block is in use yet
    ptMemBlk->zUsed = 0;
    // Set the failure handler and its user data
    ptMemBlk->pfcnFailureHandler = pfcnFailureHandler;
    ptMemBlk->pvFailureUserData = pvFailureUserData;
    // Initially, no freed blocks are available
    ptMemBlk->zFreed = 0;
    JUNO_STATUS_T tStatus = Juno_MemoryBlkValidate(ptMemBlk);
    ASSERT_SUCCESS(tStatus, return tStatus);
    // Clear the memory area
    memset(pvMemory, 0, zTypeSize * zLength);
    return tStatus;
}

static JUNO_STATUS_T Juno_MemoryBlkGet(JUNO_MEMORY_BLOCK_T *ptMemBlk, JUNO_MEMORY_T *ptMemory)
{
    // Validate the memory block structure
    JUNO_STATUS_T tStatus = Juno_MemoryBlkValidate(ptMemBlk);
    ASSERT_SUCCESS(tStatus, return tStatus);
    
    // Check if there is room for allocation (either new or from freed list)
    if(!(ptMemBlk->zUsed < ptMemBlk->zLength || ptMemBlk->zFreed))
    {
        tStatus = JUNO_STATUS_MEMALLOC_ERROR;
        // Log error through the failure handler
        FAIL(tStatus, ptMemBlk->pfcnFailureHandler, ptMemBlk->pvFailureUserData,
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
        ptMemBlk->ptMetadata[ptMemBlk->zFreed].ptFreeMem = &ptMemBlk->pvMemory[zNextFreeBlock];
        ptMemBlk->zFreed += 1;
        // Increment the used block counter
        ptMemBlk->zUsed += 1;
    }
    // Retrieve the latest free block and update free stack
    ptMemory->pvAddr = ptMemBlk->ptMetadata[ptMemBlk->zFreed-1].ptFreeMem;
    ptMemory->zSize = ptMemBlk->zTypeSize;
    ptMemory->iRefCount = 1;
    ptMemBlk->zFreed -= 1;
    ptMemBlk->ptMetadata[ptMemBlk->zFreed].ptFreeMem = NULL;
    return tStatus;
}

static JUNO_STATUS_T Juno_MemoryBlkUpdate(JUNO_MEMORY_BLOCK_T *ptMem, JUNO_MEMORY_T *ptMemory, size_t zNewSize)
{
    JUNO_STATUS_T tStatus = Juno_MemoryBlkValidate(ptMem);
    ASSERT_SUCCESS(tStatus, return tStatus);
    if(zNewSize > ptMem->zTypeSize)
    {
        tStatus = JUNO_STATUS_MEMALLOC_ERROR;
        FAIL(tStatus, ptMem->pfcnFailureHandler, ptMem->pvFailureUserData,
            "Failed to update memory, size is too big"
        );
    }
    ptMemory->zSize = zNewSize;
    return tStatus;
}

static JUNO_STATUS_T Juno_MemoryBlkPut(JUNO_MEMORY_BLOCK_T *ptMemBlk, JUNO_MEMORY_T *ptMemory)
{
    // Validate the memory block structure
    JUNO_STATUS_T tStatus = Juno_MemoryBlkValidate(ptMemBlk);
    ASSERT_SUCCESS(tStatus, return tStatus);
    // Calculate start and end addresses for the memory block area
    void *pvStartAddr = ptMemBlk->pvMemory;
    void *pvEndAddr = &ptMemBlk->pvMemory[ptMemBlk->zTypeSize * ptMemBlk->zLength];
    
    // Check if the address is outside the managed memory range or no block is in use
    if(ptMemory->pvAddr < pvStartAddr || pvEndAddr < ptMemory->pvAddr || ptMemBlk->zUsed == 0)
    {
        tStatus = JUNO_STATUS_MEMFREE_ERROR;
        // Log error if invalid address detected
        FAIL(tStatus, ptMemBlk->pfcnFailureHandler, ptMemBlk->pvFailureUserData,
            "Failed to free block memory. Invalid Address"
        );
        return tStatus;
    }
    
    // Ensure the block has not already been freed
    for(size_t i = 0; i < ptMemBlk->zFreed; ++i)
    {
        if(ptMemBlk->ptMetadata[i].ptFreeMem == ptMemory->pvAddr)
        {
            tStatus = JUNO_STATUS_MEMFREE_ERROR;
            // Log error for duplicate free attempt
            FAIL(tStatus, ptMemBlk->pfcnFailureHandler, ptMemBlk->pvFailureUserData,
                "Failed to free block memory. Memory already freed"
            );
            return tStatus;           
        }
    }
    
    // Clear the block memory
    memset(ptMemory->pvAddr, 0, ptMemBlk->zTypeSize);
    // Check if the block being freed is the last allocated block
    void *pvEndOfBlk = &ptMemBlk->pvMemory[(ptMemBlk->zUsed - 1) * ptMemBlk->zTypeSize];
    if(pvEndOfBlk == ptMemory->pvAddr)
    {
        // Simply decrement used counter without adding to free stack.
        ptMemBlk->zUsed -= 1;
    }
    else
    {
        // Otherwise, add the block back to the free stack.
        ptMemBlk->ptMetadata[ptMemBlk->zFreed].ptFreeMem = ptMemory->pvAddr;
        ptMemBlk->zFreed += 1;
    }
    ptMemory->pvAddr = NULL;
    ptMemory->zSize = 0;
    return tStatus;
}

JUNO_STATUS_T Juno_MemoryUpdate(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *ptMemory, size_t zNewSize)
{
    ASSERT_EXISTS(ptMem);
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    // Switch based on allocation type stored in the header
    switch (ptMem->tHdr.tType)
    {
        case JUNO_MEMORY_ALLOC_TYPE_BLOCK:
        {
            // Delegate to block allocation getter
            tStatus = Juno_MemoryBlkUpdate(&ptMem->tBlock, ptMemory, zNewSize);
            break;
        }
        default:
        {
            tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        }
    }
    return tStatus;
}

JUNO_STATUS_T Juno_MemoryGet(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *ptMemory, size_t zSize)
{
    ASSERT_EXISTS(ptMem);
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    if(!zSize)
    {
        tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
    }
    // Switch based on allocation type stored in the header
    switch (ptMem->tHdr.tType)
    {
        case JUNO_MEMORY_ALLOC_TYPE_BLOCK:
        {
            if(tStatus)
            {
                FAIL(tStatus, ptMem->tBlock.pfcnFailureHandler, ptMem->tBlock.pvFailureUserData,
                "Attempted to allocate memory with size 0");
                return tStatus;
            }
            // Delegate to block allocation getter
            if(zSize > ptMem->tBlock.zTypeSize)
            {
                tStatus = JUNO_STATUS_MEMALLOC_ERROR;
                FAIL(tStatus, ptMem->tBlock.pfcnFailureHandler, ptMem->tBlock.pvFailureUserData,
                    "Invalid size for block alloc"
                );
                return tStatus;
            }
            tStatus = Juno_MemoryBlkGet(&ptMem->tBlock, ptMemory);
            break;
        }
        default:
        {
            tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
            ptMemory->zSize = 0;
            ptMemory->pvAddr = NULL;
        }
    }
    return tStatus;
}

JUNO_STATUS_T Juno_MemoryPut(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *ptMemory)
{
    ASSERT_EXISTS(ptMem);
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    // Decrement the reference counts
    tStatus = Juno_MemoryDecrementRef(ptMemory);
    if(tStatus) return tStatus;
    // There are still valid references to this memory, end with success
    if(ptMemory->iRefCount)
    {
        tStatus = JUNO_STATUS_REF_IN_USE_ERROR;
        ptMemory->iRefCount += 1;
    }
    // Switch based on allocation type stored in the header
    switch (ptMem->tHdr.tType)
    {
        case JUNO_MEMORY_ALLOC_TYPE_BLOCK:
        {
            if(tStatus)
            {
                FAIL(tStatus, ptMem->tBlock.pfcnFailureHandler, ptMem->tBlock.pvFailureUserData, "Failed to free memory, reference in use");
                return tStatus;
            }
            // Delegate to block free function
            tStatus = Juno_MemoryBlkPut(&ptMem->tBlock, ptMemory);
            if(tStatus)
            {
                ptMemory->iRefCount++;
            }
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
