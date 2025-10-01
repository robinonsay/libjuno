#include "juno/ds/buff_stack_api.h"
#include "juno/macros.h"
#include "juno/status.h"
/// Initialize a buffer queue with a capacity
JUNO_STATUS_T JunoDs_Buff_StackInit(JUNO_BUFF_STACK_ROOT_T *ptStack, const JUNO_BUFF_STACK_API_T *ptApi, size_t zCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvFailureUserData)
{
    JUNO_ASSERT_EXISTS(ptStack);
    JUNO_BUFF_STACK_ROOT_T *ptStackRoot = (JUNO_BUFF_STACK_ROOT_T *)(ptStack);
    ptStackRoot->zLength = 0;
    ptStackRoot->zCapacity = zCapacity;
    ptStackRoot->_pfcnFailureHandler = pfcnFailureHdlr;
    ptStackRoot->_pvFailureUserData = pvFailureUserData;
    ptStackRoot->ptApi = ptApi;
    return JunoDs_Buff_StackVerify(ptStack);
}

/// Enqueue an item into the buffer
/// @returns The index to place the enqueued item
JUNO_STATUS_T JunoDs_Buff_StackPush(JUNO_BUFF_STACK_ROOT_T *ptStack, void *ptItem)
{
    JUNO_STATUS_T tStatus = JunoDs_Buff_StackVerify(ptStack);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_BUFF_STACK_ROOT_T *ptStackRoot = (JUNO_BUFF_STACK_ROOT_T *)(ptStack);
    if(ptStackRoot->zLength < ptStackRoot->zCapacity)
    {
        tStatus = ptStack->ptApi->SetAt(ptStack, ptItem, ptStackRoot->zLength);
        JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
        ptStackRoot->zLength += 1;
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
JUNO_STATUS_T JunoDs_Buff_StackPop(JUNO_BUFF_STACK_ROOT_T *ptStack, void *ptReturn)
{
    JUNO_STATUS_T tStatus = JunoDs_Buff_StackVerify(ptStack);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_BUFF_STACK_ROOT_T *ptStackRoot = (JUNO_BUFF_STACK_ROOT_T *)(ptStack);
    if(ptStackRoot->zLength > 0)
    {
        ptStackRoot->zLength -= 1;
        JUNO_RESULT_VOID_PTR_T tResult = ptStack->ptApi->GetAt(ptStack, ptStackRoot->zLength);
        JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult.tStatus);
        tStatus = ptStackRoot->ptApi->Copy(ptReturn, tResult.tOk);
        return tStatus;
    }
    tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
    JUNO_FAIL(tStatus, ptStackRoot->_pfcnFailureHandler, ptStackRoot->_pvFailureUserData, "Failed to enqueue data");
    return tStatus;
}


/// Peek at the next item in the queue
/// @returns the index of the next item in the queue
JUNO_RESULT_VOID_PTR_T JunoDs_Buff_StackPeek(JUNO_BUFF_STACK_ROOT_T *ptStack)
{
    JUNO_RESULT_VOID_PTR_T tResult = JUNO_ERR_RESULT(JUNO_STATUS_ERR, NULL);
    tResult.tStatus = JunoDs_Buff_StackVerify(ptStack);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    if(ptStack->zLength == 0)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        JUNO_FAIL(tResult.tStatus, ptStack->_pfcnFailureHandler, ptStack->_pvFailureUserData, "Queue is empty");
        return tResult;
    }
    tResult = ptStack->ptApi->GetAt(ptStack, 0);
    return tResult;
}
