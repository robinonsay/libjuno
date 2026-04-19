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

#include "processor_app.h"
#include "udp_msg_api.h"
#include "juno/macros.h"
#include <stdio.h>

/* --------------------------------------------------------------------------
 * Forward declarations — static lifecycle functions
 * -------------------------------------------------------------------------- */

static JUNO_STATUS_T OnStart  (JUNO_APP_ROOT_T *ptApp);
static JUNO_STATUS_T OnProcess(JUNO_APP_ROOT_T *ptApp);
static JUNO_STATUS_T OnExit   (JUNO_APP_ROOT_T *ptApp);

/* --------------------------------------------------------------------------
 * Internal vtable — static, never exposed outside this translation unit
 * -------------------------------------------------------------------------- */

static const JUNO_APP_API_T s_tProcessorAppApi = {
    OnStart,
    OnProcess,
    OnExit
};

/* --------------------------------------------------------------------------
 * Lifecycle implementations
 * -------------------------------------------------------------------------- */

// @{"req": ["REQ-UDPAPP-017"]}
static JUNO_STATUS_T OnStart(JUNO_APP_ROOT_T *ptApp)
{
    JUNO_ASSERT_EXISTS(ptApp);
    PROCESSOR_APP_T *ptProcessor = (PROCESSOR_APP_T *)ptApp;

    JUNO_STATUS_T tStatus = JunoSb_PipeInit(
        &ptProcessor->tPipe,
        UDPTH_MSG_MID,
        ptProcessor->_ptPipeArray,
        ptProcessor->_pfcnFailureHandler,
        ptProcessor->_pvFailureUserData);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);

    tStatus = ptProcessor->ptBroker->ptApi->RegisterSubscriber(
        ptProcessor->ptBroker, &ptProcessor->tPipe);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);

    return JUNO_STATUS_SUCCESS;
}

// @{"req": ["REQ-UDPAPP-018"]}
static JUNO_STATUS_T OnProcess(JUNO_APP_ROOT_T *ptApp)
{
    JUNO_ASSERT_EXISTS(ptApp);
    PROCESSOR_APP_T *ptProcessor = (PROCESSOR_APP_T *)ptApp;

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
        tStatus = ptProcessor->tPipe.tRoot.ptApi->Dequeue(
            &ptProcessor->tPipe.tRoot, tReturn);
        if (tStatus == JUNO_STATUS_OOB_ERROR)
        {
            /* Queue empty — normal drain-complete signal, not an error. */
            break;
        }
        if (tStatus != JUNO_STATUS_SUCCESS)
        {
            return tStatus;
        }
        printf("[ProcessorApp] processed seq=%u\n", tMsg.uSeqNum);
    }
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T OnExit(JUNO_APP_ROOT_T *ptApp)
{
    (void)ptApp;
    return JUNO_STATUS_SUCCESS;
}

/* --------------------------------------------------------------------------
 * Init
 * -------------------------------------------------------------------------- */

// @{"req": ["REQ-UDPAPP-016"]}
JUNO_STATUS_T ProcessorApp_Init(
    PROCESSOR_APP_T              *ptApp,
    JUNO_SB_BROKER_ROOT_T        *ptBroker,
    JUNO_DS_ARRAY_ROOT_T         *ptPipeArray,
    JUNO_FAILURE_HANDLER_T        pfcnFailureHandler,
    void                         *pvFailureUserData
)
{
    JUNO_ASSERT_EXISTS(ptApp);
    JUNO_ASSERT_EXISTS(ptBroker);
    JUNO_ASSERT_EXISTS(ptPipeArray);

    ptApp->tRoot.ptApi             = &s_tProcessorAppApi;
    ptApp->ptBroker                = ptBroker;
    ptApp->_ptPipeArray            = ptPipeArray;
    ptApp->_pfcnFailureHandler     = pfcnFailureHandler;
    ptApp->_pvFailureUserData      = pvFailureUserData;

    return JUNO_STATUS_SUCCESS;
}
