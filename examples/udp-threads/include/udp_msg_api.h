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
 * @file udp_msg_api.h
 * @brief Message API infrastructure for the udp-threads example.
 *
 * @details
 *  Provides the pointer API and array API for UDP_THREAD_MSG_T, along with
 *  the concrete UDPTH_MSG_ARRAY_T type used as backing storage for pipe queues
 *  in MonitorApp and ProcessorApp. Also defines the shared message ID constant.
 */
#ifndef UDP_MSG_API_H
#define UDP_MSG_API_H

#include "udp_api.h"
#include "juno/ds/array_api.h"
#include "juno/memory/pointer_api.h"
#include "juno/sb/broker_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* --------------------------------------------------------------------------
 * Constants
 * -------------------------------------------------------------------------- */

/** Number of messages the pipe queue can buffer. */
#define UDPTH_PIPE_CAPACITY  8u

/** Message ID used by all four apps on the software bus. */
#define UDPTH_MSG_MID  ((JUNO_SB_MID_T)1u)

/* --------------------------------------------------------------------------
 * UDPTH_MSG_ARRAY_T — concrete array for pipe queue backing storage
 * -------------------------------------------------------------------------- */

/**
 * @brief Concrete array type for the MonitorApp and ProcessorApp pipe queues.
 *
 * Embeds JUNO_DS_ARRAY_ROOT_T as first member (via JUNO_MODULE_DERIVE pattern),
 * followed by the fixed-size message buffer. The composition root allocates this
 * struct statically and injects &tArr.tRoot into MonitorApp_Init / ProcessorApp_Init.
 */
struct UDPTH_MSG_ARRAY_TAG JUNO_MODULE_DERIVE(JUNO_DS_ARRAY_ROOT_T,
    UDP_THREAD_MSG_T atBuffer[UDPTH_PIPE_CAPACITY];
);
typedef struct UDPTH_MSG_ARRAY_TAG UDPTH_MSG_ARRAY_T;

/* --------------------------------------------------------------------------
 * Globals
 * -------------------------------------------------------------------------- */

/** @brief Pointer API for UDP_THREAD_MSG_T — used with JunoMemory_PointerInit. */
extern const JUNO_POINTER_API_T g_udpThreadMsgPointerApi;

/** @brief Array API for UDPTH_MSG_ARRAY_T — used with JunoDs_ArrayInit. */
extern const JUNO_DS_ARRAY_API_T g_udpThreadMsgArrayApi;

/* --------------------------------------------------------------------------
 * Macros and helpers
 * -------------------------------------------------------------------------- */

/**
 * @brief Convenience macro to initialise a JUNO_POINTER_T for a UDP_THREAD_MSG_T.
 * @param addr Address of the UDP_THREAD_MSG_T instance.
 * @return A populated JUNO_POINTER_T describing the location, size, and alignment.
 */
#define UdpThreadMsg_PointerInit(addr) \
    JunoMemory_PointerInit(&g_udpThreadMsgPointerApi, UDP_THREAD_MSG_T, (addr))

/**
 * @brief Initialise a UDPTH_MSG_ARRAY_T with the shared array API.
 *
 * @param ptArr        Caller-owned array storage; must be non-NULL.
 * @param pfcnHandler  Optional failure handler; may be NULL.
 * @param pvUserData   Optional user data passed to the failure handler; may be NULL.
 * @return JUNO_STATUS_SUCCESS on success; non-zero on failure.
 */
static inline JUNO_STATUS_T UdpThreadMsgArray_Init(
    UDPTH_MSG_ARRAY_T *ptArr,
    JUNO_FAILURE_HANDLER_T pfcnHandler,
    JUNO_USER_DATA_T *pvUserData)
{
    return JunoDs_ArrayInit(&ptArr->tRoot, &g_udpThreadMsgArrayApi,
                            UDPTH_PIPE_CAPACITY, pfcnHandler, pvUserData);
}

#ifdef __cplusplus
}
#endif

#endif /* UDP_MSG_API_H */
