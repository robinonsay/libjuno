#ifndef JUNO_MEMORY_API_H
#define JUNO_MEMORY_API_H
#include "juno/status.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "juno/memory/memory_types.h"

typedef struct JUNO_MEMORY_BLOCK_API_TAG JUNO_MEMORY_BLOCK_API_T;
typedef struct JUNO_MEMORY_API_TAG JUNO_MEMORY_API_T;

struct JUNO_MEMORY_API_TAG
{

    JUNO_STATUS_T (*Get)(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *pvRetAddr, size_t zSize);
    JUNO_STATUS_T (*Update)(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *ptMemory, size_t zNewSize);
    JUNO_STATUS_T (*Put)(JUNO_MEMORY_ALLOC_T *ptMem, JUNO_MEMORY_T *pvAddr);
};

struct JUNO_MEMORY_BLOCK_API_TAG
{
    JUNO_STATUS_T (*Init)(
        JUNO_MEMORY_BLOCK_T *ptMemBlk,
        void *pvMemory,
        JUNO_MEMORY_METADATA_T *ptMetadata,
        size_t zTypeSize,
        size_t zLength,
        JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
        JUNO_USER_DATA_T *pvUserData
    );
};

const JUNO_MEMORY_BLOCK_API_T * Juno_MemoryBlkApi(void);
const JUNO_MEMORY_API_T * Juno_MemoryApi(void);

#ifdef __cplusplus
}
#endif
#endif


