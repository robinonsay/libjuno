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
#include "juno/log/log_api.h"
#include "template_app/template_app.h"
#include "juno/macros.h"
#include "juno/status.h"
#include <unistd.h>


static inline JUNO_STATUS_T Verify(JUNO_APP_T *ptJunoApp);


static JUNO_STATUS_T OnInit(JUNO_APP_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptJunoApp);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus)
    // Cast to the template app
    TEMPLATE_APP_T *ptTemplateApp = (TEMPLATE_APP_T *)(ptJunoApp);
    // Get the logger
    auto ptLogger = ptTemplateApp->ptLogger;
    // Get the logger api
    auto ptLoggerApi = reinterpret_cast<JUNO_LOG_ROOT_T *>(ptLogger)->ptApi;
    // Log that the app was intialized
    ptLoggerApi->LogInfo(ptLogger, "Template App Initialized");
    return tStatus;
}

static JUNO_STATUS_T OnProcess(JUNO_APP_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptJunoApp);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus)
    // Cast to the template app
    TEMPLATE_APP_T *ptTemplateApp = (TEMPLATE_APP_T *)(ptJunoApp);
    // Get the logger
    auto ptLogger = ptTemplateApp->ptLogger;
    // Get the logger api
    auto ptLoggerApi = reinterpret_cast<JUNO_LOG_ROOT_T *>(ptLogger)->ptApi;
    // Log that the app is running
    ptLoggerApi->LogDebug(ptLogger, "Template App Running");
    return tStatus;
}

static JUNO_STATUS_T OnExit(JUNO_APP_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptJunoApp);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus)
    // Cast to the template app
    TEMPLATE_APP_T *ptTemplateApp = (TEMPLATE_APP_T *)(ptJunoApp);
    // Get the logger
    auto ptLogger = ptTemplateApp->ptLogger;
    // Get the logger api
    auto ptLoggerApi = reinterpret_cast<JUNO_LOG_ROOT_T *>(ptLogger)->ptApi;
    // Log that the app is running
    ptLoggerApi->LogInfo(ptLogger, "Template App Exiting");
    return tStatus;
}

static const JUNO_APP_API_T tTemplateAppApi{
    .OnInit = OnInit,
    .OnProcess = OnProcess,
    .OnExit = OnExit
};

static inline JUNO_STATUS_T Verify(JUNO_APP_T *ptJunoApp)
{
    // Assert the pointer is not null
    JUNO_ASSERT_EXISTS(ptJunoApp);
    // Cast to the template app
    TEMPLATE_APP_T *ptTemplateApp = (TEMPLATE_APP_T *)(ptJunoApp);
    // Assert the module dependencies are present
    JUNO_JUNO_ASSERT_EXISTS_MODULE(
        /* TODO: Assert other dependencies and members here using &&*/
        ptTemplateApp &&
        ptTemplateApp->tRoot.ptApi,
        ptTemplateApp,
        "Module does not have all dependencies"
    );
    // Verify that this application is using the correct API
    if(ptTemplateApp->tRoot.ptApi != &tTemplateAppApi)
    {
        JUNO_FAIL_MODULE(JUNO_STATUS_INVALID_TYPE_ERROR, ptTemplateApp, "Module has invalid API");
        return JUNO_STATUS_INVALID_TYPE_ERROR;
    }
    return JUNO_STATUS_SUCCESS;
}

/* TODO: Insert initialization arguments for module members here*/
JUNO_STATUS_T TemplateApp(
    JUNO_APP_T *ptJunoApp,
    JUNO_LOG_T *ptLogger,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
)
{
    JUNO_ASSERT_EXISTS(ptJunoApp);
    TEMPLATE_APP_T *ptTemplateApp = (TEMPLATE_APP_T *)(ptJunoApp);
    ptTemplateApp->tRoot.ptApi = &tTemplateAppApi;
    ptTemplateApp->tRoot.JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    ptTemplateApp->tRoot.JUNO_FAILURE_USER_DATA = pvFailureUserData;
    JUNO_STATUS_T tStatus = Verify(ptJunoApp);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    ptTemplateApp->ptLogger = ptLogger;
    return tStatus;
}
