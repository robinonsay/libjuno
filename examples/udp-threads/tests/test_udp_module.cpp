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
 * @file test_udp_module.cpp
 * @brief Google Test suite for the UDP module (JunoUdp_Init and vtable dispatch).
 *
 * @details
 *  Covers requirements: REQ-UDP-001, REQ-UDP-003, REQ-UDP-006, REQ-UDP-007,
 *  REQ-UDP-008, REQ-UDP-009, REQ-UDP-012, REQ-UDP-013, REQ-UDP-016.
 *
 *  Test doubles use a vtable-injected fake JUNO_UDP_API_T.  No POSIX socket
 *  API is called from this file.  No dynamic allocation is used anywhere in
 *  this file.
 */

#include <gtest/gtest.h>
#include <string.h>
#include <stdint.h>

#include "udp_api.h"
#include "juno/status.h"
#include "juno/module.h"

/* =========================================================================
 * Test Doubles — vtable-injected fake JUNO_UDP_API_T
 * ========================================================================= */

/* Shared state for the fake vtable functions. Reset in every SetUp(). */
static struct
{
    /* call counts */
    int iSendCallCount;
    int iReceiveCallCount;
    int iFreeCalled;

    /* injected failure flags */
    bool bFailSend;
    bool bFailReceive;
    bool bFailFree;

    /* injected statuses returned when the corresponding flag is set */
    JUNO_STATUS_T tSendStatus;
    JUNO_STATUS_T tReceiveStatus;
    JUNO_STATUS_T tFreeStatus;

    /* last arguments captured by each function */
    const UDP_THREAD_MSG_T *ptLastSendMsg;
    UDP_THREAD_MSG_T       *ptLastReceiveMsg;

    /* payload written back by TestReceive (when not failing) */
    UDP_THREAD_MSG_T tReceivePayload;

} s_tFakeState;

/* ---------- TestSend ---------------------------------------------------- */
static JUNO_STATUS_T TestSend(JUNO_UDP_ROOT_T *ptRoot, const UDP_THREAD_MSG_T *ptMsg)
{
    s_tFakeState.iSendCallCount++;
    s_tFakeState.ptLastSendMsg = ptMsg;
    (void)ptRoot;

    if (s_tFakeState.bFailSend)
    {
        return s_tFakeState.tSendStatus;
    }

    return JUNO_STATUS_SUCCESS;
}

/* ---------- TestReceive ------------------------------------------------- */
static JUNO_STATUS_T TestReceive(JUNO_UDP_ROOT_T *ptRoot, UDP_THREAD_MSG_T *ptMsg)
{
    s_tFakeState.iReceiveCallCount++;
    s_tFakeState.ptLastReceiveMsg = ptMsg;
    (void)ptRoot;

    if (s_tFakeState.bFailReceive)
    {
        return s_tFakeState.tReceiveStatus;
    }

    /* Write the pre-configured payload into the output buffer. */
    if (ptMsg != NULL)
    {
        *ptMsg = s_tFakeState.tReceivePayload;
    }

    return JUNO_STATUS_SUCCESS;
}

/* ---------- TestFree ---------------------------------------------------- */
static JUNO_STATUS_T TestFree(JUNO_UDP_ROOT_T *ptRoot)
{
    s_tFakeState.iFreeCalled++;
    (void)ptRoot;
    if (s_tFakeState.bFailFree) { return s_tFakeState.tFreeStatus; }
    return JUNO_STATUS_SUCCESS;
}

/* Statically allocated fake vtable (Send, Receive, Free). */
static const JUNO_UDP_API_T s_tTestApi = {
    TestSend,
    TestReceive,
    TestFree
};

/* =========================================================================
 * Failure Handler Double
 * ========================================================================= */

static struct
{
    int          iCallCount;
    JUNO_STATUS_T tLastStatus;
    void         *pvLastUserData;
} s_tFhDouble;

static void TestFailureHandler(JUNO_STATUS_T tStatus,
                               const char   *pcMsg,
                               void         *pvUserData)
{
    (void)pcMsg;
    s_tFhDouble.iCallCount++;
    s_tFhDouble.tLastStatus   = tStatus;
    s_tFhDouble.pvLastUserData = pvUserData;
}

/* =========================================================================
 * Helper — reset all shared state
 * ========================================================================= */

static void ResetAllDoubles(void)
{
    memset(&s_tFakeState, 0, sizeof(s_tFakeState));
    memset(&s_tFhDouble,  0, sizeof(s_tFhDouble));
}

/* =========================================================================
 * Test Fixture
 * ========================================================================= */

class UdpModuleTest : public ::testing::Test
{
protected:
    JUNO_UDP_ROOT_T tUdp;

    void SetUp() override
    {
        ResetAllDoubles();
        memset(&tUdp, 0, sizeof(tUdp));
    }

    /* Convenience: initialise the root with the test double vtable and handler. */
    JUNO_STATUS_T InitWithDouble(void *pvUserData = NULL)
    {
        return JunoUdp_Init(
            &tUdp,
            &s_tTestApi,
            TestFailureHandler,
            pvUserData);
    }
};

/* =========================================================================
 * Test Cases: Module Initialization (REQ-UDP-016)
 * ========================================================================= */

// @{"verify": ["REQ-UDP-016"]}
TEST_F(UdpModuleTest, InitHappyPathWiresVtable)
{
    JUNO_STATUS_T eStatus = JunoUdp_Init(
        &tUdp, &s_tTestApi, TestFailureHandler, NULL);

    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    /* Vtable must be wired — the pointer must equal the injected vtable. */
    EXPECT_EQ(&s_tTestApi, tUdp.ptApi);
    /* Failure handler must not have been invoked on a success path. */
    EXPECT_EQ(0, s_tFhDouble.iCallCount);
}

// @{"verify": ["REQ-UDP-016", "REQ-UDP-012", "REQ-UDP-013"]}
TEST_F(UdpModuleTest, InitNullRootReturnsNullptrError)
{
    JUNO_STATUS_T eStatus = JunoUdp_Init(
        NULL, &s_tTestApi, TestFailureHandler, NULL);

    /* Must return the exact null-pointer error code. */
    EXPECT_EQ(JUNO_STATUS_NULLPTR_ERROR, eStatus);
}

// @{"verify": ["REQ-UDP-016", "REQ-UDP-012", "REQ-UDP-013"]}
TEST_F(UdpModuleTest, InitNullVtableReturnsNullptrError)
{
    JUNO_STATUS_T eStatus = JunoUdp_Init(
        &tUdp, NULL, TestFailureHandler, NULL);

    /* Must return the exact null-pointer error code. */
    EXPECT_EQ(JUNO_STATUS_NULLPTR_ERROR, eStatus);
    /* Vtable must NOT have been wired (root was zeroed in SetUp). */
    EXPECT_EQ(static_cast<const JUNO_UDP_API_T *>(NULL), tUdp.ptApi);
}

// @{"verify": ["REQ-UDP-016"]}
TEST_F(UdpModuleTest, InitStoresFailureHandlerAndUserData)
{
    int iUserData = 42;

    JUNO_STATUS_T eStatus = JunoUdp_Init(
        &tUdp, &s_tTestApi, TestFailureHandler, &iUserData);

    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    /* Failure handler function pointer must be stored. */
    EXPECT_EQ(reinterpret_cast<JUNO_FAILURE_HANDLER_T>(TestFailureHandler),
              tUdp.JUNO_FAILURE_HANDLER);
    /* User-data pointer must be stored. */
    EXPECT_EQ(reinterpret_cast<void *>(&iUserData),
              static_cast<void *>(tUdp.JUNO_FAILURE_USER_DATA));
}

// @{"verify": ["REQ-UDP-016"]}
TEST_F(UdpModuleTest, InitNullHandlerAndNullUserDataArePermitted)
{
    /* Both optional parameters may be NULL — Init must still succeed. */
    JUNO_STATUS_T eStatus = JunoUdp_Init(
        &tUdp, &s_tTestApi, NULL, NULL);

    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    EXPECT_EQ(&s_tTestApi, tUdp.ptApi);
}

/* =========================================================================
 * Test Cases: Root Struct Layout (REQ-UDP-001)
 * ========================================================================= */

// @{"verify": ["REQ-UDP-001"]}
TEST_F(UdpModuleTest, RootHoldsPtApiField)
{
    /* ptApi is the canonical vtable pointer mandated by REQ-UDP-001. */
    InitWithDouble();
    EXPECT_EQ(&s_tTestApi, tUdp.ptApi);
}

// @{"verify": ["REQ-UDP-001"]}
TEST_F(UdpModuleTest, RootHoldsFailureHandlerField)
{
    /* JUNO_FAILURE_HANDLER (expands to _pfcnFailureHandler) must be stored. */
    InitWithDouble();
    EXPECT_EQ(reinterpret_cast<JUNO_FAILURE_HANDLER_T>(TestFailureHandler),
              tUdp.JUNO_FAILURE_HANDLER);
}

/* =========================================================================
 * Test Cases: All API operations return JUNO_STATUS_T (REQ-UDP-012)
 * ========================================================================= */

// @{"verify": ["REQ-UDP-012"]}
TEST_F(UdpModuleTest, AllApiOperationsReturnStatusT)
{
    /*
     * This test exercises all three vtable operations and verifies each returns
     * a JUNO_STATUS_T comparable to JUNO_STATUS_SUCCESS.  The compile-time
     * guarantee (function pointer type in JUNO_UDP_API_T) is reinforced here
     * at run time.
     */
    InitWithDouble();

    UDP_THREAD_MSG_T tMsg;
    memset(&tMsg, 0, sizeof(tMsg));

    JUNO_STATUS_T eSend    = tUdp.ptApi->Send(&tUdp, &tMsg);
    UDP_THREAD_MSG_T tOut;
    memset(&tOut, 0, sizeof(tOut));
    JUNO_STATUS_T eReceive = tUdp.ptApi->Receive(&tUdp, &tOut);
    JUNO_STATUS_T eFree    = tUdp.ptApi->Free(&tUdp);

    EXPECT_EQ(JUNO_STATUS_SUCCESS, eSend);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eReceive);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eFree);

    /* Each operation must have been dispatched exactly once. */
    EXPECT_EQ(1, s_tFakeState.iSendCallCount);
    EXPECT_EQ(1, s_tFakeState.iReceiveCallCount);
    EXPECT_EQ(1, s_tFakeState.iFreeCalled);
}

/* =========================================================================
 * Test Cases: Failure Handler Invocation on Error (REQ-UDP-013)
 * ========================================================================= */

// @{"verify": ["REQ-UDP-013"]}
TEST_F(UdpModuleTest, FailureHandlerFiredViaJunoFailRoot)
{
    /*
     * We verify the mechanism by manually calling JUNO_FAIL_ROOT as the caller
     * would, confirming the stored handler fires.
     *
     * NOTE: JunoUdp_Init null-guard uses JUNO_ASSERT_EXISTS which does NOT
     * call the failure handler (no module instance is available for the ptRoot
     * null case). For the ptApi null case we have a valid ptRoot with no handler
     * yet, so the handler is also not called by JUNO_ASSERT_EXISTS.
     * We therefore test the handler via JUNO_FAIL_ROOT directly to confirm
     * the stored handler pointer is correctly threaded.
     */
    InitWithDouble();

    /* JUNO_FAIL_ROOT invokes the handler stored in the root. */
    JUNO_FAIL_ROOT(JUNO_STATUS_ERR, (&tUdp), "unit test injected error");

    EXPECT_EQ(1, s_tFhDouble.iCallCount);
    EXPECT_EQ(JUNO_STATUS_ERR, s_tFhDouble.tLastStatus);
}

// @{"verify": ["REQ-UDP-013"]}
TEST_F(UdpModuleTest, FailureHandlerReceivesCorrectUserData)
{
    int iUserData = 99;
    JunoUdp_Init(&tUdp, &s_tTestApi, TestFailureHandler, &iUserData);

    /* Fire the handler through the module's stored pointer. */
    JUNO_FAIL_ROOT(JUNO_STATUS_WRITE_ERROR, (&tUdp), "send failure");

    EXPECT_EQ(1, s_tFhDouble.iCallCount);
    EXPECT_EQ(JUNO_STATUS_WRITE_ERROR, s_tFhDouble.tLastStatus);
    /* User data must be the exact pointer passed to Init. */
    EXPECT_EQ(reinterpret_cast<void *>(&iUserData), s_tFhDouble.pvLastUserData);
}

// @{"verify": ["REQ-UDP-013"]}
TEST_F(UdpModuleTest, FailureHandlerNotInvokedOnSuccess)
{
    InitWithDouble();

    UDP_THREAD_MSG_T tMsg;
    memset(&tMsg, 0, sizeof(tMsg));

    tUdp.ptApi->Send(&tUdp, &tMsg);
    UDP_THREAD_MSG_T tOut;
    memset(&tOut, 0, sizeof(tOut));
    tUdp.ptApi->Receive(&tUdp, &tOut);
    tUdp.ptApi->Free(&tUdp);

    /* All operations succeeded — handler must never have been called. */
    EXPECT_EQ(0, s_tFhDouble.iCallCount);
}

/* =========================================================================
 * Test Cases: Send dispatched through vtable (REQ-UDP-006)
 * ========================================================================= */

// @{"verify": ["REQ-UDP-006", "REQ-UDP-015"]}
TEST_F(UdpModuleTest, SendDispatchesViaVtable)
{
    InitWithDouble();

    UDP_THREAD_MSG_T tMsg;
    memset(&tMsg, 0, sizeof(tMsg));
    tMsg.uSeqNum          = 7U;
    tMsg.uTimestampSec    = 100U;
    tMsg.uTimestampSubSec = 500U;
    memset(tMsg.arrPayload, 0xAB, sizeof(tMsg.arrPayload));

    JUNO_STATUS_T eStatus = tUdp.ptApi->Send(&tUdp, &tMsg);

    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    /* Vtable dispatched exactly once. */
    EXPECT_EQ(1, s_tFakeState.iSendCallCount);
    /* Message pointer forwarded to the implementation. */
    EXPECT_EQ(&tMsg, s_tFakeState.ptLastSendMsg);
    /* Verify the exact message contents were forwarded (not zero or different). */
    EXPECT_EQ(7U, s_tFakeState.ptLastSendMsg->uSeqNum);
    EXPECT_EQ(static_cast<uint8_t>(0xAB), s_tFakeState.ptLastSendMsg->arrPayload[0]);
    EXPECT_EQ(static_cast<uint8_t>(0xAB), s_tFakeState.ptLastSendMsg->arrPayload[63]);
}

// @{"verify": ["REQ-UDP-006", "REQ-UDP-012"]}
TEST_F(UdpModuleTest, SendInjectedFailureReturnsExactStatus)
{
    InitWithDouble();

    s_tFakeState.bFailSend   = true;
    s_tFakeState.tSendStatus = JUNO_STATUS_WRITE_ERROR;

    UDP_THREAD_MSG_T tMsg;
    memset(&tMsg, 0, sizeof(tMsg));

    JUNO_STATUS_T eStatus = tUdp.ptApi->Send(&tUdp, &tMsg);

    EXPECT_EQ(JUNO_STATUS_WRITE_ERROR, eStatus);
    EXPECT_EQ(1, s_tFakeState.iSendCallCount);
}

/* =========================================================================
 * Test Cases: Receive dispatched through vtable (REQ-UDP-007)
 * ========================================================================= */

// @{"verify": ["REQ-UDP-007", "REQ-UDP-015"]}
TEST_F(UdpModuleTest, ReceiveDispatchesViaVtableAndPopulatesOutput)
{
    InitWithDouble();

    /* Pre-configure the receive payload that the fake will write. */
    s_tFakeState.tReceivePayload.uSeqNum          = 99U;
    s_tFakeState.tReceivePayload.uTimestampSec    = 200U;
    s_tFakeState.tReceivePayload.uTimestampSubSec = 0U;
    memset(s_tFakeState.tReceivePayload.arrPayload, 0xCD,
           sizeof(s_tFakeState.tReceivePayload.arrPayload));

    UDP_THREAD_MSG_T tOut;
    memset(&tOut, 0, sizeof(tOut));

    JUNO_STATUS_T eStatus = tUdp.ptApi->Receive(&tUdp, &tOut);

    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    /* Vtable dispatched exactly once. */
    EXPECT_EQ(1, s_tFakeState.iReceiveCallCount);
    /* Output buffer must have been written with the payload configured in the fake. */
    EXPECT_EQ(99U, tOut.uSeqNum);
    EXPECT_EQ(200U, tOut.uTimestampSec);
    EXPECT_EQ(0U, tOut.uTimestampSubSec);
    EXPECT_EQ(static_cast<uint8_t>(0xCD), tOut.arrPayload[0]);
    EXPECT_EQ(static_cast<uint8_t>(0xCD), tOut.arrPayload[63]);
    /* Failure handler must not have been called on success. */
    EXPECT_EQ(0, s_tFhDouble.iCallCount);
}

// @{"verify": ["REQ-UDP-007", "REQ-UDP-012"]}
TEST_F(UdpModuleTest, ReceiveInjectedFailureReturnsExactStatus)
{
    InitWithDouble();

    s_tFakeState.bFailReceive    = true;
    s_tFakeState.tReceiveStatus  = JUNO_STATUS_READ_ERROR;

    UDP_THREAD_MSG_T tOut;
    memset(&tOut, 0, sizeof(tOut));

    JUNO_STATUS_T eStatus = tUdp.ptApi->Receive(&tUdp, &tOut);

    EXPECT_EQ(JUNO_STATUS_READ_ERROR, eStatus);
    EXPECT_EQ(1, s_tFakeState.iReceiveCallCount);
}

/* =========================================================================
 * Test Cases: Timeout status from Receive (REQ-UDP-008)
 * ========================================================================= */

// @{"verify": ["REQ-UDP-008"]}
TEST_F(UdpModuleTest, ReceiveTimeoutReturnsTimeoutError)
{
    InitWithDouble();

    /* Inject JUNO_STATUS_TIMEOUT_ERROR to simulate a receive timeout. */
    s_tFakeState.bFailReceive   = true;
    s_tFakeState.tReceiveStatus = JUNO_STATUS_TIMEOUT_ERROR;

    UDP_THREAD_MSG_T tOut;
    memset(&tOut, 0, sizeof(tOut));

    JUNO_STATUS_T eStatus = tUdp.ptApi->Receive(&tUdp, &tOut);

    /* Must return exactly JUNO_STATUS_TIMEOUT_ERROR, not a generic error. */
    EXPECT_EQ(JUNO_STATUS_TIMEOUT_ERROR, eStatus);
    EXPECT_EQ(1, s_tFakeState.iReceiveCallCount);

    /* Output buffer must be unchanged when a timeout occurs (no partial write). */
    const UDP_THREAD_MSG_T tZero = { 0U, 0U, 0U, { 0 } };
    EXPECT_EQ(0, memcmp(&tOut, &tZero, sizeof(tOut)));
}

// @{"verify": ["REQ-UDP-008"]}
TEST_F(UdpModuleTest, ReceiveTimeoutStatusCodeIsDistinctFromSuccess)
{
    /*
     * Documents that JUNO_STATUS_TIMEOUT_ERROR != JUNO_STATUS_SUCCESS,
     * so callers can distinguish a timeout from a successful receive.
     */
    EXPECT_NE(JUNO_STATUS_SUCCESS, JUNO_STATUS_TIMEOUT_ERROR);
    /* Also distinct from generic error. */
    EXPECT_NE(JUNO_STATUS_ERR, JUNO_STATUS_TIMEOUT_ERROR);
}

/* =========================================================================
 * Test Cases: Free dispatched through vtable (REQ-UDP-009)
 * ========================================================================= */

// @{"verify": ["REQ-UDP-009"]}
TEST_F(UdpModuleTest, FreeDispatchesViaVtable)
{
    InitWithDouble();

    JUNO_STATUS_T eStatus = tUdp.ptApi->Free(&tUdp);

    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    /* Vtable dispatched exactly once. */
    EXPECT_EQ(1, s_tFakeState.iFreeCalled);
}

// @{"verify": ["REQ-UDP-009", "REQ-UDP-012"]}
TEST_F(UdpModuleTest, FreeInjectedFailureReturnsExactStatus)
{
    InitWithDouble();

    s_tFakeState.bFailFree    = true;
    s_tFakeState.tFreeStatus  = JUNO_STATUS_INVALID_REF_ERROR;

    JUNO_STATUS_T eStatus = tUdp.ptApi->Free(&tUdp);

    EXPECT_EQ(JUNO_STATUS_INVALID_REF_ERROR, eStatus);
    EXPECT_EQ(1, s_tFakeState.iFreeCalled);
}

/* =========================================================================
 * Test Cases: Message Struct Size (REQ-UDP-015)
 * ========================================================================= */

// @{"verify": ["REQ-UDP-006", "REQ-UDP-007"]}
TEST_F(UdpModuleTest, MessageStructSizeIs76Bytes)
{
    /*
     * Documents and guards the fixed 76-byte datagram size:
     *   3 × uint32_t (12 bytes) + 64-byte payload = 76 bytes.
     * A size mismatch would silently truncate or pad datagrams.
     */
    EXPECT_EQ(static_cast<size_t>(76), sizeof(UDP_THREAD_MSG_T));
}
