#include "template_msg.h"
#include "juno/memory/pointer_api.h"

JUNO_POINTER_API_FUNC_DECLARE(TEMPLATE_MSG_T);
const JUNO_POINTER_API_T gtTemplateMsgPointerApi = JunoMemory_PointerApiInit(TEMPLATE_MSG_T);
JUNO_POINTER_API_FUNC(TEMPLATE_MSG_T, gtTemplateMsgPointerApi)
