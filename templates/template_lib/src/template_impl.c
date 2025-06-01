#include "template/template_impl.h"
#include "juno/macros.h"
#include "juno/memory/memory.h"
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
    ASSERT_EXISTS((ptTemplate->_ptPrivate && ptTemplate->_ptAlloc && ptTemplate->_tPrivateMemory.pvAddr));
    return JUNO_STATUS_SUCCESS;
}
static JUNO_STATUS_T Init(
    TEMPLATE_T *ptTemplate,
    JUNO_MEMORY_ALLOC_T *ptAlloc,
    /* TODO: Insert initialization arguments for module members here*/
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *_pvFailureUserData
)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Juno_MemoryGet(ptAlloc, &ptTemplate->_tPrivateMemory, sizeof(TEMPLATE_PRIVATE_T));
    if(tStatus) return tStatus;
    ptTemplate->ptPrivate = (TEMPLATE_PRIVATE_T *)(ptTemplate->_tPrivateMemory.pvAddr);
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
    tStatus = Juno_MemoryPut(ptTemplate->_ptAlloc, &ptTemplate->_tPrivateMemory);
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
