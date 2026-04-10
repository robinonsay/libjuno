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
 * @file test_sm.c
 * @brief Unit tests for the LibJuno State Machine (SM) API.
 */

#include "juno/sm/sm_api.h"
#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * Test Double: State API
 * ============================================================================ */

static bool gbActionCalled = false;
static bool gbShouldExit   = false;
static bool gbResetCalled  = false;

static JUNO_STATUS_T TestSm_StateAction(JUNO_SM_STATE_ROOT_T *ptState)
{
    (void)ptState;
    gbActionCalled = true;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_RESULT_BOOL_T TestSm_ShouldExit(JUNO_SM_STATE_ROOT_T *ptState)
{
    (void)ptState;
    JUNO_RESULT_BOOL_T tResult = {JUNO_STATUS_SUCCESS, gbShouldExit};
    return tResult;
}

static JUNO_STATUS_T TestSm_ResetState(JUNO_SM_STATE_ROOT_T *ptState)
{
    (void)ptState;
    gbResetCalled = true;
    return JUNO_STATUS_SUCCESS;
}

static const JUNO_SM_STATE_API_T gtTestStateApi = {
    .StateAction = TestSm_StateAction,
    .ShouldExit  = TestSm_ShouldExit,
    .ResetState  = TestSm_ResetState,
};

/* ============================================================================
 * Fixtures
 * ============================================================================ */

static JUNO_SM_ROOT_T       gtSmRoot;
static JUNO_SM_STATE_ROOT_T gtStateA;
static JUNO_SM_STATE_ROOT_T gtStateB;

void setUp(void)
{
    gtSmRoot = (JUNO_SM_ROOT_T){0};
    gtStateA = (JUNO_SM_STATE_ROOT_T){0};
    gtStateB = (JUNO_SM_STATE_ROOT_T){0};
    gbActionCalled = false;
    gbShouldExit   = false;
    gbResetCalled  = false;
}

void tearDown(void)
{
}

/* ============================================================================
 * Test Cases: SM Initialization (REQ-SM-004)
 * ============================================================================ */

// @{"verify": ["REQ-SM-004"]}
static void test_sm_init_success(void)
{
    JUNO_STATUS_T tStatus = JunoSm_StateInit(&gtSmRoot, &gtStateA, NULL,
                                              &gtTestStateApi, NULL, NULL);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);

    tStatus = JunoSm_Init(&gtSmRoot, &gtStateA, NULL, NULL);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL_PTR(&gtStateA, gtSmRoot.ptCurrentState);
}

// @{"verify": ["REQ-SM-004"]}
static void test_sm_init_null_root(void)
{
    JUNO_STATUS_T tStatus = JunoSm_Init(NULL, &gtStateA, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-SM-004"]}
static void test_sm_init_null_start_state(void)
{
    JUNO_STATUS_T tStatus = JunoSm_Init(&gtSmRoot, NULL, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: State Initialization (REQ-SM-005)
 * ============================================================================ */

// @{"verify": ["REQ-SM-005"]}
static void test_sm_state_init_with_next(void)
{
    /* Init SM first so StateA can link to it */
    gtSmRoot.ptCurrentState = &gtStateA;

    JUNO_STATUS_T tStatus = JunoSm_StateInit(&gtSmRoot, &gtStateA, &gtStateB,
                                              &gtTestStateApi, NULL, NULL);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL_PTR(&gtSmRoot, gtStateA.ptSm);
    TEST_ASSERT_EQUAL_PTR(&gtTestStateApi, gtStateA.ptApi);
    TEST_ASSERT_TRUE(gtStateA.tOptionNextState.bIsSome);
    TEST_ASSERT_EQUAL_PTR(&gtStateB, gtStateA.tOptionNextState.tSome);
}

// @{"verify": ["REQ-SM-005"]}
static void test_sm_state_init_without_next(void)
{
    gtSmRoot.ptCurrentState = &gtStateA;

    JUNO_STATUS_T tStatus = JunoSm_StateInit(&gtSmRoot, &gtStateA, NULL,
                                              &gtTestStateApi, NULL, NULL);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_FALSE(gtStateA.tOptionNextState.bIsSome);
    TEST_ASSERT_NULL(gtStateA.tOptionNextState.tSome);
}

// @{"verify": ["REQ-SM-005"]}
static void test_sm_state_init_null_state(void)
{
    JUNO_STATUS_T tStatus = JunoSm_StateInit(&gtSmRoot, NULL, NULL,
                                              &gtTestStateApi, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-SM-005"]}
static void test_sm_state_init_null_api(void)
{
    JUNO_STATUS_T tStatus = JunoSm_StateInit(&gtSmRoot, &gtStateA, NULL,
                                              NULL, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Get Current State (REQ-SM-006)
 * ============================================================================ */

// @{"verify": ["REQ-SM-006"]}
static void test_sm_get_current_state(void)
{
    JunoSm_StateInit(&gtSmRoot, &gtStateA, NULL, &gtTestStateApi, NULL, NULL);
    JunoSm_Init(&gtSmRoot, &gtStateA, NULL, NULL);

    JUNO_SM_RESULT_STATE_T tResult = JunoSm_GetCurrentState(&gtSmRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    TEST_ASSERT_EQUAL_PTR(&gtStateA, tResult.tOk);
}

// @{"verify": ["REQ-SM-006"]}
static void test_sm_get_current_state_null_root(void)
{
    JUNO_SM_RESULT_STATE_T tResult = JunoSm_GetCurrentState(NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
}

/* ============================================================================
 * Test Cases: State Transition (REQ-SM-007, REQ-SM-008)
 * ============================================================================ */

// @{"verify": ["REQ-SM-007"]}
static void test_sm_transition_to_next(void)
{
    /* Set up: A → B, B → none */
    JunoSm_StateInit(&gtSmRoot, &gtStateB, NULL, &gtTestStateApi, NULL, NULL);
    JunoSm_StateInit(&gtSmRoot, &gtStateA, &gtStateB, &gtTestStateApi, NULL, NULL);
    JunoSm_Init(&gtSmRoot, &gtStateA, NULL, NULL);

    TEST_ASSERT_EQUAL_PTR(&gtStateA, gtSmRoot.ptCurrentState);

    JUNO_SM_RESULT_OPTION_STATE_T tResult = JunoSm_TransitionState(&gtSmRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    TEST_ASSERT_TRUE(tResult.tOk.bIsSome);
    TEST_ASSERT_EQUAL_PTR(&gtStateB, tResult.tOk.tSome);
    TEST_ASSERT_EQUAL_PTR(&gtStateB, gtSmRoot.ptCurrentState);
}

// @{"verify": ["REQ-SM-008"]}
static void test_sm_transition_stays_when_none(void)
{
    /* Set up: A → none */
    JunoSm_StateInit(&gtSmRoot, &gtStateA, NULL, &gtTestStateApi, NULL, NULL);
    JunoSm_Init(&gtSmRoot, &gtStateA, NULL, NULL);

    JUNO_SM_RESULT_OPTION_STATE_T tResult = JunoSm_TransitionState(&gtSmRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    TEST_ASSERT_FALSE(tResult.tOk.bIsSome);
    TEST_ASSERT_EQUAL_PTR(&gtStateA, gtSmRoot.ptCurrentState);
}

// @{"verify": ["REQ-SM-007"]}
static void test_sm_transition_null_root(void)
{
    JUNO_SM_RESULT_OPTION_STATE_T tResult = JunoSm_TransitionState(NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
}

/* ============================================================================
 * Test Cases: SM Verification (REQ-SM-009)
 * ============================================================================ */

// @{"verify": ["REQ-SM-009"]}
static void test_sm_verify_valid(void)
{
    JunoSm_StateInit(&gtSmRoot, &gtStateA, NULL, &gtTestStateApi, NULL, NULL);
    JunoSm_Init(&gtSmRoot, &gtStateA, NULL, NULL);

    JUNO_STATUS_T tStatus = JunoSm_Verify(&gtSmRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-SM-009"]}
static void test_sm_verify_null_root(void)
{
    JUNO_STATUS_T tStatus = JunoSm_Verify(NULL);
    TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, tStatus);
}

// @{"verify": ["REQ-SM-009"]}
static void test_sm_verify_null_current_state(void)
{
    gtSmRoot.ptCurrentState = NULL;
    JUNO_STATUS_T tStatus = JunoSm_Verify(&gtSmRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, tStatus);
}

/* ============================================================================
 * Test Cases: State Verification (REQ-SM-010, REQ-SM-011)
 * ============================================================================ */

// @{"verify": ["REQ-SM-010"]}
static void test_sm_state_verify_valid(void)
{
    JunoSm_StateInit(&gtSmRoot, &gtStateA, NULL, &gtTestStateApi, NULL, NULL);
    JunoSm_Init(&gtSmRoot, &gtStateA, NULL, NULL);

    JUNO_STATUS_T tStatus = JunoSm_StateVerify(&gtStateA);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-SM-010"]}
static void test_sm_state_verify_null_state(void)
{
    JUNO_STATUS_T tStatus = JunoSm_StateVerify(NULL);
    TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, tStatus);
}

// @{"verify": ["REQ-SM-010"]}
static void test_sm_state_verify_null_api(void)
{
    gtStateA.ptApi = NULL;
    gtStateA.ptSm  = &gtSmRoot;
    JUNO_STATUS_T tStatus = JunoSm_StateVerify(&gtStateA);
    TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, tStatus);
}

// @{"verify": ["REQ-SM-010"]}
static void test_sm_state_verify_null_sm(void)
{
    gtStateA.ptApi = &gtTestStateApi;
    gtStateA.ptSm  = NULL;
    JUNO_STATUS_T tStatus = JunoSm_StateVerify(&gtStateA);
    TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, tStatus);
}

// @{"verify": ["REQ-SM-011"]}
static void test_sm_state_verify_next_null_when_some(void)
{
    gtStateA.ptApi = &gtTestStateApi;
    gtStateA.ptSm  = &gtSmRoot;
    gtStateA.tOptionNextState.bIsSome = true;
    gtStateA.tOptionNextState.tSome   = NULL;
    JUNO_STATUS_T tStatus = JunoSm_StateVerify(&gtStateA);
    TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, tStatus);
}

// @{"verify": ["REQ-SM-011"]}
static void test_sm_state_verify_next_valid_when_some(void)
{
    JunoSm_StateInit(&gtSmRoot, &gtStateA, &gtStateB, &gtTestStateApi, NULL, NULL);
    JunoSm_Init(&gtSmRoot, &gtStateA, NULL, NULL);

    JUNO_STATUS_T tStatus = JunoSm_StateVerify(&gtStateA);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Main
 * ============================================================================ */

int main(void)
{
    UNITY_BEGIN();
    /* SM Init */
    RUN_TEST(test_sm_init_success);
    RUN_TEST(test_sm_init_null_root);
    RUN_TEST(test_sm_init_null_start_state);
    /* State Init */
    RUN_TEST(test_sm_state_init_with_next);
    RUN_TEST(test_sm_state_init_without_next);
    RUN_TEST(test_sm_state_init_null_state);
    RUN_TEST(test_sm_state_init_null_api);
    /* Get Current State */
    RUN_TEST(test_sm_get_current_state);
    RUN_TEST(test_sm_get_current_state_null_root);
    /* Transition */
    RUN_TEST(test_sm_transition_to_next);
    RUN_TEST(test_sm_transition_stays_when_none);
    RUN_TEST(test_sm_transition_null_root);
    /* SM Verify */
    RUN_TEST(test_sm_verify_valid);
    RUN_TEST(test_sm_verify_null_root);
    RUN_TEST(test_sm_verify_null_current_state);
    /* State Verify */
    RUN_TEST(test_sm_state_verify_valid);
    RUN_TEST(test_sm_state_verify_null_state);
    RUN_TEST(test_sm_state_verify_null_api);
    RUN_TEST(test_sm_state_verify_null_sm);
    RUN_TEST(test_sm_state_verify_next_null_when_some);
    RUN_TEST(test_sm_state_verify_next_valid_when_some);
    return UNITY_END();
}
