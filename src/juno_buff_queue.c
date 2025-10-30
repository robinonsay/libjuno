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
#include "juno/ds/queue_api.h"
#include "juno/macros.h"
#include "juno/status.h"

/// Enqueue an item on the queue
JUNO_STATUS_T JunoDs_QueuePush(JUNO_DS_QUEUE_ROOT_T *ptQueue, JUNO_POINTER_T tItem)
{
    JUNO_ASSERT_EXISTS(ptQueue);
    JUNO_STATUS_T tStatus = JunoDs_QueueVerify(ptQueue);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_DS_ARRAY_ROOT_T *ptBuffer = ptQueue->ptQueueArray;
    if(ptQueue->zLength < ptBuffer->zCapacity)
    {
        size_t iIndex = (ptQueue->iStartIndex + ptQueue->zLength) % ptBuffer->zCapacity;
        tStatus = ptBuffer->ptApi->SetAt(ptBuffer, tItem, iIndex);
        JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
        ptQueue->zLength += 1;
    }
    else
    {
        tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        JUNO_FAIL_ROOT(tStatus, ptQueue, "Failed to enqueue data");
        return tStatus;
    }
    return tStatus;
}

/// Dequeue an item from the queue
JUNO_STATUS_T JunoDs_QueuePop(JUNO_DS_QUEUE_ROOT_T *ptQueue, JUNO_POINTER_T tReturn)
{
    JUNO_STATUS_T tStatus = JunoDs_QueueVerify(ptQueue);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerify(tReturn);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_DS_ARRAY_ROOT_T *ptBuffer = ptQueue->ptQueueArray;
    if(ptQueue->zLength > 0)
    {
        const JUNO_DS_ARRAY_API_T *ptApi = ptBuffer->ptApi;
        size_t iDequeueIndex = ptQueue->iStartIndex;
        JUNO_RESULT_POINTER_T tPtrResult = ptApi->GetAt(ptBuffer, iDequeueIndex);
        JUNO_ASSERT_SUCCESS(tPtrResult.tStatus, return tPtrResult.tStatus);
        tStatus = tPtrResult.tOk.ptApi->Copy(tReturn, JUNO_OK(tPtrResult));
        JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
        tStatus = ptApi->RemoveAt(ptBuffer, iDequeueIndex);
        JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
        ptQueue->iStartIndex = (ptQueue->iStartIndex + 1) % ptBuffer->zCapacity;
        ptQueue->zLength -= 1;
        return tStatus;
    }
    tStatus = JUNO_STATUS_ERR;
    JUNO_FAIL_ROOT(tStatus, ptQueue, "Queue is empty");
    return tStatus;
}

/// Peek at the next item in the queue
JUNO_RESULT_POINTER_T JunoDs_QueuePeek(JUNO_DS_QUEUE_ROOT_T *ptQueue)
{
    JUNO_RESULT_POINTER_T tResult = JUNO_ERR_RESULT(JUNO_STATUS_ERR, {0});
    tResult.tStatus = JunoDs_QueueVerify(ptQueue);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    JUNO_DS_ARRAY_ROOT_T *ptBuffer = ptQueue->ptQueueArray;
    if(ptQueue->zLength == 0)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        JUNO_FAIL_ROOT(tResult.tStatus, ptQueue, "Queue is empty");
        return tResult;
    }
    tResult = ptBuffer->ptApi->GetAt(ptBuffer, ptQueue->iStartIndex);
    return tResult;
}

static const JUNO_DS_QUEUE_API_T gtQueueApi =
{
    /// Enqueue an item on the queue
    JunoDs_QueuePush,
    /// Dequeue an item from the queue
    JunoDs_QueuePop,
    /// Peek at the next item in the queue
    JunoDs_QueuePeek,
};

/// Initialize a buffer queue with a capacity
JUNO_STATUS_T JunoDs_QueueInit(JUNO_DS_QUEUE_ROOT_T *ptQueue, JUNO_DS_ARRAY_ROOT_T *ptQueueArray, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvFailureUserData)
{
    JUNO_ASSERT_EXISTS(ptQueue && ptQueueArray);
    ptQueue->ptApi = &gtQueueApi;
    ptQueue->ptQueueArray = ptQueueArray;
    ptQueue->iStartIndex = 0;
    ptQueue->zLength = 0;
    ptQueue->_pfcnFailureHandler = pfcnFailureHdlr;
    ptQueue->_pvFailureUserData = pvFailureUserData;
    return JunoDs_QueueVerify(ptQueue);
}
