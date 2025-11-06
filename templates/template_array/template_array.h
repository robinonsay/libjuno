#ifndef TEMPLATE_ARRAY_H
#define TEMPLATE_ARRAY_H
#include "juno/ds/array_api.h"
#include "juno/memory/pointer_api.h"
#include "juno/module.h"
#include "juno/status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TEMPLATE_MID            (1)

typedef struct TEMPLATE_TAG
{
    //TODO: Replace me:
    int iExampleArgument;
} TEMPLATE_T;

typedef struct TEMPLATE_ARRAY_TAG JUNO_MODULE_DERIVE(JUNO_DS_ARRAY_ROOT_T,
    TEMPLATE_T *ptArrTemplateBuffer;
) TEMPLATE_ARRAY_T;

extern const JUNO_POINTER_API_T gtTemplateMsgPointerApi;

#define Template_PointerInit(addr) JunoMemory_PointerInit(&gtTemplateMsgPointerApi, TEMPLATE_T, addr)
#define Template_PointerVerify(tPointer) JunoMemory_PointerVerifyType(tPointer, TEMPLATE_T, gtTemplateMsgPointerApi)
JUNO_STATUS_T Template_ArrayInit(TEMPLATE_ARRAY_T *ptTemplateMsgPipe, TEMPLATE_T *ptArrTemplateMsgBuffer, size_t iCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvUserData);

#ifdef __cplusplus
}
#endif
#endif

