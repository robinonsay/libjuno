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

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * @file main.cpp
 * @brief Composition root for the udp-threads example.
 *
 * @details
 *  Owns all static module instances. Wires dependencies bottom-up, spawns two
 *  POSIX threads (each with its own scheduler, broker, and applications), runs
 *  for 10 seconds, then performs cooperative shutdown.
 *
 *  Thread 1: SenderApp + MonitorApp via s_tSch1 and s_tBroker1.
 *  Thread 2: UdpBridgeApp + ProcessorApp via s_tSch2 and s_tBroker2.
 *
 *  No dynamic allocation. No global mutable state beyond the static instances
 *  declared here (which are owned exclusively by this translation unit).
 */

extern "C" {
#include "juno/sch/juno_sch_api.h"
#include "juno/sb/broker_api.h"
#include "juno/thread.h"
#include "udp_api.h"
#include "udp_msg_api.h"
#include "sender_app.h"
#include "monitor_app.h"
#include "udp_bridge_app.h"
#include "processor_app.h"
#include "juno/status.h"
}
#include <unistd.h>

/* --------------------------------------------------------------------------
 * Local scheduler vtable — no production Execute implementation exists in
 * LibJuno; this file-local vtable provides the simple table-driven dispatch
 * needed by Thread1Entry and Thread2Entry.
 * -------------------------------------------------------------------------- */

static JUNO_STATUS_T SchExecute(JUNO_SCH_ROOT_T *ptSch)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    for (size_t iFrame = 0u; iFrame < ptSch->zNumMinorFrames; iFrame++)
    {
        for (size_t iApp = 0u; iApp < ptSch->zAppsPerMinorFrame; iApp++)
        {
            size_t iIdx = iFrame * ptSch->zAppsPerMinorFrame + iApp;
            JUNO_APP_ROOT_T *ptApp = ptSch->ptArrSchTable[iIdx];
            if (ptApp && ptApp->ptApi && ptApp->ptApi->OnProcess)
            {
                tStatus = ptApp->ptApi->OnProcess(ptApp);
                if (tStatus != JUNO_STATUS_SUCCESS) { return tStatus; }
            }
        }
    }
    return tStatus;
}

static const JUNO_SCH_API_T s_tSchApi = { SchExecute, NULL, NULL };

/* --------------------------------------------------------------------------
 * Static module instances — all caller-owned, no heap allocation.
 * -------------------------------------------------------------------------- */

// @{"req": ["REQ-UDPAPP-019", "REQ-UDPAPP-022"]}
/* Thread roots */
static JUNO_THREAD_ROOT_T s_tThread1;
static JUNO_THREAD_ROOT_T s_tThread2;

/* Schedulers */
static JUNO_SCH_ROOT_T s_tSch1;
static JUNO_SCH_ROOT_T s_tSch2;

/* Brokers */
static JUNO_SB_BROKER_ROOT_T s_tBroker1;
static JUNO_SB_BROKER_ROOT_T s_tBroker2;

/* Broker pipe registries (capacity 2 — one subscriber per broker) */
static JUNO_SB_PIPE_T *s_aptBroker1Registry[2u];
static JUNO_SB_PIPE_T *s_aptBroker2Registry[2u];

/* UDP modules */
static JUNO_UDP_ROOT_T s_tUdpSender;
static JUNO_UDP_ROOT_T s_tUdpReceiver;

/* Message backing arrays for pipe queues */
static UDPTH_MSG_ARRAY_T s_tMsgArray1;   /* backs MonitorApp pipe on Broker1 */
static UDPTH_MSG_ARRAY_T s_tMsgArray2;   /* backs ProcessorApp pipe on Broker2 */

/* Application instances */
static SENDER_APP_T     s_tSenderApp;
static MONITOR_APP_T    s_tMonitorApp;
static UDP_BRIDGE_APP_T s_tBridgeApp;
static PROCESSOR_APP_T  s_tProcessorApp;

/* Scheduler tables — static so they outlive any function frame */
static JUNO_APP_ROOT_T *s_arrSchTable1[2u];
static JUNO_APP_ROOT_T *s_arrSchTable2[2u];

/* --------------------------------------------------------------------------
 * Thread entry functions
 * -------------------------------------------------------------------------- */

// @{"req": ["REQ-UDPAPP-020", "REQ-UDPAPP-021"]}
static void *Thread1Entry(void *pvArg)
{
    JUNO_THREAD_ROOT_T *ptRoot = (JUNO_THREAD_ROOT_T *)pvArg;
    if (s_tSenderApp.tRoot.ptApi->OnStart((JUNO_APP_ROOT_T *)&s_tSenderApp) != JUNO_STATUS_SUCCESS)
    {
        return NULL;
    }
    if (s_tMonitorApp.tRoot.ptApi->OnStart((JUNO_APP_ROOT_T *)&s_tMonitorApp) != JUNO_STATUS_SUCCESS)
    {
        return NULL;
    }
    while (!ptRoot->bStop)
    {
        (void)s_tSch1.ptApi->Execute(&s_tSch1);  /* MCP-resolved: SchExecute (local, this file) */
    }
    (void)s_tSenderApp.tRoot.ptApi->OnExit((JUNO_APP_ROOT_T *)&s_tSenderApp);
    (void)s_tMonitorApp.tRoot.ptApi->OnExit((JUNO_APP_ROOT_T *)&s_tMonitorApp);
    return NULL;
}

static void *Thread2Entry(void *pvArg)
{
    JUNO_THREAD_ROOT_T *ptRoot = (JUNO_THREAD_ROOT_T *)pvArg;
    if (s_tBridgeApp.tRoot.ptApi->OnStart((JUNO_APP_ROOT_T *)&s_tBridgeApp) != JUNO_STATUS_SUCCESS)
    {
        return NULL;
    }
    if (s_tProcessorApp.tRoot.ptApi->OnStart((JUNO_APP_ROOT_T *)&s_tProcessorApp) != JUNO_STATUS_SUCCESS)
    {
        return NULL;
    }
    while (!ptRoot->bStop)
    {
        (void)s_tSch2.ptApi->Execute(&s_tSch2);  /* MCP-resolved: SchExecute (local, this file) */
    }
    (void)s_tBridgeApp.tRoot.ptApi->OnExit((JUNO_APP_ROOT_T *)&s_tBridgeApp);
    (void)s_tProcessorApp.tRoot.ptApi->OnExit((JUNO_APP_ROOT_T *)&s_tProcessorApp);
    return NULL;
}

/* --------------------------------------------------------------------------
 * main — initialization, startup, shutdown
 * -------------------------------------------------------------------------- */

// @{"req": ["REQ-UDPAPP-001", "REQ-UDPAPP-023"]}
int main(void)
{
    JUNO_STATUS_T tStatus;

    /* 1. Initialize UDP sender */
    tStatus = JunoUdp_Init(&s_tUdpSender, &g_junoUdpLinuxApi, NULL, NULL);
    if (tStatus != JUNO_STATUS_SUCCESS) { return 1; }

    /* 2. Initialize UDP receiver */
    tStatus = JunoUdp_Init(&s_tUdpReceiver, &g_junoUdpLinuxApi, NULL, NULL);
    if (tStatus != JUNO_STATUS_SUCCESS) { return 1; }

    /* 3. Initialize message backing array for MonitorApp pipe */
    tStatus = UdpThreadMsgArray_Init(&s_tMsgArray1, NULL, NULL);
    if (tStatus != JUNO_STATUS_SUCCESS) { return 1; }

    /* 4. Initialize message backing array for ProcessorApp pipe */
    tStatus = UdpThreadMsgArray_Init(&s_tMsgArray2, NULL, NULL);
    if (tStatus != JUNO_STATUS_SUCCESS) { return 1; }

    /* 5. Initialize Thread 1 broker */
    tStatus = JunoSb_BrokerInit(&s_tBroker1, s_aptBroker1Registry, 2u, NULL, NULL);
    if (tStatus != JUNO_STATUS_SUCCESS) { return 1; }

    /* 6. Initialize Thread 2 broker */
    tStatus = JunoSb_BrokerInit(&s_tBroker2, s_aptBroker2Registry, 2u, NULL, NULL);
    if (tStatus != JUNO_STATUS_SUCCESS) { return 1; }

    /* 7. Initialize SenderApp */
    tStatus = SenderApp_Init(&s_tSenderApp, &g_tSenderAppApi,
                             &s_tUdpSender, &s_tBroker1, NULL, NULL);
    if (tStatus != JUNO_STATUS_SUCCESS) { return 1; }

    /* 8. Initialize MonitorApp */
    tStatus = MonitorApp_Init(&s_tMonitorApp, &g_tMonitorAppApi,
                              &s_tBroker1, &s_tMsgArray1.tRoot, NULL, NULL);
    if (tStatus != JUNO_STATUS_SUCCESS) { return 1; }

    /* 9. Initialize UdpBridgeApp */
    tStatus = UdpBridgeApp_Init(&s_tBridgeApp, &g_tUdpBridgeAppApi,
                                &s_tUdpReceiver, &s_tBroker2, NULL, NULL);
    if (tStatus != JUNO_STATUS_SUCCESS) { return 1; }

    /* 10. Initialize ProcessorApp */
    tStatus = ProcessorApp_Init(&s_tProcessorApp, &g_tProcessorAppApi,
                                &s_tBroker2, &s_tMsgArray2.tRoot, NULL, NULL);
    if (tStatus != JUNO_STATUS_SUCCESS) { return 1; }

    /* 11. Build and wire Thread 1 scheduler */
    s_arrSchTable1[0u]         = (JUNO_APP_ROOT_T *)&s_tSenderApp;
    s_arrSchTable1[1u]         = (JUNO_APP_ROOT_T *)&s_tMonitorApp;
    s_tSch1.ptApi              = &s_tSchApi;
    s_tSch1.ptArrSchTable      = s_arrSchTable1;
    s_tSch1.zAppsPerMinorFrame = 2u;
    s_tSch1.zNumMinorFrames    = 1u;

    /* 12. Build and wire Thread 2 scheduler */
    s_arrSchTable2[0u]         = (JUNO_APP_ROOT_T *)&s_tBridgeApp;
    s_arrSchTable2[1u]         = (JUNO_APP_ROOT_T *)&s_tProcessorApp;
    s_tSch2.ptApi              = &s_tSchApi;
    s_tSch2.ptArrSchTable      = s_arrSchTable2;
    s_tSch2.zAppsPerMinorFrame = 2u;
    s_tSch2.zNumMinorFrames    = 1u;

    /* 13. Initialize Thread 1 */
    tStatus = JunoThread_Init(&s_tThread1, &g_junoThreadLinuxApi, NULL, NULL);
    if (tStatus != JUNO_STATUS_SUCCESS) { return 1; }

    /* 14. Initialize Thread 2 */
    tStatus = JunoThread_Init(&s_tThread2, &g_junoThreadLinuxApi, NULL, NULL);
    if (tStatus != JUNO_STATUS_SUCCESS) { return 1; }

    /* Start Thread 1 */
    tStatus = s_tThread1.ptApi->Create(&s_tThread1, Thread1Entry, &s_tThread1);
    if (tStatus != JUNO_STATUS_SUCCESS) { return 1; }

    /* Start Thread 2; on failure stop and join Thread 1 before returning */
    tStatus = s_tThread2.ptApi->Create(&s_tThread2, Thread2Entry, &s_tThread2);
    if (tStatus != JUNO_STATUS_SUCCESS)
    {
        tStatus = s_tThread1.ptApi->Stop(&s_tThread1);
        tStatus = s_tThread1.ptApi->Join(&s_tThread1);
        return 1;
    }

    sleep(10);

    /* Cooperative shutdown — Stop errors are non-fatal */
    tStatus = s_tThread1.ptApi->Stop(&s_tThread1);
    tStatus = s_tThread2.ptApi->Stop(&s_tThread2);

    tStatus = s_tThread1.ptApi->Join(&s_tThread1);
    if (tStatus != JUNO_STATUS_SUCCESS) { return 1; }

    tStatus = s_tThread2.ptApi->Join(&s_tThread2);
    if (tStatus != JUNO_STATUS_SUCCESS) { return 1; }

    return 0;
}
