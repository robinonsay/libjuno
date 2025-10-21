#ifndef TEMPLATE_MSG_H
#define TEMPLATE_MSG_H
#include "juno/ds/queue_api.h"
#include "juno/memory/pointer_api.h"
#include "juno/module.h"
#include "juno/sb/broker_api.h"
#include "juno/status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TEMPLATE_MSG_MID            (1)

typedef struct TEMPLATE_MSG_TAG
{
    //TODO: Replace me:
    int iExampleArgument;
} TEMPLATE_MSG_T;

typedef struct TEMPLATE_MSG_PIPE_TAG JUNO_MODULE_DERIVE_WITH_API(JUNO_SB_PIPE_T, JUNO_DS_QUEUE_API_T,
    TEMPLATE_MSG_T *ptArrTemplateMsgBuffer;
) TEMPLATE_MSG_PIPE_T;

extern const JUNO_POINTER_API_T gtTemplateMsgPointerApi;

#define TemplateMsg_PointerInit(addr) JunoMemory_PointerInit(&gtTemplateMsgPointerApi, TEMPLATE_MSG_T, addr)
#define TemplateMsg_PointerVerify(tPointer) JunoMemory_PointerVerifyType(tPointer, TEMPLATE_MSG_T, gtTemplateMsgPointerApi)
JUNO_STATUS_T TemplateMsg_PipeInit(TEMPLATE_MSG_PIPE_T *ptTemplateMsgPipe, TEMPLATE_MSG_T *ptArrTemplateMsgBuffer, size_t iCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvUserData);

#ifdef __cplusplus
}
#endif
#endif

