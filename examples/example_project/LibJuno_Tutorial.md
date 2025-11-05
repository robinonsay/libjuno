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

### Logging

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

### Time
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

### Instantiating APIs
LibJuno utilizes a vtable, or a table of function pointers to the specific implementation, that
is passed to the LibJuno module. This vtable is called an "API" in LibJuno nomenclature and it
provides a standard interface to the capabilities. LibJuno is implemented as a set of capabilities, or
software components that perform a set functionality. These capabilities are called "Modules" and can
be extened through derivation. Additionally APIs can be extended through derivation. When a module or API
is extended its called "derived".

Below is the API instantiation of the logger and time APIs for this project. We are providing our logging
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

### Failure Handler
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

### The Entry Point
This example project assumes its running on a POSIX system. Therefore,
we can use a main method as the entry point. Many microcontrollers have
different architectures for their entry point so you'll need to consult with
your architecture documentation on how to implement the entry point.

```cpp

int main(void)
{

```

### Dependency Injection in Action
The core principle of dependency injection is "inversion of control".
This is a fancy term for saying that modules don't allocate resources,
up stream users do. This is done at the "composition root", or the spot
in the code where all the dependencies get implemented. In this case,
the main function is the composition root. Here we need to instantiate
the time, logger, registry modules.

Applications in LibJuno are actually derived modules of `JUNO_APP_ROOT_T`.
Here we will need to instantiate the engine, and system manager application modules.

```cpp

    static JUNO_TIME_ROOT_T tTime = {0};
    static JUNO_LOG_ROOT_T tLogger = {0};
    static JUNO_SB_PIPE_T *tRegistry[10] = {0};
    static JUNO_SB_BROKER_ROOT_T tBroker = {0};
    static ENGINE_APP_T tEngineApp = {0};
    static SYSTEM_MANAGER_APP_T tSystemManagerApp = {0};

```

### Module Initialization

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

### Runtime
For this example we are going to create a very simple "schedule table" or table of applications that
will run in a specific order. This schedule table will run at best-effort. This means that when one
application is done, the next will start.

```cpp

    static JUNO_APP_ROOT_T *ptAppList[2] = {
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

### The Engine Application
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
    ENGINE_CMD_MSG_ARRAY_T tCmdArray;
    JUNO_SB_PIPE_T tCmdPipe;
    float fCurrentRpm;
);


```

#### The App Init Function
The application also needs to provide a concrete application initialization function. This function
sets dependencies and the API pointers within the application. The application has an internal "Verify"
function that checks if any of these dependencies are null.

```cpp

JUNO_STATUS_T EngineApp_Init(ENGINE_APP_T *ptEngineApp, const JUNO_LOG_ROOT_T *ptLogger, const JUNO_TIME_ROOT_T *ptTime, JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_FAILURE_HANDLER_T pfcnFailureHandler, JUNO_USER_DATA_T *pvUserData);

```

## engine_cmd_msg.h

This file is auto-generated from a LibJuno script (scripts/create_msg.py). Similarly the
engine_tlm_msg.h and engine_tlm_msg.c files are also auto-generated. We will only go over the
command file since it's identical to the telemetry file in terms of architecture.

### Engine Application Commands
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

### Engine Command Pipe
Below is the definition for the engine command pipe. This is derived from the `JUNO_SB_PIPE_T` using the 
`DERIVE_WITH_API` macro. This macro enables users to specify which API they are using.
The inheritance for a pipe is as follows:
```
JUNO_DS_ARRAY_ROOT_T -> JUNO_DS_QUEUE_ROOT_T -> JUNO_SB_PIPE_T -> Custom Pipe Type
```
Because this is a longer chain of inheritance, it's convenient to specify the API for ease-of-use.
In this case we are specifying the QUEUE api.

The pipe holds a pointer to a command buffer, which is specific to the command message type defined 
earlier. This enables the derived pipe to have specific type information (vs using a void pointer in the pipe).

```cpp

typedef struct ENGINE_CMD_MSG_ARRAY_TAG JUNO_MODULE_DERIVE(JUNO_DS_ARRAY_ROOT_T,
    ENGINE_CMD_MSG_T *ptArrEngineCmdMsgBuffer;
) ENGINE_CMD_MSG_ARRAY_T;


```

The queue api relies on LibJuno pointers. This enables the modules to write generic code safely without
specific type information. As a result, we need to specify a pointer API implementation for this command type.

```cpp

extern const JUNO_POINTER_API_T gtEngineCmdMsgPointerApi;


```

We also define convenience macros for initializing and verifying this type of pointer. This makes working
with LibJuno pointers easy.

```cpp

#define EngineCmdMsg_PointerInit(addr) JunoMemory_PointerInit(&gtEngineCmdMsgPointerApi, ENGINE_CMD_MSG_T, addr)
#define EngineCmdMsg_PointerVerify(tPointer) JunoMemory_PointerVerifyType(tPointer, ENGINE_CMD_MSG_T, gtEngineCmdMsgPointerApi)


```

Finally we define a Pipe Init function for this pipe type. This function will
initialize the pipe with the message buffer and capacity.

```cpp

JUNO_STATUS_T EngineCmdMsg_ArrayInit(ENGINE_CMD_MSG_ARRAY_T *ptEngineCmdMsgPipe, ENGINE_CMD_MSG_T *ptArrEngineCmdMsgBuffer, size_t iCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvUserData);


```

## engine_cmd_msg.c

### Engine Command Pipe Implementation

In `.c` source file we will need to implement the following functions for
the pointer and queue api:

```cpp

static JUNO_STATUS_T EngineCmdMsg_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
static JUNO_STATUS_T EngineCmdMsg_Reset(JUNO_POINTER_T tPointer);

/// Set the value at an index
static JUNO_STATUS_T SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
/// Get the value at an index
static JUNO_RESULT_POINTER_T GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
/// Remove a value at an index
static JUNO_STATUS_T RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);


```

These function will provide an interface to our specific message type
and enable users to write type-safe code within LibJuno.

We need to forward-declare these API functions so we can use the API
pointer to verify the type of the queue and pointer.

Below we will instantiate the pointer and pipe API tables.

```cpp

// Instantiate the engine_cmd msg pointer api
const JUNO_POINTER_API_T gtEngineCmdMsgPointerApi =
{
    EngineCmdMsg_Copy,
    EngineCmdMsg_Reset
};

// Instantiate the engine_cmd msg pipe api
static const JUNO_DS_ARRAY_API_T gtEngineCmdMsgPipeApi = 
{
    SetAt, GetAt, RemoveAt
};


```

#### Pointer Copy
The pointer copy function is responsible for copy memory from one pointer of the same
type to another. We verify the pointers are implemented and are of the same type
by checking the alignment, size, and api pointer. We then dereference the pointer
and copy the values since we have verified the type

```cpp

static JUNO_STATUS_T EngineCmdMsg_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc)
{
    // Verify the dest pointer
    JUNO_STATUS_T tStatus = EngineCmdMsg_PointerVerify(tDest);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Verify the src pointer
    tStatus = EngineCmdMsg_PointerVerify(tSrc);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Perform the copy
    *(ENGINE_CMD_MSG_T *) tDest.pvAddr = *(ENGINE_CMD_MSG_T *) tSrc.pvAddr;
    return tStatus;
}


```

#### Pointer Reset
The reset function will reinitialize the memory of a pointer of this message type.
In this case, it means setting the memory to 0. Similar to the copy function,
we verify the pointer type and api before dereferencing the pointer.

```cpp

static JUNO_STATUS_T EngineCmdMsg_Reset(JUNO_POINTER_T tPointer)
{
    // Verify the pointer
    JUNO_STATUS_T tStatus = EngineCmdMsg_PointerVerify(tPointer);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Perform the reset
    *(ENGINE_CMD_MSG_T *) tPointer.pvAddr = (ENGINE_CMD_MSG_T){0};
    return tStatus;
}


```

### Pipe Api Assert
We also define a macro to easily assert that the pipe type matches
our implementation. This is done by checking the pipe api pointer.

```cpp

/// Asserts the api is for the pipe
#define ENGINE_CMD_MSG_PIPE_ASSERT_API(ptArray, ...)  if(ptArray->ptApi != &gtEngineCmdMsgPipeApi) { __VA_ARGS__; }


```

### Pipe Init
We also implement the pipe init function, which sets the API pointer as well as the
message buffer and capacity.

```cpp

JUNO_STATUS_T EngineCmdMsg_ArrayInit(ENGINE_CMD_MSG_ARRAY_T *ptEngineCmdMsgPipe, ENGINE_CMD_MSG_T *ptArrEngineCmdMsgBuffer, size_t iCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvUserData)
{
    // Assert the msg pipe exists
    JUNO_ASSERT_EXISTS(ptEngineCmdMsgPipe && ptArrEngineCmdMsgBuffer);
    // Set the message buffer
    ptEngineCmdMsgPipe->ptArrEngineCmdMsgBuffer = ptArrEngineCmdMsgBuffer;

    // init the pipe
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(&ptEngineCmdMsgPipe->tRoot, &gtEngineCmdMsgPipeApi, iCapacity, pfcnFailureHdlr, pvUserData);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}


```

### Pipe Queue Implementation
Finally we implement the `SetAt`, `GetAt`, and `RemoveAt` functions.
These functions provide a type-safe interface to setting, getting, and removing
values within the command buffer at specific indicies. It essentially acts as an API
to the array.

```cpp

static JUNO_STATUS_T SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex)
{
    // Verify the array
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Assert the api
    ENGINE_CMD_MSG_PIPE_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    // Verify the pointer
    tStatus = EngineCmdMsg_PointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Check if the index is valid
    tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Cast to the pipe type
    ENGINE_CMD_MSG_ARRAY_T *ptEngineCmdMsgPipe = (ENGINE_CMD_MSG_ARRAY_T *)ptArray;
    // Init the pointer to the buffer
    JUNO_POINTER_T tIndexPointer = EngineCmdMsg_PointerInit(&ptEngineCmdMsgPipe->ptArrEngineCmdMsgBuffer[iIndex]);
    // Copy the memory to the buffer
    tStatus = tIndexPointer.ptApi->Copy(tIndexPointer, tItem);
    return tStatus;
}
/// Get the value at an index
static JUNO_RESULT_POINTER_T GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_RESULT_POINTER_T tResult = {0};
    // Verify the array
    tResult.tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    // Assert the api
    ENGINE_CMD_MSG_PIPE_ASSERT_API(ptArray,
        tResult.tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        return tResult;
    );
    // Check the index
    tResult.tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    // Cast to the pipe type
    ENGINE_CMD_MSG_ARRAY_T *ptEngineCmdMsgPipe = (ENGINE_CMD_MSG_ARRAY_T *)ptArray;
    // Create the pointer to the buffer
    JUNO_POINTER_T tIndexPointer = EngineCmdMsg_PointerInit(&ptEngineCmdMsgPipe->ptArrEngineCmdMsgBuffer[iIndex]);
    // Copy to ok result
    tResult.tOk = tIndexPointer;
    return tResult;
}
/// Remove a value at an index
static JUNO_STATUS_T RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    // Verify the array
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Assert the api
    ENGINE_CMD_MSG_PIPE_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    // Verify the index
    tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Cast to the msg pipe type
    ENGINE_CMD_MSG_ARRAY_T *ptEngineCmdMsgPipe = (ENGINE_CMD_MSG_ARRAY_T *)ptArray;
    // Create pointer to memory
    JUNO_POINTER_T tIndexPointer = EngineCmdMsg_PointerInit(&ptEngineCmdMsgPipe->ptArrEngineCmdMsgBuffer[iIndex]);
    // Reset the memory
    tStatus = tIndexPointer.ptApi->Reset(tIndexPointer);
    return tStatus;
}

```

## engine_app.c

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

```cpp


static inline JUNO_STATUS_T Verify(JUNO_APP_ROOT_T *ptJunoApp);
static JUNO_STATUS_T OnStart(JUNO_APP_ROOT_T *ptJunoApp);
static JUNO_STATUS_T OnProcess(JUNO_APP_ROOT_T *ptJunoApp);
static JUNO_STATUS_T OnExit(JUNO_APP_ROOT_T *ptJunoApp);

static const JUNO_APP_API_T tEngineAppApi = {
    .OnStart = OnStart,
    .OnProcess = OnProcess,
    .OnExit = OnExit
};


```

### Verification Function
The verification function is called as the one of the very first lines in
a LibJuno API function. Its purpose is to assert that the module contract
is met. This means that pointers are initialized, and the APIs match.

In LibJuno we can use `JUNO_ASSERT` macros that will check for certain
common conditions like null pointers. These macros will automatically
return `JUNO_STATUS_ERR` or `JUNO_STATUS_NULLPTR_ERROR` on failure.

```cpp

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


```

### The Application Init Function
The init function's primary responsibility is to assign dependencies. This function
enables the `main.c` project composition root to pass application dependencies to the application.
This function is a concrete implementation because it depends entirely on application specifics.

This function asserts the module pointer exists, assigns the dependency pointers, and finally
calls the verification function. The verification function is called last because it assumes
the pointers have been instantiated.

```cpp

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


```

### `OnStart`
`OnStart` is the very first function called in a LibJuno project and is
typically called only once. The primary purpose of `OnStart` is to
initialize state variables, perform initialization tasks specific to the
application, and allocate global application resources.

```cpp

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

```

Within `OnStart` we will initialize our command message pipe so we can receive
commands. We will also register this pipe with the broker so it can be active.

```cpp

    tStatus = EngineCmdMsg_ArrayInit(&ptEngineApp->tCmdArray, ptEngineApp->ptArrCmdBuffer, ENGINE_CMD_MSG_PIPE_DEPTH, ptJunoApp->_pfcnFailureHandler, ptJunoApp->_pvFailureUserData);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoSb_PipeInit(&ptEngineApp->tCmdPipe, ENGINE_CMD_MSG_MID, &ptEngineApp->tCmdArray.tRoot, ptJunoApp->_pfcnFailureHandler, ptJunoApp->_pvFailureUserData);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Register the subscriber
    tStatus = ptEngineApp->ptBroker->ptApi->RegisterSubscriber(ptEngineApp->ptBroker, &ptEngineApp->tCmdPipe);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);

```

For this example we will also set the current RPM to 0

```cpp

    // Initialize current RPM to 0
    ptEngineApp->fCurrentRpm = 0.0;
    return tStatus;
}


```

### `OnProcess`
OnProcess is the main run-loop function of the application. It is called on a periodic
basis, typically at a set frequency (like 1Hz, 5Hz, 50Hz, 100Hz, etc).

This is where the main functionality of the application will reside.

```cpp

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

```

In this example we will sleep the application for 500ms to simulate
extensive work being done within the application. We will also
create a convenience variable to access the pipe's queue API.

```cpp

    // sleep for half a second
    usleep(500E3);
    const JUNO_DS_QUEUE_API_T *ptCmdPipeApi = ptEngineApp->tCmdPipe.tRoot.ptApi;

```

For this example we will allocate a buffer to attempt to receive a command
from the message pipe. We will then initialize an engine command pointer with
the address to our allocated buffer.

```cpp

    ENGINE_CMD_MSG_T tEngineCmd = {0};
    JUNO_POINTER_T tEngineCmdPointer = EngineCmdMsg_PointerInit(&tEngineCmd);

```

We will then get the current timestamp using the time api provided by the
project root.

```cpp

    JUNO_TIMESTAMP_RESULT_T tTimestampResult = ptTime->ptApi->Now(ptTime);
    JUNO_ASSERT_SUCCESS(tTimestampResult.tStatus, return tTimestampResult.tStatus);

```

Next we initialize a telemetry message with the current RPM and timestamp. This is done at this
step because in the case of an error, we will jump to the `exit` point with a `goto` statement
so we always publish this telemetry.

```cpp

    ENGINE_TLM_MSG_T tEngineTlm = {ptEngineApp->fCurrentRpm, tTimestampResult.tOk};
    JUNO_POINTER_T tEngineTlmPointer = EngineTlmMsg_PointerInit(&tEngineTlm);

```

We will calculate some artificial noise to simulate an actual sensor measurement.

```cpp

    float noise = rand();
    noise = noise - RAND_MAX/2;
    noise = 2 * noise / RAND_MAX;

```

Next we will attempt to dequeue a command from the software bus. This operation will fail
if there is no command in the pipe and will jump to the exit point of this function.

```cpp

    tStatus = ptCmdPipeApi->Dequeue(&ptEngineApp->tCmdPipe.tRoot, tEngineCmdPointer);
    JUNO_ASSERT_SUCCESS(tStatus, goto exit);
/*DOC
Because we has a `JUNO_ASSERT_SUCCESS` on the previous function, we know that at this point
in the software the `dequeue` succeeded and the command buffer we provided has been populated
with an RPM. We will set that to the current RPM
*/
    ptEngineApp->fCurrentRpm = tEngineCmd.fRpm;
    ptLoggerApi->LogInfo(ptLogger, "RPM Commanded to: %f", tEngineCmd.fRpm);

```

Finally we will add some noise to the current RPM in all cases to simulate a real
measurement and publish the telemetry.

```cpp

exit:
    tEngineTlm.fRpm = ptEngineApp->fCurrentRpm + 10*noise;
    ptBroker->ptApi->Publish(ptBroker, ENGINE_TLM_MSG_MID, tEngineTlmPointer);
    return tStatus;
}


```

### `OnExit`
The `OnExit` function is typically the last function called in the application.
It is used to clean-up globally allocated resources like sockets and file descriptors before
the main run-time exits. This application does not have any resources to clean up so it just exits.

```cpp

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

```

## system_manager_app.c

The System Manager application for this car example will subscribe to the
engine apps's telemetry. Similar to the engine app's command pipe,
the system manager will read the telemetry off its telemetry pipe. This is done
by allocating a telemetry buffer.

```cpp

    // sleep for half a second
    ENGINE_TLM_MSG_T tTlmMsg = {0};
    JUNO_POINTER_T tTlmMsgPointer = EngineTlmMsg_PointerInit(&tTlmMsg);

```

Additionally the system manager will command the engine to certain RPMs.
It will create the command and command pointer here.

```cpp

    ENGINE_CMD_MSG_T tEngineCmd = {ptSystemManagerApp->fTargetRpm};
    JUNO_POINTER_T tEngineCmdPointer = EngineCmdMsg_PointerInit(&tEngineCmd);

```

The system manager will attempt to read telemetry off the software bus from the engine.
If there is telemetry it will process it and set a new RPM. If there is no telemetry it
will command the engine to the same target as before.

```cpp

    JUNO_TIME_MILLIS_RESULT_T tMillisResult = {0};
    tStatus = ptSystemManagerApp->tEngineTlmPipe.tRoot.ptApi->Dequeue(&ptSystemManagerApp->tEngineTlmPipe.tRoot, tTlmMsgPointer);
    JUNO_ASSERT_SUCCESS(tStatus, goto exit);

```

The system manager will substract the time from engine start to get an elapsed time. I will then
check if the engine is within the target RPM. If it is it will increment the RPM by 10RPM. If not
it will send the same target RPM as before.

```cpp

    tStatus = ptTime->ptApi->SubtractTime(ptTime, &tTlmMsg.tTimestamp, ptSystemManagerApp->tEngineStart);
    JUNO_ASSERT_SUCCESS(tStatus, goto exit);
    tMillisResult = ptTime->ptApi->TimestampToMillis(ptTime, tTlmMsg.tTimestamp);
    JUNO_ASSERT_SUCCESS(tMillisResult.tStatus, return tMillisResult.tStatus);
    ptLoggerApi->LogInfo(ptLogger, "Current Rpm: %f | Time: %ull", tTlmMsg.fRpm, tMillisResult.tOk);
    if(fabs(tTlmMsg.fRpm - ptSystemManagerApp->fTargetRpm) < 10.0f)
    {
        ptSystemManagerApp->fTargetRpm += 10.0f;
    }

```

Finally the system manager sets the target RPM and publishes the message

```cpp

exit:
    tEngineCmd.fRpm = ptSystemManagerApp->fTargetRpm;
    tStatus = ptBroker->ptApi->Publish(ptBroker, ENGINE_CMD_MSG_MID, tEngineCmdPointer);
    return tStatus;
}


```

## Conclusion
Hopefully this tutorial helped demonstrate how to utilize the LibJuno micro-framework within
a software project. Its intent is to provide a toolbox to developers of interfaces and
implementations that they can choose to use. Ultimately the developers will implement solutions
targeted for their project so much of the architecture is dependent on specific project needs
that can't possibly be captured here. This example project is to showcase the various capabilities
a user can choose to implement.
