#include "juno/ds/array_api.h"
#include "juno/ds/buff_queue_api.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include "juno/module.h"
#include "juno/sb/sb_api.h"
#include "juno/status.h"

static JUNO_STATUS_T Publish(JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_VALUE_POINTER_T tMid, JUNO_POINTER_T tMsg);
static JUNO_STATUS_T RegisterSubscriber(JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_SB_PIPE_T *ptPipe);
static JUNO_STATUS_T SetAt(JUNO_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
static JUNO_RESULT_POINTER_T GetAt(JUNO_ARRAY_ROOT_T *ptArray, size_t iIndex);
static JUNO_STATUS_T RemoveAt(JUNO_ARRAY_ROOT_T *ptArray, size_t iIndex);
/// Copy memory from one pointer to another
static JUNO_STATUS_T PipeCopy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
/// Reset the memory at the pointer. This could mean zero-initialization
static JUNO_STATUS_T PipeReset(JUNO_POINTER_T tPointer);

static const JUNO_SB_BROKER_API_T gtBrokerApi =
{
    Publish,
    RegisterSubscriber
};

static const JUNO_ARRAY_API_T gtRegistryApi =
{
    SetAt,
    GetAt,
    RemoveAt
};

static const JUNO_POINTER_API_T gtPipePointerApi =
{
    PipeCopy,
    PipeReset
};

JUNO_STATUS_T JunoSb_RegistryInit(JUNO_SB_PIPE_REGISTRY_T *ptRegistry, JUNO_SB_PIPE_T *ptArrRecvQueues, const size_t iCapacity)
{
    JUNO_ASSERT_EXISTS(ptRegistry && iCapacity && ptArrRecvQueues);
    ptRegistry->tRoot.ptApi = &gtRegistryApi;
    ptRegistry->ptArrItems = ptArrRecvQueues;
    ptRegistry->tRoot.zCapacity = iCapacity;
    JUNO_STATUS_T tStatus = JunoSb_RegistryVerify(ptRegistry);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}

JUNO_STATUS_T JunoSb_BrokerInit(JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_SB_PIPE_REGISTRY_T *ptRegistry, JUNO_MP_MTX_ROOT_T *ptMtx)
{
    JUNO_ASSERT_EXISTS(ptBroker && ptRegistry && ptMtx);
    ptBroker->ptApi = &gtBrokerApi;
    ptBroker->ptRegistry = ptRegistry;
    ptBroker->ptMtx = ptMtx;
    JUNO_STATUS_T tStatus = JunoSb_BrokerVerify(ptBroker);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}

static JUNO_STATUS_T Publish(JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_VALUE_POINTER_T tMid, JUNO_POINTER_T tMsg)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = JunoSb_BrokerVerify(ptBroker);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    const JUNO_VALUE_POINTER_API_T *ptValuePointerApi = tMid.ptApi;
    // Verify the value pointer api
    tStatus = JunoMemory_ValuePointerVerify(tMid);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Set flag indicating a publish occured
    for(size_t i = 0; i < ptBroker->ptRegistry->tRoot.zLength; i++)
    {
        // Get the current item
        JUNO_SB_PIPE_T *ptCurrentItem = &ptBroker->ptRegistry->ptArrItems[i];
        JUNO_RESULT_BOOL_T tBoolResult = ptValuePointerApi->Equals(tMid, ptCurrentItem->tMid);
        JUNO_ASSERT_OK(tBoolResult, return tBoolResult.tStatus);
        if(tBoolResult.tOk)
        {
            JUNO_BUFF_QUEUE_ROOT_T *ptRecvQueue = ptCurrentItem->ptRecvQueue;
            tStatus = JunoDs_Buff_QueueVerify(ptRecvQueue);
            JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
            tStatus = ptRecvQueue->ptApi->Enqueue(ptRecvQueue, tMsg);
            JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
        }
    }
    return tStatus;
}

static JUNO_STATUS_T RegisterSubscriber(JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_SB_PIPE_T *ptPipe)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = JunoSb_BrokerVerify(ptBroker);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoSb_PipeVerify(ptPipe);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    const JUNO_ARRAY_API_T *ptArrayApi = ptBroker->ptRegistry->tRoot.ptApi;
    JUNO_ARRAY_ROOT_T *ptRegistryRoot = &ptBroker->ptRegistry->tRoot;
    if(ptBroker->ptRegistry->tRoot.zLength >= ptBroker->ptRegistry->tRoot.zCapacity)
    {
        tStatus = JUNO_STATUS_ERR;
        JUNO_FAIL_ROOT(tStatus, ptBroker, "Failed to register subscriber. Registry Full");
        return tStatus;
    }
    JUNO_POINTER_T tPipePointer = JunoMemory_PointerInit(&gtPipePointerApi, JUNO_SB_PIPE_T, ptPipe);
    tStatus = ptArrayApi->SetAt(ptRegistryRoot, tPipePointer, ptRegistryRoot->zLength);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}

static JUNO_STATUS_T SetAt(JUNO_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    if(ptArray->ptApi != &gtRegistryApi)
    {
        tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        JUNO_FAIL_ROOT(tStatus, ptArray, "Invalid type. Not Pipe Registry");
        return tStatus;
    }
    JUNO_SB_PIPE_REGISTRY_T *ptRegistry = (JUNO_SB_PIPE_REGISTRY_T *) ptArray;
    tStatus = JunoSb_RegistryVerify(ptRegistry);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    if(iIndex >= ptArray->zCapacity)
    {
        tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        JUNO_FAIL_ROOT(tStatus, ptArray, "Index is oob");
        return tStatus;
    }
    JUNO_POINTER_T tItemIndex = JunoMemory_PointerInit(&gtPipePointerApi, JUNO_SB_PIPE_T, &ptRegistry->ptArrItems[iIndex]);
    tStatus = gtPipePointerApi.Copy(tItemIndex, tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}

static JUNO_RESULT_POINTER_T GetAt(JUNO_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_RESULT_POINTER_T tResult = {JUNO_STATUS_ERR, {0}};
    if(ptArray->ptApi != &gtRegistryApi)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        JUNO_FAIL_ROOT(tResult.tStatus, ptArray, "Invalid type. Not Pipe Registry");
        return tResult;
    }
    JUNO_SB_PIPE_REGISTRY_T *ptRegistry = (JUNO_SB_PIPE_REGISTRY_T *) ptArray;
    tResult.tStatus = JunoSb_RegistryVerify(ptRegistry);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    if(iIndex >= ptArray->zCapacity)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        JUNO_FAIL_ROOT(tResult.tStatus, ptArray, "Index is oob");
        return tResult;
    }
    JUNO_POINTER_T tItemIndex = JunoMemory_PointerInit(&gtPipePointerApi, JUNO_SB_PIPE_T, &ptRegistry->ptArrItems[iIndex]);
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    tResult.tOk = tItemIndex;
    return tResult;
}

static JUNO_STATUS_T RemoveAt(JUNO_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    if(ptArray->ptApi != &gtRegistryApi)
    {
        tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        JUNO_FAIL_ROOT(tStatus, ptArray, "Invalid type. Not Pipe Registry");
        return tStatus;
    }
    JUNO_SB_PIPE_REGISTRY_T *ptRegistry = (JUNO_SB_PIPE_REGISTRY_T *) ptArray;
    tStatus = JunoSb_RegistryVerify(ptRegistry);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    if(iIndex >= ptArray->zCapacity)
    {
        tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        JUNO_FAIL_ROOT(tStatus, ptArray, "Index is oob");
        return tStatus;
    }
    JUNO_POINTER_T tItemIndex = JunoMemory_PointerInit(&gtPipePointerApi, JUNO_SB_PIPE_T, &ptRegistry->ptArrItems[iIndex]);
    tStatus = gtPipePointerApi.Reset(tItemIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}

/// Copy memory from one pointer to another
static JUNO_STATUS_T PipeCopy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc)
{
    JUNO_ASSERT_POINTER_COPY(tDest, tSrc, gtPipePointerApi);
    JUNO_STATUS_T tStatus = JUNO_CHECK_POINTER_TYPE(tDest, JUNO_SB_PIPE_T, gtPipePointerApi);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JUNO_CHECK_POINTER_TYPE(tSrc, JUNO_SB_PIPE_T, gtPipePointerApi);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    *(JUNO_SB_PIPE_T *)tDest.pvAddr = *(JUNO_SB_PIPE_T *)tSrc.pvAddr;
    return tStatus;
}
/// Reset the memory at the pointer. This could mean zero-initialization
static JUNO_STATUS_T PipeReset(JUNO_POINTER_T tPointer)
{
    JUNO_STATUS_T tStatus = JUNO_CHECK_POINTER_TYPE(tPointer, JUNO_SB_PIPE_T, gtPipePointerApi);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    *(JUNO_SB_PIPE_T *)tPointer.pvAddr = (JUNO_SB_PIPE_T){0};
    return JUNO_STATUS_SUCCESS;
}
