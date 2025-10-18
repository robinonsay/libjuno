#include "juno/ds/queue_api.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include "juno/sb/broker_api.h"
#include "juno/status.h"

/// Publish the SB message with message id
static JUNO_STATUS_T Publish(JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_SB_MSG_ID_T tMid, JUNO_POINTER_T tMsg);
/// Register a subscriber to a sb message
static JUNO_STATUS_T RegisterSubscriber(JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_SB_PIPE_T *ptPipe);

static const JUNO_SB_BROKER_API_T gtBrokerApi =
{
    Publish,
    RegisterSubscriber
};

JUNO_STATUS_T JunoSb_BrokerInit(JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_SB_PIPE_T **ptPipeRegistry, size_t iRegistryCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvFailureUserData)
{
    JUNO_ASSERT_EXISTS(ptBroker);
    ptBroker->ptApi = &gtBrokerApi;
    ptBroker->ptPipeRegistry = ptPipeRegistry;
    ptBroker->zRegistryCapacity = iRegistryCapacity;
    ptBroker->zRegistryLength = 0;
    ptBroker->_pfcnFailureHandler = pfcnFailureHdlr;
    ptBroker->_pvFailureUserData = pvFailureUserData;
    JUNO_STATUS_T tStatus = JunoSb_BrokerVerify(ptBroker);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}

static JUNO_STATUS_T Publish(JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_SB_MSG_ID_T tMid, JUNO_POINTER_T tMsg)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = JunoSb_BrokerVerify(ptBroker);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Verify the value pointer api
    for(size_t i = 0; i < ptBroker->zRegistryLength; i++)
    {
        // Get the current item
        JUNO_SB_PIPE_T *ptCurrentItem = ptBroker->ptPipeRegistry[i];
        tStatus = JunoDs_Buff_QueueVerify(&ptCurrentItem->tRoot);
        JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
        // Check if the mids are equal
        if(ptCurrentItem->iMsgId == tMid)
        {
            // MIDs are equal, enqueue the message
            JUNO_DS_QUEUE_ROOT_T *ptRecvQueue = &ptCurrentItem->tRoot;
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
    // Check if the broker registry is full
    if(ptBroker->zRegistryLength >= ptBroker->zRegistryCapacity)
    {
        // Registry is full, return an error
        tStatus = JUNO_STATUS_ERR;
        JUNO_FAIL_ROOT(tStatus, ptBroker, "Failed to register subscriber. Registry Full");
        return tStatus;
    }
    ptBroker->ptPipeRegistry[ptBroker->zRegistryLength] = ptPipe;
    // Increase the length of the registry
    ptBroker->zRegistryLength += 1;
    return tStatus;
}
