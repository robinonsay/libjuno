#include "juno/memory/memory_api.h"
#define JUNO_MEMORY_DEFAULT
#include "juno/memory/memory_block.h"
#include "juno/status.h"
#include <stdio.h>

/// A SLL node
typedef struct SINGLE_LINKED_LIST_NODE_TAG SINGLE_LINKED_LIST_NODE_T;
/// THe SLL
typedef struct SINGLE_LINKED_LIST_TAG SINGLE_LINKED_LIST_T;

struct SINGLE_LINKED_LIST_NODE_TAG
{
    /// The next node in the SLL
    SINGLE_LINKED_LIST_NODE_T *ptNext;
    /// The Juno memory since the node owns it
    JUNO_MEMORY_T tMemory;
    /// The Example data in the node
    int iExampleData;
};

struct SINGLE_LINKED_LIST_TAG
{
    /// The start of the SLL
    SINGLE_LINKED_LIST_NODE_T *ptStart;
    /// The length of the SLL
    size_t zLen;
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler;
    JUNO_USER_DATA_T *pvFailureUserData;
};

/// The memory for the SLL. Supports a max of 100 nodes
JUNO_MEMORY_BLOCK(ptSllMemory, SINGLE_LINKED_LIST_NODE_T, 100);
/// The metadata for the SLL
JUNO_MEMORY_BLOCK_METADATA(ptSllMemoryMetadata, 100);

void FailureHandler(JUNO_STATUS_T tStatus, const char *pcMsg, JUNO_USER_DATA_T *ptUserData)
{
    // We don't have user data
    (void)ptUserData;
    printf("Memory operation failed: %s\n", pcMsg);
}

static JUNO_STATUS_T Push(JUNO_MEMORY_ALLOC_T *ptAlloc, SINGLE_LINKED_LIST_T *ptSll, int iData)
{
    // Initialize the status with success
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    // Initialize the current node with the start of the SLL
    SINGLE_LINKED_LIST_NODE_T *ptThisNode = ptSll->ptStart;
    // Check if the current node has been initialized
    if(!ptThisNode)
    {
        // Create a new node and return the status
        // Allocate the memory for the first node
        JUNO_MEMORY_T tMem = {};
        const JUNO_MEMORY_ALLOC_API_T *ptApi = ptAlloc->tBase.ptApi;
        JUNO_STATUS_T tStatus = ptApi->Get(ptAlloc, &tMem, sizeof(SINGLE_LINKED_LIST_NODE_T));
        if(tStatus) return tStatus;
        // Using the memory directly since the SLL will own this memory
        ptThisNode = (SINGLE_LINKED_LIST_NODE_T *) tMem.pvAddr;
        // Set the data
        ptThisNode->iExampleData = iData;
        // Give the SLL Node the memory
        ptThisNode->tMemory = tMem;
        ptSll->ptStart = ptThisNode;
        // Return the status
        ptSll->zLen += 1;
        return tStatus;
    }
    for(size_t i = 0; i < ptSll->zLen && ptThisNode->ptNext; ++i)
    {
        ptThisNode = ptThisNode->ptNext;
    }
    // Allocate the memory for the first node
    JUNO_MEMORY_T tMem = {};
    const JUNO_MEMORY_ALLOC_API_T *ptApi = ptAlloc->tBase.ptApi;
    tStatus = ptApi->Get(ptAlloc, &tMem, sizeof(SINGLE_LINKED_LIST_NODE_T));
    if(tStatus) return tStatus;
    // Using the memory directly since the SLL will own this memory
    ptThisNode->ptNext = (SINGLE_LINKED_LIST_NODE_T *) tMem.pvAddr;
    // Set the data
    ptThisNode->ptNext->iExampleData = iData;
    // Give the SLL Node the memory
    ptThisNode->ptNext->tMemory = tMem;
    // Return the status
    ptSll->zLen += 1;
    return tStatus;
}

static JUNO_STATUS_T Pop(JUNO_MEMORY_ALLOC_T *ptAlloc, SINGLE_LINKED_LIST_T *ptSll, int *piRetData)
{
    // Initialize the status with success
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    // Check if the return data is null
    if(!piRetData)
    {
        // Return data is null, call failure handler
        tStatus = JUNO_STATUS_NULLPTR_ERROR;
        FAIL(tStatus, ptSll->pfcnFailureHandler, ptSll->pvFailureUserData, "Return data is null");
        return tStatus;
    }
    // Initialize the current node with the start of the SLL
    SINGLE_LINKED_LIST_NODE_T *ptThisNode = ptSll->ptStart;
    // Check if the current node has been initialized
    if(!ptThisNode)
    {
        // Create a new node and return the status
        return JUNO_STATUS_DNE_ERROR;
    }
    ptSll->ptStart = ptThisNode->ptNext;
    *piRetData = ptThisNode->iExampleData;
    const JUNO_MEMORY_ALLOC_API_T *ptApi = ptAlloc->tBase.ptApi;
    tStatus = ptApi->Put(ptAlloc, &ptThisNode->tMemory);
    return tStatus;
}

int main()
{
    // Instantiate the memory API
    // The memory allocator
    JUNO_MEMORY_ALLOC_T tMemAlloc = {};
    // Initialize the block allocator
    JUNO_STATUS_T tStatus = JunoMemory_BlockApi(
        &tMemAlloc,
        ptSllMemory,
        ptSllMemoryMetadata,
        sizeof(SINGLE_LINKED_LIST_NODE_T),
        100,
        FailureHandler,
        NULL
    );
    // If the init has a failed status (which is non-zero) exit
    // The failure handler automatically gets calles
    if(tStatus) return -1;
    // Create the linked list with the failure handler
    SINGLE_LINKED_LIST_T tSll = {
        .pfcnFailureHandler = FailureHandler,
        .pvFailureUserData = NULL
    };
    // Create another linked list with references
    SINGLE_LINKED_LIST_T tSll2 = {
        .pfcnFailureHandler = FailureHandler,
        .pvFailureUserData = NULL
    };
    // Push example data onto the queue
    for(int i = 10; i < 20; i++)
    {
        Push(&tMemAlloc, &tSll, i);
    }
    // Copy the data to the SLL by reference
    JUNO_NEW_REF(ptNodeRef) = Juno_MemoryGetRef(&tSll.ptStart->tMemory);
    tSll2.ptStart = (SINGLE_LINKED_LIST_NODE_T *) JUNO_REF(ptNodeRef->pvAddr);
    tSll2.zLen = tSll.zLen;
    // Print the values in the queue and sll 2
    SINGLE_LINKED_LIST_NODE_T *ptThisNode = tSll.ptStart;
    SINGLE_LINKED_LIST_NODE_T *ptThisStackNode = tSll2.ptStart;
    for(size_t i = 0; i < tSll.zLen && i < tSll2.zLen; i++)
    {
        printf("SLL 1 | Node: %lu | Data: %i\n", i, ptThisNode->iExampleData);
        printf("SLL 2 | Node: %lu | Data: %i\n", i, ptThisStackNode->iExampleData);
        ptThisNode = ptThisNode->ptNext;
        ptThisStackNode = ptThisStackNode->ptNext;
    }
    // Attempt to free memory in first SLL. This will fail since
    // the references are in use
    printf("--------------------------\n");
    printf("Attempting to free memory in use.\nThis should fail and call failure handler\n"
    );
    printf("--------------------------\n");
    int iRetData = 0;
    tStatus = Pop(&tMemAlloc, &tSll, &iRetData);
    printf("--------------------------\n");
    printf("Freeing memory references.\n");
    printf("--------------------------\n");
    Juno_MemoryPutRef(&tSll2.ptStart->tMemory);
    printf("--------------------------\n");
    printf("Freeing memory\n");
    printf("--------------------------\n");
    for(size_t i = 0; i < tSll.zLen; i++)
    {
        int iRetData = 0;
        Pop(&tMemAlloc, &tSll, &iRetData);
        printf("Freeing Node: %lu\n", i);
    }
}
