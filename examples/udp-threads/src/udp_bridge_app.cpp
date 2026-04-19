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

#include "udp_bridge_app.h"
#include "udp_msg_api.h"
#include "juno/macros.h"
#include <stdio.h>

/* --------------------------------------------------------------------------
 * Forward declarations of lifecycle functions (referenced by vtable below)
 * -------------------------------------------------------------------------- */

JUNO_STATUS_T UdpBridgeApp_OnStart(JUNO_APP_ROOT_T *ptApp);
JUNO_STATUS_T UdpBridgeApp_OnProcess(JUNO_APP_ROOT_T *ptApp);
JUNO_STATUS_T UdpBridgeApp_OnExit(JUNO_APP_ROOT_T *ptApp);

/* --------------------------------------------------------------------------
 * Global vtable
 * -------------------------------------------------------------------------- */

const JUNO_APP_API_T g_tUdpBridgeAppApi = {
    UdpBridgeApp_OnStart,
    UdpBridgeApp_OnProcess,
    UdpBridgeApp_OnExit
};

/* --------------------------------------------------------------------------
 * UdpBridgeApp_Init
 * -------------------------------------------------------------------------- */

// @{"req": ["REQ-UDPAPP-011"]}
JUNO_STATUS_T UdpBridgeApp_Init(
    UDP_BRIDGE_APP_T              *ptApp,
    const JUNO_APP_API_T          *ptApi,
    JUNO_UDP_ROOT_T               *ptUdp,
    JUNO_SB_BROKER_ROOT_T         *ptBroker,
    JUNO_FAILURE_HANDLER_T         pfcnFailureHandler,
    void                          *pvFailureUserData
)
{
    JUNO_ASSERT_EXISTS(ptApp);
    JUNO_ASSERT_EXISTS(ptApi);
    JUNO_ASSERT_EXISTS(ptUdp);
    JUNO_ASSERT_EXISTS(ptBroker);

    ptApp->tRoot.ptApi                      = ptApi;
    ptApp->ptUdp                            = ptUdp;
    ptApp->ptBroker                         = ptBroker;
    ptApp->tRoot.JUNO_FAILURE_HANDLER       = pfcnFailureHandler;
    ptApp->tRoot.JUNO_FAILURE_USER_DATA     = (JUNO_USER_DATA_T *)pvFailureUserData;

    return JUNO_STATUS_SUCCESS;
}

/* --------------------------------------------------------------------------
 * Lifecycle implementations
 * -------------------------------------------------------------------------- */

// @{"req": ["REQ-UDPAPP-012"]}
JUNO_STATUS_T UdpBridgeApp_OnStart(JUNO_APP_ROOT_T *ptApp)
{
    JUNO_ASSERT_EXISTS(ptApp);
    UDP_BRIDGE_APP_T *ptBridge = (UDP_BRIDGE_APP_T *)ptApp;

    JUNO_UDP_CFG_T tCfg;
    tCfg.pcAddress  = "127.0.0.1";
    tCfg.uPort      = 9000u;
    tCfg.uTimeoutMs = 100u;
    tCfg.bIsReceiver = true;

    return ptBridge->ptUdp->ptApi->Open(ptBridge->ptUdp, &tCfg);
}

// @{"req": ["REQ-UDPAPP-013", "REQ-UDPAPP-014"]}
JUNO_STATUS_T UdpBridgeApp_OnProcess(JUNO_APP_ROOT_T *ptApp)
{
    JUNO_ASSERT_EXISTS(ptApp);
    UDP_BRIDGE_APP_T *ptBridge = (UDP_BRIDGE_APP_T *)ptApp;

    UDP_THREAD_MSG_T tMsg;
    JUNO_STATUS_T tStatus = ptBridge->ptUdp->ptApi->Receive(ptBridge->ptUdp, &tMsg);

    if (tStatus == JUNO_STATUS_TIMEOUT_ERROR)
    {
        return JUNO_STATUS_SUCCESS;  /* normal: no datagram this cycle */
    }
    if (tStatus != JUNO_STATUS_SUCCESS)
    {
        return tStatus;  /* unexpected error — propagate */
    }

    /* Received successfully — publish to Thread 2's broker */
    printf("[UdpBridgeApp] rx seq=%u → forwarding\n", tMsg.uSeqNum);
    JUNO_POINTER_T tPtr;
    tPtr.ptApi       = &g_udpThreadMsgPointerApi;
    tPtr.pvAddr      = &tMsg;
    tPtr.zSize       = sizeof(UDP_THREAD_MSG_T);
    tPtr.zAlignment  = alignof(UDP_THREAD_MSG_T);
    tStatus = ptBridge->ptBroker->ptApi->Publish(
        ptBridge->ptBroker, UDPTH_MSG_MID, tPtr);
    return tStatus;
}

// @{"req": ["REQ-UDPAPP-015"]}
JUNO_STATUS_T UdpBridgeApp_OnExit(JUNO_APP_ROOT_T *ptApp)
{
    JUNO_ASSERT_EXISTS(ptApp);
    UDP_BRIDGE_APP_T *ptBridge = (UDP_BRIDGE_APP_T *)ptApp;
    return ptBridge->ptUdp->ptApi->Close(ptBridge->ptUdp);
}
