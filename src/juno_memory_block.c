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
#include <stddef.h>
#include <stdint.h>

static const JUNO_MEMORY_ALLOC_API_T tJunoMemoryBlockApi;

static inline JUNO_STATUS_T Verify(JUNO_MEMORY_ALLOC_ROOT_T *ptJunoMemory)
{
    JUNO_ASSERT_EXISTS(ptJunoMemory);
    JUNO_MEMORY_ALLOC_BLOCK_T *ptJunoMemoryBlock = (JUNO_MEMORY_ALLOC_BLOCK_T *)(ptJunoMemory);
    JUNO_STATUS_T tStatus = JunoMemory_AllocVerify(ptJunoMemory);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_ASSERT_EXISTS_MODULE(
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

static JUNO_RESULT_POINTER_T Juno_MemoryBlkGet(JUNO_MEMORY_ALLOC_ROOT_T *ptJunoMemory, size_t zSize)
{
    JUNO_RESULT_POINTER_T tResult = {0};
    // Validate the memory block structure
    tResult.tStatus = Verify(ptJunoMemory);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    JUNO_MEMORY_ALLOC_BLOCK_T *ptMemBlk = (JUNO_MEMORY_ALLOC_BLOCK_T *)(ptJunoMemory);
    if(!zSize)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
    }
    if(tResult.tStatus)
    {
        JUNO_FAIL_MODULE(tResult.tStatus, ptMemBlk,
        "Attempted to allocate memory with size 0");
        return tResult;
    }
    // Delegate to block allocation getter
    if(zSize > ptMemBlk->zTypeSize)
    {
        tResult.tStatus = JUNO_STATUS_MEMALLOC_ERROR;
        JUNO_FAIL_MODULE(tResult.tStatus, ptMemBlk,
            "Invalid size for block alloc"
        );
        return tResult;
    }
    // Check if there is room for allocation (either new or from freed list)
    if(!(ptMemBlk->zUsed < ptMemBlk->zLength || ptMemBlk->zFreed))
    {
        tResult.tStatus = JUNO_STATUS_MEMALLOC_ERROR;
        // Log error through the failure handler
        JUNO_FAIL_MODULE(tResult.tStatus, ptMemBlk,
            "Failed to allocate block memory. Memory is full"
        );
        return tResult;
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
    tResult.tOk.ptApi = ptMemBlk->tRoot.ptPointerApi;
    // Retrieve the latest free block and update free stack
    tResult.tOk.pvAddr = ptMemBlk->ptMetadata[ptMemBlk->zFreed-1].ptFreeMem;
    tResult.tOk.zSize = ptMemBlk->zTypeSize;
    ptMemBlk->zFreed -= 1;
    ptMemBlk->ptMetadata[ptMemBlk->zFreed].ptFreeMem = NULL;
    tResult.tStatus = JunoMemory_PointerVerify(&tResult.tOk);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    tResult.tStatus = tResult.tOk.ptApi->Reset(tResult.tOk);
    return tResult;
}

static JUNO_STATUS_T Juno_MemoryBlkUpdate(JUNO_MEMORY_ALLOC_ROOT_T *ptJunoMemory, JUNO_POINTER_T *ptMemory, size_t zNewSize)
{
    // Validate the memory block structure
    JUNO_STATUS_T tStatus = Verify(ptJunoMemory);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_MEMORY_ALLOC_BLOCK_T *ptMem = (JUNO_MEMORY_ALLOC_BLOCK_T *)(ptJunoMemory);
    if(zNewSize > ptMem->zTypeSize)
    {
        tStatus = JUNO_STATUS_MEMALLOC_ERROR;
        JUNO_FAIL_MODULE(tStatus, ptMem,
            "Failed to update memory, size is too big"
        );
        return tStatus;
    }
    ptMemory->zSize = zNewSize;
    return tStatus;
}

static JUNO_STATUS_T Juno_MemoryBlkPut(JUNO_MEMORY_ALLOC_ROOT_T *ptJunoMemory, JUNO_POINTER_T *ptMemory)
{
    // Validate the memory block structure
    JUNO_STATUS_T tStatus = Verify(ptJunoMemory);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerify(ptMemory);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_MEMORY_ALLOC_BLOCK_T *ptMemBlk = (JUNO_MEMORY_ALLOC_BLOCK_T *)(ptJunoMemory);
    JUNO_ASSERT_EXISTS(ptMemory && ptMemory->pvAddr);
    JUNO_POINTER_T tMemory = *ptMemory;
    ptMemory->pvAddr = NULL;
    ptMemory->zSize = 0;
    // Calculate start and end addresses for the memory block area
    void *pvStartAddr = ptMemBlk->pvMemory;
    void *pvEndAddr = &ptMemBlk->pvMemory[ptMemBlk->zTypeSize * ptMemBlk->zLength];
    
    // Check if the address is outside the managed memory range or no block is in use
    if(tMemory.pvAddr < pvStartAddr || pvEndAddr < tMemory.pvAddr || ptMemBlk->zUsed == 0)
    {
        tStatus = JUNO_STATUS_MEMFREE_ERROR;
        // Log error if invalid address detected
        JUNO_FAIL_MODULE(tStatus, ptMemBlk,
            "Failed to free block memory. Invalid Address"
        );
        *ptMemory = tMemory;
        return tStatus;
    }
    
    // Ensure the block has not already been freed
    for(size_t i = 0; i < ptMemBlk->zFreed; ++i)
    {
        if(ptMemBlk->ptMetadata[i].ptFreeMem == tMemory.pvAddr)
        {
            tStatus = JUNO_STATUS_MEMFREE_ERROR;
            // Log error for duplicate free attempt
            JUNO_FAIL_MODULE(tStatus, ptMemBlk,
                "Failed to free block memory. Memory already freed"
            );
            *ptMemory = tMemory;
            return tStatus;           
        }
    }
    tStatus = tMemory.ptApi->Reset(tMemory);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Check if the block being freed is the last allocated block
    void *pvEndOfBlk = &ptMemBlk->pvMemory[(ptMemBlk->zUsed - 1) * ptMemBlk->zTypeSize];
    if(pvEndOfBlk == tMemory.pvAddr)
    {
        // Simply decrement used counter without adding to free stack.
        ptMemBlk->zUsed -= 1;
    }
    else
    {
        // Otherwise, add the block back to the free stack.
        ptMemBlk->ptMetadata[ptMemBlk->zFreed].ptFreeMem = tMemory.pvAddr;
        ptMemBlk->zFreed += 1;
    }

    return tStatus;
}

static const JUNO_MEMORY_ALLOC_API_T tJunoMemoryBlockApi = {
    .Get = Juno_MemoryBlkGet,
    .Update = Juno_MemoryBlkUpdate,
    .Put = Juno_MemoryBlkPut
};

/* TODO: Insert initialization arguments for module members here*/
JUNO_STATUS_T JunoMemory_BlockApi(
    JUNO_MEMORY_ALLOC_BLOCK_T *ptJunoMemoryBlock,
    const JUNO_POINTER_API_T *ptPointerApi,
    void *pvMemory,
    JUNO_MEMORY_BLOCK_METADATA_T *ptMetadata,
    size_t zTypeSize,
    size_t zLength,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
)
{
    JUNO_ASSERT_EXISTS(ptJunoMemoryBlock);
    ptJunoMemoryBlock->JUNO_MODULE_SUPER.ptApi = &tJunoMemoryBlockApi;
    ptJunoMemoryBlock->JUNO_MODULE_SUPER.JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    ptJunoMemoryBlock->JUNO_MODULE_SUPER.JUNO_FAILURE_USER_DATA = pvFailureUserData;
    ptJunoMemoryBlock->tRoot.ptPointerApi = ptPointerApi;
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
    JUNO_STATUS_T tStatus = Verify((JUNO_MEMORY_ALLOC_ROOT_T *) ptJunoMemoryBlock);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}
