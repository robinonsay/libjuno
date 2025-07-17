#include "juno/memory/memory_api.h"
#include "unity.h"
#include "unity_internals.h"
#include <stdbool.h>
#include <stddef.h>
#include "juno/math/juno_la.h"

void setUp(void)
{

}
void tearDown(void)
{

}
JUNO_MATH_MAT_NxM(MAT_3x3, 3, 3);

static void test_mat_index(void)
{
 	MAT_3x3 tMat3x3 = {.tMat = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}}};
	float tTruth[3][3] ={{0, 1, 2}, {3, 4, 5}, {6, 7, 8}};
	for(size_t i = 0; i < 3; i++)
	{
		for(size_t j = 0; j < 3; j++)
		{
			TEST_ASSERT_EQUAL_FLOAT(tTruth[i][j], tMat3x3.tMat[i][j]);
		}
	}
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_mat_index);
	return UNITY_END();
}
