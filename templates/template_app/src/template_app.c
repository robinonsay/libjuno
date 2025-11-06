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
#include "template/template_app.h"
#include "juno/macros.h"
#include "juno/status.h"
#include <stdlib.h>
#include <unistd.h>

static inline JUNO_STATUS_T Verify(TEMPLATE_APP_T *ptTemplateApp);
static JUNO_STATUS_T OnStart(JUNO_APP_ROOT_T *ptJunoApp);
static JUNO_STATUS_T OnProcess(JUNO_APP_ROOT_T *ptJunoApp);
static JUNO_STATUS_T OnExit(JUNO_APP_ROOT_T *ptJunoApp);

static const JUNO_APP_API_T tTemplateAppApi = {
    .OnStart = OnStart,
    .OnProcess = OnProcess,
    .OnExit = OnExit
};

static inline JUNO_STATUS_T Verify(TEMPLATE_APP_T *ptTemplateApp)
{
    // Assert the pointer is not null
    JUNO_ASSERT_EXISTS(ptTemplateApp);
    // Cast to the template app
    // Assert the module dependencies are present
    JUNO_ASSERT_EXISTS_MODULE(
        /* TODO: Assert other dependencies and members here using &&*/
        ptTemplateApp &&
        ptTemplateApp->tRoot.ptApi &&
        ptTemplateApp->ptLogger,
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


JUNO_STATUS_T TemplateApp_Init(TEMPLATE_APP_T *ptTemplateApp, const JUNO_LOG_ROOT_T *ptLogger, JUNO_FAILURE_HANDLER_T pfcnFailureHandler, JUNO_USER_DATA_T *pvFailureUserData)
{
    JUNO_ASSERT_EXISTS(ptTemplateApp);
    ptTemplateApp->tRoot.ptApi = &tTemplateAppApi;
    ptTemplateApp->tRoot.JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    ptTemplateApp->tRoot.JUNO_FAILURE_USER_DATA = pvFailureUserData;
    ptTemplateApp->ptLogger = ptLogger;
    JUNO_STATUS_T tStatus = Verify(ptTemplateApp);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}

static JUNO_STATUS_T OnStart(JUNO_APP_ROOT_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    TEMPLATE_APP_T *ptTemplateApp = (TEMPLATE_APP_T *)(ptJunoApp);
    tStatus = Verify(ptTemplateApp);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus)
    // Cast to the template app
    // Get the logger
    const JUNO_LOG_ROOT_T *ptLogger = ptTemplateApp->ptLogger;
    // Get the logger api
    const JUNO_LOG_API_T *ptLoggerApi = ptLogger->ptApi;
    // Log that the app was intialized
    ptLoggerApi->LogInfo(ptLogger, "Template App Initialized");
    return tStatus;
}

static JUNO_STATUS_T OnProcess(JUNO_APP_ROOT_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    TEMPLATE_APP_T *ptTemplateApp = (TEMPLATE_APP_T *)(ptJunoApp);
    tStatus = Verify(ptTemplateApp);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus)
    // Cast to the template app
    // Get the logger
    const JUNO_LOG_ROOT_T *ptLogger = ptTemplateApp->ptLogger;
    // Get the logger api
    const JUNO_LOG_API_T *ptLoggerApi = ptLogger->ptApi;
    // Log that the app is running
    ptLoggerApi->LogDebug(ptLogger, "Template App Running");
    return tStatus;
}

static JUNO_STATUS_T OnExit(JUNO_APP_ROOT_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    TEMPLATE_APP_T *ptTemplateApp = (TEMPLATE_APP_T *)(ptJunoApp);
    tStatus = Verify(ptTemplateApp);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus)
    // Cast to the template app
    // Get the logger
    const JUNO_LOG_ROOT_T *ptLogger = ptTemplateApp->ptLogger;
    // Get the logger api
    const JUNO_LOG_API_T *ptLoggerApi = ptLogger->ptApi;
    // Log that the app is running
    ptLoggerApi->LogInfo(ptLogger, "Template App Exiting");
    return tStatus;
}
