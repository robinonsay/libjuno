#ifndef JUNO_MAP_TYPES_H
#define JUNO_MAP_TYPES_H

#include "juno/macros.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JUNO_MAP_TAG JUNO_MAP_T;
typedef struct JUNO_MAP_KEY_TAG JUNO_MAP_KEY_T;
typedef struct JUNO_MAP_VALUE_TAG JUNO_MAP_VALUE_T;
typedef bool (*JUNO_MAP_KEY_EQUAL_FCN_T)(void * pvKey1, void *pvKey2);

struct JUNO_MAP_KEY_TAG
{
    void *ptKey;
};

struct JUNO_MAP_VALUE_TAG
{
    void *ptValue;
};

struct JUNO_MAP_TAG
{
    JUNO_MAP_KEY_T *pvMapKeys;
    JUNO_MAP_VALUE_T *pvMapValues;
    size_t zCapacity;
    size_t zLenHashTable;
    JUNO_MAP_KEY_EQUAL_FCN_T pfcnIsEqual;
    DECLARE_FAILURE_HANDLER;
};

#ifdef __cplusplus
}
#endif
#endif

