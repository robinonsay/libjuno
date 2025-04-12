#include "juno/hash/hash_api.h"
#include "juno/macros.h"
#include "juno/map/map.h"
#include "juno/map/map_types.h"
#include "juno/status.h"
#include <stdint.h>

static inline JUNO_STATUS_T Juno_MapValidate(JUNO_MAP_T *ptMap)
{
    if(!ptMap)
    {
        return JUNO_STATUS_NULLPTR_ERROR;
    }
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    if(!(ptMap->ptMapKeys && ptMap->ptHashApi && ptMap->zKeySize && ptMap->ptMapValues && ptMap->zCapacity && ptMap->pfcnIsEqual))
    {
        tStatus = JUNO_STATUS_NULLPTR_ERROR;
        FAIL(tStatus, ptMap->pfcnFailureHandler, ptMap->pvUserData, "Invalid dependencies");
    }
    return tStatus;

}

JUNO_STATUS_T Juno_MapInit(
    JUNO_MAP_T *ptMap,
    const JUNO_HASH_API_T *ptHashApi,
    JUNO_MAP_KEY_T *ptKeyTable,
    JUNO_MAP_VALUE_T *ptValueTable,
    size_t zKeySize,
    size_t zCapacity,
    JUNO_MAP_KEY_EQUAL_FCN_T pfcnIsEqual,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
)
{
    if(!ptMap)
    {
        return JUNO_STATUS_NULLPTR_ERROR;
    }
    ptMap->ptHashApi = ptHashApi;
    ptMap->ptMapKeys = ptKeyTable;
    ptMap->ptMapValues = ptValueTable;
    ptMap->zKeySize = zKeySize;
    ptMap->zCapacity = zCapacity;
    ptMap->pfcnIsEqual = pfcnIsEqual;
    ptMap->pfcnFailureHandler = pfcnFailureHandler;
    ptMap->pvUserData = pvFailureUserData;
    return Juno_MapValidate(ptMap);
}

static inline JUNO_STATUS_T Juno_MapGetIndex(JUNO_MAP_T *ptMap, JUNO_MAP_KEY_T tKey, size_t *pzRetSize)
{
    size_t zHash = 0;
    JUNO_STATUS_T tStatus = ptMap->ptHashApi->Hash((const uint8_t *)(tKey.ptKey), ptMap->zKeySize, &zHash);
    ASSERT_SUCCESS(tStatus, return tStatus);
    // Get the capacity
    size_t zCapacity = ptMap->zCapacity;
    // Calculate the index
    size_t zIndex = zHash % zCapacity;
    // Set the initial status
    tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
    // Iterate over the table
    for (size_t i = 0; i < zCapacity; i++)
    {
        // Get a pointer to the current key
        JUNO_MAP_KEY_T tCurKey = ptMap->ptMapKeys[zIndex];
        // Check if the spot is empty or the key is equal
        if (tCurKey.ptKey || ptMap->pfcnIsEqual(tCurKey, tKey))
        {
            // Return the index
            *pzRetSize = zIndex;
            tStatus = JUNO_STATUS_SUCCESS;
            break;
        }
        // Probe by i**2
        zIndex = (zHash + i * i) % zCapacity;
    }
    if(tStatus != JUNO_STATUS_SUCCESS)
    {
        FAIL(tStatus, ptMap->pfcnFailureHandler, ptMap->pvUserData, "Map is full");
    }
    // Return the status
    return tStatus;
}

JUNO_STATUS_T Juno_MapSet(JUNO_MAP_T *ptMap, JUNO_MAP_KEY_T tKey, JUNO_MAP_VALUE_T tValue)
{
    JUNO_STATUS_T tStatus = Juno_MapValidate(ptMap);
    ASSERT_SUCCESS(tStatus, return tStatus);
    size_t zIndex = 0;
    tStatus = Juno_MapGetIndex(ptMap, tKey, &zIndex);
    ASSERT_SUCCESS(tStatus, return tStatus);
    ptMap->ptMapKeys[zIndex] = tKey;
    ptMap->ptMapValues[zIndex] = tValue;
    return tStatus;
}

JUNO_STATUS_T Juno_MapRemove(JUNO_MAP_T *ptMap, JUNO_MAP_KEY_T tKey)
{
    JUNO_STATUS_T tStatus = Juno_MapValidate(ptMap);
    ASSERT_SUCCESS(tStatus, return tStatus);
    size_t zIndex = 0;
    tStatus = Juno_MapGetIndex(ptMap, tKey, &zIndex);
    ASSERT_SUCCESS(tStatus, return tStatus);
    ptMap->ptMapKeys[zIndex] = (JUNO_MAP_KEY_T){0};
    ptMap->ptMapValues[zIndex] = (JUNO_MAP_VALUE_T){0};
    return tStatus;
}


JUNO_STATUS_T Juno_MapGet(JUNO_MAP_T *ptMap, JUNO_MAP_KEY_T tKey, JUNO_MAP_VALUE_T *ptRetValue)
{
    JUNO_STATUS_T tStatus = Juno_MapValidate(ptMap);
    ASSERT_SUCCESS(tStatus, return tStatus);
    size_t zIndex = 0;
    tStatus = Juno_MapGetIndex(ptMap, tKey, &zIndex);
    ASSERT_SUCCESS(tStatus, return tStatus);
    if(!ptMap->ptMapKeys[zIndex].ptKey)
    {
        tStatus = JUNO_STATUS_DNE_ERROR;
        FAIL(tStatus, ptMap->pfcnFailureHandler, ptMap->pvUserData, "Key Does not exist");
        return tStatus;
    }
    *ptRetValue = ptMap->ptMapValues[zIndex];
    return tStatus;
}
