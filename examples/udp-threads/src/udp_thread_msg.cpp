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
 * @file udp_thread_msg.cpp
 * @brief Pointer API and array API implementations for UDP_THREAD_MSG_T.
 *
 * @details
 *  Provides g_udpThreadMsgPointerApi (Copy/Reset for UDP_THREAD_MSG_T) and
 *  g_udpThreadMsgArrayApi (SetAt/GetAt/RemoveAt over UDPTH_MSG_ARRAY_T backing
 *  storage). These globals are injected into pipe queues and the software bus.
 */

#include "udp_msg_api.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include <string.h>
#include <stdalign.h>

/* Forward declaration — defined later in this file. */
extern const JUNO_POINTER_API_T g_udpThreadMsgPointerApi;

/* --------------------------------------------------------------------------
 * Internal helper — construct a JUNO_POINTER_T without compound literals
 * -------------------------------------------------------------------------- */

/**
 * @brief Build a JUNO_POINTER_T for a UDP_THREAD_MSG_T slot.
 *
 * Avoids the C99 compound-literal syntax used by JunoMemory_PointerInit so
 * that this translation unit compiles cleanly as C++11.
 *
 * @param ptMsg Address of the UDP_THREAD_MSG_T instance.
 * @return Populated JUNO_POINTER_T.
 */
static JUNO_POINTER_T MakeMsgPointer(UDP_THREAD_MSG_T *ptMsg)
{
    JUNO_POINTER_T tPtr;
    tPtr.ptApi       = &g_udpThreadMsgPointerApi;
    tPtr.pvAddr      = ptMsg;
    tPtr.zSize       = sizeof(UDP_THREAD_MSG_T);
    tPtr.zAlignment  = alignof(UDP_THREAD_MSG_T);
    return tPtr;
}

/* --------------------------------------------------------------------------
 * Pointer API — Copy / Reset
 * -------------------------------------------------------------------------- */

static JUNO_STATUS_T UdpThreadMsg_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc)
{
    JUNO_STATUS_T tStatus = JunoMemory_PointerVerifyType(
        tDest, UDP_THREAD_MSG_T, g_udpThreadMsgPointerApi);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerifyType(
        tSrc, UDP_THREAD_MSG_T, g_udpThreadMsgPointerApi);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    *(UDP_THREAD_MSG_T *)tDest.pvAddr = *(const UDP_THREAD_MSG_T *)tSrc.pvAddr;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T UdpThreadMsg_Reset(JUNO_POINTER_T tPointer)
{
    JUNO_STATUS_T tStatus = JunoMemory_PointerVerifyType(
        tPointer, UDP_THREAD_MSG_T, g_udpThreadMsgPointerApi);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    UDP_THREAD_MSG_T *ptMsg = (UDP_THREAD_MSG_T *)tPointer.pvAddr;
    memset(ptMsg, 0, sizeof(UDP_THREAD_MSG_T));
    return JUNO_STATUS_SUCCESS;
}

const JUNO_POINTER_API_T g_udpThreadMsgPointerApi = {
    UdpThreadMsg_Copy,
    UdpThreadMsg_Reset
};

/* --------------------------------------------------------------------------
 * Array API — SetAt / GetAt / RemoveAt
 * -------------------------------------------------------------------------- */

static JUNO_STATUS_T UdpThreadMsgArray_SetAt(
    JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex)
{
    JUNO_ASSERT_EXISTS(ptArray && ptArray->ptApi == &g_udpThreadMsgArrayApi);
    UDPTH_MSG_ARRAY_T *ptArr = (UDPTH_MSG_ARRAY_T *)ptArray;
    JUNO_POINTER_T tDest = MakeMsgPointer(&ptArr->atBuffer[iIndex]);
    return UdpThreadMsg_Copy(tDest, tItem);
}

static JUNO_RESULT_POINTER_T UdpThreadMsgArray_GetAt(
    JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    if (ptArray == NULL || ptArray->ptApi != &g_udpThreadMsgArrayApi)
    {
        JUNO_POINTER_T tNull;
        tNull.ptApi      = NULL;
        tNull.pvAddr     = NULL;
        tNull.zSize      = 0u;
        tNull.zAlignment = 0u;
        JUNO_RESULT_POINTER_T tErr;
        tErr.tStatus = JUNO_STATUS_INVALID_REF_ERROR;
        tErr.tOk     = tNull;
        return tErr;
    }
    UDPTH_MSG_ARRAY_T *ptArr = (UDPTH_MSG_ARRAY_T *)ptArray;
    JUNO_RESULT_POINTER_T tOk;
    tOk.tStatus = JUNO_STATUS_SUCCESS;
    tOk.tOk     = MakeMsgPointer(&ptArr->atBuffer[iIndex]);
    return tOk;
}

static JUNO_STATUS_T UdpThreadMsgArray_RemoveAt(
    JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_ASSERT_EXISTS(ptArray && ptArray->ptApi == &g_udpThreadMsgArrayApi);
    UDPTH_MSG_ARRAY_T *ptArr = (UDPTH_MSG_ARRAY_T *)ptArray;
    JUNO_POINTER_T tSlot = MakeMsgPointer(&ptArr->atBuffer[iIndex]);
    return UdpThreadMsg_Reset(tSlot);
}

const JUNO_DS_ARRAY_API_T g_udpThreadMsgArrayApi = {
    UdpThreadMsgArray_SetAt,
    UdpThreadMsgArray_GetAt,
    UdpThreadMsgArray_RemoveAt
};
