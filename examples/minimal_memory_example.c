#include "juno/macros.h"
#include "juno/memory/memory_api.h"
#define JUNO_MEMORY_DEFAULT
#include "juno/memory/memory_block.h"
#include "juno/status.h"
#include <stdio.h>
#include <stdalign.h>

// Define a simple data structure
typedef struct USER_DATA_T {
    int id;
    char name[32];
    float value;
} USER_DATA_T;

// Define the pointer api for this type
static JUNO_STATUS_T UserDataCopy(JUNO_POINTER_T tDest, JUNO_POINTER_T tSrc)
{
    JUNO_STATUS_T tStatus = JUNO_CHECK_POINTER_TYPE(tDest, USER_DATA_T);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JUNO_CHECK_POINTER_TYPE(tSrc, USER_DATA_T);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    USER_DATA_T *ptDest = (USER_DATA_T *)tDest.pvAddr;
    USER_DATA_T *ptSrc = (USER_DATA_T *)tSrc.pvAddr;
    *ptDest = *ptSrc;
    return tStatus;
}

/// Reset the memory at the pointer. This could mean zero-initialization
static JUNO_STATUS_T UserDataReset(JUNO_POINTER_T tPointer)
{
    JUNO_STATUS_T tStatus = JUNO_CHECK_POINTER_TYPE(tPointer, USER_DATA_T);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    USER_DATA_T *ptBlock = (USER_DATA_T *)tPointer.pvAddr;
    *ptBlock = (USER_DATA_T){0};
    return tStatus;
}

const JUNO_POINTER_API_T gtUserDataPointerApi = {
    UserDataCopy,
    UserDataReset
};


// Declare memory block for 5 USER_DATA_T objects
JUNO_MEMORY_BLOCK(gUserDataMemory, USER_DATA_T, 5);
JUNO_MEMORY_BLOCK_METADATA(gUserDataMetadata, 5);

// Simple error handler
void ErrorHandler(JUNO_STATUS_T tStatus, const char *pcMsg, JUNO_USER_DATA_T *pvUserData) {
    (void)pvUserData; // Unused
    printf("Error %d: %s\n", tStatus, pcMsg);
}

int main(void) {
    printf("Juno Memory Module - Minimal Example\n");
    printf("------------------------------------\n\n");
    
    // Step 1: Initialize the memory allocator
    JUNO_MEMORY_ALLOC_BLOCK_T tMemAlloc = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockInit(
        &tMemAlloc,
        &gtUserDataPointerApi,
        gUserDataMemory,
        gUserDataMetadata,
        sizeof(USER_DATA_T),
        alignof(USER_DATA_T),
        5,
        ErrorHandler,
        NULL
    );
    
    if (tStatus != JUNO_STATUS_SUCCESS) {
        printf("Failed to initialize memory block\n");
        return -1;
    }
    
    printf("Memory block initialized successfully\n");
    
    // Step 2: Allocate memory
    JUNO_POINTER_T tMemory = {0};
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMemAlloc.tRoot.ptApi;
    JUNO_RESULT_POINTER_T tPointerResult = ptApi->Get(&tMemAlloc.tRoot, sizeof(USER_DATA_T));
    JUNO_ASSERT_SUCCESS(tPointerResult.tStatus, return -1);
    tMemory = tPointerResult.tOk;
    printf("Memory allocated successfully\n");
    
    // Step 3: Use the memory
    USER_DATA_T *pUserData = (USER_DATA_T *)tMemory.pvAddr;
    pUserData->id = 42;
    pUserData->value = 3.14f;
    snprintf(pUserData->name, sizeof(pUserData->name), "Example");
    
    printf("Data stored: ID=%d, Name=%s, Value=%.2f\n", 
           pUserData->id, pUserData->name, pUserData->value);


    printf("Freeing memory...\n");
    tStatus = ptApi->Put(&tMemAlloc.tRoot,  &tMemory);
    
    if (tStatus == JUNO_STATUS_SUCCESS) {
        printf("Memory freed successfully\n");
    } else {
        printf("Failed to free memory: %d\n", tStatus);
    }
    
    return 0;
}
