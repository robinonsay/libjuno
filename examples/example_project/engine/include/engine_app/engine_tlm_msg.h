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

#ifndef ENGINE_TLM_MSG_H
#define ENGINE_TLM_MSG_H
#include "juno/ds/array_api.h"
#include "juno/memory/pointer_api.h"
#include "juno/module.h"
#include "juno/sb/broker_api.h"
#include "juno/status.h"
#include "juno/time/time_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ENGINE_TLM_MSG_MID            (1)

typedef struct ENGINE_TLM_MSG_TAG
{
    //TODO: Replace me:
    float fRpm;
    JUNO_TIMESTAMP_T tTimestamp;
} ENGINE_TLM_MSG_T;

typedef struct ENGINE_TLM_MSG_ARRAY_TAG JUNO_MODULE_DERIVE(JUNO_DS_ARRAY_ROOT_T,
    ENGINE_TLM_MSG_T *ptArrEngineTlmMsgBuffer;
) ENGINE_TLM_MSG_ARRAY_T;

extern const JUNO_POINTER_API_T gtEngineTlmMsgPointerApi;

#define EngineTlmMsg_PointerInit(addr) JunoMemory_PointerInit(&gtEngineTlmMsgPointerApi, ENGINE_TLM_MSG_T, addr)
#define EngineTlmMsg_PointerVerify(tPointer) JunoMemory_PointerVerifyType(tPointer, ENGINE_TLM_MSG_T, gtEngineTlmMsgPointerApi)
JUNO_STATUS_T EngineTlmMsg_ArrayInit(ENGINE_TLM_MSG_ARRAY_T *ptEngineTlmMsgPipe, ENGINE_TLM_MSG_T *ptArrEngineTlmMsgBuffer, size_t iCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvUserData);

#ifdef __cplusplus
}
#endif
#endif

