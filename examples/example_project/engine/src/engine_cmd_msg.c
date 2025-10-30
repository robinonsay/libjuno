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
#include "juno/ds/array_api.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include "juno/status.h"

/**DOC
### Engine Command Pipe Implementation

In `.c` source file we will need to implement the following functions for
the pointer and queue api:
*/
static JUNO_STATUS_T EngineCmdMsg_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
static JUNO_STATUS_T EngineCmdMsg_Reset(JUNO_POINTER_T tPointer);

/// Set the value at an index
static JUNO_STATUS_T SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
/// Get the value at an index
static JUNO_RESULT_POINTER_T GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
/// Remove a value at an index
static JUNO_STATUS_T RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);

/**DOC
These function will provide an interface to our specific message type
and enable users to write type-safe code within LibJuno.

We need to forward-declare these API functions so we can use the API
pointer to verify the type of the queue and pointer.

Below we will instantiate the pointer and pipe API tables.
*/
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

/**DOC
#### Pointer Copy
The pointer copy function is responsible for copy memory from one pointer of the same
type to another. We verify the pointers are implemented and are of the same type
by checking the alignment, size, and api pointer. We then dereference the pointer
and copy the values since we have verified the type
*/
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

/**DOC
#### Pointer Reset
The reset function will reinitialize the memory of a pointer of this message type.
In this case, it means setting the memory to 0. Similar to the copy function,
we verify the pointer type and api before dereferencing the pointer.
*/
static JUNO_STATUS_T EngineCmdMsg_Reset(JUNO_POINTER_T tPointer)
{
    // Verify the pointer
    JUNO_STATUS_T tStatus = EngineCmdMsg_PointerVerify(tPointer);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Perform the reset
    *(ENGINE_CMD_MSG_T *) tPointer.pvAddr = (ENGINE_CMD_MSG_T){0};
    return tStatus;
}

/**DOC
### Pipe Api Assert
We also define a macro to easily assert that the pipe type matches
our implementation. This is done by checking the pipe api pointer.
*/
/// Asserts the api is for the pipe
#define ENGINE_CMD_MSG_PIPE_ASSERT_API(ptArray, ...)  if(ptArray->ptApi != &gtEngineCmdMsgPipeApi) { __VA_ARGS__; }

/**DOC
### Pipe Init
We also implement the pipe init function, which sets the API pointer as well as the
message buffer and capacity.
*/
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

/**DOC
### Pipe Queue Implementation
Finally we implement the `SetAt`, `GetAt`, and `RemoveAt` functions.
These functions provide a type-safe interface to setting, getting, and removing
values within the command buffer at specific indicies. It essentially acts as an API
to the array.
*/
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
