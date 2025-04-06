#ifndef JUNO_MEMORY_H
#define JUNO_MEMORY_H
#include "juno/macros.h"
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

#ifdef __cplusplus
}
#endif
#endif

