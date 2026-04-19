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

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * @file test_thread_module.cpp
 * @brief Google Test suite for the Thread module (JunoThread_Init and vtable dispatch).
 *
 * @details
 *  Covers requirements: REQ-THREAD-003, REQ-THREAD-005, REQ-THREAD-006,
 *  REQ-THREAD-007, REQ-THREAD-009, REQ-THREAD-010, REQ-THREAD-012,
 *  REQ-THREAD-013.
 *
 *  Test doubles use a vtable-injected fake JUNO_THREAD_API_T covering the
 *  {Stop, Join, Free} dispatch slots.  No OS thread API (pthreads) is called
 *  from this file.  No dynamic allocation is used.
 */

#include <gtest/gtest.h>
#include <cstring>

#include "juno/thread.h"
#include "juno/status.h"

/* =========================================================================
 * Test Doubles — vtable-injected fake JUNO_THREAD_API_T
 * ========================================================================= */

static bool s_bStopCalled = false;
static bool s_bJoinCalled = false;
static bool s_bFreeCalled = false;

/* Injected failure flags */
static bool          s_bStopFail        = false;
static bool          s_bJoinFail        = false;
static bool          s_bFreeFail        = false;
static JUNO_STATUS_T s_tStopFailStatus  = JUNO_STATUS_ERR;
static JUNO_STATUS_T s_tJoinFailStatus  = JUNO_STATUS_ERR;
static JUNO_STATUS_T s_tFreeFailStatus  = JUNO_STATUS_ERR;

static JUNO_STATUS_T TestStop(JUNO_THREAD_ROOT_T *ptRoot)
{
    s_bStopCalled = true;
    if (s_bStopFail)
    {
        return s_tStopFailStatus;
    }
    ptRoot->bStop = true;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestJoin(JUNO_THREAD_ROOT_T *ptRoot)
{
    (void)ptRoot;
    s_bJoinCalled = true;
    if (s_bJoinFail)
    {
        return s_tJoinFailStatus;
    }
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestFree(JUNO_THREAD_ROOT_T *ptRoot)
{
    (void)ptRoot;
    s_bFreeCalled = true;
    if (s_bFreeFail)
    {
        return s_tFreeFailStatus;
    }
    return JUNO_STATUS_SUCCESS;
}

static const JUNO_THREAD_API_T s_tTestApi = { TestStop, TestJoin, TestFree };

/* =========================================================================
 * Failure Handler Double
 * ========================================================================= */

static int s_iFhCallCount = 0;

static void TestFailureHandler(JUNO_STATUS_T /*tStatus*/,
                               const char * /*pcMessage*/,
                               void       * /*pvUserData*/)
{
    s_iFhCallCount++;
}

/* =========================================================================
 * Test Fixture
 * ========================================================================= */

class ThreadModuleTest : public ::testing::Test
{
protected:
    JUNO_THREAD_ROOT_T m_tRoot;

    void SetUp() override
    {
        /* reset all double state */
        memset(&m_tRoot, 0, sizeof(m_tRoot));

        s_bStopCalled        = false;
        s_bJoinCalled        = false;
        s_bFreeCalled        = false;
        s_bStopFail          = false;
        s_bJoinFail          = false;
        s_bFreeFail          = false;
        s_tStopFailStatus    = JUNO_STATUS_ERR;
        s_tJoinFailStatus    = JUNO_STATUS_ERR;
        s_tFreeFailStatus    = JUNO_STATUS_ERR;
        s_iFhCallCount       = 0;
    }

    void TearDown() override {}
};

/* =========================================================================
 * Test Cases: Initialization — REQ-THREAD-012
 * ========================================================================= */

// @{"verify": ["REQ-THREAD-012"]}
TEST_F(ThreadModuleTest, Init_NullRoot_ReturnsNullptrError)
{
    JUNO_STATUS_T eStatus = JunoThread_Init(NULL, &s_tTestApi, NULL, NULL);
    EXPECT_EQ(JUNO_STATUS_NULLPTR_ERROR, eStatus);
}

// @{"verify": ["REQ-THREAD-012"]}
TEST_F(ThreadModuleTest, Init_NullApi_ReturnsNullptrError)
{
    JUNO_STATUS_T eStatus = JunoThread_Init(&m_tRoot, NULL, NULL, NULL);
    EXPECT_EQ(JUNO_STATUS_NULLPTR_ERROR, eStatus);
}

// @{"verify": ["REQ-THREAD-012"]}
TEST_F(ThreadModuleTest, Init_HappyPath_WiresVtableAndClearsState)
{
    JUNO_STATUS_T eStatus = JunoThread_Init(&m_tRoot, &s_tTestApi,
                                            TestFailureHandler, NULL);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    /* vtable must be wired */
    EXPECT_EQ(&s_tTestApi, m_tRoot.ptApi);
    /* stop flag must be clear */
    EXPECT_FALSE(m_tRoot.bStop);
}

// @{"verify": ["REQ-THREAD-012"]}
TEST_F(ThreadModuleTest, Init_NullFailureHandler_IsAccepted)
{
    /* A NULL failure handler is optional — init must succeed */
    JUNO_STATUS_T eStatus = JunoThread_Init(&m_tRoot, &s_tTestApi,
                                            NULL, NULL);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    EXPECT_EQ(&s_tTestApi, m_tRoot.ptApi);
}

// @{"verify": ["REQ-THREAD-013"]}
TEST_F(ThreadModuleTest, Init_StoresFailureHandler)
{
    JUNO_STATUS_T eStatus = JunoThread_Init(&m_tRoot, &s_tTestApi,
                                            TestFailureHandler, NULL);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    /* failure handler pointer must be stored in the root */
    EXPECT_EQ(reinterpret_cast<JUNO_FAILURE_HANDLER_T>(TestFailureHandler),
              m_tRoot._pfcnFailureHandler);
}

/* =========================================================================
 * Test Cases: Stop vtable dispatch — REQ-THREAD-006
 * ========================================================================= */

// @{"verify": ["REQ-THREAD-006"]}
TEST_F(ThreadModuleTest, Stop_DispatchesViaVtable_SetsBStopTrue)
{
    JunoThread_Init(&m_tRoot, &s_tTestApi, NULL, NULL);

    JUNO_STATUS_T eStatus = m_tRoot.ptApi->Stop(&m_tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    /* vtable Stop must have been called */
    EXPECT_TRUE(s_bStopCalled);
    /* cooperative shutdown flag must be set */
    EXPECT_TRUE(m_tRoot.bStop);
}

/* =========================================================================
 * Test Cases: Join vtable dispatch — REQ-THREAD-005
 * ========================================================================= */

// @{"verify": ["REQ-THREAD-005"]}
TEST_F(ThreadModuleTest, Join_DispatchesViaVtable)
{
    JunoThread_Init(&m_tRoot, &s_tTestApi, NULL, NULL);

    JUNO_STATUS_T eStatus = m_tRoot.ptApi->Join(&m_tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    /* vtable Join must have been called */
    EXPECT_TRUE(s_bJoinCalled);
}

/* =========================================================================
 * Test Cases: Free vtable dispatch — REQ-THREAD-009
 * ========================================================================= */

// @{"verify": ["REQ-THREAD-009"]}
TEST_F(ThreadModuleTest, Free_DispatchesViaVtable)
{
    JunoThread_Init(&m_tRoot, &s_tTestApi, NULL, NULL);
    JUNO_STATUS_T eStatus = m_tRoot.ptApi->Free(&m_tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    EXPECT_TRUE(s_bFreeCalled);
}

/* =========================================================================
 * Test Cases: Stop flag readable by thread entry — REQ-THREAD-007
 * ========================================================================= */

// @{"verify": ["REQ-THREAD-007"]}
TEST_F(ThreadModuleTest, BStop_ReadableAfterStop_TrueViaRootPointer)
{
    JunoThread_Init(&m_tRoot, &s_tTestApi, NULL, NULL);

    /* Before stop, flag must be false */
    EXPECT_FALSE(m_tRoot.bStop);

    m_tRoot.ptApi->Stop(&m_tRoot);

    /* After stop, a thread entry reading ptRoot->bStop must see true.
     * This is the same root pointer that would be passed as pvArg in real usage.
     * We verify via the same pointer cast a thread entry function would use. */
    const JUNO_THREAD_ROOT_T *ptRootAsEntry =
        reinterpret_cast<const JUNO_THREAD_ROOT_T *>(&m_tRoot);
    EXPECT_TRUE(ptRootAsEntry->bStop);
}

/* =========================================================================
 * Test Cases: Error Status Return — REQ-THREAD-010
 * ========================================================================= */

// @{"verify": ["REQ-THREAD-010"]}
TEST_F(ThreadModuleTest, Stop_WhenVtableReturnsError_PropagatesExactStatus)
{
    JunoThread_Init(&m_tRoot, &s_tTestApi, NULL, NULL);

    s_bStopFail       = true;
    s_tStopFailStatus = JUNO_STATUS_INVALID_REF_ERROR;

    JUNO_STATUS_T eStatus = m_tRoot.ptApi->Stop(&m_tRoot);
    EXPECT_EQ(JUNO_STATUS_INVALID_REF_ERROR, eStatus);
}

// @{"verify": ["REQ-THREAD-010"]}
TEST_F(ThreadModuleTest, Join_WhenVtableReturnsError_PropagatesExactStatus)
{
    JunoThread_Init(&m_tRoot, &s_tTestApi, NULL, NULL);

    s_bJoinFail       = true;
    s_tJoinFailStatus = JUNO_STATUS_INVALID_REF_ERROR;

    JUNO_STATUS_T eStatus = m_tRoot.ptApi->Join(&m_tRoot);
    EXPECT_EQ(JUNO_STATUS_INVALID_REF_ERROR, eStatus);
}

// @{"verify": ["REQ-THREAD-010"]}
TEST_F(ThreadModuleTest, Free_WhenVtableReturnsError_PropagatesExactStatus)
{
    JunoThread_Init(&m_tRoot, &s_tTestApi, NULL, NULL);
    s_bFreeFail         = true;
    s_tFreeFailStatus   = JUNO_STATUS_INVALID_REF_ERROR;
    JUNO_STATUS_T eStatus = m_tRoot.ptApi->Free(&m_tRoot);
    EXPECT_EQ(JUNO_STATUS_INVALID_REF_ERROR, eStatus);
}

/* =========================================================================
 * Test Cases: Full lifecycle — REQ-THREAD-005, REQ-THREAD-006, REQ-THREAD-009
 * ========================================================================= */

// @{"verify": ["REQ-THREAD-005", "REQ-THREAD-006", "REQ-THREAD-009"]}
TEST_F(ThreadModuleTest, FullLifecycle_StopJoinFree_StateConsistent)
{
    JunoThread_Init(&m_tRoot, &s_tTestApi, TestFailureHandler, NULL);

    /* Stop */
    JUNO_STATUS_T eStop = m_tRoot.ptApi->Stop(&m_tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStop);
    EXPECT_TRUE(s_bStopCalled);
    EXPECT_TRUE(m_tRoot.bStop);

    /* Join */
    JUNO_STATUS_T eJoin = m_tRoot.ptApi->Join(&m_tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eJoin);
    EXPECT_TRUE(s_bJoinCalled);

    /* Free */
    JUNO_STATUS_T eFree = m_tRoot.ptApi->Free(&m_tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eFree);
    EXPECT_TRUE(s_bFreeCalled);

    /* Failure handler must NOT have been called on success path */
    EXPECT_EQ(0, s_iFhCallCount);
}
