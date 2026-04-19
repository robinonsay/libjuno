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

/**
 * @file udp_bridge_app.h
 * @brief Public interface for UdpBridgeApp — the UDP-to-broker bridge application.
 *
 * @details
 *  UdpBridgeApp runs on Thread 2. It opens a UDP receiver socket bound to port
 *  9000 and on each scheduler cycle attempts to receive one datagram. When a
 *  datagram arrives it is published to Thread 2's software-bus broker so that
 *  ProcessorApp may consume it. When the receive times out the app returns
 *  success immediately without publishing.
 *
 *  This module follows the LibJuno vtable / dependency-injection pattern:
 *  - @c UDP_BRIDGE_APP_T embeds @c JUNO_APP_ROOT_T as its first member,
 *    enabling safe upcast from @c JUNO_APP_ROOT_T* to the concrete app pointer.
 *  - All dependencies (@c JUNO_UDP_ROOT_T, @c JUNO_SB_BROKER_ROOT_T) are
 *    injected at @c UdpBridgeApp_Init time. The app allocates nothing.
 *  - The concrete lifecycle functions (@c UdpBridgeApp_OnStart,
 *    @c UdpBridgeApp_OnProcess, @c UdpBridgeApp_OnExit) match the
 *    @c JUNO_APP_API_T vtable signatures and are collected in
 *    @c g_tUdpBridgeAppApi.
 *
 *  Typical usage:
 *  @code{.c}
 *  UDP_BRIDGE_APP_T tBridgeApp;
 *  UdpBridgeApp_Init(
 *      &tBridgeApp,
 *      &g_tUdpBridgeAppApi,
 *      &tUdpReceiver.tRoot,
 *      &tBroker2.tRoot,
 *      NULL, NULL
 *  );
 *
 *  JUNO_APP_ROOT_T *ptApp = &tBridgeApp.tRoot;
 *  ptApp->ptApi->OnStart(ptApp);
 *  // ... scheduler loop ...
 *  ptApp->ptApi->OnProcess(ptApp);
 *  // ...
 *  ptApp->ptApi->OnExit(ptApp);
 *  @endcode
 */
#ifndef UDP_BRIDGE_APP_H
#define UDP_BRIDGE_APP_H

#include "juno/app/app_api.h"
#include "juno/sb/broker_api.h"
#include "udp_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* --------------------------------------------------------------------------
 * UDP_BRIDGE_APP_T — concrete application struct
 * -------------------------------------------------------------------------- */

/**
 * @brief Concrete struct for UdpBridgeApp.
 *
 * @details
 *  Embeds @c JUNO_APP_ROOT_T (as @c JUNO_MODULE_SUPER) via @c JUNO_MODULE_DERIVE
 *  so that the scheduler can hold a @c JUNO_APP_ROOT_T* and dispatch lifecycle
 *  calls through the vtable. The concrete implementation recovers full state by
 *  casting:
 *  @code{.c}
 *  UDP_BRIDGE_APP_T *ptBridge = (UDP_BRIDGE_APP_T *)ptApp;
 *  @endcode
 *
 *  All instances are stack- or statically-allocated by the composition root.
 *  No member of this struct is heap-allocated.
 */
struct UDP_BRIDGE_APP_TAG JUNO_MODULE_DERIVE(JUNO_APP_ROOT_T,
    /** @brief Injected UDP module instance (receiver role); must outlive this app. */
    JUNO_UDP_ROOT_T       *ptUdp;
    /** @brief Injected Thread 2 software-bus broker; must outlive this app. */
    JUNO_SB_BROKER_ROOT_T *ptBroker;
);
typedef struct UDP_BRIDGE_APP_TAG UDP_BRIDGE_APP_T;

/* --------------------------------------------------------------------------
 * UdpBridgeApp_Init
 * -------------------------------------------------------------------------- */

/**
 * @brief Initialize a UdpBridgeApp instance with its dependencies.
 *
 * @details
 *  Wires @p ptApi into @p ptApp->tRoot.ptApi, stores @p ptUdp and @p ptBroker,
 *  stores the failure handler and user data into @p ptApp->tRoot, and verifies
 *  that no required pointer is NULL. Must be called before any lifecycle function.
 *
 *  This function does not open the UDP socket; that is deferred to
 *  @c UdpBridgeApp_OnStart.
 *
 * @param ptApp               Caller-owned app instance storage; must be non-NULL.
 * @param ptApi               Vtable to wire; pass @c &g_tUdpBridgeAppApi for the
 *                            standard implementation.
 * @param ptUdp               UDP module root (receiver role); must be non-NULL and
 *                            must outlive @p ptApp.
 * @param ptBroker            Thread 2's software-bus broker root; must be non-NULL
 *                            and must outlive @p ptApp.
 * @param pfcnFailureHandler  Diagnostic callback invoked before any error return;
 *                            may be NULL.
 * @param pvFailureUserData   Opaque pointer threaded to @p pfcnFailureHandler;
 *                            may be NULL.
 * @return @c JUNO_STATUS_SUCCESS on success; @c JUNO_STATUS_NULLPTR_ERROR if any
 *         required pointer is NULL.
 */
JUNO_STATUS_T UdpBridgeApp_Init(
    UDP_BRIDGE_APP_T              *ptApp,
    const JUNO_APP_API_T          *ptApi,
    JUNO_UDP_ROOT_T               *ptUdp,
    JUNO_SB_BROKER_ROOT_T         *ptBroker,
    JUNO_FAILURE_HANDLER_T         pfcnFailureHandler,
    void                          *pvFailureUserData
);

/* --------------------------------------------------------------------------
 * Lifecycle functions — wired into g_tUdpBridgeAppApi
 * -------------------------------------------------------------------------- */

/**
 * @brief Open the UDP receiver socket bound to port 9000.
 *
 * @details
 *  Configures the UDP module with:
 *  - address @c "127.0.0.1", port @c 9000, timeout @c 100 ms, receiver role.
 *
 *  Called once by the scheduler before the processing loop begins.
 *
 * @param ptApp Pointer to the @c JUNO_APP_ROOT_T embedded in a
 *              @c UDP_BRIDGE_APP_T instance; must be non-NULL.
 * @return @c JUNO_STATUS_SUCCESS on success; non-zero on failure.
 */
JUNO_STATUS_T UdpBridgeApp_OnStart(JUNO_APP_ROOT_T *ptApp);

/**
 * @brief Receive one UDP datagram and publish it to Thread 2's broker.
 *
 * @details
 *  On each invocation attempts to receive one @c UDP_THREAD_MSG_T datagram.
 *  - If the receive times out (@c JUNO_STATUS_TIMEOUT), returns
 *    @c JUNO_STATUS_SUCCESS immediately — a quiet cycle is not an error.
 *  - If the receive succeeds, wraps the message in a @c JUNO_POINTER_T and
 *    publishes it to the broker under @c UDPTH_MSG_MID.
 *  - Any other non-success status from @c Receive or @c Publish is propagated
 *    directly to the caller.
 *
 * @param ptApp Pointer to the @c JUNO_APP_ROOT_T embedded in a
 *              @c UDP_BRIDGE_APP_T instance; must be non-NULL.
 * @return @c JUNO_STATUS_SUCCESS on success or timeout; non-zero on failure.
 */
JUNO_STATUS_T UdpBridgeApp_OnProcess(JUNO_APP_ROOT_T *ptApp);

/**
 * @brief Close the UDP receiver socket.
 *
 * @details
 *  Delegates to the UDP module's @c Close operation. Called once by the
 *  scheduler after the processing loop ends.
 *
 * @param ptApp Pointer to the @c JUNO_APP_ROOT_T embedded in a
 *              @c UDP_BRIDGE_APP_T instance; must be non-NULL.
 * @return @c JUNO_STATUS_SUCCESS on success; non-zero on failure.
 */
JUNO_STATUS_T UdpBridgeApp_OnExit(JUNO_APP_ROOT_T *ptApp);

/* --------------------------------------------------------------------------
 * Global vtable
 * -------------------------------------------------------------------------- */

/**
 * @brief Statically allocated vtable for UdpBridgeApp.
 *
 * @details
 *  Pass to @c UdpBridgeApp_Init as @p ptApi. Must remain valid for the
 *  lifetime of all @c UDP_BRIDGE_APP_T instances initialised with it.
 *
 *  Members:
 *  - @c OnStart   → @c UdpBridgeApp_OnStart
 *  - @c OnProcess → @c UdpBridgeApp_OnProcess
 *  - @c OnExit    → @c UdpBridgeApp_OnExit
 */
extern const JUNO_APP_API_T g_tUdpBridgeAppApi;

#ifdef __cplusplus
}
#endif

#endif /* UDP_BRIDGE_APP_H */
