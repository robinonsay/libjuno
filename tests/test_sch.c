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
 * @file test_sch.c
 * @brief Unit tests for the LibJuno Scheduler API.
 */

#include "juno/sch/juno_sch_api.h"
#include "juno/app/app_api.h"
#include "juno/status.h"
#include "juno/time/time_api.h"
#include "unity.h"
#include "unity_internals.h"
#include <stddef.h>
#include <string.h>

/* ============================================================================
 * Test Doubles: App
 * ============================================================================ */

static int giProcessCount;

static JUNO_STATUS_T TestApp_OnStart(JUNO_APP_ROOT_T *ptApp)
{
    (void)ptApp;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestApp_OnProcess(JUNO_APP_ROOT_T *ptApp)
{
    (void)ptApp;
    giProcessCount++;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestApp_OnExit(JUNO_APP_ROOT_T *ptApp)
{
    (void)ptApp;
    return JUNO_STATUS_SUCCESS;
}

static const JUNO_APP_API_T gtTestAppApi = {
    TestApp_OnStart,
    TestApp_OnProcess,
    TestApp_OnExit
};

/* ============================================================================
 * Test Doubles: Scheduler API
 * ============================================================================ */

static JUNO_STATUS_T TestSch_Execute(JUNO_SCH_ROOT_T *ptSch)
{
    /* Walk all minor frames, invoke OnProcess for each app slot */
    for (size_t iFrame = 0; iFrame < ptSch->zNumMinorFrames; iFrame++)
    {
        for (size_t iApp = 0; iApp < ptSch->zAppsPerMinorFrame; iApp++)
        {
            size_t iIdx = iFrame * ptSch->zAppsPerMinorFrame + iApp;
            JUNO_APP_ROOT_T *ptApp = ptSch->ptArrSchTable[iIdx];
            if (ptApp && ptApp->ptApi && ptApp->ptApi->OnProcess)
            {
                ptApp->ptApi->OnProcess(ptApp);
            }
        }
    }
    return JUNO_STATUS_SUCCESS;
}

static JUNO_TIMESTAMP_RESULT_T TestSch_GetMinorFramePeriod(JUNO_SCH_ROOT_T *ptSch)
{
    JUNO_TIMESTAMP_RESULT_T tResult = {0};
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    tResult.tOk = ptSch->tMinorFramePeriod;
    return tResult;
}

static JUNO_TIMESTAMP_RESULT_T TestSch_GetMajorFramePeriod(JUNO_SCH_ROOT_T *ptSch)
{
    JUNO_TIMESTAMP_RESULT_T tResult = {0};
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    tResult.tOk.iSeconds = ptSch->tMinorFramePeriod.iSeconds * (uint32_t)ptSch->zNumMinorFrames;
    tResult.tOk.iSubSeconds = ptSch->tMinorFramePeriod.iSubSeconds * (uint32_t)ptSch->zNumMinorFrames;
    return tResult;
}

static const JUNO_SCH_API_T gtTestSchApi = {
    TestSch_Execute,
    TestSch_GetMinorFramePeriod,
    TestSch_GetMajorFramePeriod
};

/* ============================================================================
 * Fixtures
 * ============================================================================ */

#define TEST_SCH_APPS_PER_MINOR 2
#define TEST_SCH_MINOR_FRAMES   3

static JUNO_APP_ROOT_T  gtAppA;
static JUNO_APP_ROOT_T  gtAppB;
static JUNO_SCH_ROOT_T  gtSch;

/* Flattened schedule table: 3 minor frames × 2 apps */
static JUNO_APP_ROOT_T *gptSchTable[TEST_SCH_MINOR_FRAMES * TEST_SCH_APPS_PER_MINOR];

void setUp(void)
{
    giProcessCount = 0;
    memset(&gtAppA, 0, sizeof(gtAppA));
    memset(&gtAppB, 0, sizeof(gtAppB));
    memset(&gtSch, 0, sizeof(gtSch));
    memset(gptSchTable, 0, sizeof(gptSchTable));

    gtAppA.ptApi = &gtTestAppApi;
    gtAppB.ptApi = &gtTestAppApi;

    /* Fill table: AppA in slot 0, AppB in slot 1 of each minor frame */
    for (size_t i = 0; i < TEST_SCH_MINOR_FRAMES; i++)
    {
        gptSchTable[i * TEST_SCH_APPS_PER_MINOR + 0] = &gtAppA;
        gptSchTable[i * TEST_SCH_APPS_PER_MINOR + 1] = &gtAppB;
    }

    gtSch.ptApi = &gtTestSchApi;
    gtSch.ptArrSchTable = gptSchTable;
    gtSch.zAppsPerMinorFrame = TEST_SCH_APPS_PER_MINOR;
    gtSch.zNumMinorFrames = TEST_SCH_MINOR_FRAMES;
    gtSch.tMinorFramePeriod = (JUNO_TIMESTAMP_T){.iSeconds = 0, .iSubSeconds = 100};
}

void tearDown(void) {}

/* ============================================================================
 * Test Cases: Execute Major Frame (REQ-SCH-004)
 * ============================================================================ */

// @{"verify": ["REQ-SCH-004"]}
static void test_sch_execute_invokes_all_apps(void)
{
    JUNO_STATUS_T tStatus = gtSch.ptApi->Execute(&gtSch);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    /* 2 apps × 3 minor frames = 6 OnProcess calls */
    TEST_ASSERT_EQUAL(6, giProcessCount);
}

// @{"verify": ["REQ-SCH-004"]}
static void test_sch_execute_skips_null_slot(void)
{
    /* Null out one slot */
    gptSchTable[1] = NULL;
    JUNO_STATUS_T tStatus = gtSch.ptApi->Execute(&gtSch);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    /* 3 frames × 2 slots – 1 null = 5 */
    TEST_ASSERT_EQUAL(5, giProcessCount);
}

/* ============================================================================
 * Test Cases: Minor Frame Period Query (REQ-SCH-005)
 * ============================================================================ */

// @{"verify": ["REQ-SCH-005"]}
static void test_sch_get_minor_frame_period(void)
{
    JUNO_TIMESTAMP_RESULT_T tResult = gtSch.ptApi->GetMinorFramePeriod(&gtSch);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    TEST_ASSERT_EQUAL(0, tResult.tOk.iSeconds);
    TEST_ASSERT_EQUAL(100, tResult.tOk.iSubSeconds);
}

/* ============================================================================
 * Test Cases: Major Frame Period Computation (REQ-SCH-006)
 * ============================================================================ */

// @{"verify": ["REQ-SCH-006"]}
static void test_sch_get_major_frame_period(void)
{
    JUNO_TIMESTAMP_RESULT_T tResult = gtSch.ptApi->GetMajorFramePeriod(&gtSch);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    /* 3 minor frames × 100 subseconds = 300 subseconds */
    TEST_ASSERT_EQUAL(0, tResult.tOk.iSeconds);
    TEST_ASSERT_EQUAL(300, tResult.tOk.iSubSeconds);
}

/* ============================================================================
 * Main
 * ============================================================================ */

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_sch_execute_invokes_all_apps);
    RUN_TEST(test_sch_execute_skips_null_slot);
    RUN_TEST(test_sch_get_minor_frame_period);
    RUN_TEST(test_sch_get_major_frame_period);
    return UNITY_END();
}
