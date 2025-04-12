#ifndef JUNO_MAP_API_H
#define JUNO_MAP_API_H
#include "juno/memory/memory_types.h"
#include "juno/status.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "juno/map/map_types.h"

typedef struct JUNO_MAP_API_TAG JUNO_MAP_API_T;

struct JUNO_MAP_API_TAG
{
    JUNO_STATUS_T (*Init)(
        JUNO_MAP_T *ptMap,
        const JUNO_HASH_API_T *ptHashApi,
        JUNO_MEMORY_T *ptKeyTable,
        JUNO_MEMORY_T *ptValueTable,
        size_t zCapacity,
        JUNO_MAP_KEY_EQUAL_FCN_T pfcnIsEqual,
        JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
        JUNO_USER_DATA_T *pvFailureUserData
    );
    
    JUNO_STATUS_T (*Set)(JUNO_MAP_T *ptMap, JUNO_MEMORY_T tKey, JUNO_MEMORY_T tValue);
    
    JUNO_STATUS_T (*Remove)(JUNO_MAP_T *ptMap, JUNO_MEMORY_T tKey);
    
    JUNO_STATUS_T (*Get)(JUNO_MAP_T *ptMap, JUNO_MEMORY_T tKey, JUNO_MEMORY_T *ptRetValue);
};

const JUNO_MAP_API_T * Juno_MapApi(void);

#ifdef __cplusplus
}
#endif
#endif


