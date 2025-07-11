/*
    MIT License

    Copyright (c) 2025 Robin A. Onsay

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
#include "juno/memory/memory_block.h"
#include "juno/macros.h"
#include "juno/status.h"
#include "juno/memory/memory_api.h"
#include <string.h>

static const JUNO_MEMORY_ALLOC_API_T tJunoMemoryBlockApi;

static inline JUNO_STATUS_T Verify(JUNO_MEMORY_ALLOC_T *ptJunoMemory)
{
    ASSERT_EXISTS(ptJunoMemory);
    JUNO_MEMORY_ALLOC_BLOCK_T *ptJunoMemoryBlock = (JUNO_MEMORY_ALLOC_BLOCK_T *)(ptJunoMemory);
    ASSERT_EXISTS_MODULE(
        ptJunoMemory &&
        ptJunoMemoryBlock->JUNO_MODULE_SUPER.ptApi &&
        ptJunoMemoryBlock->pvMemory &&
        ptJunoMemoryBlock->ptMetadata &&
        ptJunoMemoryBlock->zLength &&
        ptJunoMemoryBlock->zTypeSize,
        ptJunoMemoryBlock,
        "Module does not have all dependencies"
    );
    if(ptJunoMemoryBlock->JUNO_MODULE_SUPER.ptApi != &tJunoMemoryBlockApi)
    {
        JUNO_FAIL_MODULE(JUNO_STATUS_INVALID_TYPE_ERROR, ptJunoMemoryBlock, "Module has invalid API");
        return JUNO_STATUS_INVALID_TYPE_ERROR;
    }
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T Juno_MemoryBlkGet(JUNO_MEMORY_ALLOC_T *ptJunoMemory, JUNO_MEMORY_T *ptMemory, size_t zSize)
{
    // Validate the memory block structure
    JUNO_STATUS_T tStatus = Verify(ptJunoMemory);
    ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_MEMORY_ALLOC_BLOCK_T *ptMemBlk = (JUNO_MEMORY_ALLOC_BLOCK_T *)(ptJunoMemory);
    if(!zSize)
    {
        tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
    }
    if(tStatus)
    {
        JUNO_FAIL_MODULE(tStatus, ptMemBlk,
        "Attempted to allocate memory with size 0");
        return tStatus;
    }
    // Delegate to block allocation getter
    if(zSize > ptMemBlk->zTypeSize)
    {
        tStatus = JUNO_STATUS_MEMALLOC_ERROR;
        JUNO_FAIL_MODULE(tStatus, ptMemBlk,
            "Invalid size for block alloc"
        );
        return tStatus;
    }
    // Check if there is room for allocation (either new or from freed list)
    if(!(ptMemBlk->zUsed < ptMemBlk->zLength || ptMemBlk->zFreed))
    {
        tStatus = JUNO_STATUS_MEMALLOC_ERROR;
        // Log error through the failure handler
        JUNO_FAIL_MODULE(tStatus, ptMemBlk,
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

static JUNO_STATUS_T Juno_MemoryBlkUpdate(JUNO_MEMORY_ALLOC_T *ptJunoMemory, JUNO_MEMORY_T *ptMemory, size_t zNewSize)
{
    // Validate the memory block structure
    JUNO_STATUS_T tStatus = Verify(ptJunoMemory);
    ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_MEMORY_ALLOC_BLOCK_T *ptMem = (JUNO_MEMORY_ALLOC_BLOCK_T *)(ptJunoMemory);
    if(zNewSize > ptMem->zTypeSize)
    {
        tStatus = JUNO_STATUS_MEMALLOC_ERROR;
        JUNO_FAIL_MODULE(tStatus, ptMem,
            "Failed to update memory, size is too big"
        );
    }
    ptMemory->zSize = zNewSize;
    return tStatus;
}

static JUNO_STATUS_T Juno_MemoryBlkPut(JUNO_MEMORY_ALLOC_T *ptJunoMemory, JUNO_MEMORY_T *ptMemory)
{
    // Validate the memory block structure
    JUNO_STATUS_T tStatus = Verify(ptJunoMemory);
    ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_MEMORY_ALLOC_BLOCK_T *ptMemBlk = (JUNO_MEMORY_ALLOC_BLOCK_T *)(ptJunoMemory);
    // There are still valid references to this memory, end with success
    if(ptMemory->iRefCount != 1)
    {
        tStatus = JUNO_STATUS_REF_IN_USE_ERROR;
        // Log error if invalid address detected
        JUNO_FAIL_MODULE(tStatus, ptMemBlk,
            "Failed to free block memory. Memory in use"
        );
        return tStatus;
    }
    // Calculate start and end addresses for the memory block area
    void *pvStartAddr = ptMemBlk->pvMemory;
    void *pvEndAddr = &ptMemBlk->pvMemory[ptMemBlk->zTypeSize * ptMemBlk->zLength];
    
    // Check if the address is outside the managed memory range or no block is in use
    if(ptMemory->pvAddr < pvStartAddr || pvEndAddr < ptMemory->pvAddr || ptMemBlk->zUsed == 0)
    {
        tStatus = JUNO_STATUS_MEMFREE_ERROR;
        // Log error if invalid address detected
        JUNO_FAIL_MODULE(tStatus, ptMemBlk,
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
            JUNO_FAIL_MODULE(tStatus, ptMemBlk,
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
    ptMemory->iRefCount = 0;
    return tStatus;
}


static const JUNO_MEMORY_ALLOC_API_T tJunoMemoryBlockApi = {
    .Get = Juno_MemoryBlkGet,
    .Update = Juno_MemoryBlkUpdate,
    .Put = Juno_MemoryBlkPut
};

/* TODO: Insert initialization arguments for module members here*/
JUNO_STATUS_T JunoMemory_BlockApi(JUNO_MEMORY_ALLOC_T *ptJunoMemory,
    void *pvMemory,
    JUNO_MEMORY_BLOCK_METADATA_T *ptMetadata,
    size_t zTypeSize,
    size_t zLength,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
)
{
    ASSERT_EXISTS(ptJunoMemory);
    JUNO_MEMORY_ALLOC_BLOCK_T *ptJunoMemoryBlock = (JUNO_MEMORY_ALLOC_BLOCK_T *)(ptJunoMemory);
    ptJunoMemoryBlock->JUNO_MODULE_SUPER.ptApi = &tJunoMemoryBlockApi;
    ptJunoMemoryBlock->JUNO_MODULE_SUPER.JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    ptJunoMemoryBlock->JUNO_MODULE_SUPER.JUNO_FAILURE_USER_DATA = pvFailureUserData;
    // Initialize memory area and free stack pointer
    ptJunoMemoryBlock->pvMemory = (uint8_t *) pvMemory;
    ptJunoMemoryBlock->ptMetadata = ptMetadata;
    // Set type size and total number of blocks
    ptJunoMemoryBlock->zTypeSize = zTypeSize;
    ptJunoMemoryBlock->zLength = zLength;
    // No block is in use yet
    ptJunoMemoryBlock->zUsed = 0;
    // Initially, no freed blocks are available
    ptJunoMemoryBlock->zFreed = 0;
    JUNO_STATUS_T tStatus = Verify(ptJunoMemory);
    ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}
