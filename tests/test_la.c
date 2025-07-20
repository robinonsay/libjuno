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

static void test_vec3_f64_add(void)
{
	JUNO_MATH_VEC3_F64_T tVec1 = {1, 2, 3};
	JUNO_MATH_VEC3_F64_T tVec2 = {4, 5, 6};
	JUNO_MATH_VEC3_F64_T tRes = Juno_MathVec3_f64_Add(tVec1, tVec2);
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
	JUNO_MATH_VEC3_F64_T tRes = Juno_MathVec3_f64_Sub(tVec1, tVec2);
	JUNO_MATH_VEC3_F64_T tTruth = {-3, -3, -3};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec3_i32_add(void)
{
	JUNO_MATH_VEC3_I32_T tVec1 = {1, 2, 3};
	JUNO_MATH_VEC3_I32_T tVec2 = {4, 5, 6};
	JUNO_MATH_VEC3_I32_T tRes = Juno_MathVec3_i32_Add(tVec1, tVec2);
	JUNO_MATH_VEC3_I32_T tTruth = {5, 7, 9};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec3_i32_sub(void)
{
	JUNO_MATH_VEC3_I32_T tVec1 = {1, 2, 3};
	JUNO_MATH_VEC3_I32_T tVec2 = {4, 5, 6};
	JUNO_MATH_VEC3_I32_T tRes = Juno_MathVec3_i32_Sub(tVec1, tVec2);
	JUNO_MATH_VEC3_I32_T tTruth = {-3, -3, -3};
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
	RUN_TEST(test_vec3_i32_add);
	RUN_TEST(test_vec3_i32_sub);
	return UNITY_END();
}
