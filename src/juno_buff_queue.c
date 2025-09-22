#include "juno/ds/buff_queue_api.h"

/// Initialize a buffer queue with a capacity
JUNO_STATUS_T JunoDs_Buff_QueueInit(JUNO_BUFF_QUEUE_ROOT_T *ptQueue, const JUNO_BUFF_QUEUE_API_T *ptApi, size_t zCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvFailureUserData)
{
    JUNO_ASSERT_EXISTS(ptQueue);
    ptQueue->ptApi = ptApi;
    ptQueue->iStartIndex = 0;
    ptQueue->zLength = 0;
    ptQueue->zCapacity = zCapacity;
    ptQueue->_pfcnFailureHandler = pfcnFailureHdlr;
    ptQueue->_pvFailureUserData = pvFailureUserData;
    return JunoDs_Buff_QueueVerify(ptQueue);
}

/// Enqueue an item into the buffer
/// @returns The index to place the enqueued item
JUNO_STATUS_T JunoDs_Buff_QueueEnqueue(JUNO_BUFF_QUEUE_ROOT_T *ptQueue, void *ptItem)
{
    JUNO_ASSERT_EXISTS(ptQueue);
    JUNO_STATUS_T tStatus = JunoDs_Buff_QueueVerify(ptQueue);
    if(ptQueue->zLength < ptQueue->zCapacity)
    {
        size_t iIndex = (ptQueue->iStartIndex + ptQueue->zLength) % ptQueue->zCapacity;
        tStatus = ptQueue->ptApi->SetAt((JUNO_BUFF_QUEUE_T *) ptQueue, ptItem, iIndex);
        JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
        ptQueue->zLength += 1;
    }
    else
    {
        tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        JUNO_FAIL(tStatus, ptQueue->_pfcnFailureHandler, ptQueue->_pvFailureUserData, "Failed to enqueue data");
        return tStatus;
    }
    return tStatus;
}

/// Dequeue an item from the buffer
/// @returns The index to dequeue the item from
JUNO_STATUS_T JunoDs_Buff_QueueDequeue(JUNO_BUFF_QUEUE_ROOT_T *ptQueue, void *ptReturn)
{
    JUNO_STATUS_T tStatus = JunoDs_Buff_QueueVerify(ptQueue);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_ASSERT_EXISTS(ptReturn);
    if(ptQueue->zLength > 0)
    {
        const JUNO_BUFF_QUEUE_API_T *ptApi = ptQueue->ptApi;
        size_t iDequeueIndex = ptQueue->iStartIndex;
        JUNO_RESULT_VOID_PTR_T tPtrResult = ptApi->GetAt((JUNO_BUFF_QUEUE_T *) ptQueue, iDequeueIndex);
        JUNO_ASSERT_SUCCESS(tPtrResult.tStatus, return tPtrResult.tStatus);
        tStatus = ptApi->Copy(ptReturn, JUNO_OK(tPtrResult));
        JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
        tStatus = ptApi->RemoveAt((JUNO_BUFF_QUEUE_T *) ptQueue, iDequeueIndex);
        JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
        ptQueue->iStartIndex = (ptQueue->iStartIndex + 1) % ptQueue->zCapacity;
        ptQueue->zLength -= 1;
        return tStatus;
    }
    tStatus = JUNO_STATUS_ERR;
    JUNO_FAIL(tStatus, ptQueue->_pfcnFailureHandler, ptQueue->_pvFailureUserData, "Queue is empty");
    return tStatus;
}

/// Peek at the next item in the queue
/// @returns the index of the next item in the queue
JUNO_RESULT_VOID_PTR_T JunoBuff_QueuePeek(JUNO_BUFF_QUEUE_ROOT_T *ptQueue)
{
    JUNO_RESULT_VOID_PTR_T tResult = JUNO_ERR_RESULT(JUNO_STATUS_ERR, NULL);
    tResult.tStatus = JunoDs_Buff_QueueVerify(ptQueue);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    if(ptQueue->zLength == 0)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        JUNO_FAIL(tResult.tStatus, ptQueue->_pfcnFailureHandler, ptQueue->_pvFailureUserData, "Queue is empty");
        return tResult;
    }
    tResult = ptQueue->ptApi->GetAt((JUNO_BUFF_QUEUE_T *) ptQueue, ptQueue->iStartIndex);
    return tResult;
}
