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
 * @file test_array.c
 * @brief Unit tests for the LibJuno Array API.
 */

#include "juno/ds/array_api.h"
#include "juno/memory/pointer_api.h"
#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* ============================================================================
 * Test Type + Array/Pointer API Implementations
 * ============================================================================ */

typedef struct TEST_ARR_ITEM_TAG
{
    uint32_t iValue;
} TEST_ARR_ITEM_T;

/* Forward declarations */
static JUNO_STATUS_T TestArr_SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
static JUNO_RESULT_POINTER_T TestArr_GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
static JUNO_STATUS_T TestArr_RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
static JUNO_STATUS_T TestArrItem_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
static JUNO_STATUS_T TestArrItem_Reset(JUNO_POINTER_T tPointer);

/* Pointer API */
static const JUNO_POINTER_API_T gtTestItemPtrApi = {
    TestArrItem_Copy,
    TestArrItem_Reset
};

#define TestItem_PtrInit(addr) JunoMemory_PointerInit(&gtTestItemPtrApi, TEST_ARR_ITEM_T, addr)
#define TestItem_PtrVerify(ptr) JunoMemory_PointerVerifyType(ptr, TEST_ARR_ITEM_T, gtTestItemPtrApi)

/* Array API */
static const JUNO_DS_ARRAY_API_T gtTestArrayApi = {
    TestArr_SetAt,
    TestArr_GetAt,
    TestArr_RemoveAt
};

/* ============================================================================
 * Concrete Array Type
 * ============================================================================ */

#define TEST_ARR_CAPACITY 4

typedef struct TEST_ARRAY_TAG
{
    JUNO_DS_ARRAY_ROOT_T tRoot;
    TEST_ARR_ITEM_T atBuffer[TEST_ARR_CAPACITY];
} TEST_ARRAY_T;

/* ============================================================================
 * Pointer API Implementation
 * ============================================================================ */

static JUNO_STATUS_T TestArrItem_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc)
{
    JUNO_STATUS_T tStatus = TestItem_PtrVerify(tDest);
    if(tStatus != JUNO_STATUS_SUCCESS) { return tStatus; }
    tStatus = TestItem_PtrVerify(tSrc);
    if(tStatus != JUNO_STATUS_SUCCESS) { return tStatus; }
    *(TEST_ARR_ITEM_T *)tDest.pvAddr = *(const TEST_ARR_ITEM_T *)tSrc.pvAddr;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestArrItem_Reset(JUNO_POINTER_T tPointer)
{
    JUNO_STATUS_T tStatus = TestItem_PtrVerify(tPointer);
    if(tStatus != JUNO_STATUS_SUCCESS) { return tStatus; }
    *(TEST_ARR_ITEM_T *)tPointer.pvAddr = (TEST_ARR_ITEM_T){0};
    return JUNO_STATUS_SUCCESS;
}

/* ============================================================================
 * Array API Implementation
 * ============================================================================ */

static JUNO_STATUS_T TestArr_SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    if(tStatus != JUNO_STATUS_SUCCESS) { return tStatus; }
    TEST_ARRAY_T *ptArr = (TEST_ARRAY_T *)ptArray;
    JUNO_POINTER_T tDest = TestItem_PtrInit(&ptArr->atBuffer[iIndex]);
    return TestArrItem_Copy(tDest, tItem);
}

static JUNO_RESULT_POINTER_T TestArr_GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_RESULT_POINTER_T tResult = {JUNO_STATUS_ERR, {0}};
    tResult.tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    if(tResult.tStatus != JUNO_STATUS_SUCCESS) { return tResult; }
    TEST_ARRAY_T *ptArr = (TEST_ARRAY_T *)ptArray;
    tResult.tOk = TestItem_PtrInit(&ptArr->atBuffer[iIndex]);
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    return tResult;
}

static JUNO_STATUS_T TestArr_RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    if(tStatus != JUNO_STATUS_SUCCESS) { return tStatus; }
    TEST_ARRAY_T *ptArr = (TEST_ARRAY_T *)ptArray;
    JUNO_POINTER_T tSlot = TestItem_PtrInit(&ptArr->atBuffer[iIndex]);
    return TestArrItem_Reset(tSlot);
}

/* ============================================================================
 * Fixtures
 * ============================================================================ */

static TEST_ARRAY_T gtArray;

void setUp(void)
{
    memset(&gtArray, 0, sizeof(gtArray));
    JunoDs_ArrayInit(&gtArray.tRoot, &gtTestArrayApi, TEST_ARR_CAPACITY, NULL, NULL);
}

void tearDown(void) {}

/* ============================================================================
 * Test Cases: Array Initialization (REQ-ARRAY-001)
 * ============================================================================ */

// @{"verify": ["REQ-ARRAY-001"]}
static void test_array_init_success(void)
{
    TEST_ARRAY_T tArr;
    memset(&tArr, 0, sizeof(tArr));
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(&tArr.tRoot, &gtTestArrayApi,
                                              TEST_ARR_CAPACITY, NULL, NULL);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL_PTR(&gtTestArrayApi, tArr.tRoot.ptApi);
    TEST_ASSERT_EQUAL(TEST_ARR_CAPACITY, tArr.tRoot.zCapacity);
}

// @{"verify": ["REQ-ARRAY-001"]}
static void test_array_init_null_array(void)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(NULL, &gtTestArrayApi,
                                              TEST_ARR_CAPACITY, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: SetAt / GetAt / RemoveAt (REQ-ARRAY-002, REQ-ARRAY-003, REQ-ARRAY-004)
 * ============================================================================ */

// @{"verify": ["REQ-ARRAY-002"]}
static void test_array_set_at(void)
{
    TEST_ARR_ITEM_T tItem = {.iValue = 42};
    JUNO_POINTER_T tPtr = TestItem_PtrInit(&tItem);
    JUNO_STATUS_T tStatus = gtArray.tRoot.ptApi->SetAt(&gtArray.tRoot, tPtr, 0);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(42, gtArray.atBuffer[0].iValue);
}

// @{"verify": ["REQ-ARRAY-003"]}
static void test_array_get_at(void)
{
    gtArray.atBuffer[1].iValue = 99;
    JUNO_RESULT_POINTER_T tResult = gtArray.tRoot.ptApi->GetAt(&gtArray.tRoot, 1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    TEST_ARR_ITEM_T *ptItem = (TEST_ARR_ITEM_T *)tResult.tOk.pvAddr;
    TEST_ASSERT_NOT_NULL(ptItem);
    TEST_ASSERT_EQUAL(99, ptItem->iValue);
}

// @{"verify": ["REQ-ARRAY-004"]}
static void test_array_remove_at(void)
{
    gtArray.atBuffer[2].iValue = 77;
    JUNO_STATUS_T tStatus = gtArray.tRoot.ptApi->RemoveAt(&gtArray.tRoot, 2);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(0, gtArray.atBuffer[2].iValue);
}

/* ============================================================================
 * Test Cases: Bounds Checking (REQ-ARRAY-005)
 * ============================================================================ */

// @{"verify": ["REQ-ARRAY-005"]}
static void test_array_set_at_oob(void)
{
    TEST_ARR_ITEM_T tItem = {.iValue = 1};
    JUNO_POINTER_T tPtr = TestItem_PtrInit(&tItem);
    JUNO_STATUS_T tStatus = gtArray.tRoot.ptApi->SetAt(&gtArray.tRoot, tPtr,
                                                        TEST_ARR_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_OOB_ERROR, tStatus);
}

// @{"verify": ["REQ-ARRAY-005"]}
static void test_array_get_at_oob(void)
{
    JUNO_RESULT_POINTER_T tResult = gtArray.tRoot.ptApi->GetAt(&gtArray.tRoot,
                                                                TEST_ARR_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_OOB_ERROR, tResult.tStatus);
}

// @{"verify": ["REQ-ARRAY-005"]}
static void test_array_remove_at_oob(void)
{
    JUNO_STATUS_T tStatus = gtArray.tRoot.ptApi->RemoveAt(&gtArray.tRoot,
                                                           TEST_ARR_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_OOB_ERROR, tStatus);
}

/* ============================================================================
 * Test Cases: Vtable Validation (REQ-ARRAY-006)
 * ============================================================================ */

// @{"verify": ["REQ-ARRAY-006", "REQ-SYS-010"]}
static void test_array_verify_null_api(void)
{
    JUNO_DS_ARRAY_ROOT_T tBad;
    memset(&tBad, 0, sizeof(tBad));
    tBad.ptApi = NULL;
    tBad.zCapacity = 1;
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(&tBad);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Capacity Validation (REQ-ARRAY-007)
 * ============================================================================ */

// @{"verify": ["REQ-ARRAY-007"]}
static void test_array_verify_zero_capacity(void)
{
    JUNO_DS_ARRAY_ROOT_T tBad;
    memset(&tBad, 0, sizeof(tBad));
    tBad.ptApi = &gtTestArrayApi;
    tBad.zCapacity = 0;
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(&tBad);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-ARRAY-007"]}
static void test_array_init_zero_capacity(void)
{
    TEST_ARRAY_T tArr;
    memset(&tArr, 0, sizeof(tArr));
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(&tArr.tRoot, &gtTestArrayApi,
                                              0, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Main
 * ============================================================================ */

int main(void)
{
    UNITY_BEGIN();
    /* Init */
    RUN_TEST(test_array_init_success);
    RUN_TEST(test_array_init_null_array);
    /* SetAt / GetAt / RemoveAt */
    RUN_TEST(test_array_set_at);
    RUN_TEST(test_array_get_at);
    RUN_TEST(test_array_remove_at);
    /* Bounds Checking */
    RUN_TEST(test_array_set_at_oob);
    RUN_TEST(test_array_get_at_oob);
    RUN_TEST(test_array_remove_at_oob);
    /* Vtable Validation */
    RUN_TEST(test_array_verify_null_api);
    /* Capacity Validation */
    RUN_TEST(test_array_verify_zero_capacity);
    RUN_TEST(test_array_init_zero_capacity);
    return UNITY_END();
}
