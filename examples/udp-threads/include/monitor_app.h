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
 * @file monitor_app.h
 * @brief MonitorApp — Thread 1 subscriber application for the udp-threads example.
 *
 * @details
 *  MonitorApp implements the @c JUNO_APP_API_T lifecycle interface and runs on
 *  Thread 1. On each scheduler cycle it dequeues all @c UDP_THREAD_MSG_T messages
 *  that SenderApp has published to Thread 1's software-bus broker and logs them.
 *
 *  Structural pattern:
 *  - @c MONITOR_APP_T embeds @c JUNO_APP_ROOT_T as its first member, enabling
 *    safe up-cast from @c JUNO_APP_ROOT_T* to @c MONITOR_APP_T*.
 *  - A @c JUNO_SB_PIPE_T is embedded directly in the struct (no heap allocation).
 *    The broker stores a pointer to this pipe after registration in @c OnStart.
 *  - A @c JUNO_DS_ARRAY_ROOT_T* injected at @c Init time provides the backing
 *    queue storage for the pipe; it must outlive the app instance.
 *  - All other dependencies are injected at @c Init time. No resource is
 *    allocated by MonitorApp itself.
 *
 *  Lifecycle:
 *  - @c OnStart  — initializes the pipe for @c UDPTH_MSG_MID and registers it
 *                  with the broker.
 *  - @c OnProcess — drains the pipe, logging each dequeued message.
 *  - @c OnExit   — no-op; the embedded pipe is released with the struct.
 */
#ifndef MONITOR_APP_H
#define MONITOR_APP_H

#include "juno/app/app_api.h"
#include "juno/ds/array_api.h"
#include "juno/sb/broker_api.h"
#include "juno/status.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* --------------------------------------------------------------------------
 * MONITOR_APP_T — concrete application struct
 * -------------------------------------------------------------------------- */

/**
 * @brief Concrete MonitorApp instance.
 *
 * @details
 *  All storage is caller-owned (stack or static). The composition root allocates
 *  this struct and injects all dependencies via @c MonitorApp_Init.
 *
 *  Member layout:
 *  - @c JUNO_MODULE_SUPER (tRoot) — Embedded via @c JUNO_MODULE_DERIVE; the scheduler
 *                          dispatches via a @c JUNO_APP_ROOT_T* and the lifecycle
 *                          functions recover the full struct by casting to @c MONITOR_APP_T*.
 *  - @c ptBroker         — Thread 1's broker; set at Init time; never NULL after Init.
 *  - @c _ptPipeArray     — Backing array for the pipe's internal queue; injected at
 *                          Init time; must outlive the pipe registration.
 *  - @c _pfcnFailureHandler — Diagnostic callback invoked before any error return;
 *                          may be NULL.
 *  - @c _pvFailureUserData  — Opaque user data passed to the failure handler; may be NULL.
 *  - @c tPipe            — Embedded subscription pipe; initialized in @c OnStart
 *                          and registered with @c ptBroker. No separate allocation.
 */
struct MONITOR_APP_TAG JUNO_MODULE_DERIVE(JUNO_APP_ROOT_T,
    /** @brief Injected: Thread 1's software-bus broker. */
    JUNO_SB_BROKER_ROOT_T    *ptBroker;
    /** @brief Injected: backing array for the pipe's queue storage. */
    JUNO_DS_ARRAY_ROOT_T     *_ptPipeArray;
    /** @brief Diagnostic failure callback; invoked before any error return. */
    JUNO_FAILURE_HANDLER_T    _pfcnFailureHandler;
    /** @brief Opaque user data threaded to the failure handler. */
    JUNO_USER_DATA_T         *_pvFailureUserData;
    /** @brief Embedded subscription pipe; no separate allocation required. */
    JUNO_SB_PIPE_T            tPipe;
);
typedef struct MONITOR_APP_TAG MONITOR_APP_T;

/* --------------------------------------------------------------------------
 * MonitorApp_Init
 * -------------------------------------------------------------------------- */

/**
 * @brief Initialize a MonitorApp instance.
 *
 * @details
 *  Wires the internal vtable, stores all injected dependencies, and verifies
 *  that no required pointer is NULL. The pipe (@c tPipe) is NOT initialized
 *  here; pipe initialization is deferred to @c OnStart so that the broker
 *  registration occurs at the correct point in the application lifecycle.
 *
 *  The caller must ensure:
 *  - @p ptApp, @p ptBroker, and @p ptPipeArray are all non-NULL.
 *  - @p ptBroker and @p ptPipeArray outlive this @c MONITOR_APP_T instance.
 *
 * @param ptApp               Caller-owned app storage; must be non-NULL.
 * @param ptBroker            Thread 1's broker instance; must be non-NULL.
 * @param ptPipeArray         Backing array for the pipe's internal queue; must be
 *                            non-NULL and must outlive the pipe registration.
 * @param pfcnFailureHandler  Diagnostic callback invoked before any error return;
 *                            may be NULL.
 * @param pvFailureUserData   Opaque pointer passed through to the failure handler;
 *                            may be NULL.
 * @return @c JUNO_STATUS_SUCCESS on success; @c JUNO_STATUS_NULLPTR_ERROR if any
 *         required pointer is NULL.
 */
JUNO_STATUS_T MonitorApp_Init(
    MONITOR_APP_T              *ptApp,
    JUNO_SB_BROKER_ROOT_T      *ptBroker,
    JUNO_DS_ARRAY_ROOT_T       *ptPipeArray,
    JUNO_FAILURE_HANDLER_T      pfcnFailureHandler,
    void                       *pvFailureUserData
);

#ifdef __cplusplus
}
#endif

#endif /* MONITOR_APP_H */
