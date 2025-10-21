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

#include "juno/app/app_api.h"
#include "juno/log/log_api.h"
#include "juno/sb/broker_api.h"
#include "juno/time/time_api.h"
#include "engine_app/engine_app.h"
#include "juno/macros.h"
#include "juno/status.h"
#include "system_manager_app/system_manager_app.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>

/**DOC
The purpose of this tutorial is to show new users how to use LibJuno as a
embedded software micro-framework. LibJuno is intended to provide developers and
software architects the freedom to implement their software system according
to their use case, instead of users conforming to our library.

This means users will need to define their own entry point. On systems with an OS,
this would be a main function.

Additionally, users will need to implement project specific functions like logging and
time management since LibJuno doesn't assume your OS or time use-case.

# Logging

In this case, we are going to implement the logging functions using
`printf`. A fixed size buffer is utilized with `vsnprintf` to ensure
the logger doesn't see any memory overruns.
*/
static JUNO_STATUS_T LogDebug(const JUNO_LOG_ROOT_T *ptJunoLog, const char *pcMsg, ...)
{
    (void) ptJunoLog;
    va_list tVarg;
    va_start(tVarg, pcMsg);
    char pcPrintStr[512] = {0};
    vsnprintf(pcPrintStr, sizeof(pcPrintStr), pcMsg, tVarg);
    printf("DEBUG: %s\n", pcPrintStr);
    va_end(tVarg);
    return JUNO_STATUS_SUCCESS;
}
static JUNO_STATUS_T LogInfo(const JUNO_LOG_ROOT_T *ptJunoLog, const char *pcMsg, ...)
{
    (void) ptJunoLog;
    va_list tVarg;
    va_start(tVarg, pcMsg);
    char pcPrintStr[512] = {0};
    vsnprintf(pcPrintStr, sizeof(pcPrintStr), pcMsg, tVarg);
    printf("INFO: %s\n", pcPrintStr);
    va_end(tVarg);
    return JUNO_STATUS_SUCCESS;
}
static JUNO_STATUS_T LogWarning(const JUNO_LOG_ROOT_T *ptJunoLog, const char *pcMsg, ...)
{
    (void) ptJunoLog;
    va_list tVarg;
    va_start(tVarg, pcMsg);
    char pcPrintStr[512] = {0};
    vsnprintf(pcPrintStr, sizeof(pcPrintStr), pcMsg, tVarg);
    printf("WARN: %s\n", pcPrintStr);
    va_end(tVarg);
    return JUNO_STATUS_SUCCESS;
}
static JUNO_STATUS_T LogError(const JUNO_LOG_ROOT_T *ptJunoLog, const char *pcMsg, ...)
{
    (void) ptJunoLog;
    va_list tVarg;
    va_start(tVarg, pcMsg);
    char pcPrintStr[512] = {0};
    vsnprintf(pcPrintStr, sizeof(pcPrintStr), pcMsg, tVarg);
    printf("ERROR: %s\n", pcPrintStr);
    va_end(tVarg);
    return JUNO_STATUS_SUCCESS;
}

/**DOC
# Time
In this example project we will need a timestamp, so we'll implement the `Now` function.
We don't need `Sleep` or `SleepTo` so we'll provide mocks to those functions. LibJuno
provides time math functions that we can utilize so we only need to provide implementations
to these three time functions.
*/
/// Get the current time as specified by the implementation
static JUNO_TIMESTAMP_RESULT_T Now(const JUNO_TIME_ROOT_T *ptTime)
{
    JUNO_TIMESTAMP_RESULT_T tResult = {0};
    if(!ptTime)
    {
        tResult.tStatus = JUNO_STATUS_NULLPTR_ERROR;
        return tResult;
    }
    struct timespec tTime = {0};
    clock_gettime(CLOCK_MONOTONIC, &tTime);
    tResult = ptTime->ptApi->NanosToTimestamp(ptTime, tTime.tv_nsec);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    tResult.tOk.iSeconds = tTime.tv_sec;
    return tResult;
}
/// Sleep this thread until a specific time
static JUNO_STATUS_T SleepTo(const JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T tTimeToWakeup)
{
    (void) ptTime;
    (void) tTimeToWakeup;
    return JUNO_STATUS_SUCCESS;
}
/// Sleep this thread for a duration
static JUNO_STATUS_T Sleep(const JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T tDuration)
{
    (void) ptTime;
    (void) tDuration;
    return JUNO_STATUS_SUCCESS;
}

/**DOC
# Instantiating APIs
LibJuno utilizes a vtable, or a table of function pointers to the specific implementation, that
is passed to the LibJuno module. This vtable is called an "API" in LibJuno nomenclature and it
provides a standard interface to the capabilities. LibJuno is implemented as a set of capabilities, or
software components that perform a set functionality. These capabilities are called "Modules" and can
be extened through derivation. Additionally APIs can be extended through derivation. When a module or API
is extended its called "derived".

Below is tha API instantiation of the logger and time APIs for this project. We are providing our logging
implementations and time implementations. You'll notice that the time API provides a helper macro to instantiate
the API. Some APIs offer existing implementations, so a helper macro is used to inform users which functions they
need to implement.
*/
static const JUNO_LOG_API_T gtMyLoggerApi ={
    LogDebug,
    LogInfo,
    LogWarning,
    LogError
};

static const JUNO_TIME_API_T gtTimeApi = JunoTime_TimeApiInit(Now, SleepTo, Sleep);

/**DOC
# Failure Handler
LibJuno utilizes a failure handler callback. This is a function that is automatically called by
downstream code when a failure occurs. In an embedded system, you might not have a terminal or console
log running. This enables developers to have a single implementation and methodology for handling failure
within the software. The `JUNO_FAIL...` macros will automatically call the failure handler if one is
provided.
*/
void FailureHandler(JUNO_STATUS_T tStatus, const char *pcMsg, JUNO_USER_DATA_T *pvUserData)
{
    (void) pvUserData;
    printf("FAILURE: %u | %s\n", tStatus, pcMsg);
}

/**DOC
# The Entry Point
This example project assumes its running on a POSIX system. Therefore,
we can use a main method as the entry point. Many microcontrollers have
different architectures for their entry point so you'll need to consult with
your architecture documentation on how to implement the entry point.
*/
int main(void)
{
/**DOC
# Dependency Injection in Action
The core principle of dependency injection is "inversion of control".
This is a fancy term for saying that modules don't allocate resources,
up stream users do. This is done at the "composition root", or the spot
in the code where all the dependencies get implemented. In this case,
the main function is the composition root. Here we need to instantiate
the time, logger, registry modules.

Applications in LibJuno are actually derived modules of `JUNO_APP_ROOT_T`.
Here we will need to instantiate the engine, and system manager application modules.
*/
    JUNO_TIME_ROOT_T tTime = {0};
    JUNO_LOG_ROOT_T tLogger = {0};
    JUNO_SB_PIPE_T *tRegistry[10] = {0};
    JUNO_SB_BROKER_ROOT_T tBroker = {0};
    ENGINE_APP_T tEngineApp = {0};
    SYSTEM_MANAGER_APP_T tSystemManagerApp = {0};
/**DOC
# Module Initialization

Modules typically have an `Init` function that specifies the required fields to initialize a module.
Modules also utilize a `Verify` function to verify they've been initialzed. Most modules will return
a `JUNO_STATUS_NULLPTR_ERROR` if they have not been initalized and a function has been called on them.
*/
    JUNO_STATUS_T tStatus = JunoTime_TimeInit(&tTime, &gtTimeApi, FailureHandler, NULL);
    JUNO_ASSERT_SUCCESS(tStatus, return -1);
    tStatus = JunoLog_LogInit(&tLogger, &gtMyLoggerApi, FailureHandler, NULL);
    JUNO_ASSERT_SUCCESS(tStatus, return -1);
/**DOC
Notice how the broker, engine, and system manager take initialized modules as dependencies to their initialization.
This is DI in action. For example, the engine app is provided with a logger, time, and broker instead of instantiating it
itself.
*/
    tStatus = JunoSb_BrokerInit(&tBroker, tRegistry, 10, FailureHandler, NULL);
    JUNO_ASSERT_SUCCESS(tStatus, return -1);
    tStatus = EngineApp_Init(&tEngineApp, &tLogger, &tTime, &tBroker, FailureHandler, NULL);
    JUNO_ASSERT_SUCCESS(tStatus, return -1);
    tStatus = SystemManagerApp(&tSystemManagerApp, &tLogger, &tTime, &tBroker, FailureHandler, NULL);
    JUNO_ASSERT_SUCCESS(tStatus, return -1);
/**DOC
# Runtime
For this example we are going to create a very simple "schedule table" or table of applications that
will run in a specific order. This schedule table will run at best-effort. This means that when one
application is done, the next will start.
*/
    JUNO_APP_ROOT_T *ptAppList[2] = {
        &tSystemManagerApp.tRoot,
        &tEngineApp.tRoot
    };
/**DOC
Since all applications have the same interface, we can run their init function in a for-loop
and run the application `OnProcess` function in a `while(true)` loop.
*/
    for(size_t i = 0; i < 2; i++)
    {
        tStatus = ptAppList[i]->ptApi->OnInit(ptAppList[i]);
        JUNO_ASSERT_SUCCESS(tStatus, return -1);
    }
    size_t iCounter = 0;
    while(true)
    {
        ptAppList[iCounter]->ptApi->OnProcess(ptAppList[iCounter]);
        iCounter = (iCounter + 1) % 2;
    }
}
