#include "juno/memory/memory.h"
#include "juno/memory/memory_types.h"
#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct TEST_BLOCK_TAG
{
	uint32_t iTestNum;
	bool bTestFlag;
} TEST_BLOCK_T;

MEMORY_BLOCK(ptTestBlock, TEST_BLOCK_T, 10);

void setUp(void)
{
	memset(ptTestBlock, 0, sizeof(ptTestBlock));
	memset(MEMORY_FREE_STACK(ptTestBlock), 0, sizeof(MEMORY_FREE_STACK(ptTestBlock)));
}
void tearDown(void)
{

}

static void test_nominal_single_alloc_and_free(void)
{
	JUNO_MEMORY_BLOCK_T tMemBlock = {0};
	JUNO_STATUS_T tStatus = Juno_MemoryBlkInit(
		&tMemBlock,
		ptTestBlock,
		MEMORY_FREE_STACK(ptTestBlock),
		sizeof(TEST_BLOCK_T),
		10,
		NULL,
		NULL
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	JUNO_MEMORY_T tMemory = {0};
	tStatus = Juno_MemoryBlkGet(&tMemBlock, &tMemory);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	TEST_ASSERT_NOT_NULL(tMemory.pvAddr);
	TEST_ASSERT_NOT_EQUAL(0, tMemory.pvAddr);
	tStatus = Juno_MemoryBlkPut(&tMemBlock, &tMemory);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	TEST_ASSERT_NULL(tMemory.pvAddr);
	TEST_ASSERT_EQUAL(0, tMemory.pvAddr);
}

static void test_nominal_multiple_alloc_and_free(void)
{
	JUNO_MEMORY_BLOCK_T tMemBlock = {0};
	JUNO_STATUS_T tStatus = Juno_MemoryBlkInit(
		&tMemBlock,
		ptTestBlock,
		MEMORY_FREE_STACK(ptTestBlock),
		sizeof(TEST_BLOCK_T),
		10,
		NULL,
		NULL
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	TEST_BLOCK_T *ptTestPtr[10] = {0};
	JUNO_MEMORY_T ptMemory[10] = {0};
	for(int j = 0; j < 5; j++)
	{
		for(size_t i = 0; i < 10; i++)
		{
			tStatus = Juno_MemoryBlkGet(&tMemBlock, &ptMemory[i]);
			ptTestPtr[i] = ptMemory[i].pvAddr;
			TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
			TEST_ASSERT_NOT_NULL(ptMemory[i].pvAddr);
			TEST_ASSERT_NOT_EQUAL(0, ptMemory[i].pvAddr);
			ptTestPtr[i]->bTestFlag=true;
			ptTestPtr[i]->iTestNum = i;
		}
		for(size_t i = 0; i < 5; i++)
		{
			tStatus = Juno_MemoryBlkPut(&tMemBlock, &ptMemory[i]);
			TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
			TEST_ASSERT_NULL(ptMemory[i].pvAddr);
			TEST_ASSERT_EQUAL(0, ptMemory[i].pvAddr);
			ptTestPtr[i] = NULL;
		}
		for(size_t i = 0; i < 5; i++)
		{
			tStatus = Juno_MemoryBlkGet(&tMemBlock, &ptMemory[i]);
			ptTestPtr[i] = ptMemory[i].pvAddr;
			TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
			TEST_ASSERT_NOT_NULL(ptMemory[i].pvAddr);
			TEST_ASSERT_NOT_EQUAL(0, ptMemory[i].pvAddr);
		}
		for(size_t i = 0; i < 10; i++)
		{
			tStatus = Juno_MemoryBlkPut(&tMemBlock, &ptMemory[i]);
			TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
			TEST_ASSERT_NULL(ptMemory[i].pvAddr);
			TEST_ASSERT_EQUAL(0, ptMemory[i].pvAddr);
			ptTestPtr[i] = NULL;
		}
	}
}


static void test_negative_memory_empty(void)
{
	JUNO_MEMORY_BLOCK_T tMemBlock = {0};
	JUNO_STATUS_T tStatus = Juno_MemoryBlkInit(
		&tMemBlock,
		ptTestBlock,
		MEMORY_FREE_STACK(ptTestBlock),
		sizeof(TEST_BLOCK_T),
		10,
		NULL,
		NULL
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	JUNO_MEMORY_T tFailMemory = {
		.pvAddr = ptTestBlock,
		.zSize = 128
	};
	tStatus = Juno_MemoryBlkPut(&tMemBlock, &tFailMemory);
	TEST_ASSERT_EQUAL(JUNO_STATUS_MEMFREE_ERROR, tStatus);
	TEST_BLOCK_T *ptTestPtr[10] = {0};
	JUNO_MEMORY_T ptMemory[10] = {0};
	for(size_t i = 0; i < 10; i++)
	{
		tStatus = Juno_MemoryBlkGet(&tMemBlock, &ptMemory[i]);
		ptTestPtr[i] = ptMemory[i].pvAddr;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_NOT_NULL(ptMemory[i].pvAddr);
		TEST_ASSERT_NOT_EQUAL(0, ptMemory[i].pvAddr);
		ptTestPtr[i]->bTestFlag=true;
		ptTestPtr[i]->iTestNum = i;
	}
	for(size_t i = 0; i < 10; i++)
	{
		tStatus = Juno_MemoryBlkPut(&tMemBlock, &ptMemory[i]);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_NULL(ptMemory[i].pvAddr);
		TEST_ASSERT_EQUAL(0, ptMemory[i].pvAddr);
		ptTestPtr[i] = NULL;
	}
	tStatus = Juno_MemoryBlkPut(&tMemBlock, &ptMemory[0]);
	TEST_ASSERT_EQUAL(JUNO_STATUS_MEMFREE_ERROR, tStatus);
}

static void test_negative_memory_full(void)
{
	JUNO_MEMORY_BLOCK_T tMemBlock = {0};
	JUNO_STATUS_T tStatus = Juno_MemoryBlkInit(
		&tMemBlock,
		ptTestBlock,
		MEMORY_FREE_STACK(ptTestBlock),
		sizeof(TEST_BLOCK_T),
		10,
		NULL,
		NULL
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	TEST_BLOCK_T *ptTestPtr[10] = {0};
	JUNO_MEMORY_T ptMemory[10] = {0};
	for(size_t i = 0; i < 10; i++)
	{
		tStatus = Juno_MemoryBlkGet(&tMemBlock, &ptMemory[i]);
		ptTestPtr[i] = ptMemory[i].pvAddr;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_NOT_NULL(ptMemory[i].pvAddr);
		TEST_ASSERT_NOT_EQUAL(0, ptMemory[i].pvAddr);
		ptTestPtr[i]->bTestFlag=true;
		ptTestPtr[i]->iTestNum = i;
	}
	JUNO_MEMORY_T tFailMemory = {0};
	tStatus = Juno_MemoryBlkGet(&tMemBlock, &tFailMemory);
	TEST_ASSERT_EQUAL(JUNO_STATUS_MEMALLOC_ERROR, tStatus);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_nominal_single_alloc_and_free);
	RUN_TEST(test_nominal_multiple_alloc_and_free);
	RUN_TEST(test_negative_memory_full);
	RUN_TEST(test_negative_memory_empty);
	return UNITY_END();
}
