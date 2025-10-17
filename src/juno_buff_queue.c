#include "juno/ds/array_api.h"
#include "juno/ds/buff_queue_api.h"
#include "juno/macros.h"
#include "juno/memory/memory_api.h"

/// Enqueue an item on the queue
static JUNO_STATUS_T JunoEnqueue(JUNO_DS_QUEUE_ROOT_T *ptQueue, JUNO_POINTER_T tItem)
{
    JUNO_ASSERT_EXISTS(ptQueue);
    JUNO_STATUS_T tStatus = JunoDs_Buff_QueueVerify(ptQueue);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_DS_ARRAY_ROOT_T *ptBuffer = ptQueue->ptBuffer;
    if(ptBuffer->zLength < ptBuffer->zCapacity)
    {
        size_t iIndex = (ptQueue->iStartIndex + ptBuffer->zLength) % ptBuffer->zCapacity;
        tStatus = ptBuffer->ptApi->SetAt(ptBuffer, tItem, iIndex);
        JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
        ptBuffer->zLength += 1;
    }
    else
    {
        tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        JUNO_FAIL(tStatus, ptQueue->_pfcnFailureHandler, ptQueue->_pvFailureUserData, "Failed to enqueue data");
        return tStatus;
    }
    return tStatus;
}

/// Dequeue an item from the queue
static JUNO_STATUS_T JunoDequeue(JUNO_DS_QUEUE_ROOT_T *ptQueue, JUNO_POINTER_T tReturn)
{
    JUNO_STATUS_T tStatus = JunoDs_Buff_QueueVerify(ptQueue);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerify(tReturn);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_DS_ARRAY_ROOT_T *ptBuffer = ptQueue->ptBuffer;
    if(ptBuffer->zLength > 0)
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
        ptBuffer->zLength -= 1;
        return tStatus;
    }
    tStatus = JUNO_STATUS_ERR;
    JUNO_FAIL(tStatus, ptQueue->_pfcnFailureHandler, ptQueue->_pvFailureUserData, "Queue is empty");
    return tStatus;
}

/// Peek at the next item in the queue
static JUNO_RESULT_POINTER_T JunoPeek(JUNO_DS_QUEUE_ROOT_T *ptQueue)
{
    JUNO_RESULT_POINTER_T tResult = JUNO_ERR_RESULT(JUNO_STATUS_ERR, {0});
    tResult.tStatus = JunoDs_Buff_QueueVerify(ptQueue);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    JUNO_DS_ARRAY_ROOT_T *ptBuffer = ptQueue->ptBuffer;
    if(ptBuffer->zLength == 0)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        JUNO_FAIL(tResult.tStatus, ptQueue->_pfcnFailureHandler, ptQueue->_pvFailureUserData, "Queue is empty");
        return tResult;
    }
    tResult = ptBuffer->ptApi->GetAt(ptBuffer, ptQueue->iStartIndex);
    return tResult;
}

static const JUNO_DS_QUEUE_API_T gtBuffQueueApi =
{
    JunoEnqueue,
    JunoDequeue,
    JunoPeek
};

/// Initialize a buffer queue with a capacity
JUNO_STATUS_T JunoDs_Buff_QueueInit(JUNO_DS_QUEUE_ROOT_T *ptQueue, JUNO_DS_ARRAY_ROOT_T *ptBuffer, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvFailureUserData)
{
    JUNO_ASSERT_EXISTS(ptQueue);
    ptQueue->ptApi = &gtBuffQueueApi;
    ptQueue->iStartIndex = 0;
    ptQueue->ptBuffer = ptBuffer;
    ptQueue->_pfcnFailureHandler = pfcnFailureHdlr;
    ptQueue->_pvFailureUserData = pvFailureUserData;
    return JunoDs_Buff_QueueVerify(ptQueue);
}
