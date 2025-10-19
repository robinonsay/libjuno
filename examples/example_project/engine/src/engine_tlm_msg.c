#include "engine_app/engine_tlm_msg.h"
#include "juno/memory/pointer_api.h"

JUNO_POINTER_API_FUNC_DECLARE(ENGINE_TLM_MSG_T);
const JUNO_POINTER_API_T gtEngineTlmMsgPointerApi = JunoMemory_PointerApiInit(ENGINE_TLM_MSG_T);
JUNO_POINTER_API_FUNC(ENGINE_TLM_MSG_T, gtEngineTlmMsgPointerApi)
