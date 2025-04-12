#include "juno/memory/memory.h"
#include "juno/memory/memory_types.h"
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
JUNO_MEMORY_METADATA(ptTestMetadata, 10);

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
    JUNO_MEMORY_BLOCK_T tMemBlock = {0};
    JUNO_STATUS_T tStatus = Juno_MemoryBlkInit(
        &tMemBlock,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    JUNO_MEMORY_T tMemory = {0};
    tStatus = Juno_MemoryBlkGet(&tMemBlock, &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
    TEST_ASSERT_NOT_EQUAL(0, tMemory.pvAddr);
    tStatus = Juno_MemoryBlkPut(&tMemBlock, &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NULL(tMemory.pvAddr);
    TEST_ASSERT_EQUAL(0, tMemory.pvAddr);
}

static void test_nominal_multiple_alloc_and_free(void)
{
    JUNO_MEMORY_BLOCK_T tMemBlock = {0};
    JUNO_STATUS_T tStatus = Juno_MemoryBlkInit(
        &tMemBlock,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_BLOCK_T *ptTestPtr[10] = {0};
    JUNO_MEMORY_T ptMemory[10] = {0};
    for (int j = 0; j < 5; j++)
    {
        for (size_t i = 0; i < 10; i++)
        {
            tStatus = Juno_MemoryBlkGet(&tMemBlock, &ptMemory[i]);
            ptTestPtr[i] = ptMemory[i].pvAddr;
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
            TEST_ASSERT_NOT_NULL(ptMemory[i].pvAddr);
            TEST_ASSERT_NOT_EQUAL(0, ptMemory[i].pvAddr);
            ptTestPtr[i]->bTestFlag = true;
            ptTestPtr[i]->iTestNum = i;
        }
        for (size_t i = 0; i < 5; i++)
        {
            tStatus = Juno_MemoryBlkPut(&tMemBlock, &ptMemory[i]);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
            TEST_ASSERT_NULL(ptMemory[i].pvAddr);
            TEST_ASSERT_EQUAL(0, ptMemory[i].pvAddr);
            ptTestPtr[i] = NULL;
        }
        for (size_t i = 0; i < 5; i++)
        {
            tStatus = Juno_MemoryBlkGet(&tMemBlock, &ptMemory[i]);
            ptTestPtr[i] = ptMemory[i].pvAddr;
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
            TEST_ASSERT_NOT_NULL(ptMemory[i].pvAddr);
            TEST_ASSERT_NOT_EQUAL(0, ptMemory[i].pvAddr);
        }
        for (size_t i = 0; i < 10; i++)
        {
            tStatus = Juno_MemoryBlkPut(&tMemBlock, &ptMemory[i]);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
            TEST_ASSERT_NULL(ptMemory[i].pvAddr);
            TEST_ASSERT_EQUAL(0, ptMemory[i].pvAddr);
            ptTestPtr[i] = NULL;
        }
    }
}

static void test_negative_memory_empty(void)
{
    JUNO_MEMORY_BLOCK_T tMemBlock = {0};
    JUNO_STATUS_T tStatus = Juno_MemoryBlkInit(
        &tMemBlock,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    JUNO_MEMORY_T tFailMemory = {
        .pvAddr = ptTestBlock,
        .zSize = 128
    };
    tStatus = Juno_MemoryBlkPut(&tMemBlock, &tFailMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_MEMFREE_ERROR, tStatus);
    TEST_BLOCK_T *ptTestPtr[10] = {0};
    JUNO_MEMORY_T ptMemory[10] = {0};
    for (size_t i = 0; i < 10; i++)
    {
        tStatus = Juno_MemoryBlkGet(&tMemBlock, &ptMemory[i]);
        ptTestPtr[i] = ptMemory[i].pvAddr;
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_NOT_NULL(ptMemory[i].pvAddr);
        TEST_ASSERT_NOT_EQUAL(0, ptMemory[i].pvAddr);
        ptTestPtr[i]->bTestFlag = true;
        ptTestPtr[i]->iTestNum = i;
    }
    for (size_t i = 0; i < 10; i++)
    {
        tStatus = Juno_MemoryBlkPut(&tMemBlock, &ptMemory[i]);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_NULL(ptMemory[i].pvAddr);
        TEST_ASSERT_EQUAL(0, ptMemory[i].pvAddr);
        ptTestPtr[i] = NULL;
    }
    tStatus = Juno_MemoryBlkPut(&tMemBlock, &ptMemory[0]);
    TEST_ASSERT_EQUAL(JUNO_STATUS_MEMFREE_ERROR, tStatus);
}

static void test_negative_memory_full(void)
{
    JUNO_MEMORY_BLOCK_T tMemBlock = {0};
    JUNO_STATUS_T tStatus = Juno_MemoryBlkInit(
        &tMemBlock,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_BLOCK_T *ptTestPtr[10] = {0};
    JUNO_MEMORY_T ptMemory[10] = {0};
    for (size_t i = 0; i < 10; i++)
    {
        tStatus = Juno_MemoryBlkGet(&tMemBlock, &ptMemory[i]);
        ptTestPtr[i] = ptMemory[i].pvAddr;
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_NOT_NULL(ptMemory[i].pvAddr);
        TEST_ASSERT_NOT_EQUAL(0, ptMemory[i].pvAddr);
        ptTestPtr[i]->bTestFlag = true;
        ptTestPtr[i]->iTestNum = i;
    }
    JUNO_MEMORY_T tFailMemory = {0};
    tStatus = Juno_MemoryBlkGet(&tMemBlock, &tFailMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_MEMALLOC_ERROR, tStatus);
}

/* New tests for higher code coverage */

// Test initializing the memory block with invalid parameters.
static void test_invalid_init_parameters(void)
{
    JUNO_MEMORY_BLOCK_T tMemBlock = {0};
    // Passing NULL for memory and metadata.
    JUNO_STATUS_T tStatus = Juno_MemoryBlkInit(
        &tMemBlock,
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
    JUNO_MEMORY_BLOCK_T tMemBlock = {0};
    JUNO_STATUS_T tStatus = Juno_MemoryBlkInit(
        &tMemBlock,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    JUNO_MEMORY_T tMemory = {0};
    tStatus = Juno_MemoryBlkGet(&tMemBlock, &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
    // First free should succeed.
    tStatus = Juno_MemoryBlkPut(&tMemBlock, &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    // Second free should fail.
    tStatus = Juno_MemoryBlkPut(&tMemBlock, &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_MEMFREE_ERROR, tStatus);
}

// Test freeing an allocation that was never obtained.
static void test_free_unallocated(void)
{
    JUNO_MEMORY_BLOCK_T tMemBlock = {0};
    JUNO_STATUS_T tStatus = Juno_MemoryBlkInit(
        &tMemBlock,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    JUNO_MEMORY_T tMemory = {0};
    // tMemory never allocated via Get should be flagged as unallocated.
    tStatus = Juno_MemoryBlkPut(&tMemBlock, &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_MEMFREE_ERROR, tStatus);
}

// Test generic update function on block allocation.
// Note: Depending on your implementation, update might only succeed when the new size
// equals the block size. Here we attempt an update with the same size and a different size.
static void test_update_memory(void)
{
    JUNO_MEMORY_BLOCK_T tMemBlock = {0};
    JUNO_STATUS_T tStatus = Juno_MemoryBlkInit(
        &tMemBlock,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    JUNO_MEMORY_T tMemory = {0};
    tStatus = Juno_MemoryBlkGet(&tMemBlock, &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Attempt update with same size: should succeed.
    tStatus = Juno_MemoryBlkUpdate(&tMemBlock, &tMemory, sizeof(TEST_BLOCK_T));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    // Attempt update with a different size: expect failure.
    tStatus = Juno_MemoryBlkUpdate(&tMemBlock, &tMemory, sizeof(TEST_BLOCK_T) + 1);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Free the memory.
    tStatus = Juno_MemoryBlkPut(&tMemBlock, &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// Test generic memory allocation functions using the block allocator.
// Casting the block allocator to the generic interface.
static void test_generic_memory_get_put(void)
{
    // Use the block as a generic allocator.
    JUNO_MEMORY_BLOCK_T tMemBlock = {0};
    JUNO_STATUS_T tStatus = Juno_MemoryBlkInit(
        &tMemBlock,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);

    // Cast the address to the generic allocator type.
    JUNO_MEMORY_ALLOC_T *ptAlloc = (JUNO_MEMORY_ALLOC_T *)&tMemBlock;
    JUNO_MEMORY_T tMemory = {0};

    // Use the generic get function.
    tStatus = Juno_MemoryGet(ptAlloc, &tMemory, sizeof(TEST_BLOCK_T));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
    // Use the generic update function with a valid size.
    tStatus = Juno_MemoryUpdate(ptAlloc, &tMemory, sizeof(TEST_BLOCK_T));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    // Free using the generic free function.
    tStatus = Juno_MemoryPut(ptAlloc, &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NULL(tMemory.pvAddr);
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
    return UNITY_END();
}
