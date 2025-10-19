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
    JUNO_DS_ARRAY_ROOT_T *ptBuffer = &ptQueue->tRoot;
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
        JUNO_FAIL_MODULE(tStatus, ptQueue, "Failed to enqueue data");
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
    JUNO_DS_ARRAY_ROOT_T *ptBuffer = &ptQueue->tRoot;
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
    JUNO_FAIL_MODULE(tStatus, ptQueue, "Queue is empty");
    return tStatus;
}

/// Peek at the next item in the queue
JUNO_RESULT_POINTER_T JunoDs_QueuePeek(JUNO_DS_QUEUE_ROOT_T *ptQueue)
{
    JUNO_RESULT_POINTER_T tResult = JUNO_ERR_RESULT(JUNO_STATUS_ERR, {0});
    tResult.tStatus = JunoDs_QueueVerify(ptQueue);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    JUNO_DS_ARRAY_ROOT_T *ptBuffer = &ptQueue->tRoot;
    if(ptQueue->zLength == 0)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        JUNO_FAIL_MODULE(tResult.tStatus, ptQueue, "Queue is empty");
        return tResult;
    }
    tResult = ptBuffer->ptApi->GetAt(ptBuffer, ptQueue->iStartIndex);
    return tResult;
}

/// Initialize a buffer queue with a capacity
JUNO_STATUS_T JunoDs_QueueInit(JUNO_DS_QUEUE_ROOT_T *ptQueue, const JUNO_DS_QUEUE_API_T *ptQueueApi, size_t iCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvFailureUserData)
{
    JUNO_ASSERT_EXISTS(ptQueue && ptQueueApi && iCapacity);
    ptQueue->ptApi = ptQueueApi;
    ptQueue->iStartIndex = 0;
    ptQueue->zLength = 0;
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(&ptQueue->tRoot, &ptQueueApi->tRoot, iCapacity, pfcnFailureHdlr, pvFailureUserData);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return JunoDs_QueueVerify(ptQueue);
}
