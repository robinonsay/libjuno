#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include "juno/buff/buff_queue_api.h"
#include "juno/buff/buff_stack_api.h"
#include <stdbool.h>
#include <stdint.h>


void setUp(void)
{

}
void tearDown(void)
{

}

static void test_queue(void)
{
	uint8_t iTestQueue[10];
	JUNO_BUFF_QUEUE_T tQueue = {0};

	JUNO_STATUS_T tStatus = JunoBuff_QueueInit(&tQueue, sizeof(iTestQueue), NULL, NULL);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(iTestQueue); i++)
	{
		size_t iIndex = 0;
		tStatus = JunoBuff_QueueEnqueue(&tQueue, &iIndex);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		iTestQueue[iIndex] = i+1;
	}
	size_t iIndex = 0;
	tStatus = JunoBuff_QueueEnqueue(&tQueue, &iIndex);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(iTestQueue); i++)
	{
		iIndex = 0;
		tStatus = JunoBuff_QueueDequeue(&tQueue, &iIndex);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_EQUAL(i+1, iTestQueue[iIndex]);
	}
	tStatus = JunoBuff_QueueDequeue(&tQueue, &iIndex);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(iTestQueue); i++)
	{
		iIndex = 0;
		tStatus = JunoBuff_QueueEnqueue(&tQueue, &iIndex);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		iTestQueue[iIndex] = i+1;
	}
	iIndex = 0;
	tStatus = JunoBuff_QueueEnqueue(&tQueue, &iIndex);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(iTestQueue); i++)
	{
		iIndex = 0;
		tStatus = JunoBuff_QueueDequeue(&tQueue, &iIndex);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_EQUAL(i+1, iTestQueue[iIndex]);
	}
	tStatus = JunoBuff_QueueDequeue(&tQueue, &iIndex);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(iTestQueue); i++)
	{
		iIndex = 0;
		tStatus = JunoBuff_QueueEnqueue(&tQueue, &iIndex);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		iTestQueue[iIndex] = i+1;
		tStatus = JunoBuff_QueueDequeue(&tQueue, &iIndex);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_EQUAL(i+1, iTestQueue[iIndex]);
	}
	tStatus = JunoBuff_QueueDequeue(&tQueue, &iIndex);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(iTestQueue); i++)
	{
		iIndex = 0;
		tStatus = JunoBuff_QueueEnqueue(&tQueue, &iIndex);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		iTestQueue[iIndex] = i+1;
	}
	iIndex = 0;
	tStatus = JunoBuff_QueueEnqueue(&tQueue, &iIndex);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(iTestQueue); i++)
	{
		iIndex = 0;
		tStatus = JunoBuff_QueueDequeue(&tQueue, &iIndex);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_EQUAL(i+1, iTestQueue[iIndex]);
	}
	tStatus = JunoBuff_QueueDequeue(&tQueue, &iIndex);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_stack(void)
{
	uint8_t iTestStack[10];
	JUNO_BUFF_STACK_T tStack = {0};

	JUNO_STATUS_T tStatus = JunoBuff_StackInit(&tStack, sizeof(iTestStack), NULL, NULL);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(iTestStack); i++)
	{
		size_t iIndex = 0;
		tStatus = JunoBuff_StackPush(&tStack, &iIndex);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		iTestStack[iIndex] = i+1;
	}
	size_t iIndex = 0;
	tStatus = JunoBuff_StackPush(&tStack, &iIndex);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(iTestStack); i++)
	{
		iIndex = 0;
		tStatus = JunoBuff_StackPop(&tStack, &iIndex);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_EQUAL(sizeof(iTestStack) - i, iTestStack[iIndex]);
	}
	tStatus = JunoBuff_StackPop(&tStack, &iIndex);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(iTestStack); i++)
	{
		iIndex = 0;
		tStatus = JunoBuff_StackPush(&tStack, &iIndex);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		iTestStack[iIndex] = i+1;
	}
	iIndex = 0;
	tStatus = JunoBuff_StackPush(&tStack, &iIndex);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < sizeof(iTestStack); i++)
	{
		iIndex = 0;
		tStatus = JunoBuff_StackPop(&tStack, &iIndex);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_EQUAL(sizeof(iTestStack) - i, iTestStack[iIndex]);
	}
	tStatus = JunoBuff_StackPop(&tStack, &iIndex);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_queue);
	RUN_TEST(test_stack);
	return UNITY_END();
}
