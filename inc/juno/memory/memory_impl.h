#ifndef JUNO_MEMORY_IMPL_H
#define JUNO_MEMORY_IMPL_H
#include "juno/macros.h"
#include "juno/memory.h"
#include "juno/memory/memory_direct.h"
#include "juno/status.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif

static JUNO_STATUS_T Juno_MemoryBlkValidate(JUNO_MEMORY_BLOCK_T *ptMemBlk)
{
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
    ASSERT_EXISTS(ptMemBlk);
    ptMemBlk->pvMemory = (uint8_t *) pvMemory;
    ptMemBlk->pvMemoryFreeStack = pvMemoryFreeStack;
    ptMemBlk->zTypeSize = zTypeSize;
    ptMemBlk->zLength = zLength;
    ptMemBlk->zUsed = 0;
    ptMemBlk->pfcnFailureHandler = pfcnFailureHandler;
    ptMemBlk->pvUserData = pvUserData;
    ptMemBlk->zFreed = 0;
    return Juno_MemoryBlkValidate(ptMemBlk);
}

JUNO_STATUS_T Juno_MemoryBlkGet(JUNO_MEMORY_BLOCK_T *ptMemBlk, void **pvRetAddr)
{
    JUNO_STATUS_T tStatus = Juno_MemoryBlkValidate(ptMemBlk);
    ASSERT_SUCCESS(tStatus, return tStatus);
    if(!(ptMemBlk->zUsed < ptMemBlk->zLength || ptMemBlk->zFreed))
    {
        tStatus = JUNO_STATUS_MEMALLOC_ERROR;
        FAIL(tStatus, ptMemBlk->pfcnFailureHandler, ptMemBlk->pvUserData,
            "Failed to allocate block memory. Memory is full: %lu", ptMemBlk->zLength
        );
        return tStatus;
    }
    if(ptMemBlk->zFreed == 0)
    {
        // Get the next free block
        size_t zNextFreeBlock = ptMemBlk->zUsed * ptMemBlk->zTypeSize;
        // Assign this block to the freed list
        ptMemBlk->pvMemoryFreeStack[ptMemBlk->zFreed] = &ptMemBlk->pvMemory[zNextFreeBlock];
        ptMemBlk->zFreed += 1;
        // Increment the number of blocks used
        ptMemBlk->zUsed += 1;
    }
    // Get the lateset free memory
    *pvRetAddr = ptMemBlk->pvMemoryFreeStack[ptMemBlk->zFreed-1];
    ptMemBlk->zFreed -= 1;
    ptMemBlk->pvMemoryFreeStack[ptMemBlk->zFreed] = NULL;
    return tStatus;
}

JUNO_STATUS_T Juno_MemoryBlkPut(JUNO_MEMORY_BLOCK_T *ptMemBlk, void *pvAddr)
{
    JUNO_STATUS_T tStatus = Juno_MemoryBlkValidate(ptMemBlk);
    ASSERT_SUCCESS(tStatus, return tStatus);
    void *pvStartAddr = ptMemBlk->pvMemory;
    void *pvEndAddr = &ptMemBlk->pvMemory[ptMemBlk->zTypeSize * ptMemBlk->zLength];
    if(pvAddr < pvStartAddr || pvEndAddr < pvAddr)
    {
        tStatus = JUNO_STATUS_MEMFREE_ERROR;
        FAIL(tStatus, ptMemBlk->pfcnFailureHandler, ptMemBlk->pvUserData,
            "Failed to free block memory. Invalid Address"
        );
        return tStatus;
    }
    for(size_t i = 0; i < ptMemBlk->zFreed; ++i)
    {
        if(ptMemBlk->pvMemoryFreeStack[i] == pvAddr)
        {
            tStatus = JUNO_STATUS_MEMFREE_ERROR;
            FAIL(tStatus, ptMemBlk->pfcnFailureHandler, ptMemBlk->pvUserData,
                "Failed to free block memory. Memory already freed"
            );
            return tStatus;           
        }
    }
    void *pvEndOfBlk = &ptMemBlk->pvMemory[(ptMemBlk->zUsed - 1) * ptMemBlk->zTypeSize];
    if(pvEndOfBlk == pvAddr)
    {
        ptMemBlk->zUsed -= 1;
    }
    else
    {
        ptMemBlk->pvMemoryFreeStack[ptMemBlk->zFreed] = pvAddr;
        ptMemBlk->zFreed += 1;
    }
    return tStatus;
}

static const JUNO_MEMORY_BLOCK_API_T tJuno_MemoryBlkApi =
{
    .Init = Juno_MemoryBlkInit,
    .Get = Juno_MemoryBlkGet,
    .Put = Juno_MemoryBlkPut
};


const JUNO_MEMORY_BLOCK_API_T * Juno_MemoryBlkApi(void)
{
    return &tJuno_MemoryBlkApi;
}

#ifdef __cplusplus
}
#endif
#endif
