#ifndef TEMPLATE_MSG_H
#define TEMPLATE_MSG_H
#include "juno/ds/array_api.h"
#include "juno/memory/pointer_api.h"
#include "juno/module.h"
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

typedef struct TEMPLATE_MSG_ARRAY_TAG JUNO_MODULE_DERIVE(JUNO_DS_ARRAY_ROOT_T,
    TEMPLATE_MSG_T *ptArrTemplateMsgBuffer;
) TEMPLATE_MSG_ARRAY_T;

extern const JUNO_POINTER_API_T gtTemplateMsgPointerApi;

#define TemplateMsg_PointerInit(addr) JunoMemory_PointerInit(&gtTemplateMsgPointerApi, TEMPLATE_MSG_T, addr)
#define TemplateMsg_PointerVerify(tPointer) JunoMemory_PointerVerifyType(tPointer, TEMPLATE_MSG_T, gtTemplateMsgPointerApi)
JUNO_STATUS_T TemplateMsg_ArrayInit(TEMPLATE_MSG_ARRAY_T *ptTemplateMsgPipe, TEMPLATE_MSG_T *ptArrTemplateMsgBuffer, size_t iCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvUserData);

#ifdef __cplusplus
}
#endif
#endif

