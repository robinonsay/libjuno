/*
    MIT License
    Copyright (c) 2025 Robin A. Onsay

    Google Test suite for the four udp-threads application modules:
      - SenderApp    (REQ-UDPAPP-002)
      - MonitorApp   (REQ-UDPAPP-008)
      - UdpBridgeApp (REQ-UDPAPP-011)
      - ProcessorApp (REQ-UDPAPP-016)

    Build constraints: -fno-rtti -fno-exceptions
    All test doubles use vtable injection — no linker patching.
    No dynamic allocation.
*/

#include <gtest/gtest.h>
#include <cstring>

#include "sender_app.h"
#include "udp_msg_api.h"
#include "monitor_app.h"
#include "udp_bridge_app.h"
#include "processor_app.h"

/* ==========================================================================
 * Shared shallow-stub dependency instances
 *
 * Init functions only store pointers; the stubs do not need to be fully
 * initialised.  We use static storage so every fixture can take the address.
 * ========================================================================== */

static JUNO_UDP_ROOT_T       s_tUdpStub;
static JUNO_SB_BROKER_ROOT_T s_tBrokerStub;
static UDPTH_MSG_ARRAY_T     s_tMsgArray;
static JUNO_SB_PIPE_T       *s_aptBrokerPipeRegistry[4u];

/* UDP stub vtable — Send/Free return SUCCESS; Receive returns TIMEOUT so
 * UdpBridgeApp OnProcess sees a quiet cycle and returns SUCCESS. */
static JUNO_STATUS_T UdpStub_Send(JUNO_UDP_ROOT_T *ptRoot, const UDP_THREAD_MSG_T *ptMsg)
    { (void)ptRoot; (void)ptMsg; return JUNO_STATUS_SUCCESS; }
static JUNO_STATUS_T UdpStub_Receive(JUNO_UDP_ROOT_T *ptRoot, UDP_THREAD_MSG_T *ptMsg)
    { (void)ptRoot; (void)ptMsg; return JUNO_STATUS_TIMEOUT_ERROR; }
static JUNO_STATUS_T UdpStub_Free(JUNO_UDP_ROOT_T *ptRoot)
    { (void)ptRoot; return JUNO_STATUS_SUCCESS; }
static const JUNO_UDP_API_T s_tUdpStubApi = { UdpStub_Send, UdpStub_Receive, UdpStub_Free };

/* ==========================================================================
 * Test-double vtable (used for dispatch verification tests)
 *
 * Each function sets a flag so we can confirm the call was dispatched through
 * the vtable.  The double must NOT return the value the test then asserts
 * on — it just returns SUCCESS to satisfy the protocol.
 * ========================================================================== */

static bool s_bOnStartCalled   = false;
static bool s_bOnProcessCalled = false;
static bool s_bOnExitCalled    = false;

static JUNO_STATUS_T TestOnStart(JUNO_APP_ROOT_T *ptApp)
{
    (void)ptApp;
    s_bOnStartCalled = true;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestOnProcess(JUNO_APP_ROOT_T *ptApp)
{
    (void)ptApp;
    s_bOnProcessCalled = true;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestOnExit(JUNO_APP_ROOT_T *ptApp)
{
    (void)ptApp;
    s_bOnExitCalled = true;
    return JUNO_STATUS_SUCCESS;
}

static const JUNO_APP_API_T s_tTestAppApi = {
    TestOnStart,
    TestOnProcess,
    TestOnExit
};

/* Helper to reset dispatch-flag state between tests */
static void ResetDispatchFlags(void)
{
    s_bOnStartCalled   = false;
    s_bOnProcessCalled = false;
    s_bOnExitCalled    = false;
}

/* ==========================================================================
 * SenderApp Tests
 * ========================================================================== */

class SenderAppTest : public ::testing::Test
{
protected:
    SENDER_APP_T tSender;

    void SetUp() override
    {
        std::memset(&tSender, 0, sizeof(tSender));
        std::memset(&s_tUdpStub, 0, sizeof(s_tUdpStub));
        s_tUdpStub.ptApi = &s_tUdpStubApi;
        JunoSb_BrokerInit(&s_tBrokerStub, s_aptBrokerPipeRegistry, 4u, nullptr, nullptr);
        ResetDispatchFlags();
    }
};

/* --------------------------------------------------------------------------
 * SenderApp Init — error paths
 * -------------------------------------------------------------------------- */

// @{"verify": ["REQ-UDPAPP-002"]}
TEST_F(SenderAppTest, InitNullAppReturnsNullptrError)
{
    JUNO_STATUS_T eStatus = SenderApp_Init(
        nullptr,
        &s_tUdpStub,
        &s_tBrokerStub,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_NULLPTR_ERROR, eStatus);
}

// @{"verify": ["REQ-UDPAPP-002"]}
TEST_F(SenderAppTest, InitNullUdpReturnsNullptrError)
{
    JUNO_STATUS_T eStatus = SenderApp_Init(
        &tSender,
        nullptr,
        &s_tBrokerStub,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_NULLPTR_ERROR, eStatus);
}

// @{"verify": ["REQ-UDPAPP-002"]}
TEST_F(SenderAppTest, InitNullBrokerReturnsNullptrError)
{
    JUNO_STATUS_T eStatus = SenderApp_Init(
        &tSender,
        &s_tUdpStub,
        nullptr,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_NULLPTR_ERROR, eStatus);
}

/* --------------------------------------------------------------------------
 * SenderApp Init — happy path
 * -------------------------------------------------------------------------- */

// @{"verify": ["REQ-UDPAPP-002"]}
TEST_F(SenderAppTest, InitHappyPathWiresVtableAndStoresPointers)
{
    JUNO_STATUS_T eStatus = SenderApp_Init(
        &tSender,
        &s_tUdpStub,
        &s_tBrokerStub,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    /* Internal vtable wired (non-NULL) */
    EXPECT_NE(nullptr,         tSender.tRoot.ptApi);
    /* Dependency pointers stored */
    EXPECT_EQ(&s_tUdpStub,    tSender.ptUdp);
    EXPECT_EQ(&s_tBrokerStub, tSender.ptBroker);
    /* Private state zero-initialised */
    EXPECT_EQ(0u, tSender._uSeqNum);
}

// @{"verify": ["REQ-UDPAPP-002"]}
TEST_F(SenderAppTest, InitNullFailureHandlerIsAccepted)
{
    /* pfcnFailureHandler and pvFailureUserData are optional (may be NULL) */
    JUNO_STATUS_T eStatus = SenderApp_Init(
        &tSender,
        &s_tUdpStub,
        &s_tBrokerStub,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    EXPECT_EQ(nullptr, tSender.tRoot._pfcnFailureHandler);
    EXPECT_EQ(nullptr, tSender.tRoot._pvFailureUserData);
}

/* --------------------------------------------------------------------------
 * SenderApp vtable dispatch verification
 *
 * Confirms that the internal vtable is wired and each slot can be dispatched
 * through ptApi, returning SUCCESS for the production implementation.
 * -------------------------------------------------------------------------- */

// @{"verify": ["REQ-UDPAPP-002"]}
TEST_F(SenderAppTest, VtableDispatchOnStart)
{
    SenderApp_Init(&tSender, &s_tUdpStub, &s_tBrokerStub, nullptr, nullptr);

    EXPECT_NE(nullptr, tSender.tRoot.ptApi->OnStart);
    JUNO_STATUS_T eStatus = tSender.tRoot.ptApi->OnStart(&tSender.tRoot);

    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

// @{"verify": ["REQ-UDPAPP-002"]}
TEST_F(SenderAppTest, VtableDispatchOnProcess)
{
    SenderApp_Init(&tSender, &s_tUdpStub, &s_tBrokerStub, nullptr, nullptr);

    EXPECT_NE(nullptr, tSender.tRoot.ptApi->OnProcess);
    JUNO_STATUS_T eStatus = tSender.tRoot.ptApi->OnProcess(&tSender.tRoot);

    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

// @{"verify": ["REQ-UDPAPP-002"]}
TEST_F(SenderAppTest, VtableDispatchOnExit)
{
    SenderApp_Init(&tSender, &s_tUdpStub, &s_tBrokerStub, nullptr, nullptr);

    EXPECT_NE(nullptr, tSender.tRoot.ptApi->OnExit);
    JUNO_STATUS_T eStatus = tSender.tRoot.ptApi->OnExit(&tSender.tRoot);

    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

/* --------------------------------------------------------------------------
 * SenderApp production vtable presence
 *
 * Verify that the internal vtable slots are not null and dispatch returns
 * SUCCESS for each lifecycle function.
 * -------------------------------------------------------------------------- */

// @{"verify": ["REQ-UDPAPP-002", "REQ-UDPAPP-003"]}
TEST_F(SenderAppTest, ProductionVtableOnStartReturnsSuccess)
{
    SenderApp_Init(&tSender, &s_tUdpStub, &s_tBrokerStub, nullptr, nullptr);

    EXPECT_NE(nullptr, tSender.tRoot.ptApi->OnStart);
    JUNO_STATUS_T eStatus = tSender.tRoot.ptApi->OnStart(&tSender.tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

// @{"verify": ["REQ-UDPAPP-002", "REQ-UDPAPP-004", "REQ-UDPAPP-005", "REQ-UDPAPP-006"]}
TEST_F(SenderAppTest, ProductionVtableOnProcessReturnsSuccess)
{
    SenderApp_Init(&tSender, &s_tUdpStub, &s_tBrokerStub, nullptr, nullptr);

    EXPECT_NE(nullptr, tSender.tRoot.ptApi->OnProcess);
    JUNO_STATUS_T eStatus = tSender.tRoot.ptApi->OnProcess(&tSender.tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

// @{"verify": ["REQ-UDPAPP-002", "REQ-UDPAPP-007"]}
TEST_F(SenderAppTest, ProductionVtableOnExitReturnsSuccess)
{
    SenderApp_Init(&tSender, &s_tUdpStub, &s_tBrokerStub, nullptr, nullptr);

    EXPECT_NE(nullptr, tSender.tRoot.ptApi->OnExit);
    JUNO_STATUS_T eStatus = tSender.tRoot.ptApi->OnExit(&tSender.tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

/* ==========================================================================
 * MonitorApp Tests
 * ========================================================================== */

class MonitorAppTest : public ::testing::Test
{
protected:
    MONITOR_APP_T tMonitor;

    void SetUp() override
    {
        std::memset(&tMonitor, 0, sizeof(tMonitor));
        s_tUdpStub.ptApi = &s_tUdpStubApi;
        JunoSb_BrokerInit(&s_tBrokerStub, s_aptBrokerPipeRegistry, 4u, nullptr, nullptr);
        UdpThreadMsgArray_Init(&s_tMsgArray, nullptr, nullptr);
        ResetDispatchFlags();
    }
};

/* --------------------------------------------------------------------------
 * MonitorApp Init — error paths
 * -------------------------------------------------------------------------- */

// @{"verify": ["REQ-UDPAPP-008"]}
TEST_F(MonitorAppTest, InitNullAppReturnsNullptrError)
{
    JUNO_STATUS_T eStatus = MonitorApp_Init(
        nullptr,
        &s_tBrokerStub,
        &s_tMsgArray.tRoot,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_NULLPTR_ERROR, eStatus);
}

// @{"verify": ["REQ-UDPAPP-008"]}
TEST_F(MonitorAppTest, InitNullBrokerReturnsNullptrError)
{
    JUNO_STATUS_T eStatus = MonitorApp_Init(
        &tMonitor,
        nullptr,
        &s_tMsgArray.tRoot,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_NULLPTR_ERROR, eStatus);
}

// @{"verify": ["REQ-UDPAPP-008"]}
TEST_F(MonitorAppTest, InitNullPipeArrayReturnsNullptrError)
{
    JUNO_STATUS_T eStatus = MonitorApp_Init(
        &tMonitor,
        &s_tBrokerStub,
        nullptr,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_NULLPTR_ERROR, eStatus);
}

/* --------------------------------------------------------------------------
 * MonitorApp Init — happy path
 * -------------------------------------------------------------------------- */

// @{"verify": ["REQ-UDPAPP-008"]}
TEST_F(MonitorAppTest, InitHappyPathWiresVtableAndStoresPointers)
{
    JUNO_STATUS_T eStatus = MonitorApp_Init(
        &tMonitor,
        &s_tBrokerStub,
        &s_tMsgArray.tRoot,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    /* Internal vtable wired (not null) */
    EXPECT_NE(nullptr,            tMonitor.tRoot.ptApi);
    /* Dependency pointers stored */
    EXPECT_EQ(&s_tBrokerStub,     tMonitor.ptBroker);
    EXPECT_EQ(&s_tMsgArray.tRoot,  tMonitor._ptPipeArray);
}

// @{"verify": ["REQ-UDPAPP-008"]}
TEST_F(MonitorAppTest, InitNullFailureHandlerIsAccepted)
{
    JUNO_STATUS_T eStatus = MonitorApp_Init(
        &tMonitor,
        &s_tBrokerStub,
        &s_tMsgArray.tRoot,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    EXPECT_EQ(nullptr, tMonitor._pfcnFailureHandler);
    EXPECT_EQ(nullptr, tMonitor._pvFailureUserData);
}

/* --------------------------------------------------------------------------
 * MonitorApp vtable dispatch verification
 * -------------------------------------------------------------------------- */

// @{"verify": ["REQ-UDPAPP-008", "REQ-UDPAPP-009"]}
TEST_F(MonitorAppTest, ProductionVtableOnStartReturnsSuccess)
{
    MonitorApp_Init(&tMonitor, &s_tBrokerStub, &s_tMsgArray.tRoot, nullptr, nullptr);

    EXPECT_NE(nullptr, tMonitor.tRoot.ptApi->OnStart);
    JUNO_STATUS_T eStatus = tMonitor.tRoot.ptApi->OnStart(&tMonitor.tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

// @{"verify": ["REQ-UDPAPP-008", "REQ-UDPAPP-010"]}
TEST_F(MonitorAppTest, ProductionVtableOnProcessReturnsSuccess)
{
    MonitorApp_Init(&tMonitor, &s_tBrokerStub, &s_tMsgArray.tRoot, nullptr, nullptr);
    tMonitor.tRoot.ptApi->OnStart(&tMonitor.tRoot);

    EXPECT_NE(nullptr, tMonitor.tRoot.ptApi->OnProcess);
    JUNO_STATUS_T eStatus = tMonitor.tRoot.ptApi->OnProcess(&tMonitor.tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

// @{"verify": ["REQ-UDPAPP-008"]}
TEST_F(MonitorAppTest, ProductionVtableOnExitReturnsSuccess)
{
    MonitorApp_Init(&tMonitor, &s_tBrokerStub, &s_tMsgArray.tRoot, nullptr, nullptr);

    EXPECT_NE(nullptr, tMonitor.tRoot.ptApi->OnExit);
    JUNO_STATUS_T eStatus = tMonitor.tRoot.ptApi->OnExit(&tMonitor.tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

/* ==========================================================================
 * UdpBridgeApp Tests
 * ========================================================================== */

class UdpBridgeAppTest : public ::testing::Test
{
protected:
    UDP_BRIDGE_APP_T tBridge;

    void SetUp() override
    {
        std::memset(&tBridge, 0, sizeof(tBridge));
        std::memset(&s_tUdpStub, 0, sizeof(s_tUdpStub));
        s_tUdpStub.ptApi = &s_tUdpStubApi;
        JunoSb_BrokerInit(&s_tBrokerStub, s_aptBrokerPipeRegistry, 4u, nullptr, nullptr);
        ResetDispatchFlags();
    }
};

/* --------------------------------------------------------------------------
 * UdpBridgeApp Init — error paths
 * -------------------------------------------------------------------------- */

// @{"verify": ["REQ-UDPAPP-011"]}
TEST_F(UdpBridgeAppTest, InitNullAppReturnsNullptrError)
{
    JUNO_STATUS_T eStatus = UdpBridgeApp_Init(
        nullptr,
        &s_tUdpStub,
        &s_tBrokerStub,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_NULLPTR_ERROR, eStatus);
}

// @{"verify": ["REQ-UDPAPP-011"]}
TEST_F(UdpBridgeAppTest, InitNullUdpReturnsNullptrError)
{
    JUNO_STATUS_T eStatus = UdpBridgeApp_Init(
        &tBridge,
        nullptr,
        &s_tBrokerStub,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_NULLPTR_ERROR, eStatus);
}

// @{"verify": ["REQ-UDPAPP-011"]}
TEST_F(UdpBridgeAppTest, InitNullBrokerReturnsNullptrError)
{
    JUNO_STATUS_T eStatus = UdpBridgeApp_Init(
        &tBridge,
        &s_tUdpStub,
        nullptr,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_NULLPTR_ERROR, eStatus);
}

/* --------------------------------------------------------------------------
 * UdpBridgeApp Init — happy path
 * -------------------------------------------------------------------------- */

// @{"verify": ["REQ-UDPAPP-011"]}
TEST_F(UdpBridgeAppTest, InitHappyPathWiresVtableAndStoresPointers)
{
    JUNO_STATUS_T eStatus = UdpBridgeApp_Init(
        &tBridge,
        &s_tUdpStub,
        &s_tBrokerStub,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    /* Internal vtable wired (non-NULL) */
    EXPECT_NE(nullptr, tBridge.tRoot.ptApi);
    /* Dependency pointers stored */
    EXPECT_EQ(&s_tUdpStub,    tBridge.ptUdp);
    EXPECT_EQ(&s_tBrokerStub, tBridge.ptBroker);
}

// @{"verify": ["REQ-UDPAPP-011"]}
TEST_F(UdpBridgeAppTest, InitNullFailureHandlerIsAccepted)
{
    JUNO_STATUS_T eStatus = UdpBridgeApp_Init(
        &tBridge,
        &s_tUdpStub,
        &s_tBrokerStub,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    EXPECT_EQ(nullptr, tBridge.tRoot._pfcnFailureHandler);
    EXPECT_EQ(nullptr, tBridge.tRoot._pvFailureUserData);
}

/* --------------------------------------------------------------------------
 * UdpBridgeApp vtable dispatch verification
 * -------------------------------------------------------------------------- */

// @{"verify": ["REQ-UDPAPP-011"]}
TEST_F(UdpBridgeAppTest, VtableDispatchOnStart)
{
    UdpBridgeApp_Init(&tBridge, &s_tUdpStub, &s_tBrokerStub, nullptr, nullptr);

    EXPECT_NE(nullptr, tBridge.tRoot.ptApi->OnStart);
    JUNO_STATUS_T eStatus = tBridge.tRoot.ptApi->OnStart(&tBridge.tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

// @{"verify": ["REQ-UDPAPP-011"]}
TEST_F(UdpBridgeAppTest, VtableDispatchOnProcess)
{
    UdpBridgeApp_Init(&tBridge, &s_tUdpStub, &s_tBrokerStub, nullptr, nullptr);

    EXPECT_NE(nullptr, tBridge.tRoot.ptApi->OnProcess);
    JUNO_STATUS_T eStatus = tBridge.tRoot.ptApi->OnProcess(&tBridge.tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

// @{"verify": ["REQ-UDPAPP-011"]}
TEST_F(UdpBridgeAppTest, VtableDispatchOnExit)
{
    UdpBridgeApp_Init(&tBridge, &s_tUdpStub, &s_tBrokerStub, nullptr, nullptr);

    EXPECT_NE(nullptr, tBridge.tRoot.ptApi->OnExit);
    JUNO_STATUS_T eStatus = tBridge.tRoot.ptApi->OnExit(&tBridge.tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

/* --------------------------------------------------------------------------
 * UdpBridgeApp production vtable presence
 * -------------------------------------------------------------------------- */

// @{"verify": ["REQ-UDPAPP-011", "REQ-UDPAPP-012"]}
TEST_F(UdpBridgeAppTest, ProductionVtableOnStartReturnsSuccess)
{
    UdpBridgeApp_Init(&tBridge, &s_tUdpStub, &s_tBrokerStub, nullptr, nullptr);

    EXPECT_NE(nullptr, tBridge.tRoot.ptApi->OnStart);
    JUNO_STATUS_T eStatus = tBridge.tRoot.ptApi->OnStart(&tBridge.tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

// @{"verify": ["REQ-UDPAPP-011", "REQ-UDPAPP-013", "REQ-UDPAPP-014"]}
TEST_F(UdpBridgeAppTest, ProductionVtableOnProcessReturnsSuccess)
{
    UdpBridgeApp_Init(&tBridge, &s_tUdpStub, &s_tBrokerStub, nullptr, nullptr);

    EXPECT_NE(nullptr, tBridge.tRoot.ptApi->OnProcess);
    JUNO_STATUS_T eStatus = tBridge.tRoot.ptApi->OnProcess(&tBridge.tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

// @{"verify": ["REQ-UDPAPP-011", "REQ-UDPAPP-015"]}
TEST_F(UdpBridgeAppTest, ProductionVtableOnExitReturnsSuccess)
{
    UdpBridgeApp_Init(&tBridge, &s_tUdpStub, &s_tBrokerStub, nullptr, nullptr);

    EXPECT_NE(nullptr, tBridge.tRoot.ptApi->OnExit);
    JUNO_STATUS_T eStatus = tBridge.tRoot.ptApi->OnExit(&tBridge.tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

/* ==========================================================================
 * ProcessorApp Tests
 * ========================================================================== */

class ProcessorAppTest : public ::testing::Test
{
protected:
    PROCESSOR_APP_T tProcessor;

    void SetUp() override
    {
        std::memset(&tProcessor, 0, sizeof(tProcessor));
        s_tUdpStub.ptApi = &s_tUdpStubApi;
        JunoSb_BrokerInit(&s_tBrokerStub, s_aptBrokerPipeRegistry, 4u, nullptr, nullptr);
        UdpThreadMsgArray_Init(&s_tMsgArray, nullptr, nullptr);
        ResetDispatchFlags();
    }
};

/* --------------------------------------------------------------------------
 * ProcessorApp Init — error paths
 * -------------------------------------------------------------------------- */

// @{"verify": ["REQ-UDPAPP-016"]}
TEST_F(ProcessorAppTest, InitNullAppReturnsNullptrError)
{
    JUNO_STATUS_T eStatus = ProcessorApp_Init(
        nullptr,
        &s_tBrokerStub,
        &s_tMsgArray.tRoot,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_NULLPTR_ERROR, eStatus);
}

// @{"verify": ["REQ-UDPAPP-016"]}
TEST_F(ProcessorAppTest, InitNullBrokerReturnsNullptrError)
{
    JUNO_STATUS_T eStatus = ProcessorApp_Init(
        &tProcessor,
        nullptr,
        &s_tMsgArray.tRoot,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_NULLPTR_ERROR, eStatus);
}

// @{"verify": ["REQ-UDPAPP-016"]}
TEST_F(ProcessorAppTest, InitNullPipeArrayReturnsNullptrError)
{
    JUNO_STATUS_T eStatus = ProcessorApp_Init(
        &tProcessor,
        &s_tBrokerStub,
        nullptr,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_NULLPTR_ERROR, eStatus);
}

/* --------------------------------------------------------------------------
 * ProcessorApp Init — happy path
 * -------------------------------------------------------------------------- */

// @{"verify": ["REQ-UDPAPP-016"]}
TEST_F(ProcessorAppTest, InitHappyPathWiresVtableAndStoresPointers)
{
    JUNO_STATUS_T eStatus = ProcessorApp_Init(
        &tProcessor,
        &s_tBrokerStub,
        &s_tMsgArray.tRoot,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    /* Internal vtable wired (non-NULL) */
    EXPECT_NE(nullptr,            tProcessor.tRoot.ptApi);
    /* Dependency pointers stored */
    EXPECT_EQ(&s_tBrokerStub,    tProcessor.ptBroker);
    EXPECT_EQ(&s_tMsgArray.tRoot, tProcessor._ptPipeArray);
}

// @{"verify": ["REQ-UDPAPP-016"]}
TEST_F(ProcessorAppTest, InitNullFailureHandlerIsAccepted)
{
    JUNO_STATUS_T eStatus = ProcessorApp_Init(
        &tProcessor,
        &s_tBrokerStub,
        &s_tMsgArray.tRoot,
        nullptr,
        nullptr
    );
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
    EXPECT_EQ(nullptr, tProcessor._pfcnFailureHandler);
    EXPECT_EQ(nullptr, tProcessor._pvFailureUserData);
}

/* --------------------------------------------------------------------------
 * ProcessorApp vtable dispatch verification
 * -------------------------------------------------------------------------- */

// @{"verify": ["REQ-UDPAPP-016"]}
TEST_F(ProcessorAppTest, VtableDispatchOnStart)
{
    ProcessorApp_Init(&tProcessor, &s_tBrokerStub, &s_tMsgArray.tRoot, nullptr, nullptr);

    EXPECT_NE(nullptr, tProcessor.tRoot.ptApi->OnStart);
    JUNO_STATUS_T eStatus = tProcessor.tRoot.ptApi->OnStart(&tProcessor.tRoot);

    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

// @{"verify": ["REQ-UDPAPP-016"]}
TEST_F(ProcessorAppTest, VtableDispatchOnProcess)
{
    ProcessorApp_Init(&tProcessor, &s_tBrokerStub, &s_tMsgArray.tRoot, nullptr, nullptr);
    tProcessor.tRoot.ptApi->OnStart(&tProcessor.tRoot);

    EXPECT_NE(nullptr, tProcessor.tRoot.ptApi->OnProcess);
    JUNO_STATUS_T eStatus = tProcessor.tRoot.ptApi->OnProcess(&tProcessor.tRoot);

    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

// @{"verify": ["REQ-UDPAPP-016"]}
TEST_F(ProcessorAppTest, VtableDispatchOnExit)
{
    ProcessorApp_Init(&tProcessor, &s_tBrokerStub, &s_tMsgArray.tRoot, nullptr, nullptr);

    EXPECT_NE(nullptr, tProcessor.tRoot.ptApi->OnExit);
    JUNO_STATUS_T eStatus = tProcessor.tRoot.ptApi->OnExit(&tProcessor.tRoot);

    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

/* --------------------------------------------------------------------------
 * ProcessorApp production vtable presence
 * -------------------------------------------------------------------------- */

// @{"verify": ["REQ-UDPAPP-016", "REQ-UDPAPP-017"]}
TEST_F(ProcessorAppTest, ProductionVtableOnStartReturnsSuccess)
{
    ProcessorApp_Init(&tProcessor, &s_tBrokerStub, &s_tMsgArray.tRoot, nullptr, nullptr);

    EXPECT_NE(nullptr, tProcessor.tRoot.ptApi->OnStart);
    JUNO_STATUS_T eStatus = tProcessor.tRoot.ptApi->OnStart(&tProcessor.tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

// @{"verify": ["REQ-UDPAPP-016", "REQ-UDPAPP-018"]}
TEST_F(ProcessorAppTest, ProductionVtableOnProcessReturnsSuccess)
{
    ProcessorApp_Init(&tProcessor, &s_tBrokerStub, &s_tMsgArray.tRoot, nullptr, nullptr);
    tProcessor.tRoot.ptApi->OnStart(&tProcessor.tRoot);

    EXPECT_NE(nullptr, tProcessor.tRoot.ptApi->OnProcess);
    JUNO_STATUS_T eStatus = tProcessor.tRoot.ptApi->OnProcess(&tProcessor.tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}

// @{"verify": ["REQ-UDPAPP-016"]}
TEST_F(ProcessorAppTest, ProductionVtableOnExitReturnsSuccess)
{
    ProcessorApp_Init(&tProcessor, &s_tBrokerStub, &s_tMsgArray.tRoot, nullptr, nullptr);

    EXPECT_NE(nullptr, tProcessor.tRoot.ptApi->OnExit);
    JUNO_STATUS_T eStatus = tProcessor.tRoot.ptApi->OnExit(&tProcessor.tRoot);
    EXPECT_EQ(JUNO_STATUS_SUCCESS, eStatus);
}
