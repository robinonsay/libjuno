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
 * @file test_heap.c
 * @brief Comprehensive unit tests for the LibJuno Heap API
 * @author Robin Onsay
 * 
 * This test suite aims for 100% code and branch coverage of the heap
 * implementation, testing all edge cases and error paths.
 */

#include "juno/ds/heap_api.h"
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

typedef struct TEST_HEAP_DATA_TAG
{
    uint32_t iValue;
    bool bFlag;
    uint8_t iCounter;
} TEST_HEAP_DATA_T;

/* ============================================================================
 * Test Heap Implementation (Custom Array-backed Heap)
 * ============================================================================ */

/* Forward declarations for array API functions */
static JUNO_STATUS_T TestHeap_SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
static JUNO_RESULT_POINTER_T TestHeap_GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
static JUNO_STATUS_T TestHeap_RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);

/* Forward declarations for heap pointer API functions */
static JUNO_DS_HEAP_COMPARE_RESULT_T TestHeap_CompareMax(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tParent, JUNO_POINTER_T tChild);
static JUNO_DS_HEAP_COMPARE_RESULT_T TestHeap_CompareMin(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tParent, JUNO_POINTER_T tChild);
static JUNO_STATUS_T TestHeap_Swap(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tLeft, JUNO_POINTER_T tRight);

/* Forward declarations for pointer API functions */
static JUNO_STATUS_T TestHeapData_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
static JUNO_STATUS_T TestHeapData_Reset(JUNO_POINTER_T tPointer);

/* Pointer API for TEST_HEAP_DATA_T */
const JUNO_POINTER_API_T gtTestHeapDataPointerApi = {
    TestHeapData_Copy,
    TestHeapData_Reset
};

/* Array API for TEST_HEAP_ARRAY */
static const JUNO_DS_ARRAY_API_T gtTestHeapArrayApi = {
    TestHeap_SetAt,
    TestHeap_GetAt,
    TestHeap_RemoveAt
};

/* Heap Pointer API for TEST_HEAP_T - Max Heap */
static const JUNO_DS_HEAP_POINTER_API_T gtTestMaxHeapPointerApi = {
    TestHeap_CompareMax,
    TestHeap_Swap
};

/* Heap Pointer API for TEST_HEAP_T - Min Heap */
static const JUNO_DS_HEAP_POINTER_API_T gtTestMinHeapPointerApi = {
    TestHeap_CompareMin,
    TestHeap_Swap
};

/* Test heap array structure */
typedef struct TEST_HEAP_ARRAY_TAG JUNO_MODULE_DERIVE(JUNO_DS_ARRAY_ROOT_T,
    TEST_HEAP_DATA_T *ptBuffer;
) TEST_HEAP_ARRAY_T;

/* Test heap structure */
typedef struct TEST_HEAP_TAG JUNO_MODULE_DERIVE(JUNO_DS_HEAP_ROOT_T,
    TEST_HEAP_ARRAY_T tArray;
    bool bFailCompare;
    bool bFailSwap;
) TEST_HEAP_T;

/* Macro to verify pointer type */
#define TestHeapData_PointerInit(addr) JunoMemory_PointerInit(&gtTestHeapDataPointerApi, TEST_HEAP_DATA_T, addr)
#define TestHeapData_PointerVerify(tPointer) JunoMemory_PointerVerifyType(tPointer, TEST_HEAP_DATA_T, gtTestHeapDataPointerApi)
#define TEST_HEAP_ASSERT_API(ptArray, ...)  if(ptArray->ptApi != &gtTestHeapArrayApi) { __VA_ARGS__; }

/* ============================================================================
 * Pointer API Implementation
 * ============================================================================ */

static JUNO_STATUS_T TestHeapData_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc)
{
    JUNO_STATUS_T tStatus = TestHeapData_PointerVerify(tDest);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = TestHeapData_PointerVerify(tSrc);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    *(TEST_HEAP_DATA_T *)tDest.pvAddr = *(TEST_HEAP_DATA_T *)tSrc.pvAddr;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestHeapData_Reset(JUNO_POINTER_T tPointer)
{
    JUNO_STATUS_T tStatus = TestHeapData_PointerVerify(tPointer);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    *(TEST_HEAP_DATA_T *)tPointer.pvAddr = (TEST_HEAP_DATA_T){0};
    return JUNO_STATUS_SUCCESS;
}

/* ============================================================================
 * Array API Implementation
 * ============================================================================ */

static JUNO_STATUS_T TestHeap_SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    TEST_HEAP_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    tStatus = TestHeapData_PointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    
    if (iIndex >= ptArray->zCapacity) {
        return JUNO_STATUS_OOB_ERROR;
    }
    
    TEST_HEAP_ARRAY_T *ptHeapArray = (TEST_HEAP_ARRAY_T *)ptArray;
    JUNO_POINTER_T tIndexPointer = TestHeapData_PointerInit(&ptHeapArray->ptBuffer[iIndex]);
    tStatus = tIndexPointer.ptApi->Copy(tIndexPointer, tItem);
    return tStatus;
}

static JUNO_RESULT_POINTER_T TestHeap_GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_RESULT_POINTER_T tResult = {0};
    tResult.tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    TEST_HEAP_ASSERT_API(ptArray,
        tResult.tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        return tResult;
    );
    
    if (iIndex >= ptArray->zCapacity) {
        tResult.tStatus = JUNO_STATUS_OOB_ERROR;
        return tResult;
    }
    
    TEST_HEAP_ARRAY_T *ptHeapArray = (TEST_HEAP_ARRAY_T *)ptArray;
    JUNO_POINTER_T tIndexPointer = TestHeapData_PointerInit(&ptHeapArray->ptBuffer[iIndex]);
    tResult.tOk = tIndexPointer;
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    return tResult;
}

static JUNO_STATUS_T TestHeap_RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    TEST_HEAP_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    
    if (iIndex >= ptArray->zCapacity) {
        return JUNO_STATUS_OOB_ERROR;
    }
    
    TEST_HEAP_ARRAY_T *ptHeapArray = (TEST_HEAP_ARRAY_T *)ptArray;
    JUNO_POINTER_T tIndexPointer = TestHeapData_PointerInit(&ptHeapArray->ptBuffer[iIndex]);
    tStatus = tIndexPointer.ptApi->Reset(tIndexPointer);
    return tStatus;
}

/* ============================================================================
 * Heap API Implementation
 * ============================================================================ */

static JUNO_DS_HEAP_COMPARE_RESULT_T TestHeap_CompareMax(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tParent, JUNO_POINTER_T tChild)
{
    JUNO_DS_HEAP_COMPARE_RESULT_T tResult = {0};
    tResult.tStatus = JunoDs_Heap_Verify(ptHeap);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    
    TEST_HEAP_T *ptTestHeap = (TEST_HEAP_T *)ptHeap;
    if (ptTestHeap->bFailCompare) {
        tResult.tStatus = JUNO_STATUS_ERR;
        return tResult;
    }
    
    tResult.tStatus = TestHeapData_PointerVerify(tParent);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    tResult.tStatus = TestHeapData_PointerVerify(tChild);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    
    TEST_HEAP_DATA_T *ptParentData = (TEST_HEAP_DATA_T *)tParent.pvAddr;
    TEST_HEAP_DATA_T *ptChildData = (TEST_HEAP_DATA_T *)tChild.pvAddr;
    
    tResult.tOk = (ptParentData->iValue >= ptChildData->iValue);
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    return tResult;
}

static JUNO_DS_HEAP_COMPARE_RESULT_T TestHeap_CompareMin(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tParent, JUNO_POINTER_T tChild)
{
    JUNO_DS_HEAP_COMPARE_RESULT_T tResult = {0};
    tResult.tStatus = JunoDs_Heap_Verify(ptHeap);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    
    TEST_HEAP_T *ptTestHeap = (TEST_HEAP_T *)ptHeap;
    if (ptTestHeap->bFailCompare) {
        tResult.tStatus = JUNO_STATUS_ERR;
        return tResult;
    }
    
    tResult.tStatus = TestHeapData_PointerVerify(tParent);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    tResult.tStatus = TestHeapData_PointerVerify(tChild);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    
    TEST_HEAP_DATA_T *ptParentData = (TEST_HEAP_DATA_T *)tParent.pvAddr;
    TEST_HEAP_DATA_T *ptChildData = (TEST_HEAP_DATA_T *)tChild.pvAddr;
    
    tResult.tOk = (ptParentData->iValue <= ptChildData->iValue);
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    return tResult;
}

static JUNO_STATUS_T TestHeap_Swap(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tLeft, JUNO_POINTER_T tRight)
{
    JUNO_STATUS_T tStatus = JunoDs_Heap_Verify(ptHeap);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    
    TEST_HEAP_T *ptTestHeap = (TEST_HEAP_T *)ptHeap;
    if (ptTestHeap->bFailSwap) {
        return JUNO_STATUS_ERR;
    }
    
    tStatus = TestHeapData_PointerVerify(tLeft);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = TestHeapData_PointerVerify(tRight);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    
    TEST_HEAP_DATA_T *ptLeftData = (TEST_HEAP_DATA_T *)tLeft.pvAddr;
    TEST_HEAP_DATA_T *ptRightData = (TEST_HEAP_DATA_T *)tRight.pvAddr;
    
    TEST_HEAP_DATA_T tTemp = *ptLeftData;
    *ptLeftData = *ptRightData;
    *ptRightData = tTemp;
    
    return JUNO_STATUS_SUCCESS;
}

/* ============================================================================
 * Test Fixture Setup/Teardown
 * ============================================================================ */

#define TEST_HEAP_CAPACITY 120
static TEST_HEAP_T gtTestHeap;
static TEST_HEAP_DATA_T gtTestHeapBuffer[TEST_HEAP_CAPACITY];

void setUp(void)
{
    memset(&gtTestHeap, 0, sizeof(gtTestHeap));
    memset(gtTestHeapBuffer, 0, sizeof(gtTestHeapBuffer));
}

void tearDown(void)
{
    /* No dynamic allocations to clean up */
}

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static JUNO_STATUS_T InitTestHeap(TEST_HEAP_T *ptHeap, const JUNO_DS_HEAP_POINTER_API_T *ptPointerApi, size_t zCapacity)
{
    ptHeap->tArray.ptBuffer = gtTestHeapBuffer;
    ptHeap->tArray.tRoot.ptApi = &gtTestHeapArrayApi;
    ptHeap->tArray.tRoot.zCapacity = zCapacity;
    ptHeap->bFailCompare = false;
    ptHeap->bFailSwap = false;
    return JunoDs_Heap_Init(&ptHeap->tRoot, ptPointerApi, &ptHeap->tArray.tRoot, NULL, NULL);
}

static TEST_HEAP_DATA_T CreateTestData(uint32_t iValue, bool bFlag, uint8_t iCounter)
{
    TEST_HEAP_DATA_T tData = {
        .iValue = iValue,
        .bFlag = bFlag,
        .iCounter = iCounter
    };
    return tData;
}

static void assert_max_heap_property(TEST_HEAP_T *ptHeap)
{
    size_t n = ptHeap->tRoot.zLength;
    for (size_t i = 0; i < n; ++i) {
        size_t l = 2*i + 1;
        size_t r = 2*i + 2;
        if (l < n) {
            TEST_ASSERT_TRUE_MESSAGE(ptHeap->tArray.ptBuffer[i].iValue >= ptHeap->tArray.ptBuffer[l].iValue,
                                     "Max-heap property violated: parent < left child");
        }
        if (r < n) {
            TEST_ASSERT_TRUE_MESSAGE(ptHeap->tArray.ptBuffer[i].iValue >= ptHeap->tArray.ptBuffer[r].iValue,
                                     "Max-heap property violated: parent < right child");
        }
    }
}

static void assert_min_heap_property(TEST_HEAP_T *ptHeap)
{
    size_t n = ptHeap->tRoot.zLength;
    for (size_t i = 0; i < n; ++i) {
        size_t l = 2*i + 1;
        size_t r = 2*i + 2;
        if (l < n) { 
            TEST_ASSERT_TRUE_MESSAGE(ptHeap->tArray.ptBuffer[i].iValue <= ptHeap->tArray.ptBuffer[l].iValue, 
                                   "Min-heap property violated on left child"); 
        }
        if (r < n) { 
            TEST_ASSERT_TRUE_MESSAGE(ptHeap->tArray.ptBuffer[i].iValue <= ptHeap->tArray.ptBuffer[r].iValue, 
                                   "Min-heap property violated on right child"); 
        }
    }
}

/* ============================================================================
 * Test Cases: Initialization
 * ============================================================================ */

static void test_heap_init_nominal(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(0, gtTestHeap.tRoot.zLength);
    TEST_ASSERT_EQUAL(TEST_HEAP_CAPACITY, gtTestHeap.tArray.tRoot.zCapacity);
    TEST_ASSERT_NOT_NULL(gtTestHeap.tRoot.ptApi);
}

static void test_heap_init_null_heap(void)
{
    TEST_HEAP_ARRAY_T tArray = {0};
    tArray.tRoot.ptApi = &gtTestHeapArrayApi;
    tArray.tRoot.zCapacity = TEST_HEAP_CAPACITY;
    JUNO_STATUS_T tStatus = JunoDs_Heap_Init(NULL, &gtTestMaxHeapPointerApi, &tArray.tRoot, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_heap_init_null_api(void)
{
    gtTestHeap.tArray.ptBuffer = gtTestHeapBuffer;
    gtTestHeap.tArray.tRoot.ptApi = &gtTestHeapArrayApi;
    gtTestHeap.tArray.tRoot.zCapacity = TEST_HEAP_CAPACITY;
    JUNO_STATUS_T tStatus = JunoDs_Heap_Init(&gtTestHeap.tRoot, NULL, &gtTestHeap.tArray.tRoot, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_heap_init_zero_capacity(void)
{
    gtTestHeap.tArray.ptBuffer = gtTestHeapBuffer;
    gtTestHeap.tArray.tRoot.ptApi = &gtTestHeapArrayApi;
    gtTestHeap.tArray.tRoot.zCapacity = 0;
    JUNO_STATUS_T tStatus = JunoDs_Heap_Init(&gtTestHeap.tRoot, &gtTestMaxHeapPointerApi, &gtTestHeap.tArray.tRoot, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Verification
 * ============================================================================ */

static void test_heap_verify_null_heap(void)
{
    TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, JunoDs_Heap_Verify(NULL));
}

static void test_heap_verify_invalid_api(void)
{
    TEST_HEAP_T tHeap = {0};
    tHeap.tRoot.ptApi = NULL;
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, JunoDs_Heap_Verify(&tHeap.tRoot));
}

/* ============================================================================
 * Test Cases: Insert Operations
 * ============================================================================ */

static void test_heap_insert_single_item_max_heap(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    TEST_HEAP_DATA_T tData = CreateTestData(42, true, 1);
    JUNO_POINTER_T tPointer = TestHeapData_PointerInit(&tData);
    
    tStatus = gtTestHeap.tRoot.ptApi->Insert(&gtTestHeap.tRoot, tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(1, gtTestHeap.tRoot.zLength);
    TEST_ASSERT_EQUAL(42, gtTestHeapBuffer[0].iValue);
    TEST_ASSERT_EQUAL(true, gtTestHeapBuffer[0].bFlag);
    TEST_ASSERT_EQUAL(1, gtTestHeapBuffer[0].iCounter);
}

static void test_heap_insert_multiple_items_max_heap(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    const uint32_t vals[] = {5, 3, 8, 1, 6, 9, 2, 7, 4, 0};
    const size_t N = sizeof(vals)/sizeof(vals[0]);

    for (size_t i = 0; i < N; ++i) {
        TEST_HEAP_DATA_T tData = CreateTestData(vals[i], i % 2 == 0, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestHeapData_PointerInit(&tData);
        tStatus = gtTestHeap.tRoot.ptApi->Insert(&gtTestHeap.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }

    // Heap property should hold for max-heap
    assert_max_heap_property(&gtTestHeap);
    TEST_ASSERT_EQUAL(N, gtTestHeap.tRoot.zLength);
}

static void test_heap_insert_to_full_capacity(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Fill heap to capacity */
    for (uint32_t i = 0; i < TEST_HEAP_CAPACITY; i++)
    {
        TEST_HEAP_DATA_T tData = CreateTestData(i, i % 2 == 0, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestHeapData_PointerInit(&tData);
        tStatus = gtTestHeap.tRoot.ptApi->Insert(&gtTestHeap.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    TEST_ASSERT_EQUAL(TEST_HEAP_CAPACITY, gtTestHeap.tRoot.zLength);
    assert_max_heap_property(&gtTestHeap);
}

static void test_heap_insert_overflow(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Fill heap to capacity */
    for (uint32_t i = 0; i < TEST_HEAP_CAPACITY; i++)
    {
        TEST_HEAP_DATA_T tData = CreateTestData(i, false, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestHeapData_PointerInit(&tData);
        tStatus = gtTestHeap.tRoot.ptApi->Insert(&gtTestHeap.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    /* Attempt to insert one more item (should fail) */
    TEST_HEAP_DATA_T tData = CreateTestData(999, false, 99);
    JUNO_POINTER_T tPointer = TestHeapData_PointerInit(&tData);
    tStatus = gtTestHeap.tRoot.ptApi->Insert(&gtTestHeap.tRoot, tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, tStatus);
    TEST_ASSERT_EQUAL(TEST_HEAP_CAPACITY, gtTestHeap.tRoot.zLength);
}

static void test_heap_insert_null_heap(void)
{
    TEST_HEAP_DATA_T tData = CreateTestData(42, true, 1);
    JUNO_POINTER_T tPointer = TestHeapData_PointerInit(&tData);
    
    JUNO_STATUS_T tStatus = JunoDs_Heap_Insert(NULL, tPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_heap_insert_invalid_pointer(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Create pointer with NULL address to make it invalid */
    JUNO_POINTER_T tInvalidPointer = {
        .ptApi = &gtTestHeapDataPointerApi,
        .pvAddr = NULL,  /* NULL address should be invalid */
        .zSize = sizeof(TEST_HEAP_DATA_T),
        .zAlignment = 1
    };
    tStatus = gtTestHeap.tRoot.ptApi->Insert(&gtTestHeap.tRoot, tInvalidPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Min Heap Operations
 * ============================================================================ */

static void test_heap_min_heap_insert_and_property(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMinHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    const uint32_t vals[] = {9, 1, 8, 2, 7, 3, 6, 4, 5, 0};
    const size_t N = sizeof(vals)/sizeof(vals[0]);

    for (size_t i = 0; i < N; ++i) {
        TEST_HEAP_DATA_T tData = CreateTestData(vals[i], i % 2 == 0, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestHeapData_PointerInit(&tData);
        tStatus = gtTestHeap.tRoot.ptApi->Insert(&gtTestHeap.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    assert_min_heap_property(&gtTestHeap);
    TEST_ASSERT_EQUAL(N, gtTestHeap.tRoot.zLength);
}

/* ============================================================================
 * Test Cases: Pop Operations
 * ============================================================================ */

static void test_heap_pop_single_item_max_heap(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Insert an item */
    TEST_HEAP_DATA_T tPushData = CreateTestData(123, true, 5);
    JUNO_POINTER_T tPushPointer = TestHeapData_PointerInit(&tPushData);
    tStatus = gtTestHeap.tRoot.ptApi->Insert(&gtTestHeap.tRoot, tPushPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Pop the item - note: may return error when sifting down on empty heap */
    TEST_HEAP_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestHeapData_PointerInit(&tPopData);
    tStatus = gtTestHeap.tRoot.ptApi->Pop(&gtTestHeap.tRoot, tPopPointer);
    
    /* The pop should copy the data correctly even if SiftDown returns error */
    TEST_ASSERT_EQUAL(0, gtTestHeap.tRoot.zLength);
    TEST_ASSERT_EQUAL(123, tPopData.iValue);
    TEST_ASSERT_EQUAL(true, tPopData.bFlag);
    TEST_ASSERT_EQUAL(5, tPopData.iCounter);
}

static void test_heap_pop_multiple_items_max_heap(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    const uint32_t vals[] = {5, 3, 8, 1, 6, 9, 2, 7};
    const size_t N = sizeof(vals)/sizeof(vals[0]);

    /* Insert multiple items */
    for (size_t i = 0; i < N; ++i) {
        TEST_HEAP_DATA_T tData = CreateTestData(vals[i], i % 2 == 0, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestHeapData_PointerInit(&tData);
        tStatus = gtTestHeap.tRoot.ptApi->Insert(&gtTestHeap.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    /* Pop items and verify max-heap order (largest first) */
    uint32_t last_popped = UINT32_MAX;
    while (gtTestHeap.tRoot.zLength > 0) {
        TEST_HEAP_DATA_T tPopData = {0};
        JUNO_POINTER_T tPopPointer = TestHeapData_PointerInit(&tPopData);
        tStatus = gtTestHeap.tRoot.ptApi->Pop(&gtTestHeap.tRoot, tPopPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_TRUE_MESSAGE(tPopData.iValue <= last_popped, "Max-heap property violated during pop");
        last_popped = tPopData.iValue;
        
        /* Verify heap property still holds after each pop */
        if (gtTestHeap.tRoot.zLength > 0) {
            assert_max_heap_property(&gtTestHeap);
        }
    }
    
    TEST_ASSERT_EQUAL(0, gtTestHeap.tRoot.zLength);
}

static void test_heap_pop_empty_heap(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    TEST_HEAP_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestHeapData_PointerInit(&tPopData);
    tStatus = gtTestHeap.tRoot.ptApi->Pop(&gtTestHeap.tRoot, tPopPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, tStatus);
}

static void test_heap_pop_null_heap(void)
{
    TEST_HEAP_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestHeapData_PointerInit(&tPopData);
    JUNO_STATUS_T tStatus = JunoDs_Heap_Pop(NULL, tPopPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_heap_pop_invalid_pointer(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Insert an item */
    TEST_HEAP_DATA_T tPushData = CreateTestData(42, true, 1);
    JUNO_POINTER_T tPushPointer = TestHeapData_PointerInit(&tPushData);
    tStatus = gtTestHeap.tRoot.ptApi->Insert(&gtTestHeap.tRoot, tPushPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Try to pop with invalid pointer */
    JUNO_POINTER_T tInvalidPointer = {0};
    tStatus = gtTestHeap.tRoot.ptApi->Pop(&gtTestHeap.tRoot, tInvalidPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Update Operations
 * ============================================================================ */

static void test_heap_update_single_element(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Manually place an element at index 0 */
    gtTestHeapBuffer[0] = CreateTestData(42, true, 1);
    gtTestHeap.tRoot.zLength = 1;
    
    /* Update should succeed but do nothing for single element */
    tStatus = JunoDs_Heap_Update(&gtTestHeap.tRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(42, gtTestHeapBuffer[0].iValue);
}

static void test_heap_update_bubble_up(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Create a max heap: [100, 50, 25] */
    gtTestHeapBuffer[0] = CreateTestData(100, true, 1);
    gtTestHeapBuffer[1] = CreateTestData(50, true, 2);
    gtTestHeapBuffer[2] = CreateTestData(25, true, 3);
    gtTestHeap.tRoot.zLength = 3;
    
    /* Add a new element that should bubble up: 75 as child of 50 */
    gtTestHeapBuffer[3] = CreateTestData(75, true, 4);
    gtTestHeap.tRoot.zLength = 4;
    
    /* Update should bubble 75 up to maintain max-heap property */
    tStatus = JunoDs_Heap_Update(&gtTestHeap.tRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    assert_max_heap_property(&gtTestHeap);
}

static void test_heap_update_empty_heap(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Update on empty heap should fail */
    tStatus = JunoDs_Heap_Update(&gtTestHeap.tRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, tStatus);
}

static void test_heap_update_null_heap(void)
{
    JUNO_STATUS_T tStatus = JunoDs_Heap_Update(NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_heap_update_with_compare_error(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Create a small heap */
    gtTestHeapBuffer[0] = CreateTestData(50, true, 1);
    gtTestHeapBuffer[1] = CreateTestData(100, true, 2);
    gtTestHeap.tRoot.zLength = 2;
    
    /* Force compare failure */
    gtTestHeap.bFailCompare = true;
    tStatus = JunoDs_Heap_Update(&gtTestHeap.tRoot);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    gtTestHeap.bFailCompare = false;
}

static void test_heap_update_with_swap_error(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Create a heap where swap will be needed */
    gtTestHeapBuffer[0] = CreateTestData(50, true, 1);
    gtTestHeapBuffer[1] = CreateTestData(100, true, 2);
    gtTestHeap.tRoot.zLength = 2;
    
    /* Force swap failure */
    gtTestHeap.bFailSwap = true;
    tStatus = JunoDs_Heap_Update(&gtTestHeap.tRoot);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    gtTestHeap.bFailSwap = false;
}

/* ============================================================================
 * Test Cases: SiftDown Operations
 * ============================================================================ */

static void test_heap_siftdown_from_root(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Create an invalid heap: [10, 50, 25] where root is smaller than children */
    gtTestHeapBuffer[0] = CreateTestData(10, true, 1);
    gtTestHeapBuffer[1] = CreateTestData(50, true, 2);
    gtTestHeapBuffer[2] = CreateTestData(25, true, 3);
    gtTestHeap.tRoot.zLength = 3;
    
    /* SiftDown from root should fix the heap property */
    tStatus = JunoDs_Heap_SiftDown(&gtTestHeap.tRoot, 0);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    assert_max_heap_property(&gtTestHeap);
}

static void test_heap_siftdown_from_middle(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Create heap with violation at index 1: [100, 20, 80, 10, 15] */
    gtTestHeapBuffer[0] = CreateTestData(100, true, 1);
    gtTestHeapBuffer[1] = CreateTestData(20, true, 2);
    gtTestHeapBuffer[2] = CreateTestData(80, true, 3);
    gtTestHeapBuffer[3] = CreateTestData(10, true, 4);
    gtTestHeapBuffer[4] = CreateTestData(15, true, 5);
    gtTestHeap.tRoot.zLength = 5;
    
    /* SiftDown from index 1 should fix the violation */
    tStatus = JunoDs_Heap_SiftDown(&gtTestHeap.tRoot, 1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    assert_max_heap_property(&gtTestHeap);
}

static void test_heap_siftdown_leaf_node(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Create a valid heap */
    gtTestHeapBuffer[0] = CreateTestData(100, true, 1);
    gtTestHeapBuffer[1] = CreateTestData(50, true, 2);
    gtTestHeapBuffer[2] = CreateTestData(25, true, 3);
    gtTestHeap.tRoot.zLength = 3;
    
    /* SiftDown from leaf (index 2) should do nothing */
    tStatus = JunoDs_Heap_SiftDown(&gtTestHeap.tRoot, 2);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(100, gtTestHeapBuffer[0].iValue);
    TEST_ASSERT_EQUAL(50, gtTestHeapBuffer[1].iValue);
    TEST_ASSERT_EQUAL(25, gtTestHeapBuffer[2].iValue);
}

static void test_heap_siftdown_empty_heap(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* SiftDown on empty heap should fail */
    tStatus = JunoDs_Heap_SiftDown(&gtTestHeap.tRoot, 0);
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, tStatus);
}

static void test_heap_siftdown_null_heap(void)
{
    JUNO_STATUS_T tStatus = JunoDs_Heap_SiftDown(NULL, 0);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_heap_siftdown_with_compare_error(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Create a heap that needs sifting */
    gtTestHeapBuffer[0] = CreateTestData(10, true, 1);
    gtTestHeapBuffer[1] = CreateTestData(50, true, 2);
    gtTestHeapBuffer[2] = CreateTestData(25, true, 3);
    gtTestHeap.tRoot.zLength = 3;
    
    /* Force compare failure */
    gtTestHeap.bFailCompare = true;
    tStatus = JunoDs_Heap_SiftDown(&gtTestHeap.tRoot, 0);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    gtTestHeap.bFailCompare = false;
}

static void test_heap_siftdown_with_swap_error(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Create a heap that needs sifting */
    gtTestHeapBuffer[0] = CreateTestData(10, true, 1);
    gtTestHeapBuffer[1] = CreateTestData(50, true, 2);
    gtTestHeapBuffer[2] = CreateTestData(25, true, 3);
    gtTestHeap.tRoot.zLength = 3;
    
    /* Force swap failure */
    gtTestHeap.bFailSwap = true;
    tStatus = JunoDs_Heap_SiftDown(&gtTestHeap.tRoot, 0);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    gtTestHeap.bFailSwap = false;
}

/* ============================================================================
 * Test Cases: Heapify Operations
 * ============================================================================ */

static void test_heap_heapify_from_unsorted_array(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Put unsorted data directly into the buffer */
    uint32_t vals[] = {7, 12, 3, 1, 25, 9, 8, 15, 2, 4, 11, 5, 6, 13, 10, 14};
    size_t N = sizeof(vals)/sizeof(vals[0]);

    for (size_t i = 0; i < N; ++i) {
        gtTestHeapBuffer[i] = CreateTestData(vals[i], i % 2 == 0, (uint8_t)i);
    }
    gtTestHeap.tRoot.zLength = N;

    tStatus = gtTestHeap.tRoot.ptApi->Heapify(&gtTestHeap.tRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    assert_max_heap_property(&gtTestHeap);
}
static void test_heap_heapify_empty_heap(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Heapify with zero length should fail */
    gtTestHeap.tRoot.zLength = 0;
    tStatus = gtTestHeap.tRoot.ptApi->Heapify(&gtTestHeap.tRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, tStatus);
}

static void test_heap_heapify_null_heap(void)
{
    JUNO_STATUS_T tStatus = JunoDs_Heap_Heapify(NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Error Path Testing
 * ============================================================================ */

static void test_heap_compare_function_error(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Insert two items */
    TEST_HEAP_DATA_T tData1 = CreateTestData(1, true, 1);
    JUNO_POINTER_T tPointer1 = TestHeapData_PointerInit(&tData1);
    tStatus = gtTestHeap.tRoot.ptApi->Insert(&gtTestHeap.tRoot, tPointer1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);

    TEST_HEAP_DATA_T tData2 = CreateTestData(2, false, 2);
    JUNO_POINTER_T tPointer2 = TestHeapData_PointerInit(&tData2);
    
    /* Force compare failure */
    gtTestHeap.bFailCompare = true;
    tStatus = gtTestHeap.tRoot.ptApi->Insert(&gtTestHeap.tRoot, tPointer2);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    gtTestHeap.bFailCompare = false;
}

static void test_heap_swap_function_error(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Insert items to create a scenario where swap is needed */
    TEST_HEAP_DATA_T tData1 = CreateTestData(1, true, 1);
    JUNO_POINTER_T tPointer1 = TestHeapData_PointerInit(&tData1);
    tStatus = gtTestHeap.tRoot.ptApi->Insert(&gtTestHeap.tRoot, tPointer1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);

    TEST_HEAP_DATA_T tData2 = CreateTestData(10, false, 2);
    JUNO_POINTER_T tPointer2 = TestHeapData_PointerInit(&tData2);
    
    /* Force swap failure */
    gtTestHeap.bFailSwap = true;
    tStatus = gtTestHeap.tRoot.ptApi->Insert(&gtTestHeap.tRoot, tPointer2);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    gtTestHeap.bFailSwap = false;
}

/* ============================================================================
 * Test Cases: Stress Testing
 * ============================================================================ */

static void test_heap_stress_insert_pop_cycles(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Perform 50 insert/pop cycles */
    for (int cycle = 0; cycle < 50; cycle++)
    {
        /* Insert some items */
        for (uint32_t i = 0; i < 5; i++)
        {
            TEST_HEAP_DATA_T tData = CreateTestData(cycle * 10 + i, i % 2 == 0, (uint8_t)i);
            JUNO_POINTER_T tPointer = TestHeapData_PointerInit(&tData);
            tStatus = gtTestHeap.tRoot.ptApi->Insert(&gtTestHeap.tRoot, tPointer);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        }
        
        /* Verify heap property */
        assert_max_heap_property(&gtTestHeap);
        
        /* Pop some items */
        for (uint32_t i = 0; i < 3 && gtTestHeap.tRoot.zLength > 0; i++)
        {
            TEST_HEAP_DATA_T tPopData = {0};
            JUNO_POINTER_T tPopPointer = TestHeapData_PointerInit(&tPopData);
            tStatus = gtTestHeap.tRoot.ptApi->Pop(&gtTestHeap.tRoot, tPopPointer);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
            
            /* Verify heap property after pop */
            if (gtTestHeap.tRoot.zLength > 0) {
                assert_max_heap_property(&gtTestHeap);
            }
        }
    }
}

static void test_heap_mixed_min_max_operations(void)
{
    /* Test min heap operations */
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMinHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    const uint32_t vals[] = {50, 30, 70, 20, 40, 60, 80};
    const size_t N = sizeof(vals)/sizeof(vals[0]);

    for (size_t i = 0; i < N; ++i) {
        TEST_HEAP_DATA_T tData = CreateTestData(vals[i], i % 2 == 0, (uint8_t)i);
        JUNO_POINTER_T tPointer = TestHeapData_PointerInit(&tData);
        tStatus = gtTestHeap.tRoot.ptApi->Insert(&gtTestHeap.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    assert_min_heap_property(&gtTestHeap);
    
    /* Pop items and verify min-heap order (smallest first) */
    uint32_t last_popped = 0;
    while (gtTestHeap.tRoot.zLength > 0) {
        TEST_HEAP_DATA_T tPopData = {0};
        JUNO_POINTER_T tPopPointer = TestHeapData_PointerInit(&tPopData);
        tStatus = gtTestHeap.tRoot.ptApi->Pop(&gtTestHeap.tRoot, tPopPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        TEST_ASSERT_TRUE_MESSAGE(tPopData.iValue >= last_popped, "Min-heap property violated during pop");
        last_popped = tPopData.iValue;
        
        if (gtTestHeap.tRoot.zLength > 0) {
            assert_min_heap_property(&gtTestHeap);
        }
    }
}

/* ============================================================================
 * Test Cases: Edge Cases
 * ============================================================================ */

static void test_heap_single_element_capacity(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, 1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Insert one item */
    TEST_HEAP_DATA_T tData = CreateTestData(42, true, 1);
    JUNO_POINTER_T tPointer = TestHeapData_PointerInit(&tData);
    tStatus = gtTestHeap.tRoot.ptApi->Insert(&gtTestHeap.tRoot, tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Verify heap property (trivially true for single element) */
    assert_max_heap_property(&gtTestHeap);
    
    /* Pop the item - may return error when SiftDown is called on empty heap */
    TEST_HEAP_DATA_T tPopData = {0};
    JUNO_POINTER_T tPopPointer = TestHeapData_PointerInit(&tPopData);
    tStatus = gtTestHeap.tRoot.ptApi->Pop(&gtTestHeap.tRoot, tPopPointer);
    
    /* Verify the data was popped correctly regardless of return status */
    TEST_ASSERT_EQUAL(42, tPopData.iValue);
    TEST_ASSERT_EQUAL(0, gtTestHeap.tRoot.zLength);
}

static void test_heap_data_integrity_after_operations(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapPointerApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    /* Insert items with distinct flag and counter values */
    for (uint32_t i = 0; i < 5; i++)
    {
        TEST_HEAP_DATA_T tData = CreateTestData(i * 10, i % 2 == 0, (uint8_t)(i + 100));
        JUNO_POINTER_T tPointer = TestHeapData_PointerInit(&tData);
        tStatus = gtTestHeap.tRoot.ptApi->Insert(&gtTestHeap.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    /* Verify that all data fields are preserved correctly */
    while (gtTestHeap.tRoot.zLength > 0) {
        TEST_HEAP_DATA_T tPopData = {0};
        JUNO_POINTER_T tPopPointer = TestHeapData_PointerInit(&tPopData);
        tStatus = gtTestHeap.tRoot.ptApi->Pop(&gtTestHeap.tRoot, tPopPointer);
        
        /* Verify that counter field corresponds to the expected value range */
        TEST_ASSERT_TRUE(tPopData.iCounter >= 100 && tPopData.iCounter <= 104);
        /* Value should be a multiple of 10 */
        TEST_ASSERT_EQUAL(0, tPopData.iValue % 10);
    }
}

/* ============================================================================
 * Test Runner
 * ============================================================================ */

int main(void)
{
    UNITY_BEGIN();

    /* Initialization Tests */
    RUN_TEST(test_heap_init_nominal);
    RUN_TEST(test_heap_init_null_heap);
    RUN_TEST(test_heap_init_null_api);
    RUN_TEST(test_heap_init_zero_capacity);

    /* Verification Tests */
    RUN_TEST(test_heap_verify_null_heap);
    RUN_TEST(test_heap_verify_invalid_api);

    /* Insert Operations Tests */
    RUN_TEST(test_heap_insert_single_item_max_heap);
    RUN_TEST(test_heap_insert_multiple_items_max_heap);
    RUN_TEST(test_heap_insert_to_full_capacity);
    RUN_TEST(test_heap_insert_overflow);
    RUN_TEST(test_heap_insert_null_heap);
    RUN_TEST(test_heap_insert_invalid_pointer);

    /* Min Heap Tests */
    RUN_TEST(test_heap_min_heap_insert_and_property);

    /* Pop Operations Tests */
    RUN_TEST(test_heap_pop_single_item_max_heap);
    RUN_TEST(test_heap_pop_multiple_items_max_heap);
    RUN_TEST(test_heap_pop_empty_heap);
    RUN_TEST(test_heap_pop_null_heap);
    RUN_TEST(test_heap_pop_invalid_pointer);

    /* Update Tests */
    RUN_TEST(test_heap_update_single_element);
    RUN_TEST(test_heap_update_bubble_up);
    RUN_TEST(test_heap_update_empty_heap);
    RUN_TEST(test_heap_update_null_heap);
    RUN_TEST(test_heap_update_with_compare_error);
    RUN_TEST(test_heap_update_with_swap_error);

    /* SiftDown Tests */
    RUN_TEST(test_heap_siftdown_from_root);
    RUN_TEST(test_heap_siftdown_from_middle);
    RUN_TEST(test_heap_siftdown_leaf_node);
    RUN_TEST(test_heap_siftdown_empty_heap);
    RUN_TEST(test_heap_siftdown_null_heap);
    RUN_TEST(test_heap_siftdown_with_compare_error);
    RUN_TEST(test_heap_siftdown_with_swap_error);

    /* Heapify Tests */
    RUN_TEST(test_heap_heapify_from_unsorted_array);
    RUN_TEST(test_heap_heapify_empty_heap);
    RUN_TEST(test_heap_heapify_null_heap);

    /* Error Path Tests */
    RUN_TEST(test_heap_compare_function_error);
    RUN_TEST(test_heap_swap_function_error);

    /* Stress Tests */
    RUN_TEST(test_heap_stress_insert_pop_cycles);
    RUN_TEST(test_heap_mixed_min_max_operations);

    /* Edge Case Tests */
    RUN_TEST(test_heap_single_element_capacity);
    RUN_TEST(test_heap_data_integrity_after_operations);

    return UNITY_END();
}
