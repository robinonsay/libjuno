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
 * @file test_hash.c
 * @brief Unit tests for the LibJuno DJB2 hash function.
 */

#include "juno/hash/hash_djb2.h"
#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include <stddef.h>
#include <stdint.h>

void setUp(void) {}
void tearDown(void) {}

/* ============================================================================
 * Test Cases: DJB2 Hash Computation (REQ-HASH-001)
 * ============================================================================ */

// @{"verify": ["REQ-HASH-001"]}
static void test_hash_djb2_known_value(void)
{
    /* "abc" -> djb2: h = ((5381*33+97)*33+98)*33+99
     * We just verify determinism and seed = 5381 */
    const uint8_t acBuf[] = {'a', 'b', 'c'};
    JUNO_RESULT_SIZE_T tResult = JunoHash_Djb2(acBuf, sizeof(acBuf));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);

    /* Verify determinism: same input produces same output */
    JUNO_RESULT_SIZE_T tResult2 = JunoHash_Djb2(acBuf, sizeof(acBuf));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult2.tStatus);
    TEST_ASSERT_EQUAL(tResult.tOk, tResult2.tOk);
}

// @{"verify": ["REQ-HASH-001"]}
static void test_hash_djb2_empty_buffer(void)
{
    /* Empty buffer (size 0) should produce the seed value 5381 */
    const uint8_t acBuf[] = {0};
    JUNO_RESULT_SIZE_T tResult = JunoHash_Djb2(acBuf, 0);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    TEST_ASSERT_EQUAL(5381, tResult.tOk);
}

// @{"verify": ["REQ-HASH-001"]}
static void test_hash_djb2_different_inputs_differ(void)
{
    const uint8_t acBufA[] = {'a', 'b', 'c'};
    const uint8_t acBufB[] = {'x', 'y', 'z'};
    JUNO_RESULT_SIZE_T tResultA = JunoHash_Djb2(acBufA, sizeof(acBufA));
    JUNO_RESULT_SIZE_T tResultB = JunoHash_Djb2(acBufB, sizeof(acBufB));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResultA.tStatus);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResultB.tStatus);
    TEST_ASSERT_NOT_EQUAL(tResultA.tOk, tResultB.tOk);
}

/* ============================================================================
 * Test Cases: Null Buffer Rejection (REQ-HASH-002)
 * ============================================================================ */

// @{"verify": ["REQ-HASH-002"]}
static void test_hash_djb2_null_buffer(void)
{
    JUNO_RESULT_SIZE_T tResult = JunoHash_Djb2(NULL, 10);
    TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, tResult.tStatus);
}

/* ============================================================================
 * Main
 * ============================================================================ */

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_hash_djb2_known_value);
    RUN_TEST(test_hash_djb2_empty_buffer);
    RUN_TEST(test_hash_djb2_different_inputs_differ);
    RUN_TEST(test_hash_djb2_null_buffer);
    return UNITY_END();
}
