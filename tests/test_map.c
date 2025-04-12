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
	for(int i = 0; i < 10; i++)
	{
		TEST_ASSERT_EQUAL(tKeyTbl[i].ptKey, NULL);
		TEST_ASSERT_EQUAL(tValTbl[i].ptValue, NULL);
	}
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_nominal_map);
	return UNITY_END();
}
