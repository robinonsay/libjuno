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
#include "juno/ds/array_api.h"
#include "juno/macros.h"
#include "juno/ds/map_api.h"
#include "juno/memory/pointer_api.h"
#include "juno/status.h"
#include "juno/macros.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static const JUNO_MAP_API_T gtMapApi =
{
    JunoDs_MapGet,
    JunoDs_MapSet,
    JunoDs_MapRemove,
};

JUNO_STATUS_T JunoDs_MapInit(JUNO_MAP_ROOT_T *ptMapRoot, const JUNO_MAP_HASHABLE_POINTER_API_T *ptHashablePointerApi, const JUNO_VALUE_POINTER_API_T *ptValuePointerApi, JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_FAILURE_HANDLER_T pfcnFailureHandler, JUNO_USER_DATA_T *pvUserData)
{
    JUNO_ASSERT_EXISTS(ptMapRoot);
    ptMapRoot->ptApi = &gtMapApi;
    ptMapRoot->ptHashablePointerApi = ptHashablePointerApi;
    ptMapRoot->ptValuePointerApi = ptValuePointerApi;
    ptMapRoot->ptHashMap = ptArray;
    ptMapRoot->_pfcnFailureHandler = pfcnFailureHandler;
    ptMapRoot->_pvFailureUserData = pvUserData;
    JUNO_STATUS_T tStatus = JunoDs_MapVerify(ptMapRoot);
    return tStatus;
}

static JUNO_RESULT_POINTER_T JunoDs_MapGetWithKey(JUNO_MAP_ROOT_T *ptJunoMap, JUNO_POINTER_T tItem)
{
    JUNO_RESULT_POINTER_T tResult = {0};
    tResult.tStatus = JunoDs_MapVerify(ptJunoMap);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    tResult.tStatus = JunoMemory_PointerVerify(tItem);
    const JUNO_MAP_HASHABLE_POINTER_API_T *ptPointerApi = ptJunoMap->ptHashablePointerApi;
    JUNO_RESULT_SIZE_T tHashResult = ptPointerApi->Hash(tItem);
    tResult.tStatus = tHashResult.tStatus;
    JUNO_ASSERT_SUCCESS(tHashResult.tStatus, return tResult);
    size_t iHash = tHashResult.tOk;
    // Get the capacity
    JUNO_DS_ARRAY_ROOT_T *ptHashMap = ptJunoMap->ptHashMap;
    const JUNO_DS_ARRAY_API_T *ptArrayApi = ptHashMap->ptApi;
    size_t zCapacity = ptHashMap->zCapacity;
    tResult.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
    // Calculate the index
    // Set the initial status
    // Iterate over the table
    for (size_t i = 0; i < zCapacity; i++)
    {
        size_t zIndex = (iHash + i) % zCapacity;
        // Get a pointer to the current key
        JUNO_RESULT_POINTER_T tPtrResult = ptArrayApi->GetAt(ptHashMap, zIndex);
        tResult.tStatus = tPtrResult.tStatus;
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
        if((const JUNO_POINTER_API_T *)tPtrResult.tOk.ptApi != tItem.ptApi)
        {
            tResult.tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
            JUNO_FAIL_ROOT(tResult.tStatus, ptJunoMap, "Invalid pointer type");
            return tResult;
        }
        JUNO_RESULT_BOOL_T tBoolResult = ptPointerApi->IsValueNull(tPtrResult.tOk);
        tResult.tStatus = tBoolResult.tStatus;
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
        bool bIsEmpty = tBoolResult.tOk;
        // Check if the spot is empty or the key is equal
        if(bIsEmpty)
        {
            // Return the index
            tResult.tOk = tPtrResult.tOk;
            tResult.tStatus = JUNO_STATUS_SUCCESS;
            break;
        }
        tBoolResult = ptJunoMap->ptValuePointerApi->Equals(tItem, tPtrResult.tOk);
        tResult.tStatus = tBoolResult.tStatus;
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
        bool bIsEqual = tBoolResult.tOk;
        if(bIsEqual)
        {
            // Return the index
            tResult.tOk = tPtrResult.tOk;
            tResult.tStatus = JUNO_STATUS_SUCCESS;
            break;
        }
        tResult.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
    }
    // Return the status
    return tResult;
}

JUNO_RESULT_POINTER_T JunoDs_MapGet(JUNO_MAP_ROOT_T *ptJunoMap, JUNO_POINTER_T tItem)
{
    JUNO_RESULT_POINTER_T tResult = {0};
    tResult.tStatus = JunoDs_MapVerify(ptJunoMap);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    tResult.tStatus = JunoMemory_PointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    // Get the index for the key
    tResult = JunoDs_MapGetWithKey(ptJunoMap, tItem);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    // Check if the value at the index is empty
    JUNO_RESULT_BOOL_T tBoolResult = ptJunoMap->ptHashablePointerApi->IsValueNull(tResult.tOk);
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
    return tResult;
}

JUNO_STATUS_T JunoDs_MapSet(JUNO_MAP_ROOT_T *ptJunoMap, JUNO_POINTER_T tItem)
{
    JUNO_STATUS_T tStatus = JunoDs_MapVerify(ptJunoMap);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Get the index for the key
    JUNO_RESULT_POINTER_T tItemResult = JunoDs_MapGetWithKey(ptJunoMap, tItem);
    tStatus = tItemResult.tStatus;
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    if(tItem.ptApi != tItemResult.tOk.ptApi)
    {
        tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        JUNO_FAIL_ROOT(tStatus, ptJunoMap, "Invalid map type");
        return tStatus;
    }
    tStatus = tItem.ptApi->Copy(tItemResult.tOk, tItem);
    return tStatus;
}

JUNO_STATUS_T JunoDs_MapRemove(JUNO_MAP_ROOT_T *ptJunoMap, JUNO_POINTER_T tItem)
{
    JUNO_RESULT_POINTER_T tResult = {0};
    tResult.tStatus = JunoDs_MapVerify(ptJunoMap);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult.tStatus);
    tResult.tStatus = JunoMemory_PointerVerify(tItem);
    const JUNO_MAP_HASHABLE_POINTER_API_T *ptPointerApi = ptJunoMap->ptHashablePointerApi;
    JUNO_RESULT_SIZE_T tHashResult = ptPointerApi->Hash(tItem);
    tResult.tStatus = tHashResult.tStatus;
    JUNO_ASSERT_SUCCESS(tHashResult.tStatus, return tResult.tStatus);
    size_t iHash = tHashResult.tOk;
    // Get the capacity
    JUNO_DS_ARRAY_ROOT_T *ptHashMap = ptJunoMap->ptHashMap;
    const JUNO_DS_ARRAY_API_T *ptArrayApi = ptHashMap->ptApi;
    size_t zCapacity = ptHashMap->zCapacity;
    tResult.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
    // Calculate the index
    // Set the initial status
    // Iterate over the table
    for (size_t i = 0; i < zCapacity; i++)
    {
        size_t zIndex = (iHash + i) % zCapacity;
        JUNO_RESULT_POINTER_T tPtrResult = ptArrayApi->GetAt(ptHashMap, zIndex);
        tResult.tStatus = tPtrResult.tStatus;
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult.tStatus);
        if((const JUNO_POINTER_API_T *)tPtrResult.tOk.ptApi != tItem.ptApi)
        {
            tResult.tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
            JUNO_FAIL_ROOT(tResult.tStatus, ptJunoMap, "Invalid pointer type");
            return tResult.tStatus;
        }
        // Get a pointer to the current key
        JUNO_RESULT_BOOL_T tBoolResult = ptPointerApi->IsValueNull(tPtrResult.tOk);
        tResult.tStatus = tBoolResult.tStatus;
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult.tStatus);
        bool bIsEmpty = tBoolResult.tOk;
        // Check if the spot is empty or the key is equal
        if(bIsEmpty)
        {
            break;
        }
        tBoolResult = ptJunoMap->ptValuePointerApi->Equals(tItem, tPtrResult.tOk);
        tResult.tStatus = tBoolResult.tStatus;
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult.tStatus);
        bool bIsEqual = tBoolResult.tOk;
        if(bIsEqual)
        {
            // Return the index
            tResult.tStatus = ptArrayApi->RemoveAt(ptHashMap, zIndex);
            break;
        }
        tResult.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
    }
    // Return the status
    return tResult.tStatus;
}
