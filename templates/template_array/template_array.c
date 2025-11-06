#include "template_array.h"
#include "juno/ds/array_api.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include "juno/status.h"

static JUNO_STATUS_T Template_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
static JUNO_STATUS_T Template_Reset(JUNO_POINTER_T tPointer);

/// Set the value at an index
static JUNO_STATUS_T SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
/// Get the value at an index
static JUNO_RESULT_POINTER_T GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
/// Remove a value at an index
static JUNO_STATUS_T RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);

// Instantiate the template  pointer api
const JUNO_POINTER_API_T gtTemplatePointerApi =
{
    Template_Copy,
    Template_Reset
};

// Instantiate the template  array api
static const JUNO_DS_ARRAY_API_T gtTemplateArrayApi =
{
    SetAt, GetAt, RemoveAt
};

static JUNO_STATUS_T Template_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc)
{
    // Verify the dest pointer
    JUNO_STATUS_T tStatus = Template_PointerVerify(tDest);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Verify the src pointer
    tStatus = Template_PointerVerify(tSrc);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Perform the copy
    *(TEMPLATE_T *) tDest.pvAddr = *(TEMPLATE_T *) tSrc.pvAddr;
    return tStatus;
}

static JUNO_STATUS_T Template_Reset(JUNO_POINTER_T tPointer)
{
    // Verify the pointer
    JUNO_STATUS_T tStatus = Template_PointerVerify(tPointer);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Perform the reset
    *(TEMPLATE_T *) tPointer.pvAddr = (TEMPLATE_T){0};
    return tStatus;
}


/// Asserts the api is for the array
#define TEMPLATE_ARRAY_ASSERT_API(ptArray, ...)  if(ptArray->ptApi != &gtTemplateArrayApi) { __VA_ARGS__; }

JUNO_STATUS_T Template_ArrayInit(TEMPLATE_ARRAY_T *ptTemplateArray, TEMPLATE_T *ptArrTemplateBuffer, size_t iCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvUserData)
{
    // Assert the  array exists
    JUNO_ASSERT_EXISTS(ptTemplateArray && ptArrTemplateBuffer);
    // Set the message buffer
    ptTemplateArray->ptArrTemplateBuffer = ptArrTemplateBuffer;
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(&ptTemplateArray->tRoot, &gtTemplateArrayApi, iCapacity, pfcnFailureHdlr, pvUserData);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // init the array
    return tStatus;
}

static JUNO_STATUS_T SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex)
{
    // Verify the array
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Assert the api
    TEMPLATE_ARRAY_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    // Verify the pointer
    tStatus = Template_PointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Check if the index is valid
    tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Cast to the array type
    TEMPLATE_ARRAY_T *ptTemplateArray = (TEMPLATE_ARRAY_T *)ptArray;
    // Init the pointer to the buffer
    JUNO_POINTER_T tIndexPointer = Template_PointerInit(&ptTemplateArray->ptArrTemplateBuffer[iIndex]);
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
    TEMPLATE_ARRAY_ASSERT_API(ptArray,
        tResult.tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        return tResult;
    );
    // Check the index
    tResult.tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    // Cast to the array type
    TEMPLATE_ARRAY_T *ptTemplateArray = (TEMPLATE_ARRAY_T *)ptArray;
    // Create the pointer to the buffer
    JUNO_POINTER_T tIndexPointer = Template_PointerInit(&ptTemplateArray->ptArrTemplateBuffer[iIndex]);
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
    TEMPLATE_ARRAY_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    // Verify the index
    tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    // Cast to the  array type
    TEMPLATE_ARRAY_T *ptTemplateArray = (TEMPLATE_ARRAY_T *)ptArray;
    // Create pointer to memory
    JUNO_POINTER_T tIndexPointer = Template_PointerInit(&ptTemplateArray->ptArrTemplateBuffer[iIndex]);
    // Reset the memory
    tStatus = tIndexPointer.ptApi->Reset(tIndexPointer);
    return tStatus;
}
