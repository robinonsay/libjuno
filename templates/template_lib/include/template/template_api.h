/**
    This header contains the template library API
    @author
*/
#ifndef TEMPLATE_API_H
#define TEMPLATE_API_H
#include "juno/status.h"
#include "juno/module.h"
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct TEMPLATE_API_TAG TEMPLATE_API_T;
JUNO_MODULE_DECLARE(TEMPLATE_T);

JUNO_MODULE(TEMPLATE_T, TEMPLATE_API_T,
    /*
    
        TODO: Add implementation independent member variables here
    
    */
);

struct TEMPLATE_API_TAG
{
    /// Initializes the module and resources for template
    JUNO_STATUS_T (*Init)(
        TEMPLATE_T *ptTemplate
        /* TODO: Insert initialization arguments for module members here*/
    );
    /// Frees resources allocated by template
    JUNO_STATUS_T (*Free)(TEMPLATE_T *ptTemplate);
};

#ifdef __cplusplus
}
#endif
#endif // TEMPLATE_API_H
