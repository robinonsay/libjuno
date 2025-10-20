#include "engine_app/engine_cmd_msg.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include "juno/status.h"

static JUNO_STATUS_T EngineCmdMsg_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
static JUNO_STATUS_T EngineCmdMsg_Reset(JUNO_POINTER_T tPointer);

/// Set the value at an index
static JUNO_STATUS_T SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
/// Get the value at an index
static JUNO_RESULT_POINTER_T GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
/// Remove a value at an index
static JUNO_STATUS_T RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);

// Instantiate the engine_cmd msg pointer api
const JUNO_POINTER_API_T gtEngineCmdMsgPointerApi =
{
    EngineCmdMsg_Copy,
    EngineCmdMsg_Reset
};

// Instantiate the engine_cmd msg pipe api
static const JUNO_DS_QUEUE_API_T gtEngineCmdMsgPipeApi = JunoDs_QueueApiInit(SetAt, GetAt, RemoveAt);

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

static JUNO_STATUS_T EngineCmdMsg_Reset(JUNO_POINTER_T tPointer)
{
    // Verify the pointer
    JUNO_STATUS_T tStatus = EngineCmdMsg_PointerVerify(tPointer);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Perform the reset
    *(ENGINE_CMD_MSG_T *) tPointer.pvAddr = (ENGINE_CMD_MSG_T){0};
    return tStatus;
}


/// Asserts the api is for the pipe
#define ENGINE_CMD_MSG_PIPE_ASSERT_API(ptArray, ...)  if(ptArray->ptApi != &gtEngineCmdMsgPipeApi.tRoot) { __VA_ARGS__; }

JUNO_STATUS_T EngineCmdMsg_PipeInit(ENGINE_CMD_MSG_PIPE_T *ptEngineCmdMsgPipe, ENGINE_CMD_MSG_T *ptArrEngineCmdMsgBuffer, size_t iCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvUserData)
{
    // Assert the msg pipe exists
    JUNO_ASSERT_EXISTS(ptEngineCmdMsgPipe && ptArrEngineCmdMsgBuffer);
    // Set the message buffer
    ptEngineCmdMsgPipe->ptArrEngineCmdMsgBuffer = ptArrEngineCmdMsgBuffer;
    // init the pipe
    JUNO_STATUS_T tStatus = JunoSb_PipeInit(&ptEngineCmdMsgPipe->tRoot, &gtEngineCmdMsgPipeApi, ENGINE_CMD_MSG_MID, iCapacity, pfcnFailureHdlr, pvUserData);
    return tStatus;
}

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
    ENGINE_CMD_MSG_PIPE_T *ptEngineCmdMsgPipe = (ENGINE_CMD_MSG_PIPE_T *)ptArray;
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
    ENGINE_CMD_MSG_PIPE_T *ptEngineCmdMsgPipe = (ENGINE_CMD_MSG_PIPE_T *)ptArray;
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
    ENGINE_CMD_MSG_PIPE_T *ptEngineCmdMsgPipe = (ENGINE_CMD_MSG_PIPE_T *)ptArray;
    // Create pointer to memory
    JUNO_POINTER_T tIndexPointer = EngineCmdMsg_PointerInit(&ptEngineCmdMsgPipe->ptArrEngineCmdMsgBuffer[iIndex]);
    // Reset the memory
    tStatus = tIndexPointer.ptApi->Reset(tIndexPointer);
    return tStatus;
}
