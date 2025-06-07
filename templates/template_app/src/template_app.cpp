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
#include "template_app/template_app_api.h"
#include "juno/macros.h"
#include "juno/status.h"
#include "template_app/template_app_api.h"
#include <iostream>
#include <ostream>
#include <unistd.h>


static inline JUNO_STATUS_T Verify(JUNO_APP_T *ptJunoApp);


static JUNO_STATUS_T Init(JUNO_APP_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptJunoApp);
    ASSERT_SUCCESS(tStatus, return tStatus)
    // TEMPLATE_APP_T *ptTemplateApp = (TEMPLATE_APP_T *)(ptJunoApp);
    std::cout << "Hello World Init" << std::endl;
    return tStatus;
}

static JUNO_STATUS_T Run(JUNO_APP_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptJunoApp);
    ASSERT_SUCCESS(tStatus, return tStatus)
    // TEMPLATE_APP_T *ptTemplateApp = (TEMPLATE_APP_T *)(ptJunoApp);
    /*
    
    TODO: Initialize resources here
    
    */
    std::cout << "Hello World Run" << std::endl;
    sleep(1);
    return tStatus;
}

static JUNO_STATUS_T Exit(JUNO_APP_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptJunoApp);
    ASSERT_SUCCESS(tStatus, return tStatus)
    // TEMPLATE_APP_T *ptTemplateApp = (TEMPLATE_APP_T *)(ptJunoApp);
    /*
    
    TODO: Initialize resources here
    
    */
    std::cout << "Hello World Exit" << std::endl;
    return tStatus;
}

static const JUNO_APP_API_T tTemplateAppApi{
    .Init = Init,
    .Run = Run,
    .Exit = Exit
};

static inline JUNO_STATUS_T Verify(JUNO_APP_T *ptJunoApp)
{
    ASSERT_EXISTS(ptJunoApp);
    TEMPLATE_APP_T *ptTemplateApp = (TEMPLATE_APP_T *)(ptJunoApp);
    ASSERT_EXISTS_MODULE(
        ptTemplateApp && ptTemplateApp->JUNO_MODULE_SUPER.ptApi
        /* TODO: Assert other dependencies and members here using &&*/,
        ptTemplateApp,
        "Module does not have all dependencies"
    );
    if(ptTemplateApp->JUNO_MODULE_SUPER.ptApi != &tTemplateAppApi)
    {
        FAIL_MODULE(JUNO_STATUS_INVALID_TYPE_ERROR, ptTemplateApp, "Module has invalid API");
        return JUNO_STATUS_INVALID_TYPE_ERROR;
    }
    return JUNO_STATUS_SUCCESS;
}

/* TODO: Insert initialization arguments for module members here*/
JUNO_STATUS_T JunoFsw_TemplateApp(JUNO_APP_T *ptJunoApp, JUNO_FAILURE_HANDLER_T pfcnFailureHandler, JUNO_USER_DATA_T *pvFailureUserData)
{
    ASSERT_EXISTS(ptJunoApp);
    TEMPLATE_APP_T *ptTemplateApp = (TEMPLATE_APP_T *)(ptJunoApp);
    ptTemplateApp->JUNO_MODULE_SUPER.ptApi = &tTemplateAppApi;
    ptTemplateApp->JUNO_MODULE_SUPER.JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    ptTemplateApp->JUNO_MODULE_SUPER.JUNO_FAILURE_USER_DATA = pvFailureUserData;
    JUNO_STATUS_T tStatus = Verify(ptJunoApp);
    ASSERT_SUCCESS(tStatus, return tStatus);
    /*
    
    TODO: Assign private members here
    
    */
    return tStatus;
}
