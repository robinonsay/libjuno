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
 * @file udp_linux.h
 * @brief Linux/POSIX-specific derivation and initialiser for the UDP module.
 *
 * @details
 *  This header is the platform layer for the UDP socket module. It extends
 *  the freestanding interface (@c udp_api.h) with Linux/POSIX-specific state
 *  (socket file descriptor, peer/bind address) and provides a RAII initialiser
 *  that wires the vtable and opens the socket in a single call.
 *
 *  **Do not include this header in freestanding translation units.** It pulls
 *  in @c <sys/socket.h> and @c <netinet/in.h> which are POSIX-only.
 *
 *  Typical usage:
 *  @code{.c}
 *  #include "udp_linux.h"
 *
 *  static JUNO_UDP_T tUdp = {0};
 *
 *  JUNO_UDP_CFG_T tCfg = { "127.0.0.1", 5000, false };
 *  JUNO_STATUS_T tStatus = JunoUdp_LinuxInit(&tUdp, &tCfg, NULL, NULL);
 *  if (tStatus != JUNO_STATUS_SUCCESS) { // handle error }
 *
 *  UDP_THREAD_MSG_T tMsg = {0};
 *  tUdp.tRoot.ptApi->Send(&tUdp.tRoot, &tMsg);
 *  tUdp.tRoot.ptApi->Free(&tUdp.tRoot);   // closes socket
 *  @endcode
 */
#ifndef JUNO_UDP_LINUX_H
#define JUNO_UDP_LINUX_H

#include "udp_api.h"
#include <sys/socket.h>
#include <netinet/in.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* --------------------------------------------------------------------------
 * JUNO_UDP_LINUX_T — Linux/POSIX derivation
 * -------------------------------------------------------------------------- */

/**
 * @brief Linux POSIX derivation of the UDP module.
 *
 * @details
 *  Embeds @c JUNO_UDP_ROOT_T as its first member (@c tRoot, via
 *  @c JUNO_MODULE_DERIVE), enabling safe up-cast to the root for vtable
 *  dispatch. Owns the POSIX socket state that cannot be expressed in
 *  freestanding-compatible types and therefore cannot reside in the root.
 *
 *  Members:
 *  - @c _iSockFd  — POSIX socket file descriptor; @c -1 when closed/invalid.
 *  - @c _tAddr    — Peer address (sender) or bind address (receiver),
 *                   populated by @c JunoUdp_LinuxInit.
 *
 *  Callers allocate a @c JUNO_UDP_T union and pass it to @c JunoUdp_LinuxInit;
 *  they need not interact with this type directly.
 */
// @{"req": ["REQ-UDP-001", "REQ-UDP-014"]}
struct JUNO_UDP_LINUX_TAG JUNO_MODULE_DERIVE(JUNO_UDP_ROOT_T,
    /** @brief POSIX socket file descriptor; -1 when the socket is closed/invalid. */
    int                _iSockFd;
    /** @brief Peer or bind address, depending on socket role. */
    struct sockaddr_in _tAddr;
);
typedef struct JUNO_UDP_LINUX_TAG JUNO_UDP_LINUX_T;

/* --------------------------------------------------------------------------
 * JUNO_UDP_T — module union (complete definition)
 * -------------------------------------------------------------------------- */

/**
 * @brief Type-safe polymorphic handle for a UDP module instance.
 *
 * @details
 *  Defined here (rather than in @c udp_api.h) because the union body requires
 *  @c JUNO_UDP_LINUX_T to be complete, and that type includes POSIX fields
 *  (@c struct @c sockaddr_in) that are not freestanding-compatible.
 *
 *  Callers allocate this union (stack or static) and pass it to
 *  @c JunoUdp_LinuxInit, which wires the vtable and opens the socket. All
 *  subsequent API calls use @c &tUdp.tRoot.
 */
union JUNO_UDP_TAG JUNO_MODULE(JUNO_UDP_API_T, JUNO_UDP_ROOT_T,
    /** @brief Linux POSIX derivation view. */
    JUNO_UDP_LINUX_T tLinux;
);

/* --------------------------------------------------------------------------
 * Platform init function
 * -------------------------------------------------------------------------- */

/**
 * @brief Initialise a Linux UDP module instance and open the socket (RAII).
 *
 * @details
 *  Wires the internal Linux vtable (@c g_junoUdpLinuxApi) into the module,
 *  stores the failure handler, and immediately opens a POSIX UDP socket
 *  configured according to @p ptCfg. If @c bIsReceiver is @c true the socket
 *  is bound to the local port; otherwise it is connected to the remote address.
 *
 *  Callers do NOT pass @p ptApi — the vtable is selected internally.
 *
 *  On failure the socket is not opened and the module is left in a safe state
 *  with @c _iSockFd = -1.
 *
 *  Initialisation sequence:
 *  1. Guard @p ptUdp and @p ptCfg (returns @c JUNO_STATUS_NULLPTR_ERROR if NULL).
 *  2. Call @c JunoUdp_Init with the internal Linux vtable.
 *  3. Create a POSIX UDP socket.
 *  4. Bind (receiver) or connect (sender) the socket per @p ptCfg.
 *  5. Store the socket fd and address in @c ptUdp->tLinux.
 *  6. Return @c JUNO_STATUS_SUCCESS.
 *
 * @param ptUdp               Caller-owned module union storage; must be non-NULL.
 * @param ptCfg               Socket configuration (address, port, role); must be non-NULL.
 * @param pfcnFailureHandler  Diagnostic callback invoked before any error return; may be NULL.
 * @param pvFailureUserData   Opaque user data pointer passed to the failure handler; may be NULL.
 * @return @c JUNO_STATUS_SUCCESS on success; non-zero on failure (e.g., socket creation error).
 */
// @{"req": ["REQ-UDP-003", "REQ-UDP-004", "REQ-UDP-005", "REQ-UDP-016"]}
JUNO_STATUS_T JunoUdp_LinuxInit(
    JUNO_UDP_T            *ptUdp,
    const JUNO_UDP_CFG_T  *ptCfg,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    void                  *pvFailureUserData
);

#ifdef __cplusplus
}
#endif

#endif /* JUNO_UDP_LINUX_H */
