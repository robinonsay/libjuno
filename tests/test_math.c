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
	JUNO_VEC2_F64_T tVec1 = {{1, 2}};
	JUNO_VEC2_F64_T tVec2 = {{4, 5}};
	JUNO_VEC2_F64_T tRes = Juno_Vec2_F64_Add(tVec1, tVec2);
	JUNO_VEC2_F64_T tTruth = {{5, 7}};
	for(uint8_t i = 0; i < 2; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec2_f64_sub(void)
{
	JUNO_VEC2_F64_T tVec1 = {{1, 2}};
	JUNO_VEC2_F64_T tVec2 = {{4, 5}};
	JUNO_VEC2_F64_T tRes = Juno_Vec2_F64_Sub(tVec1, tVec2);
	JUNO_VEC2_F64_T tTruth = {{-3, -3}};
	for(uint8_t i = 0; i < 2; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec2_f64_mult(void)
{
	JUNO_VEC2_F64_T tVec1 = {{1, 2}};
	JUNO_VEC2_F64_T tRes = Juno_Vec2_F64_Mult(tVec1, 2);
	JUNO_VEC2_F64_T tTruth = {{2, 4}};
	for(uint8_t i = 0; i < 2; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec2_f64_dot(void)
{
	JUNO_VEC2_F64_T tVec1 = {{1, 2}};
	JUNO_VEC2_F64_T tVec2 = {{4, 5}};
	double tRes = Juno_Vec2_F64_Dot(tVec1, tVec2);
	double tTruth = 14;
	TEST_ASSERT_EQUAL_DOUBLE(tTruth, tRes);
}

static void test_vec2_f64_cross(void)
{
	JUNO_VEC2_F64_T tVec1 = {{1, 2}};
	JUNO_VEC2_F64_T tVec2 = {{4, 5}};
	double tRes = Juno_Vec2_F64_Cross(tVec1, tVec2);
	double tTruth = -3;
	TEST_ASSERT_EQUAL_DOUBLE(tTruth, tRes);
}

static void test_vec2_f64_L2(void)
{
	JUNO_VEC2_F64_T tVec1 = {{3, 4}};
	double tRes = Juno_Vec2_F64_L2Norm(tVec1);
	double tTruth = 5;
	TEST_ASSERT_EQUAL_DOUBLE(tTruth, tRes);
}

static void test_vec2_f32_add(void)
{
	JUNO_VEC2_F32_T tVec1 = {{1, 2}};
	JUNO_VEC2_F32_T tVec2 = {{4, 5}};
	JUNO_VEC2_F32_T tRes = Juno_Vec2_F32_Add(tVec1, tVec2);
	JUNO_VEC2_F32_T tTruth = {{5, 7}};
	for(uint8_t i = 0; i < 2; i++)
	{
		TEST_ASSERT_EQUAL_FLOAT(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec2_f32_sub(void)
{
	JUNO_VEC2_F32_T tVec1 = {{1, 2}};
	JUNO_VEC2_F32_T tVec2 = {{4, 5}};
	JUNO_VEC2_F32_T tRes = Juno_Vec2_F32_Sub(tVec1, tVec2);
	JUNO_VEC2_F32_T tTruth = {{-3, -3}};
	for(uint8_t i = 0; i < 2; i++)
	{
		TEST_ASSERT_EQUAL_FLOAT(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec2_f32_mult(void)
{
	JUNO_VEC2_F32_T tVec1 = {{1, 2}};
	JUNO_VEC2_F32_T tRes = Juno_Vec2_F32_Mult(tVec1, 2);
	JUNO_VEC2_F32_T tTruth = {{2, 4}};
	for(uint8_t i = 0; i < 2; i++)
	{
		TEST_ASSERT_EQUAL_FLOAT(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec2_f32_dot(void)
{
	JUNO_VEC2_F32_T tVec1 = {{1, 2}};
	JUNO_VEC2_F32_T tVec2 = {{4, 5}};
	double tRes = Juno_Vec2_F32_Dot(tVec1, tVec2);
	double tTruth = 14;
	TEST_ASSERT_EQUAL_FLOAT(tTruth, tRes);
}

static void test_vec2_f32_cross(void)
{
	JUNO_VEC2_F32_T tVec1 = {{1, 2}};
	JUNO_VEC2_F32_T tVec2 = {{4, 5}};
	double tRes = Juno_Vec2_F32_Cross(tVec1, tVec2);
	double tTruth = -3;
	TEST_ASSERT_EQUAL_FLOAT(tTruth, tRes);
}

static void test_vec2_f32_L2(void)
{
	JUNO_VEC2_F32_T tVec1 = {{3, 4}};
	float tRes = Juno_Vec2_F32_L2Norm(tVec1);
	float tTruth = 5;
	TEST_ASSERT_EQUAL_FLOAT(tTruth, tRes);
}

static void test_vec2_i32_add(void)
{
	JUNO_VEC2_I32_T tVec1 = {{1, 2}};
	JUNO_VEC2_I32_T tVec2 = {{4, 5}};
	JUNO_VEC2_I32_T tRes = Juno_Vec2_I32_Add(tVec1, tVec2);
	JUNO_VEC2_I32_T tTruth = {{5, 7}};
	for(uint8_t i = 0; i < 2; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec2_i32_sub(void)
{
	JUNO_VEC2_I32_T tVec1 = {{1, 2}};
	JUNO_VEC2_I32_T tVec2 = {{4, 5}};
	JUNO_VEC2_I32_T tRes = Juno_Vec2_I32_Sub(tVec1, tVec2);
	JUNO_VEC2_I32_T tTruth = {{-3, -3}};
	for(uint8_t i = 0; i < 2; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec2_i32_mult(void)
{
	JUNO_VEC2_I32_T tVec1 = {{1, 2}};
	JUNO_VEC2_I32_T tRes = Juno_Vec2_I32_Mult(tVec1, 2);
	JUNO_VEC2_I32_T tTruth = {{2, 4}};
	for(uint8_t i = 0; i < 2; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec2_i32_dot(void)
{
	JUNO_VEC2_I32_T tVec1 = {{1, 2}};
	JUNO_VEC2_I32_T tVec2 = {{4, 5}};
	int32_t tRes = Juno_Vec2_I32_Dot(tVec1, tVec2);
	int32_t tTruth = 14;
	TEST_ASSERT_EQUAL_DOUBLE(tTruth, tRes);
}

static void test_vec2_i32_cross(void)
{
	JUNO_VEC2_I32_T tVec1 = {{1, 2}};
	JUNO_VEC2_I32_T tVec2 = {{4, 5}};
	int32_t tRes = Juno_Vec2_I32_Cross(tVec1, tVec2);
	int32_t tTruth = -3;
	TEST_ASSERT_EQUAL_DOUBLE(tTruth, tRes);
}

static void test_vec2_i32_L2(void)
{
	JUNO_VEC2_I32_T tVec1 = {{3, 4}};
	float tRes = Juno_Vec2_I32_L2Norm(tVec1);
	float tTruth = 5;
	TEST_ASSERT_EQUAL_FLOAT(tTruth, tRes);
}

static void test_vec3_f64_add(void)
{
	JUNO_VEC3_F64_T tVec1 = {{1, 2, 3}};
	JUNO_VEC3_F64_T tVec2 = {{4, 5, 6}};
	JUNO_VEC3_F64_T tRes = Juno_Vec3_F64_Add(tVec1, tVec2);
	JUNO_VEC3_F64_T tTruth = {{5, 7, 9}};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec3_f64_sub(void)
{
	JUNO_VEC3_F64_T tVec1 = {{1, 2, 3}};
	JUNO_VEC3_F64_T tVec2 = {{4, 5, 6}};
	JUNO_VEC3_F64_T tRes = Juno_Vec3_F64_Sub(tVec1, tVec2);
	JUNO_VEC3_F64_T tTruth = {{-3, -3, -3}};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec3_f64_mult(void)
{
	JUNO_VEC3_F64_T tVec1 = {{1, 2, 3}};
	JUNO_VEC3_F64_T tRes = Juno_Vec3_F64_Mult(tVec1, 2);
	JUNO_VEC3_F64_T tTruth = {{2, 4, 6}};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec3_f64_dot(void)
{
	JUNO_VEC3_F64_T tVec1 = {{1, 2, 3}};
	JUNO_VEC3_F64_T tVec2 = {{4, 5, 6}};
	double tRes = Juno_Vec3_F64_Dot(tVec1, tVec2);
	double tTruth = 32;
	TEST_ASSERT_EQUAL_DOUBLE(tTruth, tRes);
}

static void test_vec3_f64_cross(void)
{
	JUNO_VEC3_F64_T tVec1 = {{1, 2, 3}};
	JUNO_VEC3_F64_T tVec2 = {{4, 5, 6}};
	JUNO_VEC3_F64_T tRes = Juno_Vec3_F64_Cross(tVec1, tVec2);
	JUNO_VEC3_F64_T tTruth = {{-3, 6, -3}};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec3_f64_L2(void)
{
	JUNO_VEC3_F64_T tVec1 = {{1, 2, 2}};
	double tRes = Juno_Vec3_F64_L2Norm(tVec1);
	double tTruth = 3;
	TEST_ASSERT_EQUAL_DOUBLE(tTruth, tRes);
}

static void test_vec3_f32_add(void)
{
	JUNO_VEC3_F32_T tVec1 = {{1, 2, 3}};
	JUNO_VEC3_F32_T tVec2 = {{4, 5, 6}};
	JUNO_VEC3_F32_T tRes = Juno_Vec3_F32_Add(tVec1, tVec2);
	JUNO_VEC3_F32_T tTruth = {{5, 7, 9}};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec3_f32_sub(void)
{
	JUNO_VEC3_F32_T tVec1 = {{1, 2, 3}};
	JUNO_VEC3_F32_T tVec2 = {{4, 5, 6}};
	JUNO_VEC3_F32_T tRes = Juno_Vec3_F32_Sub(tVec1, tVec2);
	JUNO_VEC3_F32_T tTruth = {{-3, -3, -3}};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec3_f32_mult(void)
{
	JUNO_VEC3_F32_T tVec1 = {{1, 2, 3}};
	JUNO_VEC3_F32_T tRes = Juno_Vec3_F32_Mult(tVec1, 2);
	JUNO_VEC3_F32_T tTruth = {{2, 4, 6}};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec3_f32_dot(void)
{
	JUNO_VEC3_F32_T tVec1 = {{1, 2, 3}};
	JUNO_VEC3_F32_T tVec2 = {{4, 5, 6}};
	double tRes = Juno_Vec3_F32_Dot(tVec1, tVec2);
	double tTruth = 32;
	TEST_ASSERT_EQUAL_DOUBLE(tTruth, tRes);
}

static void test_vec3_f32_cross(void)
{
	JUNO_VEC3_F32_T tVec1 = {{1, 2, 3}};
	JUNO_VEC3_F32_T tVec2 = {{4, 5, 6}};
	JUNO_VEC3_F32_T tRes = Juno_Vec3_F32_Cross(tVec1, tVec2);
	JUNO_VEC3_F32_T tTruth = {{-3, 6, -3}};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec3_f32_L2(void)
{
	JUNO_VEC3_F32_T tVec1 = {{1, 2, 2}};
	float tRes = Juno_Vec3_F32_L2Norm(tVec1);
	float tTruth = 3;
	TEST_ASSERT_EQUAL_FLOAT(tTruth, tRes);
}

static void test_vec3_i32_add(void)
{
	JUNO_VEC3_I32_T tVec1 = {{1, 2, 3}};
	JUNO_VEC3_I32_T tVec2 = {{4, 5, 6}};
	JUNO_VEC3_I32_T tRes = Juno_Vec3_I32_Add(tVec1, tVec2);
	JUNO_VEC3_I32_T tTruth = {{5, 7, 9}};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec3_i32_sub(void)
{
	JUNO_VEC3_I32_T tVec1 = {{1, 2, 3}};
	JUNO_VEC3_I32_T tVec2 = {{4, 5, 6}};
	JUNO_VEC3_I32_T tRes = Juno_Vec3_I32_Sub(tVec1, tVec2);
	JUNO_VEC3_I32_T tTruth = {{-3, -3, -3}};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec3_i32_mult(void)
{
	JUNO_VEC3_I32_T tVec1 = {{1, 2, 3}};
	JUNO_VEC3_I32_T tRes = Juno_Vec3_I32_Mult(tVec1, 2);
	JUNO_VEC3_I32_T tTruth = {{2, 4, 6}};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec3_i32_dot(void)
{
	JUNO_VEC3_I32_T tVec1 = {{1, 2, 3}};
	JUNO_VEC3_I32_T tVec2 = {{4, 5, 6}};
	double tRes = Juno_Vec3_I32_Dot(tVec1, tVec2);
	double tTruth = 32;
	TEST_ASSERT_EQUAL_DOUBLE(tTruth, tRes);
}

static void test_vec3_i32_cross(void)
{
	JUNO_VEC3_I32_T tVec1 = {{1, 2, 3}};
	JUNO_VEC3_I32_T tVec2 = {{4, 5, 6}};
	JUNO_VEC3_I32_T tRes = Juno_Vec3_I32_Cross(tVec1, tVec2);
	JUNO_VEC3_I32_T tTruth = {{-3, 6, -3}};
	for(uint8_t i = 0; i < 3; i++)
	{
		TEST_ASSERT_EQUAL_DOUBLE(tTruth.arr[i], tRes.arr[i]);
	}
}

static void test_vec3_i32_L2(void)
{
	JUNO_VEC3_I32_T tVec1 = {{1, 2, 2}};
	float tRes = Juno_Vec3_I32_L2Norm(tVec1);
	float tTruth = 3;
	TEST_ASSERT_EQUAL_FLOAT(tTruth, tRes);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_vec2_f64_add);
	RUN_TEST(test_vec2_f64_sub);
	RUN_TEST(test_vec2_f64_mult);
	RUN_TEST(test_vec2_f64_dot);
	RUN_TEST(test_vec2_f64_cross);
	RUN_TEST(test_vec2_f64_L2);
	RUN_TEST(test_vec2_f32_add);
	RUN_TEST(test_vec2_f32_sub);
	RUN_TEST(test_vec2_f32_mult);
	RUN_TEST(test_vec2_f32_dot);
	RUN_TEST(test_vec2_f32_cross);
	RUN_TEST(test_vec2_f32_L2);
	RUN_TEST(test_vec2_i32_add);
	RUN_TEST(test_vec2_i32_sub);
	RUN_TEST(test_vec2_i32_mult);
	RUN_TEST(test_vec2_i32_dot);
	RUN_TEST(test_vec2_i32_cross);
	RUN_TEST(test_vec2_i32_L2);
	RUN_TEST(test_vec3_f64_add);
	RUN_TEST(test_vec3_f64_sub);
	RUN_TEST(test_vec3_f64_mult);
	RUN_TEST(test_vec3_f64_dot);
	RUN_TEST(test_vec3_f64_cross);
	RUN_TEST(test_vec3_f64_L2);
	RUN_TEST(test_vec3_f32_add);
	RUN_TEST(test_vec3_f32_sub);
	RUN_TEST(test_vec3_f32_mult);
	RUN_TEST(test_vec3_f32_dot);
	RUN_TEST(test_vec3_f32_cross);
	RUN_TEST(test_vec3_f32_L2);
	RUN_TEST(test_vec3_i32_add);
	RUN_TEST(test_vec3_i32_sub);
	RUN_TEST(test_vec3_i32_mult);
	RUN_TEST(test_vec3_i32_dot);
	RUN_TEST(test_vec3_i32_cross);
	RUN_TEST(test_vec3_i32_L2);
	return UNITY_END();
}
