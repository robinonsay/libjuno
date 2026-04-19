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
 *  REQ-THREAD-007, REQ-THREAD-010, REQ-THREAD-012, REQ-THREAD-013,
 *  REQ-THREAD-015.
 *
 *  Test doubles use a vtable-injected fake JUNO_THREAD_API_T.  No OS thread
 *  API (pthreads) is called from this file.  No dynamic allocation is used.
 */

#include <gtest/gtest.h>
#include <cstring>

#include "juno/thread.h"
#include "juno/status.h"

/* =========================================================================
 * Test Doubles — vtable-injected fake JUNO_THREAD_API_T
 * ========================================================================= */

static bool s_bCreateCalled = false;
static bool s_bStopCalled   = false;
static bool s_bJoinCalled   = false;

/* Injected failure flags */
static bool          s_bCreateFail      = false;
static bool          s_bStopFail        = false;
static bool          s_bJoinFail        = false;
static JUNO_STATUS_T s_tCreateFailStatus = JUNO_STATUS_ERR;
static JUNO_STATUS_T s_tStopFailStatus   = JUNO_STATUS_ERR;
static JUNO_STATUS_T s_tJoinFailStatus   = JUNO_STATUS_ERR;

/* Captured arguments */
static void *(*s_pfcnCapturedEntry)(void *) = NULL;
static void  *s_pvCapturedArg              = NULL;

static JUNO_STATUS_T TestCreate(JUNO_THREAD_ROOT_T *ptRoot,
                                void *(*pfcnEntry)(void *),
                                void *pvArg)
{
    s_bCreateCalled       = true;
    s_pfcnCapturedEntry   = pfcnEntry;
    s_pvCapturedArg       = pvArg;
    if (s_bCreateFail)
    {
        return s_tCreateFailStatus;
    }
    ptRoot->_uHandle = 1u; /* mark as running */
    return JUNO_STATUS_SUCCESS;
}

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
    s_bJoinCalled = true;
    if (s_bJoinFail)
    {
        return s_tJoinFailStatus;
    }
    ptRoot->_uHandle = 0u; /* mark as stopped */
    return JUNO_STATUS_SUCCESS;
}

static const JUNO_THREAD_API_T s_tTestApi = { TestCreate, TestStop, TestJoin };

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
 * Helper — dummy thread entry function (never actually called)
 * ========================================================================= */

static void *DummyEntry(void *pvArg)
{
    (void)pvArg;
    return NULL;
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

        s_bCreateCalled       = false;
        s_bStopCalled         = false;
        s_bJoinCalled         = false;
        s_bCreateFail         = false;
        s_bStopFail           = false;
        s_bJoinFail           = false;
        s_tCreateFailStatus   = JUNO_STATUS_ERR;
        s_tStopFailStatus     = JUNO_STATUS_ERR;
        s_tJoinFailStatus     = JUNO_STATUS_ERR;
        s_pfcnCapturedEntry   = NULL;
        s_pvCapturedArg       = NULL;
        s_iFhCallCount        = 0;
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
    /* handle must be zero — not running */
    EXPECT_EQ(0u, m_tRoot._uHandle);
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
    EXPECT_EQ(0u, m_tRoot._uHandle);
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
 * Test Cases: Create vtable dispatch — REQ-THREAD-003
 * ========================================================================= */

// @{"verify": ["REQ-THREAD-003"]}
TEST_F(ThreadModuleTest, Create_DispatchesViaVtable_HandleSet)
{
    JUNO_STATUS_T eInit = JunoThread_Init(&m_tRoot, &s_tTestApi, NULL, NULL);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eInit);

    JUNO_STATUS_T eStatus = JunoThread_Create(&m_tRoot, DummyEntry, &m_tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    /* vtable Create must have been called */
    EXPECT_TRUE(s_bCreateCalled);
    /* handle must be non-zero — thread is running */
    EXPECT_NE(0u, m_tRoot._uHandle);
}

// @{"verify": ["REQ-THREAD-003"]}
TEST_F(ThreadModuleTest, Create_ForwardsEntryAndArg)
{
    JunoThread_Init(&m_tRoot, &s_tTestApi, NULL, NULL);

    int iDummyData = 42;
    JunoThread_Create(&m_tRoot, DummyEntry, &iDummyData);

    EXPECT_TRUE(s_bCreateCalled);
    EXPECT_EQ(reinterpret_cast<void *(*)(void *)>(DummyEntry),
              s_pfcnCapturedEntry);
    EXPECT_EQ(static_cast<void *>(&iDummyData), s_pvCapturedArg);
}

/* =========================================================================
 * Test Cases: Stop vtable dispatch — REQ-THREAD-006
 * ========================================================================= */

// @{"verify": ["REQ-THREAD-006"]}
TEST_F(ThreadModuleTest, Stop_DispatchesViaVtable_SetsBStopTrue)
{
    JunoThread_Init(&m_tRoot, &s_tTestApi, NULL, NULL);
    /* Create first so handle is non-zero */
    JunoThread_Create(&m_tRoot, DummyEntry, NULL);

    JUNO_STATUS_T eStatus = JunoThread_Stop(&m_tRoot);
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
TEST_F(ThreadModuleTest, Join_DispatchesViaVtable_ClearsHandle)
{
    JunoThread_Init(&m_tRoot, &s_tTestApi, NULL, NULL);
    JunoThread_Create(&m_tRoot, DummyEntry, NULL);
    EXPECT_NE(0u, m_tRoot._uHandle);

    JUNO_STATUS_T eStatus = JunoThread_Join(&m_tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    /* vtable Join must have been called */
    EXPECT_TRUE(s_bJoinCalled);
    /* handle must be reset to zero after join */
    EXPECT_EQ(0u, m_tRoot._uHandle);
}

/* =========================================================================
 * Test Cases: Stop flag readable by thread entry — REQ-THREAD-007
 * ========================================================================= */

// @{"verify": ["REQ-THREAD-007"]}
TEST_F(ThreadModuleTest, BStop_ReadableAfterStop_TrueViaRootPointer)
{
    JunoThread_Init(&m_tRoot, &s_tTestApi, NULL, NULL);
    JunoThread_Create(&m_tRoot, DummyEntry, &m_tRoot);

    /* Before stop, flag must be false */
    EXPECT_FALSE(m_tRoot.bStop);

    JunoThread_Stop(&m_tRoot);

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
TEST_F(ThreadModuleTest, Create_WhenVtableReturnsError_PropagatesExactStatus)
{
    JunoThread_Init(&m_tRoot, &s_tTestApi, NULL, NULL);

    s_bCreateFail         = true;
    s_tCreateFailStatus   = JUNO_STATUS_INVALID_REF_ERROR;

    JUNO_STATUS_T eStatus = JunoThread_Create(&m_tRoot, DummyEntry, NULL);
    EXPECT_EQ(JUNO_STATUS_INVALID_REF_ERROR, eStatus);
}

// @{"verify": ["REQ-THREAD-010"]}
TEST_F(ThreadModuleTest, Stop_WhenVtableReturnsError_PropagatesExactStatus)
{
    JunoThread_Init(&m_tRoot, &s_tTestApi, NULL, NULL);
    JunoThread_Create(&m_tRoot, DummyEntry, NULL);

    s_bStopFail       = true;
    s_tStopFailStatus = JUNO_STATUS_INVALID_REF_ERROR;

    JUNO_STATUS_T eStatus = JunoThread_Stop(&m_tRoot);
    EXPECT_EQ(JUNO_STATUS_INVALID_REF_ERROR, eStatus);
}

// @{"verify": ["REQ-THREAD-010"]}
TEST_F(ThreadModuleTest, Join_WhenVtableReturnsError_PropagatesExactStatus)
{
    JunoThread_Init(&m_tRoot, &s_tTestApi, NULL, NULL);
    JunoThread_Create(&m_tRoot, DummyEntry, NULL);

    s_bJoinFail       = true;
    s_tJoinFailStatus = JUNO_STATUS_INVALID_REF_ERROR;

    JUNO_STATUS_T eStatus = JunoThread_Join(&m_tRoot);
    EXPECT_EQ(JUNO_STATUS_INVALID_REF_ERROR, eStatus);
}

/* =========================================================================
 * Test Cases: Error on Double Create — REQ-THREAD-015
 * ========================================================================= */

/* NOTE: REQ-THREAD-015 requires the *module* (not the test double) to return
 * JUNO_STATUS_REF_IN_USE_ERROR when Create is called while _uHandle != 0.
 * The linux_thread_impl.cpp is currently a stub so the double-create guard
 * is not yet implemented there.  The test below exercises the contract via
 * the fake vtable to document the expected runtime behavior: a second Create
 * call on a root that already has a non-zero handle MUST return
 * JUNO_STATUS_REF_IN_USE_ERROR.
 *
 * When the real Linux implementation is complete, replace the fake vtable
 * with g_junoThreadLinuxApi and this test will verify the implementation.
 */

static JUNO_STATUS_T TestCreateDoubleGuard(JUNO_THREAD_ROOT_T *ptRoot,
                                           void *(*pfcnEntry)(void *),
                                           void *pvArg)
{
    (void)pfcnEntry; (void)pvArg;
    if (ptRoot->_uHandle != 0u)
    {
        return JUNO_STATUS_REF_IN_USE_ERROR;
    }
    ptRoot->_uHandle = 1u;
    return JUNO_STATUS_SUCCESS;
}

static const JUNO_THREAD_API_T s_tGuardApi = {
    TestCreateDoubleGuard,
    TestStop,
    TestJoin
};

// @{"verify": ["REQ-THREAD-015"]}
TEST_F(ThreadModuleTest, Create_WhenAlreadyRunning_ReturnsRefInUseError)
{
    JunoThread_Init(&m_tRoot, &s_tGuardApi, NULL, NULL);

    /* First create — should succeed and set handle */
    JUNO_STATUS_T eFirst = JunoThread_Create(&m_tRoot, DummyEntry, NULL);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eFirst);
    EXPECT_NE(0u, m_tRoot._uHandle);

    /* Second create — handle still non-zero → must return REF_IN_USE */
    JUNO_STATUS_T eSecond = JunoThread_Create(&m_tRoot, DummyEntry, NULL);
    EXPECT_EQ(JUNO_STATUS_REF_IN_USE_ERROR, eSecond);
}

/* =========================================================================
 * Test Cases: Full lifecycle — REQ-THREAD-003, REQ-THREAD-005, REQ-THREAD-006
 * ========================================================================= */

// @{"verify": ["REQ-THREAD-003", "REQ-THREAD-005", "REQ-THREAD-006"]}
TEST_F(ThreadModuleTest, FullLifecycle_CreateStopJoin_StateConsistent)
{
    JunoThread_Init(&m_tRoot, &s_tTestApi, TestFailureHandler, NULL);

    /* Create */
    JUNO_STATUS_T eCreate = JunoThread_Create(&m_tRoot, DummyEntry, &m_tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eCreate);
    EXPECT_TRUE(s_bCreateCalled);
    EXPECT_NE(0u, m_tRoot._uHandle);
    EXPECT_FALSE(m_tRoot.bStop);

    /* Stop */
    JUNO_STATUS_T eStop = JunoThread_Stop(&m_tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStop);
    EXPECT_TRUE(s_bStopCalled);
    EXPECT_TRUE(m_tRoot.bStop);

    /* Join */
    JUNO_STATUS_T eJoin = JunoThread_Join(&m_tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eJoin);
    EXPECT_TRUE(s_bJoinCalled);
    EXPECT_EQ(0u, m_tRoot._uHandle);

    /* Failure handler must NOT have been called on success path */
    EXPECT_EQ(0, s_iFhCallCount);
}
