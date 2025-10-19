#ifndef ENGINE_TLM_MSG_PIPE_H
#define ENGINE_TLM_MSG_PIPE_H
#include "juno/ds/queue_api.h"
#include "juno/memory/pointer_api.h"
#include "juno/module.h"
#include "juno/sb/broker_api.h"
#include "juno/status.h"
#include "engine_app/engine_tlm_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ENGINE_TLM_MSG_PIPE_DEPTH     (1)

typedef struct ENGINE_TLM_MSG_PIPE_TAG JUNO_MODULE_DERIVE_WITH_API(JUNO_SB_PIPE_T, JUNO_DS_QUEUE_API_T,
    ENGINE_TLM_MSG_T ptArrCmdPipe[ENGINE_TLM_MSG_PIPE_DEPTH];
) ENGINE_TLM_MSG_PIPE_T;

JUNO_STATUS_T EngineTlmMsgPipeInit(ENGINE_TLM_MSG_PIPE_T *ptCmdPipe, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvUserData);


#ifdef __cplusplus
}
#endif
#endif

