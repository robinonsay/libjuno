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
#include "juno/memory/memory_api.h"
#include "juno/module.h"
#include "juno/status.h"
#include "juno/macros.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


JUNO_STATUS_T JunoDs_MapInit(JUNO_MAP_T *ptMap, const JUNO_MAP_API_T *ptApi, size_t zCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHandler, JUNO_USER_DATA_T *pvUserData)
{
    JUNO_ASSERT_EXISTS(ptMap);
    ptMap->ptApi = ptApi;
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(&ptMap->tRoot, &ptApi->tRoot, zCapacity, pfcnFailureHandler, pvUserData); 
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoDs_MapVerify(ptMap);
    return tStatus;
}

static JUNO_RESULT_MAP_HASHABLE_POINTER_T JunoDs_MapGetWithKey(JUNO_MAP_T *ptJunoMap, JUNO_MAP_HASHABLE_POINTER_T tItem)
{
    JUNO_RESULT_MAP_HASHABLE_POINTER_T tResult = {0};
    tResult.tStatus = JunoDs_MapVerify(ptJunoMap);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    tResult.tStatus = JunoDs_MapHashablePointerVerify(tItem);
    const JUNO_MAP_HASHABLE_POINTER_API_T *ptPointerApi = tItem.ptApi;
    JUNO_RESULT_SIZE_T tHashResult = ptPointerApi->Hash(tItem);
    tResult.tStatus = tHashResult.tStatus;
    JUNO_ASSERT_SUCCESS(tHashResult.tStatus, return tResult);
    size_t iHash = tHashResult.tOk;
    // Get the capacity
    size_t zCapacity = ptJunoMap->tRoot.zCapacity;
    tResult.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
    // Calculate the index
    // Set the initial status
    // Iterate over the table
    for (size_t i = 0; i < zCapacity; i++)
    {
        size_t zIndex = (iHash + i) % zCapacity;
        // Get a pointer to the current key
        JUNO_RESULT_POINTER_T tPtrResult = ptJunoMap->ptApi->tRoot.GetAt(&ptJunoMap->tRoot, zIndex);
        tResult.tStatus = tPtrResult.tStatus;
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
        if((const JUNO_MAP_HASHABLE_POINTER_API_T *)tPtrResult.tOk.ptApi != tItem.ptApi)
        {
            tResult.tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
            JUNO_FAIL_MODULE(tResult.tStatus, ptJunoMap, "Invalid pointer type");
            return tResult;
        }
        JUNO_MAP_HASHABLE_POINTER_T tAddr = {0};
        tAddr.tRoot.tRoot = tPtrResult.tOk;
        JUNO_RESULT_BOOL_T tBoolResult = ptPointerApi->IsNull(tAddr);
        tResult.tStatus = tBoolResult.tStatus;
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
        bool bIsEmpty = tBoolResult.tOk;
        // Check if the spot is empty or the key is equal
        if(bIsEmpty)
        {
            // Return the index
            tResult.tOk.tRoot.tRoot = tPtrResult.tOk;
            tResult.tStatus = JUNO_STATUS_SUCCESS;
            break;
        }
        tBoolResult = ptPointerApi->tRoot.Equals(tItem.tRoot, tAddr.tRoot);
        tResult.tStatus = tBoolResult.tStatus;
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
        bool bIsEqual = tBoolResult.tOk;
        if(bIsEqual)
        {
            // Return the index
            tResult.tOk.tRoot.tRoot = tPtrResult.tOk;
            tResult.tStatus = JUNO_STATUS_SUCCESS;
            break;
        }
        tResult.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
    }
    // Return the status
    return tResult;
}

JUNO_RESULT_MAP_HASHABLE_POINTER_T JunoDs_MapGet(JUNO_MAP_T *ptJunoMap, JUNO_MAP_HASHABLE_POINTER_T tItem)
{
    JUNO_RESULT_MAP_HASHABLE_POINTER_T tResult = {0};
    tResult.tStatus = JunoDs_MapVerify(ptJunoMap);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    tResult.tStatus = JunoDs_MapHashablePointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    // Get the index for the key
    tResult = JunoDs_MapGetWithKey(ptJunoMap, tItem);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    // Check if the value at the index is empty
    JUNO_RESULT_BOOL_T tBoolResult = tResult.tOk.ptApi->IsNull(tResult.tOk);
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

JUNO_STATUS_T JunoDs_MapSet(JUNO_MAP_T *ptJunoMap, JUNO_MAP_HASHABLE_POINTER_T tItem)
{
    JUNO_STATUS_T tStatus = JunoDs_MapVerify(ptJunoMap);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoDs_MapHashablePointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Get the index for the key
    JUNO_RESULT_MAP_HASHABLE_POINTER_T tItemResult = JunoDs_MapGetWithKey(ptJunoMap, tItem);
    tStatus = tItemResult.tStatus;
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    if(tItem.ptApi != tItemResult.tOk.ptApi)
    {
        tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        JUNO_FAIL_MODULE(tStatus, ptJunoMap, "Invalid map type");
        return tStatus;
    }
    tItem.ptApi->tRoot.tRoot.Copy(tItemResult.tOk.tRoot.tRoot, tItem.tRoot.tRoot);
    return tStatus;
}

JUNO_STATUS_T JunoDs_MapRemove(JUNO_MAP_T *ptJunoMap, JUNO_MAP_HASHABLE_POINTER_T tItem)
{
    JUNO_RESULT_MAP_HASHABLE_POINTER_T tResult = {0};
    tResult.tStatus = JunoDs_MapVerify(ptJunoMap);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult.tStatus);
    tResult.tStatus = JunoDs_MapHashablePointerVerify(tItem);
    const JUNO_MAP_HASHABLE_POINTER_API_T *ptPointerApi = tItem.ptApi;
    JUNO_RESULT_SIZE_T tHashResult = ptPointerApi->Hash(tItem);
    tResult.tStatus = tHashResult.tStatus;
    JUNO_ASSERT_SUCCESS(tHashResult.tStatus, return tResult.tStatus);
    size_t iHash = tHashResult.tOk;
    // Get the capacity
    size_t zCapacity = ptJunoMap->tRoot.zCapacity;
    tResult.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
    // Calculate the index
    // Set the initial status
    // Iterate over the table
    for (size_t i = 0; i < zCapacity; i++)
    {
        size_t zIndex = (iHash + i) % zCapacity;
        JUNO_RESULT_POINTER_T tPtrResult = ptJunoMap->ptApi->tRoot.GetAt(&ptJunoMap->tRoot, zIndex);
        tResult.tStatus = tPtrResult.tStatus;
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult.tStatus);
        if((const JUNO_MAP_HASHABLE_POINTER_API_T *)tPtrResult.tOk.ptApi != tItem.ptApi)
        {
            tResult.tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
            JUNO_FAIL_MODULE(tResult.tStatus, ptJunoMap, "Invalid pointer type");
            return tResult.tStatus;
        }
        JUNO_MAP_HASHABLE_POINTER_T tAddr = {0};
        tAddr.tRoot.tRoot = tPtrResult.tOk;
        // Get a pointer to the current key
        JUNO_RESULT_BOOL_T tBoolResult = ptPointerApi->IsNull(tAddr);
        tResult.tStatus = tBoolResult.tStatus;
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult.tStatus);
        bool bIsEmpty = tBoolResult.tOk;
        // Check if the spot is empty or the key is equal
        if(bIsEmpty)
        {
            break;
        }
        tBoolResult = ptPointerApi->tRoot.Equals(tItem.tRoot, tAddr.tRoot);
        tResult.tStatus = tBoolResult.tStatus;
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult.tStatus);
        bool bIsEqual = tBoolResult.tOk;
        if(bIsEqual)
        {
            // Return the index
            tResult.tStatus = ptJunoMap->ptApi->tRoot.RemoveAt(&ptJunoMap->tRoot, zIndex);
            break;
        }
        tResult.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
    }
    // Return the status
    return tResult.tStatus;
}
