#include "juno/hash/hash_api.h"
#include "juno/map/map_api.h"
#define JUNO_HASH_DEFAULT
#include "juno/hash/hash_djb2.h"
#define JUNO_MAP_DEFAULT
#include "juno/map/map_impl.h"
#include "juno/status.h"
#include "unity.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static JUNO_HASH_T gtHash = {};

static const JUNO_MAP_API_T *gptMapApi = {};

typedef struct
{
	char key[32];
} TEST_KEY;

typedef struct
{
	int value;
} TEST_VALUE;


void setUp(void)
{
	JunoHash_Djb2Api(&gtHash, NULL, NULL);
}
void tearDown(void)
{

}

static bool IsKeyEqual(JUNO_MEMORY_T tKey1, JUNO_MEMORY_T tKey2)
{
	TEST_KEY *ptKey1 = (TEST_KEY*)(tKey1.pvAddr);
	TEST_KEY *ptKey2 = (TEST_KEY*)(tKey2.pvAddr);
	return strcmp(ptKey1->key, ptKey2->key) == 0;
}

static void test_nominal_map(void)
{
	JUNO_MAP_T tMap = {};
	JUNO_MEMORY_T tKeyTbl[10] = {};
	JUNO_MEMORY_T tValTbl[10] = {};
	JUNO_STATUS_T tStatus = JunoMap_ImplApi(
		&tMap,
		&gtHash,
		tKeyTbl,
		tValTbl,
		10,
		IsKeyEqual,
		NULL,
		NULL
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	gptMapApi = tMap.ptApi;
	TEST_KEY tTestKeys[10] = {};
	TEST_VALUE tTestValues[10] = {};
	for(int i = 0; i < 10; i++)
	{
		sprintf(tTestKeys[i].key, "Hello_%i", i+1);
		tTestValues[i].value = i+1;
		tStatus = gptMapApi->Set(
			&tMap,
			(JUNO_MEMORY_T){.pvAddr = &tTestKeys[i], .zSize = sizeof(*tTestKeys)},
			(JUNO_MEMORY_T){.pvAddr = &tTestValues[i], .zSize = sizeof(*tTestValues)}
		);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	TEST_KEY tTestExtra = {.key = "Hello"};
	TEST_VALUE tTestExtraV = {.value = 10};
	tStatus = gptMapApi->Set(
		&tMap,
		(JUNO_MEMORY_T){.pvAddr = &tTestExtra, .zSize = sizeof(TEST_KEY)},
		(JUNO_MEMORY_T){.pvAddr = &tTestExtraV, .zSize = sizeof(TEST_VALUE)}
	);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(int i = 0; i < 10; i++)
	{
		JUNO_MEMORY_T tValue = {};
		tStatus = gptMapApi->Get(
			&tMap,
			(JUNO_MEMORY_T){.pvAddr = &tTestKeys[i], .zSize = sizeof(TEST_KEY)},
			&tValue
		);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_VALUE *ptValue = (TEST_VALUE *)(tValue.pvAddr);
		TEST_ASSERT_EQUAL(i+1, ptValue->value);
	}
	for(int i = 0; i < 10; i++)
	{
		tStatus = gptMapApi->Remove(
			&tMap,
			(JUNO_MEMORY_T){.pvAddr = &tTestKeys[i], .zSize = sizeof(TEST_KEY)}
		);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		JUNO_MEMORY_T tValue = {};
		tStatus = gptMapApi->Get(
			&tMap,
			(JUNO_MEMORY_T){.pvAddr = &tTestKeys[i], .zSize = sizeof(TEST_KEY)},
			&tValue
		);
		TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
}
/* Test: Initialize map with null pointer arguments */
static void test_null_init(void)
{
	JUNO_MAP_T tMap = {};
	JUNO_MEMORY_T tKeyTbl[5] = {};
	JUNO_MEMORY_T tValTbl[5] = {};
	/* Passing NULL for key table */
	JUNO_STATUS_T tStatus = JunoMap_ImplApi(
		&tMap,
		&gtHash,
		NULL,
		tValTbl,
		5,
		IsKeyEqual,
		NULL,
		NULL
	);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	gptMapApi = tMap.ptApi;

	/* Reset and pass NULL for value table */
	tMap = (JUNO_MAP_T){};
	tStatus = JunoMap_ImplApi(
		&tMap,
		&gtHash,
		tKeyTbl,
		NULL,
		5,
		IsKeyEqual,
		NULL,
		NULL
	);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* Test: Set operation with null map pointer */
static void test_null_set(void)
{
	JUNO_MAP_T tMap = {};
	JUNO_MEMORY_T tKeyTbl[10] = {};
	JUNO_MEMORY_T tValTbl[10] = {};
	JUNO_STATUS_T tStatus = JunoMap_ImplApi(
		&tMap,
		&gtHash,
		tKeyTbl,
		tValTbl,
		10,
		IsKeyEqual,
		NULL,
		NULL
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	gptMapApi = tMap.ptApi;
	TEST_KEY tKey = {.key = "Test"};
	TEST_VALUE tValue = {.value = 123};
	/* Call Set with null map pointer */
	tStatus = gptMapApi->Set(
		NULL,
		(JUNO_MEMORY_T){.pvAddr = &tKey, .zSize = sizeof(TEST_KEY)},
		(JUNO_MEMORY_T){.pvAddr = &tValue, .zSize = sizeof(TEST_VALUE)}
	);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* Test: Get operation with null map pointer */
static void test_null_get(void)
{
	JUNO_MAP_T tMap = {};
	JUNO_MEMORY_T tKeyTbl[10] = {};
	JUNO_MEMORY_T tValTbl[10] = {};
	JUNO_STATUS_T tStatus = JunoMap_ImplApi(
		&tMap,
		&gtHash,
		tKeyTbl,
		tValTbl,
		10,
		IsKeyEqual,
		NULL,
		NULL
	);
	gptMapApi = tMap.ptApi;
	TEST_KEY tKey = {.key = "Test"};
	JUNO_MEMORY_T tValue = {};
	tStatus = gptMapApi->Get(
		NULL,
		(JUNO_MEMORY_T){.pvAddr = &tKey, .zSize = sizeof(TEST_KEY)},
		&tValue
	);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* Test: Remove operation with null map pointer */
static void test_null_remove(void)
{
	JUNO_MAP_T tMap = {};
	JUNO_MEMORY_T tKeyTbl[10] = {};
	JUNO_MEMORY_T tValTbl[10] = {};
	JUNO_STATUS_T tStatus = JunoMap_ImplApi(
		&tMap,
		&gtHash,
		tKeyTbl,
		tValTbl,
		10,
		IsKeyEqual,
		NULL,
		NULL
	);
	gptMapApi = tMap.ptApi;
	TEST_KEY tKey = {.key = "Test"};
	tStatus = gptMapApi->Remove(
		NULL,
		(JUNO_MEMORY_T){.pvAddr = &tKey, .zSize = sizeof(TEST_KEY)}
	);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* Test: Remove a key that was never inserted */
static void test_remove_nonexistent_key(void)
{
	JUNO_MAP_T tMap = {};
	JUNO_MEMORY_T tKeyTbl[5] = {};
	JUNO_MEMORY_T tValTbl[5] = {};
	JUNO_STATUS_T tStatus = JunoMap_ImplApi(
		&tMap,
		&gtHash,
		tKeyTbl,
		tValTbl,
		5,
		IsKeyEqual,
		NULL,
		NULL
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	gptMapApi = tMap.ptApi;
	TEST_KEY tKey = {.key = "Nonexistent"};
	tStatus = gptMapApi->Remove(
		&tMap,
		(JUNO_MEMORY_T){.pvAddr = &tKey, .zSize = sizeof(TEST_KEY)}
	);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* Test: Insert duplicate keys, expecting error/duplicate rejection */
static void test_duplicate_key_insertion(void)
{
	JUNO_MAP_T tMap = {};
	JUNO_MEMORY_T tKeyTbl[5] = {};
	JUNO_MEMORY_T tValTbl[5] = {};
	JUNO_STATUS_T tStatus = JunoMap_ImplApi(
		&tMap,
		&gtHash,
		tKeyTbl,
		tValTbl,
		5,
		IsKeyEqual,
		NULL,
		NULL
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	gptMapApi = tMap.ptApi;
	TEST_KEY tKey = {.key = "Duplicate"};
	TEST_VALUE tValue1 = {.value = 1};
	TEST_VALUE tValue2 = {.value = 2};

	/* First insertion should succeed */
	tStatus = gptMapApi->Set(
		&tMap,
		(JUNO_MEMORY_T){.pvAddr = &tKey, .zSize = sizeof(TEST_KEY)},
		(JUNO_MEMORY_T){.pvAddr = &tValue1, .zSize = sizeof(TEST_VALUE)}
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);

	/* Second insertion with same key expected to fail */
	tStatus = gptMapApi->Set(
		&tMap,
		(JUNO_MEMORY_T){.pvAddr = &tKey, .zSize = sizeof(TEST_KEY)},
		(JUNO_MEMORY_T){.pvAddr = &tValue2, .zSize = sizeof(TEST_VALUE)}
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	JUNO_MEMORY_T tRetVal = {};
	tStatus = gptMapApi->Get(
		&tMap,
		(JUNO_MEMORY_T){.pvAddr = &tKey, .zSize = sizeof(TEST_KEY)},
		&tRetVal
	);
	TEST_VALUE *ptRetVal = (TEST_VALUE *)(tRetVal.pvAddr);
	TEST_ASSERT_EQUAL(tValue2.value, ptRetVal->value);
}

/* Test: Overflow the map capacity by inserting more keys than allowed */
static void test_overflow_map(void)
{
	const int capacity = 3;
	JUNO_MAP_T tMap = {};
	JUNO_MEMORY_T tKeyTbl[3] = {};
	JUNO_MEMORY_T tValTbl[3] = {};
	JUNO_STATUS_T tStatus = JunoMap_ImplApi(
		&tMap,
		&gtHash,
		tKeyTbl,
		tValTbl,
		capacity,
		IsKeyEqual,
		NULL,
		NULL
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	gptMapApi = tMap.ptApi;
	TEST_KEY tTestKeys[4] = {};
	TEST_VALUE tTestValues[4] = {};

	/* Insert keys equal to capacity */
	for(int i = 0; i < capacity; i++)
	{
		sprintf(tTestKeys[i].key, "Key_%i", i);
		tTestValues[i].value = i;
		tStatus = gptMapApi->Set(
			&tMap,
			(JUNO_MEMORY_T){.pvAddr = &tTestKeys[i], .zSize = sizeof(*tTestKeys)},
			(JUNO_MEMORY_T){.pvAddr = &tTestValues[i], .zSize = sizeof(*tTestValues)}
		);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	/* Attempt to insert one more key beyond capacity */
	sprintf(tTestKeys[3].key, "Key_%i", 3);
	tTestValues[3].value = 3;
	tStatus = gptMapApi->Set(
		&tMap,
			(JUNO_MEMORY_T){.pvAddr = &tTestKeys[3], .zSize = sizeof(*tTestKeys)},
			(JUNO_MEMORY_T){.pvAddr = &tTestValues[3], .zSize = sizeof(*tTestValues)}
	);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_nominal_map);
	RUN_TEST(test_null_init);
	RUN_TEST(test_null_set);
	RUN_TEST(test_null_get);
	RUN_TEST(test_null_remove);
	RUN_TEST(test_remove_nonexistent_key);
	RUN_TEST(test_duplicate_key_insertion);
	RUN_TEST(test_overflow_map);
	return UNITY_END();
}
