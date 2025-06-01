/**
    This header contains the template library API
    @author
*/
#ifndef TEMPLATE_API_H
#define TEMPLATE_API_H
#include "juno/status.h"
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct TEMPLATE_API_TAG TEMPLATE_API_T;
typedef struct TEMPLATE_TAG TEMPLATE_T;
typedef struct TEMPLATE_PRIVATE_TAG TEMPLATE_PRIVATE_T;

struct TEMPLATE_TAG
{
    TEMPLATE_PRIVATE_T *_ptPrivate;
    /*
    
        TODO: Add implementation independent member variables here

    */
    JUNO_FAILURE_HANDLER_T _pfcnFailureHandler;
    JUNO_USER_DATA_T *_pvFailureUserData;
};

struct TEMPLATE_API_TAG
{
    /// Initializes the module and resources for template
    JUNO_STATUS_T (*Init)(
        TEMPLATE_T *ptTemplate,
        TEMPLATE_PRIVATE_T *ptPrivate,
        /* TODO: Insert initialization arguments for module members here*/
        JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
        JUNO_USER_DATA_T *_pvFailureUserData
    );
    /// Frees resources allocated by template
    JUNO_STATUS_T (*Free)(TEMPLATE_T *ptTemplate);
};

#ifdef __cplusplus
}
#endif
#endif // TEMPLATE_API_H
