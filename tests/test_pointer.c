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
 * @file test_pointer.c
 * @brief Unit tests for the LibJuno Pointer API.
 */

#include "juno/memory/pointer_api.h"
#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* ============================================================================
 * Test Type and Pointer API Implementation
 * ============================================================================ */

typedef struct TEST_PTR_VALUE_TAG
{
    uint32_t iData;
} TEST_PTR_VALUE_T;

static JUNO_STATUS_T TestPtr_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
static JUNO_STATUS_T TestPtr_Reset(JUNO_POINTER_T tPointer);

static const JUNO_POINTER_API_T gtTestPtrApi = {
    TestPtr_Copy,
    TestPtr_Reset
};

#define TestPtr_Init(addr) JunoMemory_PointerInit(&gtTestPtrApi, TEST_PTR_VALUE_T, addr)
#define TestPtr_Verify(ptr) JunoMemory_PointerVerifyType(ptr, TEST_PTR_VALUE_T, gtTestPtrApi)

static JUNO_STATUS_T TestPtr_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc)
{
    JUNO_STATUS_T tStatus = TestPtr_Verify(tDest);
    if(tStatus != JUNO_STATUS_SUCCESS) { return tStatus; }
    tStatus = TestPtr_Verify(tSrc);
    if(tStatus != JUNO_STATUS_SUCCESS) { return tStatus; }
    *(TEST_PTR_VALUE_T *)tDest.pvAddr = *(const TEST_PTR_VALUE_T *)tSrc.pvAddr;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestPtr_Reset(JUNO_POINTER_T tPointer)
{
    JUNO_STATUS_T tStatus = TestPtr_Verify(tPointer);
    if(tStatus != JUNO_STATUS_SUCCESS) { return tStatus; }
    *(TEST_PTR_VALUE_T *)tPointer.pvAddr = (TEST_PTR_VALUE_T){0};
    return JUNO_STATUS_SUCCESS;
}

/* ============================================================================
 * Fixtures
 * ============================================================================ */

static TEST_PTR_VALUE_T gtValA;
static TEST_PTR_VALUE_T gtValB;

void setUp(void)
{
    memset(&gtValA, 0, sizeof(gtValA));
    memset(&gtValB, 0, sizeof(gtValB));
}

void tearDown(void) {}

/* ============================================================================
 * Test Cases: Pointer Copy (REQ-POINTER-002)
 * ============================================================================ */

// @{"verify": ["REQ-POINTER-002"]}
static void test_pointer_copy_success(void)
{
    gtValA.iData = 0xCAFE;
    JUNO_POINTER_T tSrc = TestPtr_Init(&gtValA);
    JUNO_POINTER_T tDest = TestPtr_Init(&gtValB);
    JUNO_STATUS_T tStatus = tSrc.ptApi->Copy(tDest, tSrc);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL_HEX32(0xCAFE, gtValB.iData);
}

// @{"verify": ["REQ-POINTER-002"]}
static void test_pointer_copy_null_src_addr(void)
{
    JUNO_POINTER_T tSrc = TestPtr_Init(NULL);
    JUNO_POINTER_T tDest = TestPtr_Init(&gtValB);
    JUNO_STATUS_T tStatus = gtTestPtrApi.Copy(tDest, tSrc);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Pointer Reset (REQ-POINTER-003)
 * ============================================================================ */

// @{"verify": ["REQ-POINTER-003"]}
static void test_pointer_reset_success(void)
{
    gtValA.iData = 0xDEAD;
    JUNO_POINTER_T tPtr = TestPtr_Init(&gtValA);
    JUNO_STATUS_T tStatus = tPtr.ptApi->Reset(tPtr);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL_HEX32(0, gtValA.iData);
}

// @{"verify": ["REQ-POINTER-003"]}
static void test_pointer_reset_null_addr(void)
{
    JUNO_POINTER_T tPtr = TestPtr_Init(NULL);
    JUNO_STATUS_T tStatus = gtTestPtrApi.Reset(tPtr);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Pointer Descriptor Validation (REQ-POINTER-004)
 * ============================================================================ */

// @{"verify": ["REQ-POINTER-004"]}
static void test_pointer_verify_valid(void)
{
    JUNO_POINTER_T tPtr = TestPtr_Init(&gtValA);
    JUNO_STATUS_T tStatus = JunoMemory_PointerVerify(tPtr);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-POINTER-004"]}
static void test_pointer_verify_null_api(void)
{
    JUNO_POINTER_T tPtr = {NULL, &gtValA, sizeof(TEST_PTR_VALUE_T), alignof(TEST_PTR_VALUE_T)};
    JUNO_STATUS_T tStatus = JunoMemory_PointerVerify(tPtr);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-POINTER-004", "REQ-SYS-008"]}
static void test_pointer_verify_null_addr(void)
{
    JUNO_POINTER_T tPtr = TestPtr_Init(NULL);
    JUNO_STATUS_T tStatus = JunoMemory_PointerVerify(tPtr);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-POINTER-004"]}
static void test_pointer_verify_zero_size(void)
{
    JUNO_POINTER_T tPtr = TestPtr_Init(&gtValA);
    tPtr.zSize = 0;
    JUNO_STATUS_T tStatus = JunoMemory_PointerVerify(tPtr);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Pointer Type Verification (REQ-POINTER-005)
 * ============================================================================ */

// @{"verify": ["REQ-POINTER-005"]}
static void test_pointer_verify_type_match(void)
{
    JUNO_POINTER_T tPtr = TestPtr_Init(&gtValA);
    JUNO_STATUS_T tStatus = TestPtr_Verify(tPtr);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-POINTER-005"]}
static void test_pointer_verify_type_wrong_size(void)
{
    JUNO_POINTER_T tPtr = TestPtr_Init(&gtValA);
    tPtr.zSize = 1; /* wrong size */
    JUNO_STATUS_T tStatus = TestPtr_Verify(tPtr);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-POINTER-005"]}
static void test_pointer_verify_type_wrong_alignment(void)
{
    JUNO_POINTER_T tPtr = TestPtr_Init(&gtValA);
    tPtr.zAlignment = 1; /* wrong alignment */
    JUNO_STATUS_T tStatus = TestPtr_Verify(tPtr);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Main
 * ============================================================================ */

int main(void)
{
    UNITY_BEGIN();
    /* Copy */
    RUN_TEST(test_pointer_copy_success);
    RUN_TEST(test_pointer_copy_null_src_addr);
    /* Reset */
    RUN_TEST(test_pointer_reset_success);
    RUN_TEST(test_pointer_reset_null_addr);
    /* Descriptor Validation */
    RUN_TEST(test_pointer_verify_valid);
    RUN_TEST(test_pointer_verify_null_api);
    RUN_TEST(test_pointer_verify_null_addr);
    RUN_TEST(test_pointer_verify_zero_size);
    /* Type Verification */
    RUN_TEST(test_pointer_verify_type_match);
    RUN_TEST(test_pointer_verify_type_wrong_size);
    RUN_TEST(test_pointer_verify_type_wrong_alignment);
    return UNITY_END();
}
