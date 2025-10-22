# Engine_app Docs

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