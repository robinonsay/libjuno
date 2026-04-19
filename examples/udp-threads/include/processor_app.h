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
 * @file processor_app.h
 * @brief Public interface for ProcessorApp — the message-processing application on Thread 2.
 *
 * @details
 *  ProcessorApp implements the @c JUNO_APP_API_T lifecycle interface. It subscribes
 *  to Thread 2's software-bus broker (via an embedded @c JUNO_SB_PIPE_T) and, on
 *  each scheduler cycle, dequeues and processes all messages that UdpBridgeApp has
 *  published under @c UDPTH_MSG_MID. It mirrors the MonitorApp pattern on Thread 1.
 *
 *  The subscription pipe (@c tPipe) is embedded directly in the struct — no separate
 *  allocation is required. Its backing array (@c _ptPipeArray) and the broker pointer
 *  (@c ptBroker) are injected at @c ProcessorApp_Init time; pipe initialization itself
 *  is deferred to @c ProcessorApp_OnStart so that registration with the broker occurs
 *  in the correct lifecycle phase.
 *
 *  All memory is caller-owned and injected. ProcessorApp allocates nothing.
 *
 *  Typical usage:
 *  @code{.c}
 *  PROCESSOR_APP_T tProcessor;
 *  ProcessorApp_Init(&tProcessor,
 *                    &tBroker2, &tPipeArray,
 *                    MyFailureHandler, NULL);
 *
 *  tProcessor.tRoot.ptApi->OnStart(&tProcessor.tRoot);
 *  // ... scheduler loop ...
 *  tProcessor.tRoot.ptApi->OnProcess(&tProcessor.tRoot);
 *  // ...
 *  tProcessor.tRoot.ptApi->OnExit(&tProcessor.tRoot);
 *  @endcode
 */
#ifndef PROCESSOR_APP_H
#define PROCESSOR_APP_H

#include "juno/app/app_api.h"
#include "juno/ds/array_api.h"
#include "juno/module.h"
#include "juno/sb/broker_api.h"
#include "juno/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* --------------------------------------------------------------------------
 * PROCESSOR_APP_T — concrete ProcessorApp struct
 * -------------------------------------------------------------------------- */

/**
 * @brief Concrete ProcessorApp instance.
 *
 * @details
 *  Embeds @c JUNO_APP_ROOT_T as its first member via @c JUNO_MODULE_DERIVE,
 *  enabling safe up-cast from a @c JUNO_APP_ROOT_T * to @c PROCESSOR_APP_T *
 *  inside the lifecycle callbacks. The scheduler holds a @c JUNO_APP_ROOT_T *
 *  and dispatches via the vtable; each callback recovers the full struct with:
 *  @code{.c}
 *  PROCESSOR_APP_T *ptProcessor = (PROCESSOR_APP_T *)ptApp;
 *  @endcode
 *  The embedded root is accessible as @c JUNO_MODULE_SUPER (aliased to @c tRoot).
 *
 *  The @c tPipe member is embedded — no separate allocation is needed. The
 *  composition root owns this struct; no field is heap-allocated. @c _ptPipeArray,
 *  @c _pfcnFailureHandler, and @c _pvFailureUserData are stored at init time and
 *  forwarded to @c JunoSb_PipeInit during @c OnStart.
 */
struct PROCESSOR_APP_TAG JUNO_MODULE_DERIVE(JUNO_APP_ROOT_T,
    /** @brief Injected Thread 2 broker. Lifetime must exceed ProcessorApp. */
    JUNO_SB_BROKER_ROOT_T    *ptBroker;
    /** @brief Private: injected backing array for the subscription pipe. Lifetime must exceed ProcessorApp. */
    JUNO_DS_ARRAY_ROOT_T     *_ptPipeArray;
    /** @brief Private: diagnostic failure callback; forwarded to JunoSb_PipeInit in OnStart. */
    JUNO_FAILURE_HANDLER_T    _pfcnFailureHandler;
    /** @brief Private: opaque user data pointer passed to the failure handler. */
    JUNO_USER_DATA_T         *_pvFailureUserData;
    /** @brief Embedded subscription pipe; initialized in OnStart, not at Init time. */
    JUNO_SB_PIPE_T            tPipe;
);
typedef struct PROCESSOR_APP_TAG PROCESSOR_APP_T;

/* --------------------------------------------------------------------------
 * ProcessorApp_Init — wire vtable and inject dependencies
 * -------------------------------------------------------------------------- */

/**
 * @brief Initialize a ProcessorApp instance.
 *
 * @details
 *  Wires the internal static vtable into @p ptApp->tRoot.ptApi, stores
 *  @p ptBroker, @p ptPipeArray, the failure handler, and user data. Verifies
 *  that no required injected pointer is NULL before returning. Pipe
 *  initialization is deferred to the @c OnStart lifecycle callback; this
 *  function does not call @c JunoSb_PipeInit or @c RegisterSubscriber.
 *
 *  Must be called before any lifecycle operation. All caller-allocated storage;
 *  this function does not allocate any memory.
 *
 * @param ptApp               Caller-owned ProcessorApp storage; must be non-NULL.
 * @param ptBroker            Thread 2 broker root; must be non-NULL and outlive
 *                            @p ptApp.
 * @param ptPipeArray         Backing array for the subscription pipe; must be
 *                            non-NULL and outlive @p ptApp.
 * @param pfcnFailureHandler  Diagnostic callback invoked before any error return;
 *                            may be NULL.
 * @param pvFailureUserData   Opaque pointer passed to the failure handler; may be NULL.
 * @return @c JUNO_STATUS_SUCCESS on success; @c JUNO_STATUS_NULLPTR_ERROR if any
 *         required pointer is NULL.
 */
// @{"req": ["REQ-UDPAPP-016"]}
JUNO_STATUS_T ProcessorApp_Init(
    PROCESSOR_APP_T              *ptApp,
    JUNO_SB_BROKER_ROOT_T        *ptBroker,
    JUNO_DS_ARRAY_ROOT_T         *ptPipeArray,
    JUNO_FAILURE_HANDLER_T        pfcnFailureHandler,
    void                         *pvFailureUserData
);

#ifdef __cplusplus
}
#endif

#endif /* PROCESSOR_APP_H */
