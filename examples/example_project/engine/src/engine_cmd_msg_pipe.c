#include "engine_app/engine_cmd_msg_pipe.h"
#include "engine_app/engine_cmd_msg.h"
#include "juno/ds/array_api.h"
#include "juno/ds/queue_api.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include "juno/sb/broker_api.h"
#include "juno/status.h"

/// Set the value at an index
static JUNO_STATUS_T SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
/// Get the value at an index
static JUNO_RESULT_POINTER_T GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
/// Remove a value at an index
static JUNO_STATUS_T RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);

static const JUNO_DS_QUEUE_API_T gtCmdPipeApi = JunoDs_QueueApiInit(SetAt, GetAt, RemoveAt);



JUNO_STATUS_T EngineCmdMsgPipeInit(ENGINE_CMD_MSG_PIPE_T *ptCmdPipe, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvUserData)
{
    JUNO_ASSERT_EXISTS(ptCmdPipe);
    JUNO_STATUS_T tStatus = JunoSb_PipeInit(&ptCmdPipe->tRoot, &gtCmdPipeApi, ENGINE_CMD_MSG_MID, ENGINE_CMD_MSG_PIPE_DEPTH, pfcnFailureHdlr, pvUserData);
    return tStatus;
}

static JUNO_STATUS_T SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_ASSERT_POINTER(tStatus, tItem, ENGINE_CMD_MSG_T, gtEngineCmdMsgPointerApi);
    tStatus = JunoDs_ArrayCheckIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    ENGINE_CMD_MSG_PIPE_T *ptCmdPipe = (ENGINE_CMD_MSG_PIPE_T *)ptArray;
    JUNO_POINTER_T tIndexPointer = JunoMemory_PointerInit(&gtEngineCmdMsgPointerApi, ENGINE_CMD_MSG_T, &ptCmdPipe->ptArrCmdPipe[iIndex]);
    tStatus = tIndexPointer.ptApi->Copy(tIndexPointer, tItem);
    return tStatus;
}
/// Get the value at an index
static JUNO_RESULT_POINTER_T GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_RESULT_POINTER_T tResult = {0};
    tResult.tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    tResult.tStatus = JunoDs_ArrayCheckIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    ENGINE_CMD_MSG_PIPE_T *ptCmdPipe = (ENGINE_CMD_MSG_PIPE_T *)ptArray;
    JUNO_POINTER_T tIndexPointer = JunoMemory_PointerInit(&gtEngineCmdMsgPointerApi, ENGINE_CMD_MSG_T, &ptCmdPipe->ptArrCmdPipe[iIndex]);
    tResult.tOk = tIndexPointer;
    return tResult;
}
/// Remove a value at an index
static JUNO_STATUS_T RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoDs_ArrayCheckIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    ENGINE_CMD_MSG_PIPE_T *ptCmdPipe = (ENGINE_CMD_MSG_PIPE_T *)ptArray;
    JUNO_POINTER_T tIndexPointer = JunoMemory_PointerInit(&gtEngineCmdMsgPointerApi, ENGINE_CMD_MSG_T, &ptCmdPipe->ptArrCmdPipe[iIndex]);
    tStatus = tIndexPointer.ptApi->Reset(tIndexPointer);
    return tStatus;
}
