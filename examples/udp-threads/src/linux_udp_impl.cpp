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
 * @file linux_udp_impl.cpp
 * @brief Linux/POSIX implementation of the UDP module vtable.
 *
 * @details
 *  This is the only translation unit that includes POSIX socket headers.
 *  It provides the static @c s_tJunoUdpLinuxApi vtable that implements
 *  @c Send, @c Receive, and @c Free using BSD sockets, and exports
 *  @c JunoUdp_LinuxInit as the single RAII entry point that wires the vtable
 *  and opens the socket in one call.
 *
 *  Socket file descriptor and address state reside in the @c JUNO_UDP_LINUX_T
 *  derivation, not in the freestanding root. Vtable functions down-cast the
 *  root pointer to @c JUNO_UDP_LINUX_T * to access that state.
 *
 *  All memory is caller-owned; this module allocates nothing.
 */

/* POSIX headers must be outside extern "C" to avoid C++ linkage issues */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/* udp_linux.h pulls in <sys/socket.h>/<netinet/in.h> itself; include it
 * outside extern "C" so the POSIX struct definitions are complete when
 * JUNO_UDP_LINUX_T is instantiated. */
#include "udp_linux.h"

extern "C"
{
#include "juno/status.h"
#include "juno/macros.h"
}

/* --------------------------------------------------------------------------
 * Static helper: open and configure the UDP socket (not in vtable)
 * -------------------------------------------------------------------------- */

/**
 * @brief Open and configure a UDP socket into the Linux derivation.
 *
 * @details
 *  Creates a @c SOCK_DGRAM socket. For receivers, binds to @c INADDR_ANY on
 *  the configured port. For senders, connects to the configured remote address
 *  and port (defaulting to loopback if @p ptCfg->pcAddress is NULL).
 *
 *  Stores the resulting file descriptor and address in @p ptLinux->_iSockFd
 *  and @p ptLinux->_tAddr respectively.
 *
 * @param ptLinux Linux derivation instance; must be non-NULL and pre-initialised.
 * @param ptCfg   Socket configuration; must be non-NULL.
 * @return @c JUNO_STATUS_SUCCESS on success; @c JUNO_STATUS_ERR on POSIX failure.
 */
// @{"req": ["REQ-UDP-003", "REQ-UDP-004", "REQ-UDP-005"]}
static JUNO_STATUS_T OpenSocket(JUNO_UDP_LINUX_T *ptLinux, const JUNO_UDP_CFG_T *ptCfg)
{
    JUNO_ASSERT_EXISTS(ptLinux && ptCfg);
    JUNO_UDP_ROOT_T *ptRoot = &ptLinux->tRoot;

    int iFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (iFd < 0)
    {
        JUNO_FAIL_ROOT(JUNO_STATUS_ERR, ptRoot, "socket() failed");
        return JUNO_STATUS_ERR;
    }

    memset(&ptLinux->_tAddr, 0, sizeof(ptLinux->_tAddr));
    ptLinux->_tAddr.sin_family = AF_INET;
    ptLinux->_tAddr.sin_port   = htons(ptCfg->uPort);

    if (ptCfg->bIsReceiver)
    {
        /* Receiver: bind to INADDR_ANY on the configured port */
        ptLinux->_tAddr.sin_addr.s_addr = INADDR_ANY;
        if (bind(iFd, (struct sockaddr *)&ptLinux->_tAddr, sizeof(ptLinux->_tAddr)) < 0)
        {
            close(iFd);
            JUNO_FAIL_ROOT(JUNO_STATUS_ERR, ptRoot, "bind() failed");
            return JUNO_STATUS_ERR;
        }
    }
    else
    {
        /* Sender: connect to the remote address */
        if (ptCfg->pcAddress != NULL)
        {
            inet_pton(AF_INET, ptCfg->pcAddress, &ptLinux->_tAddr.sin_addr);
        }
        else
        {
            ptLinux->_tAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        }
        if (connect(iFd, (struct sockaddr *)&ptLinux->_tAddr, sizeof(ptLinux->_tAddr)) < 0)
        {
            close(iFd);
            JUNO_FAIL_ROOT(JUNO_STATUS_ERR, ptRoot, "connect() failed");
            return JUNO_STATUS_ERR;
        }
    }

    ptLinux->_iSockFd = iFd;
    return JUNO_STATUS_SUCCESS;
}

/* --------------------------------------------------------------------------
 * Static vtable function implementations
 * -------------------------------------------------------------------------- */

/**
 * @brief Send exactly one @c UDP_THREAD_MSG_T datagram.
 *
 * @details
 *  Down-casts @p ptRoot to @c JUNO_UDP_LINUX_T to access the socket fd.
 *  Transmits the full message in a single @c send() call. Returns
 *  @c JUNO_STATUS_ERR if fewer than @c sizeof(UDP_THREAD_MSG_T) bytes are sent.
 *
 * @param ptRoot Module root instance; socket must be open.
 * @param ptMsg  Message to send; must be non-NULL.
 * @return @c JUNO_STATUS_SUCCESS on success; @c JUNO_STATUS_ERR on failure.
 */
// @{"req": ["REQ-UDP-006", "REQ-UDP-015"]}
static JUNO_STATUS_T Send(JUNO_UDP_ROOT_T *ptRoot, const UDP_THREAD_MSG_T *ptMsg)
{
    JUNO_ASSERT_EXISTS(ptRoot && ptMsg);
    JUNO_UDP_LINUX_T *ptLinux = (JUNO_UDP_LINUX_T *)ptRoot;
    ssize_t iSent = send(ptLinux->_iSockFd, ptMsg, sizeof(UDP_THREAD_MSG_T), 0);
    if (iSent != (ssize_t)sizeof(UDP_THREAD_MSG_T))
    {
        JUNO_FAIL_ROOT(JUNO_STATUS_ERR, ptRoot, "send() failed");
        return JUNO_STATUS_ERR;
    }
    return JUNO_STATUS_SUCCESS;
}

/**
 * @brief Receive exactly one @c UDP_THREAD_MSG_T datagram.
 *
 * @details
 *  Down-casts @p ptRoot to @c JUNO_UDP_LINUX_T to access the socket fd.
 *  Blocks until a datagram arrives. On a system error with @c EAGAIN or
 *  @c EWOULDBLOCK, returns @c JUNO_STATUS_TIMEOUT_ERROR without invoking
 *  the failure handler — timeout is a normal, expected condition for polled
 *  receivers. On a datagram of unexpected size, returns
 *  @c JUNO_STATUS_INVALID_DATA_ERROR.
 *
 * @param ptRoot Module root instance; socket must be open.
 * @param ptMsg  Output buffer; must be non-NULL.
 * @return @c JUNO_STATUS_SUCCESS on success;
 *         @c JUNO_STATUS_TIMEOUT_ERROR on timeout (not a failure);
 *         @c JUNO_STATUS_INVALID_DATA_ERROR on wrong datagram size;
 *         @c JUNO_STATUS_ERR on other POSIX error.
 */
// @{"req": ["REQ-UDP-007", "REQ-UDP-008", "REQ-UDP-015"]}
static JUNO_STATUS_T Receive(JUNO_UDP_ROOT_T *ptRoot, UDP_THREAD_MSG_T *ptMsg)
{
    JUNO_ASSERT_EXISTS(ptRoot && ptMsg);
    JUNO_UDP_LINUX_T *ptLinux = (JUNO_UDP_LINUX_T *)ptRoot;
    ssize_t iRecv = recv(ptLinux->_iSockFd, ptMsg, sizeof(UDP_THREAD_MSG_T), 0);
    if (iRecv < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            /* Normal timeout — NOT a failure; do NOT invoke failure handler */
            return JUNO_STATUS_TIMEOUT_ERROR;
        }
        JUNO_FAIL_ROOT(JUNO_STATUS_ERR, ptRoot, "recv() failed");
        return JUNO_STATUS_ERR;
    }
    if (iRecv != (ssize_t)sizeof(UDP_THREAD_MSG_T))
    {
        JUNO_FAIL_ROOT(JUNO_STATUS_INVALID_DATA_ERROR, ptRoot, "unexpected datagram size");
        return JUNO_STATUS_INVALID_DATA_ERROR;
    }
    return JUNO_STATUS_SUCCESS;
}

/**
 * @brief Close the UDP socket and reset the descriptor to -1 (RAII cleanup).
 *
 * @details
 *  Down-casts @p ptRoot to @c JUNO_UDP_LINUX_T to access the socket fd.
 *  Idempotent: if @c _iSockFd is already -1 (never opened or already freed),
 *  returns @c JUNO_STATUS_SUCCESS without calling @c close().
 *
 * @param ptRoot Module root instance; must be non-NULL.
 * @return @c JUNO_STATUS_SUCCESS always (after a successful guard check).
 */
// @{"req": ["REQ-UDP-009"]}
static JUNO_STATUS_T Free(JUNO_UDP_ROOT_T *ptRoot)
{
    JUNO_ASSERT_EXISTS(ptRoot);
    JUNO_UDP_LINUX_T *ptLinux = (JUNO_UDP_LINUX_T *)ptRoot;
    if (ptLinux->_iSockFd >= 0)
    {
        close(ptLinux->_iSockFd);
        ptLinux->_iSockFd = -1;
    }
    return JUNO_STATUS_SUCCESS;
}

/* --------------------------------------------------------------------------
 * Vtable definition — must appear before JunoUdp_LinuxInit
 * -------------------------------------------------------------------------- */

/**
 * @brief Statically allocated Linux/POSIX UDP vtable.
 *
 * @details
 *  Wired into module roots by @c JunoUdp_LinuxInit. Callers do not reference
 *  this object directly; it is an implementation detail of this translation
 *  unit. This object has static storage duration and must remain valid for the
 *  lifetime of all roots initialised with it.
 */
// @{"req": ["REQ-UDP-014"]}
static const JUNO_UDP_API_T s_tJunoUdpLinuxApi =
{
    Send,
    Receive,
    Free
};

/* --------------------------------------------------------------------------
 * Platform init (RAII entry point)
 * -------------------------------------------------------------------------- */

/**
 * @brief Initialise a Linux UDP module instance and open the socket (RAII).
 *
 * @details
 *  Wires @c s_tJunoUdpLinuxApi into the root via @c JunoUdp_Init, stores the
 *  failure handler, initialises @c _iSockFd to @c -1, then calls
 *  @c OpenSocket to create and bind/connect the POSIX socket.
 *
 *  On failure the socket is not opened and the module is left in a safe state
 *  with @c _iSockFd = -1.
 *
 * @param ptUdp               Caller-owned module union storage; must be non-NULL.
 * @param ptCfg               Socket configuration (address, port, role); must be non-NULL.
 * @param pfcnFailureHandler  Diagnostic callback invoked before any error return; may be NULL.
 * @param pvFailureUserData   Opaque user data pointer passed to the failure handler; may be NULL.
 * @return @c JUNO_STATUS_SUCCESS on success; non-zero on failure.
 */
// @{"req": ["REQ-UDP-003", "REQ-UDP-004", "REQ-UDP-005", "REQ-UDP-016"]}
extern "C" JUNO_STATUS_T JunoUdp_LinuxInit(
    JUNO_UDP_T            *ptUdp,
    const JUNO_UDP_CFG_T  *ptCfg,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    void                  *pvFailureUserData
)
{
    JUNO_ASSERT_EXISTS(ptUdp);
    JUNO_ASSERT_EXISTS(ptCfg);
    JUNO_STATUS_T tStatus = JunoUdp_Init(
        &ptUdp->tRoot, &s_tJunoUdpLinuxApi,
        pfcnFailureHandler, pvFailureUserData
    );
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    ptUdp->tLinux._iSockFd = -1;
    tStatus = OpenSocket(&ptUdp->tLinux, ptCfg);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return JUNO_STATUS_SUCCESS;
}
