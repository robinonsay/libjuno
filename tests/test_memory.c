#include "juno/memory/memory.h"
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
	void *ptTestPtr = NULL;
	tStatus = Juno_MemoryBlkGet(&tMemBlock, &ptTestPtr);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	tStatus = Juno_MemoryBlkPut(&tMemBlock, ptTestPtr);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
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
	for(int j = 0; j < 5; j++)
	{
		for(size_t i = 0; i < 10; i++)
		{
			void *pvMem = NULL;
			tStatus = Juno_MemoryBlkGet(&tMemBlock, &pvMem);
			ptTestPtr[i] = pvMem;
			TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
			ptTestPtr[i]->bTestFlag=true;
			ptTestPtr[i]->iTestNum = i;
		}
		for(size_t i = 0; i < 5; i++)
		{
			tStatus = Juno_MemoryBlkPut(&tMemBlock, ptTestPtr[i]);
			TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
			ptTestPtr[i] = NULL;
		}
		for(size_t i = 0; i < 5; i++)
		{
			void *pvMem = NULL;
			tStatus = Juno_MemoryBlkGet(&tMemBlock, &pvMem);
			ptTestPtr[i] = pvMem;
			TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		}
		for(size_t i = 0; i < 10; i++)
		{
			tStatus = Juno_MemoryBlkPut(&tMemBlock, ptTestPtr[i]);
			TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
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

	tStatus = Juno_MemoryBlkPut(&tMemBlock, ptTestBlock);
	TEST_ASSERT_EQUAL(JUNO_STATUS_MEMFREE_ERROR, tStatus);
	TEST_BLOCK_T *ptTestPtr[10] = {0};
	for(size_t i = 0; i < 10; i++)
	{
		void *pvMem = NULL;
		tStatus = Juno_MemoryBlkGet(&tMemBlock, &pvMem);
		ptTestPtr[i] = pvMem;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		ptTestPtr[i]->bTestFlag=true;
		ptTestPtr[i]->iTestNum = i;
	}
	for(size_t i = 0; i < 10; i++)
	{
		tStatus = Juno_MemoryBlkPut(&tMemBlock, ptTestPtr[i]);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	tStatus = Juno_MemoryBlkPut(&tMemBlock, ptTestPtr[0]);
	TEST_ASSERT_EQUAL(JUNO_STATUS_MEMFREE_ERROR, tStatus);
	tStatus = Juno_MemoryBlkPut(&tMemBlock, ptTestPtr);
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
	for(size_t i = 0; i < 10; i++)
	{
		void *pvMem = NULL;
		tStatus = Juno_MemoryBlkGet(&tMemBlock, &pvMem);
		ptTestPtr[i] = pvMem;
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		ptTestPtr[i]->bTestFlag=true;
		ptTestPtr[i]->iTestNum = i;
	}
	void *pvMem = NULL;
	tStatus = Juno_MemoryBlkGet(&tMemBlock, &pvMem);
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
