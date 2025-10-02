#include "juno/memory/memory_api.h"
#include "juno/module.h"
#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include "juno/ds/buff_queue_api.h"
#include "juno/ds/buff_stack_api.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>


void setUp(void)
{

}
void tearDown(void)
{

}



typedef struct TEST_ARRAY JUNO_MODULE_DERIVE(JUNO_ARRAY_ROOT_T,
	uint8_t iTestQueue[10];
) TEST_ARRAY;


static inline JUNO_STATUS_T Queue_SetAt(JUNO_ARRAY_ROOT_T *ptQueue, JUNO_POINTER_T tItem, size_t iIndex);
static inline JUNO_RESULT_POINTER_T Queue_GetAt(JUNO_ARRAY_ROOT_T *ptQueue, size_t iIndex);
static inline JUNO_STATUS_T Queue_RemoveAt(JUNO_ARRAY_ROOT_T *ptQueue, size_t iIndex);
static inline JUNO_STATUS_T Queue_Copy(JUNO_POINTER_T tDest, JUNO_POINTER_T tSrc);
static inline JUNO_STATUS_T Queue_Reset(JUNO_POINTER_T tDest);

const JUNO_ARRAY_API_T gtQueueApi = {
	Queue_SetAt,
	Queue_GetAt,
	Queue_RemoveAt,
};

const JUNO_POINTER_API_T gtPointerApi = {
	.Copy = Queue_Copy,
	.Reset = Queue_Reset,
};

union JUNO_ARRAY_ROOT_T JUNO_MODULE(void, JUNO_ARRAY_ROOT_T,
	TEST_ARRAY tTestQueue;
);

static void test_queue(void)
{
	// Prepare the backing array (buffer)
	TEST_ARRAY tBuffer = (TEST_ARRAY){0};
	JUNO_ARRAY_ROOT_T *ptArray = (JUNO_ARRAY_ROOT_T *)&tBuffer;

	// Initialize the array root fields
	ptArray->ptApi        = &gtQueueApi;
	ptArray->ptPointerApi = &gtPointerApi;
	ptArray->zCapacity    = sizeof(tBuffer.iTestQueue);
	ptArray->zLength      = 0;

	// Initialize the queue with the backing array
	JUNO_BUFF_QUEUE_ROOT_T tQueue = (JUNO_BUFF_QUEUE_ROOT_T){0};
	JUNO_STATUS_T tStatus = JunoDs_Buff_QueueInit(&tQueue, ptArray, NULL, NULL);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	uint8_t iValue = 0;
	JUNO_POINTER_T tValPtr = Juno_PointerInit(&gtPointerApi, uint8_t, &iValue);

	// Fill the queue
	for (size_t i = 0; i < sizeof(tBuffer.iTestQueue); i++)
	{
		iValue = (uint8_t)(i + 1);
		tStatus = tQueue.ptApi->Enqueue(&tQueue, tValPtr);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	// One too many
	tStatus = tQueue.ptApi->Enqueue(&tQueue, tValPtr);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);

	// Drain the queue
	for (size_t i = 0; i < sizeof(tBuffer.iTestQueue); i++)
	{
		tStatus = tQueue.ptApi->Dequeue(&tQueue, tValPtr);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_EQUAL(i + 1, iValue);
	}
	// One too many
	tStatus = tQueue.ptApi->Dequeue(&tQueue, tValPtr);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);

	// Fill and drain again
	for (size_t i = 0; i < sizeof(tBuffer.iTestQueue); i++)
	{
		iValue = (uint8_t)(i + 1);
		tStatus = tQueue.ptApi->Enqueue(&tQueue, tValPtr);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	tStatus = tQueue.ptApi->Enqueue(&tQueue, tValPtr);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for (size_t i = 0; i < sizeof(tBuffer.iTestQueue); i++)
	{
		tStatus = tQueue.ptApi->Dequeue(&tQueue, tValPtr);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_EQUAL(i + 1, iValue);
	}
	tStatus = tQueue.ptApi->Dequeue(&tQueue, tValPtr);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);

	// Interleave enqueue/dequeue
	for (size_t i = 0; i < sizeof(tBuffer.iTestQueue); i++)
	{
		iValue = (uint8_t)(i + 1);
		tStatus = tQueue.ptApi->Enqueue(&tQueue, tValPtr);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		tStatus = tQueue.ptApi->Dequeue(&tQueue, tValPtr);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_EQUAL(i + 1, iValue);
	}
	tStatus = tQueue.ptApi->Dequeue(&tQueue, tValPtr);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);

	// Fill, half-drain, wrap-around, then drain all
	for (size_t i = 0; i < sizeof(tBuffer.iTestQueue); i++)
	{
		iValue = (uint8_t)(i + 1);
		tStatus = tQueue.ptApi->Enqueue(&tQueue, tValPtr);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	tStatus = tQueue.ptApi->Enqueue(&tQueue, tValPtr);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for (size_t i = 0; i < sizeof(tBuffer.iTestQueue); i++)
	{
		tStatus = tQueue.ptApi->Dequeue(&tQueue, tValPtr);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_EQUAL(i + 1, iValue);
	}
	tStatus = tQueue.ptApi->Dequeue(&tQueue, tValPtr);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	memset(&tBuffer.iTestQueue, 0, sizeof(tBuffer.iTestQueue));
	for (size_t i = 0; i < sizeof(tBuffer.iTestQueue); i++)
	{
		iValue = (uint8_t)(i + 1);
		tStatus = tQueue.ptApi->Enqueue(&tQueue, tValPtr);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	for (size_t i = 0; i < sizeof(tBuffer.iTestQueue) / 2; i++)
	{
		tStatus = tQueue.ptApi->Dequeue(&tQueue, tValPtr);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	for (size_t i = 0; i < sizeof(tBuffer.iTestQueue) / 2; i++)
	{
		iValue = (uint8_t)(i + 1 + sizeof(tBuffer.iTestQueue) / 2);
		tStatus = tQueue.ptApi->Enqueue(&tQueue, tValPtr);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	tStatus = tQueue.ptApi->Enqueue(&tQueue, tValPtr);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for (size_t i = 0; i < sizeof(tBuffer.iTestQueue); i++)
	{
		tStatus = tQueue.ptApi->Dequeue(&tQueue, tValPtr);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		uint8_t iTruth = (uint8_t)((i % (sizeof(tBuffer.iTestQueue) / 2)) + 1 + sizeof(tBuffer.iTestQueue) / 2);
		TEST_ASSERT_EQUAL(iTruth, iValue);
	}
	tStatus = tQueue.ptApi->Dequeue(&tQueue, tValPtr);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_queue);
	return UNITY_END();
}

static inline JUNO_STATUS_T Queue_SetAt(JUNO_ARRAY_ROOT_T *ptQueue, JUNO_POINTER_T tItem, size_t iIndex)
{
	uint8_t *iItem = (uint8_t *) tItem.pvAddr;
	TEST_ARRAY *ptTestQueue = (TEST_ARRAY *) ptQueue;
	ptTestQueue->iTestQueue[iIndex] = *iItem;
	return JUNO_STATUS_SUCCESS;
}

static inline JUNO_RESULT_POINTER_T Queue_GetAt(JUNO_ARRAY_ROOT_T *ptQueue, size_t iIndex)
{
	TEST_ARRAY *ptTestQueue = (TEST_ARRAY *) ptQueue;
	JUNO_RESULT_POINTER_T tResult = JUNO_OK_RESULT(Juno_PointerInit(&gtPointerApi, uint8_t, &ptTestQueue->iTestQueue[iIndex]));
	return tResult;
}

static inline JUNO_STATUS_T Queue_RemoveAt(JUNO_ARRAY_ROOT_T *ptQueue, size_t iIndex)
{
	TEST_ARRAY *ptTestQueue = (TEST_ARRAY *) ptQueue;
	ptTestQueue->iTestQueue[iIndex] = 0;
	return JUNO_STATUS_SUCCESS;
}

static inline JUNO_STATUS_T Queue_Copy(JUNO_POINTER_T tDest, JUNO_POINTER_T tSrc)
{
	uint8_t *iDest = (uint8_t *) tDest.pvAddr;
	uint8_t *iSrc = (uint8_t *) tSrc.pvAddr;
	*iDest = *iSrc;
	return JUNO_STATUS_SUCCESS;
}

static inline JUNO_STATUS_T Queue_Reset(JUNO_POINTER_T tDest)
{
	uint8_t *iDest = (uint8_t *) tDest.pvAddr;
	*iDest = 0;
	return JUNO_STATUS_SUCCESS;
}

