#include "template/template_impl.h"
#include "juno/macros.h"
#include "juno/status.h"

static inline JUNO_STATUS_T Validate(TEMPLATE_T *ptTemplate)
{
    if(!ptTemplate)
    {
        return JUNO_STATUS_NULLPTR_ERROR;
    }
    /*
    
        TODO: Assert other dependencies and members here

    */
    ASSERT_EXISTS((ptTemplate->_ptPrivate));
    return JUNO_STATUS_SUCCESS;
}
static JUNO_STATUS_T Init(
    TEMPLATE_T *ptTemplate,
    TEMPLATE_PRIVATE_T *ptPrivate,
    /* TODO: Insert initialization arguments for module members here*/
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *_pvFailureUserData
)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    ptTemplate->_ptPrivate = ptPrivate;
    ptTemplate->_pfcnFailureHandler = pfcnFailureHandler;
    ptTemplate->_pvFailureUserData = _pvFailureUserData;
    /*
    
        TODO: Assign other member variables here

    */
    tStatus = Validate(ptTemplate);
    /*
    
        TODO: Initialize resources here

    */
    return tStatus;
}
static JUNO_STATUS_T Free(TEMPLATE_T *ptTemplate)
{
    JUNO_STATUS_T tStatus = Validate(ptTemplate);
    /*
    
        TODO: Free resources here

    */
    return tStatus;
}
static const TEMPLATE_API_T tTemplateApi = {
    .Init = Init,
    /*
        VERY IMPORTANT:
        Assign API implementations here.
        FAILING TO DO SO COULD RESULT IN UNDEFINED BEHAVIOR
    */
    .Free = Free
};

const TEMPLATE_API_T * Tempalte_ImplApi()
{
    return &tTemplateApi;
}
