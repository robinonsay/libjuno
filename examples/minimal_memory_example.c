#include "juno/memory/memory.h"
#include "juno/memory/memory_types.h"
#include "juno/status.h"
#include <stdio.h>

// Define a simple data structure
typedef struct {
    int id;
    char name[32];
    float value;
} USER_DATA_T;

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
    JUNO_MEMORY_ALLOC_T tMemAlloc = {0};
    JUNO_STATUS_T tStatus = Juno_MemoryBlkInit(
        &tMemAlloc.tBlock,
        gUserDataMemory,
        gUserDataMetadata,
        sizeof(USER_DATA_T),
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
    JUNO_MEMORY_T tMemory = {0};
    tStatus = Juno_MemoryGet(&tMemAlloc, &tMemory, sizeof(USER_DATA_T));
    
    if (tStatus != JUNO_STATUS_SUCCESS) {
        printf("Failed to allocate memory\n");
        return -1;
    }
    
    printf("Memory allocated successfully\n");
    
    // Step 3: Use the memory
    USER_DATA_T *pUserData = (USER_DATA_T *)tMemory.pvAddr;
    pUserData->id = 42;
    pUserData->value = 3.14f;
    snprintf(pUserData->name, sizeof(pUserData->name), "Example");
    
    printf("Data stored: ID=%d, Name=%s, Value=%.2f\n", 
           pUserData->id, pUserData->name, pUserData->value);
    
    // Step 4: Create a reference to share the memory
    printf("\nCreating a reference to memory...\n");
    JUNO_NEW_REF(userDataRef) = Juno_MemoryGetRef(&tMemory);
    
    printf("Reference count: %zu\n", tMemory.iRefCount);
    
    // Access through the reference
    USER_DATA_T *pSharedData = (USER_DATA_T *)JUNO_REF(userDataRef)->pvAddr;
    printf("Accessed via reference: ID=%d, Name=%s, Value=%.2f\n", 
           pSharedData->id, pSharedData->name, pSharedData->value);
    
    // Step 5: Try to free memory while reference exists
    printf("\nAttempting to free memory with active references...\n");
    tStatus = Juno_MemoryPut(&tMemAlloc, &tMemory);
    
    if (tStatus == JUNO_STATUS_REF_IN_USE_ERROR) {
        printf("As expected, could not free memory with active references\n");
    }
    
    // Step 6: Release reference and try again
    printf("\nReleasing reference...\n");
    Juno_MemoryPutRef(JUNO_REF(userDataRef));
    printf("Reference count after release: %zu\n", tMemory.iRefCount);
    
    printf("Freeing memory...\n");
    tStatus = Juno_MemoryPut(&tMemAlloc, &tMemory);
    
    if (tStatus == JUNO_STATUS_SUCCESS) {
        printf("Memory freed successfully\n");
    } else {
        printf("Failed to free memory: %d\n", tStatus);
    }
    
    return 0;
}