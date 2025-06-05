/*
    MIT License

    Copyright (c) Year Robin A. Onsay

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.
*/
#include "juno/hash/hash_api.h"
#include "juno/macros.h"
#include "juno/map/map.h"
#include "juno/map/map_api.h"
#include "juno/map/map_types.h"
#include "juno/memory/memory_types.h"
#include "juno/status.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static inline JUNO_STATUS_T Juno_MapValidate(JUNO_MAP_T *ptMap)
{
    if(!ptMap)
    {
        return JUNO_STATUS_NULLPTR_ERROR;
    }
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    if(!(ptMap->ptMapKeys && ptMap->ptHashApi && ptMap->ptMapValues && ptMap->zCapacity && ptMap->pfcnIsEqual))
    {
        tStatus = JUNO_STATUS_NULLPTR_ERROR;
        FAIL(tStatus, ptMap->pfcnFailureHandler, ptMap->pvFailureUserData, "Invalid dependencies");
    }
    return tStatus;

}

JUNO_STATUS_T Juno_MapInit(
    JUNO_MAP_T *ptMap,
    const JUNO_HASH_API_T *ptHashApi,
    JUNO_MEMORY_T *ptKeyTable,
    JUNO_MEMORY_T *ptValueTable,
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
    ptMap->zCapacity = zCapacity;
    ptMap->zLenHashTable = 0;
    ptMap->pfcnIsEqual = pfcnIsEqual;
    ptMap->pfcnFailureHandler = pfcnFailureHandler;
    ptMap->pvFailureUserData = pvFailureUserData;
    JUNO_STATUS_T tStatus =  Juno_MapValidate(ptMap);
    ASSERT_SUCCESS(tStatus, return tStatus);
    memset(ptKeyTable, 0, sizeof(JUNO_MEMORY_T) * zCapacity);
    memset(ptValueTable, 0, sizeof(JUNO_MEMORY_T) * zCapacity);
    return tStatus;
}

static inline JUNO_STATUS_T Juno_MapGetIndex(JUNO_MAP_T *ptMap, JUNO_MEMORY_T tKey, size_t *pzRetSize, bool bShallEqual)
{
    size_t zHash = 0;
    JUNO_STATUS_T tStatus = ptMap->ptHashApi->Hash((const uint8_t *)(tKey.pvAddr), tKey.zSize, &zHash);
    ASSERT_SUCCESS(tStatus, return tStatus);
    // Get the capacity
    size_t zCapacity = ptMap->zCapacity;
    tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
    // Calculate the index
    size_t zIndex = zHash % zCapacity;
    // Set the initial status
    // Iterate over the table
    for (size_t i = 0; i < zCapacity; i++)
    {
        // Get a pointer to the current key
        JUNO_MEMORY_T tCurKey = ptMap->ptMapKeys[zIndex];
        // Check if the spot is empty or the key is equal
        if((!tCurKey.pvAddr && !bShallEqual) || (tCurKey.pvAddr && tKey.pvAddr && ptMap->pfcnIsEqual(tCurKey, tKey)))
        {
            // Return the index
            *pzRetSize = zIndex;
            tStatus = JUNO_STATUS_SUCCESS;
            ptMap->zLenHashTable += 1;
            break;
        }
        zIndex = (zHash + i) % zCapacity;
    }
    if(tStatus != JUNO_STATUS_SUCCESS)
    {
        FAIL(tStatus, ptMap->pfcnFailureHandler, ptMap->pvFailureUserData, "Map is full");
    }
    // Return the status
    return tStatus;
}

JUNO_STATUS_T Juno_MapSet(JUNO_MAP_T *ptMap, JUNO_MEMORY_T tKey, JUNO_MEMORY_T tValue)
{
    JUNO_STATUS_T tStatus = Juno_MapValidate(ptMap);
    ASSERT_SUCCESS(tStatus, return tStatus);
    size_t zIndex = 0;
    tStatus = Juno_MapGetIndex(ptMap, tKey, &zIndex, false);
    ASSERT_SUCCESS(tStatus, return tStatus);
    ptMap->ptMapKeys[zIndex] = tKey;
    ptMap->ptMapValues[zIndex] = tValue;
    return tStatus;
}

JUNO_STATUS_T Juno_MapRemove(JUNO_MAP_T *ptMap, JUNO_MEMORY_T tKey)
{
    JUNO_STATUS_T tStatus = Juno_MapValidate(ptMap);
    ASSERT_SUCCESS(tStatus, return tStatus);
    size_t zIndex = 0;
    tStatus = Juno_MapGetIndex(ptMap, tKey, &zIndex, true);
    ASSERT_SUCCESS(tStatus, return tStatus);
    ptMap->ptMapKeys[zIndex] = (JUNO_MEMORY_T){0};
    ptMap->ptMapValues[zIndex] = (JUNO_MEMORY_T){0};
    return tStatus;
}


JUNO_STATUS_T Juno_MapGet(JUNO_MAP_T *ptMap, JUNO_MEMORY_T tKey, JUNO_MEMORY_T *ptRetValue)
{
    JUNO_STATUS_T tStatus = Juno_MapValidate(ptMap);
    ASSERT_SUCCESS(tStatus, return tStatus);
    size_t zIndex = 0;
    tStatus = Juno_MapGetIndex(ptMap, tKey, &zIndex, true);
    ASSERT_SUCCESS(tStatus, return tStatus);
    if(!ptMap->ptMapKeys[zIndex].pvAddr)
    {
        tStatus = JUNO_STATUS_DNE_ERROR;
        FAIL(tStatus, ptMap->pfcnFailureHandler, ptMap->pvFailureUserData, "Key Does not exist");
        return tStatus;
    }
    *ptRetValue = ptMap->ptMapValues[zIndex];
    return tStatus;
}

static const JUNO_MAP_API_T tJunoMapApi =
{
    .Init = Juno_MapInit,
    .Set = Juno_MapSet,
    .Remove = Juno_MapRemove,
    .Get = Juno_MapGet
};

const JUNO_MAP_API_T * Juno_MapApi(void)
{
    return &tJunoMapApi;
}
