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
#include "juno/sm/juno_sm.h"
#include "juno/macros.h"
#include "juno/status.h"
#include "juno/sm/sm_api.h"


static inline JUNO_STATUS_T Verify(JUNO_SM_T *ptJunoSm);


static JUNO_STATUS_T ExampleFunction(JUNO_SM_T *ptJunoSm)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptJunoSm);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus)
    // JUNO_SM_IMPL_T *ptJunoSmImpl = (JUNO_SM_IMPL_T *)(ptJunoSm);
    /*
    
    TODO: Initialize resources here
    
    */
    return tStatus;
}

static const JUNO_SM_API_T tJunoSmImplApi = {
    .ExampleFunction = ExampleFunction,
    /*
        VERY IMPORTANT:
        Assign API implementations here.
        FAILING TO DO SO COULD RESULT IN UNDEFINED BEHAVIOR
    */
};

static inline JUNO_STATUS_T Verify(JUNO_SM_T *ptJunoSm)
{
    JUNO_ASSERT_EXISTS(ptJunoSm);
    JUNO_SM_IMPL_T *ptJunoSmImpl = (JUNO_SM_IMPL_T *)(ptJunoSm);
    JUNO_JUNO_ASSERT_EXISTS_MODULE(
        ptJunoSm && ptJunoSmImpl->tRoot.ptApi
        /* TODO: Assert other dependencies and members here using &&*/,
        ptJunoSmImpl,
        "Module does not have all dependencies"
    );
    if(ptJunoSmImpl->tRoot.ptApi != &tJunoSmImplApi)
    {
        JUNO_FAIL_MODULE(JUNO_STATUS_INVALID_TYPE_ERROR, ptJunoSmImpl, "Module has invalid API");
        return JUNO_STATUS_INVALID_TYPE_ERROR;
    }
    return JUNO_STATUS_SUCCESS;
}

/* TODO: Insert initialization arguments for module members here*/
JUNO_STATUS_T JunoSm_ImplApi(JUNO_SM_T *ptJunoSm, JUNO_FAILURE_HANDLER_T pfcnFailureHandler, JUNO_USER_DATA_T *pvFailureUserData)
{
    JUNO_ASSERT_EXISTS(ptJunoSm);
    JUNO_SM_IMPL_T *ptJunoSmImpl = (JUNO_SM_IMPL_T *)(ptJunoSm);
    ptJunoSmImpl->tRoot.ptApi = &tJunoSmImplApi;
    ptJunoSmImpl->tRoot.JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    ptJunoSmImpl->tRoot.JUNO_FAILURE_USER_DATA = pvFailureUserData;
    JUNO_STATUS_T tStatus = Verify(ptJunoSm);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    /*
    
    TODO: Assign private members here
    
    */
    return tStatus;
}
