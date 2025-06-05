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


JUNO_MODULE_DERIVE_DECLARE(TEMPLATE_IMPL_T);

JUNO_MODULE_DERIVE(TEMPLATE_IMPL_T, TEMPLATE_BASE_T,
    /*
    
    TODO: Include implementation specific members here
    
    */
);

#ifndef TEMPLATE_DERIVED
/**
    This is the default implementation for `TEMPLATE_T`.
    If you want to derive new implementations for `TEMPLATE_T`
    use `#define TEMPLATE_DERIVED` prior to including
    `#include "template_impl.h"`
*/
JUNO_MODULE(TEMPLATE_T, TEMPLATE_BASE_T,
    TEMPLATE_IMPL_T tTemplateImpl;
);
#endif

/* TODO: Insert initialization arguments for module members here*/
JUNO_STATUS_T Template_ImplApi(TEMPLATE_T *ptTemplateImpl, JUNO_FAILURE_HANDLER_T pfcnFailureHandler, JUNO_USER_DATA_T *pvFailureUserData);
#ifdef __cplusplus
}
#endif
#endif // TEMPLATE_IMPL_H

