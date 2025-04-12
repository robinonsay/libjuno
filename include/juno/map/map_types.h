#ifndef JUNO_MAP_TYPES_H
#define JUNO_MAP_TYPES_H

#include "juno/hash/hash_api.h"
#include "juno/macros.h"
#include "juno/memory/memory_types.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JUNO_MAP_TAG JUNO_MAP_T;
typedef bool (*JUNO_MAP_KEY_EQUAL_FCN_T)(JUNO_MEMORY_T ptKey1, JUNO_MEMORY_T ptKey2);

struct JUNO_MAP_TAG
{
    const JUNO_HASH_API_T *ptHashApi;
    JUNO_MEMORY_T *ptMapKeys;
    JUNO_MEMORY_T *ptMapValues;
    size_t zCapacity;
    size_t zLenHashTable;
    JUNO_MAP_KEY_EQUAL_FCN_T pfcnIsEqual;
    DECLARE_FAILURE_HANDLER;
};

#ifdef __cplusplus
}
#endif
#endif

