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
typedef struct JUNO_MEMORY_ALLOC_HDR_TAG JUNO_MEMORY_ALLOC_HDR_T;
typedef struct JUNO_MEMORY_TAG JUNO_MEMORY_T;
typedef union JUNO_MEMORY_ALLOC_TAG JUNO_MEMORY_ALLOC_T;
typedef enum JUNO_MEMORY_ALLOC_TYPE_TAG
{
    JUNO_MEMORY_ALLOC_TYPE_RESERVED = 0,
    JUNO_MEMORY_ALLOC_TYPE_BLOCK    = 1,
} JUNO_MEMORY_ALLOC_TYPE_T;

struct JUNO_MEMORY_ALLOC_HDR_TAG
{
    JUNO_MEMORY_ALLOC_TYPE_T tType;
};

struct JUNO_MEMORY_TAG
{
    void *pvAddr;
    size_t zSize;
};

struct JUNO_MEMORY_BLOCK_TAG
{
    JUNO_MEMORY_ALLOC_HDR_T tHdr;
    uint8_t *pvMemory;
    uint8_t **pvMemoryFreeStack;
    size_t zTypeSize;
    size_t zLength;
    size_t zUsed;
    size_t zFreed;
    DECLARE_FAILURE_HANDLER;
};

union JUNO_MEMORY_ALLOC_TAG
{
    JUNO_MEMORY_ALLOC_HDR_T tHdr;
    JUNO_MEMORY_BLOCK_T tBlock;
};

#ifdef __cplusplus
}
#endif
#endif

