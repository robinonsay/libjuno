#ifndef JUNO_MEMORY_DIRECT_H
#define JUNO_MEMORY_DIRECT_H
#include "juno/memory/memory_types.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

JUNO_STATUS_T Juno_MemoryBlkInit(
    JUNO_MEMORY_BLOCK_T *ptMemBlk,
    void *pvMemory,
    uint8_t **pvMemoryFreeStack,
    size_t zTypeSize,
    size_t zLength,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvUserData
);
JUNO_STATUS_T Juno_MemoryBlkGet(JUNO_MEMORY_BLOCK_T *ptMemBlk, void **pvRetAddr);
JUNO_STATUS_T Juno_MemoryBlkPut(JUNO_MEMORY_BLOCK_T *ptMemBlk, void *pvAddr);

#ifdef __cplusplus
}
#endif
#endif
