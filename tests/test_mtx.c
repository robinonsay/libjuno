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
 * @file test_mtx.c
 * @brief Unit tests for the LibJuno Mutex API verification.
 */

#include "juno/mp/mtx_api.h"
#include "juno/status.h"
#include "juno/types.h"
#include "unity.h"
#include "unity_internals.h"
#include <stddef.h>

/* ============================================================================
 * Test Doubles
 * ============================================================================ */

static JUNO_RESULT_BOOL_T TestMtx_TryLock(JUNO_MP_MTX_ROOT_T *ptMtx)
{
    (void)ptMtx;
    JUNO_RESULT_BOOL_T tResult = {JUNO_STATUS_SUCCESS, true};
    return tResult;
}

static JUNO_STATUS_T TestMtx_Lock(JUNO_MP_MTX_ROOT_T *ptMtx)
{
    (void)ptMtx;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestMtx_Free(JUNO_MP_MTX_ROOT_T *ptMtx)
{
    (void)ptMtx;
    return JUNO_STATUS_SUCCESS;
}

static const JUNO_MP_MTX_API_T gtTestMtxApi = {
    TestMtx_TryLock,
    TestMtx_Lock,
    TestMtx_Free
};

/* ============================================================================
 * Fixtures
 * ============================================================================ */

static JUNO_MP_MTX_ROOT_T gtMtx;

void setUp(void)
{
    gtMtx = (JUNO_MP_MTX_ROOT_T){0};
    gtMtx.ptApi = &gtTestMtxApi;
}

void tearDown(void) {}

/* ============================================================================
 * Test Cases: Mutex Vtable Validation (REQ-MTX-005)
 * ============================================================================ */

// @{"verify": ["REQ-MTX-005"]}
static void test_mtx_verify_valid(void)
{
    JUNO_STATUS_T tStatus = JunoMp_MtxVerify(&gtMtx);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-MTX-005"]}
static void test_mtx_verify_null_root(void)
{
    JUNO_STATUS_T tStatus = JunoMp_MtxVerify(NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-MTX-005"]}
static void test_mtx_verify_null_api(void)
{
    gtMtx.ptApi = NULL;
    JUNO_STATUS_T tStatus = JunoMp_MtxVerify(&gtMtx);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-MTX-005"]}
static void test_mtx_verify_null_trylock(void)
{
    JUNO_MP_MTX_API_T tBadApi = {NULL, TestMtx_Lock, TestMtx_Free};
    gtMtx.ptApi = &tBadApi;
    JUNO_STATUS_T tStatus = JunoMp_MtxVerify(&gtMtx);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-MTX-005"]}
static void test_mtx_verify_null_lock(void)
{
    JUNO_MP_MTX_API_T tBadApi = {TestMtx_TryLock, NULL, TestMtx_Free};
    gtMtx.ptApi = &tBadApi;
    JUNO_STATUS_T tStatus = JunoMp_MtxVerify(&gtMtx);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-MTX-005"]}
static void test_mtx_verify_null_free(void)
{
    JUNO_MP_MTX_API_T tBadApi = {TestMtx_TryLock, TestMtx_Lock, NULL};
    gtMtx.ptApi = &tBadApi;
    JUNO_STATUS_T tStatus = JunoMp_MtxVerify(&gtMtx);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Main
 * ============================================================================ */

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_mtx_verify_valid);
    RUN_TEST(test_mtx_verify_null_root);
    RUN_TEST(test_mtx_verify_null_api);
    RUN_TEST(test_mtx_verify_null_trylock);
    RUN_TEST(test_mtx_verify_null_lock);
    RUN_TEST(test_mtx_verify_null_free);
    return UNITY_END();
}
