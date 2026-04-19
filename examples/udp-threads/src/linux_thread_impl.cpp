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
 *  Provides the static @c s_tJunoThreadLinuxApi vtable and the RAII entry
 *  point @c JunoThread_LinuxInit.  Thread creation is handled entirely inside
 *  @c JunoThread_LinuxInit (RAII pattern); no separate @c Create vtable slot
 *  exists.  The @c pthread_t handle is stored in the @c JUNO_THREAD_LINUX_T
 *  derivation, not in the freestanding root, so @c thread_api.h remains
 *  compilable without POSIX headers.
 *
 *  pthreads headers are included only in this translation unit via
 *  @c juno/thread_linux.h; all other translation units that need only the
 *  generic interface include @c juno/thread_api.h.
 */

extern "C"
{
#include "juno/thread_linux.h"
#include "juno/status.h"
#include "juno/macros.h"
#include <string.h>
}

/* -------------------------------------------------------------------------
 * Static vtable function implementations
 * ---------------------------------------------------------------------- */

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
 *  Calls @c pthread_join on the handle stored in the Linux derivation and
 *  blocks until the thread entry function returns.  On success, zeroes the
 *  @c _tHandle field so the instance may be safely reused or freed.
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
    JUNO_THREAD_LINUX_T *ptLinux = (JUNO_THREAD_LINUX_T *)(ptRoot);
    if (pthread_join(ptLinux->_tHandle, NULL) != 0)
    {
        JUNO_FAIL_ROOT(JUNO_STATUS_ERR, ptRoot, "pthread_join() failed");
        return JUNO_STATUS_ERR;
    }
    memset(&ptLinux->_tHandle, 0, sizeof(pthread_t));
    return JUNO_STATUS_SUCCESS;
}

/**
 * @brief Release platform resources held by the Thread module instance.
 *
 * @details
 *  Zeroes the @c _tHandle field in the Linux derivation, resetting it to a
 *  known-clean state.  Must be called only after @c Join has returned
 *  successfully.
 *
 * @param ptRoot Thread module root instance (caller-owned). Must not be NULL.
 * @return @c JUNO_STATUS_SUCCESS on success.
 * @return @c JUNO_STATUS_NULLPTR_ERROR if @p ptRoot is NULL.
 */
// @{"req": ["REQ-THREAD-008", "REQ-THREAD-009"]}
static JUNO_STATUS_T Free(JUNO_THREAD_ROOT_T *ptRoot)
{
    JUNO_ASSERT_EXISTS(ptRoot);
    JUNO_THREAD_LINUX_T *ptLinux = (JUNO_THREAD_LINUX_T *)(ptRoot);
    memset(&ptLinux->_tHandle, 0, sizeof(pthread_t));
    return JUNO_STATUS_SUCCESS;
}

/* -------------------------------------------------------------------------
 * Platform vtable definition
 * ---------------------------------------------------------------------- */
// @{"req": ["REQ-THREAD-002", "REQ-THREAD-014"]}
static const JUNO_THREAD_API_T s_tJunoThreadLinuxApi = {
    Stop,
    Join,
    Free
};

/* -------------------------------------------------------------------------
 * Platform RAII initialisation
 * ---------------------------------------------------------------------- */

/**
 * @brief Initialise a Thread module instance and immediately spawn the OS thread.
 *
 * @details
 *  1. Guards @p ptThread and @p pfcnEntry (returns
 *     @c JUNO_STATUS_NULLPTR_ERROR if either is NULL).
 *  2. Calls @c JunoThread_Init to wire @c s_tJunoThreadLinuxApi, clear
 *     @c bStop, and store the failure handler.
 *  3. Zeroes @c ptThread->tLinux._tHandle.
 *  4. Calls @c pthread_create; on failure returns @c JUNO_STATUS_ERR.
 *  5. Stores the resulting @c pthread_t in @c ptThread->tLinux._tHandle.
 *
 * @param ptThread            Caller-owned @c JUNO_THREAD_T union. Must not be NULL.
 * @param pfcnEntry           Thread entry function. Must not be NULL.
 * @param pvArg               Argument forwarded verbatim to @p pfcnEntry; may be NULL.
 * @param pfcnFailureHandler  Diagnostic callback invoked before any error return; may be NULL.
 * @param pvFailureUserData   Opaque pointer threaded to @p pfcnFailureHandler; may be NULL.
 * @return @c JUNO_STATUS_SUCCESS on success.
 * @return @c JUNO_STATUS_NULLPTR_ERROR if a required pointer is NULL.
 * @return @c JUNO_STATUS_ERR if @c pthread_create() fails.
 */
// @{"req": ["REQ-THREAD-003", "REQ-THREAD-004", "REQ-THREAD-010", "REQ-THREAD-011", "REQ-THREAD-012", "REQ-THREAD-013", "REQ-THREAD-015"]}
extern "C" JUNO_STATUS_T JunoThread_LinuxInit(
    JUNO_THREAD_T          *ptThread,
    void *(*pfcnEntry)(void *),
    void                   *pvArg,
    JUNO_FAILURE_HANDLER_T  pfcnFailureHandler,
    void                   *pvFailureUserData
)
{
    JUNO_ASSERT_EXISTS(ptThread);
    JUNO_ASSERT_EXISTS(pfcnEntry);
    JUNO_THREAD_ROOT_T *ptRoot = &ptThread->tRoot;
    JUNO_STATUS_T tStatus = JunoThread_Init(
        ptRoot, &s_tJunoThreadLinuxApi,
        pfcnFailureHandler, pvFailureUserData
    );
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    memset(&ptThread->tLinux._tHandle, 0, sizeof(pthread_t));
    if (pthread_create(&ptThread->tLinux._tHandle, NULL, pfcnEntry, pvArg) != 0)
    {
        JUNO_FAIL_ROOT(JUNO_STATUS_ERR, ptRoot, "pthread_create() failed");
        return JUNO_STATUS_ERR;
    }
    return JUNO_STATUS_SUCCESS;
}
