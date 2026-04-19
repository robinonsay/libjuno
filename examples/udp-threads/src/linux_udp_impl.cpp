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
 *  It provides the statically-allocated @c g_junoUdpLinuxApi vtable that
 *  implements @c Open, @c Send, @c Receive, and @c Close using BSD sockets.
 *
 *  All memory is caller-owned; this module allocates nothing.
 */

extern "C"
{
#include "udp_api.h"
#include "juno/status.h"
#include "juno/macros.h"
}

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>

/* --------------------------------------------------------------------------
 * Static vtable function implementations
 * -------------------------------------------------------------------------- */

/**
 * @brief Open and configure a UDP socket.
 *
 * @details
 *  Creates a SOCK_DGRAM socket. If @p ptCfg->uTimeoutMs is non-zero, sets
 *  @c SO_RCVTIMEO on the socket. For receivers, binds to @c INADDR_ANY on the
 *  configured port. For senders, connects to the configured remote address and
 *  port (defaulting to loopback if @p ptCfg->pcAddress is NULL).
 *
 * @param ptRoot Module root instance; must be non-NULL.
 * @param ptCfg  Socket configuration; must be non-NULL.
 * @return @c JUNO_STATUS_SUCCESS on success; @c JUNO_STATUS_ERR on POSIX failure.
 */
// @{"req": ["REQ-UDP-003", "REQ-UDP-004", "REQ-UDP-005"]}
static JUNO_STATUS_T Open(JUNO_UDP_ROOT_T *ptRoot, const JUNO_UDP_CFG_T *ptCfg)
{
    JUNO_ASSERT_EXISTS(ptRoot && ptCfg);

    int iFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (iFd < 0)
    {
        JUNO_FAIL_ROOT(JUNO_STATUS_ERR, ptRoot, "socket() failed");
        return JUNO_STATUS_ERR;
    }

    /* Set receive timeout if configured */
    if (ptCfg->uTimeoutMs > 0u)
    {
        struct timeval tTimeout;
        tTimeout.tv_sec  = (time_t)(ptCfg->uTimeoutMs / 1000u);
        tTimeout.tv_usec = (suseconds_t)((ptCfg->uTimeoutMs % 1000u) * 1000u);
        if (setsockopt(iFd, SOL_SOCKET, SO_RCVTIMEO,
                       &tTimeout, sizeof(tTimeout)) < 0)
        {
            close(iFd);
            JUNO_FAIL_ROOT(JUNO_STATUS_ERR, ptRoot, "setsockopt(SO_RCVTIMEO) failed");
            return JUNO_STATUS_ERR;
        }
    }

    struct sockaddr_in tAddr;
    memset(&tAddr, 0, sizeof(tAddr));
    tAddr.sin_family = AF_INET;
    tAddr.sin_port   = htons(ptCfg->uPort);

    if (ptCfg->bIsReceiver)
    {
        /* Receiver: bind to INADDR_ANY on the configured port */
        tAddr.sin_addr.s_addr = INADDR_ANY;
        if (bind(iFd, (struct sockaddr *)&tAddr, sizeof(tAddr)) < 0)
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
            inet_pton(AF_INET, ptCfg->pcAddress, &tAddr.sin_addr);
        }
        else
        {
            tAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        }
        if (connect(iFd, (struct sockaddr *)&tAddr, sizeof(tAddr)) < 0)
        {
            close(iFd);
            JUNO_FAIL_ROOT(JUNO_STATUS_ERR, ptRoot, "connect() failed");
            return JUNO_STATUS_ERR;
        }
    }

    ptRoot->_iSockFd = (intptr_t)iFd;
    return JUNO_STATUS_SUCCESS;
}

/**
 * @brief Send exactly one @c UDP_THREAD_MSG_T datagram.
 *
 * @details
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
    ssize_t iSent = send((int)ptRoot->_iSockFd, ptMsg, sizeof(UDP_THREAD_MSG_T), 0);
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
 *  Blocks until a datagram arrives or the receive timeout (set by @c Open)
 *  expires. On timeout (@c EAGAIN / @c EWOULDBLOCK), returns
 *  @c JUNO_STATUS_TIMEOUT_ERROR without invoking the failure handler — timeout
 *  is a normal, expected condition for polled receivers. On a datagram of
 *  unexpected size, returns @c JUNO_STATUS_INVALID_DATA_ERROR.
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
    ssize_t iRecv = recv((int)ptRoot->_iSockFd, ptMsg, sizeof(UDP_THREAD_MSG_T), 0);
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
 * @brief Close the UDP socket and reset the descriptor to -1.
 *
 * @details
 *  Idempotent: if @c _iSockFd is already -1 (never opened or already closed),
 *  returns @c JUNO_STATUS_SUCCESS without calling @c close().
 *
 * @param ptRoot Module root instance; must be non-NULL.
 * @return @c JUNO_STATUS_SUCCESS always (after a successful guard check).
 */
// @{"req": ["REQ-UDP-009"]}
static JUNO_STATUS_T Close(JUNO_UDP_ROOT_T *ptRoot)
{
    JUNO_ASSERT_EXISTS(ptRoot);
    if (ptRoot->_iSockFd >= 0)
    {
        close((int)ptRoot->_iSockFd);
        ptRoot->_iSockFd = -1;
    }
    return JUNO_STATUS_SUCCESS;
}

/* --------------------------------------------------------------------------
 * Vtable definition
 * -------------------------------------------------------------------------- */

/**
 * @brief Statically allocated Linux/POSIX UDP vtable.
 *
 * @details
 *  Declared @c extern in @c udp_api.h. Pass to @c JunoUdp_Init to use the
 *  real POSIX socket implementation. This object has static storage duration
 *  and must remain valid for the lifetime of all roots initialised with it.
 */
// @{"req": ["REQ-UDP-014"]}
const JUNO_UDP_API_T g_junoUdpLinuxApi =
{
    Open,
    Send,
    Receive,
    Close
};
