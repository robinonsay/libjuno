#include "juno/ds/array_api.h"
#include "juno/ds/buff_queue_api.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include "juno/module.h"
#include "juno/sb/broker_api.h"
#include "juno/status.h"

/// Publish the SB message with message id
static JUNO_STATUS_T Publish(JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_VALUE_POINTER_T tMid, JUNO_POINTER_T tMsg);
/// Register a subscriber to a sb message
static JUNO_STATUS_T RegisterSubscriber(JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_SB_PIPE_T *ptPipe);
/// Set a buff queue at an index    
static JUNO_STATUS_T RegistrySetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
/// Get a buff queue at an index
static JUNO_RESULT_POINTER_T RegistryGetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
/// Remove a buff queue at an index
static JUNO_STATUS_T RegistryRemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
/// Copy memory from one pointer to another
static JUNO_STATUS_T PipeCopy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
/// Reset the memory at the pointer. This could mean zero-initialization
static JUNO_STATUS_T PipeReset(JUNO_POINTER_T tPointer);

static const JUNO_SB_BROKER_API_T gtBrokerApi =
{
    Publish,
    RegisterSubscriber
};

static const JUNO_DS_ARRAY_API_T gtRegistryApi =
{
    RegistrySetAt,
    RegistryGetAt,
    RegistryRemoveAt
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

JUNO_STATUS_T JunoSb_BrokerInit(JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_SB_PIPE_REGISTRY_T *ptRegistry)
{
    JUNO_ASSERT_EXISTS(ptBroker && ptRegistry);
    ptBroker->ptApi = &gtBrokerApi;
    ptBroker->ptRegistry = ptRegistry;
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
        // Check if the mids are equal
        JUNO_RESULT_BOOL_T tBoolResult = ptValuePointerApi->Equals(tMid, ptCurrentItem->tMid);
        JUNO_ASSERT_OK(tBoolResult, return tBoolResult.tStatus);
        if(tBoolResult.tOk)
        {
            // MIDs are equal, enqueue the message
            JUNO_DS_QUEUE_ROOT_T *ptRecvQueue = ptCurrentItem->ptRecvQueue;
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
    const JUNO_DS_ARRAY_API_T *ptArrayApi = ptBroker->ptRegistry->tRoot.ptApi;
    JUNO_DS_ARRAY_ROOT_T *ptRegistryRoot = &ptBroker->ptRegistry->tRoot;
    // Check if the broker registry is full
    if(ptRegistryRoot->zLength >= ptRegistryRoot->zCapacity)
    {
        // Registry is full, return an error
        tStatus = JUNO_STATUS_ERR;
        JUNO_FAIL_ROOT(tStatus, ptBroker, "Failed to register subscriber. Registry Full");
        return tStatus;
    }
    // Initialize the pointer
    JUNO_POINTER_T tPipePointer = JunoMemory_PointerInit(&gtPipePointerApi, JUNO_SB_PIPE_T, ptPipe);
    // Set the pipe at the end of the registry
    tStatus = ptArrayApi->SetAt(ptRegistryRoot, tPipePointer, ptRegistryRoot->zLength);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Increase the length of the registry
    ptRegistryRoot->zLength += 1;
    return tStatus;
}

static JUNO_STATUS_T RegistrySetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Check if the array is using the registry api
    if(ptArray->ptApi != &gtRegistryApi)
    {
        tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        JUNO_FAIL_ROOT(tStatus, ptArray, "Invalid type. Not Pipe Registry");
        return tStatus;
    }
    // cast the registry
    JUNO_SB_PIPE_REGISTRY_T *ptRegistry = (JUNO_SB_PIPE_REGISTRY_T *) ptArray;
    // verify the registry
    tStatus = JunoSb_RegistryVerify(ptRegistry);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // verify the pointer
    tStatus = JunoMemory_PointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Check if the index is oob
    if(iIndex >= ptArray->zCapacity)
    {
        tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        JUNO_FAIL_ROOT(tStatus, ptArray, "Index is oob");
        return tStatus;
    }
    // initialize the pointer to the index
    JUNO_POINTER_T tItemIndex = JunoMemory_PointerInit(&gtPipePointerApi, JUNO_SB_PIPE_T, &ptRegistry->ptArrItems[iIndex]);
    // copy the value at the pointer
    tStatus = gtPipePointerApi.Copy(tItemIndex, tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}

static JUNO_RESULT_POINTER_T RegistryGetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_RESULT_POINTER_T tResult = {JUNO_STATUS_ERR, {0}};
    tResult.tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    // Check if the array api is a registry
    if(ptArray->ptApi != &gtRegistryApi)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        JUNO_FAIL_ROOT(tResult.tStatus, ptArray, "Invalid type. Not Pipe Registry");
        return tResult;
    }
    // cast the registry
    JUNO_SB_PIPE_REGISTRY_T *ptRegistry = (JUNO_SB_PIPE_REGISTRY_T *) ptArray;
    // verify the registry
    tResult.tStatus = JunoSb_RegistryVerify(ptRegistry);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    // check if the index is oob
    if(iIndex >= ptArray->zCapacity)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        JUNO_FAIL_ROOT(tResult.tStatus, ptArray, "Index is oob");
        return tResult;
    }
    // initialize the pointer to the item
    JUNO_POINTER_T tItemIndex = JunoMemory_PointerInit(&gtPipePointerApi, JUNO_SB_PIPE_T, &ptRegistry->ptArrItems[iIndex]);
    // return the pointer
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    tResult.tOk = tItemIndex;
    return tResult;
}

static JUNO_STATUS_T RegistryRemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // check if the array is a registry
    if(ptArray->ptApi != &gtRegistryApi)
    {
        tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        JUNO_FAIL_ROOT(tStatus, ptArray, "Invalid type. Not Pipe Registry");
        return tStatus;
    }
    // cast the registry
    JUNO_SB_PIPE_REGISTRY_T *ptRegistry = (JUNO_SB_PIPE_REGISTRY_T *) ptArray;
    // verify the registry
    tStatus = JunoSb_RegistryVerify(ptRegistry);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // check if the index is oob
    if(iIndex >= ptArray->zCapacity)
    {
        tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        JUNO_FAIL_ROOT(tStatus, ptArray, "Index is oob");
        return tStatus;
    }
    // get a pointer to the item
    JUNO_POINTER_T tItemIndex = JunoMemory_PointerInit(&gtPipePointerApi, JUNO_SB_PIPE_T, &ptRegistry->ptArrItems[iIndex]);
    // reset the item
    tStatus = gtPipePointerApi.Reset(tItemIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}

/// Copy memory from one pointer to another
static JUNO_STATUS_T PipeCopy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc)
{
    JUNO_ASSERT_POINTER_COPY(tDest, tSrc, gtPipePointerApi);
    // check the pointer type
    JUNO_STATUS_T tStatus = JunoMemory_PointerCheckType(tDest, JUNO_SB_PIPE_T, gtPipePointerApi);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerCheckType(tSrc, JUNO_SB_PIPE_T, gtPipePointerApi);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // copy the src memory to the destination
    *(JUNO_SB_PIPE_T *)tDest.pvAddr = *(JUNO_SB_PIPE_T *)tSrc.pvAddr;
    return tStatus;
}
/// Reset the memory at the pointer. This could mean zero-initialization
static JUNO_STATUS_T PipeReset(JUNO_POINTER_T tPointer)
{
    // check the pointer type
    JUNO_STATUS_T tStatus = JunoMemory_PointerCheckType(tPointer, JUNO_SB_PIPE_T, gtPipePointerApi);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // set the pointer value to zero
    *(JUNO_SB_PIPE_T *)tPointer.pvAddr = (JUNO_SB_PIPE_T){0};
    return JUNO_STATUS_SUCCESS;
}
