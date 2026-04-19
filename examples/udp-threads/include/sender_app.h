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
 * @file sender_app.h
 * @brief Public interface for SenderApp — the UDP sender application on Thread 1.
 *
 * @details
 *  SenderApp implements the @c JUNO_APP_API_T lifecycle interface. On each
 *  scheduler cycle it builds a @c UDP_THREAD_MSG_T with a monotonically
 *  incrementing sequence counter, publishes it to Thread 1's software-bus
 *  broker (so MonitorApp can observe it locally), and transmits it via the
 *  UDP module to the loopback address so UdpBridgeApp on Thread 2 can receive
 *  it.
 *
 *  All memory is caller-owned and injected. SenderApp allocates nothing.
 *
 *  Typical usage:
 *  @code{.c}
 *  SENDER_APP_T tSender;
 *  SenderApp_Init(&tSender,
 *                 &tUdp.tRoot, &tBroker,
 *                 MyFailureHandler, NULL);
 *
 *  tSender.tRoot.ptApi->OnStart(&tSender.tRoot);
 *  // ... scheduler loop ...
 *  tSender.tRoot.ptApi->OnProcess(&tSender.tRoot);
 *  // ...
 *  tSender.tRoot.ptApi->OnExit(&tSender.tRoot);
 *  @endcode
 */
#ifndef SENDER_APP_H
#define SENDER_APP_H

#include <stdint.h>
#include "juno/app/app_api.h"
#include "juno/sb/broker_api.h"
#include "udp_api.h"
#include "juno/module.h"
#include "juno/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* --------------------------------------------------------------------------
 * SENDER_APP_T — concrete SenderApp struct
 * -------------------------------------------------------------------------- */

/**
 * @brief Concrete SenderApp instance.
 *
 * @details
 *  Embeds @c JUNO_APP_ROOT_T via @c JUNO_MODULE_DERIVE, accessible as
 *  @c JUNO_MODULE_SUPER (@c tRoot), enabling safe up-cast from
 *  a @c JUNO_APP_ROOT_T * to @c SENDER_APP_T * inside the lifecycle callbacks.
 *  The scheduler holds a @c JUNO_APP_ROOT_T * and dispatches via the vtable;
 *  each callback recovers the full struct with:
 *  @code{.c}
 *  SENDER_APP_T *ptSender = (SENDER_APP_T *)ptApp;
 *  @endcode
 *
 *  All fields other than the embedded root are either injected at @c SenderApp_Init
 *  time or are private mutable state (@c _uSeqNum). The composition root owns
 *  this struct; no field is heap-allocated.
 */
struct SENDER_APP_TAG JUNO_MODULE_DERIVE(JUNO_APP_ROOT_T,
    /** @brief Injected UDP module (configured as sender). Lifetime must exceed SenderApp. */
    JUNO_UDP_ROOT_T       *ptUdp;
    /** @brief Injected Thread 1 broker. Lifetime must exceed SenderApp. */
    JUNO_SB_BROKER_ROOT_T *ptBroker;
    /** @brief Private: monotonic sequence counter; zero-initialized in OnStart. */
    uint32_t               _uSeqNum;
);
typedef struct SENDER_APP_TAG SENDER_APP_T;

/* --------------------------------------------------------------------------
 * SenderApp_Init — wire vtable and inject dependencies
 * -------------------------------------------------------------------------- */

/**
 * @brief Initialize a SenderApp instance.
 *
 * @details
 *  Wires the internal static vtable into @p ptApp->tRoot.ptApi, stores
 *  @p ptUdp and @p ptBroker, stores the failure handler and user data into
 *  the root, and sets @c _uSeqNum to zero. Verifies that no required injected
 *  pointer is NULL before returning.
 *
 *  Must be called before any lifecycle operation. All caller-allocated storage;
 *  this function does not allocate any memory.
 *
 * @param ptApp               Caller-owned SenderApp storage; must be non-NULL.
 * @param ptUdp               UDP module root configured for sender role; must be
 *                            non-NULL and outlive @p ptApp.
 * @param ptBroker            Thread 1 broker root; must be non-NULL and outlive
 *                            @p ptApp.
 * @param pfcnFailureHandler  Diagnostic callback invoked before any error return;
 *                            may be NULL.
 * @param pvFailureUserData   Opaque pointer passed to the failure handler; may be NULL.
 * @return @c JUNO_STATUS_SUCCESS on success; @c JUNO_STATUS_NULLPTR_ERROR if any
 *         required pointer is NULL.
 */
// @{"req": ["REQ-UDPAPP-002"]}
JUNO_STATUS_T SenderApp_Init(
    SENDER_APP_T              *ptApp,
    JUNO_UDP_ROOT_T           *ptUdp,
    JUNO_SB_BROKER_ROOT_T     *ptBroker,
    JUNO_FAILURE_HANDLER_T     pfcnFailureHandler,
    void                      *pvFailureUserData
);

#ifdef __cplusplus
}
#endif

#endif /* SENDER_APP_H */
