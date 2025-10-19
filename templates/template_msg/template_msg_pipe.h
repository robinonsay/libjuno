#ifndef TEMPLATE_MSG_PIPE_H
#define TEMPLATE_MSG_PIPE_H
#include "juno/memory/pointer_api.h"
#include "juno/module.h"
#include "juno/sb/broker_api.h"
#include "juno/status.h"
#include "template_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TEMPLATE_MSG_PIPE_DEPTH     (1)

typedef struct TEMPLATE_MSG_PIPE_TAG JUNO_MODULE_DERIVE(JUNO_SB_PIPE_T,
    TEMPLATE_MSG_T ptArrCmdPipe[TEMPLATE_MSG_PIPE_DEPTH];
) TEMPLATE_MSG_PIPE_T;

JUNO_STATUS_T TemplateMsgPipeInit(TEMPLATE_MSG_PIPE_T *ptCmdPipe, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvUserData);


#ifdef __cplusplus
}
#endif
#endif

