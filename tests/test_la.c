#include "juno/math/juno_math_types.h"
#include "unity.h"
#include "unity_internals.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
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

static void test_vec3_f64_add(void)
{
	JUNO_MATH_VEC3_F64_T tVec1 = {1, 2, 3};
	JUNO_MATH_VEC3_F64_T tVec2 = {4, 5, 6};
	JUNO_MATH_VEC3_F64_T tRes = Juno_MathVec3f_Add(tVec1, tVec2);
	JUNO_MATH_VEC3_F64_T tTruth = {5, 7, 9};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec3_f64_sub(void)
{
	JUNO_MATH_VEC3_F64_T tVec1 = {1, 2, 3};
	JUNO_MATH_VEC3_F64_T tVec2 = {4, 5, 6};
	JUNO_MATH_VEC3_F64_T tRes = Juno_MathVec3f_Add(tVec1, tVec2);
	JUNO_MATH_VEC3_F64_T tTruth = {5, 7, 9};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_vec3_f64_add);
	RUN_TEST(test_vec3_f64_sub);
	return UNITY_END();
}
