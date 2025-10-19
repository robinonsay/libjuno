#include "engine_app/engine_cmd_msg.h"
#include "juno/memory/pointer_api.h"

JUNO_POINTER_API_FUNC_DECLARE(ENGINE_CMD_MSG_T);
const JUNO_POINTER_API_T gtEngineCmdMsgPointerApi = JunoMemory_PointerApiInit(ENGINE_CMD_MSG_T);
JUNO_POINTER_API_FUNC(ENGINE_CMD_MSG_T, gtEngineCmdMsgPointerApi)
