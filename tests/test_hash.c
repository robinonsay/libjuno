#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include <stdbool.h>
#include <stdint.h>
#include "juno/hash/hash_api.h"

static const JUNO_HASH_API_T *gptHashApi = NULL;

void setUp(void)
{
	gptHashApi = Juno_HashDjB2Api();
}
void tearDown(void)
{

}

static void test_djb2_hash(void)
{
	const char pcTestString[16] = "Hello World!";
	size_t zRetHash = 0;
	JUNO_STATUS_T tStatus = gptHashApi->Hash((const uint8_t *)(pcTestString), sizeof(pcTestString), &zRetHash);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	const char pcTestString2[16] = "Test World!";
	size_t zRetHash2 = 0;
	tStatus = gptHashApi->Hash((const uint8_t *)(pcTestString2), sizeof(pcTestString2), &zRetHash2);
	TEST_ASSERT_NOT_EQUAL(zRetHash, zRetHash2);
	
	
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_djb2_hash);
	return UNITY_END();
}
