#ifndef ENGINE_CMD_MSG_H
#define ENGINE_CMD_MSG_H
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

extern const JUNO_POINTER_API_T gtEngineCmdMsgPointerApi;
#define EngineCmdMsg_PointerInit(addr) JunoMemory_PointerInit(&gtEngineCmdMsgPointerApi, ENGINE_CMD_MSG_T, addr)

#ifdef __cplusplus
}
#endif
#endif

