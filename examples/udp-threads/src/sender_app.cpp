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

extern "C" {
#include "sender_app.h"
#include "udp_msg_api.h"
#include "juno/status.h"
#include "juno/macros.h"
#include <stdio.h>
}
#include <string.h>
#include <stdalign.h>

/* --------------------------------------------------------------------------
 * Lifecycle vtable implementations
 * -------------------------------------------------------------------------- */

// @{"req": ["REQ-UDPAPP-003"]}
JUNO_STATUS_T SenderApp_OnStart(JUNO_APP_ROOT_T *ptApp)
{
    JUNO_ASSERT_EXISTS(ptApp);
    SENDER_APP_T *ptSender = (SENDER_APP_T *)ptApp;

    JUNO_UDP_CFG_T tCfg;
    tCfg.pcAddress   = "127.0.0.1";
    tCfg.uPort       = 9000u;
    tCfg.uTimeoutMs  = 0u;
    tCfg.bIsReceiver = false;

    JUNO_STATUS_T tStatus = ptSender->ptUdp->ptApi->Open(ptSender->ptUdp, &tCfg);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);

    ptSender->_uSeqNum = 0u;
    return JUNO_STATUS_SUCCESS;
}

// @{"req": ["REQ-UDPAPP-004", "REQ-UDPAPP-005", "REQ-UDPAPP-006"]}
JUNO_STATUS_T SenderApp_OnProcess(JUNO_APP_ROOT_T *ptApp)
{
    JUNO_ASSERT_EXISTS(ptApp);
    SENDER_APP_T *ptSender = (SENDER_APP_T *)ptApp;

    UDP_THREAD_MSG_T tMsg;
    memset(&tMsg, 0, sizeof(tMsg));
    tMsg.uSeqNum          = ++ptSender->_uSeqNum;
    tMsg.uTimestampSec    = 0u;
    tMsg.uTimestampSubSec = 0u;

    /* Publish to Thread 1's broker (REQ-UDPAPP-004) */
    JUNO_POINTER_T tPtr;
    tPtr.ptApi      = &g_udpThreadMsgPointerApi;
    tPtr.pvAddr     = &tMsg;
    tPtr.zSize      = sizeof(UDP_THREAD_MSG_T);
    tPtr.zAlignment = alignof(UDP_THREAD_MSG_T);
    JUNO_STATUS_T tStatus = ptSender->ptBroker->ptApi->Publish(
        ptSender->ptBroker, UDPTH_MSG_MID, tPtr);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);

    printf("[SenderApp]    tx seq=%u\n", tMsg.uSeqNum);

    /* Transmit via UDP (REQ-UDPAPP-005) */
    tStatus = ptSender->ptUdp->ptApi->Send(ptSender->ptUdp, &tMsg);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);

    return JUNO_STATUS_SUCCESS;
}

// @{"req": ["REQ-UDPAPP-007"]}
JUNO_STATUS_T SenderApp_OnExit(JUNO_APP_ROOT_T *ptApp)
{
    JUNO_ASSERT_EXISTS(ptApp);
    SENDER_APP_T *ptSender = (SENDER_APP_T *)ptApp;
    return ptSender->ptUdp->ptApi->Close(ptSender->ptUdp);
}

/* --------------------------------------------------------------------------
 * Production vtable
 * -------------------------------------------------------------------------- */

const JUNO_APP_API_T g_tSenderAppApi = {
    SenderApp_OnStart,
    SenderApp_OnProcess,
    SenderApp_OnExit
};

/* --------------------------------------------------------------------------
 * Init — wire vtable and inject dependencies
 * -------------------------------------------------------------------------- */

// @{"req": ["REQ-UDPAPP-002"]}
JUNO_STATUS_T SenderApp_Init(
    SENDER_APP_T              *ptApp,
    const JUNO_APP_API_T      *ptApi,
    JUNO_UDP_ROOT_T           *ptUdp,
    JUNO_SB_BROKER_ROOT_T     *ptBroker,
    JUNO_FAILURE_HANDLER_T     pfcnFailureHandler,
    void                      *pvFailureUserData
)
{
    JUNO_ASSERT_EXISTS(ptApp);
    JUNO_ASSERT_EXISTS(ptApi);
    JUNO_ASSERT_EXISTS(ptUdp);
    JUNO_ASSERT_EXISTS(ptBroker);

    ptApp->tRoot.ptApi                  = ptApi;
    ptApp->tRoot.JUNO_FAILURE_HANDLER   = pfcnFailureHandler;
    ptApp->tRoot.JUNO_FAILURE_USER_DATA = (JUNO_USER_DATA_T *)pvFailureUserData;
    ptApp->ptUdp                        = ptUdp;
    ptApp->ptBroker                     = ptBroker;
    ptApp->_uSeqNum                     = 0;

    return JUNO_STATUS_SUCCESS;
}
