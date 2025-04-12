#include "juno/memory/memory_types.h"
#include "juno/status.h"
#include "juno/string/string_types.h"
#include "unity.h"
#include "unity_internals.h"
#include <stdbool.h>
#include <string.h>
#include "juno/memory/memory.h"
#include "juno/string/string.h"


typedef struct
{
	char data[256];
} TEST_DATA_T;

JUNO_MEMORY_BLOCK(ptStringMemory, TEST_DATA_T, 10);
JUNO_MEMORY_METADATA(ptStringMemoryMetadata, 10);
JUNO_MEMORY_ALLOC_T gtMemoryBlock = {0};

void setUp(void)
{
	memset(ptStringMemory, 0, sizeof(ptStringMemory));
	memset(&gtMemoryBlock, 0, sizeof(gtMemoryBlock));
	Juno_MemoryBlkInit(
		&gtMemoryBlock.tBlock,
		ptStringMemory,
		ptStringMemoryMetadata,
		sizeof(TEST_DATA_T),
		10,
		NULL,
		NULL
	);
}
void tearDown(void)
{

}

static void test_nominal_string(void)
{
	JUNO_STRING_T tStr = {0};
	const char *pcTestStr = "Hello World!";
	JUNO_STATUS_T tStatus = Juno_StringInit(
		&tStr,
		&gtMemoryBlock,
		pcTestStr,
		strlen(pcTestStr),
		NULL,
		NULL
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	int i = 0;
	for(const char *c=pcTestStr; *c;c++)
	{
		const char *pcChar = tStr.tMemory.pvAddr;
		TEST_ASSERT_EQUAL(pcTestStr[i], pcChar[i]);
		i += 1;
	}
	tStatus = Juno_StringFree(&tStr);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_nominal_concat(void)
{
	JUNO_STRING_T tStr[2] = {0};
	const char *pcTestStr = "Hello ";
	const char *pcTestStr2 = "World!";
	JUNO_STATUS_T tStatus = Juno_StringInit(
		&tStr[0],
		&gtMemoryBlock,
		pcTestStr,
		strlen(pcTestStr),
		NULL,
		NULL
	);
	tStatus = Juno_StringInit(
		&tStr[1],
		&gtMemoryBlock,
		pcTestStr2,
		strlen(pcTestStr2),
		NULL,
		NULL
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	tStatus = Juno_StringConcat(&tStr[0], &tStr[1]);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	int i = 0;
	const char *pcTruthStr = "Hello World!";
	for(const char *c=pcTruthStr; *c;c++)
	{
		const char *pcChar = tStr[0].tMemory.pvAddr;
		TEST_ASSERT_EQUAL(pcTruthStr[i], pcChar[i]);
		i += 1;
	}
	tStatus = Juno_StringFree(&tStr[0]);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	tStatus = Juno_StringFree(&tStr[1]);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_nominal_append(void)
{
	JUNO_STRING_T tStr = {0};
	const char *pcTestStr = "Hello ";
	const char *pcTestStr2 = "World!";
	JUNO_STATUS_T tStatus = Juno_StringInit(
		&tStr,
		&gtMemoryBlock,
		pcTestStr,
		strlen(pcTestStr),
		NULL,
		NULL
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	tStatus = Juno_StringAppend(&tStr, pcTestStr2, strlen(pcTestStr2));
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	int i = 0;
	const char *pcTruthStr = "Hello World!";
	for(const char *c=pcTruthStr; *c;c++)
	{
		const char *pcChar = tStr.tMemory.pvAddr;
		TEST_ASSERT_EQUAL(pcTruthStr[i], pcChar[i]);
		i += 1;
	}
	tStatus = Juno_StringFree(&tStr);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}


int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_nominal_string);
	RUN_TEST(test_nominal_concat);
	RUN_TEST(test_nominal_append);
	return UNITY_END();
}
