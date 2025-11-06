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
 * @file test_stack.c
 * @brief Comprehensive unit tests for the LibJuno Stack API
 * @author Robin Onsay
 * 
 * This test suite aims for 100% code and branch coverage of the stack
 * implementation, testing all edge cases and error paths.
 */

#include "juno/ds/stack_api.h"
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

typedef struct TEST_STACK_DATA_TAG
{
    uint32_t iValue;
    bool bFlag;
    uint8_t iCounter;
} TEST_STACK_DATA_T;

/* ============================================================================
 * Test Stack Implementation (Custom Array-backed Stack)
 * ============================================================================ */

typedef struct TEST_ARRAY_TAG JUNO_MODULE_DERIVE(JUNO_DS_ARRAY_ROOT_T,
    TEST_STACK_DATA_T *ptBuffer;
) TEST_ARRAY_T;

typedef struct TEST_STACK_TAG
{
    JUNO_DS_STACK_ROOT_T tRoot;
    TEST_ARRAY_T tArray;
} TEST_STACK_T;

/* Forward declarations for array API functions */
static JUNO_STATUS_T TestStack_SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
static JUNO_RESULT_POINTER_T TestStack_GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
static JUNO_STATUS_T TestStack_RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);

/* Forward declarations for pointer API functions */
static JUNO_STATUS_T TestStackData_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
static JUNO_STATUS_T TestStackData_Reset(JUNO_POINTER_T tPointer);

/* Pointer API for TEST_STACK_DATA_T */
const JUNO_POINTER_API_T gtTestStackDataPointerApi = {
    TestStackData_Copy,
    TestStackData_Reset
};

/* Array API for TEST_ARRAY_T */
static const JUNO_DS_ARRAY_API_T gtTestArrayApi = {
    TestStack_SetAt,
    TestStack_GetAt,
    TestStack_RemoveAt
};

/* Macro to verify pointer type */
#define TestStackData_PointerInit(addr) JunoMemory_PointerInit(&gtTestStackDataPointerApi, TEST_STACK_DATA_T, addr)
#define TestStackData_PointerVerify(tPointer) JunoMemory_PointerVerifyType(tPointer, TEST_STACK_DATA_T, gtTestStackDataPointerApi)
#define TEST_ARRAY_ASSERT_API(ptArray, ...)  if(ptArray->ptApi != &gtTestArrayApi) { __VA_ARGS__; }

/* ============================================================================
 * Pointer API Implementation
 * ============================================================================ */

static JUNO_STATUS_T TestStackData_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc)
{
    JUNO_STATUS_T tStatus = TestStackData_PointerVerify(tDest);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = TestStackData_PointerVerify(tSrc);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    *(TEST_STACK_DATA_T *)tDest.pvAddr = *(TEST_STACK_DATA_T *)tSrc.pvAddr;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestStackData_Reset(JUNO_POINTER_T tPointer)
{
    JUNO_STATUS_T tStatus = TestStackData_PointerVerify(tPointer);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    *(TEST_STACK_DATA_T *)tPointer.pvAddr = (TEST_STACK_DATA_T){0};
    return JUNO_STATUS_SUCCESS;
}

/* ============================================================================
 * Array API Implementation
 * ============================================================================ */

static JUNO_STATUS_T TestStack_SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    TEST_ARRAY_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    tStatus = TestStackData_PointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    
    TEST_ARRAY_T *ptTestArray = (TEST_ARRAY_T *)ptArray;
    JUNO_POINTER_T tIndexPointer = TestStackData_PointerInit(&ptTestArray->ptBuffer[iIndex]);
    tStatus = tIndexPointer.ptApi->Copy(tIndexPointer, tItem);
    return tStatus;
}

static JUNO_RESULT_POINTER_T TestStack_GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_RESULT_POINTER_T tResult = {0};
    tResult.tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    TEST_ARRAY_ASSERT_API(ptArray,
        tResult.tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        return tResult;
    );
    tResult.tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    
    TEST_ARRAY_T *ptTestArray = (TEST_ARRAY_T *)ptArray;
    JUNO_POINTER_T tIndexPointer = TestStackData_PointerInit(&ptTestArray->ptBuffer[iIndex]);
    tResult.tOk = tIndexPointer;
    return tResult;
}

static JUNO_STATUS_T TestStack_RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    TEST_ARRAY_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    
    TEST_ARRAY_T *ptTestArray = (TEST_ARRAY_T *)ptArray;
    JUNO_POINTER_T tIndexPointer = TestStackData_PointerInit(&ptTestArray->ptBuffer[iIndex]);
    tStatus = tIndexPointer.ptApi->Reset(tIndexPointer);
    return tStatus;
}

/* ============================================================================
 * Test Fixture Setup/Teardown
 * ============================================================================ */

#define TEST_STACK_CAPACITY 10
static TEST_STACK_T gtTestStack;
static TEST_STACK_DATA_T gtTestStackBuffer[TEST_STACK_CAPACITY];

void setUp(void)
{
    memset(&gtTestStack, 0, sizeof(gtTestStack));
    memset(gtTestStackBuffer, 0, sizeof(gtTestStackBuffer));
}

void tearDown(void)
{
    /* No dynamic allocations to clean up */
}

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static JUNO_STATUS_T InitTestStack(TEST_STACK_T *ptStack, size_t zCapacity)
{
    ptStack->tArray.ptBuffer = gtTestStackBuffer;
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(&ptStack->tArray.tRoot, &gtTestArrayApi, zCapacity, NULL, NULL);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return JunoDs_StackInit(&ptStack->tRoot, &ptStack->tArray.tRoot, NULL, NULL);
}

static TEST_STACK_DATA_T CreateTestData(uint32_t iValue, bool bFlag, uint8_t iCounter)
{
    TEST_STACK_DATA_T tData = {
        .iValue = iValue,
        .bFlag = bFlag,
        .iCounter = iCounter
    };
    return tData;
}

/* ============================================================================
 * Test Cases: Initialization
 * ============================================================================ */

static void test_stack_init_nominal(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(0, gtTestStack.tRoot.zLength);
    TEST_ASSERT_EQUAL(TEST_STACK_CAPACITY, gtTestStack.tArray.tRoot.zCapacity);
    TEST_ASSERT_NOT_NULL(gtTestStack.tRoot.ptApi);
}

static void test_stack_init_null_stack(void)
{
    JUNO_STATUS_T tStatus = JunoDs_StackInit(NULL, &gtTestStack.tArray.tRoot, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_stack_init_null_api(void)
{
    gtTestStack.tArray.ptBuffer = gtTestStackBuffer;
    JUNO_STATUS_T tStatus = JunoDs_StackInit(&gtTestStack.tRoot, NULL, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_stack_init_zero_capacity(void)
{
    gtTestStack.tArray.ptBuffer = gtTestStackBuffer;
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(&gtTestStack.tArray.tRoot, &gtTestArrayApi, 0, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Push Operations
 * ============================================================================ */

static void test_stack_push_single_item(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    TEST_STACK_DATA_T tData = CreateTestData(42, true, 1);
    JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
    
    tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(1, gtTestStack.tRoot.zLength);
    TEST_ASSERT_EQUAL(42, gtTestStackBuffer[0].iValue);
    TEST_ASSERT_EQUAL(true, gtTestStackBuffer[0].bFlag);
    TEST_ASSERT_EQUAL(1, gtTestStackBuffer[0].iCounter);
}

static void test_stack_push_multiple_items(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    for (uint32_t i = 0; i < 5; i++)
    {
        TEST_STACK_DATA_T tData = CreateTestData(i * 10, (i % 2 == 0), (uint8_t)i);
        JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
        tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_EQUAL(i + 1, gtTestStack.tRoot.zLength);
    }
    
    /* Verify all items were stored correctly */
    for (uint32_t i = 0; i < 5; i++)
    {
        TEST_ASSERT_EQUAL(i * 10, gtTestStackBuffer[i].iValue);
        TEST_ASSERT_EQUAL((i % 2 == 0), gtTestStackBuffer[i].bFlag);
        TEST_ASSERT_EQUAL(i, gtTestStackBuffer[i].iCounter);
    }
}

static void test_stack_push_to_full_capacity(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Fill stack to capacity */
    for (uint32_t i = 0; i < TEST_STACK_CAPACITY; i++)
    {
        TEST_STACK_DATA_T tData = CreateTestData(i, true, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
        tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    TEST_ASSERT_EQUAL(TEST_STACK_CAPACITY, gtTestStack.tRoot.zLength);
}

static void test_stack_push_overflow(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Fill stack to capacity */
    for (uint32_t i = 0; i < TEST_STACK_CAPACITY; i++)
    {
        TEST_STACK_DATA_T tData = CreateTestData(i, true, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
        tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    /* Attempt to push one more item (should fail) */
    TEST_STACK_DATA_T tData = CreateTestData(999, false, 99);
    JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
    tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_OOB_ERROR, tStatus);
    TEST_ASSERT_EQUAL(TEST_STACK_CAPACITY, gtTestStack.tRoot.zLength);
}

static void test_stack_push_null_stack(void)
{
    TEST_STACK_DATA_T tData = CreateTestData(42, true, 1);
    JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
    
    JUNO_STATUS_T tStatus = JunoDs_StackPush(NULL, tPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_stack_push_invalid_pointer(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    JUNO_POINTER_T tInvalidPointer = {0};
    tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tInvalidPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Pop Operations
 * ============================================================================ */

static void test_stack_pop_single_item(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push an item */
    TEST_STACK_DATA_T tPushData = CreateTestData(123, true, 5);
    JUNO_POINTER_T tPushPointer = TestStackData_PointerInit(&tPushData);
    tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPushPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Pop the item */
    TEST_STACK_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestStackData_PointerInit(&tPopData);
    tStatus = JunoDs_StackPop(&gtTestStack.tRoot, tPopPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(0, gtTestStack.tRoot.zLength);
    TEST_ASSERT_EQUAL(123, tPopData.iValue);
    TEST_ASSERT_EQUAL(true, tPopData.bFlag);
    TEST_ASSERT_EQUAL(5, tPopData.iCounter);
}

static void test_stack_pop_multiple_items_lifo_order(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push multiple items */
    for (uint32_t i = 0; i < 5; i++)
    {
        TEST_STACK_DATA_T tData = CreateTestData(i * 100, (i % 2 == 0), (uint8_t)i);
        JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
        tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    /* Pop items and verify LIFO order (last in, first out) */
    for (int i = 4; i >= 0; i--)
    {
        TEST_STACK_DATA_T tPopData = {0};
        JUNO_POINTER_T tPopPointer = TestStackData_PointerInit(&tPopData);
        tStatus = JunoDs_StackPop(&gtTestStack.tRoot, tPopPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_EQUAL((uint32_t)i * 100, tPopData.iValue);
        TEST_ASSERT_EQUAL((i % 2 == 0), tPopData.bFlag);
        TEST_ASSERT_EQUAL((uint8_t)i, tPopData.iCounter);
    }
    
    TEST_ASSERT_EQUAL(0, gtTestStack.tRoot.zLength);
}

static void test_stack_pop_empty_stack(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    TEST_STACK_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestStackData_PointerInit(&tPopData);
    tStatus = JunoDs_StackPop(&gtTestStack.tRoot, tPopPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_OOB_ERROR, tStatus);
}

static void test_stack_pop_null_stack(void)
{
    TEST_STACK_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestStackData_PointerInit(&tPopData);
    JUNO_STATUS_T tStatus = JunoDs_StackPop(NULL, tPopPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_stack_pop_invalid_pointer(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push an item */
    TEST_STACK_DATA_T tPushData = CreateTestData(42, true, 1);
    JUNO_POINTER_T tPushPointer = TestStackData_PointerInit(&tPushData);
    tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPushPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Try to pop with invalid pointer */
    JUNO_POINTER_T tInvalidPointer = {0};
    tStatus = JunoDs_StackPop(&gtTestStack.tRoot, tInvalidPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Peek Operations
 * ============================================================================ */

static void test_stack_peek_single_item(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push an item */
    TEST_STACK_DATA_T tPushData = CreateTestData(777, false, 7);
    JUNO_POINTER_T tPushPointer = TestStackData_PointerInit(&tPushData);
    tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPushPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Peek at the item */
    JUNO_RESULT_POINTER_T tResult = JunoDs_StackPeek(&gtTestStack.tRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    TEST_ASSERT_NOT_NULL(tResult.tOk.pvAddr);
    
    TEST_STACK_DATA_T *ptPeekedData = (TEST_STACK_DATA_T *)tResult.tOk.pvAddr;
    TEST_ASSERT_EQUAL(777, ptPeekedData->iValue);
    TEST_ASSERT_EQUAL(false, ptPeekedData->bFlag);
    TEST_ASSERT_EQUAL(7, ptPeekedData->iCounter);
    
    /* Verify length unchanged */
    TEST_ASSERT_EQUAL(1, gtTestStack.tRoot.zLength);
}

static void test_stack_peek_top_element(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push items */
    for (uint32_t i = 0; i < 3; i++)
    {
        TEST_STACK_DATA_T tData = CreateTestData(i, true, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
        tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    /* Peek should return the top element (LIFO - last pushed item) */
    JUNO_RESULT_POINTER_T tResult = JunoDs_StackPeek(&gtTestStack.tRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    TEST_STACK_DATA_T *ptData = (TEST_STACK_DATA_T *)tResult.tOk.pvAddr;
    TEST_ASSERT_EQUAL(2, ptData->iValue);  /* Top element is the last pushed (index 2) */
    
    TEST_ASSERT_EQUAL(3, gtTestStack.tRoot.zLength);
}

static void test_stack_peek_multiple_times(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push items */
    for (uint32_t i = 0; i < 3; i++)
    {
        TEST_STACK_DATA_T tData = CreateTestData(i, true, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
        tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    /* Peek multiple times - should always return the top item (LIFO semantics) */
    for (int i = 0; i < 5; i++)
    {
        JUNO_RESULT_POINTER_T tResult = JunoDs_StackPeek(&gtTestStack.tRoot);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
        TEST_STACK_DATA_T *ptData = (TEST_STACK_DATA_T *)tResult.tOk.pvAddr;
        TEST_ASSERT_EQUAL(2, ptData->iValue);  /* Top element is the last pushed (value 2) */
    }
    
    TEST_ASSERT_EQUAL(3, gtTestStack.tRoot.zLength);
}

static void test_stack_peek_empty_stack(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    JUNO_RESULT_POINTER_T tResult = JunoDs_StackPeek(&gtTestStack.tRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_OOB_ERROR, tResult.tStatus);
}

static void test_stack_peek_null_stack(void)
{
    JUNO_RESULT_POINTER_T tResult = JunoDs_StackPeek(NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
}

static void test_stack_peek_after_pop(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push three items */
    for (uint32_t i = 0; i < 3; i++)
    {
        TEST_STACK_DATA_T tData = CreateTestData(i * 10, true, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
        tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    /* Pop top item */
    TEST_STACK_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestStackData_PointerInit(&tPopData);
    tStatus = JunoDs_StackPop(&gtTestStack.tRoot, tPopPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(20, tPopData.iValue);  /* Top was value 20 */
    
    /* Peek should return the new top item (LIFO - next item is value 10) */
    JUNO_RESULT_POINTER_T tResult = JunoDs_StackPeek(&gtTestStack.tRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    TEST_STACK_DATA_T *ptData = (TEST_STACK_DATA_T *)tResult.tOk.pvAddr;
    TEST_ASSERT_EQUAL(10, ptData->iValue);  /* New top element after pop is value 10 */
}

/* ============================================================================
 * Test Cases: Stress Testing
 * ============================================================================ */

static void test_stack_stress_push_pop_cycles(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Perform 100 push/pop cycles */
    for (int cycle = 0; cycle < 100; cycle++)
    {
        /* Random number of items to push (1-5) */
        uint32_t pushCount = (cycle % 5) + 1;
        for (uint32_t i = 0; i < pushCount && gtTestStack.tRoot.zLength < TEST_STACK_CAPACITY; i++)
        {
            TEST_STACK_DATA_T tData = CreateTestData(cycle * 1000 + i, (i % 2 == 0), (uint8_t)i);
            JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
            tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        }
        
        /* Random number of items to pop (1-3) */
        uint32_t popCount = (cycle % 3) + 1;
        for (uint32_t i = 0; i < popCount && gtTestStack.tRoot.zLength > 0; i++)
        {
            TEST_STACK_DATA_T tPopData = {0};
            JUNO_POINTER_T tPopPointer = TestStackData_PointerInit(&tPopData);
            tStatus = JunoDs_StackPop(&gtTestStack.tRoot, tPopPointer);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        }
    }
}

static void test_stack_interleaved_operations(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, 5);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push 1 */
    TEST_STACK_DATA_T tData1 = CreateTestData(1, true, 1);
    JUNO_POINTER_T tPointer1 = TestStackData_PointerInit(&tData1);
    tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Peek */
    JUNO_RESULT_POINTER_T tPeekResult = JunoDs_StackPeek(&gtTestStack.tRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tPeekResult.tStatus);
    TEST_ASSERT_EQUAL(1, ((TEST_STACK_DATA_T *)tPeekResult.tOk.pvAddr)->iValue);
    
    /* Push 2 */
    TEST_STACK_DATA_T tData2 = CreateTestData(2, false, 2);
    JUNO_POINTER_T tPointer2 = TestStackData_PointerInit(&tData2);
    tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer2);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Peek (should be 2 - LIFO semantics, last pushed item is on top) */
    tPeekResult = JunoDs_StackPeek(&gtTestStack.tRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tPeekResult.tStatus);
    TEST_ASSERT_EQUAL(2, ((TEST_STACK_DATA_T *)tPeekResult.tOk.pvAddr)->iValue);
    
    /* Pop 1 (should get 2) */
    TEST_STACK_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestStackData_PointerInit(&tPopData);
    tStatus = JunoDs_StackPop(&gtTestStack.tRoot, tPopPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(2, tPopData.iValue);
    
    /* Peek again (should be 1) */
    tPeekResult = JunoDs_StackPeek(&gtTestStack.tRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tPeekResult.tStatus);
    TEST_ASSERT_EQUAL(1, ((TEST_STACK_DATA_T *)tPeekResult.tOk.pvAddr)->iValue);
    
    /* Push 3 */
    TEST_STACK_DATA_T tData3 = CreateTestData(3, true, 3);
    JUNO_POINTER_T tPointer3 = TestStackData_PointerInit(&tData3);
    tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer3);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Pop remaining (should be 3, then 1) */
    tStatus = JunoDs_StackPop(&gtTestStack.tRoot, tPopPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(3, tPopData.iValue);
    
    tStatus = JunoDs_StackPop(&gtTestStack.tRoot, tPopPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(1, tPopData.iValue);
    
    TEST_ASSERT_EQUAL(0, gtTestStack.tRoot.zLength);
}

static void test_stack_fill_empty_cycles(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Perform multiple fill/empty cycles */
    for (int cycle = 0; cycle < 5; cycle++)
    {
        /* Fill stack completely */
        for (uint32_t i = 0; i < TEST_STACK_CAPACITY; i++)
        {
            TEST_STACK_DATA_T tData = CreateTestData(cycle * 100 + i, true, (uint8_t)i);
            JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
            tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        }
        
        TEST_ASSERT_EQUAL(TEST_STACK_CAPACITY, gtTestStack.tRoot.zLength);
        
        /* Empty stack completely (verify LIFO order) */
        for (int i = TEST_STACK_CAPACITY - 1; i >= 0; i--)
        {
            TEST_STACK_DATA_T tPopData = {0};
            JUNO_POINTER_T tPopPointer = TestStackData_PointerInit(&tPopData);
            tStatus = JunoDs_StackPop(&gtTestStack.tRoot, tPopPointer);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
            TEST_ASSERT_EQUAL(cycle * 100 + (uint32_t)i, tPopData.iValue);
        }
        
        TEST_ASSERT_EQUAL(0, gtTestStack.tRoot.zLength);
    }
}

/* ============================================================================
 * Test Cases: API Verification
 * ============================================================================ */

static void test_stack_verify_valid_stack(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    tStatus = JunoDs_StackVerify(&gtTestStack.tRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_stack_verify_null_stack(void)
{
    JUNO_STATUS_T tStatus = JunoDs_StackVerify(NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_stack_verify_invalid_api(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Corrupt the API */
    gtTestStack.tRoot.ptApi = NULL;
    
    tStatus = JunoDs_StackVerify(&gtTestStack.tRoot);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_stack_verify_missing_push_function(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Create invalid API with missing Push */
    static JUNO_DS_STACK_API_T tInvalidApi = {0};
    tInvalidApi.Pop = JunoDs_StackPop;
    tInvalidApi.Peek = JunoDs_StackPeek;
    gtTestStack.tRoot.ptApi = &tInvalidApi;
    
    tStatus = JunoDs_StackVerify(&gtTestStack.tRoot);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_stack_verify_missing_pop_function(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Create invalid API with missing Pop */
    static JUNO_DS_STACK_API_T tInvalidApi = {0};
    tInvalidApi.Push = JunoDs_StackPush;
    tInvalidApi.Peek = JunoDs_StackPeek;
    gtTestStack.tRoot.ptApi = &tInvalidApi;
    
    tStatus = JunoDs_StackVerify(&gtTestStack.tRoot);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_stack_verify_missing_peek_function(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Create invalid API with missing Peek */
    static JUNO_DS_STACK_API_T tInvalidApi = {0};
    tInvalidApi.Push = JunoDs_StackPush;
    tInvalidApi.Pop = JunoDs_StackPop;
    gtTestStack.tRoot.ptApi = &tInvalidApi;
    
    tStatus = JunoDs_StackVerify(&gtTestStack.tRoot);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Edge Cases
 * ============================================================================ */

static void test_stack_single_element_capacity(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, 1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push one item */
    TEST_STACK_DATA_T tData = CreateTestData(99, true, 9);
    JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
    tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Stack is full */
    tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_OOB_ERROR, tStatus);
    
    /* Peek at item */
    JUNO_RESULT_POINTER_T tPeekResult = JunoDs_StackPeek(&gtTestStack.tRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tPeekResult.tStatus);
    TEST_ASSERT_EQUAL(99, ((TEST_STACK_DATA_T *)tPeekResult.tOk.pvAddr)->iValue);
    
    /* Pop the item */
    TEST_STACK_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestStackData_PointerInit(&tPopData);
    tStatus = JunoDs_StackPop(&gtTestStack.tRoot, tPopPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(99, tPopData.iValue);
    
    /* Stack is empty */
    tStatus = JunoDs_StackPop(&gtTestStack.tRoot, tPopPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_OOB_ERROR, tStatus);
}

static void test_stack_data_integrity_after_operations(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push items with specific patterns */
    for (uint32_t i = 0; i < 5; i++)
    {
        TEST_STACK_DATA_T tData = CreateTestData(0xAA550000 | i, (i & 1), (uint8_t)(0xFF - i));
        JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
        tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    /* Verify data integrity in LIFO order */
    for (int i = 4; i >= 0; i--)
    {
        TEST_STACK_DATA_T tPopData = {0};
        JUNO_POINTER_T tPopPointer = TestStackData_PointerInit(&tPopData);
        tStatus = JunoDs_StackPop(&gtTestStack.tRoot, tPopPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_EQUAL(0xAA550000 | (uint32_t)i, tPopData.iValue);
        TEST_ASSERT_EQUAL((i & 1), tPopData.bFlag);
        TEST_ASSERT_EQUAL(0xFF - (uint8_t)i, tPopData.iCounter);
    }
}

static void test_stack_alternating_push_pop(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Alternate between push and pop operations */
    for (uint32_t i = 0; i < 20; i++)
    {
        /* Push */
        TEST_STACK_DATA_T tData = CreateTestData(i, (i % 2 == 0), (uint8_t)i);
        JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
        tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        
        /* Peek */
        JUNO_RESULT_POINTER_T tPeekResult = JunoDs_StackPeek(&gtTestStack.tRoot);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tPeekResult.tStatus);
        TEST_ASSERT_EQUAL(i, ((TEST_STACK_DATA_T *)tPeekResult.tOk.pvAddr)->iValue);
        
        /* Pop */
        TEST_STACK_DATA_T tPopData = {0};
        JUNO_POINTER_T tPopPointer = TestStackData_PointerInit(&tPopData);
        tStatus = JunoDs_StackPop(&gtTestStack.tRoot, tPopPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_EQUAL(i, tPopData.iValue);
        
        /* Stack should be empty after each cycle */
        TEST_ASSERT_EQUAL(0, gtTestStack.tRoot.zLength);
    }
}

static void test_stack_peek_consistency_during_operations(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push and verify peek after each push */
    for (uint32_t i = 0; i < 5; i++)
    {
        TEST_STACK_DATA_T tData = CreateTestData(i * 10, true, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
        tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        
        /* Peek should always show the top item (LIFO - last pushed item) */
        JUNO_RESULT_POINTER_T tPeekResult = JunoDs_StackPeek(&gtTestStack.tRoot);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tPeekResult.tStatus);
        TEST_ASSERT_EQUAL(i * 10, ((TEST_STACK_DATA_T *)tPeekResult.tOk.pvAddr)->iValue);  /* Top item is the last pushed */
    }
}

/* ============================================================================
 * Test Cases: Array API Error Paths
 * ============================================================================ */

static void test_stack_array_api_set_at_invalid_api(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Corrupt the array API to test error path */
    const JUNO_DS_ARRAY_API_T *ptOriginalApi = gtTestStack.tArray.tRoot.ptApi;
    static JUNO_DS_ARRAY_API_T tBadApi = {0};
    tBadApi = *ptOriginalApi;
    gtTestStack.tArray.tRoot.ptApi = &tBadApi;
    
    TEST_STACK_DATA_T tData = CreateTestData(1, true, 1);
    JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
    tStatus = TestStack_SetAt(&gtTestStack.tArray.tRoot, tPointer, 0);
    TEST_ASSERT_EQUAL(JUNO_STATUS_INVALID_TYPE_ERROR, tStatus);
    
    /* Restore */
    gtTestStack.tArray.tRoot.ptApi = ptOriginalApi;
}

static void test_stack_array_api_get_at_out_of_bounds(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, 5);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    JUNO_RESULT_POINTER_T tResult = TestStack_GetAt(&gtTestStack.tArray.tRoot, 10);
    TEST_ASSERT_EQUAL(JUNO_STATUS_OOB_ERROR, tResult.tStatus);
}

static void test_stack_array_api_remove_at_out_of_bounds(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, 5);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    tStatus = TestStack_RemoveAt(&gtTestStack.tArray.tRoot, 5);
    TEST_ASSERT_EQUAL(JUNO_STATUS_OOB_ERROR, tStatus);
}

static void test_stack_array_api_null_array(void)
{
    TEST_STACK_DATA_T tData = CreateTestData(1, true, 1);
    JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
    
    JUNO_STATUS_T tStatus = TestStack_SetAt(NULL, tPointer, 0);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    JUNO_RESULT_POINTER_T tResult = TestStack_GetAt(NULL, 0);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    
    tStatus = TestStack_RemoveAt(NULL, 0);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Boundary Conditions
 * ============================================================================ */

static void test_stack_push_at_exact_capacity(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, 3);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push exactly to capacity */
    for (uint32_t i = 0; i < 3; i++)
    {
        TEST_STACK_DATA_T tData = CreateTestData(i, true, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
        tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_EQUAL(i + 1, gtTestStack.tRoot.zLength);
    }
    
    /* Verify no overflow */
    TEST_ASSERT_EQUAL(3, gtTestStack.tRoot.zLength);
    
    /* Next push should fail */
    TEST_STACK_DATA_T tData = CreateTestData(999, false, 99);
    JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
    tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_OOB_ERROR, tStatus);
}

static void test_stack_pop_until_empty(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, 3);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push items */
    for (uint32_t i = 0; i < 3; i++)
    {
        TEST_STACK_DATA_T tData = CreateTestData(i, true, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
        tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    /* Pop until empty */
    for (int i = 2; i >= 0; i--)
    {
        TEST_STACK_DATA_T tPopData = {0};
        JUNO_POINTER_T tPopPointer = TestStackData_PointerInit(&tPopData);
        tStatus = JunoDs_StackPop(&gtTestStack.tRoot, tPopPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_EQUAL((uint32_t)i, tPopData.iValue);
        TEST_ASSERT_EQUAL(i, gtTestStack.tRoot.zLength);
    }
    
    /* Verify empty */
    TEST_ASSERT_EQUAL(0, gtTestStack.tRoot.zLength);
    
    /* Next pop should fail */
    TEST_STACK_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestStackData_PointerInit(&tPopData);
    tStatus = JunoDs_StackPop(&gtTestStack.tRoot, tPopPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_OOB_ERROR, tStatus);
}

/* ============================================================================
 * Test Cases: Pointer API Coverage
 * ============================================================================ */

static void test_stack_pointer_copy_invalid_dest(void)
{
    TEST_STACK_DATA_T tSrcData = CreateTestData(42, true, 1);
    JUNO_POINTER_T tSrcPointer = TestStackData_PointerInit(&tSrcData);
    
    JUNO_POINTER_T tInvalidDest = {0};
    JUNO_STATUS_T tStatus = TestStackData_Copy(tInvalidDest, tSrcPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_stack_pointer_copy_invalid_src(void)
{
    TEST_STACK_DATA_T tDestData = {0};
    JUNO_POINTER_T tDestPointer = TestStackData_PointerInit(&tDestData);
    
    JUNO_POINTER_T tInvalidSrc = {0};
    JUNO_STATUS_T tStatus = TestStackData_Copy(tDestPointer, tInvalidSrc);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_stack_pointer_reset_invalid(void)
{
    JUNO_POINTER_T tInvalidPointer = {0};
    JUNO_STATUS_T tStatus = TestStackData_Reset(tInvalidPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_stack_pointer_reset_valid(void)
{
    TEST_STACK_DATA_T tData = CreateTestData(42, true, 5);
    JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
    
    JUNO_STATUS_T tStatus = TestStackData_Reset(tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(0, tData.iValue);
    TEST_ASSERT_EQUAL(false, tData.bFlag);
    TEST_ASSERT_EQUAL(0, tData.iCounter);
}

/* ============================================================================
 * Additional Edge Case Tests for Maximum Coverage
 * ============================================================================ */

static void test_stack_push_with_setAt_failure(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Create a corrupted array API that will cause SetAt to fail */
    static JUNO_DS_ARRAY_API_T tCorruptedArrayApi = {0}; /* All function pointers NULL */
    
    /* Replace array API with corrupted one */
    gtTestStack.tArray.tRoot.ptApi = &tCorruptedArrayApi;
    
    TEST_STACK_DATA_T tData = CreateTestData(42, true, 1);
    JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
    
    /* Push should fail due to SetAt failure */
    tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_stack_pop_with_getAt_failure(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push an item first with valid API */
    TEST_STACK_DATA_T tData = CreateTestData(42, true, 1);
    JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
    tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Create a corrupted array API that will cause GetAt to fail */
    static JUNO_DS_ARRAY_API_T tCorruptedArrayApi = {0}; /* All function pointers NULL */
    
    /* Replace array API with corrupted one */
    gtTestStack.tArray.tRoot.ptApi = &tCorruptedArrayApi;
    
    TEST_STACK_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestStackData_PointerInit(&tPopData);
    
    /* Pop should fail due to GetAt failure */
    tStatus = JunoDs_StackPop(&gtTestStack.tRoot, tPopPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_stack_peek_with_getAt_failure(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push an item first with valid API */
    TEST_STACK_DATA_T tData = CreateTestData(42, true, 1);
    JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
    tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Create a corrupted array API that will cause GetAt to fail */
    static JUNO_DS_ARRAY_API_T tCorruptedArrayApi = {0}; /* All function pointers NULL */
    
    /* Replace array API with corrupted one */
    gtTestStack.tArray.tRoot.ptApi = &tCorruptedArrayApi;
    
    /* Peek should fail due to GetAt failure */
    JUNO_RESULT_POINTER_T tResult = JunoDs_StackPeek(&gtTestStack.tRoot);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
}

static void test_stack_pop_copy_failure(void)
{
    JUNO_STATUS_T tStatus = InitTestStack(&gtTestStack, TEST_STACK_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Push an item first */
    TEST_STACK_DATA_T tData = CreateTestData(42, true, 1);
    JUNO_POINTER_T tPointer = TestStackData_PointerInit(&tData);
    tStatus = JunoDs_StackPush(&gtTestStack.tRoot, tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Create a pointer with NULL API to cause copy failure */
    TEST_STACK_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestStackData_PointerInit(&tPopData);
    tPopPointer.ptApi = NULL; /* This will cause Copy to fail */
    
    /* Pop should fail due to Copy failure */
    tStatus = JunoDs_StackPop(&gtTestStack.tRoot, tPopPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_stack_init_with_array_verify_failure(void)
{
    gtTestStack.tArray.ptBuffer = gtTestStackBuffer;
    
    /* Create a corrupted array API that will cause array verification to fail */
    static JUNO_DS_ARRAY_API_T tCorruptedArrayApi = {0}; /* All function pointers NULL */
    
    /* Init array with corrupted API */
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(&gtTestStack.tArray.tRoot, &tCorruptedArrayApi, TEST_STACK_CAPACITY, NULL, NULL);
    
    /* Array init should fail or stack init should fail due to array verification failure */
    if (tStatus == JUNO_STATUS_SUCCESS) {
        tStatus = JunoDs_StackInit(&gtTestStack.tRoot, &gtTestStack.tArray.tRoot, NULL, NULL);
    }
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

int main(void)
{
    UNITY_BEGIN();
    
    /* Initialization Tests */
    RUN_TEST(test_stack_init_nominal);
    RUN_TEST(test_stack_init_null_stack);
    RUN_TEST(test_stack_init_null_api);
    RUN_TEST(test_stack_init_zero_capacity);
    
    /* Push Tests */
    RUN_TEST(test_stack_push_single_item);
    RUN_TEST(test_stack_push_multiple_items);
    RUN_TEST(test_stack_push_to_full_capacity);
    RUN_TEST(test_stack_push_overflow);
    RUN_TEST(test_stack_push_null_stack);
    RUN_TEST(test_stack_push_invalid_pointer);
    
    /* Pop Tests */
    RUN_TEST(test_stack_pop_single_item);
    RUN_TEST(test_stack_pop_multiple_items_lifo_order);
    RUN_TEST(test_stack_pop_empty_stack);
    RUN_TEST(test_stack_pop_null_stack);
    RUN_TEST(test_stack_pop_invalid_pointer);
    
    /* Peek Tests */
    RUN_TEST(test_stack_peek_single_item);
    RUN_TEST(test_stack_peek_top_element);
    RUN_TEST(test_stack_peek_multiple_times);
    RUN_TEST(test_stack_peek_empty_stack);
    RUN_TEST(test_stack_peek_null_stack);
    RUN_TEST(test_stack_peek_after_pop);
    
    /* Stress Tests */
    RUN_TEST(test_stack_stress_push_pop_cycles);
    RUN_TEST(test_stack_interleaved_operations);
    RUN_TEST(test_stack_fill_empty_cycles);
    
    /* API Verification Tests */
    RUN_TEST(test_stack_verify_valid_stack);
    RUN_TEST(test_stack_verify_null_stack);
    RUN_TEST(test_stack_verify_invalid_api);
    RUN_TEST(test_stack_verify_missing_push_function);
    RUN_TEST(test_stack_verify_missing_pop_function);
    RUN_TEST(test_stack_verify_missing_peek_function);
    
    /* Edge Case Tests */
    RUN_TEST(test_stack_single_element_capacity);
    RUN_TEST(test_stack_data_integrity_after_operations);
    RUN_TEST(test_stack_alternating_push_pop);
    RUN_TEST(test_stack_peek_consistency_during_operations);
    
    /* Array API Error Path Tests */
    RUN_TEST(test_stack_array_api_set_at_invalid_api);
    RUN_TEST(test_stack_array_api_get_at_out_of_bounds);
    RUN_TEST(test_stack_array_api_remove_at_out_of_bounds);
    RUN_TEST(test_stack_array_api_null_array);
    
    /* Boundary Condition Tests */
    RUN_TEST(test_stack_push_at_exact_capacity);
    RUN_TEST(test_stack_pop_until_empty);
    
    /* Pointer API Coverage Tests */
    RUN_TEST(test_stack_pointer_copy_invalid_dest);
    RUN_TEST(test_stack_pointer_copy_invalid_src);
    RUN_TEST(test_stack_pointer_reset_invalid);
    RUN_TEST(test_stack_pointer_reset_valid);
    
    /* Additional Edge Cases */
    RUN_TEST(test_stack_push_with_setAt_failure);
    RUN_TEST(test_stack_pop_with_getAt_failure); 
    RUN_TEST(test_stack_peek_with_getAt_failure);
    RUN_TEST(test_stack_pop_copy_failure);
    RUN_TEST(test_stack_init_with_array_verify_failure);
    
    return UNITY_END();
}
