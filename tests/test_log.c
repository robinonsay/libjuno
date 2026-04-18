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
 * @file test_log.c
 * @brief Unit tests for the LibJuno Log module initialization.
 */

#include "juno/log/log_api.h"
#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include <stdarg.h>
#include <stddef.h>

/* ============================================================================
 * Test Doubles
 * ============================================================================ */

static JUNO_STATUS_T TestLog_Debug(const JUNO_LOG_ROOT_T *ptLog, const char *pcMsg, ...)
{
    (void)ptLog;
    va_list ap;
    va_start(ap, pcMsg);
    va_end(ap);
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestLog_Info(const JUNO_LOG_ROOT_T *ptLog, const char *pcMsg, ...)
{
    (void)ptLog;
    va_list ap;
    va_start(ap, pcMsg);
    va_end(ap);
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestLog_Warning(const JUNO_LOG_ROOT_T *ptLog, const char *pcMsg, ...)
{
    (void)ptLog;
    va_list ap;
    va_start(ap, pcMsg);
    va_end(ap);
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestLog_Error(const JUNO_LOG_ROOT_T *ptLog, const char *pcMsg, ...)
{
    (void)ptLog;
    va_list ap;
    va_start(ap, pcMsg);
    va_end(ap);
    return JUNO_STATUS_SUCCESS;
}

static const JUNO_LOG_API_T gtTestLogApi = {
    TestLog_Debug,
    TestLog_Info,
    TestLog_Warning,
    TestLog_Error
};

static void TestFailureHandler(JUNO_STATUS_T tStatus, const char *pcCustomMessage, JUNO_USER_DATA_T *pvUserData)
{
    (void)tStatus;
    (void)pcCustomMessage;
    (void)pvUserData;
}

/* ============================================================================
 * Fixtures
 * ============================================================================ */

static JUNO_LOG_ROOT_T gtLog;

void setUp(void)
{
    gtLog = (JUNO_LOG_ROOT_T){0};
}

void tearDown(void) {}

/* ============================================================================
 * Test Cases: Log Initialization (REQ-LOG-006)
 * ============================================================================ */

// @{"verify": ["REQ-LOG-006"]}
static void test_log_init_success(void)
{
    JUNO_STATUS_T tStatus = JunoLog_LogInit(&gtLog, &gtTestLogApi, NULL, NULL);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL_PTR(&gtTestLogApi, gtLog.ptApi);
}

// @{"verify": ["REQ-LOG-006"]}
static void test_log_init_stores_failure_handler_and_userdata(void)
{
    int iUserData = 42;
    JUNO_STATUS_T tStatus = JunoLog_LogInit(&gtLog, &gtTestLogApi, TestFailureHandler, &iUserData);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL_PTR(&gtTestLogApi, gtLog.ptApi);
    TEST_ASSERT_EQUAL_PTR(TestFailureHandler, gtLog._pfcnFailureHandler);
    TEST_ASSERT_EQUAL_PTR(&iUserData, gtLog._pvFailureUserData);
}

// @{"verify": ["REQ-LOG-006"]}
static void test_log_init_null_root(void)
{
    JUNO_STATUS_T tStatus = JunoLog_LogInit(NULL, &gtTestLogApi, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Main
 * ============================================================================ */

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_log_init_success);
    RUN_TEST(test_log_init_stores_failure_handler_and_userdata);
    RUN_TEST(test_log_init_null_root);
    return UNITY_END();
}
