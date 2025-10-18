/**DOC
# Software Bus Example Overview
In this example we will walkthrough the process of implementing a
LibJuno software bus. See the doxygen documentation for the broker_api
for detailed design documentation on the broker.

# Type Definitions
First we declate the message type definitions that are going to
be flowing over the software bus on this thread. In this example
we have are defining messages that will be sent to and from the engine
application. The engine can be commanded to an RPM and can telemeter
the current measured RPM.

We also set parameters like the Message Identifier (MID) and the
pipe depth, or the maximum number of messages the subscriber will
receive of that type. In this case, the engine application will
receive a maximum of 10 commands during an execution cycle, and
the car's main computer will recveive a maximum of 5 telemetry messages.
*/

#include "juno/ds/array_api.h"
#include "juno/memory/pointer_api.h"
#include "juno/module.h"
#include "juno/sb/broker_api.h"
#include "juno/status.h"
#include "juno/time/time_api.h"
#include <stdint.h>

/// The engine command to set the RPM
typedef struct ENGINE_RPM_CMD_TAG ENGINE_RPM_CMD_T;
/// An array to store received rpm commands
typedef struct ENGINE_RPM_CMD_ARRAY_TAG ENGINE_RPM_CMD_ARRAY_T;
#define ENGINE_RPM_CMD_PIPE_DEPTH   10
typedef uint16_t ENGINE_MID;
const ENGINE_MID giENGINE_RPM_CMD_MID = 1;
/// The engine telemtry of what the RPM actually is
typedef struct ENGINE_RPM_TLM_TAG ENGINE_RPM_TLM_T;
/// An array to store received rpm telemetry
typedef struct ENGINE_RPM_TLM_ARRAY_TAG ENGINE_RPM_TLM_ARRAY_T;
#define ENGINE_RPM_TLM_PIPE_DEPTH   5
const ENGINE_MID giENGINE_RPM_TLM_MID = 2;

struct ENGINE_RPM_CMD_TAG
{
    /// The engine RPM
    float fEngineRpm;
};

struct ENGINE_RPM_TLM_TAG
{
    /// The engine RPM
    float fEngineRpm;
    /// The timestamp of the telemetry
    JUNO_TIMESTAMP_T tTimestamp;
};

/**DOC
## Defining the Pointer Types

We need to define the pointer types for the messages, and the value pointer types for the
MID. This will be used to safely copy, reset, and compare these types
*/
typedef struct ENGINE_RPM_CMD_POINTER_T JUNO_POINTER_DERIVE(JUNO_POINTER_T, ENGINE_RPM_CMD_T) ENGINE_RPM_CMD_POINTER_T;
typedef struct ENGINE_RPM_TLM_POINTER_T JUNO_POINTER_DERIVE(JUNO_POINTER_T, ENGINE_RPM_TLM_T) ENGINE_RPM_TLM_POINTER_T;
typedef struct ENGINE_MID_POINTER_T JUNO_POINTER_DERIVE(JUNO_VALUE_POINTER_T, ENGINE_MID) ENGINE_MID_POINTER_T;

/**DOC
## Defining the Array Types

Next we need to implement the array types as derivations of the `JUNO_DS_ARRAY_ROOT_T`
module and implement the API. This inversion of control design paradigm enables
developers to write C code without knowing the exact type they are using.
This implementation is safe because you typically only up-cast from the derived type
to the root module. Down-casts only occur within the derived function implementations
where they can verify the type by checking the API pointer.

Theses array types will be used to with the pipe queue to receive messages.
*/
struct ENGINE_RPM_CMD_ARRAY_TAG JUNO_MODULE_DERIVE(JUNO_DS_ARRAY_ROOT_T,
    /// The array of commands
    ENGINE_RPM_CMD_T tArrCmds[ENGINE_RPM_CMD_PIPE_DEPTH];
);

struct ENGINE_RPM_TLM_ARRAY_TAG JUNO_MODULE_DERIVE(JUNO_DS_ARRAY_ROOT_T,
    /// The array of telemetry
    ENGINE_RPM_TLM_T tArrTelem[ENGINE_RPM_TLM_PIPE_DEPTH];
);

/**DOC
## Decalring the Api Functions

We need to decalre the pointer api implemnentations. LibJuno offers
a convience macro to aid in this process since it can become repetative.

We also need to declare the Array API functions.

We forward declare these functions so we can implement the API struct for
both the pointer APIs and the Array APIs.
*/
JUNO_POINTER_API_FUNC_DECLARE(ENGINE_RPM_CMD_T);
JUNO_POINTER_API_FUNC_DECLARE(ENGINE_RPM_TLM_T);
JUNO_VALUE_POINTER_API_FUNC_DECLARE(ENGINE_MID);

/// Checks if the MIDs are equal
/// Set the value at an index
static JUNO_STATUS_T EngineRpmCmdSetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
/// Get the value at an index
static JUNO_RESULT_POINTER_T EngineRpmCmdGetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
/// Remove a value at an index
static JUNO_STATUS_T EngineRpmCmdRemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);

/// Set the value at an index
static JUNO_STATUS_T EngineRpmTlmSetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
/// Get the value at an index
static JUNO_RESULT_POINTER_T EngineRpmTlmGetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
/// Remove a value at an index
static JUNO_STATUS_T EngineRpmTlmRemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);

const JUNO_DS_ARRAY_API_T gtEngineRpmCmdArrayApi =
{
    EngineRpmCmdSetAt,
    EngineRpmCmdGetAt,
    EngineRpmCmdRemoveAt,
};

const JUNO_DS_ARRAY_API_T gtEngineRpmTlmArrayApi =
{
    EngineRpmTlmSetAt,
    EngineRpmTlmGetAt,
    EngineRpmTlmRemoveAt,
};

const JUNO_POINTER_API_T gtEngineRpmCmdPointerApi = JUNO_POINTER_API(ENGINE_RPM_CMD_T);
const JUNO_POINTER_API_T gtEngineRpmTlmPointerApi = JUNO_POINTER_API(ENGINE_RPM_TLM_T);
const JUNO_VALUE_POINTER_API_T gtEngineRpmMidPointerApi = JUNO_VALUE_POINTER_API(ENGINE_MID);

int main(void)
{
    return 0;
}

JUNO_POINTER_API_FUNC(ENGINE_RPM_CMD_T, ENGINE_RPM_CMD_POINTER_T, gtEngineRpmCmdPointerApi)
JUNO_POINTER_API_FUNC(ENGINE_RPM_TLM_T, ENGINE_RPM_TLM_POINTER_T, gtEngineRpmTlmPointerApi)
JUNO_VALUE_POINTER_API_FUNC(ENGINE_MID, ENGINE_MID_POINTER_T, gtEngineRpmMidPointerApi.tRoot)

/// Set the value at an index
static JUNO_STATUS_T EngineRpmCmdSetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_POINTER(tStatus, tItem, ENGINE_RPM_CMD_T, gtEngineRpmCmdPointerApi);
    ENGINE_RPM_CMD_ARRAY_T *ptCmdArray = (ENGINE_RPM_CMD_ARRAY_T *) ptArray;
    ENGINE_RPM_CMD_POINTER_T tMsgPointer = {tItem};
    if(iIndex >= ptArray->zCapacity)
    {
        tStatus = JUNO_STATUS_ERR;
        JUNO_FAIL_MODULE(tStatus, ptCmdArray, "Index is oob");
        return tStatus;
    }
    ptCmdArray->tArrCmds[iIndex] = *tMsgPointer.tPtr;
    return tStatus;
}
/// Get the value at an index
static JUNO_RESULT_POINTER_T EngineRpmCmdGetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_RESULT_POINTER_T tResult = {0};
    tResult.tStatus = JunoDs_ArrayVerify(ptArray);
    ENGINE_RPM_CMD_ARRAY_T *ptCmdArray = (ENGINE_RPM_CMD_ARRAY_T *) ptArray;
    if(iIndex >= ptArray->zCapacity)
    {
        tResult.tStatus = JUNO_STATUS_ERR;
        JUNO_FAIL_MODULE(tResult.tStatus, ptCmdArray, "Index is oob");
        return tResult;
    }
    tResult.tOk = JunoMemory_PointerInit(&gtEngineRpmCmdPointerApi, ENGINE_RPM_CMD_T, &ptCmdArray->tArrCmds[iIndex]);
    return tResult;
}
/// Remove a value at an index
static JUNO_STATUS_T EngineRpmCmdRemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    ENGINE_RPM_CMD_ARRAY_T *ptCmdArray = (ENGINE_RPM_CMD_ARRAY_T *) ptArray;
    if(iIndex >= ptArray->zCapacity)
    {
        tStatus = JUNO_STATUS_ERR;
        JUNO_FAIL_MODULE(tStatus, ptCmdArray, "Index is oob");
        return tStatus;
    }
    ptCmdArray->tArrCmds[iIndex] = (ENGINE_RPM_CMD_T){0};
    return tStatus;
}

/// Set the value at an index
static JUNO_STATUS_T EngineRpmTlmSetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_POINTER(tStatus, tItem, ENGINE_RPM_CMD_T, gtEngineRpmCmdPointerApi);
    ENGINE_RPM_TLM_ARRAY_T *ptTlmArray = (ENGINE_RPM_TLM_ARRAY_T *) ptArray;
    ENGINE_RPM_TLM_POINTER_T tMsgPointer = {tItem};
    if(iIndex >= ptArray->zCapacity)
    {
        tStatus = JUNO_STATUS_ERR;
        JUNO_FAIL_MODULE(tStatus, ptTlmArray, "Index is oob");
        return tStatus;
    }
    ptTlmArray->tArrTelem[iIndex] = *tMsgPointer.tPtr;
    return tStatus;
}
/// Get the value at an index
static JUNO_RESULT_POINTER_T EngineRpmTlmGetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_RESULT_POINTER_T tResult = {0};
    tResult.tStatus = JunoDs_ArrayVerify(ptArray);
    ENGINE_RPM_TLM_ARRAY_T *ptTlmArray = (ENGINE_RPM_TLM_ARRAY_T *) ptArray;
    if(iIndex >= ptArray->zCapacity)
    {
        tResult.tStatus = JUNO_STATUS_ERR;
        JUNO_FAIL_MODULE(tResult.tStatus, ptTlmArray, "Index is oob");
        return tResult;
    }
    tResult.tOk = JunoMemory_PointerInit(&gtEngineRpmTlmPointerApi, ENGINE_RPM_TLM_T, &ptTlmArray->tArrTelem[iIndex]);
    return tResult;
}
/// Remove a value at an index
static JUNO_STATUS_T EngineRpmTlmRemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    ENGINE_RPM_TLM_ARRAY_T *ptTlmArray = (ENGINE_RPM_TLM_ARRAY_T *) ptArray;
    if(iIndex >= ptArray->zCapacity)
    {
        tStatus = JUNO_STATUS_ERR;
        JUNO_FAIL_MODULE(tStatus, ptTlmArray, "Index is oob");
        return tStatus;
    }
    ptTlmArray->tArrTelem[iIndex] = (ENGINE_RPM_TLM_T){0};
    return tStatus;
}
