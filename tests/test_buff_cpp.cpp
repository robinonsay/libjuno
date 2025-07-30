#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include "juno/buff/buff_api.hpp"
#include <cstddef>
#include <cstdint>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>


void setUp(void)
{

}
void tearDown(void)
{

}



static void test_queue(void)
{
	using BUFF_T = JUNO_BUFF_QUEUE_T<uint32_t, 10>;
	JUNO_ARRAY_T<uint32_t, 10> tArrBuff{};
	auto tTestQueueResult = BUFF_T::New(tArrBuff, NULL, NULL);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tTestQueueResult.tStatus);
	auto tTestQueue = tTestQueueResult.tSuccess;
	for(size_t i = 0; i < tTestQueue.tRoot.zCapacity; i++)
	{
		auto tStatus = tTestQueue.Enqueue(i+1);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	auto tStatus = tTestQueue.Enqueue(11);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < tTestQueue.tRoot.zCapacity; i++)
	{
		auto tResult = tTestQueue.Dequeue();
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
		TEST_ASSERT_EQUAL(i+1, tResult.tSuccess);
	}
}

static void test_stack(void)
{
	using STACK_T = JUNO_BUFF_STACK_T<uint32_t, 10>;
	auto tTestQueueResult = STACK_T::New(NULL, NULL);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tTestQueueResult.tStatus);
	auto tTestStack = tTestQueueResult.tSuccess;
	for(size_t i = 0; i < tTestStack.tRoot.zCapacity; i++)
	{
		auto tStatus = tTestStack.Push(i+1);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	auto tStatus = tTestStack.Push(11);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(size_t i = 0; i < tTestStack.tRoot.zCapacity; i++)
	{
		auto tResult = tTestStack.Pop();
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
		TEST_ASSERT_EQUAL(tTestStack.tRoot.zCapacity - i, tResult.tSuccess);
	}
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_queue);
	RUN_TEST(test_stack);
	return UNITY_END();
}
