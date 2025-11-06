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

#include "engine_app/engine_tlm_msg.h"
#include "juno/ds/array_api.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include "juno/status.h"

static JUNO_STATUS_T EngineTlmMsg_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
static JUNO_STATUS_T EngineTlmMsg_Reset(JUNO_POINTER_T tPointer);

/// Set the value at an index
static JUNO_STATUS_T SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
/// Get the value at an index
static JUNO_RESULT_POINTER_T GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
/// Remove a value at an index
static JUNO_STATUS_T RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);

// Instantiate the engine_tlm msg pointer api
const JUNO_POINTER_API_T gtEngineTlmMsgPointerApi =
{
    EngineTlmMsg_Copy,
    EngineTlmMsg_Reset
};

// Instantiate the engine_tlm msg pipe api
static const JUNO_DS_ARRAY_API_T gtEngineTlmMsgPipeApi =
{
    SetAt, GetAt, RemoveAt
};

static JUNO_STATUS_T EngineTlmMsg_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc)
{
    // Verify the dest pointer
    JUNO_STATUS_T tStatus = EngineTlmMsg_PointerVerify(tDest);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Verify the src pointer
    tStatus = EngineTlmMsg_PointerVerify(tSrc);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Perform the copy
    *(ENGINE_TLM_MSG_T *) tDest.pvAddr = *(ENGINE_TLM_MSG_T *) tSrc.pvAddr;
    return tStatus;
}

static JUNO_STATUS_T EngineTlmMsg_Reset(JUNO_POINTER_T tPointer)
{
    // Verify the pointer
    JUNO_STATUS_T tStatus = EngineTlmMsg_PointerVerify(tPointer);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Perform the reset
    *(ENGINE_TLM_MSG_T *) tPointer.pvAddr = (ENGINE_TLM_MSG_T){0};
    return tStatus;
}


/// Asserts the api is for the pipe
#define ENGINE_TLM_MSG_PIPE_ASSERT_API(ptArray, ...)  if(ptArray->ptApi != &gtEngineTlmMsgPipeApi) { __VA_ARGS__; }

JUNO_STATUS_T EngineTlmMsg_ArrayInit(ENGINE_TLM_MSG_ARRAY_T *ptEngineTlmMsgPipe, ENGINE_TLM_MSG_T *ptArrEngineTlmMsgBuffer, size_t iCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvUserData)
{
    // Assert the msg pipe exists
    JUNO_ASSERT_EXISTS(ptEngineTlmMsgPipe && ptArrEngineTlmMsgBuffer);
    // Set the message buffer
    ptEngineTlmMsgPipe->ptArrEngineTlmMsgBuffer = ptArrEngineTlmMsgBuffer;
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(&ptEngineTlmMsgPipe->tRoot, &gtEngineTlmMsgPipeApi, iCapacity, pfcnFailureHdlr, pvUserData);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // init the pipe
    return tStatus;
}

static JUNO_STATUS_T SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex)
{
    // Verify the array
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Assert the api
    ENGINE_TLM_MSG_PIPE_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    // Verify the pointer
    tStatus = EngineTlmMsg_PointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Check if the index is valid
    tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Cast to the pipe type
    ENGINE_TLM_MSG_ARRAY_T *ptEngineTlmMsgPipe = (ENGINE_TLM_MSG_ARRAY_T *)ptArray;
    // Init the pointer to the buffer
    JUNO_POINTER_T tIndexPointer = EngineTlmMsg_PointerInit(&ptEngineTlmMsgPipe->ptArrEngineTlmMsgBuffer[iIndex]);
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
    ENGINE_TLM_MSG_PIPE_ASSERT_API(ptArray,
        tResult.tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        return tResult;
    );
    // Check the index
    tResult.tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    // Cast to the pipe type
    ENGINE_TLM_MSG_ARRAY_T *ptEngineTlmMsgPipe = (ENGINE_TLM_MSG_ARRAY_T *)ptArray;
    // Create the pointer to the buffer
    JUNO_POINTER_T tIndexPointer = EngineTlmMsg_PointerInit(&ptEngineTlmMsgPipe->ptArrEngineTlmMsgBuffer[iIndex]);
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
    ENGINE_TLM_MSG_PIPE_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    // Verify the index
    tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Cast to the msg pipe type
    ENGINE_TLM_MSG_ARRAY_T *ptEngineTlmMsgPipe = (ENGINE_TLM_MSG_ARRAY_T *)ptArray;
    // Create pointer to memory
    JUNO_POINTER_T tIndexPointer = EngineTlmMsg_PointerInit(&ptEngineTlmMsgPipe->ptArrEngineTlmMsgBuffer[iIndex]);
    // Reset the memory
    tStatus = tIndexPointer.ptApi->Reset(tIndexPointer);
    return tStatus;
}
