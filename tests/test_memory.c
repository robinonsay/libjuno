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

/**************** Additional Tests for Juno_Memset and Juno_Memcpy ****************/

static void test_juno_memset_positive(void)
{
	// Create a buffer and fill with zero first
	uint8_t buffer[50];
	memset(buffer, 0, sizeof(buffer));
	// Set all 50 bytes to 0xAA
	Juno_Memset(buffer, 0xAA, sizeof(buffer));
	for (size_t i = 0; i < sizeof(buffer); i++)
	{
		TEST_ASSERT_EQUAL_UINT8(0xAA, buffer[i]);
	}
}

static void test_juno_memset_noop(void)
{
	// Create a buffer and fill with a pattern
	uint8_t buffer[20];
	for (size_t i = 0; i < sizeof(buffer); i++)
	{
		buffer[i] = 0x55;
	}
	// Call Juno_Memset with zero size to ensure nothing changes
	Juno_Memset(buffer, 0xFF, 0);
	for (size_t i = 0; i < sizeof(buffer); i++)
	{
		TEST_ASSERT_EQUAL_UINT8(0x55, buffer[i]);
	}
}

static void test_juno_memcpy_positive(void)
{
	// Create source buffer with incremental values and destination buffer initialized to zero.
	uint8_t src[64];
	uint8_t dest[64];
	for (size_t i = 0; i < sizeof(src); i++)
	{
		src[i] = (uint8_t)(i & 0xFF);
		dest[i] = 0;
	}
	// Copy entire buffer from src to dest.
	Juno_Memcpy(dest, src, sizeof(src));
	for (size_t i = 0; i < sizeof(dest); i++)
	{
		TEST_ASSERT_EQUAL_UINT8(src[i], dest[i]);
	}
}

static void test_juno_memcpy_zero_bytes(void)
{
	// Create source and destination buffers filled with different patterns.
	uint8_t src[32], dest[32], original_dest[32];
	memset(src, 0x77, sizeof(src));
	memset(dest, 0x88, sizeof(dest));
	memcpy(original_dest, dest, sizeof(dest));
	// Copy 0 bytes.
	Juno_Memcpy(dest, src, 0);
	// Verify destination remains unchanged.
	for (size_t i = 0; i < sizeof(dest); i++)
	{
		TEST_ASSERT_EQUAL_UINT8(original_dest[i], dest[i]);
	}
}

/**************** New Tests for Unaligned Memory ****************/

static void test_juno_memset_unaligned(void)
{
	// Create a buffer larger than needed and fill with a known pattern.
	uint8_t buffer[60];
	for (size_t i = 0; i < sizeof(buffer); i++)
	{
		buffer[i] = 0x11;
	}
	// Choose an unaligned offset; buffer+1 is typically unaligned for size_t.
	uint8_t *unaligned_ptr = buffer + 1;
	size_t set_size = 30;
	// Set the block of memory at the unaligned pointer to 0xBB.
	Juno_Memset(unaligned_ptr, 0xBB, set_size);
	// Verify the set area was updated.
	for (size_t i = 0; i < set_size; i++)
	{
		TEST_ASSERT_EQUAL_UINT8(0xBB, unaligned_ptr[i]);
	}
	// Verify the bytes before and after the region remain unchanged.
	TEST_ASSERT_EQUAL_UINT8(0x11, buffer[0]);
	for (size_t i = set_size + 1; i < sizeof(buffer); i++)
	{
		TEST_ASSERT_EQUAL_UINT8(0x11, buffer[i]);
	}
}

static void test_juno_memcpy_unaligned(void)
{
	// Create source and destination buffers with extra room.
	uint8_t src[70], dest[70];
	// Initialize source with a pattern and destination with a different pattern.
	for (size_t i = 0; i < sizeof(src); i++)
	{
		src[i] = (uint8_t)((i + 1) & 0xFF);
		dest[i] = 0x33;
	}
	// Use unaligned pointers for both source and destination.
	uint8_t *src_unaligned = src + 1;
	uint8_t *dest_unaligned = dest + 1;
	size_t copy_size = 40;
	// Perform unaligned copy.
	Juno_Memcpy(dest_unaligned, src_unaligned, copy_size);
	// Verify the copied region.
	for (size_t i = 0; i < copy_size; i++)
	{
		TEST_ASSERT_EQUAL_UINT8(src_unaligned[i], dest_unaligned[i]);
	}
	// Verify the bytes outside the region remain unchanged.
	TEST_ASSERT_EQUAL_UINT8(0x33, dest[0]);
	for (size_t i = copy_size + 1; i < sizeof(dest); i++)
	{
		TEST_ASSERT_EQUAL_UINT8(0x33, dest[i]);
	}
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_nominal_single_alloc_and_free);
	RUN_TEST(test_nominal_multiple_alloc_and_free);
	RUN_TEST(test_negative_memory_full);
	RUN_TEST(test_negative_memory_empty);
	RUN_TEST(test_juno_memset_positive);
	RUN_TEST(test_juno_memset_noop);
	RUN_TEST(test_juno_memcpy_positive);
	RUN_TEST(test_juno_memcpy_zero_bytes);
	RUN_TEST(test_juno_memset_unaligned);
	RUN_TEST(test_juno_memcpy_unaligned);
	return UNITY_END();
}
