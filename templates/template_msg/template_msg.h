#ifndef TEMPLATE_MSG_H
#define TEMPLATE_MSG_H
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

extern const JUNO_POINTER_API_T gtTemplateMsgPointerApi;

#define TemplateMsg_PointerInit(addr) JunoMemory_PointerInit(&gtTemplateMsgPointerApi, TEMPLATE_MSG_T, addr)

#ifdef __cplusplus
}
#endif
#endif

