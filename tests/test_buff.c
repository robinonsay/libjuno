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

union JUNO_BUFF_QUEUE_T JUNO_MODULE(void, JUNO_BUFF_QUEUE_ROOT_T,
);

union JUNO_BUFF_STACK_T JUNO_MODULE(void, JUNO_BUFF_STACK_ROOT_T,
);

static void test_queue(void)
{
	uint8_t iTestQueue[10] = {0};
	JUNO_BUFF_QUEUE_ROOT_T tQueueRoot = {0};
	JUNO_RESULT_SIZE_T tResult = {0};
	JUNO_BUFF_QUEUE_T tQueue = {.tRoot = tQueueRoot};
	tResult.tStatus = JunoBuff_QueueInit(&tQueue, sizeof(iTestQueue), NULL, NULL);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
	for(size_t i = 0; i < sizeof(iTestQueue); i++)
	{
		size_t iIndex = 0;
		tResult = JunoBuff_QueueEnqueue(&tQueue);
		iIndex = tResult.tSuccess;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
		iTestQueue[iIndex] = i+1;
	}
	size_t iIndex = 0;
	tResult = JunoBuff_QueueEnqueue(&tQueue);
	iIndex = tResult.tSuccess;
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
	for(size_t i = 0; i < sizeof(iTestQueue); i++)
	{
		iIndex = 0;
		tResult = JunoBuff_QueueDequeue(&tQueue);
		iIndex = tResult.tSuccess;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
		TEST_ASSERT_EQUAL(i+1, iTestQueue[iIndex]);
	}
	tResult = JunoBuff_QueueDequeue(&tQueue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
	for(size_t i = 0; i < sizeof(iTestQueue); i++)
	{
		iIndex = 0;
		tResult = JunoBuff_QueueEnqueue(&tQueue);
		iIndex = tResult.tSuccess;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
		iTestQueue[iIndex] = i+1;
	}
	iIndex = 0;
	tResult = JunoBuff_QueueEnqueue(&tQueue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
	for(size_t i = 0; i < sizeof(iTestQueue); i++)
	{
		iIndex = 0;
		tResult = JunoBuff_QueueDequeue(&tQueue);
		iIndex = tResult.tSuccess;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
		TEST_ASSERT_EQUAL(i+1, iTestQueue[iIndex]);
	}
	tResult = JunoBuff_QueueDequeue(&tQueue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
	for(size_t i = 0; i < sizeof(iTestQueue); i++)
	{
		iIndex = 0;
		tResult = JunoBuff_QueueEnqueue(&tQueue);
		iIndex = tResult.tSuccess;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
		iTestQueue[iIndex] = i+1;
		tResult = JunoBuff_QueueDequeue(&tQueue);
		iIndex = tResult.tSuccess;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
		TEST_ASSERT_EQUAL(i+1, iTestQueue[iIndex]);
	}
	tResult = JunoBuff_QueueDequeue(&tQueue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
	for(size_t i = 0; i < sizeof(iTestQueue); i++)
	{
		iIndex = 0;
		tResult = JunoBuff_QueueEnqueue(&tQueue);
		iIndex = tResult.tSuccess;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
		iTestQueue[iIndex] = i+1;
	}
	iIndex = 0;
	tResult = JunoBuff_QueueEnqueue(&tQueue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
	for(size_t i = 0; i < sizeof(iTestQueue); i++)
	{
		iIndex = 0;
		tResult = JunoBuff_QueueDequeue(&tQueue);
		iIndex = tResult.tSuccess;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
		TEST_ASSERT_EQUAL(i+1, iTestQueue[iIndex]);
	}
	tResult = JunoBuff_QueueDequeue(&tQueue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
	memset(iTestQueue, 0, sizeof(iTestQueue));
	for(size_t i = 0; i < sizeof(iTestQueue); i++)
	{
		iIndex = 0;
		tResult = JunoBuff_QueueEnqueue(&tQueue);
		iIndex = tResult.tSuccess;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
		iTestQueue[iIndex] = i+1;
	}
	for(size_t i = 0; i < sizeof(iTestQueue)/2; i++)
	{
		iIndex = 0;
		tResult = JunoBuff_QueueDequeue(&tQueue);
		iIndex = tResult.tSuccess;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
		TEST_ASSERT_EQUAL(i+1, iTestQueue[iIndex]);
		iTestQueue[iIndex] = 0;
	}
	for(size_t i = 0; i < sizeof(iTestQueue)/2; i++)
	{
		iIndex = 0;
		tResult = JunoBuff_QueueEnqueue(&tQueue);
		iIndex = tResult.tSuccess;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
		iTestQueue[iIndex] = i + 1+ sizeof(iTestQueue)/2;
	}
	iIndex = 0;
	tResult = JunoBuff_QueueEnqueue(&tQueue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
	for(size_t i = 0; i < sizeof(iTestQueue); i++)
	{
		iIndex = 0;
		tResult = JunoBuff_QueueDequeue(&tQueue);
		iIndex = tResult.tSuccess;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
		uint8_t iTruth = (i % (sizeof(iTestQueue)/2)) + 1 + sizeof(iTestQueue)/2;
		TEST_ASSERT_EQUAL(iTruth, iTestQueue[iIndex]);
		iTestQueue[iIndex] = 0;
	}
	tResult = JunoBuff_QueueDequeue(&tQueue);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
}

static void test_stack(void)
{
	uint8_t iTestStack[10];
	JUNO_BUFF_STACK_ROOT_T tStackRoot = {0};
	JUNO_RESULT_SIZE_T tResult = {0};
	JUNO_BUFF_STACK_T tStack = {.tRoot = tStackRoot};
	tResult.tStatus = JunoBuff_StackInit(&tStack, sizeof(iTestStack), NULL, NULL);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
	for(size_t i = 0; i < sizeof(iTestStack); i++)
	{
		size_t iIndex = 0;
		tResult = JunoBuff_StackPush(&tStack);
		iIndex = tResult.tSuccess;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
		iTestStack[iIndex] = i+1;
	}
	size_t iIndex = 0;
	tResult = JunoBuff_StackPush(&tStack);
	iIndex = tResult.tSuccess;
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
	for(size_t i = 0; i < sizeof(iTestStack); i++)
	{
		iIndex = 0;
		tResult = JunoBuff_StackPop(&tStack);
		iIndex = tResult.tSuccess;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
		TEST_ASSERT_EQUAL(sizeof(iTestStack) - i, iTestStack[iIndex]);
	}
	tResult = JunoBuff_StackPop(&tStack);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
	for(size_t i = 0; i < sizeof(iTestStack); i++)
	{
		iIndex = 0;
		tResult = JunoBuff_StackPush(&tStack);
		iIndex = tResult.tSuccess;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
		iTestStack[iIndex] = i+1;
	}
	iIndex = 0;
	tResult = JunoBuff_StackPush(&tStack);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
	for(size_t i = 0; i < sizeof(iTestStack); i++)
	{
		iIndex = 0;
		tResult = JunoBuff_StackPop(&tStack);
		iIndex = tResult.tSuccess;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
		TEST_ASSERT_EQUAL(sizeof(iTestStack) - i, iTestStack[iIndex]);
	}
	tResult = JunoBuff_StackPop(&tStack);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_queue);
	RUN_TEST(test_stack);
	return UNITY_END();
}
