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

typedef struct TEST_QUEUE JUNO_MODULE_DERIVE(JUNO_BUFF_QUEUE_ROOT_T,
	uint8_t iTestQueue[10];
) TEST_QUEUE;


static inline JUNO_STATUS_T Queue_SetAt(JUNO_BUFF_QUEUE_ROOT_T *ptQueue, void *ptItem, size_t iIndex);
static inline JUNO_RESULT_VOID_PTR_T Queue_GetAt(JUNO_BUFF_QUEUE_ROOT_T *ptQueue, size_t iIndex);
static inline JUNO_STATUS_T Queue_RemoveAt(JUNO_BUFF_QUEUE_ROOT_T *ptQueue, size_t iIndex);
static inline JUNO_STATUS_T Queue_Copy(void *ptDest, void *ptSrc);

const JUNO_BUFF_QUEUE_API_T gtQueueApi = {
	Queue_SetAt,
	Queue_GetAt,
	Queue_RemoveAt,
	Queue_Copy
};

union JUNO_BUFF_QUEUE_ROOT_T JUNO_MODULE(void, JUNO_BUFF_QUEUE_ROOT_T,
	TEST_QUEUE tTestQueue;
);

typedef struct TEST_STACK JUNO_MODULE_DERIVE(JUNO_BUFF_STACK_ROOT_T,
	uint8_t iTestStack[10];
) TEST_STACK;

static inline JUNO_STATUS_T Stack_SetAt(JUNO_BUFF_STACK_ROOT_T *ptStack, void *ptItem, size_t iIndex);
static inline JUNO_RESULT_VOID_PTR_T Stack_GetAt(JUNO_BUFF_STACK_ROOT_T *ptStack, size_t iIndex);
static inline JUNO_STATUS_T Stack_RemoveAt(JUNO_BUFF_STACK_ROOT_T *ptStack, size_t iIndex);
static inline JUNO_STATUS_T Stack_Copy(void *ptDest, void *ptSrc);

const JUNO_BUFF_STACK_API_T gtStackApi = {
	Stack_SetAt,
	Stack_GetAt,
	Stack_RemoveAt,
	Stack_Copy
};

union JUNO_BUFF_STACK_ROOT_T JUNO_MODULE(void, JUNO_BUFF_STACK_ROOT_T,
	TEST_STACK tTestStack;
);

static void test_queue(void)
{
	TEST_QUEUE tTestQueue = {0};
	JUNO_STATUS_T tStatus = JunoDs_Buff_QueueInit(&tTestQueue.tRoot, &gtQueueApi, 10, NULL, NULL);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	TEST_QUEUE tQueue = tTestQueue;
	uint8_t iValue = 0;
	for(size_t i = 0; i < sizeof(tTestQueue.iTestQueue); i++)
	{
		iValue = i + 1;
		tStatus = JunoDs_Buff_QueueEnqueue(&tQueue.tRoot, &iValue);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	tStatus = JunoDs_Buff_QueueEnqueue(&tQueue.tRoot, &iValue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(tTestQueue.iTestQueue); i++)
	{
		tStatus = JunoDs_Buff_QueueDequeue(&tQueue.tRoot, &iValue);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_EQUAL(i+1, iValue);
	}
	tStatus = JunoDs_Buff_QueueDequeue(&tQueue.tRoot, &iValue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(tTestQueue.iTestQueue); i++)
	{
		iValue = i + 1;
		tStatus = JunoDs_Buff_QueueEnqueue(&tQueue.tRoot, &iValue);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	tStatus = JunoDs_Buff_QueueEnqueue(&tQueue.tRoot, &iValue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(tTestQueue.iTestQueue); i++)
	{
		tStatus = JunoDs_Buff_QueueDequeue(&tQueue.tRoot, &iValue);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_EQUAL(i+1, iValue);
	}
	tStatus = JunoDs_Buff_QueueDequeue(&tQueue.tRoot, &iValue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(tTestQueue.iTestQueue); i++)
	{
		iValue = i + 1;
		tStatus = JunoDs_Buff_QueueEnqueue(&tQueue.tRoot, &iValue);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		tStatus = JunoDs_Buff_QueueDequeue(&tQueue.tRoot, &iValue);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_EQUAL(i+1, iValue);
	}
	tStatus = JunoDs_Buff_QueueDequeue(&tQueue.tRoot, &iValue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(tTestQueue.iTestQueue); i++)
	{
		iValue = i + 1;
		tStatus = JunoDs_Buff_QueueEnqueue(&tQueue.tRoot, &iValue);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	tStatus = JunoDs_Buff_QueueEnqueue(&tQueue.tRoot, &iValue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(tTestQueue.iTestQueue); i++)
	{
		tStatus = JunoDs_Buff_QueueDequeue(&tQueue.tRoot, &iValue);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_EQUAL(i+1, iValue);
	}
	tStatus = JunoDs_Buff_QueueDequeue(&tQueue.tRoot, &iValue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	memset(&tTestQueue.iTestQueue, 0, sizeof(tTestQueue.iTestQueue));
	for(size_t i = 0; i < sizeof(tTestQueue.iTestQueue); i++)
	{
		iValue = i + 1;
		tStatus = JunoDs_Buff_QueueEnqueue(&tQueue.tRoot, &iValue);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	for(size_t i = 0; i < sizeof(tTestQueue.iTestQueue)/2; i++)
	{
		tStatus = JunoDs_Buff_QueueDequeue(&tQueue.tRoot, &iValue);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	for(size_t i = 0; i < sizeof(tTestQueue.iTestQueue)/2; i++)
	{
		iValue = i + 1 + sizeof(tTestQueue.iTestQueue)/2;
		tStatus = JunoDs_Buff_QueueEnqueue(&tQueue.tRoot, &iValue);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	tStatus = JunoDs_Buff_QueueEnqueue(&tQueue.tRoot, &iValue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(tTestQueue.iTestQueue); i++)
	{
		tStatus = JunoDs_Buff_QueueDequeue(&tQueue.tRoot, &iValue);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		uint8_t iTruth = (i % (sizeof(tTestQueue.iTestQueue)/2)) + 1 + sizeof(tTestQueue.iTestQueue)/2;
		TEST_ASSERT_EQUAL(iTruth, iValue);
	}
	tStatus = JunoDs_Buff_QueueDequeue(&tQueue.tRoot, &iValue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_stack(void)
{
	TEST_STACK tTestStack = (TEST_STACK){0};
	JUNO_STATUS_T tStatus = JunoDs_Buff_StackInit(&tTestStack.tRoot, &gtStackApi, 10, NULL, NULL);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	TEST_STACK tStack = tTestStack;
	uint8_t iValue = 0;
	for(size_t i = 0; i < sizeof(tTestStack.iTestStack); i++)
	{
		iValue = i + 1;
		tStatus = JunoDs_Buff_StackPush(&tStack.tRoot, &iValue);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	tStatus = JunoDs_Buff_StackPush(&tStack.tRoot, &iValue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(tTestStack.iTestStack); i++)
	{
		iValue = 0;
		tStatus = JunoDs_Buff_StackPop(&tStack.tRoot, &iValue);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_EQUAL(sizeof(tTestStack.iTestStack) - i, iValue);
	}
	tStatus = JunoDs_Buff_StackPop(&tStack.tRoot, &iValue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(tTestStack.iTestStack); i++)
	{
		iValue = i + 1;
		tStatus = JunoDs_Buff_StackPush(&tStack.tRoot, &iValue);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	tStatus = JunoDs_Buff_StackPush(&tStack.tRoot, &iValue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(tTestStack.iTestStack); i++)
	{
		iValue = 0;
		tStatus = JunoDs_Buff_StackPop(&tStack.tRoot, &iValue);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_EQUAL(sizeof(tTestStack.iTestStack) - i, iValue);
	}
	tStatus = JunoDs_Buff_StackPop(&tStack.tRoot, &iValue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_queue);
	RUN_TEST(test_stack);
	return UNITY_END();
}

static inline JUNO_STATUS_T Queue_SetAt(JUNO_BUFF_QUEUE_ROOT_T *ptQueue, void *ptItem, size_t iIndex)
{
	uint8_t *iItem = (void *) ptItem;
	TEST_QUEUE *ptTestQueue = (TEST_QUEUE *) ptQueue;
	ptTestQueue->iTestQueue[iIndex] = *iItem;
	return JUNO_STATUS_SUCCESS;
}

static inline JUNO_RESULT_VOID_PTR_T Queue_GetAt(JUNO_BUFF_QUEUE_ROOT_T *ptQueue, size_t iIndex)
{
	TEST_QUEUE *ptTestQueue = (TEST_QUEUE *) ptQueue;
	JUNO_RESULT_VOID_PTR_T tResult = JUNO_OK_RESULT(&ptTestQueue->iTestQueue[iIndex]);
	return tResult;
}

static inline JUNO_STATUS_T Queue_RemoveAt(JUNO_BUFF_QUEUE_ROOT_T *ptQueue, size_t iIndex)
{
	TEST_QUEUE *ptTestQueue = (TEST_QUEUE *) ptQueue;
	ptTestQueue->iTestQueue[iIndex] = 0;
	return JUNO_STATUS_SUCCESS;
}

static inline JUNO_STATUS_T Queue_Copy(void *ptDest, void *ptSrc)
{
	uint8_t *iDest = (void *) ptDest;
	uint8_t *iSrc= (void *) ptSrc;
	*iDest = *iSrc;
	return JUNO_STATUS_SUCCESS;
}

static inline JUNO_STATUS_T Stack_SetAt(JUNO_BUFF_STACK_ROOT_T *ptStack, void *ptItem, size_t iIndex)
{
	uint8_t *iItem = (void *) ptItem;
	TEST_STACK *ptTestStack = (TEST_STACK *) ptStack;
	ptTestStack->iTestStack[iIndex] = *iItem;
	return JUNO_STATUS_SUCCESS;
}

static inline JUNO_RESULT_VOID_PTR_T Stack_GetAt(JUNO_BUFF_STACK_ROOT_T *ptStack, size_t iIndex)
{
	TEST_STACK *ptTestStack = (TEST_STACK *) ptStack;
	JUNO_RESULT_VOID_PTR_T tResult = JUNO_OK_RESULT(&ptTestStack->iTestStack[iIndex]);
	return tResult;
}

static inline JUNO_STATUS_T Stack_RemoveAt(JUNO_BUFF_STACK_ROOT_T *ptStack, size_t iIndex)
{
	TEST_STACK *ptTestStack = (TEST_STACK *) ptStack;
	ptTestStack->iTestStack[iIndex] = 0;
	return JUNO_STATUS_SUCCESS;
}

static inline JUNO_STATUS_T Stack_Copy(void *ptDest, void *ptSrc)
{
	uint8_t *iDest = (void *) ptDest;
	uint8_t *iSrc= (void *) ptSrc;
	*iDest = *iSrc;
	return JUNO_STATUS_SUCCESS;
}

