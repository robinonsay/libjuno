#include "template/template_impl.h"
#include "juno/macros.h"
#include "juno/status.h"
#include "template/template_api.h"

static const TEMPLATE_API_T tTemplateImplApi;

static inline JUNO_STATUS_T Verify(TEMPLATE_T *ptTemplate)
{
    ASSERT_EXISTS(ptTemplate);
    ASSERT_EXISTS_MODULE(
        ptTemplate && ptTemplate->ptApi
        /* TODO: Assert other dependencies and members here using &&*/,
        ptTemplate,
        "Module does not have all dependencies"
    );
    if(ptTemplate->ptApi != &tTemplateImplApi)
    {
        FAIL_MODULE(JUNO_STATUS_INVALID_TYPE_ERROR, ptTemplate, "Module has invalid API");
        return JUNO_STATUS_INVALID_TYPE_ERROR;
    }
    return JUNO_STATUS_SUCCESS;
}


static inline JUNO_STATUS_T VerifyImpl(TEMPLATE_IMPL_T *ptTemplateImpl)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    ASSERT_EXISTS(ptTemplateImpl);
    /*
    
    TODO: Assert other validation items for implementation

    */
    return tStatus;
}

static JUNO_STATUS_T Init(
    TEMPLATE_T *ptTemplate
    /* TODO: Insert initialization arguments for module members here*/
)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptTemplate);
    ASSERT_SUCCESS(tStatus, return tStatus)
    TEMPLATE_IMPL_T *ptTemplateImpl = (TEMPLATE_IMPL_T *)(ptTemplate);

    /*
    
    TODO: Initialize resources here
    
    */
    tStatus = VerifyImpl(ptTemplateImpl);
    ASSERT_SUCCESS(tStatus, return tStatus)
    return tStatus;
}
static JUNO_STATUS_T Free(TEMPLATE_T *ptTemplate)
{

    JUNO_STATUS_T tStatus = Verify(ptTemplate);
    ASSERT_SUCCESS(tStatus, return tStatus)
    TEMPLATE_IMPL_T *ptTemplateImpl = (TEMPLATE_IMPL_T *)(ptTemplate);
    tStatus = VerifyImpl(ptTemplateImpl);
    ASSERT_SUCCESS(tStatus, return tStatus)
    /*
    
        TODO: Free resources here

    */
    return tStatus;
}
static const TEMPLATE_API_T tTemplateImplApi = {
    .Init = Init,
    /*
        VERY IMPORTANT:
        Assign API implementations here.
        FAILING TO DO SO COULD RESULT IN UNDEFINED BEHAVIOR
    */
    .Free = Free
};

/* TODO: Insert initialization arguments for module members here*/
JUNO_STATUS_T Template_ImplApi(TEMPLATE_IMPL_T *ptTemplateImpl, JUNO_FAILURE_HANDLER_T pfcnFailureHandler, JUNO_USER_DATA_T *pvFailureUserData)
{
    ASSERT_EXISTS(ptTemplateImpl);
    TEMPLATE_T *ptSuper = &ptTemplateImpl->tSuper;
    ptSuper->ptApi = &tTemplateImplApi;
    ptSuper->JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    ptSuper->JUNO_FAILURE_USER_DATA = pvFailureUserData;
    JUNO_STATUS_T tStatus = Verify(ptSuper);
    ASSERT_SUCCESS(tStatus, return tStatus);
    /*
    
    TODO: Assign private members here
    
    */
    return tStatus;
}
