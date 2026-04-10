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
 * @file test_sb.c
 * @brief Unit tests for the LibJuno Software Bus (SB) Broker API.
 */

#include "juno/sb/broker_api.h"
#include "juno/ds/array_api.h"
#include "juno/ds/queue_api.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* ============================================================================
 * Test Message Type
 * ============================================================================ */

typedef struct TEST_SB_MSG_TAG
{
    uint32_t iPayload;
} TEST_SB_MSG_T;

/* ============================================================================
 * Test Array Implementation (backing store for pipes/queues)
 * ============================================================================ */

#define TEST_SB_PIPE_CAPACITY 4

typedef struct TEST_SB_ARRAY_TAG
{
    JUNO_DS_ARRAY_ROOT_T tRoot;
    TEST_SB_MSG_T atBuffer[TEST_SB_PIPE_CAPACITY];
} TEST_SB_ARRAY_T;

/* Forward declarations */
static JUNO_STATUS_T TestSb_SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
static JUNO_RESULT_POINTER_T TestSb_GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
static JUNO_STATUS_T TestSb_RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
static JUNO_STATUS_T TestSbMsg_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
static JUNO_STATUS_T TestSbMsg_Reset(JUNO_POINTER_T tPointer);

/* Pointer API */
const JUNO_POINTER_API_T gtTestSbMsgPointerApi = {
    TestSbMsg_Copy,
    TestSbMsg_Reset
};

#define TestSbMsg_PointerInit(addr) JunoMemory_PointerInit(&gtTestSbMsgPointerApi, TEST_SB_MSG_T, addr)
#define TestSbMsg_PointerVerify(tPointer) JunoMemory_PointerVerifyType(tPointer, TEST_SB_MSG_T, gtTestSbMsgPointerApi)
#define TEST_SB_ASSERT_API(ptArray, ...) if(ptArray->ptApi != &gtTestSbArrayApi) { __VA_ARGS__; }

/* Array API */
static const JUNO_DS_ARRAY_API_T gtTestSbArrayApi = {
    TestSb_SetAt,
    TestSb_GetAt,
    TestSb_RemoveAt
};

/* ============================================================================
 * Pointer API Implementation
 * ============================================================================ */

static JUNO_STATUS_T TestSbMsg_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc)
{
    JUNO_STATUS_T tStatus = TestSbMsg_PointerVerify(tDest);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = TestSbMsg_PointerVerify(tSrc);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    *(TEST_SB_MSG_T *)tDest.pvAddr = *(TEST_SB_MSG_T *)tSrc.pvAddr;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestSbMsg_Reset(JUNO_POINTER_T tPointer)
{
    JUNO_STATUS_T tStatus = TestSbMsg_PointerVerify(tPointer);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    *(TEST_SB_MSG_T *)tPointer.pvAddr = (TEST_SB_MSG_T){0};
    return JUNO_STATUS_SUCCESS;
}

/* ============================================================================
 * Array API Implementation
 * ============================================================================ */

static JUNO_STATUS_T TestSb_SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    TEST_SB_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    tStatus = TestSbMsg_PointerVerify(tItem);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    TEST_SB_ARRAY_T *ptTestArray = (TEST_SB_ARRAY_T *)ptArray;
    JUNO_POINTER_T tDest = TestSbMsg_PointerInit(&ptTestArray->atBuffer[iIndex]);
    return TestSbMsg_Copy(tDest, tItem);
}

static JUNO_RESULT_POINTER_T TestSb_GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_RESULT_POINTER_T tResult = {JUNO_STATUS_ERR, {0}};
    tResult.tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    TEST_SB_ASSERT_API(ptArray, tResult.tStatus = JUNO_STATUS_INVALID_TYPE_ERROR; return tResult);
    TEST_SB_ARRAY_T *ptTestArray = (TEST_SB_ARRAY_T *)ptArray;
    tResult.tOk = TestSbMsg_PointerInit(&ptTestArray->atBuffer[iIndex]);
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    return tResult;
}

static JUNO_STATUS_T TestSb_RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    TEST_SB_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    TEST_SB_ARRAY_T *ptTestArray = (TEST_SB_ARRAY_T *)ptArray;
    JUNO_POINTER_T tSlot = TestSbMsg_PointerInit(&ptTestArray->atBuffer[iIndex]);
    return TestSbMsg_Reset(tSlot);
}

/* ============================================================================
 * Fixtures
 * ============================================================================ */

#define TEST_SB_REGISTRY_CAPACITY 4

static JUNO_SB_BROKER_ROOT_T gtBroker;
static JUNO_SB_PIPE_T       *gptPipeRegistry[TEST_SB_REGISTRY_CAPACITY];

/* Pipe A – MID 1 */
static TEST_SB_ARRAY_T gtPipeArrayA;
static JUNO_SB_PIPE_T  gtPipeA;

/* Pipe B – MID 1 (second subscriber to same topic) */
static TEST_SB_ARRAY_T gtPipeArrayB;
static JUNO_SB_PIPE_T  gtPipeB;

/* Pipe C – MID 2 (different topic) */
static TEST_SB_ARRAY_T gtPipeArrayC;
static JUNO_SB_PIPE_T  gtPipeC;

static void InitTestArray(TEST_SB_ARRAY_T *ptArr)
{
    memset(ptArr, 0, sizeof(*ptArr));
    JunoDs_ArrayInit(&ptArr->tRoot, &gtTestSbArrayApi, TEST_SB_PIPE_CAPACITY,
                     NULL, NULL);
}

void setUp(void)
{
    memset(&gtBroker, 0, sizeof(gtBroker));
    memset(gptPipeRegistry, 0, sizeof(gptPipeRegistry));
    memset(&gtPipeA, 0, sizeof(gtPipeA));
    memset(&gtPipeB, 0, sizeof(gtPipeB));
    memset(&gtPipeC, 0, sizeof(gtPipeC));

    InitTestArray(&gtPipeArrayA);
    InitTestArray(&gtPipeArrayB);
    InitTestArray(&gtPipeArrayC);
}

void tearDown(void)
{
}

/* ============================================================================
 * Test Cases: Broker Initialization (REQ-SB-003)
 * ============================================================================ */

// @{"verify": ["REQ-SB-003"]}
static void test_sb_broker_init_success(void)
{
    JUNO_STATUS_T tStatus = JunoSb_BrokerInit(&gtBroker, gptPipeRegistry,
                                               TEST_SB_REGISTRY_CAPACITY,
                                               NULL, NULL);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_NOT_NULL(gtBroker.ptApi);
    TEST_ASSERT_EQUAL_PTR(gptPipeRegistry, gtBroker.ptPipeRegistry);
    TEST_ASSERT_EQUAL(TEST_SB_REGISTRY_CAPACITY, gtBroker.zRegistryCapacity);
    TEST_ASSERT_EQUAL(0, gtBroker.zRegistryLength);
}

// @{"verify": ["REQ-SB-003"]}
static void test_sb_broker_init_null_broker(void)
{
    JUNO_STATUS_T tStatus = JunoSb_BrokerInit(NULL, gptPipeRegistry,
                                               TEST_SB_REGISTRY_CAPACITY,
                                               NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-SB-003"]}
static void test_sb_broker_init_null_registry(void)
{
    JUNO_STATUS_T tStatus = JunoSb_BrokerInit(&gtBroker, NULL,
                                               TEST_SB_REGISTRY_CAPACITY,
                                               NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Pipe Initialization (REQ-SB-005)
 * ============================================================================ */

// @{"verify": ["REQ-SB-005"]}
static void test_sb_pipe_init_success(void)
{
    JUNO_STATUS_T tStatus = JunoSb_PipeInit(&gtPipeA, 42, &gtPipeArrayA.tRoot,
                                             NULL, NULL);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(42, gtPipeA.iMsgId);
}

// @{"verify": ["REQ-SB-005"]}
static void test_sb_pipe_init_null_pipe(void)
{
    JUNO_STATUS_T tStatus = JunoSb_PipeInit(NULL, 1, &gtPipeArrayA.tRoot,
                                             NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Register Subscriber (REQ-SB-008, REQ-SB-009)
 * ============================================================================ */

// @{"verify": ["REQ-SB-008"]}
static void test_sb_register_subscriber(void)
{
    JunoSb_BrokerInit(&gtBroker, gptPipeRegistry,
                       TEST_SB_REGISTRY_CAPACITY, NULL, NULL);
    JunoSb_PipeInit(&gtPipeA, 1, &gtPipeArrayA.tRoot, NULL, NULL);

    JUNO_STATUS_T tStatus = gtBroker.ptApi->RegisterSubscriber(&gtBroker, &gtPipeA);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(1, gtBroker.zRegistryLength);
    TEST_ASSERT_EQUAL_PTR(&gtPipeA, gtBroker.ptPipeRegistry[0]);
}

// @{"verify": ["REQ-SB-009"]}
static void test_sb_register_full_rejection(void)
{
    /* Use a capacity-1 broker */
    JUNO_SB_PIPE_T *ptSmallRegistry[1];
    JunoSb_BrokerInit(&gtBroker, ptSmallRegistry, 1, NULL, NULL);
    JunoSb_PipeInit(&gtPipeA, 1, &gtPipeArrayA.tRoot, NULL, NULL);
    JunoSb_PipeInit(&gtPipeB, 2, &gtPipeArrayB.tRoot, NULL, NULL);

    JUNO_STATUS_T tStatus = gtBroker.ptApi->RegisterSubscriber(&gtBroker, &gtPipeA);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);

    /* Second registration should fail – registry full */
    tStatus = gtBroker.ptApi->RegisterSubscriber(&gtBroker, &gtPipeB);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Publish Fan-Out / Non-Matching Skip (REQ-SB-006, REQ-SB-007)
 * ============================================================================ */

// @{"verify": ["REQ-SB-006"]}
static void test_sb_publish_fan_out(void)
{
    JunoSb_BrokerInit(&gtBroker, gptPipeRegistry,
                       TEST_SB_REGISTRY_CAPACITY, NULL, NULL);
    JunoSb_PipeInit(&gtPipeA, 1, &gtPipeArrayA.tRoot, NULL, NULL);
    JunoSb_PipeInit(&gtPipeB, 1, &gtPipeArrayB.tRoot, NULL, NULL);

    gtBroker.ptApi->RegisterSubscriber(&gtBroker, &gtPipeA);
    gtBroker.ptApi->RegisterSubscriber(&gtBroker, &gtPipeB);

    TEST_SB_MSG_T tMsg = {.iPayload = 0xDEAD};
    JUNO_POINTER_T tMsgPtr = TestSbMsg_PointerInit(&tMsg);

    JUNO_STATUS_T tStatus = gtBroker.ptApi->Publish(&gtBroker, 1, tMsgPtr);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);

    /* Both pipes should have received the message */
    TEST_ASSERT_EQUAL(1, gtPipeA.tRoot.zLength);
    TEST_ASSERT_EQUAL(1, gtPipeB.tRoot.zLength);

    /* Verify contents of pipe A */
    TEST_SB_MSG_T tOut = {0};
    JUNO_POINTER_T tOutPtr = TestSbMsg_PointerInit(&tOut);
    tStatus = gtPipeA.tRoot.ptApi->Dequeue(&gtPipeA.tRoot, tOutPtr);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL_HEX32(0xDEAD, tOut.iPayload);
}

// @{"verify": ["REQ-SB-007"]}
static void test_sb_publish_skip_non_matching(void)
{
    JunoSb_BrokerInit(&gtBroker, gptPipeRegistry,
                       TEST_SB_REGISTRY_CAPACITY, NULL, NULL);
    JunoSb_PipeInit(&gtPipeA, 1, &gtPipeArrayA.tRoot, NULL, NULL);
    JunoSb_PipeInit(&gtPipeC, 2, &gtPipeArrayC.tRoot, NULL, NULL);

    gtBroker.ptApi->RegisterSubscriber(&gtBroker, &gtPipeA);
    gtBroker.ptApi->RegisterSubscriber(&gtBroker, &gtPipeC);

    TEST_SB_MSG_T tMsg = {.iPayload = 0xBEEF};
    JUNO_POINTER_T tMsgPtr = TestSbMsg_PointerInit(&tMsg);

    /* Publish to MID 1 */
    JUNO_STATUS_T tStatus = gtBroker.ptApi->Publish(&gtBroker, 1, tMsgPtr);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);

    /* Pipe A (MID=1) should have it, Pipe C (MID=2) should not */
    TEST_ASSERT_EQUAL(1, gtPipeA.tRoot.zLength);
    TEST_ASSERT_EQUAL(0, gtPipeC.tRoot.zLength);
}

/* ============================================================================
 * Test Cases: Broker Verification (REQ-SB-010, REQ-SB-011, REQ-SB-012)
 * ============================================================================ */

// @{"verify": ["REQ-SB-010"]}
static void test_sb_broker_verify_null_api(void)
{
    gtBroker.ptApi = NULL;
    gtBroker.ptPipeRegistry = gptPipeRegistry;
    gtBroker.zRegistryCapacity = TEST_SB_REGISTRY_CAPACITY;
    JUNO_STATUS_T tStatus = JunoSb_BrokerVerify(&gtBroker);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-SB-011", "REQ-SYS-011"]}
static void test_sb_broker_verify_null_registry(void)
{
    JunoSb_BrokerInit(&gtBroker, gptPipeRegistry,
                       TEST_SB_REGISTRY_CAPACITY, NULL, NULL);
    gtBroker.ptPipeRegistry = NULL;
    JUNO_STATUS_T tStatus = JunoSb_BrokerVerify(&gtBroker);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-SB-011"]}
static void test_sb_broker_verify_zero_capacity(void)
{
    JunoSb_BrokerInit(&gtBroker, gptPipeRegistry,
                       TEST_SB_REGISTRY_CAPACITY, NULL, NULL);
    gtBroker.zRegistryCapacity = 0;
    JUNO_STATUS_T tStatus = JunoSb_BrokerVerify(&gtBroker);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-SB-012"]}
static void test_sb_pipe_verify_valid(void)
{
    JunoSb_PipeInit(&gtPipeA, 1, &gtPipeArrayA.tRoot, NULL, NULL);
    JUNO_STATUS_T tStatus = JunoSb_PipeVerify(&gtPipeA);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// @{"verify": ["REQ-SB-012"]}
static void test_sb_pipe_verify_null(void)
{
    JUNO_STATUS_T tStatus = JunoSb_PipeVerify(NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Main
 * ============================================================================ */

int main(void)
{
    UNITY_BEGIN();
    /* Broker Init */
    RUN_TEST(test_sb_broker_init_success);
    RUN_TEST(test_sb_broker_init_null_broker);
    RUN_TEST(test_sb_broker_init_null_registry);
    /* Pipe Init */
    RUN_TEST(test_sb_pipe_init_success);
    RUN_TEST(test_sb_pipe_init_null_pipe);
    /* Register */
    RUN_TEST(test_sb_register_subscriber);
    RUN_TEST(test_sb_register_full_rejection);
    /* Publish */
    RUN_TEST(test_sb_publish_fan_out);
    RUN_TEST(test_sb_publish_skip_non_matching);
    /* Verification */
    RUN_TEST(test_sb_broker_verify_null_api);
    RUN_TEST(test_sb_broker_verify_null_registry);
    RUN_TEST(test_sb_broker_verify_zero_capacity);
    RUN_TEST(test_sb_pipe_verify_valid);
    RUN_TEST(test_sb_pipe_verify_null);
    return UNITY_END();
}
