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

#include "monitor_app.h"
#include "udp_msg_api.h"
#include "juno/macros.h"

/* --------------------------------------------------------------------------
 * Lifecycle implementations
 * -------------------------------------------------------------------------- */

// @{"req": ["REQ-UDPAPP-009"]}
JUNO_STATUS_T MonitorApp_OnStart(JUNO_APP_ROOT_T *ptApp)
{
    JUNO_ASSERT_EXISTS(ptApp);
    MONITOR_APP_T *ptMonitor = (MONITOR_APP_T *)ptApp;

    JUNO_STATUS_T tStatus = JunoSb_PipeInit(
        &ptMonitor->tPipe,
        UDPTH_MSG_MID,
        ptMonitor->_ptPipeArray,
        ptMonitor->_pfcnFailureHandler,
        ptMonitor->_pvFailureUserData);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);

    tStatus = ptMonitor->ptBroker->ptApi->RegisterSubscriber(
        ptMonitor->ptBroker, &ptMonitor->tPipe);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);

    return JUNO_STATUS_SUCCESS;
}

// @{"req": ["REQ-UDPAPP-010"]}
JUNO_STATUS_T MonitorApp_OnProcess(JUNO_APP_ROOT_T *ptApp)
{
    JUNO_ASSERT_EXISTS(ptApp);
    MONITOR_APP_T *ptMonitor = (MONITOR_APP_T *)ptApp;

    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    while (true)
    {
        UDP_THREAD_MSG_T tMsg;
        /* Avoid C99 compound-literal macro; build JUNO_POINTER_T field-by-field
         * so this translation unit compiles cleanly as C++11. */
        JUNO_POINTER_T tReturn;
        tReturn.ptApi      = &g_udpThreadMsgPointerApi;
        tReturn.pvAddr     = &tMsg;
        tReturn.zSize      = sizeof(UDP_THREAD_MSG_T);
        tReturn.zAlignment = alignof(UDP_THREAD_MSG_T);
        tStatus = ptMonitor->tPipe.tRoot.ptApi->Dequeue(
            &ptMonitor->tPipe.tRoot, tReturn);
        if (tStatus == JUNO_STATUS_OOB_ERROR)
        {
            /* Queue empty — normal drain-complete signal, not an error. */
            break;
        }
        if (tStatus != JUNO_STATUS_SUCCESS)
        {
            return tStatus;
        }
        /* Message consumed. In a production app, log/print tMsg here.
         * For this example we just drain the queue. */
        (void)tMsg;
    }
    return JUNO_STATUS_SUCCESS;
}

JUNO_STATUS_T MonitorApp_OnExit(JUNO_APP_ROOT_T *ptApp)
{
    (void)ptApp;
    return JUNO_STATUS_SUCCESS;
}

/* --------------------------------------------------------------------------
 * Vtable
 * -------------------------------------------------------------------------- */

const JUNO_APP_API_T g_tMonitorAppApi = {
    MonitorApp_OnStart,
    MonitorApp_OnProcess,
    MonitorApp_OnExit
};

/* --------------------------------------------------------------------------
 * Init
 * -------------------------------------------------------------------------- */

// @{"req": ["REQ-UDPAPP-008"]}
JUNO_STATUS_T MonitorApp_Init(
    MONITOR_APP_T              *ptApp,
    const JUNO_APP_API_T       *ptApi,
    JUNO_SB_BROKER_ROOT_T      *ptBroker,
    JUNO_DS_ARRAY_ROOT_T       *ptPipeArray,
    JUNO_FAILURE_HANDLER_T      pfcnFailureHandler,
    void                       *pvFailureUserData
)
{
    JUNO_ASSERT_EXISTS(ptApp);
    JUNO_ASSERT_EXISTS(ptApi);
    JUNO_ASSERT_EXISTS(ptBroker);
    JUNO_ASSERT_EXISTS(ptPipeArray);

    ptApp->tRoot.ptApi             = ptApi;
    ptApp->ptBroker                = ptBroker;
    ptApp->_ptPipeArray            = ptPipeArray;
    ptApp->_pfcnFailureHandler     = pfcnFailureHandler;
    ptApp->_pvFailureUserData      = pvFailureUserData;

    return JUNO_STATUS_SUCCESS;
}
