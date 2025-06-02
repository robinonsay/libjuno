#include "template/template_impl.h"
#include "juno/macros.h"
#include "juno/status.h"

static inline JUNO_STATUS_T Validate(TEMPLATE_IMPL_T *ptImpl)
{
    ASSERT_EXISTS((ptImpl /* TODO: Assert other dependencies and members here using &&*/));
    return JUNO_STATUS_SUCCESS;
}
static JUNO_STATUS_T Init(
    TEMPLATE_T *ptTemplate,
    /* TODO: Insert initialization arguments for module members here*/
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *_pvFailureUserData
)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    TEMPLATE_IMPL_T *ptImpl = (TEMPLATE_IMPL_T *)(ptTemplate);
    ptTemplate->JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    ptTemplate->JUNO_FAILURE_USER_DATA = _pvFailureUserData;
    /*
    
        TODO: Assign other member variables here

    */
    tStatus = Validate(ptImpl);
    /*
    
        TODO: Initialize resources here

    */
    return tStatus;
}
static JUNO_STATUS_T Free(TEMPLATE_T *ptTemplate)
{

    TEMPLATE_IMPL_T *ptImpl = (TEMPLATE_IMPL_T *)(ptTemplate);
    JUNO_STATUS_T tStatus = Validate(ptImpl);
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
