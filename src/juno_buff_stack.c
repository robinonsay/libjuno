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
#include "juno/ds/stack_api.h"
#include "juno/macros.h"
#include "juno/memory/memory_api.h"
#include "juno/status.h"


/// Enqueue an item into the buffer
/// @returns The index to place the enqueued item
JUNO_STATUS_T JunoDs_StackPush(JUNO_DS_STACK_ROOT_T *ptStack, JUNO_POINTER_T tItem)
{
    JUNO_STATUS_T tStatus = JunoDs_StackVerify(ptStack);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_DS_ARRAY_ROOT_T *ptBuffer = ptStack->ptStackArray;
    if(ptStack->zLength < ptBuffer->zCapacity)
    {
        tStatus = ptBuffer->ptApi->SetAt(ptBuffer, tItem, ptStack->zLength);
        JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
        ptStack->zLength += 1;
    }
    else
    {
        tStatus = JUNO_STATUS_OOB_ERROR;
        JUNO_FAIL_ROOT(tStatus, ptStack, "Failed to enqueue data");
    }
    return tStatus;
}


/// Dequeue an item from the buffer
/// @returns The index to dequeue the item from
JUNO_STATUS_T JunoDs_StackPop(JUNO_DS_STACK_ROOT_T *ptStack, JUNO_POINTER_T tReturn)
{
    JUNO_STATUS_T tStatus = JunoDs_StackVerify(ptStack);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerify(tReturn);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_DS_STACK_ROOT_T *ptStackRoot = (JUNO_DS_STACK_ROOT_T *)(ptStack);
    JUNO_DS_ARRAY_ROOT_T *ptBuffer = ptStack->ptStackArray;
    if(ptStack->zLength > 0)
    {
        ptStack->zLength -= 1;
        JUNO_RESULT_POINTER_T tResult = ptBuffer->ptApi->GetAt(ptBuffer, ptStack->zLength);
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult.tStatus);
        tStatus = tReturn.ptApi->Copy(tReturn, tResult.tOk);
        return tStatus;
    }
    tStatus = JUNO_STATUS_OOB_ERROR;
    JUNO_FAIL_ROOT(tStatus, ptStackRoot, "Failed to enqueue data");
    return tStatus;
}


/// Peek at the next item in the queue
/// @returns the index of the next item in the queue
JUNO_RESULT_POINTER_T JunoDs_StackPeek(JUNO_DS_STACK_ROOT_T *ptStack)
{
    JUNO_RESULT_POINTER_T tResult = JUNO_ERR_RESULT(JUNO_STATUS_ERR, {0});
    tResult.tStatus = JunoDs_StackVerify(ptStack);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    if(ptStack->zLength == 0)
    {
        tResult.tStatus = JUNO_STATUS_OOB_ERROR;
        JUNO_FAIL_ROOT(tResult.tStatus, ptStack, "Queue is empty");
        return tResult;
    }
    tResult = ptStack->ptStackArray->ptApi->GetAt(ptStack->ptStackArray, ptStack->zLength - 1);
    return tResult;
}

static const JUNO_DS_STACK_API_T gtStackApi =
{
    JunoDs_StackPush,
    JunoDs_StackPop,
    JunoDs_StackPeek,
};

/// Initialize a buffer queue with a capacity
JUNO_STATUS_T JunoDs_StackInit(JUNO_DS_STACK_ROOT_T *ptStack, JUNO_DS_ARRAY_ROOT_T *ptStackArray, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvFailureUserData)
{
    JUNO_ASSERT_EXISTS(ptStack);
    ptStack->ptApi = &gtStackApi;
    ptStack->ptStackArray = ptStackArray;
    ptStack->zLength = 0;
    ptStack->_pfcnFailureHandler = pfcnFailureHdlr;
    ptStack->_pvFailureUserData = pvFailureUserData;
    return JunoDs_StackVerify(ptStack);
}
