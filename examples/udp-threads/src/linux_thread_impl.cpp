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
 * @file linux_thread_impl.cpp
 * @brief Linux/pthreads implementation of the Thread module vtable.
 *
 * @details
 *  Provides the @c g_junoThreadLinuxApi constant vtable that binds
 *  POSIX pthread operations to the LibJuno Thread module interface.
 *  pthreads headers are included only in this translation unit; the
 *  public @c thread.h header remains freestanding-clean.
 */

extern "C"
{
#include "juno/thread.h"
#include "juno/status.h"
#include "juno/macros.h"
}

#include <pthread.h>
#include <stdint.h>

/* -------------------------------------------------------------------------
 * Static vtable function implementations
 * ---------------------------------------------------------------------- */

/**
 * @brief Create and start an OS thread via pthreads.
 *
 * @details
 *  Guards against a double-create by checking @c _uHandle.  Clears @c bStop
 *  before spawning so the entry function always starts with a clean flag.
 *  The @c pthread_t handle is stored as a @c uintptr_t opaque value.
 *
 * @param ptRoot      Thread module root instance (caller-owned). Must not be NULL.
 * @param pfcnEntry   Thread entry function. Must not be NULL.
 * @param pvArg       Argument forwarded verbatim to @p pfcnEntry.
 * @return @c JUNO_STATUS_SUCCESS on success.
 * @return @c JUNO_STATUS_NULLPTR_ERROR if @p ptRoot or @p pfcnEntry is NULL.
 * @return @c JUNO_STATUS_REF_IN_USE_ERROR if a thread is already running.
 * @return @c JUNO_STATUS_ERR if @c pthread_create() fails.
 */
// @{"req": ["REQ-THREAD-003", "REQ-THREAD-004", "REQ-THREAD-010", "REQ-THREAD-015"]}
static JUNO_STATUS_T Create(
    JUNO_THREAD_ROOT_T *ptRoot,
    void *(*pfcnEntry)(void *),
    void *pvArg)
{
    JUNO_ASSERT_EXISTS(ptRoot && pfcnEntry);

    if (ptRoot->_uHandle != 0u)
    {
        JUNO_FAIL_ROOT(JUNO_STATUS_REF_IN_USE_ERROR, ptRoot, "thread already running");
        return JUNO_STATUS_REF_IN_USE_ERROR;
    }

    ptRoot->bStop = false;

    pthread_t tHandle;
    if (pthread_create(&tHandle, NULL, pfcnEntry, pvArg) != 0)
    {
        JUNO_FAIL_ROOT(JUNO_STATUS_ERR, ptRoot, "pthread_create() failed");
        return JUNO_STATUS_ERR;
    }

    ptRoot->_uHandle = (uintptr_t)tHandle;
    return JUNO_STATUS_SUCCESS;
}

/**
 * @brief Signal the managed thread to exit cooperatively.
 *
 * @details
 *  Sets @c ptRoot->bStop to @c true.  Does not cancel, signal, or interrupt
 *  the thread; the entry function must poll @c bStop and return voluntarily.
 *
 * @param ptRoot Thread module root instance (caller-owned). Must not be NULL.
 * @return @c JUNO_STATUS_SUCCESS on success.
 * @return @c JUNO_STATUS_NULLPTR_ERROR if @p ptRoot is NULL.
 */
// @{"req": ["REQ-THREAD-006", "REQ-THREAD-007"]}
static JUNO_STATUS_T Stop(JUNO_THREAD_ROOT_T *ptRoot)
{
    JUNO_ASSERT_EXISTS(ptRoot);
    ptRoot->bStop = true;
    return JUNO_STATUS_SUCCESS;
}

/**
 * @brief Block until the managed thread exits.
 *
 * @details
 *  Idempotent: returns @c JUNO_STATUS_SUCCESS immediately if @c _uHandle is
 *  already @c 0 (thread was never started or has already been joined).
 *  On success, resets @c _uHandle to @c 0 so the root may be reused.
 *
 * @param ptRoot Thread module root instance (caller-owned). Must not be NULL.
 * @return @c JUNO_STATUS_SUCCESS on success.
 * @return @c JUNO_STATUS_NULLPTR_ERROR if @p ptRoot is NULL.
 * @return @c JUNO_STATUS_ERR if @c pthread_join() fails.
 */
// @{"req": ["REQ-THREAD-005"]}
static JUNO_STATUS_T Join(JUNO_THREAD_ROOT_T *ptRoot)
{
    JUNO_ASSERT_EXISTS(ptRoot);

    if (ptRoot->_uHandle == 0u)
    {
        /* Already stopped or never started — idempotent success */
        return JUNO_STATUS_SUCCESS;
    }

    pthread_t tHandle = (pthread_t)ptRoot->_uHandle;
    if (pthread_join(tHandle, NULL) != 0)
    {
        JUNO_FAIL_ROOT(JUNO_STATUS_ERR, ptRoot, "pthread_join() failed");
        return JUNO_STATUS_ERR;
    }

    ptRoot->_uHandle = 0u;
    return JUNO_STATUS_SUCCESS;
}

/* -------------------------------------------------------------------------
 * Platform vtable definition
 * ---------------------------------------------------------------------- */

/**
 * @brief Linux/pthreads vtable for the Thread module.
 *
 * @details
 *  Pass @c &g_junoThreadLinuxApi to @c JunoThread_Init on Linux/POSIX targets.
 */
// @{"req": ["REQ-THREAD-011"]}
const JUNO_THREAD_API_T g_junoThreadLinuxApi = {
    Create,
    Stop,
    Join
};
