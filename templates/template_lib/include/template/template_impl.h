/**
    This header contains the template impl implementation
    @author
*/
#ifndef TEMPLATE_IMPL_H
#define TEMPLATE_IMPL_H
#include "juno/module.h"
#include "juno/status.h"
#include "template_api.h"
#ifdef __cplusplus
extern "C"
{
#endif

JUNO_MODULE_DECLARE(TEMPLATE_IMPL_T);

JUNO_MODULE_DERIVE(TEMPLATE_IMPL_T, TEMPLATE_T,
/*

    TODO: Include implementation specific members here

*/
);

/* TODO: Insert initialization arguments for module members here*/
JUNO_STATUS_T Template_ImplApi(TEMPLATE_IMPL_T *ptTemplateImpl, JUNO_FAILURE_HANDLER_T pfcnFailureHandler, JUNO_USER_DATA_T *pvFailureUserData);
#ifdef __cplusplus
}
#endif
#endif // TEMPLATE_IMPL_H

