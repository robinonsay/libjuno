#ifndef ENGINE_CMD_MSG_H
#define ENGINE_CMD_MSG_H
#include "juno/ds/queue_api.h"
#include "juno/memory/pointer_api.h"
#include "juno/module.h"
#include "juno/sb/broker_api.h"
#include "juno/status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ENGINE_CMD_MSG_MID            (0x1800)

typedef struct ENGINE_CMD_MSG_TAG
{
    float fRpm;
} ENGINE_CMD_MSG_T;

typedef struct ENGINE_CMD_MSG_PIPE_TAG JUNO_MODULE_DERIVE_WITH_API(JUNO_SB_PIPE_T, JUNO_DS_QUEUE_API_T,
    ENGINE_CMD_MSG_T *ptArrEngineCmdMsgBuffer;
) ENGINE_CMD_MSG_PIPE_T;

extern const JUNO_POINTER_API_T gtEngineCmdMsgPointerApi;

#define EngineCmdMsg_PointerInit(addr) JunoMemory_PointerInit(&gtEngineCmdMsgPointerApi, ENGINE_CMD_MSG_T, addr)
#define EngineCmdMsg_PointerVerify(tPointer) JunoMemory_PointerVerifyType(tPointer, ENGINE_CMD_MSG_T, gtEngineCmdMsgPointerApi)
JUNO_STATUS_T EngineCmdMsg_PipeInit(ENGINE_CMD_MSG_PIPE_T *ptEngineCmdMsgPipe, ENGINE_CMD_MSG_T *ptArrEngineCmdMsgBuffer, size_t iCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvUserData);

#ifdef __cplusplus
}
#endif
#endif

