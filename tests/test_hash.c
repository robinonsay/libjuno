#include "juno/hash/hash_api.h"
#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include <stdbool.h>
#include <stdint.h>
#define JUNO_HASH_DEFAULT
#include "juno/hash/hash_djb2.h"

void setUp(void)
{
}
void tearDown(void)
{

}

static void test_djb2_hash(void)
{
	JUNO_HASH_T tHash = {};
	JUNO_STATUS_T tStatus = JunoHash_Djb2Api(&tHash, NULL, NULL);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	const JUNO_HASH_API_T *ptHashApi = tHash.ptApi;
	const char pcTestString[16] = "Hello World!";
	size_t zRetHash = 0;
	tStatus = ptHashApi->Hash(&tHash, (const uint8_t *)(pcTestString), sizeof(pcTestString), &zRetHash);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	const char pcTestString2[16] = "Test World!";
	size_t zRetHash2 = 0;
	tStatus = ptHashApi->Hash(&tHash, (const uint8_t *)(pcTestString2), sizeof(pcTestString2), &zRetHash2);
	TEST_ASSERT_NOT_EQUAL(zRetHash, zRetHash2);
	
	
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_djb2_hash);
	return UNITY_END();
}
