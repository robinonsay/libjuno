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
#include "engine_app/engine_cmd_msg.h"
#include "engine_app/engine_tlm_msg.h"
#include "juno/ds/queue_api.h"
#include "juno/log/log_api.h"
#include "engine_app/engine_app.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include "juno/sb/broker_api.h"
#include "juno/status.h"
#include "juno/time/time_api.h"
#include <stdlib.h>
#include <unistd.h>

/**DOC
Next we need to implement the application. `engine_app.c` will act as a
composition root for application specific logic. This means that the application can
own some dependencies, like its message buffers and pipes. The line between
dependencies an application should own and dependencies the `main.c` root should
own is fuzzy. It depends on developers specific use cases. In general,
items like the pipes, memory buffers, and application specific dependencies should
be owned by the application. Dependencies that reach to more than one application
should be owned by the project composition root (`main.c`).

### Application Api

As stated previously, LibJuno applications are derived from the `JUNO_APP_ROOT_T`.
This means we need to implement the API defined by the root module.
We forward declare the api functions and the verification function so we can check
and verify that the pointer being passed to our function is in fact a
`ENGINE_APP_T` type by asserting that the api pointers match.
*/

static inline JUNO_STATUS_T Verify(JUNO_APP_ROOT_T *ptJunoApp);
static JUNO_STATUS_T OnStart(JUNO_APP_ROOT_T *ptJunoApp);
static JUNO_STATUS_T OnProcess(JUNO_APP_ROOT_T *ptJunoApp);
static JUNO_STATUS_T OnExit(JUNO_APP_ROOT_T *ptJunoApp);

static const JUNO_APP_API_T tEngineAppApi = {
    .OnStart = OnStart,
    .OnProcess = OnProcess,
    .OnExit = OnExit
};

/**DOC
### Verification Function
The verification function is called as the one of the very first lines in
a LibJuno API function. Its purpose is to assert that the module contract
is met. This means that pointers are initialized, and the APIs match.

In LibJuno we can use `JUNO_ASSERT` macros that will check for certain
common conditions like null pointers. These macros will automatically
return `JUNO_STATUS_ERR` or `JUNO_STATUS_NULLPTR_ERROR` on failure.
*/
static inline JUNO_STATUS_T Verify(JUNO_APP_ROOT_T *ptJunoApp)
{
    // Assert the pointer is not null
    JUNO_ASSERT_EXISTS(ptJunoApp);
    // Cast to the engine app
    ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp);
    // Assert the module dependencies are present
    JUNO_ASSERT_EXISTS_MODULE(
        /* TODO: Assert other dependencies and members here using &&*/
        ptEngineApp &&
        ptEngineApp->tRoot.ptApi &&
        ptEngineApp->ptLogger &&
        ptEngineApp->ptTime &&
        ptEngineApp->ptBroker,
        ptEngineApp,
        "Module does not have all dependencies"
    );
    // Verify that this application is using the correct API
    if(ptEngineApp->tRoot.ptApi != &tEngineAppApi)
    {
        JUNO_FAIL_MODULE(JUNO_STATUS_INVALID_TYPE_ERROR, ptEngineApp, "Module has invalid API");
        return JUNO_STATUS_INVALID_TYPE_ERROR;
    }
    return JUNO_STATUS_SUCCESS;
}

/**DOC
### The Application Init Function
The init function's primary responsibility is to assign dependencies. This function
enables the `main.c` project composition root to pass application dependencies to the application.
This function is a concrete implementation because it depends entirely on application specifics.

This function asserts the module pointer exists, assigns the dependency pointers, and finally
calls the verification function. The verification function is called last because it assumes
the pointers have been instantiated.
*/
JUNO_STATUS_T EngineApp_Init(ENGINE_APP_T *ptJunoApp, const JUNO_LOG_ROOT_T *ptLogger, const JUNO_TIME_ROOT_T *ptTime, JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_FAILURE_HANDLER_T pfcnFailureHandler, JUNO_USER_DATA_T *pvFailureUserData)
{
    JUNO_ASSERT_EXISTS(ptJunoApp);
    ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp);
    ptEngineApp->tRoot.ptApi = &tEngineAppApi;
    ptEngineApp->tRoot.JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    ptEngineApp->tRoot.JUNO_FAILURE_USER_DATA = pvFailureUserData;
    ptEngineApp->ptLogger = ptLogger;
    ptEngineApp->ptTime = ptTime;
    ptEngineApp->ptBroker = ptBroker;
    JUNO_STATUS_T tStatus = Verify(&ptJunoApp->tRoot);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}

/**DOC
### `OnStart`
`OnStart` is the very first function called in a LibJuno project and is
typically called only once. The primary purpose of `OnStart` is to
initialize state variables, perform initialization tasks specific to the
application, and allocate global application resources.
*/
static JUNO_STATUS_T OnStart(JUNO_APP_ROOT_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptJunoApp);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus)
    // Cast to the engine app
    ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp);
    // Get the logger
    const JUNO_LOG_ROOT_T *ptLogger = ptEngineApp->ptLogger;
    // Get the logger api
    const JUNO_LOG_API_T *ptLoggerApi = ptLogger->ptApi;
    // Log that the app was intialized
    ptLoggerApi->LogInfo(ptLogger, "Engine App Initialized");
    // Initialize the cmd pipe
/**DOC
Within `OnStart` we will initialize our command message pipe so we can receive
commands. We will also register this pipe with the broker so it can be active.
*/
    tStatus = EngineCmdMsg_PipeInit(&ptEngineApp->tCmdPipe, ptEngineApp->ptArrCmdBuffer, ENGINE_CMD_MSG_PIPE_DEPTH, ptJunoApp->_pfcnFailureHandler, ptJunoApp->_pvFailureUserData);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Register the subscriber
    tStatus = ptEngineApp->ptBroker->ptApi->RegisterSubscriber(ptEngineApp->ptBroker, &ptEngineApp->tCmdPipe.tRoot);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
/**DOC
For this example we will also set the current RPM to 0
*/
    // Initialize current RPM to 0
    ptEngineApp->fCurrentRpm = 0.0;
    return tStatus;
}

/**DOC
### `OnProcess`
OnProcess is the main run-loop function of the application. It is called on a periodic
basis, typically at a set frequency (like 1Hz, 5Hz, 50Hz, 100Hz, etc).

This is where the main functionality of the application will reside.
*/
static JUNO_STATUS_T OnProcess(JUNO_APP_ROOT_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptJunoApp);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus)
    // Cast to the engine app
    ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp);
    // Get the logger
    const JUNO_LOG_ROOT_T *ptLogger = ptEngineApp->ptLogger;
    // Get the logger api
    const JUNO_LOG_API_T *ptLoggerApi = ptLogger->ptApi;
    // Get the broker
    JUNO_SB_BROKER_ROOT_T *ptBroker = ptEngineApp->ptBroker;
    // Get the time api
    const JUNO_TIME_ROOT_T *ptTime = ptEngineApp->ptTime;
    // Log that the app is running
    ptLoggerApi->LogDebug(ptLogger, "Engine App Running");
/**DOC
In this example we will sleep the application for 500ms to simulate
extensive work being done within the application. We will also
create a convenience variable to access the pipe's queue API.
*/
    // sleep for half a second
    usleep(500E3);
    const JUNO_DS_QUEUE_API_T *ptCmdPipeApi = ptEngineApp->tCmdPipe.ptApi;
/**DOC
For this example we will allocate a buffer to attempt to receive a command
from the message pipe. We will then initialize an engine command pointer with
the address to our allocated buffer.
*/
    ENGINE_CMD_MSG_T tEngineCmd = {0};
    JUNO_POINTER_T tEngineCmdPointer = EngineCmdMsg_PointerInit(&tEngineCmd);
/**DOC
We will then get the current timestamp using the time api provided by the
project root. 
*/
    JUNO_TIMESTAMP_RESULT_T tTimestampResult = ptTime->ptApi->Now(ptTime);
    JUNO_ASSERT_SUCCESS(tTimestampResult.tStatus, return tTimestampResult.tStatus);
/**DOC
Next we initialize a telemetry message with the current RPM and timestamp. This is done at this
step because in the case of an error, we will jump to the `exit` point with a `goto` statement
so we always publish this telemetry.
*/
    ENGINE_TLM_MSG_T tEngineTlm = {ptEngineApp->fCurrentRpm, tTimestampResult.tOk};
    JUNO_POINTER_T tEngineTlmPointer = EngineTlmMsg_PointerInit(&tEngineTlm);
/**DOC
We will calculate some artificial noise to simulate an actual sensor measurement.
*/
    float noise = rand();
    noise = noise - RAND_MAX/2;
    noise = 2 * noise / RAND_MAX;
/**DOC
Next we will attempt to dequeue a command from the software bus. This operation will fail
if there is no command in the pipe and will jump to the exit point of this function.
*/
    tStatus = ptCmdPipeApi->Dequeue(&ptEngineApp->tCmdPipe.tRoot.tRoot, tEngineCmdPointer);
    JUNO_ASSERT_SUCCESS(tStatus, goto exit);
/*DOC
Because we has a `JUNO_ASSERT_SUCCESS` on the previous function, we know that at this point
in the software the `dequeue` succeeded and the command buffer we provided has been populated
with an RPM. We will set that to the current RPM
*/
    ptEngineApp->fCurrentRpm = tEngineCmd.fRpm;
    ptLoggerApi->LogInfo(ptLogger, "RPM Commanded to: %f", tEngineCmd.fRpm);
/**DOC
Finally we will add some noise to the current RPM in all cases to simulate a real
measurement and publish the telemetry.
*/
exit:
    tEngineTlm.fRpm = ptEngineApp->fCurrentRpm + 10*noise;
    ptBroker->ptApi->Publish(ptBroker, ENGINE_TLM_MSG_MID, tEngineTlmPointer);
    return tStatus;
}

/**DOC
### `OnExit`
The `OnExit` function is typically the last function called in the application.
It is used to clean-up globally allocated resources like sockets and file descriptors before
the main run-time exits. This application does not have any resources to clean up so it just exits.
*/
static JUNO_STATUS_T OnExit(JUNO_APP_ROOT_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptJunoApp);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus)
    // Cast to the engine app
    ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp);
    // Get the logger
    const JUNO_LOG_ROOT_T *ptLogger = ptEngineApp->ptLogger;
    // Get the logger api
    const JUNO_LOG_API_T *ptLoggerApi = ptLogger->ptApi;
    // Log that the app is running
    ptLoggerApi->LogInfo(ptLogger, "Engine App Exiting");
    return tStatus;
}
