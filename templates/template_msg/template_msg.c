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

#include "template_msg.h"
#include "juno/ds/array_api.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include "juno/status.h"

static JUNO_STATUS_T TemplateMsg_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
static JUNO_STATUS_T TemplateMsg_Reset(JUNO_POINTER_T tPointer);

/// Set the value at an index
static JUNO_STATUS_T SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
/// Get the value at an index
static JUNO_RESULT_POINTER_T GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
/// Remove a value at an index
static JUNO_STATUS_T RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);

// Instantiate the template msg pointer api
const JUNO_POINTER_API_T gtTemplateMsgPointerApi =
{
    TemplateMsg_Copy,
    TemplateMsg_Reset
};

// Instantiate the template msg pipe api
static const JUNO_DS_ARRAY_API_T gtTemplateMsgPipeApi =
{
    SetAt, GetAt, RemoveAt
};

static JUNO_STATUS_T TemplateMsg_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc)
{
    // Verify the dest pointer
    JUNO_STATUS_T tStatus = TemplateMsg_PointerVerify(tDest);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Verify the src pointer
    tStatus = TemplateMsg_PointerVerify(tSrc);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Perform the copy
    *(TEMPLATE_MSG_T *) tDest.pvAddr = *(TEMPLATE_MSG_T *) tSrc.pvAddr;
    return tStatus;
}

static JUNO_STATUS_T TemplateMsg_Reset(JUNO_POINTER_T tPointer)
{
    // Verify the pointer
    JUNO_STATUS_T tStatus = TemplateMsg_PointerVerify(tPointer);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Perform the reset
    *(TEMPLATE_MSG_T *) tPointer.pvAddr = (TEMPLATE_MSG_T){0};
    return tStatus;
}


/// Asserts the api is for the pipe
#define TEMPLATE_MSG_PIPE_ASSERT_API(ptArray, ...)  if(ptArray->ptApi != &gtTemplateMsgPipeApi) { __VA_ARGS__; }

JUNO_STATUS_T TemplateMsg_ArrayInit(TEMPLATE_MSG_ARRAY_T *ptTemplateMsgPipe, TEMPLATE_MSG_T *ptArrTemplateMsgBuffer, size_t iCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvUserData)
{
    // Assert the msg pipe exists
    JUNO_ASSERT_EXISTS(ptTemplateMsgPipe && ptArrTemplateMsgBuffer);
    // Set the message buffer
    ptTemplateMsgPipe->ptArrTemplateMsgBuffer = ptArrTemplateMsgBuffer;
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(&ptTemplateMsgPipe->tRoot, &gtTemplateMsgPipeApi, iCapacity, pfcnFailureHdlr, pvUserData);
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
    TEMPLATE_MSG_PIPE_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    // Verify the pointer
    tStatus = TemplateMsg_PointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Check if the index is valid
    tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Cast to the pipe type
    TEMPLATE_MSG_ARRAY_T *ptTemplateMsgPipe = (TEMPLATE_MSG_ARRAY_T *)ptArray;
    // Init the pointer to the buffer
    JUNO_POINTER_T tIndexPointer = TemplateMsg_PointerInit(&ptTemplateMsgPipe->ptArrTemplateMsgBuffer[iIndex]);
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
    TEMPLATE_MSG_PIPE_ASSERT_API(ptArray,
        tResult.tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        return tResult;
    );
    // Check the index
    tResult.tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    // Cast to the pipe type
    TEMPLATE_MSG_ARRAY_T *ptTemplateMsgPipe = (TEMPLATE_MSG_ARRAY_T *)ptArray;
    // Create the pointer to the buffer
    JUNO_POINTER_T tIndexPointer = TemplateMsg_PointerInit(&ptTemplateMsgPipe->ptArrTemplateMsgBuffer[iIndex]);
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
    TEMPLATE_MSG_PIPE_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    // Verify the index
    tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Cast to the msg pipe type
    TEMPLATE_MSG_ARRAY_T *ptTemplateMsgPipe = (TEMPLATE_MSG_ARRAY_T *)ptArray;
    // Create pointer to memory
    JUNO_POINTER_T tIndexPointer = TemplateMsg_PointerInit(&ptTemplateMsgPipe->ptArrTemplateMsgBuffer[iIndex]);
    // Reset the memory
    tStatus = tIndexPointer.ptApi->Reset(tIndexPointer);
    return tStatus;
}
