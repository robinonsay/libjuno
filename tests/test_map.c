#include "juno/hash/hash_api.h"
#include "juno/map/map_types.h"
#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "juno/map/map_api.h"

static const JUNO_MAP_API_T *gptMapApi = NULL;
static const JUNO_HASH_API_T *gptHashApi = NULL;

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
	gptMapApi = Juno_MapApi();
	gptHashApi = Juno_HashDjB2Api();
}
void tearDown(void)
{

}

static bool IsKeyEqual(JUNO_MAP_KEY_T tKey1, JUNO_MAP_KEY_T tKey2)
{
	TEST_KEY *ptKey1 = (TEST_KEY*)(tKey1.ptKey);
	TEST_KEY *ptKey2 = (TEST_KEY*)(tKey2.ptKey);
	return strcmp(ptKey1->key, ptKey2->key) == 0;
}

static void test_nominal_map(void)
{
	JUNO_MAP_T tMap = {};
	JUNO_MAP_KEY_T tKeyTbl[10] = {};
	JUNO_MAP_VALUE_T tValTbl[10] = {};
	JUNO_STATUS_T tStatus = gptMapApi->Init(
		&tMap,
		gptHashApi,
		tKeyTbl,
		tValTbl,
		sizeof(TEST_KEY),
		10,
		IsKeyEqual,
		NULL,
		NULL
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	TEST_KEY tTestKeys[10] = {};
	TEST_VALUE tTestValues[10] = {};
	for(int i = 0; i < 10; i++)
	{
		sprintf(tTestKeys[i].key, "Hello_%i", i+1);
		tTestValues[i].value = i+1;
		tStatus = gptMapApi->Set(
			&tMap,
			(JUNO_MAP_KEY_T){.ptKey = &tTestKeys[i]},
			(JUNO_MAP_VALUE_T){.ptValue = &tTestValues[i]}
		);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	TEST_KEY tTestExtra = {.key = "Hello"};
	TEST_VALUE tTestExtraV = {.value = 10};
	tStatus = gptMapApi->Set(
		&tMap,
		(JUNO_MAP_KEY_T){.ptKey = &tTestExtra},
		(JUNO_MAP_VALUE_T){.ptValue = &tTestExtraV}
	);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for(int i = 0; i < 10; i++)
	{
		JUNO_MAP_VALUE_T tValue = {};
		tStatus = gptMapApi->Get(
			&tMap,
			(JUNO_MAP_KEY_T){.ptKey = &tTestKeys[i]},
			&tValue
		);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_VALUE *ptValue = (TEST_VALUE *)(tValue.ptValue);
		TEST_ASSERT_EQUAL(i+1, ptValue->value);
	}
	for(int i = 0; i < 10; i++)
	{
		tStatus = gptMapApi->Remove(
			&tMap,
			(JUNO_MAP_KEY_T){.ptKey = &tTestKeys[i]}
		);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		JUNO_MAP_VALUE_T tValue = {};
		tStatus = gptMapApi->Get(
			&tMap,
			(JUNO_MAP_KEY_T){.ptKey = &tTestKeys[i]},
			&tValue
		);
		TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
}
/* Test: Initialize map with null pointer arguments */
static void test_null_init(void)
{
	JUNO_MAP_T tMap = {};
	JUNO_MAP_KEY_T tKeyTbl[5] = {};
	JUNO_MAP_VALUE_T tValTbl[5] = {};
	/* Passing NULL for key table */
	JUNO_STATUS_T tStatus = gptMapApi->Init(
		&tMap,
		gptHashApi,
		NULL,
		tValTbl,
		sizeof(TEST_KEY),
		5,
		IsKeyEqual,
		NULL,
		NULL
	);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);

	/* Reset and pass NULL for value table */
	tMap = (JUNO_MAP_T){};
	tStatus = gptMapApi->Init(
		&tMap,
		gptHashApi,
		tKeyTbl,
		NULL,
		sizeof(TEST_KEY),
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
	TEST_KEY tKey = {.key = "Test"};
	TEST_VALUE tValue = {.value = 123};
	/* Call Set with null map pointer */
	JUNO_STATUS_T tStatus = gptMapApi->Set(
		NULL,
		(JUNO_MAP_KEY_T){.ptKey = &tKey},
		(JUNO_MAP_VALUE_T){.ptValue = &tValue}
	);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* Test: Get operation with null map pointer */
static void test_null_get(void)
{
	TEST_KEY tKey = {.key = "Test"};
	JUNO_MAP_VALUE_T tValue = {};
	JUNO_STATUS_T tStatus = gptMapApi->Get(
		NULL,
		(JUNO_MAP_KEY_T){.ptKey = &tKey},
		&tValue
	);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* Test: Remove operation with null map pointer */
static void test_null_remove(void)
{
	TEST_KEY tKey = {.key = "Test"};
	JUNO_STATUS_T tStatus = gptMapApi->Remove(
		NULL,
		(JUNO_MAP_KEY_T){.ptKey = &tKey}
	);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* Test: Remove a key that was never inserted */
static void test_remove_nonexistent_key(void)
{
	JUNO_MAP_T tMap = {};
	JUNO_MAP_KEY_T tKeyTbl[5] = {};
	JUNO_MAP_VALUE_T tValTbl[5] = {};
	JUNO_STATUS_T tStatus = gptMapApi->Init(
		&tMap,
		gptHashApi,
		tKeyTbl,
		tValTbl,
		sizeof(TEST_KEY),
		5,
		IsKeyEqual,
		NULL,
		NULL
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	TEST_KEY tKey = {.key = "Nonexistent"};
	tStatus = gptMapApi->Remove(
		&tMap,
		(JUNO_MAP_KEY_T){.ptKey = &tKey}
	);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* Test: Insert duplicate keys, expecting error/duplicate rejection */
static void test_duplicate_key_insertion(void)
{
	JUNO_MAP_T tMap = {};
	JUNO_MAP_KEY_T tKeyTbl[5] = {};
	JUNO_MAP_VALUE_T tValTbl[5] = {};
	JUNO_STATUS_T tStatus = gptMapApi->Init(
		&tMap,
		gptHashApi,
		tKeyTbl,
		tValTbl,
		sizeof(TEST_KEY),
		5,
		IsKeyEqual,
		NULL,
		NULL
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	TEST_KEY tKey = {.key = "Duplicate"};
	TEST_VALUE tValue1 = {.value = 1};
	TEST_VALUE tValue2 = {.value = 2};

	/* First insertion should succeed */
	tStatus = gptMapApi->Set(
		&tMap,
		(JUNO_MAP_KEY_T){.ptKey = &tKey},
		(JUNO_MAP_VALUE_T){.ptValue = &tValue1}
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);

	/* Second insertion with same key expected to fail */
	tStatus = gptMapApi->Set(
		&tMap,
		(JUNO_MAP_KEY_T){.ptKey = &tKey},
		(JUNO_MAP_VALUE_T){.ptValue = &tValue2}
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	JUNO_MAP_VALUE_T tRetVal = {};
	tStatus = gptMapApi->Get(
		&tMap,
		(JUNO_MAP_KEY_T){.ptKey = &tKey},
		&tRetVal
	);
	TEST_VALUE *ptRetVal = (TEST_VALUE *)(tRetVal.ptValue);
	TEST_ASSERT_EQUAL(tValue2.value, ptRetVal->value);
}

/* Test: Overflow the map capacity by inserting more keys than allowed */
static void test_overflow_map(void)
{
	const int capacity = 3;
	JUNO_MAP_T tMap = {};
	JUNO_MAP_KEY_T tKeyTbl[3] = {};
	JUNO_MAP_VALUE_T tValTbl[3] = {};
	JUNO_STATUS_T tStatus = gptMapApi->Init(
		&tMap,
		gptHashApi,
		tKeyTbl,
		tValTbl,
		sizeof(TEST_KEY),
		capacity,
		IsKeyEqual,
		NULL,
		NULL
	);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	TEST_KEY tTestKeys[4] = {};
	TEST_VALUE tTestValues[4] = {};

	/* Insert keys equal to capacity */
	for(int i = 0; i < capacity; i++)
	{
		sprintf(tTestKeys[i].key, "Key_%i", i);
		tTestValues[i].value = i;
		tStatus = gptMapApi->Set(
			&tMap,
			(JUNO_MAP_KEY_T){.ptKey = &tTestKeys[i]},
			(JUNO_MAP_VALUE_T){.ptValue = &tTestValues[i]}
		);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	/* Attempt to insert one more key beyond capacity */
	sprintf(tTestKeys[3].key, "Key_%i", 3);
	tTestValues[3].value = 3;
	tStatus = gptMapApi->Set(
		&tMap,
		(JUNO_MAP_KEY_T){.ptKey = &tTestKeys[3]},
		(JUNO_MAP_VALUE_T){.ptValue = &tTestValues[3]}
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
