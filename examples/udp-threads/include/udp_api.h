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
 * @file udp_api.h
 * @brief Public C11 interface for the UDP socket module (udp-threads example).
 *
 * @details
 *  This header defines the freestanding-compatible public interface for the
 *  UDP socket module used by the udp-threads example. It follows the LibJuno
 *  vtable/dependency-injection module pattern:
 *
 *  - Interface layer (this header): no POSIX or OS-specific includes; may be
 *    compiled with @c -nostdlib @c -ffreestanding.
 *  - Implementation layer (@c linux_udp_impl.cpp): the only translation unit
 *    that includes POSIX socket headers. It provides @c g_junoUdpLinuxApi.
 *
 *  All memory is caller-owned and injected. The module allocates nothing.
 *
 *  Typical usage:
 *  @code{.c}
 *  JUNO_UDP_T tUdp;
 *  JunoUdp_Init(&tUdp.tRoot, &g_junoUdpLinuxApi, NULL, NULL);
 *
 *  JUNO_UDP_CFG_T tCfg = { "127.0.0.1", 5000, 500, false };
 *  tUdp.tRoot.ptApi->Open(&tUdp.tRoot, &tCfg);
 *
 *  UDP_THREAD_MSG_T tMsg = { 0 };
 *  tUdp.tRoot.ptApi->Send(&tUdp.tRoot, &tMsg);
 *  tUdp.tRoot.ptApi->Close(&tUdp.tRoot);
 *  @endcode
 */
#ifndef JUNO_UDP_API_H
#define JUNO_UDP_API_H

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

/* --------------------------------------------------------------------------
 * Forward declarations
 * -------------------------------------------------------------------------- */

/** @brief Forward declaration of the UDP module root type. */
typedef struct JUNO_UDP_ROOT_TAG  JUNO_UDP_ROOT_T;

/** @brief Forward declaration of the UDP vtable (API) type. */
typedef struct JUNO_UDP_API_TAG   JUNO_UDP_API_T;

/** @brief Forward declaration of the Linux derivation type. */
typedef struct JUNO_UDP_LINUX_TAG JUNO_UDP_LINUX_T;

/** @brief Forward declaration of the UDP module union type. */
typedef union  JUNO_UDP_TAG       JUNO_UDP_T;

/* --------------------------------------------------------------------------
 * UDP_THREAD_MSG_T — fixed-size datagram message
 * -------------------------------------------------------------------------- */

/**
 * @brief Fixed-size UDP datagram message transferred between sender and receiver.
 *
 * @details
 *  Every Send and Receive call transfers exactly one @c UDP_THREAD_MSG_T as an
 *  atomic datagram (@c sizeof(UDP_THREAD_MSG_T) = 76 bytes). The fixed,
 *  compile-time-known size ensures datagrams are never truncated and no partial
 *  reads or writes are possible at the message boundary.
 *
 *  The @c arr prefix on @c arrPayload denotes a fixed-size static array field
 *  (an extension to the project Hungarian notation convention).
 */
// @{"req": ["REQ-UDP-018"]}
typedef struct UDP_THREAD_MSG_TAG
{
    /** @brief Monotonically increasing sequence number; wraps at UINT32_MAX. */
    uint32_t uSeqNum;
    /** @brief Whole-second component of the sender's timestamp. */
    uint32_t uTimestampSec;
    /** @brief Sub-second component of the sender's timestamp (units defined by application). */
    uint32_t uTimestampSubSec;
    /** @brief Fixed-size application-defined payload (64 bytes). */
    uint8_t  arrPayload[64];
} UDP_THREAD_MSG_T;

/* --------------------------------------------------------------------------
 * JUNO_UDP_CFG_T — socket configuration
 * -------------------------------------------------------------------------- */

/**
 * @brief Configuration passed to @c Open to create and configure a UDP socket.
 *
 * @details
 *  The caller allocates this struct and it need only remain valid for the
 *  duration of the @c Open call. No pointer from this struct is retained
 *  by the module after @c Open returns.
 *
 *  The @c bIsReceiver field selects the socket role:
 *  - @c true  — implementation calls @c bind() for incoming datagrams (receiver).
 *  - @c false — implementation calls @c connect() to associate with the remote
 *               address and port (sender).
 */
// @{"req": ["REQ-UDP-017"]}
typedef struct JUNO_UDP_CFG_TAG
{
    /** @brief IPv4 address string (e.g. "127.0.0.1"); NULL or "0.0.0.0" for receiver. */
    const char *pcAddress;
    /** @brief UDP port number in host byte order. */
    uint16_t    uPort;
    /** @brief Receive timeout in milliseconds; 0 means no timeout (blocking). */
    uint32_t    uTimeoutMs;
    /** @brief true = bind to local port (receiver); false = connect to remote (sender). */
    bool        bIsReceiver;
} JUNO_UDP_CFG_T;

/* --------------------------------------------------------------------------
 * JUNO_UDP_API_T — vtable
 * -------------------------------------------------------------------------- */

/**
 * @brief Vtable defining the UDP socket module interface.
 *
 * @details
 *  Every concrete UDP implementation (Linux POSIX, test double) provides a
 *  statically allocated @c JUNO_UDP_API_T whose function pointers are wired
 *  into a root by @c JunoUdp_Init. All four operations take the module root as
 *  their first argument and return @c JUNO_STATUS_T for uniform error propagation.
 */
// @{"req": ["REQ-UDP-002"]}
struct JUNO_UDP_API_TAG
{
    /**
     * @brief Open and configure the UDP socket.
     * @param ptRoot Module root instance; must be non-NULL and initialised.
     * @param ptCfg  Socket configuration; must be non-NULL and valid for the call.
     * @return @c JUNO_STATUS_SUCCESS on success; non-zero on failure.
     */
    JUNO_STATUS_T (*Open)   (JUNO_UDP_ROOT_T *ptRoot, const JUNO_UDP_CFG_T *ptCfg);

    /**
     * @brief Send exactly one @c UDP_THREAD_MSG_T datagram.
     * @param ptRoot Module root instance; socket must be open.
     * @param ptMsg  Message to send; must be non-NULL and valid for the call.
     * @return @c JUNO_STATUS_SUCCESS on success; non-zero on failure.
     */
    JUNO_STATUS_T (*Send)   (JUNO_UDP_ROOT_T *ptRoot, const UDP_THREAD_MSG_T *ptMsg);

    /**
     * @brief Receive exactly one @c UDP_THREAD_MSG_T datagram.
     * @param ptRoot Module root instance; socket must be open.
     * @param ptMsg  Output buffer; must be non-NULL and valid for the call.
     * @return @c JUNO_STATUS_SUCCESS on success; @c JUNO_STATUS_TIMEOUT_ERROR on
     *         timeout (normal condition, failure handler not invoked); non-zero on
     *         other failures.
     */
    JUNO_STATUS_T (*Receive)(JUNO_UDP_ROOT_T *ptRoot, UDP_THREAD_MSG_T *ptMsg);

    /**
     * @brief Close the UDP socket and reset the socket descriptor to -1.
     * @details Idempotent: safe to call even if @c Open was never called or already
     *          closed. Returns @c JUNO_STATUS_SUCCESS without error in that case.
     * @param ptRoot Module root instance; must be non-NULL.
     * @return @c JUNO_STATUS_SUCCESS on success; non-zero on failure.
     */
    JUNO_STATUS_T (*Close)  (JUNO_UDP_ROOT_T *ptRoot);
};

/* --------------------------------------------------------------------------
 * JUNO_UDP_ROOT_T — module root
 * -------------------------------------------------------------------------- */

/**
 * @brief UDP module root; the primary instance type used throughout the API.
 *
 * @details
 *  Defined via @c JUNO_MODULE_ROOT, which expands to a struct body containing:
 *  - @c ptApi                  — vtable pointer wired by @c JunoUdp_Init.
 *  - @c _pfcnFailureHandler    — diagnostic callback; invoked before any error return.
 *  - @c _pvFailureUserData     — opaque pointer passed to the failure handler.
 *  - @c _iSockFd               — POSIX socket descriptor; sentinel @c -1 = closed.
 *
 *  The @c _iSockFd field uses @c intptr_t to be wide enough for any file
 *  descriptor on both 32-bit and 64-bit platforms without casting.
 *
 *  The caller allocates (stack or static) and owns this struct. Its lifetime
 *  must exceed that of all API calls made on it.
 */
// @{"req": ["REQ-UDP-001", "REQ-UDP-010", "REQ-UDP-011", "REQ-UDP-012"]}
struct JUNO_UDP_ROOT_TAG JUNO_MODULE_ROOT(JUNO_UDP_API_T,
    /** @brief POSIX socket file descriptor; -1 when the socket is closed/invalid. */
    intptr_t _iSockFd;
);
typedef struct JUNO_UDP_ROOT_TAG JUNO_UDP_ROOT_T;

/* --------------------------------------------------------------------------
 * JUNO_UDP_LINUX_T — Linux/POSIX derivation
 * -------------------------------------------------------------------------- */

/**
 * @brief Linux POSIX derivation of the UDP module.
 *
 * @details
 *  Embeds @c JUNO_UDP_ROOT_T as its first member (@c tRoot via JUNO_MODULE_SUPER),
 *  enabling safe up-cast to the root for vtable dispatch. Because @c _iSockFd is
 *  carried directly in the root via the @c JUNO_MODULE_ROOT variadic argument,
 *  this derivation requires no additional fields.
 *
 *  Callers allocate a @c JUNO_UDP_T union and pass @c &tUdp.tRoot to
 *  @c JunoUdp_Init; they need not interact with this type directly.
 */
struct JUNO_UDP_LINUX_TAG JUNO_MODULE_DERIVE(JUNO_UDP_ROOT_T, JUNO_MODULE_EMPTY);
typedef struct JUNO_UDP_LINUX_TAG JUNO_UDP_LINUX_T;

/* --------------------------------------------------------------------------
 * JUNO_UDP_T — module union
 * -------------------------------------------------------------------------- */

/**
 * @brief Type-safe polymorphic handle for a UDP module instance.
 *
 * @details
 *  Callers allocate this union (stack or static). Initialise via:
 *  @code{.c}
 *  JUNO_UDP_T tUdp;
 *  JunoUdp_Init(&tUdp.tRoot, &g_junoUdpLinuxApi, pfcnHandler, pvUserData);
 *  @endcode
 *
 *  All subsequent API calls use @c &tUdp.tRoot. The @c tLinux member is
 *  present to satisfy the LibJuno module union pattern; application code does
 *  not need to access it directly.
 */
union JUNO_UDP_TAG JUNO_MODULE(JUNO_UDP_API_T, JUNO_UDP_ROOT_T,
    /** @brief Linux POSIX derivation view. */
    JUNO_UDP_LINUX_T tLinux;
);
typedef union JUNO_UDP_TAG JUNO_UDP_T;

/* --------------------------------------------------------------------------
 * Public function declarations
 * -------------------------------------------------------------------------- */

/**
 * @brief Initialise a UDP module root with a concrete vtable and failure handler.
 *
 * @details
 *  Wires @p ptApi into @p ptRoot->ptApi, stores the failure handler and user
 *  data, and sets @p ptRoot->_iSockFd to @c -1 (no socket open). Must be called
 *  before any vtable operation.
 *
 *  Initialisation sequence:
 *  1. Guard @p ptRoot (returns @c JUNO_STATUS_NULLPTR_ERROR if NULL).
 *  2. Guard @p ptApi  (returns @c JUNO_STATUS_NULLPTR_ERROR if NULL).
 *  3. Wire vtable, failure handler, user data.
 *  4. Set @c _iSockFd = -1.
 *  5. Return @c JUNO_STATUS_SUCCESS.
 *
 * @param ptRoot              Caller-owned root storage; must be non-NULL.
 * @param ptApi               Vtable (Linux implementation or test double); must be non-NULL.
 * @param pfcnFailureHandler  Diagnostic callback invoked before any error return; may be NULL.
 * @param pvFailureUserData   Opaque user data pointer threaded to the failure handler; may be NULL.
 * @return @c JUNO_STATUS_SUCCESS on success; @c JUNO_STATUS_NULLPTR_ERROR if any
 *         required pointer is NULL.
 */
JUNO_STATUS_T JunoUdp_Init(
    JUNO_UDP_ROOT_T        *ptRoot,
    const JUNO_UDP_API_T   *ptApi,
    JUNO_FAILURE_HANDLER_T  pfcnFailureHandler,
    void                   *pvFailureUserData
);

/* --------------------------------------------------------------------------
 * Linux vtable global
 * -------------------------------------------------------------------------- */

/**
 * @brief Statically allocated Linux/POSIX UDP vtable.
 *
 * @details
 *  Defined in @c linux_udp_impl.cpp. Pass to @c JunoUdp_Init to use the real
 *  POSIX socket implementation. Must remain valid for the lifetime of all roots
 *  initialised with it.
 */
extern const JUNO_UDP_API_T g_junoUdpLinuxApi;

#ifdef __cplusplus
}
#endif

#endif /* JUNO_UDP_API_H */
