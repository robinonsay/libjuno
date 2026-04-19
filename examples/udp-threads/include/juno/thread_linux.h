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
 * @file thread_linux.h
 * @brief Linux/POSIX-specific concrete derivation of the LibJuno Thread module.
 *
 * @details
 *  This header provides the Linux/pthreads concrete derivation of the Thread
 *  module defined by @c juno/thread_api.h.  It is the ONLY Thread-module
 *  header that may include @c pthread.h; all other translation units that
 *  require only the abstract interface should include @c juno/thread_api.h.
 *
 *  The derivation struct @c JUNO_THREAD_LINUX_T extends the freestanding root
 *  (@c JUNO_THREAD_ROOT_T) by adding the native OS thread handle (@c pthread_t).
 *  Because @c pthread_t cannot be represented as a freestanding type it is
 *  confined here, away from the generic interface.
 *
 *  @c JunoThread_LinuxInit follows a RAII model: it wires the vtable, stores
 *  the failure handler, and immediately spawns the OS thread via
 *  @c pthread_create.  No separate @c Create step is required.
 *
 *  Typical usage:
 *  @code{.c}
 *  #include "juno/thread_linux.h"
 *
 *  static JUNO_THREAD_T tThread = {0};
 *
 *  // Initialise and spawn the thread in one call
 *  JunoThread_LinuxInit(&tThread, WorkerEntry, &tThread.tRoot,
 *                       FailureHandler, NULL);
 *
 *  // Cooperative shutdown sequence
 *  tThread.tRoot.ptApi->Stop(&tThread.tRoot);
 *  tThread.tRoot.ptApi->Join(&tThread.tRoot);
 *  tThread.tRoot.ptApi->Free(&tThread.tRoot);
 *  @endcode
 */

#ifndef JUNO_THREAD_LINUX_H
#define JUNO_THREAD_LINUX_H

#include "juno/thread_api.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* -------------------------------------------------------------------------
 * Linux/POSIX derivation
 * ---------------------------------------------------------------------- */

/**
 * @brief Linux/POSIX concrete derivation of the Thread module.
 *
 * @details
 *  Embeds @c JUNO_THREAD_ROOT_T as its first member (@c tRoot, aliased by
 *  @c JUNO_MODULE_SUPER), enabling a safe pointer up-cast from any vtable
 *  callback that receives a @c JUNO_THREAD_ROOT_T * parameter.
 *
 *  The @c _tHandle field stores the native @c pthread_t returned by
 *  @c pthread_create.  It is a private implementation detail; callers must
 *  not read or write it directly.
 */
// @{"req": ["REQ-THREAD-010", "REQ-THREAD-015"]}
struct JUNO_THREAD_LINUX_TAG JUNO_MODULE_DERIVE(JUNO_THREAD_ROOT_T,
    /** @brief Native POSIX thread handle; valid after a successful
     *         @c JunoThread_LinuxInit call and until @c Free returns. */
    pthread_t _tHandle;
);
typedef struct JUNO_THREAD_LINUX_TAG JUNO_THREAD_LINUX_T;

/* -------------------------------------------------------------------------
 * Module union
 * ---------------------------------------------------------------------- */

/**
 * @brief Type-safe polymorphic handle for a Thread module instance.
 *
 * @details
 *  Callers allocate this union (stack or static) and pass @c &tThread.tRoot
 *  to @c JunoThread_Init and to all subsequent vtable dispatches.  The union
 *  is defined here — in the platform header — because it requires the complete
 *  type @c JUNO_THREAD_LINUX_T, which in turn requires @c pthread.h.
 *
 *  The union body satisfies the forward declaration of @c JUNO_THREAD_T made
 *  in @c juno/thread_api.h.
 *
 *  @code{.c}
 *  static JUNO_THREAD_T tThread = {0};
 *  JunoThread_LinuxInit(&tThread, WorkerEntry, &tThread.tRoot,
 *                       FailureHandler, NULL);
 *  tThread.tRoot.ptApi->Stop(&tThread.tRoot);
 *  tThread.tRoot.ptApi->Join(&tThread.tRoot);
 *  tThread.tRoot.ptApi->Free(&tThread.tRoot);
 *  @endcode
 */
union JUNO_THREAD_TAG JUNO_MODULE(JUNO_THREAD_API_T, JUNO_THREAD_ROOT_T,
    /** @brief Linux/POSIX derivation view. */
    JUNO_THREAD_LINUX_T tLinux;
);

/* -------------------------------------------------------------------------
 * Platform initialisation (RAII)
 * ---------------------------------------------------------------------- */

/**
 * @brief Initialise a Thread module instance and immediately spawn the OS thread.
 *
 * @details
 *  This function combines generic module initialisation with platform thread
 *  creation in a single RAII call:
 *
 *  1. Guards @p ptThread (returns @c JUNO_STATUS_NULLPTR_ERROR if NULL).
 *  2. Guards @p pfcnEntry (returns @c JUNO_STATUS_NULLPTR_ERROR if NULL).
 *  3. Calls @c JunoThread_Init internally, wiring @c g_junoThreadLinuxApi as
 *     the vtable — callers do NOT pass a @c ptApi parameter.
 *  4. Clears @c bStop to @c false.
 *  5. Calls @c pthread_create with @p pfcnEntry and @p pvArg; on failure
 *     returns @c JUNO_STATUS_ERR.
 *  6. Stores the resulting @c pthread_t in @c ptThread->tLinux._tHandle.
 *
 * @param ptThread            Caller-owned @c JUNO_THREAD_T union. Must not be NULL.
 * @param pfcnEntry           Thread entry function (POSIX signature). Must not be NULL.
 *                            Typically reads @c bStop from its argument to detect
 *                            cooperative shutdown.
 * @param pvArg               Argument forwarded verbatim to @p pfcnEntry; may be NULL.
 *                            Callers typically pass @c &ptThread->tRoot so the entry
 *                            function can read @c bStop.
 * @param pfcnFailureHandler  Diagnostic callback invoked before any error return; may be NULL.
 * @param pvFailureUserData   Opaque pointer threaded to @p pfcnFailureHandler; may be NULL.
 * @return @c JUNO_STATUS_SUCCESS on success; @c JUNO_STATUS_NULLPTR_ERROR if a
 *         required pointer is NULL; @c JUNO_STATUS_ERR if @c pthread_create fails.
 */
// @{"req": ["REQ-THREAD-003", "REQ-THREAD-004", "REQ-THREAD-011", "REQ-THREAD-012", "REQ-THREAD-013"]}
JUNO_STATUS_T JunoThread_LinuxInit(
    JUNO_THREAD_T          *ptThread,
    void *(*pfcnEntry)(void *),
    void                   *pvArg,
    JUNO_FAILURE_HANDLER_T  pfcnFailureHandler,
    void                   *pvFailureUserData
);

#ifdef __cplusplus
}
#endif

#endif /* JUNO_THREAD_LINUX_H */
