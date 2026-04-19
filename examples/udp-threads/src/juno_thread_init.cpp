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

#include "juno/thread.h"
#include "juno/macros.h"

/**
 * @brief Initialize a Thread module root instance.
 *
 * @details
 *  Wires the vtable pointer into the root, zeroes the opaque OS thread handle
 *  sentinel, clears the cooperative stop flag, and stores the optional failure
 *  handler and its associated user-data pointer. This function must be called
 *  before any vtable dispatch function (Create, Stop, Join).
 *
 * @param ptRoot               Caller-owned root storage. Must not be NULL.
 *                             or a test double). Must not be NULL.
 * @param pfcnFailureHandler   Optional diagnostic callback; may be NULL.
 * @param pvFailureUserData    Opaque user data passed to @p pfcnFailureHandler;
 *                             may be NULL.
 * @return @c JUNO_STATUS_SUCCESS on success, @c JUNO_STATUS_NULLPTR_ERROR if
 *         @p ptRoot or @p ptApi is NULL.
 */
// @{"req": ["REQ-THREAD-012", "REQ-THREAD-013"]}
extern "C" JUNO_STATUS_T JunoThread_Init(
    JUNO_THREAD_ROOT_T          *ptRoot,
    const JUNO_THREAD_API_T     *ptApi,
    JUNO_FAILURE_HANDLER_T       pfcnFailureHandler,
    void                        *pvFailureUserData
)
{
    JUNO_ASSERT_EXISTS(ptRoot);
    JUNO_ASSERT_EXISTS(ptApi);

    ptRoot->ptApi                  = ptApi;
    ptRoot->_uHandle               = 0;
    ptRoot->bStop                  = false;
    ptRoot->_pfcnFailureHandler    = pfcnFailureHandler;
    ptRoot->_pvFailureUserData     = pvFailureUserData;

    return JUNO_STATUS_SUCCESS;
}
