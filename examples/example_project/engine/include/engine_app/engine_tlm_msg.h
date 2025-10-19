#ifndef ENGINE_TLM_MSG_H
#define ENGINE_TLM_MSG_H
#include "juno/memory/pointer_api.h"
#include "juno/module.h"
#include "juno/sb/broker_api.h"
#include "juno/status.h"
#include "juno/time/time_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ENGINE_TLM_MSG_MID            (0x0800)

typedef struct ENGINE_TLM_MSG_TAG
{
    float fRpm;
    JUNO_TIMESTAMP_T tTimestamp;
} ENGINE_TLM_MSG_T;

extern const JUNO_POINTER_API_T gtEngineTlmMsgPointerApi;
#define EngineTlmMsg_PointerInit(addr) JunoMemory_PointerInit(&gtEngineTlmMsgPointerApi, ENGINE_TLM_MSG_T, addr)
#ifdef __cplusplus
}
#endif
#endif

