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

#include "juno/ds/array_api.h"
#include "juno/ds/queue_api.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include "juno/module.h"
#include "juno/status.h"
#include <stddef.h>
#include <stdio.h>
#include <stdalign.h>
#include <string.h>

/**DOC
# The Queue Example

In this example we will implement a queue. Queue's are
intended to be memory safe in LibJuno so we need to implement
an array for our type that tell's the queue how to get, set, and
remove values. We also need to implement a pointer API for this
type to tell the queue how to safely access the memory.

For Context:
* The pointer API answers the question: "How should LibJuno access my type safely?"
* The array API answers the question: "How should I index into my array safely?"

The Queue uses the answers to these two questions to implement a queue. A
similar paradigm is used on the stack, map, and heap APIs.

Typically users will use the `scrips/create_array.py` script
to generate the boilerplate for arrays and pointers.

First we will define out type, and derive an array.
*/
// Define a simple data structure
typedef struct USER_DATA_T {
    int id;
    char name[32];
    float value;
} USER_DATA_T;

typedef struct USER_DATA_BUFFER_T JUNO_MODULE_DERIVE(JUNO_DS_ARRAY_ROOT_T,
    USER_DATA_T tBuffer[100];
) USER_DATA_BUFFER_T;

/**DOC
Then we will forward declare the functions we need to implement
and define the APIs.
*/
static JUNO_STATUS_T UserDataCopy(JUNO_POINTER_T tDest, JUNO_POINTER_T tSrc);
static JUNO_STATUS_T UserDataReset(JUNO_POINTER_T tPointer);
static JUNO_STATUS_T UserDataSetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
static JUNO_RESULT_POINTER_T UserDataGetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
static JUNO_STATUS_T UserDataRemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);

const JUNO_POINTER_API_T gtUserDataPointerApi = {
    UserDataCopy,
    UserDataReset
};

const JUNO_DS_ARRAY_API_T gtUserDataArrayApi = {
    UserDataSetAt,
    UserDataGetAt,
    UserDataRemoveAt
};

/**DOC
We then need to implement the Copy and Reset functions
for the pointer. We will verify the type matches
and safely dereference the pointer to perform the copy and reset.
*/
// Define the pointer api for this type
static JUNO_STATUS_T UserDataCopy(JUNO_POINTER_T tDest, JUNO_POINTER_T tSrc)
{
    JUNO_STATUS_T tStatus = JunoMemory_PointerVerifyType(tDest, USER_DATA_T, gtUserDataPointerApi);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerifyType(tSrc, USER_DATA_T, gtUserDataPointerApi);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    USER_DATA_T *ptDest = (USER_DATA_T *)tDest.pvAddr;
    USER_DATA_T *ptSrc = (USER_DATA_T *)tSrc.pvAddr;
    *ptDest = *ptSrc;
    return tStatus;
}

/// Reset the memory at the pointer. This could mean zero-initialization
static JUNO_STATUS_T UserDataReset(JUNO_POINTER_T tPointer)
{
    JUNO_STATUS_T tStatus = JunoMemory_PointerVerifyType(tPointer, USER_DATA_T, gtUserDataPointerApi);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    USER_DATA_T *ptBlock = (USER_DATA_T *)tPointer.pvAddr;
    *ptBlock = (USER_DATA_T){0};
    return tStatus;
}

/**DOC
Then we need to define the array access. Again we will verify the type
and perform the actions using the pointer API we defined.
*/
/// Set the value at an index
static JUNO_STATUS_T UserDataSetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoMemory_PointerVerifyType(tItem, USER_DATA_T, gtUserDataPointerApi);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    USER_DATA_BUFFER_T *ptMyArray = (USER_DATA_BUFFER_T *) ptArray;
    JUNO_POINTER_T tDest = JunoMemory_PointerInit(&gtUserDataPointerApi, USER_DATA_T, &ptMyArray->tBuffer[iIndex]);
    return tDest.ptApi->Copy(tDest, tItem);
}

/// Get the value at an index
static JUNO_RESULT_POINTER_T UserDataGetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_RESULT_POINTER_T tResult = {0};
    tResult.tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    tResult.tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    USER_DATA_BUFFER_T *ptMyArray = (USER_DATA_BUFFER_T *) ptArray;
    JUNO_POINTER_T tRet = JunoMemory_PointerInit(&gtUserDataPointerApi, USER_DATA_T, &ptMyArray->tBuffer[iIndex]);
    tResult.tOk = tRet;
    return tResult;
}

/// Remove a value at an index
static JUNO_STATUS_T UserDataRemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_STATUS_T tStatus = {0};
    tStatus= JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus= JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    USER_DATA_BUFFER_T *ptMyArray = (USER_DATA_BUFFER_T *) ptArray;
    JUNO_POINTER_T tRet = JunoMemory_PointerInit(&gtUserDataPointerApi, USER_DATA_T, &ptMyArray->tBuffer[iIndex]);
    tStatus = tRet.ptApi->Reset(tRet);
    return tStatus;
}

/**DOC
We will also implement an error handler. This will automatically get called when
LibJuno encounters an error. In this case we will print the message and handle
 the case when the queue is simple empty or full.
*/
// Simple error handler
void ErrorHandler(JUNO_STATUS_T tStatus, const char *pcMsg, JUNO_USER_DATA_T *pvUserData) {
    (void)pvUserData; // Unused
    if(tStatus == JUNO_STATUS_OOB_ERROR)
    {
        printf("Queue Full or Empty!\n");
    }
    else
    {
        printf("Error %d: %s\n", tStatus, pcMsg);
    }
}

/**DOC
Next we need to init the array and queue
*/
int main(void) {
    printf("Juno Queue Module - Minimal Example\n");
    printf("------------------------------------\n\n");
    
    // Step 1: Initialize the array and queue
    USER_DATA_BUFFER_T tMyArray = {0};
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(&tMyArray.tRoot, &gtUserDataArrayApi, sizeof(tMyArray.tBuffer)/sizeof(tMyArray.tBuffer[0]), ErrorHandler, NULL);
    JUNO_ASSERT_SUCCESS(tStatus,
        printf("Failed to initialize Array\n");
        return -1
    );
    JUNO_DS_QUEUE_ROOT_T tMyQueue = {0};
    tStatus = JunoDs_QueueInit(&tMyQueue, &tMyArray.tRoot, ErrorHandler, NULL);
    JUNO_ASSERT_SUCCESS(tStatus,
        printf("Failed to initialize Queue\n");
        return -1;
    );
    
    printf("Queue initialized successfully\n");
/**DOC
Finally we will enqueue the array until it is full. We know it's full when it
returns an error.
*/
    int i = 0;
    while (tStatus == JUNO_STATUS_SUCCESS)
    {

        USER_DATA_T tMyData = {i, "", 1.2 * i};
        snprintf(tMyData.name, sizeof(tMyData.name), "Hell %i", i);
        JUNO_POINTER_T tMyPointer = JunoMemory_PointerInit(&gtUserDataPointerApi, USER_DATA_T, &tMyData);
        tStatus = tMyQueue.ptApi->Enqueue(&tMyQueue, tMyPointer);
        i += 1;
    }
/**DOC
Then we will dequeue the array.
*/
    tStatus = JUNO_STATUS_SUCCESS;
    while(tStatus == JUNO_STATUS_SUCCESS)
    {
        USER_DATA_T tMyData = {0};
        JUNO_POINTER_T tRetPointer = JunoMemory_PointerInit(&gtUserDataPointerApi, USER_DATA_T, &tMyData);
        tStatus = tMyQueue.ptApi->Dequeue(&tMyQueue, tRetPointer);
        printf("User Data: %i, %s, %f\n", tMyData.id, tMyData.name, tMyData.value);
    }
    return 0;
}
/**DOC
In conclusion we implemented a type-specific Array and pointer and used
that to utilize the Queue module within LibJuno.
*/
