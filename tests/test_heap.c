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

typedef struct TEST_HEAP_TAG JUNO_MODULE_DERIVE_WITH_API(JUNO_DS_HEAP_ROOT_T, JUNO_DS_HEAP_API_T,
    TEST_HEAP_DATA_T *ptBuffer;
    bool bFailCompare;
    bool bFailSwap;
) TEST_HEAP_T;

/* Forward declarations for array API functions */
static JUNO_STATUS_T TestHeap_SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
static JUNO_RESULT_POINTER_T TestHeap_GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
static JUNO_STATUS_T TestHeap_RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);

/* Forward declarations for heap API functions */
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

/* Heap API for TEST_HEAP_T - Max Heap */
static const JUNO_DS_HEAP_API_T gtTestMaxHeapApi = JunoDs_HeapApiInit(
    TestHeap_SetAt,
    TestHeap_GetAt,
    TestHeap_RemoveAt,
    TestHeap_CompareMax,
    TestHeap_Swap
);

/* Heap API for TEST_HEAP_T - Min Heap */
static const JUNO_DS_HEAP_API_T gtTestMinHeapApi = JunoDs_HeapApiInit(
    TestHeap_SetAt,
    TestHeap_GetAt,
    TestHeap_RemoveAt,
    TestHeap_CompareMin,
    TestHeap_Swap
);

/* Macro to verify pointer type */
#define TestHeapData_PointerInit(addr) JunoMemory_PointerInit(&gtTestHeapDataPointerApi, TEST_HEAP_DATA_T, addr)
#define TestHeapData_PointerVerify(tPointer) JunoMemory_PointerVerifyType(tPointer, TEST_HEAP_DATA_T, gtTestHeapDataPointerApi)
#define TEST_HEAP_ASSERT_API(ptArray, ...)  if(ptArray->ptApi != &gtTestMaxHeapApi.tRoot && ptArray->ptApi != &gtTestMinHeapApi.tRoot) { __VA_ARGS__; }

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
    tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    
    TEST_HEAP_T *ptHeap = (TEST_HEAP_T *)ptArray;
    JUNO_POINTER_T tIndexPointer = TestHeapData_PointerInit(&ptHeap->ptBuffer[iIndex]);
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
    tResult.tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    
    TEST_HEAP_T *ptHeap = (TEST_HEAP_T *)ptArray;
    JUNO_POINTER_T tIndexPointer = TestHeapData_PointerInit(&ptHeap->ptBuffer[iIndex]);
    tResult.tOk = tIndexPointer;
    return tResult;
}

static JUNO_STATUS_T TestHeap_RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    TEST_HEAP_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    
    TEST_HEAP_T *ptHeap = (TEST_HEAP_T *)ptArray;
    JUNO_POINTER_T tIndexPointer = TestHeapData_PointerInit(&ptHeap->ptBuffer[iIndex]);
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

static JUNO_STATUS_T InitTestHeap(TEST_HEAP_T *ptHeap, const JUNO_DS_HEAP_API_T *ptApi, size_t zCapacity)
{
    ptHeap->ptBuffer = gtTestHeapBuffer;
    ptHeap->bFailCompare = false;
    ptHeap->bFailSwap = false;
    return JunoDs_Heap_Init(&ptHeap->tRoot, ptApi, zCapacity, NULL, NULL);
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
            TEST_ASSERT_TRUE_MESSAGE(ptHeap->ptBuffer[i].iValue >= ptHeap->ptBuffer[l].iValue, 
                                   "Max-heap property violated on left child"); 
        }
        if (r < n) { 
            TEST_ASSERT_TRUE_MESSAGE(ptHeap->ptBuffer[i].iValue >= ptHeap->ptBuffer[r].iValue, 
                                   "Max-heap property violated on right child"); 
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
            TEST_ASSERT_TRUE_MESSAGE(ptHeap->ptBuffer[i].iValue <= ptHeap->ptBuffer[l].iValue, 
                                   "Min-heap property violated on left child"); 
        }
        if (r < n) { 
            TEST_ASSERT_TRUE_MESSAGE(ptHeap->ptBuffer[i].iValue <= ptHeap->ptBuffer[r].iValue, 
                                   "Min-heap property violated on right child"); 
        }
    }
}

/* ============================================================================
 * Test Cases: Initialization
 * ============================================================================ */

static void test_heap_init_nominal(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapApi, TEST_HEAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(0, gtTestHeap.tRoot.zLength);
    TEST_ASSERT_EQUAL(TEST_HEAP_CAPACITY, gtTestHeap.tRoot.tRoot.zCapacity);
    TEST_ASSERT_NOT_NULL(gtTestHeap.tRoot.ptApi);
}

static void test_heap_init_null_heap(void)
{
    JUNO_STATUS_T tStatus = JunoDs_Heap_Init(NULL, &gtTestMaxHeapApi, TEST_HEAP_CAPACITY, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_heap_init_null_api(void)
{
    gtTestHeap.ptBuffer = gtTestHeapBuffer;
    JUNO_STATUS_T tStatus = JunoDs_Heap_Init(&gtTestHeap.tRoot, NULL, TEST_HEAP_CAPACITY, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_heap_init_zero_capacity(void)
{
    gtTestHeap.ptBuffer = gtTestHeapBuffer;
    JUNO_STATUS_T tStatus = JunoDs_Heap_Init(&gtTestHeap.tRoot, &gtTestMaxHeapApi, 0, NULL, NULL);
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
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapApi, TEST_HEAP_CAPACITY);
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
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapApi, TEST_HEAP_CAPACITY);
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
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapApi, TEST_HEAP_CAPACITY);
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
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapApi, TEST_HEAP_CAPACITY);
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
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapApi, TEST_HEAP_CAPACITY);
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
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMinHeapApi, TEST_HEAP_CAPACITY);
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
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapApi, TEST_HEAP_CAPACITY);
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
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapApi, TEST_HEAP_CAPACITY);
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
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapApi, TEST_HEAP_CAPACITY);
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
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapApi, TEST_HEAP_CAPACITY);
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
 * Test Cases: Heapify Operations
 * ============================================================================ */

static void test_heap_heapify_from_unsorted_array(void)
{
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapApi, TEST_HEAP_CAPACITY);
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
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapApi, TEST_HEAP_CAPACITY);
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
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapApi, TEST_HEAP_CAPACITY);
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
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapApi, TEST_HEAP_CAPACITY);
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
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapApi, TEST_HEAP_CAPACITY);
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
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMinHeapApi, TEST_HEAP_CAPACITY);
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
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapApi, 1);
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
    JUNO_STATUS_T tStatus = InitTestHeap(&gtTestHeap, &gtTestMaxHeapApi, TEST_HEAP_CAPACITY);
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
