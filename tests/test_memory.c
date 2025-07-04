#include "juno/memory/memory_api.h"
#define JUNO_MEMORY_DEFAULT
#include "juno/memory/memory_block.h"
#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

typedef struct TEST_BLOCK_TAG
{
    uint32_t iTestNum;
    bool bTestFlag;
} TEST_BLOCK_T;

JUNO_MEMORY_BLOCK(ptTestBlock, TEST_BLOCK_T, 10);
JUNO_MEMORY_BLOCK_METADATA(ptTestMetadata, 10);

void setUp(void)
{
    memset(ptTestBlock, 0, sizeof(ptTestBlock));
    memset(ptTestMetadata, 0, sizeof(ptTestMetadata));
}

void tearDown(void)
{
    // No teardown required.
}

static void test_nominal_single_alloc_and_free(void)
{
    JUNO_MEMORY_ALLOC_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockApi(
        &tMem,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    JUNO_MEMORY_T tMemory = {0};
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tJunoMemoryBlock.tRoot.ptApi;
    tStatus = ptApi->Get(&tMem,  &tMemory,  sizeof(TEST_BLOCK_T));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
    TEST_ASSERT_NOT_EQUAL(0, tMemory.pvAddr);
    TEST_ASSERT_EQUAL(1, tMemory.iRefCount);
    tStatus = ptApi->Put(&tMem,  &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NULL(tMemory.pvAddr);
    TEST_ASSERT_EQUAL(0, tMemory.pvAddr);
    TEST_ASSERT_EQUAL(0, tMemory.iRefCount);
    tStatus = ptApi->Get(&tMem,  &tMemory,  sizeof(TEST_BLOCK_T));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
    TEST_ASSERT_NOT_EQUAL(0, tMemory.pvAddr);
    TEST_ASSERT_EQUAL(1, tMemory.iRefCount);
    JUNO_NEW_REF(tMemory) = Juno_MemoryGetRef(&tMemory);
    TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
    TEST_ASSERT_NOT_EQUAL(0, tMemory.pvAddr);
    TEST_ASSERT_EQUAL(2, tMemory.iRefCount);
    TEST_ASSERT_NOT_NULL(JUNO_REF(tMemory)->pvAddr);
    TEST_ASSERT_NOT_EQUAL(0, JUNO_REF(tMemory)->pvAddr);
    TEST_ASSERT_EQUAL(2, JUNO_REF(tMemory)->iRefCount);
    tStatus = ptApi->Put(&tMem,  JUNO_REF(tMemory));
    TEST_ASSERT_EQUAL(JUNO_STATUS_REF_IN_USE_ERROR, tStatus);
    TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
    TEST_ASSERT_NOT_EQUAL(0, tMemory.pvAddr);
    TEST_ASSERT_EQUAL(2, tMemory.iRefCount);
    JUNO_REF(tMemory) = Juno_MemoryGetRef(&tMemory);
    TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
    TEST_ASSERT_NOT_EQUAL(0, tMemory.pvAddr);
    TEST_ASSERT_EQUAL(3, tMemory.iRefCount);
    TEST_ASSERT_NOT_NULL(JUNO_REF(tMemory)->pvAddr);
    TEST_ASSERT_NOT_EQUAL(0, JUNO_REF(tMemory)->pvAddr);
    TEST_ASSERT_EQUAL(3, JUNO_REF(tMemory)->iRefCount);
    Juno_MemoryPutRef(JUNO_REF(tMemory));
    TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
    TEST_ASSERT_NOT_EQUAL(0, tMemory.pvAddr);
    TEST_ASSERT_EQUAL(2, tMemory.iRefCount);
    Juno_MemoryPutRef(JUNO_REF(tMemory));
    TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
    TEST_ASSERT_NOT_EQUAL(0, tMemory.pvAddr);
    TEST_ASSERT_EQUAL(1, tMemory.iRefCount);
    tStatus = ptApi->Put(&tMem,  &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NULL(tMemory.pvAddr);
    TEST_ASSERT_EQUAL(0, tMemory.pvAddr);
    TEST_ASSERT_EQUAL(0, tMemory.iRefCount);

}

static void test_nominal_multiple_alloc_and_free(void)
{
    JUNO_MEMORY_ALLOC_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockApi(
        &tMem,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tJunoMemoryBlock.tRoot.ptApi;
    TEST_BLOCK_T *ptTestPtr[10] = {0};
    JUNO_MEMORY_T ptMemory[10] = {0};
    for (int j = 0; j < 5; j++)
    {
        for (size_t i = 0; i < 10; i++)
        {
            tStatus = ptApi->Get(&tMem,  &ptMemory[i],  sizeof(TEST_BLOCK_T));
            ptTestPtr[i] = ptMemory[i].pvAddr;
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
            TEST_ASSERT_NOT_NULL(ptMemory[i].pvAddr);
            TEST_ASSERT_NOT_EQUAL(0, ptMemory[i].pvAddr);
            ptTestPtr[i]->bTestFlag = true;
            ptTestPtr[i]->iTestNum = i;
        }
        for (size_t i = 0; i < 5; i++)
        {
            tStatus = ptApi->Put(&tMem,  &ptMemory[i]);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
            TEST_ASSERT_NULL(ptMemory[i].pvAddr);
            TEST_ASSERT_EQUAL(0, ptMemory[i].pvAddr);
            ptTestPtr[i] = NULL;
        }
        for (size_t i = 0; i < 5; i++)
        {
            tStatus = ptApi->Get(&tMem,  &ptMemory[i],  sizeof(TEST_BLOCK_T));
            ptTestPtr[i] = ptMemory[i].pvAddr;
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
            TEST_ASSERT_NOT_NULL(ptMemory[i].pvAddr);
            TEST_ASSERT_NOT_EQUAL(0, ptMemory[i].pvAddr);
        }
        for (size_t i = 0; i < 10; i++)
        {
            tStatus = ptApi->Put(&tMem,  &ptMemory[i]);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
            TEST_ASSERT_NULL(ptMemory[i].pvAddr);
            TEST_ASSERT_EQUAL(0, ptMemory[i].pvAddr);
            ptTestPtr[i] = NULL;
        }
    }
}

static void test_negative_memory_empty(void)
{
    JUNO_MEMORY_ALLOC_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockApi(
        &tMem,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tJunoMemoryBlock.tRoot.ptApi;
    JUNO_MEMORY_T tFailMemory = {
        .pvAddr = ptTestBlock,
        .zSize = 128
    };
    tStatus = ptApi->Put(&tMem,  &tFailMemory);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_BLOCK_T *ptTestPtr[10] = {0};
    JUNO_MEMORY_T ptMemory[10] = {0};
    for (size_t i = 0; i < 10; i++)
    {
        tStatus = ptApi->Get(&tMem,  &ptMemory[i],  sizeof(TEST_BLOCK_T));
        ptTestPtr[i] = ptMemory[i].pvAddr;
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_NOT_NULL(ptMemory[i].pvAddr);
        TEST_ASSERT_NOT_EQUAL(0, ptMemory[i].pvAddr);
        ptTestPtr[i]->bTestFlag = true;
        ptTestPtr[i]->iTestNum = i;
    }
    for (size_t i = 0; i < 10; i++)
    {
        tStatus = ptApi->Put(&tMem,  &ptMemory[i]);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_NULL(ptMemory[i].pvAddr);
        TEST_ASSERT_EQUAL(0, ptMemory[i].pvAddr);
        ptTestPtr[i] = NULL;
    }
    tStatus = ptApi->Put(&tMem, &ptMemory[0]);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_negative_memory_full(void)
{
    JUNO_MEMORY_ALLOC_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockApi(
        &tMem,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tJunoMemoryBlock.tRoot.ptApi;
    TEST_BLOCK_T *ptTestPtr[10] = {0};
    JUNO_MEMORY_T ptMemory[10] = {0};
    for (size_t i = 0; i < 10; i++)
    {
        tStatus = ptApi->Get(&tMem,  &ptMemory[i],  sizeof(TEST_BLOCK_T));
        ptTestPtr[i] = ptMemory[i].pvAddr;
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_NOT_NULL(ptMemory[i].pvAddr);
        TEST_ASSERT_NOT_EQUAL(0, ptMemory[i].pvAddr);
        ptTestPtr[i]->bTestFlag = true;
        ptTestPtr[i]->iTestNum = i;
    }
    JUNO_MEMORY_T tFailMemory = {0};
    tStatus = ptApi->Get(&tMem,  &tFailMemory,  sizeof(TEST_BLOCK_T));
    TEST_ASSERT_EQUAL(JUNO_STATUS_MEMALLOC_ERROR, tStatus);
}

/* New tests for higher code coverage */

// Test initializing the memory block with invalid parameters.
static void test_invalid_init_parameters(void)
{
    JUNO_MEMORY_ALLOC_T tMem = {0};
    // Passing NULL for memory and metadata.
    JUNO_STATUS_T tStatus = JunoMemory_BlockApi(
        &tMem,
        NULL,
        NULL,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    // Expect failure (error status) when invalid pointers are used.
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// Test double-free of the same allocation.
static void test_double_free(void)
{
    JUNO_MEMORY_ALLOC_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockApi(
        &tMem,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tJunoMemoryBlock.tRoot.ptApi;
    JUNO_MEMORY_T tMemory = {0};
    tStatus = ptApi->Get(&tMem,  &tMemory,  sizeof(TEST_BLOCK_T));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
    // First free should succeed.
    tStatus = ptApi->Put(&tMem,  &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    // Second free should fail.
    tStatus = ptApi->Put(&tMem,  &tMemory);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// Test freeing an allocation that was never obtained.
static void test_free_unallocated(void)
{
    JUNO_MEMORY_ALLOC_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockApi(
        &tMem,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tJunoMemoryBlock.tRoot.ptApi;
    JUNO_MEMORY_T tMemory = {0};
    // tMemory never allocated via Get should be flagged as unallocated.
    tStatus = ptApi->Put(&tMem,  &tMemory);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// Test generic update function on block allocation.
// Note: Depending on your implementation, update might only succeed when the new size
// equals the block size. Here we attempt an update with the same size and a different size.
static void test_update_memory(void)
{
    JUNO_MEMORY_ALLOC_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockApi(
        &tMem,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tJunoMemoryBlock.tRoot.ptApi;
    
    JUNO_MEMORY_T tMemory = {0};
    tStatus = ptApi->Get(&tMem,  &tMemory,  sizeof(TEST_BLOCK_T));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Attempt update with same size: should succeed.
    tStatus = ptApi->Update(&tMem, &tMemory, sizeof(TEST_BLOCK_T));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    // Attempt update with a different size: expect failure.
    tStatus = ptApi->Update(&tMem, &tMemory, sizeof(TEST_BLOCK_T) + 1);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Free the memory.
    tStatus = ptApi->Put(&tMem,  &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// Test generic memory allocation functions using the block allocator.
// Casting the block allocator to the generic interface.
static void test_generic_memory_get_put(void)
{
    // Use the block as a generic allocator.
    JUNO_MEMORY_ALLOC_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockApi(
        &tMem,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tJunoMemoryBlock.tRoot.ptApi;

    // Cast the address to the generic allocator type.
    JUNO_MEMORY_ALLOC_T *ptAlloc = (JUNO_MEMORY_ALLOC_T *)&tMem;
    JUNO_MEMORY_T tMemory = {0};

    // Use the generic get function.
    tStatus = ptApi->Get(ptAlloc,  &tMemory,  sizeof(TEST_BLOCK_T));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
    // Use the generic update function with a valid size.
    tStatus = ptApi->Update(ptAlloc, &tMemory, sizeof(TEST_BLOCK_T));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    // Free using the generic free function.
    tStatus = ptApi->Put(ptAlloc,  &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NULL(tMemory.pvAddr);
}

// Test allocation with zero size
static void test_zero_size_allocation(void)
{
    JUNO_MEMORY_ALLOC_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockApi(
        &tMem,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tJunoMemoryBlock.tRoot.ptApi;
    
    JUNO_MEMORY_T tMemory = {0};
    // Attempt zero-size allocation
    tStatus = ptApi->Get(&tMem, &tMemory, 0);
    // The implementation should handle this gracefully
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}
// Test allocation with zero size
static void test_bad_api(void)
{
    JUNO_MEMORY_ALLOC_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockApi(
        &tMem,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tJunoMemoryBlock.tRoot.ptApi;
    tMem.tRoot.ptApi = NULL;
    JUNO_MEMORY_T tMemory = {0};
    tStatus = ptApi->Get(&tMem, &tMemory, 0);
    tStatus = ptApi->Put(&tMem, &tMemory);
    tStatus = ptApi->Update(&tMem, &tMemory, 0);
    // The implementation should handle this gracefully
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    tMem.tRoot.ptApi = &ptApi[1];
    tStatus = ptApi->Get(&tMem, &tMemory, 0);
    tStatus = ptApi->Put(&tMem, &tMemory);
    tStatus = ptApi->Update(&tMem, &tMemory, 0);
    // The implementation should handle this gracefully
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}



static void test_invalid_size_and_addr(void)
{
    JUNO_MEMORY_ALLOC_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockApi(
        &tMem,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    JUNO_MEMORY_T tMemory = {0};
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tJunoMemoryBlock.tRoot.ptApi;
    tStatus = ptApi->Get(&tMem,  &tMemory,  sizeof(TEST_BLOCK_T)+1);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    tStatus = ptApi->Get(&tMem,  &tMemory,  sizeof(TEST_BLOCK_T));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    JUNO_MEMORY_T tMemory2 = {0};
    tStatus = ptApi->Get(&tMem,  &tMemory,  sizeof(TEST_BLOCK_T));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
    TEST_ASSERT_NOT_EQUAL(0, tMemory.pvAddr);
    TEST_ASSERT_EQUAL(1, tMemory.iRefCount);
    tMemory2 = tMemory;
    tStatus = ptApi->Put(&tMem,  &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    tMem.tJunoMemoryBlock.zFreed = 1;
    tMem.tJunoMemoryBlock.ptMetadata[0].ptFreeMem = (uint8_t *)(tMemory2.pvAddr);
    tStatus = ptApi->Put(&tMem,  &tMemory2);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_nominal_single_alloc_and_free);
    RUN_TEST(test_nominal_multiple_alloc_and_free);
    RUN_TEST(test_negative_memory_full);
    RUN_TEST(test_negative_memory_empty);
    RUN_TEST(test_invalid_init_parameters);
    RUN_TEST(test_double_free);
    RUN_TEST(test_free_unallocated);
    RUN_TEST(test_update_memory);
    RUN_TEST(test_generic_memory_get_put);
    RUN_TEST(test_zero_size_allocation);
    RUN_TEST(test_bad_api);
    RUN_TEST(test_invalid_size_and_addr);
    return UNITY_END();
}
