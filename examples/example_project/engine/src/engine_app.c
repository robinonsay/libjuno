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
#include "engine_app/engine_cmd_msg.h"
#include "engine_app/engine_cmd_msg_pipe.h"
#include "engine_app/engine_tlm_msg.h"
#include "juno/ds/queue_api.h"
#include "juno/log/log_api.h"
#include "engine_app/engine_app.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include "juno/sb/broker_api.h"
#include "juno/status.h"
#include "juno/time/time_api.h"
#include <stdlib.h>
#include <unistd.h>


static inline JUNO_STATUS_T Verify(JUNO_APP_ROOT_T *ptJunoApp);


static JUNO_STATUS_T OnInit(JUNO_APP_ROOT_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptJunoApp);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus)
    // Cast to the engine app
    ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp);
    // Get the logger
    const JUNO_LOG_ROOT_T *ptLogger = ptEngineApp->ptLogger;
    // Get the logger api
    const JUNO_LOG_API_T *ptLoggerApi = ptLogger->ptApi;
    // Log that the app was intialized
    ptLoggerApi->LogInfo(ptLogger, "Engine App Initialized");
    // Initialize the cmd pipe
    tStatus = EngineCmdMsgPipeInit(&ptEngineApp->tCmdPipe, ptJunoApp->_pfcnFailureHandler, ptJunoApp->_pvFailureUserData);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Register the subscriber
    tStatus = ptEngineApp->ptBroker->ptApi->RegisterSubscriber(ptEngineApp->ptBroker, &ptEngineApp->tCmdPipe.tRoot);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}

static JUNO_STATUS_T OnProcess(JUNO_APP_ROOT_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptJunoApp);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus)
    // Cast to the engine app
    ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp);
    // Get the logger
    const JUNO_LOG_ROOT_T *ptLogger = ptEngineApp->ptLogger;
    // Get the logger api
    const JUNO_LOG_API_T *ptLoggerApi = ptLogger->ptApi;
    // Get the broker
    JUNO_SB_BROKER_ROOT_T *ptBroker = ptEngineApp->ptBroker;
    // Get the time api
    const JUNO_TIME_ROOT_T *ptTime = ptEngineApp->ptTime;
    // Log that the app is running
    ptLoggerApi->LogDebug(ptLogger, "Engine App Running");
    // sleep for half a second
    usleep(500E3);
    const JUNO_DS_QUEUE_API_T *ptCmdPipeApi = ptEngineApp->tCmdPipe.ptApi;
    ENGINE_CMD_MSG_T tEngineCmd = {0};
    JUNO_POINTER_T tEngineCmdPointer = EngineCmdMsg_PointerInit(&tEngineCmd);
    JUNO_TIMESTAMP_RESULT_T tTimestampResult = ptTime->ptApi->Now(ptTime);
    JUNO_ASSERT_SUCCESS(tTimestampResult.tStatus, return tTimestampResult.tStatus);
    ENGINE_TLM_MSG_T tEngineTlm = {ptEngineApp->fCurrentRpm, tTimestampResult.tOk};
    JUNO_POINTER_T tEngineTlmPointer = EngineTlmMsg_PointerInit(&tEngineTlm);
    float noise = rand();
    noise = noise - RAND_MAX/2;
    noise = 2 * noise / RAND_MAX;
    tStatus = ptCmdPipeApi->Dequeue(&ptEngineApp->tCmdPipe.tRoot.tRoot, tEngineCmdPointer);
    JUNO_ASSERT_SUCCESS(tStatus, goto exit);
    ptEngineApp->fCurrentRpm = tEngineCmd.fRpm;
    ptLoggerApi->LogInfo(ptLogger, "RPM Commanded to: %f", tEngineCmd.fRpm);
exit:
    tEngineTlm.fRpm = ptEngineApp->fCurrentRpm + 10*noise;
    ptBroker->ptApi->Publish(ptBroker, ENGINE_TLM_MSG_MID, tEngineTlmPointer);
    return tStatus;
}

static JUNO_STATUS_T OnExit(JUNO_APP_ROOT_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptJunoApp);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus)
    // Cast to the engine app
    ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp);
    // Get the logger
    const JUNO_LOG_ROOT_T *ptLogger = ptEngineApp->ptLogger;
    // Get the logger api
    const JUNO_LOG_API_T *ptLoggerApi = ptLogger->ptApi;
    // Log that the app is running
    ptLoggerApi->LogInfo(ptLogger, "Engine App Exiting");
    return tStatus;
}

static const JUNO_APP_API_T tEngineAppApi = {
    .OnInit = OnInit,
    .OnProcess = OnProcess,
    .OnExit = OnExit
};

static inline JUNO_STATUS_T Verify(JUNO_APP_ROOT_T *ptJunoApp)
{
    // Assert the pointer is not null
    JUNO_ASSERT_EXISTS(ptJunoApp);
    // Cast to the engine app
    ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp);
    // Assert the module dependencies are present
    JUNO_ASSERT_EXISTS_MODULE(
        /* TODO: Assert other dependencies and members here using &&*/
        ptEngineApp &&
        ptEngineApp->tRoot.ptApi &&
        ptEngineApp->ptLogger &&
        ptEngineApp->ptTime &&
        ptEngineApp->ptBroker,
        ptEngineApp,
        "Module does not have all dependencies"
    );
    // Verify that this application is using the correct API
    if(ptEngineApp->tRoot.ptApi != &tEngineAppApi)
    {
        JUNO_FAIL_MODULE(JUNO_STATUS_INVALID_TYPE_ERROR, ptEngineApp, "Module has invalid API");
        return JUNO_STATUS_INVALID_TYPE_ERROR;
    }
    return JUNO_STATUS_SUCCESS;
}

/* TODO: Insert initialization arguments for module members here*/
JUNO_STATUS_T EngineApp_Init(
    ENGINE_APP_T *ptJunoApp,
    const JUNO_LOG_ROOT_T *ptLogger,
    const JUNO_TIME_ROOT_T *ptTime,
    JUNO_SB_BROKER_ROOT_T *ptBroker,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
)
{
    JUNO_ASSERT_EXISTS(ptJunoApp);
    ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp);
    ptEngineApp->tRoot.ptApi = &tEngineAppApi;
    ptEngineApp->tRoot.JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    ptEngineApp->tRoot.JUNO_FAILURE_USER_DATA = pvFailureUserData;
    ptEngineApp->ptLogger = ptLogger;
    ptEngineApp->ptTime = ptTime;
    ptEngineApp->ptBroker = ptBroker;
    JUNO_STATUS_T tStatus = Verify(&ptJunoApp->tRoot);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}

