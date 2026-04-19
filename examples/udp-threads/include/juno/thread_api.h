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
 * @file thread_api.h
 * @brief Freestanding C11 interface for the LibJuno Thread module.
 *
 * @details
 *  Provides a vtable-based abstraction for managing a single OS thread.
 *  This header is freestanding-compatible: it does not include any POSIX
 *  or OS-specific headers (@c pthread.h, @c unistd.h, etc.) and may be
 *  compiled with @c -nostdlib @c -ffreestanding.
 *
 *  The module supports a cooperative shutdown model.  The thread entry
 *  function periodically reads @c ptRoot->bStop; when it is @c true the
 *  entry function exits its loop and returns.  The caller then calls
 *  @c Join to reap the OS thread.
 *
 *  Platform-specific details (OS handle, thread creation) are confined to
 *  @c thread_linux.h and the corresponding implementation translation unit.
 *  The union @c JUNO_THREAD_T provides storage large enough for any
 *  derivation; callers allocate it and pass @c &tThread.tRoot to all API
 *  functions.
 *
 *  Typical usage:
 *  @code{.c}
 *  #include "juno/thread_linux.h"   // platform header provides JunoThread_LinuxInit
 *
 *  static JUNO_THREAD_T tThread = {0};
 *
 *  // Platform init spawns the thread immediately (RAII)
 *  JunoThread_LinuxInit(&tThread, MyEntryFunction, &tThread.tRoot,
 *                       FailureHandler, NULL);
 *
 *  // Cooperative shutdown
 *  tThread.tRoot.ptApi->Stop(&tThread.tRoot);
 *  tThread.tRoot.ptApi->Join(&tThread.tRoot);
 *  tThread.tRoot.ptApi->Free(&tThread.tRoot);
 *  @endcode
 */

#ifndef JUNO_THREAD_API_H
#define JUNO_THREAD_API_H

#include <stdint.h>
#include <stdbool.h>
#include "juno/status.h"
#include "juno/module.h"
#include "juno/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* -------------------------------------------------------------------------
 * Forward declarations
 * ---------------------------------------------------------------------- */

/** @brief Forward declaration of the Thread module root struct. */
typedef struct JUNO_THREAD_ROOT_TAG JUNO_THREAD_ROOT_T;

/** @brief Forward declaration of the Thread vtable struct. */
typedef struct JUNO_THREAD_API_TAG  JUNO_THREAD_API_T;

/** @brief Forward declaration of the Linux/POSIX derivation struct. */
typedef struct JUNO_THREAD_LINUX_TAG JUNO_THREAD_LINUX_T;

/** @brief Forward declaration of the Thread module union.
 *  The full union body is defined in @c juno/thread_linux.h after the
 *  platform derivation type is complete. */
typedef union  JUNO_THREAD_TAG      JUNO_THREAD_T;

/* -------------------------------------------------------------------------
 * Module root
 * ---------------------------------------------------------------------- */

/**
 * @brief Thread module root struct (freestanding, cross-platform).
 *
 * @details
 *  The root struct is the shared, freestanding portion of the Thread module.
 *  It carries the vtable pointer, the optional failure handler and its user
 *  data (all injected by @c JUNO_MODULE_ROOT), and the single cross-platform
 *  state field needed by thread entry functions.
 *
 *  The OS thread handle (@c pthread_t or equivalent) is NOT stored here.
 *  It lives in the platform derivation (e.g., @c JUNO_THREAD_LINUX_T) so
 *  that this header remains compilable without any POSIX headers.
 *
 *  The @c bStop field has no leading underscore because it is part of the
 *  public cooperative-shutdown protocol: the thread entry function reads
 *  @c ptRoot->bStop directly to decide when to exit its loop.
 */
// @{"req": ["REQ-THREAD-001", "REQ-THREAD-007", "REQ-THREAD-008", "REQ-THREAD-009"]}
struct JUNO_THREAD_ROOT_TAG JUNO_MODULE_ROOT(JUNO_THREAD_API_T,
    /** @brief Cooperative shutdown flag. Set to @c true by @c Stop to signal
     *         the thread entry function to exit its scheduling loop. */
    volatile bool bStop;
);

/* -------------------------------------------------------------------------
 * Vtable
 * ---------------------------------------------------------------------- */

/**
 * @brief Thread module vtable (API struct).
 *
 * @details
 *  Each function pointer receives the module root as its first argument,
 *  following the LibJuno vtable dispatch convention.  Concrete platform
 *  implementations populate this struct and expose it as a @c const global
 *  (e.g., @c g_junoThreadLinuxApi).
 *
 *  Thread creation is intentionally absent from this vtable.  The platform
 *  init function (@c JunoThread_LinuxInit) is responsible for spawning the
 *  OS thread as part of RAII initialisation; no separate @c Create step is
 *  needed at the generic interface level.
 */
// @{"req": ["REQ-THREAD-002", "REQ-THREAD-014"]}
struct JUNO_THREAD_API_TAG
{
    /**
     * @brief Signal the managed thread to exit cooperatively.
     *
     * @details
     *  Sets @c ptRoot->bStop = true.  Does not cancel or forcibly terminate
     *  the OS thread; the thread entry function is responsible for observing
     *  the flag and returning voluntarily.
     *
     * @param ptRoot  Thread module root instance (caller-owned). Must not be NULL.
     * @return @c JUNO_STATUS_SUCCESS on success; non-zero on failure.
     */
    JUNO_STATUS_T (*Stop)(JUNO_THREAD_ROOT_T *ptRoot);

    /**
     * @brief Block until the managed thread exits.
     *
     * @details
     *  Calls the platform join primitive (e.g., @c pthread_join) and blocks
     *  until the thread entry function returns.  Must only be called after
     *  @c Stop (or after the thread has independently set @c bStop and exited).
     *
     * @param ptRoot  Thread module root instance (caller-owned). Must not be NULL.
     * @return @c JUNO_STATUS_SUCCESS on success; non-zero on failure.
     */
    JUNO_STATUS_T (*Join)(JUNO_THREAD_ROOT_T *ptRoot);

    /**
     * @brief Release platform resources held by the Thread module instance.
     *
     * @details
     *  Resets the OS handle and any platform-specific state so the instance
     *  may be reinitialised.  Must be called only after @c Join has returned.
     *
     * @param ptRoot  Thread module root instance (caller-owned). Must not be NULL.
     * @return @c JUNO_STATUS_SUCCESS on success; non-zero on failure.
     */
    JUNO_STATUS_T (*Free)(JUNO_THREAD_ROOT_T *ptRoot);
};

/* -------------------------------------------------------------------------
 * Generic initialisation
 * ---------------------------------------------------------------------- */

/**
 * @brief Initialise a Thread module root with a concrete vtable and failure handler.
 *
 * @details
 *  Wires @p ptApi into @p ptRoot->ptApi, stores the failure handler and its
 *  user data, and clears the cooperative-shutdown flag (@c bStop = false).
 *  Must be called before any vtable dispatch.
 *
 *  Callers that use the Linux/POSIX implementation should call
 *  @c JunoThread_LinuxInit instead; it calls this function internally and
 *  also spawns the OS thread.
 *
 * @param ptRoot              Caller-owned root storage. Must not be NULL.
 * @param ptApi               Vtable (e.g., @c &g_junoThreadLinuxApi). Must not be NULL.
 * @param pfcnFailureHandler  Diagnostic callback invoked before any error return; may be NULL.
 * @param pvFailureUserData   Opaque pointer threaded to @p pfcnFailureHandler; may be NULL.
 * @return @c JUNO_STATUS_SUCCESS on success; @c JUNO_STATUS_NULLPTR_ERROR if
 *         @p ptRoot or @p ptApi is NULL.
 */
JUNO_STATUS_T JunoThread_Init(
    JUNO_THREAD_ROOT_T          *ptRoot,
    const JUNO_THREAD_API_T     *ptApi,
    JUNO_FAILURE_HANDLER_T       pfcnFailureHandler,
    void                        *pvFailureUserData
);

#ifdef __cplusplus
}
#endif

#endif /* JUNO_THREAD_API_H */
