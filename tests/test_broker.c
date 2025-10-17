#include "juno/ds/array_api.h"
#include "juno/ds/buff_queue_api.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include "juno/module.h"
#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include <stdbool.h>
#include <string.h>
#include "juno/sb/broker_api.h"

typedef uint32_t TEST_MID;
typedef struct TEST_MSG_T TEST_MSG_T;
typedef struct MOCK_QUEUE_T MOCK_QUEUE_T;

struct TEST_MSG_T
{
	uint32_t iNumber;
	char iTestData[256];
};

/// Copy memory from one pointer to another
static JUNO_STATUS_T MidCopy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
/// Reset the memory at the pointer. This could mean zero-initialization
static JUNO_STATUS_T MidReset(JUNO_POINTER_T tPointer);
static JUNO_RESULT_BOOL_T MidEquals(const JUNO_VALUE_POINTER_T tLeft, const JUNO_VALUE_POINTER_T tRight);

/// Copy memory from one pointer to another
static JUNO_STATUS_T MsgCopy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
/// Reset the memory at the pointer. This could mean zero-initialization
static JUNO_STATUS_T MsgReset(JUNO_POINTER_T tPointer);

/// Enqueue an item on the queue
static JUNO_STATUS_T MockEnqueue(JUNO_DS_QUEUE_ROOT_T *ptQueue, JUNO_POINTER_T tItem);
/// Dequeue an item from the queue
static JUNO_STATUS_T MockDequeue(JUNO_DS_QUEUE_ROOT_T *ptQueue, JUNO_POINTER_T tReturn);
/// Peek at the next item in the queue
static JUNO_RESULT_POINTER_T MockPeek(JUNO_DS_QUEUE_ROOT_T *ptQueue);

/// Mock array functions (minimal implementation for verification)
static JUNO_STATUS_T MockArraySetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
static JUNO_RESULT_POINTER_T MockArrayGetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
static JUNO_STATUS_T MockArrayRemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);

/*
This is the mock queue for this test. This can be used in the pipe for test messages since
it accepts a pointer to the root of this module.
*/
struct MOCK_QUEUE_T JUNO_MODULE_DERIVE(JUNO_DS_QUEUE_ROOT_T, 
	TEST_MSG_T tMsgBuffer[10];
	size_t iCount;
	size_t iHead;
	size_t iTail;
	bool bEnqueueShouldFail;
);

// Mock array buffers for the queues
static JUNO_DS_ARRAY_ROOT_T gtMockArrayBuffers[5];

const JUNO_VALUE_POINTER_API_T gtMidValuePointerApi =
{
	{
		MidCopy,
		MidReset
	},
	MidEquals
};

const JUNO_POINTER_API_T gtMsgPointerApi =
{
	MsgCopy,
	MsgReset
};

static const JUNO_DS_QUEUE_API_T gtMockQueueApi =
{
	MockEnqueue,
	MockDequeue,
	MockPeek
};

static const JUNO_DS_ARRAY_API_T gtMockArrayApi =
{
	MockArraySetAt,
	MockArrayGetAt,
	MockArrayRemoveAt
};

// Global test fixtures
static JUNO_SB_BROKER_ROOT_T gtBroker;
static JUNO_SB_PIPE_REGISTRY_T gtRegistry;
static JUNO_SB_PIPE_T gtPipes[5];
static MOCK_QUEUE_T gtMockQueues[5];
static TEST_MID gtMids[5];
static bool gbFailureHandlerCalled;
static JUNO_STATUS_T gtLastFailureStatus;

static void TestFailureHandler(JUNO_STATUS_T tStatus, const char *pcCustomMessage, JUNO_USER_DATA_T *pvUserData)
{
	(void)pcCustomMessage;
	(void)pvUserData;
	gbFailureHandlerCalled = true;
	gtLastFailureStatus = tStatus;
}

static JUNO_VALUE_POINTER_T CreateMidValuePointer(TEST_MID *ptMid)
{
	JUNO_VALUE_POINTER_T tResult;
	tResult.JUNO_MODULE_SUPER = JunoMemory_PointerInit(&gtMidValuePointerApi.tRoot, TEST_MID, ptMid);
	tResult.ptApi = &gtMidValuePointerApi;
	return tResult;
}

static void InitMockQueue(size_t iIndex, TEST_MID tMid, bool bShouldFail)
{
	gtMockQueues[iIndex].JUNO_MODULE_SUPER.ptApi = &gtMockQueueApi;
	gtMockQueues[iIndex].JUNO_MODULE_SUPER.ptBuffer = &gtMockArrayBuffers[iIndex];
	gtMockQueues[iIndex].JUNO_MODULE_SUPER.iStartIndex = 0;
	gtMockQueues[iIndex].iCount = 0;
	gtMockQueues[iIndex].iHead = 0;
	gtMockQueues[iIndex].iTail = 0;
	gtMockQueues[iIndex].bEnqueueShouldFail = bShouldFail;
	
	gtMids[iIndex] = tMid;
	gtPipes[iIndex].ptRecvQueue = &gtMockQueues[iIndex].JUNO_MODULE_SUPER;
	gtPipes[iIndex].tMid = CreateMidValuePointer(&gtMids[iIndex]);
}

void setUp(void)
{
	memset(&gtBroker, 0, sizeof(gtBroker));
	memset(&gtRegistry, 0, sizeof(gtRegistry));
	memset(gtPipes, 0, sizeof(gtPipes));
	memset(gtMockQueues, 0, sizeof(gtMockQueues));
	memset(gtMids, 0, sizeof(gtMids));
	memset(gtMockArrayBuffers, 0, sizeof(gtMockArrayBuffers));
	gbFailureHandlerCalled = false;
	gtLastFailureStatus = JUNO_STATUS_SUCCESS;
	
	// Initialize all mock array buffers
	for (size_t i = 0; i < 5; i++)
	{
		gtMockArrayBuffers[i].ptApi = &gtMockArrayApi;
		gtMockArrayBuffers[i].zCapacity = 10;
		gtMockArrayBuffers[i].zLength = 0;
	}
}

void tearDown(void)
{

}


static void test_registry_init_success(void)
{
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(&gtRegistry, gtPipes, 5);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	TEST_ASSERT_NOT_NULL(gtRegistry.tRoot.ptApi);
	TEST_ASSERT_EQUAL_PTR(gtPipes, gtRegistry.ptArrItems);
	TEST_ASSERT_EQUAL(5, gtRegistry.tRoot.zCapacity);
	TEST_ASSERT_EQUAL(0, gtRegistry.tRoot.zLength);
}

static void test_registry_init_null_registry(void)
{
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(NULL, gtPipes, 5);
	TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, tStatus);
}

static void test_registry_init_null_pipes(void)
{
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(&gtRegistry, NULL, 5);
	TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, tStatus);
}

static void test_registry_init_zero_capacity(void)
{
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(&gtRegistry, gtPipes, 0);
	TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, tStatus);
}

static void test_broker_init_success(void)
{
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(&gtRegistry, gtPipes, 5);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	tStatus = JunoSb_BrokerInit(&gtBroker, &gtRegistry);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	TEST_ASSERT_NOT_NULL(gtBroker.ptApi);
	TEST_ASSERT_EQUAL_PTR(&gtRegistry, gtBroker.ptRegistry);
}

static void test_broker_init_null_broker(void)
{
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(&gtRegistry, gtPipes, 5);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	tStatus = JunoSb_BrokerInit(NULL, &gtRegistry);
	TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, tStatus);
}

static void test_broker_init_null_registry(void)
{
	JUNO_STATUS_T tStatus = JunoSb_BrokerInit(&gtBroker, NULL);
	TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, tStatus);
}

static void test_register_subscriber_success(void)
{
	// Initialize broker and registry
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(&gtRegistry, gtPipes, 5);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	tStatus = JunoSb_BrokerInit(&gtBroker, &gtRegistry);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	// Initialize mock queue
	InitMockQueue(0, 100, false);
	
	// Register subscriber
	tStatus = gtBroker.ptApi->RegisterSubscriber(&gtBroker, &gtPipes[0]);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	TEST_ASSERT_EQUAL(1, gtRegistry.tRoot.zLength);
}

static void test_register_subscriber_multiple_success(void)
{
	// Initialize broker and registry
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(&gtRegistry, gtPipes, 5);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	tStatus = JunoSb_BrokerInit(&gtBroker, &gtRegistry);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	// Register 3 subscribers
	for (size_t i = 0; i < 3; i++)
	{
		InitMockQueue(i, 100 + (uint32_t)i, false);
		tStatus = gtBroker.ptApi->RegisterSubscriber(&gtBroker, &gtPipes[i]);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	
	TEST_ASSERT_EQUAL(3, gtRegistry.tRoot.zLength);
}

static void test_register_subscriber_registry_full(void)
{
	// Initialize broker with capacity of 2
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(&gtRegistry, gtPipes, 2);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	gtBroker.JUNO_FAILURE_HANDLER = TestFailureHandler;
	tStatus = JunoSb_BrokerInit(&gtBroker, &gtRegistry);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	// Register 2 subscribers successfully
	for (size_t i = 0; i < 2; i++)
	{
		InitMockQueue(i, 100 + (uint32_t)i, false);
		tStatus = gtBroker.ptApi->RegisterSubscriber(&gtBroker, &gtPipes[i]);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	
	// Try to register third subscriber - should fail
	InitMockQueue(2, 102, false);
	
	gbFailureHandlerCalled = false;
	tStatus = gtBroker.ptApi->RegisterSubscriber(&gtBroker, &gtPipes[2]);
	TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, tStatus);
	TEST_ASSERT_TRUE(gbFailureHandlerCalled);
	TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, gtLastFailureStatus);
	TEST_ASSERT_EQUAL(2, gtRegistry.tRoot.zLength);
}

static void test_register_subscriber_null_broker(void)
{
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(&gtRegistry, gtPipes, 5);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	tStatus = JunoSb_BrokerInit(&gtBroker, &gtRegistry);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	InitMockQueue(0, 100, false);
	
	tStatus = gtBroker.ptApi->RegisterSubscriber(NULL, &gtPipes[0]);
	TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, tStatus);
}

static void test_register_subscriber_null_pipe(void)
{
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(&gtRegistry, gtPipes, 5);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	tStatus = JunoSb_BrokerInit(&gtBroker, &gtRegistry);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	tStatus = gtBroker.ptApi->RegisterSubscriber(&gtBroker, NULL);
	TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, tStatus);
}

static void test_publish_single_subscriber(void)
{
	// Initialize broker and registry
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(&gtRegistry, gtPipes, 5);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	tStatus = JunoSb_BrokerInit(&gtBroker, &gtRegistry);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	// Setup mock queue and register
	InitMockQueue(0, 200, false);
	tStatus = gtBroker.ptApi->RegisterSubscriber(&gtBroker, &gtPipes[0]);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	// Create test message
	TEST_MSG_T tTestMsg = {.iNumber = 42};
	strcpy(tTestMsg.iTestData, "Test Message");
	JUNO_POINTER_T tMsgPointer = JunoMemory_PointerInit(&gtMsgPointerApi, TEST_MSG_T, &tTestMsg);
	
	// Publish message
	JUNO_VALUE_POINTER_T tMid = CreateMidValuePointer(&gtMids[0]);
	tStatus = gtBroker.ptApi->Publish(&gtBroker, tMid, tMsgPointer);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	// Verify message was enqueued
	TEST_ASSERT_EQUAL(1, gtMockQueues[0].iCount);
	TEST_ASSERT_EQUAL(42, gtMockQueues[0].tMsgBuffer[0].iNumber);
	TEST_ASSERT_EQUAL_STRING("Test Message", gtMockQueues[0].tMsgBuffer[0].iTestData);
}

static void test_publish_multiple_subscribers_same_mid(void)
{
	// Initialize broker and registry
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(&gtRegistry, gtPipes, 5);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	tStatus = JunoSb_BrokerInit(&gtBroker, &gtRegistry);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	// Setup 3 subscribers with same MID
	for (size_t i = 0; i < 3; i++)
	{
		InitMockQueue(i, 300, false); // Same MID for all
		tStatus = gtBroker.ptApi->RegisterSubscriber(&gtBroker, &gtPipes[i]);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	
	// Create test message
	TEST_MSG_T tTestMsg = {.iNumber = 999};
	strcpy(tTestMsg.iTestData, "Broadcast");
	JUNO_POINTER_T tMsgPointer = JunoMemory_PointerInit(&gtMsgPointerApi, TEST_MSG_T, &tTestMsg);
	
	// Setup publish MID
	TEST_MID tPubMid = 300;
	JUNO_VALUE_POINTER_T tMid = CreateMidValuePointer(&tPubMid);
	
	// Publish message
	tStatus = gtBroker.ptApi->Publish(&gtBroker, tMid, tMsgPointer);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	// Verify all queues received the message
	for (size_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL(1, gtMockQueues[i].iCount);
		TEST_ASSERT_EQUAL(999, gtMockQueues[i].tMsgBuffer[0].iNumber);
		TEST_ASSERT_EQUAL_STRING("Broadcast", gtMockQueues[i].tMsgBuffer[0].iTestData);
	}
}

static void test_publish_multiple_subscribers_different_mids(void)
{
	// Initialize broker and registry
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(&gtRegistry, gtPipes, 5);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	tStatus = JunoSb_BrokerInit(&gtBroker, &gtRegistry);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	// Setup 3 subscribers with different MIDs
	for (size_t i = 0; i < 3; i++)
	{
		InitMockQueue(i, 400 + (uint32_t)i, false); // Different MID for each
		tStatus = gtBroker.ptApi->RegisterSubscriber(&gtBroker, &gtPipes[i]);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	
	// Create test message
	TEST_MSG_T tTestMsg = {.iNumber = 777};
	strcpy(tTestMsg.iTestData, "Targeted");
	JUNO_POINTER_T tMsgPointer = JunoMemory_PointerInit(&gtMsgPointerApi, TEST_MSG_T, &tTestMsg);
	
	// Publish to MID 401 (only second subscriber)
	TEST_MID tPubMid = 401;
	JUNO_VALUE_POINTER_T tMid = CreateMidValuePointer(&tPubMid);
	
	tStatus = gtBroker.ptApi->Publish(&gtBroker, tMid, tMsgPointer);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	// Verify only second queue received the message
	TEST_ASSERT_EQUAL(0, gtMockQueues[0].iCount);
	TEST_ASSERT_EQUAL(1, gtMockQueues[1].iCount);
	TEST_ASSERT_EQUAL(0, gtMockQueues[2].iCount);
	
	TEST_ASSERT_EQUAL(777, gtMockQueues[1].tMsgBuffer[0].iNumber);
	TEST_ASSERT_EQUAL_STRING("Targeted", gtMockQueues[1].tMsgBuffer[0].iTestData);
}

static void test_publish_no_matching_subscribers(void)
{
	// Initialize broker and registry
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(&gtRegistry, gtPipes, 5);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	tStatus = JunoSb_BrokerInit(&gtBroker, &gtRegistry);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	// Setup subscriber with MID 500
	InitMockQueue(0, 500, false);
	tStatus = gtBroker.ptApi->RegisterSubscriber(&gtBroker, &gtPipes[0]);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	// Create test message
	TEST_MSG_T tTestMsg = {.iNumber = 111};
	strcpy(tTestMsg.iTestData, "No Match");
	JUNO_POINTER_T tMsgPointer = JunoMemory_PointerInit(&gtMsgPointerApi, TEST_MSG_T, &tTestMsg);
	
	// Publish to MID 999 (no subscriber)
	TEST_MID tPubMid = 999;
	JUNO_VALUE_POINTER_T tMid = CreateMidValuePointer(&tPubMid);
	
	tStatus = gtBroker.ptApi->Publish(&gtBroker, tMid, tMsgPointer);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	// Verify no messages were enqueued
	TEST_ASSERT_EQUAL(0, gtMockQueues[0].iCount);
}

static void test_publish_empty_registry(void)
{
	// Initialize broker with empty registry
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(&gtRegistry, gtPipes, 5);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	tStatus = JunoSb_BrokerInit(&gtBroker, &gtRegistry);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	// Create test message
	TEST_MSG_T tTestMsg = {.iNumber = 123};
	JUNO_POINTER_T tMsgPointer = JunoMemory_PointerInit(&gtMsgPointerApi, TEST_MSG_T, &tTestMsg);
	
	// Setup publish MID
	TEST_MID tPubMid = 600;
	JUNO_VALUE_POINTER_T tMid = CreateMidValuePointer(&tPubMid);
	
	// Should succeed but do nothing
	tStatus = gtBroker.ptApi->Publish(&gtBroker, tMid, tMsgPointer);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_publish_null_broker(void)
{
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(&gtRegistry, gtPipes, 5);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	tStatus = JunoSb_BrokerInit(&gtBroker, &gtRegistry);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	TEST_MSG_T tTestMsg = {.iNumber = 123};
	JUNO_POINTER_T tMsgPointer = JunoMemory_PointerInit(&gtMsgPointerApi, TEST_MSG_T, &tTestMsg);
	
	TEST_MID tPubMid = 600;
	JUNO_VALUE_POINTER_T tMid = CreateMidValuePointer(&tPubMid);
	
	tStatus = gtBroker.ptApi->Publish(NULL, tMid, tMsgPointer);
	TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, tStatus);
}

static void test_publish_enqueue_failure(void)
{
	// Initialize broker and registry
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(&gtRegistry, gtPipes, 5);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	tStatus = JunoSb_BrokerInit(&gtBroker, &gtRegistry);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	// Setup mock queue that will fail on enqueue
	InitMockQueue(0, 700, true); // Force failure
	tStatus = gtBroker.ptApi->RegisterSubscriber(&gtBroker, &gtPipes[0]);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	// Create test message
	TEST_MSG_T tTestMsg = {.iNumber = 888};
	JUNO_POINTER_T tMsgPointer = JunoMemory_PointerInit(&gtMsgPointerApi, TEST_MSG_T, &tTestMsg);
	
	// Publish should fail
	JUNO_VALUE_POINTER_T tMid = CreateMidValuePointer(&gtMids[0]);
	tStatus = gtBroker.ptApi->Publish(&gtBroker, tMid, tMsgPointer);
	TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, tStatus);
	TEST_ASSERT_EQUAL(0, gtMockQueues[0].iCount);
}

static void test_publish_multiple_messages_to_same_subscriber(void)
{
	// Initialize broker and registry
	JUNO_STATUS_T tStatus = JunoSb_RegistryInit(&gtRegistry, gtPipes, 5);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	tStatus = JunoSb_BrokerInit(&gtBroker, &gtRegistry);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	// Setup subscriber
	InitMockQueue(0, 800, false);
	tStatus = gtBroker.ptApi->RegisterSubscriber(&gtBroker, &gtPipes[0]);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	
	// Publish 5 messages
	JUNO_VALUE_POINTER_T tMid = CreateMidValuePointer(&gtMids[0]);
	for (uint32_t i = 0; i < 5; i++)
	{
		TEST_MSG_T tTestMsg = {.iNumber = i};
		JUNO_POINTER_T tMsgPointer = JunoMemory_PointerInit(&gtMsgPointerApi, TEST_MSG_T, &tTestMsg);
		
		tStatus = gtBroker.ptApi->Publish(&gtBroker, tMid, tMsgPointer);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	
	// Verify all messages were enqueued
	TEST_ASSERT_EQUAL(5, gtMockQueues[0].iCount);
	for (size_t i = 0; i < 5; i++)
	{
		TEST_ASSERT_EQUAL(i, gtMockQueues[0].tMsgBuffer[i].iNumber);
	}
}

int main(void)
{
	UNITY_BEGIN();
	
	// Registry init tests
	RUN_TEST(test_registry_init_success);
	RUN_TEST(test_registry_init_null_registry);
	RUN_TEST(test_registry_init_null_pipes);
	RUN_TEST(test_registry_init_zero_capacity);
	
	// Broker init tests
	RUN_TEST(test_broker_init_success);
	RUN_TEST(test_broker_init_null_broker);
	RUN_TEST(test_broker_init_null_registry);
	
	// RegisterSubscriber tests
	RUN_TEST(test_register_subscriber_success);
	RUN_TEST(test_register_subscriber_multiple_success);
	RUN_TEST(test_register_subscriber_registry_full);
	RUN_TEST(test_register_subscriber_null_broker);
	RUN_TEST(test_register_subscriber_null_pipe);
	
	// Publish tests
	RUN_TEST(test_publish_single_subscriber);
	RUN_TEST(test_publish_multiple_subscribers_same_mid);
	RUN_TEST(test_publish_multiple_subscribers_different_mids);
	RUN_TEST(test_publish_no_matching_subscribers);
	RUN_TEST(test_publish_empty_registry);
	RUN_TEST(test_publish_null_broker);
	RUN_TEST(test_publish_enqueue_failure);
	RUN_TEST(test_publish_multiple_messages_to_same_subscriber);
	
	return UNITY_END();
}

/// Copy memory from one pointer to another
static JUNO_STATUS_T MidCopy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc)
{
	JUNO_STATUS_T tStatus = JunoMemory_PointerVerify(tDest);
	JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
	tStatus = JunoMemory_PointerVerify(tSrc);
	JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
	JUNO_ASSERT_POINTER_COPY(tDest, tSrc, gtMidValuePointerApi.tRoot);
	*(TEST_MID*) tDest.pvAddr = *(TEST_MID *)tSrc.pvAddr;
	return tStatus;
}

/// Reset the memory at the pointer. This could mean zero-initialization
static JUNO_STATUS_T MidReset(JUNO_POINTER_T tPointer)
{
	JUNO_STATUS_T tStatus = JunoMemory_PointerVerify(tPointer);
	JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
	tStatus = JunoMemory_PointerCheckType(tPointer, TEST_MID, gtMidValuePointerApi.tRoot);
	JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
	*(TEST_MID *) tPointer.pvAddr = 0;
	return tStatus;
}

static JUNO_RESULT_BOOL_T MidEquals(const JUNO_VALUE_POINTER_T tLeft, const JUNO_VALUE_POINTER_T tRight)
{
	JUNO_RESULT_BOOL_T tResult = {0};
	tResult.tStatus = JunoMemory_ValuePointerVerify(tLeft);
	JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
	tResult.tStatus = JunoMemory_ValuePointerVerify(tRight);
	JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
	tResult.tStatus = JunoMemory_PointerCheckType(tLeft.tRoot, TEST_MID, gtMidValuePointerApi.tRoot);
	JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
	tResult.tStatus = JunoMemory_PointerCheckType(tRight.tRoot, TEST_MID, gtMidValuePointerApi.tRoot);
	JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
	tResult.tOk = (*(TEST_MID*) tLeft.tRoot.pvAddr) == (*(TEST_MID *)tRight.tRoot.pvAddr);
	return tResult;
}

/// Copy memory from one pointer to another
static JUNO_STATUS_T MsgCopy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc)
{
	JUNO_STATUS_T tStatus = JunoMemory_PointerVerify(tDest);
	JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
	tStatus = JunoMemory_PointerVerify(tSrc);
	JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
	JUNO_ASSERT_POINTER_COPY(tDest, tSrc, gtMsgPointerApi);
	*(TEST_MSG_T*) tDest.pvAddr = *(TEST_MSG_T *)tSrc.pvAddr;
	return tStatus;
}
/// Reset the memory at the pointer. This could mean zero-initialization
static JUNO_STATUS_T MsgReset(JUNO_POINTER_T tPointer)
{
	JUNO_STATUS_T tStatus = JunoMemory_PointerVerify(tPointer);
	JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
	tStatus = JunoMemory_PointerCheckType(tPointer, TEST_MSG_T, gtMsgPointerApi);
	JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
	*(TEST_MSG_T *) tPointer.pvAddr = (TEST_MSG_T){0};
	return tStatus;
}

/// Enqueue an item on the queue
static JUNO_STATUS_T MockEnqueue(JUNO_DS_QUEUE_ROOT_T *ptQueue, JUNO_POINTER_T tItem)
{
	// Cast to mock queue
	MOCK_QUEUE_T *ptMockQueue = (MOCK_QUEUE_T *)ptQueue;
	
	// Check if enqueue should fail
	if (ptMockQueue->bEnqueueShouldFail)
	{
		return JUNO_STATUS_ERR;
	}
	
	// Check if buffer is full
	if (ptMockQueue->iCount >= 10)
	{
		return JUNO_STATUS_ERR;
	}
	
	// Verify item pointer
	JUNO_STATUS_T tStatus = JunoMemory_PointerVerify(tItem);
	if (tStatus != JUNO_STATUS_SUCCESS)
	{
		return tStatus;
	}
	
	// Copy message to buffer
	tStatus = gtMsgPointerApi.Copy(
		JunoMemory_PointerInit(&gtMsgPointerApi, TEST_MSG_T, &ptMockQueue->tMsgBuffer[ptMockQueue->iTail]),
		tItem
	);
	if (tStatus != JUNO_STATUS_SUCCESS)
	{
		return tStatus;
	}
	
	// Update indices
	ptMockQueue->iTail = (ptMockQueue->iTail + 1) % 10;
	ptMockQueue->iCount++;
	
	return JUNO_STATUS_SUCCESS;
}

/// Dequeue an item from the queue
static JUNO_STATUS_T MockDequeue(JUNO_DS_QUEUE_ROOT_T *ptQueue, JUNO_POINTER_T tReturn)
{
	// Cast to mock queue
	MOCK_QUEUE_T *ptMockQueue = (MOCK_QUEUE_T *)ptQueue;
	
	// Check if buffer is empty
	if (ptMockQueue->iCount == 0)
	{
		return JUNO_STATUS_ERR;
	}
	
	// Verify return pointer
	JUNO_STATUS_T tStatus = JunoMemory_PointerVerify(tReturn);
	if (tStatus != JUNO_STATUS_SUCCESS)
	{
		return tStatus;
	}
	
	// Copy message from buffer
	tStatus = gtMsgPointerApi.Copy(
		tReturn,
		JunoMemory_PointerInit(&gtMsgPointerApi, TEST_MSG_T, &ptMockQueue->tMsgBuffer[ptMockQueue->iHead])
	);
	if (tStatus != JUNO_STATUS_SUCCESS)
	{
		return tStatus;
	}
	
	// Update indices
	ptMockQueue->iHead = (ptMockQueue->iHead + 1) % 10;
	ptMockQueue->iCount--;
	
	return JUNO_STATUS_SUCCESS;
}

/// Peek at the next item in the queue
static JUNO_RESULT_POINTER_T MockPeek(JUNO_DS_QUEUE_ROOT_T *ptQueue)
{
	JUNO_RESULT_POINTER_T tResult = {JUNO_STATUS_ERR, {0}};
	
	// Cast to mock queue
	MOCK_QUEUE_T *ptMockQueue = (MOCK_QUEUE_T *)ptQueue;
	
	// Check if buffer is empty
	if (ptMockQueue->iCount == 0)
	{
		tResult.tStatus = JUNO_STATUS_ERR;
		return tResult;
	}
	
	// Return pointer to head message
	tResult.tStatus = JUNO_STATUS_SUCCESS;
	tResult.tOk = JunoMemory_PointerInit(&gtMsgPointerApi, TEST_MSG_T, &ptMockQueue->tMsgBuffer[ptMockQueue->iHead]);
	
	return tResult;
}

/// Mock array functions (minimal implementation for verification)
static JUNO_STATUS_T MockArraySetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex)
{
	(void)ptArray;
	(void)tItem;
	(void)iIndex;
	// Not used in this mock
	return JUNO_STATUS_SUCCESS;
}

static JUNO_RESULT_POINTER_T MockArrayGetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
	(void)ptArray;
	(void)iIndex;
	JUNO_RESULT_POINTER_T tResult = {JUNO_STATUS_SUCCESS, {0}};
	// Not used in this mock
	return tResult;
}

static JUNO_STATUS_T MockArrayRemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
	(void)ptArray;
	(void)iIndex;
	// Not used in this mock
	return JUNO_STATUS_SUCCESS;
}
