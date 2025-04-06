#ifndef JUNO_MEMORY_H
#define JUNO_MEMORY_H
#include "juno/macros.h"
#include "juno/status.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define MEMORY_FREE_STACK(name) pz##name##FreeStack
#define MEMORY_BLOCK(name, type, length) \
static type name[length] = {0}; \
static uint8_t* MEMORY_FREE_STACK(name)[length] = {0}

typedef struct JUNO_MEMORY_BLOCK_TAG JUNO_MEMORY_BLOCK_T;
typedef struct JUNO_MEMORY_BLOCK_API_TAG JUNO_MEMORY_BLOCK_API_T;

struct JUNO_MEMORY_BLOCK_TAG
{
    uint8_t *pvMemory;
    uint8_t **pvMemoryFreeStack;
    size_t zTypeSize;
    size_t zLength;
    size_t zUsed;
    size_t zFreed;
    DECLARE_FAILURE_HANDLER;
};

struct JUNO_MEMORY_BLOCK_API_TAG
{
    JUNO_STATUS_T (*Init)(
        JUNO_MEMORY_BLOCK_T *ptMemBlk,
        void *pvMemory,
        uint8_t **pvMemoryFreeStack,
        size_t zTypeSize,
        size_t zLength,
        JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
        JUNO_USER_DATA_T *pvUserData
    );
    JUNO_STATUS_T (*Get)(JUNO_MEMORY_BLOCK_T *ptMemBlk, void **pvRetAddr);
    JUNO_STATUS_T (*Put)(JUNO_MEMORY_BLOCK_T *ptMemBlk, void *pvAddr);
};

const JUNO_MEMORY_BLOCK_API_T * Juno_MemoryBlkApi(void);

#ifdef __cplusplus
}
#endif
#endif

