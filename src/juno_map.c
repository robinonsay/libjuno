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
#include "juno/macros.h"
#include "juno/ds/map_api.h"
#include "juno/memory/memory_api.h"
#include "juno/status.h"
#include "juno/macros.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

JUNO_RESULT_SIZE_T JunoMap_GetIndex(JUNO_MAP_ROOT_T *ptJunoMap, JUNO_POINTER_T tKey)
{
    JUNO_RESULT_SIZE_T tResult = {0, 0};
    tResult.tStatus = JunoMap_Verify(ptJunoMap);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    tResult.tStatus = JunoMemory_PointerVerify(tKey);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    const JUNO_MAP_BUFFER_API_T *ptApi = ptJunoMap->ptBufferApi;
    tResult = ptApi->Hash(tKey);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    size_t iHash = tResult.tOk;
    // Get the capacity
    size_t zCapacity = ptJunoMap->zCapacity;
    tResult.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
    // Calculate the index
    // Set the initial status
    // Iterate over the table
    for (size_t i = 0; i < zCapacity; i++)
    {
        size_t zIndex = (iHash + i) % zCapacity;
        // Get a pointer to the current key

        JUNO_RESULT_BOOL_T tBoolResult = ptApi->IsEmpty( zIndex);
        tResult.tStatus = tBoolResult.tStatus;
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
        bool bIsEmpty = tBoolResult.tOk;
        // Check if the spot is empty or the key is equal
        if(bIsEmpty)
        {
            // Return the index
            tResult.tOk = zIndex;
            tResult.tStatus = JUNO_STATUS_SUCCESS;
            break;
        }
        JUNO_RESULT_POINTER_T tPtrResult = ptApi->GetKey(zIndex);
        tResult.tStatus = tPtrResult.tStatus;
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
        JUNO_POINTER_T tAddr = tPtrResult.tOk;
        tBoolResult = ptApi->KeyIsEqual(tKey, tAddr);
        tResult.tStatus = tBoolResult.tStatus;
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
        bool bIsEqual = tBoolResult.tOk;
        if(bIsEqual)
        {
            // Return the index
            tResult.tOk = zIndex;
            tResult.tStatus = JUNO_STATUS_SUCCESS;
            break;
        }
        tResult.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
    }
    // Return the status
    return tResult;
}


/// Set the value at a given key
static inline JUNO_STATUS_T JunoMap_Set(JUNO_MAP_ROOT_T *ptJunoMap, JUNO_POINTER_T tKey, JUNO_POINTER_T tValue)
{
    // verify the map
    JUNO_STATUS_T tStatus = JunoMap_Verify(ptJunoMap);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerify(tKey);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerify(tValue);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Get the index for the key
    JUNO_RESULT_SIZE_T tSizeResult = JunoMap_GetIndex(ptJunoMap, tKey);
    tStatus = tSizeResult.tStatus;
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    const JUNO_MAP_BUFFER_API_T *ptApi = ptJunoMap->ptBufferApi;
    // Set the key for the index
    tStatus = ptApi->SetKey(tSizeResult.tOk, tKey);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Set the value for the index
    tStatus = ptApi->SetValue(tSizeResult.tOk, tValue);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}

/// Get the value for a given key
static inline JUNO_RESULT_POINTER_T JunoMap_Get(JUNO_MAP_ROOT_T *ptJunoMap, JUNO_POINTER_T tKey)
{
    JUNO_RESULT_POINTER_T tResult = {0, {0}};
    tResult.tStatus = JunoMap_Verify(ptJunoMap);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    tResult.tStatus = JunoMemory_PointerVerify(tKey);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    // Get the index for the key
    JUNO_RESULT_SIZE_T tSizeResult = JunoMap_GetIndex(ptJunoMap, tKey);
    tResult.tStatus = tSizeResult.tStatus;
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    const JUNO_MAP_BUFFER_API_T *ptApi = ptJunoMap->ptBufferApi;
    // Check if the value at the index is empty
    JUNO_RESULT_BOOL_T tBoolResult = ptApi->IsEmpty(tSizeResult.tOk);
    tResult.tStatus = tBoolResult.tStatus;
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    bool bIsEmpty = tBoolResult.tOk;
    if(bIsEmpty)
    {
        // Its empty so return DNE error
        tResult.tStatus = JUNO_STATUS_DNE_ERROR;
        return tResult;
    }
    // Get the value
    return ptApi->GetValue(tSizeResult.tOk);
}

/// Remove the key and value ata given key
static inline JUNO_STATUS_T JunoMap_Remove(JUNO_MAP_ROOT_T *ptJunoMap, JUNO_POINTER_T tKey)
{
    JUNO_STATUS_T tStatus = JunoMap_Verify(ptJunoMap);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerify(tKey);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Get the index for the given key
    JUNO_RESULT_SIZE_T tSizeResult = JunoMap_GetIndex(ptJunoMap, tKey);
    tStatus = tSizeResult.tStatus;
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    const JUNO_MAP_BUFFER_API_T *ptApi = ptJunoMap->ptBufferApi;
    // Check if the key is empty
    JUNO_RESULT_BOOL_T tBoolResult = ptApi->IsEmpty(tSizeResult.tOk);
    tStatus = tBoolResult.tStatus;
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    bool bIsEmpty = tBoolResult.tOk;
    if(bIsEmpty)
    {
        // Its empty so fail silently
        return tStatus;
    }
    //  Remove the key, value at the index
    tStatus = ptApi->Remove(tSizeResult.tOk);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}

static const JUNO_MAP_API_T gtJunoMapApi =
{
    JunoMap_Get,
    JunoMap_Set,
    JunoMap_Remove
};


JUNO_STATUS_T JunoMap_Init(JUNO_MAP_ROOT_T *ptMapRoot, const JUNO_MAP_BUFFER_API_T *ptBufferApi, size_t zCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHandler, JUNO_USER_DATA_T *pvUserData)
{
    JUNO_ASSERT_EXISTS(ptMapRoot);
    ptMapRoot->ptApi = &gtJunoMapApi;
    ptMapRoot->ptBufferApi = ptBufferApi;
    ptMapRoot->zCapacity = zCapacity;
    ptMapRoot->_pfcnFailureHandler = pfcnFailureHandler;
    ptMapRoot->_pvFailureUserData = pvUserData;
    return JunoMap_Verify(ptMapRoot);
}
