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
 * @file juno_udp_init.cpp
 * @brief Implementation of JunoUdp_Init for the udp-threads example.
 *
 * @details
 *  This translation unit implements the module initialisation function for the
 *  UDP socket module. It is intentionally free of POSIX headers; all
 *  platform-specific code lives in linux_udp_impl.cpp.
 *
 *  Including udp_api.h (which wraps its declarations in @c extern "C") gives
 *  @c JunoUdp_Init C linkage automatically when compiled as C++.
 */

#include "udp_api.h"
#include "juno/macros.h"

/**
 * @brief Initialise a UDP module root with a concrete vtable and failure handler.
 *
 * @details
 *  Wires @p ptApi into @p ptRoot->ptApi and stores the optional failure handler
 *  and user data. Must be called before any vtable operation on the root.
 *  Platform-specific socket state (fd, address) is owned by the derivation
 *  (@c JUNO_UDP_LINUX_T) and initialised in @c JunoUdp_LinuxInit.
 *
 * @param ptRoot             Caller-owned root storage; must be non-NULL.
 * @param ptApi              Vtable (Linux implementation or test double); must be non-NULL.
 * @param pfcnFailureHandler Diagnostic callback invoked before any error return; may be NULL.
 * @param pvFailureUserData  Opaque user data pointer threaded to the failure handler; may be NULL.
 * @return @c JUNO_STATUS_SUCCESS on success; @c JUNO_STATUS_NULLPTR_ERROR if
 *         @p ptRoot or @p ptApi is NULL.
 */
// @{"req": ["REQ-UDP-016", "REQ-UDP-013"]}
JUNO_STATUS_T JunoUdp_Init(
    JUNO_UDP_ROOT_T        *ptRoot,
    const JUNO_UDP_API_T   *ptApi,
    JUNO_FAILURE_HANDLER_T  pfcnFailureHandler,
    void                   *pvFailureUserData
)
{
    JUNO_ASSERT_EXISTS(ptRoot);
    JUNO_ASSERT_EXISTS(ptApi);

    ptRoot->ptApi                       = ptApi;
    ptRoot->JUNO_FAILURE_HANDLER        = pfcnFailureHandler;
    ptRoot->JUNO_FAILURE_USER_DATA      = (JUNO_USER_DATA_T *)pvFailureUserData;

    return JUNO_STATUS_SUCCESS;
}
