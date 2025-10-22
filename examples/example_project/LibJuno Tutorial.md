# LibJuno Tutorial

## main.c

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

```cpp

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


```

# Time
In this example project we will need a timestamp, so we'll implement the `Now` function.
We don't need `Sleep` or `SleepTo` so we'll provide mocks to those functions. LibJuno
provides time math functions that we can utilize so we only need to provide implementations
to these three time functions.

```cpp

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


```

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

```cpp

static const JUNO_LOG_API_T gtMyLoggerApi ={
    LogDebug,
    LogInfo,
    LogWarning,
    LogError
};

static const JUNO_TIME_API_T gtTimeApi = JunoTime_TimeApiInit(Now, SleepTo, Sleep);


```

# Failure Handler
LibJuno utilizes a failure handler callback. This is a function that is automatically called by
downstream code when a failure occurs. In an embedded system, you might not have a terminal or console
log running. This enables developers to have a single implementation and methodology for handling failure
within the software. The `JUNO_FAIL...` macros will automatically call the failure handler if one is
provided.

```cpp

void FailureHandler(JUNO_STATUS_T tStatus, const char *pcMsg, JUNO_USER_DATA_T *pvUserData)
{
    (void) pvUserData;
    printf("FAILURE: %u | %s\n", tStatus, pcMsg);
}


```

# The Entry Point
This example project assumes its running on a POSIX system. Therefore,
we can use a main method as the entry point. Many microcontrollers have
different architectures for their entry point so you'll need to consult with
your architecture documentation on how to implement the entry point.

```cpp

int main(void)
{

```

# Dependency Injection in Action
The core principle of dependency injection is "inversion of control".
This is a fancy term for saying that modules don't allocate resources,
up stream users do. This is done at the "composition root", or the spot
in the code where all the dependencies get implemented. In this case,
the main function is the composition root. Here we need to instantiate
the time, logger, registry modules.

Applications in LibJuno are actually derived modules of `JUNO_APP_ROOT_T`.
Here we will need to instantiate the engine, and system manager application modules.

```cpp

    JUNO_TIME_ROOT_T tTime = {0};
    JUNO_LOG_ROOT_T tLogger = {0};
    JUNO_SB_PIPE_T *tRegistry[10] = {0};
    JUNO_SB_BROKER_ROOT_T tBroker = {0};
    ENGINE_APP_T tEngineApp = {0};
    SYSTEM_MANAGER_APP_T tSystemManagerApp = {0};

```

# Module Initialization

Modules typically have an `Init` function that specifies the required fields to initialize a module.
Modules also utilize a `Verify` function to verify they've been initialzed. Most modules will return
a `JUNO_STATUS_NULLPTR_ERROR` if they have not been initalized and a function has been called on them.

```cpp

    JUNO_STATUS_T tStatus = JunoTime_TimeInit(&tTime, &gtTimeApi, FailureHandler, NULL);
    JUNO_ASSERT_SUCCESS(tStatus, return -1);
    tStatus = JunoLog_LogInit(&tLogger, &gtMyLoggerApi, FailureHandler, NULL);
    JUNO_ASSERT_SUCCESS(tStatus, return -1);

```

Notice how the broker, engine, and system manager take initialized modules as dependencies to their initialization.
This is DI in action. For example, the engine app is provided with a logger, time, and broker instead of instantiating it
itself.

```cpp

    tStatus = JunoSb_BrokerInit(&tBroker, tRegistry, 10, FailureHandler, NULL);
    JUNO_ASSERT_SUCCESS(tStatus, return -1);
    tStatus = EngineApp_Init(&tEngineApp, &tLogger, &tTime, &tBroker, FailureHandler, NULL);
    JUNO_ASSERT_SUCCESS(tStatus, return -1);
    tStatus = SystemManagerApp(&tSystemManagerApp, &tLogger, &tTime, &tBroker, FailureHandler, NULL);
    JUNO_ASSERT_SUCCESS(tStatus, return -1);

```

# Runtime
For this example we are going to create a very simple "schedule table" or table of applications that
will run in a specific order. This schedule table will run at best-effort. This means that when one
application is done, the next will start.

```cpp

    JUNO_APP_ROOT_T *ptAppList[2] = {
        &tSystemManagerApp.tRoot,
        &tEngineApp.tRoot
    };

```

Since all applications have the same interface, we can run their init function in a for-loop
and run the application `OnProcess` function in a `while(true)` loop.

```cpp

    for(size_t i = 0; i < 2; i++)
    {
        tStatus = ptAppList[i]->ptApi->OnStart(ptAppList[i]);
        JUNO_ASSERT_SUCCESS(tStatus, return -1);
    }
    size_t iCounter = 0;
    while(true)
    {
        ptAppList[iCounter]->ptApi->OnProcess(ptAppList[iCounter]);
        iCounter = (iCounter + 1) % 2;
    }
}

```

## engine_app.h

# The Engine Application
Next we will be going over a typical LibJuno application. For this tutorial project
we are defining an engine application that manages the engine hardware for our car.
In this example the engine needs to receive a command for the RPM and telemeter the
actual RPM of the engine.

## Deriving an Application
Applications in LibJuno are derived from the application root. This provides
a common API for applications to utilize. A standard application has three
lifecycle functions:
1. OnStart: The first function called in the application. It is called
only once at the beginning of the application lifecycle. This is where
developers would initialize resources that will be utilized by the
application
2. OnProcess: This is the "main run-loop" function of the application.
This function is called by the main run-time in a loop and is the primary
run function of the app.
3. OnExit: This is the last function called by the application. This is
where developers would clean up resources allocated by the application.
A typical runtime would call application exit functions when the software
is closing.

## The Application Structure
Within the derived application users are expected to place
pointers to dependencies they require from the main runtime
and hold resources the application needs to own and pass from
one lifecycle event to the next. In this case the engine app
needs the following from the runtime:
* Logger
* Time
* Broker

And the application needs to allocate:
* A command buffer for engine commands. This is a memory allocation
for commands that will be received by the application
* A command pipe. This is the module that manages the command buffer
* The current RPM. This is the current state of our engine.

```cpp

typedef struct ENGINE_APP_TAG ENGINE_APP_T;
#define ENGINE_CMD_MSG_PIPE_DEPTH   (1)

struct ENGINE_APP_TAG JUNO_MODULE_DERIVE(JUNO_APP_ROOT_T,
    const JUNO_LOG_ROOT_T *ptLogger;
    const JUNO_TIME_ROOT_T *ptTime;
    JUNO_SB_BROKER_ROOT_T *ptBroker;
    ENGINE_CMD_MSG_T ptArrCmdBuffer[ENGINE_CMD_MSG_PIPE_DEPTH];
    ENGINE_CMD_MSG_PIPE_T tCmdPipe;
    float fCurrentRpm;
);


```

## The App Init Function
The application also needs to provide a concrete application initialization function. This function
sets dependencies and the API pointers within the application. The application has an internal "Verify"
function that checks if any of these dependencies are null.

```cpp

JUNO_STATUS_T EngineApp_Init(
    ENGINE_APP_T *ptEngineApp,
    const JUNO_LOG_ROOT_T *ptLogger,
    const JUNO_TIME_ROOT_T *ptTime,
    JUNO_SB_BROKER_ROOT_T *ptBroker,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvUserData
);

#ifdef __cplusplus
}
#endif
#endif // ENGINE_APP_API_H

```

## engine_cmd_msg.h

## Engine Application Commands
This file is auto-generated from a LibJuno script (scripts/create_msg.py). Similarly the
engine_tlm_msg.h and engine_tlm_msg.c files are also auto-generated. We will only go over the
command file since it's identical to the telemetry file in terms of architecture.

In LibJuno the software bus is operated on a single thread. A single broker is implemented
for each thread of execution and the broker distributes messages to various queues that
are subscribed on the software bus. These queues are called pipes. Pipes are derived LibJuno
queues with a message ID subscription.

The message ID is a unique identifier for the type of message. Each message type will have its
own message ID. In this example the engine command MID is `0x1800`. This number can be arbitrary.
The only requirement is the MIDs are unique for every message type.

```cpp

#define ENGINE_CMD_MSG_MID            (0x1800)


```

The command message contains the RPM. Another application can send this command on the software bus
and tell the engine what RPM it should be set to. The engine app will then control the engine so it's
set to the new RPM.

```cpp

typedef struct ENGINE_CMD_MSG_TAG
{
    float fRpm;
} ENGINE_CMD_MSG_T;


```

## Engine Command Pipe
Below is the definition for the engine command pipe. This is derived from the `JUNO_SB_PIPE_T` using the 
`DERIVE_WITH_API` macro. This macro enables users to specify which API they are using.
The inheritance for a pipe is as follows:
```
JUNO_DS_ARRAY_ROOT_T -> JUNO_DS_QUEUE_T -> JUNO_SB_PIPE_T -> Custom Pipe Type
```
Because this is a longer chain of inheritance, it's convenient to specify the API for ease-of-use.
In this case we are specifying the QUEUE api.

The pipe holds a pointer to a command buffer, which is specific to the command message type defined 
earlier. This enables the derived pipe to have specific type information (vs using a void pointer in the pipe).

```cpp

typedef struct ENGINE_CMD_MSG_PIPE_TAG JUNO_MODULE_DERIVE_WITH_API(JUNO_SB_PIPE_T, JUNO_DS_QUEUE_API_T,
    ENGINE_CMD_MSG_T *ptArrEngineCmdMsgBuffer;
) ENGINE_CMD_MSG_PIPE_T;


```

The queue api relies on LibJuno pointers. This enables the modules to write generic code safely without
specific type information. As a result, we need to specify a pointer API implementation for this command type.

```cpp

extern const JUNO_POINTER_API_T gtEngineCmdMsgPointerApi;


```

We also define convience macros for initializing and verifying this type of pointer. This makes working
with LibJuno pointers easy.

```cpp

#define EngineCmdMsg_PointerInit(addr) JunoMemory_PointerInit(&gtEngineCmdMsgPointerApi, ENGINE_CMD_MSG_T, addr)
#define EngineCmdMsg_PointerVerify(tPointer) JunoMemory_PointerVerifyType(tPointer, ENGINE_CMD_MSG_T, gtEngineCmdMsgPointerApi)
JUNO_STATUS_T EngineCmdMsg_PipeInit(ENGINE_CMD_MSG_PIPE_T *ptEngineCmdMsgPipe, ENGINE_CMD_MSG_T *ptArrEngineCmdMsgBuffer, size_t iCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvUserData);

#ifdef __cplusplus
}
#endif
#endif


```