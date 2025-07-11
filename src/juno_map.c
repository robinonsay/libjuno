/*
    MIT License

    Copyright (c) 2025 Robin A. Onsay

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
#include "juno/map/map_impl.h"
#include "juno/status.h"
#include "juno/macros.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static const JUNO_MAP_API_T tJunoMapImplApi;

static inline JUNO_STATUS_T Verify(JUNO_MAP_T *ptJunoMap)
{
    ASSERT_EXISTS(ptJunoMap);
    JUNO_MAP_IMPL_T *ptJunoMapImpl = (JUNO_MAP_IMPL_T *)(ptJunoMap);
    ASSERT_EXISTS_MODULE(
        ptJunoMap &&
        ptJunoMapImpl->JUNO_MODULE_SUPER.ptApi &&
        ptJunoMapImpl->JUNO_MODULE_SUPER.ptMapKeys &&
        ptJunoMapImpl->JUNO_MODULE_SUPER.ptHash &&
        ptJunoMapImpl->JUNO_MODULE_SUPER.ptMapValues &&
        ptJunoMapImpl->JUNO_MODULE_SUPER.zCapacity &&
        ptJunoMapImpl->JUNO_MODULE_SUPER.pfcnIsEqual,
        ptJunoMapImpl,
        "Module does not have all dependencies"
    );
    if(ptJunoMapImpl->JUNO_MODULE_SUPER.ptApi != &tJunoMapImplApi)
    {
        JUNO_FAIL_MODULE(JUNO_STATUS_INVALID_TYPE_ERROR, ptJunoMapImpl, "Module has invalid API");
        return JUNO_STATUS_INVALID_TYPE_ERROR;
    }
    return JUNO_STATUS_SUCCESS;
}

static inline JUNO_STATUS_T Juno_MapGetIndex(JUNO_MAP_T *ptJunoMap, JUNO_MEMORY_T tKey, size_t *pzRetSize, bool bShallEqual)
{

    JUNO_MAP_IMPL_T *ptMap = (JUNO_MAP_IMPL_T *)(ptJunoMap);
    size_t zHash = 0;
    JUNO_HASH_ROOT_T *ptHash = (JUNO_HASH_ROOT_T *)(ptMap->tRoot.ptHash);
    JUNO_STATUS_T tStatus = ptHash->ptApi->Hash(ptMap->tRoot.ptHash, (const uint8_t *)(tKey.pvAddr), tKey.zSize, &zHash);
    ASSERT_SUCCESS(tStatus, return tStatus);
    // Get the capacity
    size_t zCapacity = ptMap->tRoot.zCapacity;
    tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
    // Calculate the index
    size_t zIndex = zHash % zCapacity;
    // Set the initial status
    // Iterate over the table
    for (size_t i = 0; i < zCapacity; i++)
    {
        // Get a pointer to the current key
        JUNO_MEMORY_T tCurKey = ptMap->tRoot.ptMapKeys[zIndex];
        // Check if the spot is empty or the key is equal
        if((!tCurKey.pvAddr && !bShallEqual) || (tCurKey.pvAddr && tKey.pvAddr && ptMap->tRoot.pfcnIsEqual(tCurKey, tKey)))
        {
            // Return the index
            *pzRetSize = zIndex;
            tStatus = JUNO_STATUS_SUCCESS;
            ptMap->tRoot.zLenHashTable += 1;
            break;
        }
        zIndex = (zHash + i) % zCapacity;
    }
    if(tStatus != JUNO_STATUS_SUCCESS)
    {
        JUNO_FAIL_MODULE(tStatus, ptMap, "Map is full");
    }
    // Return the status
    return tStatus;
}

static JUNO_STATUS_T Juno_MapSet(JUNO_MAP_T *ptJunoMap, JUNO_MEMORY_T tKey, JUNO_MEMORY_T tValue)
{
    JUNO_MAP_IMPL_T *ptMap = (JUNO_MAP_IMPL_T *)(ptJunoMap);
    JUNO_STATUS_T tStatus = Verify(ptJunoMap);
    ASSERT_SUCCESS(tStatus, return tStatus);
    size_t zIndex = 0;
    tStatus = Juno_MapGetIndex(ptJunoMap, tKey, &zIndex, false);
    ASSERT_SUCCESS(tStatus, return tStatus);
    ptMap->tRoot.ptMapKeys[zIndex] = tKey;
    ptMap->tRoot.ptMapValues[zIndex] = tValue;
    return tStatus;
}

static JUNO_STATUS_T Juno_MapRemove(JUNO_MAP_T *ptJunoMap, JUNO_MEMORY_T tKey)
{
    JUNO_MAP_IMPL_T *ptMap = (JUNO_MAP_IMPL_T *)(ptJunoMap);
    JUNO_STATUS_T tStatus = Verify(ptJunoMap);
    ASSERT_SUCCESS(tStatus, return tStatus);
    size_t zIndex = 0;
    tStatus = Juno_MapGetIndex(ptJunoMap, tKey, &zIndex, true);
    ASSERT_SUCCESS(tStatus, return tStatus);
    ptMap->tRoot.ptMapKeys[zIndex] = (JUNO_MEMORY_T){0};
    ptMap->tRoot.ptMapValues[zIndex] = (JUNO_MEMORY_T){0};
    return tStatus;
}


static JUNO_STATUS_T Juno_MapGet(JUNO_MAP_T *ptJunoMap, JUNO_MEMORY_T tKey, JUNO_MEMORY_T *ptRetValue)
{
    JUNO_MAP_IMPL_T *ptMap = (JUNO_MAP_IMPL_T *)(ptJunoMap);
    JUNO_STATUS_T tStatus = Verify(ptJunoMap);
    ASSERT_SUCCESS(tStatus, return tStatus);
    size_t zIndex = 0;
    tStatus = Juno_MapGetIndex(ptJunoMap, tKey, &zIndex, true);
    ASSERT_SUCCESS(tStatus, return tStatus);
    if(!ptMap->tRoot.ptMapKeys[zIndex].pvAddr)
    {
        tStatus = JUNO_STATUS_DNE_ERROR;
        JUNO_FAIL_MODULE(tStatus, ptMap, "Key Does not exist");
        return tStatus;
    }
    *ptRetValue = ptMap->tRoot.ptMapValues[zIndex];
    return tStatus;
}

static const JUNO_MAP_API_T tJunoMapImplApi =
{
    .Set = Juno_MapSet,
    .Remove = Juno_MapRemove,
    .Get = Juno_MapGet
};

JUNO_STATUS_T JunoMap_ImplApi(
    JUNO_MAP_T *ptJunoMap,
    JUNO_HASH_T *ptHash,
    JUNO_MEMORY_T *ptKeyTable,
    JUNO_MEMORY_T *ptValueTable,
    size_t zCapacity,
    JUNO_MAP_KEY_EQUAL_FCN_T pfcnIsEqual,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
)
{
    ASSERT_EXISTS(ptJunoMap);
    JUNO_MAP_IMPL_T *ptJunoMapImpl = (JUNO_MAP_IMPL_T *)(ptJunoMap);
    ptJunoMapImpl->JUNO_MODULE_SUPER.ptApi = &tJunoMapImplApi;
    ptJunoMapImpl->JUNO_MODULE_SUPER.JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    ptJunoMapImpl->JUNO_MODULE_SUPER.JUNO_FAILURE_USER_DATA = pvFailureUserData;
    ptJunoMapImpl->JUNO_MODULE_SUPER.ptHash = ptHash;
    ptJunoMapImpl->JUNO_MODULE_SUPER.ptMapKeys = ptKeyTable;
    ptJunoMapImpl->JUNO_MODULE_SUPER.ptMapValues = ptValueTable;
    ptJunoMapImpl->JUNO_MODULE_SUPER.zCapacity = zCapacity;
    ptJunoMapImpl->JUNO_MODULE_SUPER.zLenHashTable = 0;
    ptJunoMapImpl->JUNO_MODULE_SUPER.pfcnIsEqual = pfcnIsEqual;
    JUNO_STATUS_T tStatus = Verify(ptJunoMap);
    ASSERT_SUCCESS(tStatus, return tStatus);
    memset(ptKeyTable, 0, sizeof(JUNO_MEMORY_T) * zCapacity);
    memset(ptValueTable, 0, sizeof(JUNO_MEMORY_T) * zCapacity);
    return tStatus;
}
