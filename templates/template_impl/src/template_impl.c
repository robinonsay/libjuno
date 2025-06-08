/*
    MIT License

    Copyright (c) 2025 Robin A. Onsay

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.
*/
#include "template/template_impl.h"
#include "juno/macros.h"
#include "juno/status.h"
#include "template/template_api.h"


static inline JUNO_STATUS_T Verify(TEMPLATE_T *ptTemplate);


static JUNO_STATUS_T ExampleFunction(TEMPLATE_T *ptTemplate)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptTemplate);
    ASSERT_SUCCESS(tStatus, return tStatus)
    TEMPLATE_IMPL_T *ptTemplateImpl = (TEMPLATE_IMPL_T *)(ptTemplate);
    /*
    
    TODO: Initialize resources here
    
    */
    return tStatus;
}

static const TEMPLATE_API_T tTemplateImplApi = {
    .ExampleFunction = ExampleFunction,
    /*
        VERY IMPORTANT:
        Assign API implementations here.
        FAILING TO DO SO COULD RESULT IN UNDEFINED BEHAVIOR
    */
};

static inline JUNO_STATUS_T Verify(TEMPLATE_T *ptTemplate)
{
    ASSERT_EXISTS(ptTemplate);
    TEMPLATE_IMPL_T *ptTemplateImpl = (TEMPLATE_IMPL_T *)(ptTemplate);
    ASSERT_EXISTS_MODULE(
        ptTemplate && ptTemplateImpl->JUNO_MODULE_SUPER.ptApi
        /* TODO: Assert other dependencies and members here using &&*/,
        ptTemplateImpl,
        "Module does not have all dependencies"
    );
    if(ptTemplateImpl->JUNO_MODULE_SUPER.ptApi != &tTemplateImplApi)
    {
        FAIL_MODULE(JUNO_STATUS_INVALID_TYPE_ERROR, ptTemplateImpl, "Module has invalid API");
        return JUNO_STATUS_INVALID_TYPE_ERROR;
    }
    return JUNO_STATUS_SUCCESS;
}

/* TODO: Insert initialization arguments for module members here*/
JUNO_STATUS_T Template_ImplApi(TEMPLATE_T *ptTemplate, JUNO_FAILURE_HANDLER_T pfcnFailureHandler, JUNO_USER_DATA_T *pvFailureUserData)
{
    ASSERT_EXISTS(ptTemplate);
    TEMPLATE_IMPL_T *ptTemplateImpl = (TEMPLATE_IMPL_T *)(ptTemplate);
    ptTemplateImpl->JUNO_MODULE_SUPER.ptApi = &tTemplateImplApi;
    ptTemplateImpl->JUNO_MODULE_SUPER.JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    ptTemplateImpl->JUNO_MODULE_SUPER.JUNO_FAILURE_USER_DATA = pvFailureUserData;
    JUNO_STATUS_T tStatus = Verify(ptTemplate);
    ASSERT_SUCCESS(tStatus, return tStatus);
    /*
    
    TODO: Assign private members here
    
    */
    return tStatus;
}
