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
#include "engine_app/engine_tlm_msg.h"
#include "system_manager_app/engine_tlm_msg_pipe.h"
#include "juno/ds/queue_api.h"
#include "juno/log/log_api.h"
#include "system_manager_app/system_manager_app.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include "juno/sb/broker_api.h"
#include "juno/status.h"
#include "juno/time/time_api.h"
#include <math.h>
#include <stdlib.h>
#include <unistd.h>


static inline JUNO_STATUS_T Verify(JUNO_APP_ROOT_T *ptJunoApp);


static JUNO_STATUS_T OnInit(JUNO_APP_ROOT_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptJunoApp);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus)
    // Cast to the systemmanager app
    SYSTEM_MANAGER_APP_T *ptSystemManagerApp = (SYSTEM_MANAGER_APP_T *)(ptJunoApp);
    // Get the logger
    const JUNO_LOG_ROOT_T *ptLogger = ptSystemManagerApp->ptLogger;
    // Get the logger api
    const JUNO_LOG_API_T *ptLoggerApi = ptLogger->ptApi;
    // Log that the app was intialized
    ptLoggerApi->LogInfo(ptLogger, "SystemManager App Initialized");
    // Initialize the cmd pipe
    tStatus = EngineTlmMsgPipeInit(&ptSystemManagerApp->tEngineTlmPipe, ptJunoApp->_pfcnFailureHandler, ptJunoApp->_pvFailureUserData);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Register the subscriber
    tStatus = ptSystemManagerApp->ptBroker->ptApi->RegisterSubscriber(ptSystemManagerApp->ptBroker, &ptSystemManagerApp->tEngineTlmPipe.tRoot);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_TIMESTAMP_RESULT_T tTimestampResult = ptSystemManagerApp->ptTime->ptApi->Now(ptSystemManagerApp->ptTime);
    tStatus = tTimestampResult.tStatus;
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    ptSystemManagerApp->tEngineStart = tTimestampResult.tOk;
    return tStatus;
}

static JUNO_STATUS_T OnProcess(JUNO_APP_ROOT_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptJunoApp);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus)
    // Cast to the systemmanager app
    SYSTEM_MANAGER_APP_T *ptSystemManagerApp = (SYSTEM_MANAGER_APP_T *)(ptJunoApp);
    // Get the logger
    const JUNO_LOG_ROOT_T *ptLogger = ptSystemManagerApp->ptLogger;
    // Get the logger api
    const JUNO_LOG_API_T *ptLoggerApi = ptLogger->ptApi;
    // Get the broker
    JUNO_SB_BROKER_ROOT_T *ptBroker = ptSystemManagerApp->ptBroker;
    // Get the time api
    const JUNO_TIME_ROOT_T *ptTime = ptSystemManagerApp->ptTime;
    // Log that the app is running
    ptLoggerApi->LogDebug(ptLogger, "SystemManager App Running");
    // sleep for half a second
    ENGINE_TLM_MSG_T tTlmMsg = {0};
    JUNO_POINTER_T tTlmMsgPointer = EngineTlmMsg_PointerInit(&tTlmMsg);
    ENGINE_CMD_MSG_T tEngineCmd = {ptSystemManagerApp->fTargetRpm};
    JUNO_POINTER_T tEngineCmdPointer = EngineCmdMsg_PointerInit(&tEngineCmd);
    JUNO_TIME_MILLIS_RESULT_T tMillisResult = {0};
    tStatus = ptSystemManagerApp->tEngineTlmPipe.ptApi->Dequeue(&ptSystemManagerApp->tEngineTlmPipe.tRoot.tRoot, tTlmMsgPointer);
    JUNO_ASSERT_SUCCESS(tStatus, goto exit);
    tStatus = ptTime->ptApi->SubtractTime(ptTime, &tTlmMsg.tTimestamp, ptSystemManagerApp->tEngineStart);
    JUNO_ASSERT_SUCCESS(tStatus, goto exit);
    tMillisResult = ptTime->ptApi->TimestampToMillis(ptTime, tTlmMsg.tTimestamp);
    JUNO_ASSERT_SUCCESS(tMillisResult.tStatus, return tMillisResult.tStatus);
    ptLoggerApi->LogInfo(ptLogger, "Current Rpm: %f | Time: %ull", tTlmMsg.fRpm, tMillisResult.tOk);
    if(fabs(tTlmMsg.fRpm - ptSystemManagerApp->fTargetRpm) < 10.0f)
    {
        ptSystemManagerApp->fTargetRpm += 10.0f;
    }
exit:
    tEngineCmd.fRpm = ptSystemManagerApp->fTargetRpm;
    tStatus = ptBroker->ptApi->Publish(ptBroker, ENGINE_CMD_MSG_MID, tEngineCmdPointer);
    return tStatus;
}

static JUNO_STATUS_T OnExit(JUNO_APP_ROOT_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptJunoApp);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus)
    // Cast to the systemmanager app
    SYSTEM_MANAGER_APP_T *ptSystemManagerApp = (SYSTEM_MANAGER_APP_T *)(ptJunoApp);
    // Get the logger
    const JUNO_LOG_ROOT_T *ptLogger = ptSystemManagerApp->ptLogger;
    // Get the logger api
    const JUNO_LOG_API_T *ptLoggerApi = ptLogger->ptApi;
    // Log that the app is running
    ptLoggerApi->LogInfo(ptLogger, "SystemManager App Exiting");
    return tStatus;
}

static const JUNO_APP_API_T tSystemManagerAppApi = {
    .OnInit = OnInit,
    .OnProcess = OnProcess,
    .OnExit = OnExit
};

static inline JUNO_STATUS_T Verify(JUNO_APP_ROOT_T *ptJunoApp)
{
    // Assert the pointer is not null
    JUNO_ASSERT_EXISTS(ptJunoApp);
    // Cast to the systemmanager app
    SYSTEM_MANAGER_APP_T *ptSystemManagerApp = (SYSTEM_MANAGER_APP_T *)(ptJunoApp);
    // Assert the module dependencies are present
    JUNO_ASSERT_EXISTS_MODULE(
        /* TODO: Assert other dependencies and members here using &&*/
        ptSystemManagerApp &&
        ptSystemManagerApp->tRoot.ptApi &&
        ptSystemManagerApp->ptLogger &&
        ptSystemManagerApp->ptTime &&
        ptSystemManagerApp->ptBroker,
        ptSystemManagerApp,
        "Module does not have all dependencies"
    );
    // Verify that this application is using the correct API
    if(ptSystemManagerApp->tRoot.ptApi != &tSystemManagerAppApi)
    {
        JUNO_FAIL_MODULE(JUNO_STATUS_INVALID_TYPE_ERROR, ptSystemManagerApp, "Module has invalid API");
        return JUNO_STATUS_INVALID_TYPE_ERROR;
    }
    return JUNO_STATUS_SUCCESS;
}

/* TODO: Insert initialization arguments for module members here*/
JUNO_STATUS_T SystemManagerApp(
    SYSTEM_MANAGER_APP_T *ptSystemManagerApp,
    JUNO_LOG_ROOT_T *ptLogger,
    JUNO_TIME_ROOT_T *ptTime,
    JUNO_SB_BROKER_ROOT_T *ptBroker,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvUserData
)
{
    JUNO_ASSERT_EXISTS(ptSystemManagerApp);
    ptSystemManagerApp->tRoot.ptApi = &tSystemManagerAppApi;
    ptSystemManagerApp->tRoot.JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    ptSystemManagerApp->tRoot.JUNO_FAILURE_USER_DATA = pvUserData;
    ptSystemManagerApp->ptLogger = ptLogger;
    ptSystemManagerApp->ptTime = ptTime;
    ptSystemManagerApp->ptBroker = ptBroker;
    JUNO_STATUS_T tStatus = Verify(&ptSystemManagerApp->tRoot);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}


