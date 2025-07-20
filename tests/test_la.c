#include "juno/math/juno_math_types.h"
#include "unity.h"
#include "unity_internals.h"
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include "juno/math/juno_math.h"

void setUp(void)
{

}
void tearDown(void)
{

}

// static void test_mat_index(void)
// {
//  	MAT_3x3 tMat3x3 = {.tRoot = {.iNDim = 3, .iMDim = 3}, .tMat = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}}};
// 	float tTruth[3][3] ={{0, 1, 2}, {3, 4, 5}, {6, 7, 8}};
// 	for(size_t i = 0; i < 3; i++)
// 	{
// 		for(size_t j = 0; j < 3; j++)
// 		{
// 			TEST_ASSERT_EQUAL_FLOAT(tTruth[i][j], tMat3x3.tMat[i][j]);
// 			JUNO_MATH_MAT_NxM_FLOAT_T *ptMatNxM = (JUNO_MATH_MAT_NxM_FLOAT_T*) &tMat3x3;
// 			TEST_ASSERT_EQUAL_FLOAT(tTruth[i][j], ptMatNxM->tMat[i*ptMatNxM->tRoot.iMDim + j]);
// 		}
// 	}
// }

static void test_vec_index(void)
{
	JUNO_MATH_VEC3F_T tMyVec = {1, 2, 3};
	float tTruth[3] = {1, 2,3};
	for(size_t j = 0; j < 3; j++)
	{
		TEST_ASSERT_EQUAL_FLOAT(tTruth[j], tMyVec.v[j]);
	}
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_vec_index);
	return UNITY_END();
}
