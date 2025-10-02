#include "juno/macros.h"
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
/// Copy memory from one pointer to another
static JUNO_STATUS_T Copy(JUNO_POINTER_T tDest, JUNO_POINTER_T tSrc)
{
    JUNO_STATUS_T tStatus = JUNO_CHECK_POINTER_TYPE(tDest, TEST_BLOCK_T);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JUNO_CHECK_POINTER_TYPE(tSrc, TEST_BLOCK_T);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    TEST_BLOCK_T *ptDest = (TEST_BLOCK_T *)tDest.pvAddr;
    TEST_BLOCK_T *ptSrc = (TEST_BLOCK_T *)tSrc.pvAddr;
    *ptDest = *ptSrc;
    return tStatus;
}

/// Reset the memory at the pointer. This could mean zero-initialization
static JUNO_STATUS_T Reset(JUNO_POINTER_T tPointer)
{
    JUNO_STATUS_T tStatus = JUNO_CHECK_POINTER_TYPE(tPointer, TEST_BLOCK_T);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    TEST_BLOCK_T *ptBlock = (TEST_BLOCK_T *)tPointer.pvAddr;
    *ptBlock = (TEST_BLOCK_T){0};
    return tStatus;
}

const JUNO_POINTER_API_T gtTestBlockApi = {
    Copy,
    Reset
};

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
    JUNO_MEMORY_ALLOC_BLOCK_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockInit(
        &tMem,
        &gtTestBlockApi,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    JUNO_POINTER_T tMemory = {0};
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tRoot.ptApi;
    JUNO_RESULT_POINTER_T tPointerResult = ptApi->Get(&tMem.tRoot,  sizeof(TEST_BLOCK_T));
    tStatus = tPointerResult.tStatus;
    tMemory = tPointerResult.tOk;
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
    TEST_ASSERT_NOT_EQUAL(0, tMemory.pvAddr);
    tStatus = ptApi->Put(&tMem.tRoot,  &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NULL(tMemory.pvAddr);
    TEST_ASSERT_EQUAL(0, tMemory.pvAddr);
    tPointerResult = ptApi->Get(&tMem.tRoot,  sizeof(TEST_BLOCK_T));
    tStatus = tPointerResult.tStatus;
    tMemory = tPointerResult.tOk;
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
    TEST_ASSERT_NOT_EQUAL(0, tMemory.pvAddr);
    tStatus = ptApi->Put(&tMem.tRoot,  &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NULL(tMemory.pvAddr);
    TEST_ASSERT_EQUAL(0, tMemory.pvAddr);
}

static void test_nominal_multiple_alloc_and_free(void)
{
    JUNO_MEMORY_ALLOC_BLOCK_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockInit(
        &tMem,
        &gtTestBlockApi,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tRoot.ptApi;
    TEST_BLOCK_T *ptTestPtr[10] = {0};
    JUNO_POINTER_T ptMemory[10] = {0};
    for (int j = 0; j < 5; j++)
    {
        for (size_t i = 0; i < 10; i++)
        {
            JUNO_RESULT_POINTER_T tPointerResult = ptApi->Get(&tMem.tRoot, sizeof(TEST_BLOCK_T));
            tStatus = tPointerResult.tStatus;
            ptMemory[i] = tPointerResult.tOk;
            ptTestPtr[i] = ptMemory[i].pvAddr;
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
            TEST_ASSERT_NOT_NULL(ptMemory[i].pvAddr);
            TEST_ASSERT_NOT_EQUAL(0, ptMemory[i].pvAddr);
            ptTestPtr[i]->bTestFlag = true;
            ptTestPtr[i]->iTestNum = i;
        }
        for (size_t i = 0; i < 5; i++)
        {
            tStatus = ptApi->Put(&tMem.tRoot,  &ptMemory[i]);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
            TEST_ASSERT_NULL(ptMemory[i].pvAddr);
            TEST_ASSERT_EQUAL(0, ptMemory[i].pvAddr);
            ptTestPtr[i] = NULL;
        }
        for (size_t i = 0; i < 5; i++)
        {
            JUNO_RESULT_POINTER_T tPointerResult = ptApi->Get(&tMem.tRoot, sizeof(TEST_BLOCK_T));
            tStatus = tPointerResult.tStatus;
            ptMemory[i] = tPointerResult.tOk;
            ptTestPtr[i] = ptMemory[i].pvAddr;
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
            TEST_ASSERT_NOT_NULL(ptMemory[i].pvAddr);
            TEST_ASSERT_NOT_EQUAL(0, ptMemory[i].pvAddr);
        }
        for (size_t i = 0; i < 10; i++)
        {
            tStatus = ptApi->Put(&tMem.tRoot,  &ptMemory[i]);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
            TEST_ASSERT_NULL(ptMemory[i].pvAddr);
            TEST_ASSERT_EQUAL(0, ptMemory[i].pvAddr);
            ptTestPtr[i] = NULL;
        }
    }
}

static void test_negative_memory_empty(void)
{
    JUNO_MEMORY_ALLOC_BLOCK_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockInit(
        &tMem,
        &gtTestBlockApi,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tRoot.ptApi;
    JUNO_POINTER_T tFailMemory = {
        .pvAddr = ptTestBlock,
        .zSize = 128
    };
    tStatus = ptApi->Put(&tMem.tRoot,  &tFailMemory);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_BLOCK_T *ptTestPtr[10] = {0};
    JUNO_POINTER_T ptMemory[10] = {0};
    for (size_t i = 0; i < 10; i++)
    {
        JUNO_RESULT_POINTER_T tPointerResult = ptApi->Get(&tMem.tRoot, sizeof(TEST_BLOCK_T));
        tStatus = tPointerResult.tStatus;
        ptMemory[i] = tPointerResult.tOk;
        ptTestPtr[i] = ptMemory[i].pvAddr;
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_NOT_NULL(ptMemory[i].pvAddr);
        TEST_ASSERT_NOT_EQUAL(0, ptMemory[i].pvAddr);
        ptTestPtr[i]->bTestFlag = true;
        ptTestPtr[i]->iTestNum = i;
    }
    for (size_t i = 0; i < 10; i++)
    {
        tStatus = ptApi->Put(&tMem.tRoot,  &ptMemory[i]);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_NULL(ptMemory[i].pvAddr);
        TEST_ASSERT_EQUAL(0, ptMemory[i].pvAddr);
        ptTestPtr[i] = NULL;
    }
    tStatus = ptApi->Put(&tMem.tRoot, &ptMemory[0]);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_negative_memory_full(void)
{
    JUNO_MEMORY_ALLOC_BLOCK_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockInit(
        &tMem,
        &gtTestBlockApi,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tRoot.ptApi;
    TEST_BLOCK_T *ptTestPtr[10] = {0};
    JUNO_POINTER_T ptMemory[10] = {0};
    for (size_t i = 0; i < 10; i++)
    {
        JUNO_RESULT_POINTER_T tPointerResult = ptApi->Get(&tMem.tRoot, sizeof(TEST_BLOCK_T));
        tStatus = tPointerResult.tStatus;
        ptMemory[i] = tPointerResult.tOk;
        ptTestPtr[i] = ptMemory[i].pvAddr;
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_NOT_NULL(ptMemory[i].pvAddr);
        TEST_ASSERT_NOT_EQUAL(0, ptMemory[i].pvAddr);
        ptTestPtr[i]->bTestFlag = true;
        ptTestPtr[i]->iTestNum = i;
    }
    JUNO_RESULT_POINTER_T tPointerResult = ptApi->Get(&tMem.tRoot, sizeof(TEST_BLOCK_T));
    tStatus = tPointerResult.tStatus;
    TEST_ASSERT_EQUAL(JUNO_STATUS_MEMALLOC_ERROR, tStatus);
}

/* New tests for higher code coverage */

static void test_invalid_init_parameters(void)
{
    JUNO_MEMORY_ALLOC_BLOCK_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockInit(
        &tMem,
        NULL,
        NULL,
        NULL,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_double_free(void)
{
    JUNO_MEMORY_ALLOC_BLOCK_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockInit(
        &tMem,
        &gtTestBlockApi,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tRoot.ptApi;
    JUNO_POINTER_T tMemory = {0};
    JUNO_RESULT_POINTER_T tPointerResult = ptApi->Get(&tMem.tRoot, sizeof(TEST_BLOCK_T));
    tStatus = tPointerResult.tStatus;
    tMemory = tPointerResult.tOk;
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
    tStatus = ptApi->Put(&tMem.tRoot,  &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    tStatus = ptApi->Put(&tMem.tRoot,  &tMemory);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_free_unallocated(void)
{
    JUNO_MEMORY_ALLOC_BLOCK_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockInit(
        &tMem,
        &gtTestBlockApi,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tRoot.ptApi;
    JUNO_POINTER_T tMemory = {0};
    tStatus = ptApi->Put(&tMem.tRoot,  &tMemory);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_update_memory(void)
{
    JUNO_MEMORY_ALLOC_BLOCK_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockInit(
        &tMem,
        &gtTestBlockApi,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tRoot.ptApi;
    
    JUNO_POINTER_T tMemory = {0};
    JUNO_RESULT_POINTER_T tPointerResult = ptApi->Get(&tMem.tRoot, sizeof(TEST_BLOCK_T));
    tStatus = tPointerResult.tStatus;
    tMemory = tPointerResult.tOk;
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    tStatus = ptApi->Update(&tMem.tRoot, &tMemory, sizeof(TEST_BLOCK_T));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    tStatus = ptApi->Update(&tMem.tRoot, &tMemory, sizeof(TEST_BLOCK_T) + 1);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    tStatus = ptApi->Put(&tMem.tRoot,  &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_generic_memory_get_put(void)
{
    JUNO_MEMORY_ALLOC_BLOCK_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockInit(
        &tMem,
        &gtTestBlockApi,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tRoot.ptApi;

    JUNO_MEMORY_ALLOC_ROOT_T *ptAlloc = &tMem.tRoot;
    JUNO_POINTER_T tMemory = {0};

    JUNO_RESULT_POINTER_T tPointerResult = ptApi->Get(ptAlloc, sizeof(TEST_BLOCK_T));
    tStatus = tPointerResult.tStatus;
    tMemory = tPointerResult.tOk;
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
    tStatus = ptApi->Update(ptAlloc, &tMemory, sizeof(TEST_BLOCK_T));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    tStatus = ptApi->Put(ptAlloc,  &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NULL(tMemory.pvAddr);
}

static void test_zero_size_allocation(void)
{
    JUNO_MEMORY_ALLOC_BLOCK_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockInit(
        &tMem,
        &gtTestBlockApi,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tRoot.ptApi;
    
    JUNO_RESULT_POINTER_T tPointerResult = ptApi->Get(&tMem.tRoot, 0);
    tStatus = tPointerResult.tStatus;
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}
static void test_bad_api(void)
{
    JUNO_MEMORY_ALLOC_BLOCK_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockInit(
        &tMem,
        &gtTestBlockApi,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tRoot.ptApi;

    tMem.tRoot.ptApi = NULL;
    JUNO_POINTER_T tMemory = {0};
    JUNO_RESULT_POINTER_T tPointerResult = ptApi->Get(&tMem.tRoot, 0);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tPointerResult.tStatus);
    tStatus = ptApi->Put(&tMem.tRoot, &tMemory);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    tStatus = ptApi->Update(&tMem.tRoot, &tMemory, 0);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);

    tMem.tRoot.ptApi = &ptApi[1];
    tPointerResult = ptApi->Get(&tMem.tRoot, 0);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tPointerResult.tStatus);
    tStatus = ptApi->Put(&tMem.tRoot, &tMemory);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    tStatus = ptApi->Update(&tMem.tRoot, &tMemory, 0);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_invalid_size_and_addr(void)
{
    JUNO_MEMORY_ALLOC_BLOCK_T tMem = {0};
    JUNO_STATUS_T tStatus = JunoMemory_BlockInit(
        &tMem,
        &gtTestBlockApi,
        ptTestBlock,
        ptTestMetadata,
        sizeof(TEST_BLOCK_T),
        10,
        NULL,
        NULL
    );
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    JUNO_POINTER_T tMemory = {0};
    const JUNO_MEMORY_ALLOC_API_T *ptApi = tMem.tRoot.ptApi;
    JUNO_RESULT_POINTER_T tPointerResult = ptApi->Get(&tMem.tRoot,  sizeof(TEST_BLOCK_T)+1);
    tStatus = tPointerResult.tStatus;
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    tPointerResult = ptApi->Get(&tMem.tRoot,  sizeof(TEST_BLOCK_T));
    tStatus = tPointerResult.tStatus;
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    tMemory = tPointerResult.tOk;
    JUNO_POINTER_T tMemory2 = {0};
    tPointerResult = ptApi->Get(&tMem.tRoot,  sizeof(TEST_BLOCK_T));
    tStatus = tPointerResult.tStatus;
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    tMemory = tPointerResult.tOk;
    TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
    TEST_ASSERT_NOT_EQUAL(0, tMemory.pvAddr);
    tMemory2 = tMemory;
    tStatus = ptApi->Put(&tMem.tRoot,  &tMemory);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    tMem.zFreed = 1;
    tMem.ptMetadata[0].ptFreeMem = (uint8_t *)(tMemory2.pvAddr);
    tStatus = ptApi->Put(&tMem.tRoot,  &tMemory2);
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

