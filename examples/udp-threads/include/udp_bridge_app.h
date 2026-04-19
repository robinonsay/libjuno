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
 *  UdpBridgeApp runs on Thread 2. On each scheduler cycle it attempts to receive
 *  one datagram from the already-open UDP receiver socket. When a datagram arrives
 *  it is published to Thread 2's software-bus broker so that ProcessorApp may
 *  consume it. When the receive times out the app returns success immediately
 *  without publishing.
 *
 *  This module follows the LibJuno vtable / dependency-injection pattern:
 *  - @c UDP_BRIDGE_APP_T embeds @c JUNO_APP_ROOT_T as its first member,
 *    enabling safe upcast from @c JUNO_APP_ROOT_T* to the concrete app pointer.
 *  - All dependencies (@c JUNO_UDP_ROOT_T, @c JUNO_SB_BROKER_ROOT_T) are
 *    injected at @c UdpBridgeApp_Init time. The app allocates nothing.
 *  - The concrete lifecycle functions are @c static inside @c udp_bridge_app.cpp
 *    and are wired into a @c static @c const vtable in that translation unit.
 *    Callers never reference the vtable by name.
 *
 *  Typical usage:
 *  @code{.c}
 *  UDP_BRIDGE_APP_T tBridgeApp;
 *  UdpBridgeApp_Init(
 *      &tBridgeApp,
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
 *  Wires the internal static vtable into @p ptApp->tRoot.ptApi, stores
 *  @p ptUdp and @p ptBroker, stores the failure handler and user data into
 *  @p ptApp->tRoot, and verifies that no required pointer is NULL. Must be
 *  called before any lifecycle function.
 *
 *  The UDP socket is opened by the composition root (@c main.cpp) via
 *  @c JunoUdp_LinuxInit before the thread starts; @c OnStart does not
 *  re-open it.
 *
 * @param ptApp               Caller-owned app instance storage; must be non-NULL.
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
    JUNO_UDP_ROOT_T               *ptUdp,
    JUNO_SB_BROKER_ROOT_T         *ptBroker,
    JUNO_FAILURE_HANDLER_T         pfcnFailureHandler,
    void                          *pvFailureUserData
);

#ifdef __cplusplus
}
#endif

#endif /* UDP_BRIDGE_APP_H */
