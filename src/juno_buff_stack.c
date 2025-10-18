#include "juno/ds/array_api.h"
#include "juno/ds/stack_api.h"
#include "juno/macros.h"
#include "juno/memory/memory_api.h"
#include "juno/status.h"


/// Enqueue an item into the buffer
/// @returns The index to place the enqueued item
static JUNO_STATUS_T JunoDs_Buff_StackPush(JUNO_DS_STACK_ROOT_T *ptStack, JUNO_POINTER_T tItem)
{
    JUNO_STATUS_T tStatus = JunoDs_Buff_StackVerify(ptStack);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_DS_STACK_ROOT_T *ptStackRoot = (JUNO_DS_STACK_ROOT_T *)(ptStack);
    JUNO_DS_ARRAY_ROOT_T *ptBuffer = ptStack->ptBuffer;
    if(ptStack->zLength < ptBuffer->zCapacity)
    {
        tStatus = ptBuffer->ptApi->SetAt(ptBuffer, tItem, ptStack->zLength);
        JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
        ptStack->zLength += 1;
    }
    else
    {
        tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        JUNO_FAIL(tStatus, ptStackRoot->_pfcnFailureHandler, ptStackRoot->_pvFailureUserData, "Failed to enqueue data");
    }
    return tStatus;
}


/// Dequeue an item from the buffer
/// @returns The index to dequeue the item from
static JUNO_STATUS_T JunoDs_Buff_StackPop(JUNO_DS_STACK_ROOT_T *ptStack, JUNO_POINTER_T tReturn)
{
    JUNO_STATUS_T tStatus = JunoDs_Buff_StackVerify(ptStack);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerify(tReturn);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_DS_STACK_ROOT_T *ptStackRoot = (JUNO_DS_STACK_ROOT_T *)(ptStack);
    JUNO_DS_ARRAY_ROOT_T *ptBuffer = ptStack->ptBuffer;
    if(ptStack->zLength > 0)
    {
        ptStack->zLength -= 1;
        JUNO_RESULT_POINTER_T tResult = ptBuffer->ptApi->GetAt(ptBuffer, ptStack->zLength);
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult.tStatus);
        tStatus = tReturn.ptApi->Copy(tReturn, tResult.tOk);
        return tStatus;
    }
    tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
    JUNO_FAIL(tStatus, ptStackRoot->_pfcnFailureHandler, ptStackRoot->_pvFailureUserData, "Failed to enqueue data");
    return tStatus;
}


/// Peek at the next item in the queue
/// @returns the index of the next item in the queue
static JUNO_RESULT_POINTER_T JunoDs_Buff_StackPeek(JUNO_DS_STACK_ROOT_T *ptStack)
{
    JUNO_RESULT_POINTER_T tResult = JUNO_ERR_RESULT(JUNO_STATUS_ERR, {0});
    tResult.tStatus = JunoDs_Buff_StackVerify(ptStack);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    JUNO_DS_ARRAY_ROOT_T *ptBuffer = ptStack->ptBuffer;
    if(ptStack->zLength == 0)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        JUNO_FAIL(tResult.tStatus, ptStack->_pfcnFailureHandler, ptStack->_pvFailureUserData, "Queue is empty");
        return tResult;
    }
    tResult = ptBuffer->ptApi->GetAt(ptBuffer, 0);
    return tResult;
}

static const JUNO_DS_STACK_API_T tStackApi = {
    JunoDs_Buff_StackPush,
    JunoDs_Buff_StackPop,
    JunoDs_Buff_StackPeek,
};

/// Initialize a buffer queue with a capacity
JUNO_STATUS_T JunoDs_Buff_StackInit(JUNO_DS_STACK_ROOT_T *ptStack, JUNO_DS_ARRAY_ROOT_T *ptBuffer, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvFailureUserData)
{
    JUNO_ASSERT_EXISTS(ptStack);
    JUNO_DS_STACK_ROOT_T *ptStackRoot = (JUNO_DS_STACK_ROOT_T *)(ptStack);
    ptStackRoot->ptBuffer = ptBuffer;
    ptStackRoot->_pfcnFailureHandler = pfcnFailureHdlr;
    ptStackRoot->_pvFailureUserData = pvFailureUserData;
    ptStackRoot->ptApi = &tStackApi;
    return JunoDs_Buff_StackVerify(ptStack);
}
