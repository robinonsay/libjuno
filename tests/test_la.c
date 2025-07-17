#include "unity.h"
#include "unity_internals.h"
#include <stdbool.h>
#include "juno/math/juno_la.hpp"

void setUp(void)
{

}
void tearDown(void)
{

}

static void test_template(void)
{
    TEST_ASSERT_TRUE(true);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_template);
	return UNITY_END();
}
