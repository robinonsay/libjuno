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
 * @file thread.h
 * @brief Freestanding C11 interface for the LibJuno Thread module.
 *
 * @details
 *  Provides a vtable-based abstraction for creating, stopping, and joining a
 *  single OS thread. All OS-specific types (e.g., @c pthread_t) are hidden
 *  behind an opaque @c uintptr_t handle so that this header remains compatible
 *  with C11 freestanding environments and does not expose any POSIX or
 *  platform-specific symbols.
 *
 *  The module supports a cooperative shutdown model: the caller invokes
 *  @c Stop to set the @c bStop flag in the module root, the thread entry
 *  function reads the flag and exits its loop voluntarily, and the caller
 *  then invokes @c Join to wait for the thread to finish.
 *
 *  Typical usage:
 *  @code{.c}
 *  static JUNO_THREAD_ROOT_T tThread;
 *
 *  // Initialize with the Linux vtable
 *  JunoThread_Init(&tThread, &g_junoThreadLinuxApi, NULL, NULL);
 *
 *  // Start the thread
 *  JunoThread_Create(&tThread, MyEntryFunction, &tThread);
 *
 *  // Signal cooperative shutdown and wait
 *  JunoThread_Stop(&tThread);
 *  JunoThread_Join(&tThread);
 *  @endcode
 */

#ifndef JUNO_THREAD_H
#define JUNO_THREAD_H

#include <stdint.h>
#include <stddef.h>
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
typedef struct JUNO_THREAD_API_TAG JUNO_THREAD_API_T;

/* -------------------------------------------------------------------------
 * Module root
 * ---------------------------------------------------------------------- */

/**
 * @brief Thread module root struct.
 *
 * @details
 *  The root struct is the shared, freestanding portion of the Thread module.
 *  It carries the vtable pointer, the optional failure handler, and all
 *  per-instance state fields required by the module.
 *
 *  Fields with a leading underscore are implementation details; callers must
 *  not read or write them directly.  The @c bStop field has no leading
 *  underscore because it is part of the public interface: the caller-supplied
 *  thread entry function reads @c ptRoot->bStop directly to decide when to
 *  exit its scheduling loop.
 *
 *  @note The @c _uHandle field stores the OS thread handle (e.g., @c pthread_t)
 *        cast to @c uintptr_t. This keeps all OS types out of the public header.
 *        A value of @c 0 means no thread is currently running.
 */
// @{"req": ["REQ-THREAD-001", "REQ-THREAD-007", "REQ-THREAD-008", "REQ-THREAD-009"]}
struct JUNO_THREAD_ROOT_TAG JUNO_MODULE_ROOT(JUNO_THREAD_API_T,
    /** @brief Opaque OS thread handle; @c 0 = not running. Private. */
    uintptr_t _uHandle;
    /** @brief Cooperative shutdown flag. @c true signals the thread to exit. */
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
 *  following the LibJuno vtable dispatch convention. Concrete implementations
 *  (e.g., the Linux/pthreads implementation) populate this struct and expose
 *  it as a @c const global.
 */
// @{"req": ["REQ-THREAD-002", "REQ-THREAD-014"]}
struct JUNO_THREAD_API_TAG
{
    /**
     * @brief Create and start an OS thread.
     *
     * @param ptRoot      Thread module root instance (caller-owned).
     * @param pfcnEntry   Thread entry function with POSIX signature
     *                    @c void*(*)(void*).  Must not be NULL.
     * @param pvArg       Argument forwarded to @p pfcnEntry. Callers
     *                    typically pass @p ptRoot so the entry function
     *                    can read @c bStop.
     * @return @c JUNO_STATUS_SUCCESS on success, or a non-zero status code
     *         on failure.
     */
    JUNO_STATUS_T (*Create)(JUNO_THREAD_ROOT_T *ptRoot,
                            void *(*pfcnEntry)(void *),
                            void *pvArg);

    /**
     * @brief Signal the managed thread to exit cooperatively.
     *
     * @details Sets @c ptRoot->bStop = true. Does not cancel or signal the
     *          thread; the thread entry function is responsible for observing
     *          the flag and returning.
     *
     * @param ptRoot Thread module root instance (caller-owned).
     * @return @c JUNO_STATUS_SUCCESS on success, or a non-zero status code
     *         on failure.
     */
    JUNO_STATUS_T (*Stop)(JUNO_THREAD_ROOT_T *ptRoot);

    /**
     * @brief Block until the managed thread exits.
     *
     * @details After a successful return, @c _uHandle is reset to @c 0 and
     *          the root may be reused for another @c Create call.
     *
     * @param ptRoot Thread module root instance (caller-owned).
     * @return @c JUNO_STATUS_SUCCESS on success, or a non-zero status code
     *         on failure.
     */
    JUNO_STATUS_T (*Join)(JUNO_THREAD_ROOT_T *ptRoot);
};

/* -------------------------------------------------------------------------
 * Initialization
 * ---------------------------------------------------------------------- */

/**
 * @brief Initialize a Thread module root instance.
 *
 * @details
 *  Wires the vtable pointer, stores the optional failure handler, zeroes the
 *  thread handle sentinel, and clears the stop flag. Must be called before
 *  any vtable dispatch function.
 *
 * @param ptRoot               Caller-owned root storage. Must not be NULL.
 * @param ptApi                Vtable to inject (e.g., @c &g_junoThreadLinuxApi
 *                             or a test double). Must not be NULL.
 * @param pfcnFailureHandler   Optional diagnostic callback; may be NULL.
 * @param pvFailureUserData    Opaque user data passed to @p pfcnFailureHandler;
 *                             may be NULL.
 * @return @c JUNO_STATUS_SUCCESS on success, @c JUNO_STATUS_NULLPTR_ERROR if
 *         @p ptRoot or @p ptApi is NULL.
 */
JUNO_STATUS_T JunoThread_Init(
    JUNO_THREAD_ROOT_T          *ptRoot,
    const JUNO_THREAD_API_T     *ptApi,
    JUNO_FAILURE_HANDLER_T       pfcnFailureHandler,
    void                        *pvFailureUserData
);

/* -------------------------------------------------------------------------
 * Platform vtable declarations
 * ---------------------------------------------------------------------- */

/**
 * @brief Linux/pthreads vtable for the Thread module.
 *
 * @details Populated by the Linux implementation translation unit
 *          (@c juno_thread_linux.cpp). Pass @c &g_junoThreadLinuxApi to
 *          @c JunoThread_Init on Linux/POSIX targets.
 */
extern const JUNO_THREAD_API_T g_junoThreadLinuxApi;

#ifdef __cplusplus
}
#endif

#endif /* JUNO_THREAD_H */
