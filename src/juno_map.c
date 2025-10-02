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
#include "juno/status.h"
#include "juno/macros.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

JUNO_RESULT_SIZE_T JunoMap_GetIndex(JUNO_MAP_ROOT_T *ptJunoMap, void *ptKey)
{
    JUNO_RESULT_SIZE_T tResult = {0, 0};
    tResult.tStatus = JunoMap_Verify(ptJunoMap);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    const JUNO_MAP_API_T *ptApi = ptJunoMap->ptApi;
    tResult = ptApi->Hash(ptKey);
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
        JUNO_RESULT_VOID_PTR_T tPtrResult = ptApi->GetKey(zIndex);
        tResult.tStatus = tPtrResult.tStatus;
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
        void *pvAddr = tPtrResult.tOk;
        tBoolResult = ptApi->KeyIsEqual(ptKey, pvAddr);
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
