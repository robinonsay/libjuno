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

static void test_vec2_f64_add(void)
{
	JUNO_MATH_VEC2_F64_T tVec1 = {1, 2};
	JUNO_MATH_VEC2_F64_T tVec2 = {4, 5};
	JUNO_MATH_VEC2_F64_T tRes = Juno_MathVec2_f64_Add(tVec1, tVec2);
	JUNO_MATH_VEC2_F64_T tTruth = {5, 7};
	for(uint8_t i = 0; i < 2; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec2_f64_sub(void)
{
	JUNO_MATH_VEC2_F64_T tVec1 = {1, 2};
	JUNO_MATH_VEC2_F64_T tVec2 = {4, 5};
	JUNO_MATH_VEC2_F64_T tRes = Juno_MathVec2_f64_Sub(tVec1, tVec2);
	JUNO_MATH_VEC2_F64_T tTruth = {-3, -3};
	for(uint8_t i = 0; i < 2; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec2_f64_mult(void)
{
	JUNO_MATH_VEC2_F64_T tVec1 = {1, 2};
	JUNO_MATH_VEC2_F64_T tRes = Juno_MathVec2_f64_Mult(tVec1, 2);
	JUNO_MATH_VEC2_F64_T tTruth = {2, 4};
	for(uint8_t i = 0; i < 2; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec2_f64_dot(void)
{
	JUNO_MATH_VEC2_F64_T tVec1 = {1, 2};
	JUNO_MATH_VEC2_F64_T tVec2 = {4, 5};
	double tRes = Juno_MathVec2_f64_Dot(tVec1, tVec2);
	double tTruth = 14;
	TEST_ASSERT_EQUAL_DOUBLE(tTruth, tRes);
}

static void test_vec2_f64_cross(void)
{
	JUNO_MATH_VEC2_F64_T tVec1 = {1, 2};
	JUNO_MATH_VEC2_F64_T tVec2 = {4, 5};
	double tRes = Juno_MathVec2_f64_Cross(tVec1, tVec2);
	double tTruth = -3;
	TEST_ASSERT_EQUAL_DOUBLE(tTruth, tRes);
}

static void test_vec2_i32_add(void)
{
	JUNO_MATH_VEC2_I32_T tVec1 = {1, 2};
	JUNO_MATH_VEC2_I32_T tVec2 = {4, 5};
	JUNO_MATH_VEC2_I32_T tRes = Juno_MathVec2_i32_Add(tVec1, tVec2);
	JUNO_MATH_VEC2_I32_T tTruth = {5, 7};
	for(uint8_t i = 0; i < 2; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec2_i32_sub(void)
{
	JUNO_MATH_VEC2_I32_T tVec1 = {1, 2};
	JUNO_MATH_VEC2_I32_T tVec2 = {4, 5};
	JUNO_MATH_VEC2_I32_T tRes = Juno_MathVec2_i32_Sub(tVec1, tVec2);
	JUNO_MATH_VEC2_I32_T tTruth = {-3, -3};
	for(uint8_t i = 0; i < 2; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec2_i32_mult(void)
{
	JUNO_MATH_VEC2_I32_T tVec1 = {1, 2};
	JUNO_MATH_VEC2_I32_T tRes = Juno_MathVec2_i32_Mult(tVec1, 2);
	JUNO_MATH_VEC2_I32_T tTruth = {2, 4};
	for(uint8_t i = 0; i < 2; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec2_i32_dot(void)
{
	JUNO_MATH_VEC2_I32_T tVec1 = {1, 2};
	JUNO_MATH_VEC2_I32_T tVec2 = {4, 5};
	int32_t tRes = Juno_MathVec2_i32_Dot(tVec1, tVec2);
	int32_t tTruth = 14;
	TEST_ASSERT_EQUAL_DOUBLE(tTruth, tRes);
}

static void test_vec2_i32_cross(void)
{
	JUNO_MATH_VEC2_I32_T tVec1 = {1, 2};
	JUNO_MATH_VEC2_I32_T tVec2 = {4, 5};
	int32_t tRes = Juno_MathVec2_i32_Cross(tVec1, tVec2);
	int32_t tTruth = -3;
	TEST_ASSERT_EQUAL_DOUBLE(tTruth, tRes);
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

static void test_vec3_f64_mult(void)
{
	JUNO_MATH_VEC3_F64_T tVec1 = {1, 2, 3};
	JUNO_MATH_VEC3_F64_T tRes = Juno_MathVec3_f64_Mult(tVec1, 2);
	JUNO_MATH_VEC3_F64_T tTruth = {2, 4, 6};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec3_f64_dot(void)
{
	JUNO_MATH_VEC3_F64_T tVec1 = {1, 2, 3};
	JUNO_MATH_VEC3_F64_T tVec2 = {4, 5, 6};
	double tRes = Juno_MathVec3_f64_Dot(tVec1, tVec2);
	double tTruth = 32;
	TEST_ASSERT_EQUAL_DOUBLE(tTruth, tRes);
}

static void test_vec3_f64_cross(void)
{
	JUNO_MATH_VEC3_F64_T tVec1 = {1, 2, 3};
	JUNO_MATH_VEC3_F64_T tVec2 = {4, 5, 6};
	JUNO_MATH_VEC3_F64_T tRes = Juno_MathVec3_f64_Cross(tVec1, tVec2);
	JUNO_MATH_VEC3_F64_T tTruth = {-3, 6, -3};
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

static void test_vec3_i32_mult(void)
{
	JUNO_MATH_VEC3_I32_T tVec1 = {1, 2, 3};
	JUNO_MATH_VEC3_I32_T tRes = Juno_MathVec3_i32_Mult(tVec1, 2);
	JUNO_MATH_VEC3_I32_T tTruth = {2, 4, 6};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec3_i32_dot(void)
{
	JUNO_MATH_VEC3_I32_T tVec1 = {1, 2, 3};
	JUNO_MATH_VEC3_I32_T tVec2 = {4, 5, 6};
	double tRes = Juno_MathVec3_i32_Dot(tVec1, tVec2);
	double tTruth = 32;
	TEST_ASSERT_EQUAL_DOUBLE(tTruth, tRes);
}

static void test_vec3_i32_cross(void)
{
	JUNO_MATH_VEC3_I32_T tVec1 = {1, 2, 3};
	JUNO_MATH_VEC3_I32_T tVec2 = {4, 5, 6};
	JUNO_MATH_VEC3_I32_T tRes = Juno_MathVec3_i32_Cross(tVec1, tVec2);
	JUNO_MATH_VEC3_I32_T tTruth = {-3, 6, -3};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_vec2_f64_add);
	RUN_TEST(test_vec2_f64_sub);
	RUN_TEST(test_vec2_f64_mult);
	RUN_TEST(test_vec2_f64_dot);
	RUN_TEST(test_vec2_f64_cross);
	RUN_TEST(test_vec2_i32_add);
	RUN_TEST(test_vec2_i32_sub);
	RUN_TEST(test_vec2_i32_mult);
	RUN_TEST(test_vec2_i32_dot);
	RUN_TEST(test_vec2_i32_cross);
	RUN_TEST(test_vec3_f64_add);
	RUN_TEST(test_vec3_f64_sub);
	RUN_TEST(test_vec3_f64_mult);
	RUN_TEST(test_vec3_f64_dot);
	RUN_TEST(test_vec3_f64_cross);
	RUN_TEST(test_vec3_i32_add);
	RUN_TEST(test_vec3_i32_sub);
	RUN_TEST(test_vec3_i32_mult);
	RUN_TEST(test_vec3_i32_dot);
	RUN_TEST(test_vec3_i32_cross);
	return UNITY_END();
}
