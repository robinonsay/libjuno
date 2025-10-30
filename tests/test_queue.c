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

/**
 * @file test_queue.c
 * @brief Comprehensive unit tests for the LibJuno Queue API
 * @author Robin Onsay
 * 
 * This test suite aims for 100% code and branch coverage of the queue
 * implementation, testing all edge cases and error paths.
 */

#include "juno/ds/queue_api.h"
#include "juno/ds/array_api.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

/* ============================================================================
 * Test Data Type Definition
 * ============================================================================ */

typedef struct TEST_QUEUE_DATA_TAG
{
    uint32_t iValue;
    bool bFlag;
    uint8_t iCounter;
} TEST_QUEUE_DATA_T;

/* ============================================================================
 * Test Queue Implementation (Custom Array-backed Queue)
 * ============================================================================ */

typedef struct TEST_QUEUE_ARRAY_TAG
{
    JUNO_DS_ARRAY_ROOT_T tRoot;
    TEST_QUEUE_DATA_T *ptBuffer;
} TEST_QUEUE_ARRAY_T;

/* Forward declarations for array API functions */
static JUNO_STATUS_T TestQueue_SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
static JUNO_RESULT_POINTER_T TestQueue_GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
static JUNO_STATUS_T TestQueue_RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);

/* Forward declarations for pointer API functions */
static JUNO_STATUS_T TestQueueData_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
static JUNO_STATUS_T TestQueueData_Reset(JUNO_POINTER_T tPointer);

/* Pointer API for TEST_QUEUE_DATA_T */
const JUNO_POINTER_API_T gtTestQueueDataPointerApi = {
    TestQueueData_Copy,
    TestQueueData_Reset
};

/* Array API for TEST_QUEUE_ARRAY_T */
static const JUNO_DS_ARRAY_API_T gtTestQueueArrayApi = {
    TestQueue_SetAt,
    TestQueue_GetAt,
    TestQueue_RemoveAt
};

/* Macro to verify pointer type */
#define TestQueueData_PointerInit(addr) JunoMemory_PointerInit(&gtTestQueueDataPointerApi, TEST_QUEUE_DATA_T, addr)
#define TestQueueData_PointerVerify(tPointer) JunoMemory_PointerVerifyType(tPointer, TEST_QUEUE_DATA_T, gtTestQueueDataPointerApi)
#define TEST_QUEUE_ASSERT_API(ptArray, ...)  if(ptArray->ptApi != &gtTestQueueArrayApi) { __VA_ARGS__; }

/* ============================================================================
 * Pointer API Implementation
 * ============================================================================ */

static JUNO_STATUS_T TestQueueData_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc)
{
    JUNO_STATUS_T tStatus = TestQueueData_PointerVerify(tDest);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = TestQueueData_PointerVerify(tSrc);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    *(TEST_QUEUE_DATA_T *)tDest.pvAddr = *(TEST_QUEUE_DATA_T *)tSrc.pvAddr;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestQueueData_Reset(JUNO_POINTER_T tPointer)
{
    JUNO_STATUS_T tStatus = TestQueueData_PointerVerify(tPointer);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    *(TEST_QUEUE_DATA_T *)tPointer.pvAddr = (TEST_QUEUE_DATA_T){0};
    return JUNO_STATUS_SUCCESS;
}

/* ============================================================================
 * Array API Implementation
 * ============================================================================ */

static JUNO_STATUS_T TestQueue_SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    TEST_QUEUE_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    tStatus = TestQueueData_PointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    
    TEST_QUEUE_ARRAY_T *ptQueueArray = (TEST_QUEUE_ARRAY_T *)ptArray;
    JUNO_POINTER_T tIndexPointer = TestQueueData_PointerInit(&ptQueueArray->ptBuffer[iIndex]);
    tStatus = tIndexPointer.ptApi->Copy(tIndexPointer, tItem);
    return tStatus;
}

static JUNO_RESULT_POINTER_T TestQueue_GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_RESULT_POINTER_T tResult = {0};
    tResult.tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    TEST_QUEUE_ASSERT_API(ptArray,
        tResult.tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        return tResult;
    );
    tResult.tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    
    TEST_QUEUE_ARRAY_T *ptQueueArray = (TEST_QUEUE_ARRAY_T *)ptArray;
    JUNO_POINTER_T tIndexPointer = TestQueueData_PointerInit(&ptQueueArray->ptBuffer[iIndex]);
    tResult.tOk = tIndexPointer;
    return tResult;
}

static JUNO_STATUS_T TestQueue_RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    TEST_QUEUE_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    
    TEST_QUEUE_ARRAY_T *ptQueueArray = (TEST_QUEUE_ARRAY_T *)ptArray;
    JUNO_POINTER_T tIndexPointer = TestQueueData_PointerInit(&ptQueueArray->ptBuffer[iIndex]);
    tStatus = tIndexPointer.ptApi->Reset(tIndexPointer);
    return tStatus;
}

/* ============================================================================
 * Test Fixture Setup/Teardown
 * ============================================================================ */

#define TEST_QUEUE_CAPACITY 10
static JUNO_DS_QUEUE_ROOT_T gtTestQueue;
static TEST_QUEUE_ARRAY_T gtTestQueueArray;
static TEST_QUEUE_DATA_T gtTestQueueBuffer[TEST_QUEUE_CAPACITY];

void setUp(void)
{
    memset(&gtTestQueue, 0, sizeof(gtTestQueue));
    memset(&gtTestQueueArray, 0, sizeof(gtTestQueueArray));
    memset(gtTestQueueBuffer, 0, sizeof(gtTestQueueBuffer));
}

void tearDown(void)
{
    /* No dynamic allocations to clean up */
}

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static JUNO_STATUS_T InitTestQueue(JUNO_DS_QUEUE_ROOT_T *ptQueue, size_t zCapacity)
{
    gtTestQueueArray.ptBuffer = gtTestQueueBuffer;
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(&gtTestQueueArray.tRoot, &gtTestQueueArrayApi, zCapacity, NULL, NULL);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return JunoDs_QueueInit(ptQueue, &gtTestQueueArray.tRoot, NULL, NULL);
}

static TEST_QUEUE_DATA_T CreateTestData(uint32_t iValue, bool bFlag, uint8_t iCounter)
{
    TEST_QUEUE_DATA_T tData = {
        .iValue = iValue,
        .bFlag = bFlag,
        .iCounter = iCounter
    };
    return tData;
}

/* ============================================================================
 * Test Cases: Initialization
 * ============================================================================ */

static void test_queue_init_nominal(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(0, gtTestQueue.iStartIndex);
    TEST_ASSERT_EQUAL(0, gtTestQueue.zLength);
    TEST_ASSERT_EQUAL(TEST_QUEUE_CAPACITY, gtTestQueue.ptQueueArray->zCapacity);
    TEST_ASSERT_NOT_NULL(gtTestQueue.ptApi);
}

static void test_queue_init_null_queue(void)
{
    gtTestQueueArray.ptBuffer = gtTestQueueBuffer;
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(&gtTestQueueArray.tRoot, &gtTestQueueArrayApi, TEST_QUEUE_CAPACITY, NULL, NULL);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    tStatus = JunoDs_QueueInit(NULL, &gtTestQueueArray.tRoot, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_queue_init_null_array(void)
{
    JUNO_STATUS_T tStatus = JunoDs_QueueInit(&gtTestQueue, NULL, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_queue_init_zero_capacity(void)
{
    gtTestQueueArray.ptBuffer = gtTestQueueBuffer;
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(&gtTestQueueArray.tRoot, &gtTestQueueArrayApi, 0, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Enqueue (Push) Operations
 * ============================================================================ */

static void test_queue_push_single_item(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    TEST_QUEUE_DATA_T tData = CreateTestData(42, true, 1);
    JUNO_POINTER_T tPointer = TestQueueData_PointerInit(&tData);
    
    tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(1, gtTestQueue.zLength);
    TEST_ASSERT_EQUAL(42, gtTestQueueBuffer[0].iValue);
    TEST_ASSERT_EQUAL(true, gtTestQueueBuffer[0].bFlag);
    TEST_ASSERT_EQUAL(1, gtTestQueueBuffer[0].iCounter);
}

static void test_queue_push_multiple_items(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    for (uint32_t i = 0; i < 5; i++)
    {
        TEST_QUEUE_DATA_T tData = CreateTestData(i * 10, (i % 2 == 0), (uint8_t)i);
        JUNO_POINTER_T tPointer = TestQueueData_PointerInit(&tData);
        tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_EQUAL(i + 1, gtTestQueue.zLength);
    }
    
    /* Verify all items were stored correctly */
    for (uint32_t i = 0; i < 5; i++)
    {
        TEST_ASSERT_EQUAL(i * 10, gtTestQueueBuffer[i].iValue);
        TEST_ASSERT_EQUAL((i % 2 == 0), gtTestQueueBuffer[i].bFlag);
        TEST_ASSERT_EQUAL(i, gtTestQueueBuffer[i].iCounter);
    }
}

static void test_queue_push_to_full_capacity(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Fill queue to capacity */
    for (uint32_t i = 0; i < TEST_QUEUE_CAPACITY; i++)
    {
        TEST_QUEUE_DATA_T tData = CreateTestData(i, true, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestQueueData_PointerInit(&tData);
        tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    TEST_ASSERT_EQUAL(TEST_QUEUE_CAPACITY, gtTestQueue.zLength);
}

static void test_queue_push_overflow(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Fill queue to capacity */
    for (uint32_t i = 0; i < TEST_QUEUE_CAPACITY; i++)
    {
        TEST_QUEUE_DATA_T tData = CreateTestData(i, true, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestQueueData_PointerInit(&tData);
        tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    /* Attempt to push one more item (should fail) */
    TEST_QUEUE_DATA_T tData = CreateTestData(999, false, 99);
    JUNO_POINTER_T tPointer = TestQueueData_PointerInit(&tData);
    tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_INVALID_SIZE_ERROR, tStatus);
    TEST_ASSERT_EQUAL(TEST_QUEUE_CAPACITY, gtTestQueue.zLength);
}

static void test_queue_push_null_queue(void)
{
    TEST_QUEUE_DATA_T tData = CreateTestData(42, true, 1);
    JUNO_POINTER_T tPointer = TestQueueData_PointerInit(&tData);
    
    JUNO_STATUS_T tStatus = JunoDs_QueuePush(NULL, tPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_queue_push_invalid_pointer(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    JUNO_POINTER_T tInvalidPointer = {0};
    tStatus = JunoDs_QueuePush(&gtTestQueue, tInvalidPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Dequeue (Pop) Operations
 * ============================================================================ */

static void test_queue_pop_single_item(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push an item */
    TEST_QUEUE_DATA_T tPushData = CreateTestData(123, true, 5);
    JUNO_POINTER_T tPushPointer = TestQueueData_PointerInit(&tPushData);
    tStatus = JunoDs_QueuePush(&gtTestQueue, tPushPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Pop the item */
    TEST_QUEUE_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestQueueData_PointerInit(&tPopData);
    tStatus = JunoDs_QueuePop(&gtTestQueue, tPopPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(0, gtTestQueue.zLength);
    TEST_ASSERT_EQUAL(123, tPopData.iValue);
    TEST_ASSERT_EQUAL(true, tPopData.bFlag);
    TEST_ASSERT_EQUAL(5, tPopData.iCounter);
}

static void test_queue_pop_multiple_items_fifo_order(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push multiple items */
    for (uint32_t i = 0; i < 5; i++)
    {
        TEST_QUEUE_DATA_T tData = CreateTestData(i * 100, (i % 2 == 0), (uint8_t)i);
        JUNO_POINTER_T tPointer = TestQueueData_PointerInit(&tData);
        tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    /* Pop items and verify FIFO order */
    for (uint32_t i = 0; i < 5; i++)
    {
        TEST_QUEUE_DATA_T tPopData = {0};
        JUNO_POINTER_T tPopPointer = TestQueueData_PointerInit(&tPopData);
        tStatus = JunoDs_QueuePop(&gtTestQueue, tPopPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_EQUAL(i * 100, tPopData.iValue);
        TEST_ASSERT_EQUAL((i % 2 == 0), tPopData.bFlag);
        TEST_ASSERT_EQUAL(i, tPopData.iCounter);
    }
    
    TEST_ASSERT_EQUAL(0, gtTestQueue.zLength);
}

static void test_queue_pop_empty_queue(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    TEST_QUEUE_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestQueueData_PointerInit(&tPopData);
    tStatus = JunoDs_QueuePop(&gtTestQueue, tPopPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, tStatus);
}

static void test_queue_pop_null_queue(void)
{
    TEST_QUEUE_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestQueueData_PointerInit(&tPopData);
    JUNO_STATUS_T tStatus = JunoDs_QueuePop(NULL, tPopPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_queue_pop_invalid_pointer(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push an item */
    TEST_QUEUE_DATA_T tPushData = CreateTestData(42, true, 1);
    JUNO_POINTER_T tPushPointer = TestQueueData_PointerInit(&tPushData);
    tStatus = JunoDs_QueuePush(&gtTestQueue, tPushPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Try to pop with invalid pointer */
    JUNO_POINTER_T tInvalidPointer = {0};
    tStatus = JunoDs_QueuePop(&gtTestQueue, tInvalidPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Peek Operations
 * ============================================================================ */

static void test_queue_peek_single_item(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push an item */
    TEST_QUEUE_DATA_T tPushData = CreateTestData(777, false, 7);
    JUNO_POINTER_T tPushPointer = TestQueueData_PointerInit(&tPushData);
    tStatus = JunoDs_QueuePush(&gtTestQueue, tPushPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Peek at the item */
    JUNO_RESULT_POINTER_T tResult = JunoDs_QueuePeek(&gtTestQueue);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    TEST_ASSERT_NOT_NULL(tResult.tOk.pvAddr);
    
    TEST_QUEUE_DATA_T *ptPeekedData = (TEST_QUEUE_DATA_T *)tResult.tOk.pvAddr;
    TEST_ASSERT_EQUAL(777, ptPeekedData->iValue);
    TEST_ASSERT_EQUAL(false, ptPeekedData->bFlag);
    TEST_ASSERT_EQUAL(7, ptPeekedData->iCounter);
    
    /* Verify length unchanged */
    TEST_ASSERT_EQUAL(1, gtTestQueue.zLength);
}

static void test_queue_peek_multiple_times(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push items */
    for (uint32_t i = 0; i < 3; i++)
    {
        TEST_QUEUE_DATA_T tData = CreateTestData(i, true, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestQueueData_PointerInit(&tData);
        tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    /* Peek multiple times - should always return first item */
    for (int i = 0; i < 5; i++)
    {
        JUNO_RESULT_POINTER_T tResult = JunoDs_QueuePeek(&gtTestQueue);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
        TEST_QUEUE_DATA_T *ptData = (TEST_QUEUE_DATA_T *)tResult.tOk.pvAddr;
        TEST_ASSERT_EQUAL(0, ptData->iValue);
    }
    
    TEST_ASSERT_EQUAL(3, gtTestQueue.zLength);
}

static void test_queue_peek_empty_queue(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    JUNO_RESULT_POINTER_T tResult = JunoDs_QueuePeek(&gtTestQueue);
    TEST_ASSERT_EQUAL(JUNO_STATUS_INVALID_SIZE_ERROR, tResult.tStatus);
}

static void test_queue_peek_null_queue(void)
{
    JUNO_RESULT_POINTER_T tResult = JunoDs_QueuePeek(NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
}

static void test_queue_peek_after_pop(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push two items */
    for (uint32_t i = 0; i < 2; i++)
    {
        TEST_QUEUE_DATA_T tData = CreateTestData(i * 10, true, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestQueueData_PointerInit(&tData);
        tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    /* Pop first item */
    TEST_QUEUE_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestQueueData_PointerInit(&tPopData);
    tStatus = JunoDs_QueuePop(&gtTestQueue, tPopPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Peek should now return second item */
    JUNO_RESULT_POINTER_T tResult = JunoDs_QueuePeek(&gtTestQueue);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    TEST_QUEUE_DATA_T *ptData = (TEST_QUEUE_DATA_T *)tResult.tOk.pvAddr;
    TEST_ASSERT_EQUAL(10, ptData->iValue);
}

/* ============================================================================
 * Test Cases: Circular Buffer Wraparound
 * ============================================================================ */

static void test_queue_circular_wraparound(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, 5);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Fill queue */
    for (uint32_t i = 0; i < 5; i++)
    {
        TEST_QUEUE_DATA_T tData = CreateTestData(i, true, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestQueueData_PointerInit(&tData);
        tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    /* Pop 3 items */
    for (uint32_t i = 0; i < 3; i++)
    {
        TEST_QUEUE_DATA_T tPopData = {0};
        JUNO_POINTER_T tPopPointer = TestQueueData_PointerInit(&tPopData);
        tStatus = JunoDs_QueuePop(&gtTestQueue, tPopPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_EQUAL(i, tPopData.iValue);
    }
    
    TEST_ASSERT_EQUAL(3, gtTestQueue.iStartIndex);
    TEST_ASSERT_EQUAL(2, gtTestQueue.zLength);
    
    /* Push 3 more items (should wrap around) */
    for (uint32_t i = 10; i < 13; i++)
    {
        TEST_QUEUE_DATA_T tData = CreateTestData(i, false, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestQueueData_PointerInit(&tData);
        tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    TEST_ASSERT_EQUAL(5, gtTestQueue.zLength);
    
    /* Verify FIFO order maintained across wraparound */
    uint32_t expected[] = {3, 4, 10, 11, 12};
    for (uint32_t i = 0; i < 5; i++)
    {
        TEST_QUEUE_DATA_T tPopData = {0};
        JUNO_POINTER_T tPopPointer = TestQueueData_PointerInit(&tPopData);
        tStatus = JunoDs_QueuePop(&gtTestQueue, tPopPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_EQUAL(expected[i], tPopData.iValue);
    }
}

static void test_queue_wraparound_at_boundary(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, 4);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Perform multiple push/pop cycles to test wraparound */
    for (int cycle = 0; cycle < 3; cycle++)
    {
        /* Fill queue */
        for (uint32_t i = 0; i < 4; i++)
        {
            TEST_QUEUE_DATA_T tData = CreateTestData(cycle * 100 + i, true, (uint8_t)i);
            JUNO_POINTER_T tPointer = TestQueueData_PointerInit(&tData);
            tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        }
        
        /* Empty queue */
        for (uint32_t i = 0; i < 4; i++)
        {
            TEST_QUEUE_DATA_T tPopData = {0};
            JUNO_POINTER_T tPopPointer = TestQueueData_PointerInit(&tPopData);
            tStatus = JunoDs_QueuePop(&gtTestQueue, tPopPointer);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
            TEST_ASSERT_EQUAL(cycle * 100 + i, tPopData.iValue);
        }
        
        /* Verify queue is empty and start index wrapped */
        TEST_ASSERT_EQUAL(0, gtTestQueue.zLength);
    }
}

/* ============================================================================
 * Test Cases: Stress Testing
 * ============================================================================ */

static void test_queue_stress_push_pop_cycles(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Perform 100 push/pop cycles */
    for (int cycle = 0; cycle < 100; cycle++)
    {
        /* Random number of items to push (1-5) */
        uint32_t pushCount = (cycle % 5) + 1;
        for (uint32_t i = 0; i < pushCount && gtTestQueue.zLength < TEST_QUEUE_CAPACITY; i++)
        {
            TEST_QUEUE_DATA_T tData = CreateTestData(cycle * 1000 + i, (i % 2 == 0), (uint8_t)i);
            JUNO_POINTER_T tPointer = TestQueueData_PointerInit(&tData);
            tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        }
        
        /* Random number of items to pop (1-3) */
        uint32_t popCount = (cycle % 3) + 1;
        for (uint32_t i = 0; i < popCount && gtTestQueue.zLength > 0; i++)
        {
            TEST_QUEUE_DATA_T tPopData = {0};
            JUNO_POINTER_T tPopPointer = TestQueueData_PointerInit(&tPopData);
            tStatus = JunoDs_QueuePop(&gtTestQueue, tPopPointer);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        }
    }
}

static void test_queue_interleaved_operations(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, 5);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push 1 */
    TEST_QUEUE_DATA_T tData1 = CreateTestData(1, true, 1);
    JUNO_POINTER_T tPointer1 = TestQueueData_PointerInit(&tData1);
    tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Peek */
    JUNO_RESULT_POINTER_T tPeekResult = JunoDs_QueuePeek(&gtTestQueue);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tPeekResult.tStatus);
    TEST_ASSERT_EQUAL(1, ((TEST_QUEUE_DATA_T *)tPeekResult.tOk.pvAddr)->iValue);
    
    /* Push 2 */
    TEST_QUEUE_DATA_T tData2 = CreateTestData(2, false, 2);
    JUNO_POINTER_T tPointer2 = TestQueueData_PointerInit(&tData2);
    tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer2);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Pop 1 */
    TEST_QUEUE_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestQueueData_PointerInit(&tPopData);
    tStatus = JunoDs_QueuePop(&gtTestQueue, tPopPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(1, tPopData.iValue);
    
    /* Peek again (should be 2) */
    tPeekResult = JunoDs_QueuePeek(&gtTestQueue);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tPeekResult.tStatus);
    TEST_ASSERT_EQUAL(2, ((TEST_QUEUE_DATA_T *)tPeekResult.tOk.pvAddr)->iValue);
    
    /* Push 3 */
    TEST_QUEUE_DATA_T tData3 = CreateTestData(3, true, 3);
    JUNO_POINTER_T tPointer3 = TestQueueData_PointerInit(&tData3);
    tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer3);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Pop remaining */
    for (uint32_t expected = 2; expected <= 3; expected++)
    {
        tStatus = JunoDs_QueuePop(&gtTestQueue, tPopPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_EQUAL(expected, tPopData.iValue);
    }
    
    TEST_ASSERT_EQUAL(0, gtTestQueue.zLength);
}

/* ============================================================================
 * Test Cases: API Verification
 * ============================================================================ */

static void test_queue_verify_valid_queue(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    tStatus = JunoDs_QueueVerify(&gtTestQueue);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_queue_verify_null_queue(void)
{
    JUNO_STATUS_T tStatus = JunoDs_QueueVerify(NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_queue_verify_invalid_api(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Corrupt the API */
    gtTestQueue.ptApi = NULL;
    
    tStatus = JunoDs_QueueVerify(&gtTestQueue);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_queue_api_verify_valid(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    tStatus = JunoDs_QueueApiVerify(gtTestQueue.ptApi);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_queue_api_verify_null(void)
{
    JUNO_STATUS_T tStatus = JunoDs_QueueApiVerify(NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_queue_api_verify_missing_functions(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    JUNO_DS_QUEUE_API_T tInvalidApi = *gtTestQueue.ptApi;
    tInvalidApi.Enqueue = NULL;
    
    tStatus = JunoDs_QueueApiVerify(&tInvalidApi);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    tInvalidApi = *gtTestQueue.ptApi;
    tInvalidApi.Dequeue = NULL;
    tStatus = JunoDs_QueueApiVerify(&tInvalidApi);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    tInvalidApi = *gtTestQueue.ptApi;
    tInvalidApi.Peek = NULL;
    tStatus = JunoDs_QueueApiVerify(&tInvalidApi);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Edge Cases
 * ============================================================================ */

static void test_queue_single_element_capacity(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, 1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push one item */
    TEST_QUEUE_DATA_T tData = CreateTestData(99, true, 9);
    JUNO_POINTER_T tPointer = TestQueueData_PointerInit(&tData);
    tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Queue is full */
    tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_INVALID_SIZE_ERROR, tStatus);
    
    /* Pop the item */
    TEST_QUEUE_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestQueueData_PointerInit(&tPopData);
    tStatus = JunoDs_QueuePop(&gtTestQueue, tPopPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(99, tPopData.iValue);
    
    /* Queue is empty */
    tStatus = JunoDs_QueuePop(&gtTestQueue, tPopPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, tStatus);
}

static void test_queue_boundary_index_calculation(void)
{
    /* Test with capacity that will test modulo arithmetic */
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, 7);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Fill and empty queue multiple times to exercise index wrap */
    for (int round = 0; round < 3; round++)
    {
        for (uint32_t i = 0; i < 7; i++)
        {
            TEST_QUEUE_DATA_T tData = CreateTestData(round * 100 + i, true, (uint8_t)i);
            JUNO_POINTER_T tPointer = TestQueueData_PointerInit(&tData);
            tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        }
        
        for (uint32_t i = 0; i < 7; i++)
        {
            TEST_QUEUE_DATA_T tPopData = {0};
            JUNO_POINTER_T tPopPointer = TestQueueData_PointerInit(&tPopData);
            tStatus = JunoDs_QueuePop(&gtTestQueue, tPopPointer);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
            TEST_ASSERT_EQUAL(round * 100 + i, tPopData.iValue);
        }
    }
}

static void test_queue_data_integrity_after_operations(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push items with specific patterns */
    for (uint32_t i = 0; i < 5; i++)
    {
        TEST_QUEUE_DATA_T tData = CreateTestData(0xAA550000 | i, (i & 1), (uint8_t)(0xFF - i));
        JUNO_POINTER_T tPointer = TestQueueData_PointerInit(&tData);
        tStatus = JunoDs_QueuePush(&gtTestQueue, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    /* Verify data integrity */
    for (uint32_t i = 0; i < 5; i++)
    {
        TEST_QUEUE_DATA_T tPopData = {0};
        JUNO_POINTER_T tPopPointer = TestQueueData_PointerInit(&tPopData);
        tStatus = JunoDs_QueuePop(&gtTestQueue, tPopPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_EQUAL(0xAA550000 | i, tPopData.iValue);
        TEST_ASSERT_EQUAL((i & 1), tPopData.bFlag);
        TEST_ASSERT_EQUAL(0xFF - i, tPopData.iCounter);
    }
}

/* ============================================================================
 * Test Cases: Array API Error Paths
 * ============================================================================ */

static void test_queue_array_api_set_at_invalid_api(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, TEST_QUEUE_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Corrupt the array API to test error path */
    const JUNO_DS_ARRAY_API_T *ptOriginalApi = gtTestQueue.ptQueueArray->ptApi;
    static JUNO_DS_ARRAY_API_T tBadApi = {0};
    tBadApi = *ptOriginalApi;
    gtTestQueue.ptQueueArray->ptApi = &tBadApi;
    
    TEST_QUEUE_DATA_T tData = CreateTestData(1, true, 1);
    JUNO_POINTER_T tPointer = TestQueueData_PointerInit(&tData);
    tStatus = TestQueue_SetAt(gtTestQueue.ptQueueArray, tPointer, 0);
    TEST_ASSERT_EQUAL(JUNO_STATUS_INVALID_TYPE_ERROR, tStatus);
    
    /* Restore */
    gtTestQueue.ptQueueArray->ptApi = ptOriginalApi;
}

static void test_queue_array_api_get_at_out_of_bounds(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, 5);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    JUNO_RESULT_POINTER_T tResult = TestQueue_GetAt(gtTestQueue.ptQueueArray, 10);
    TEST_ASSERT_EQUAL(JUNO_STATUS_OOB_ERROR, tResult.tStatus);
}

static void test_queue_array_api_remove_at_out_of_bounds(void)
{
    JUNO_STATUS_T tStatus = InitTestQueue(&gtTestQueue, 5);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    tStatus = TestQueue_RemoveAt(gtTestQueue.ptQueueArray, 5);
    TEST_ASSERT_EQUAL(JUNO_STATUS_OOB_ERROR, tStatus);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

int main(void)
{
    UNITY_BEGIN();
    
    /* Initialization Tests */
    RUN_TEST(test_queue_init_nominal);
    RUN_TEST(test_queue_init_null_queue);
    RUN_TEST(test_queue_init_null_array);
    RUN_TEST(test_queue_init_zero_capacity);
    
    /* Enqueue Tests */
    RUN_TEST(test_queue_push_single_item);
    RUN_TEST(test_queue_push_multiple_items);
    RUN_TEST(test_queue_push_to_full_capacity);
    RUN_TEST(test_queue_push_overflow);
    RUN_TEST(test_queue_push_null_queue);
    RUN_TEST(test_queue_push_invalid_pointer);
    
    /* Dequeue Tests */
    RUN_TEST(test_queue_pop_single_item);
    RUN_TEST(test_queue_pop_multiple_items_fifo_order);
    RUN_TEST(test_queue_pop_empty_queue);
    RUN_TEST(test_queue_pop_null_queue);
    RUN_TEST(test_queue_pop_invalid_pointer);
    
    /* Peek Tests */
    RUN_TEST(test_queue_peek_single_item);
    RUN_TEST(test_queue_peek_multiple_times);
    RUN_TEST(test_queue_peek_empty_queue);
    RUN_TEST(test_queue_peek_null_queue);
    RUN_TEST(test_queue_peek_after_pop);
    
    /* Circular Buffer Tests */
    RUN_TEST(test_queue_circular_wraparound);
    RUN_TEST(test_queue_wraparound_at_boundary);
    
    /* Stress Tests */
    RUN_TEST(test_queue_stress_push_pop_cycles);
    RUN_TEST(test_queue_interleaved_operations);
    
    /* API Verification Tests */
    RUN_TEST(test_queue_verify_valid_queue);
    RUN_TEST(test_queue_verify_null_queue);
    RUN_TEST(test_queue_verify_invalid_api);
    RUN_TEST(test_queue_api_verify_valid);
    RUN_TEST(test_queue_api_verify_null);
    RUN_TEST(test_queue_api_verify_missing_functions);
    
    /* Edge Case Tests */
    RUN_TEST(test_queue_single_element_capacity);
    RUN_TEST(test_queue_boundary_index_calculation);
    RUN_TEST(test_queue_data_integrity_after_operations);
    
    /* Array API Error Path Tests */
    RUN_TEST(test_queue_array_api_set_at_invalid_api);
    RUN_TEST(test_queue_array_api_get_at_out_of_bounds);
    RUN_TEST(test_queue_array_api_remove_at_out_of_bounds);
    
    return UNITY_END();
}
