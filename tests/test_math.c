/*
    MIT License

    Copyright (c) 2025 Robin A. Onsay

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.
*/

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
	double tRes = Juno_Vec2_F64_L2Norm2(tVec1);
	double tTruth = 25;
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
	float tRes = Juno_Vec2_F32_L2Norm2(tVec1);
	float tTruth = 25;
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
	float tRes = Juno_Vec2_I32_L2Norm2(tVec1);
	float tTruth = 25;
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
	double tRes = Juno_Vec3_F64_L2Norm2(tVec1);
	double tTruth = 9;
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
	float tRes = Juno_Vec3_F32_L2Norm2(tVec1);
	float tTruth = 9;
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
	float tRes = Juno_Vec3_I32_L2Norm2(tVec1);
	float tTruth = 9;
	TEST_ASSERT_EQUAL_FLOAT(tTruth, tRes);
}

static void test_rquat_f64_add(void)
{
    JUNO_RQUAT_F64_T q0 = {{1.0,  2.0,  3.0,  4.0}};
    JUNO_RQUAT_F64_T q1 = {{4.0,  3.0,  2.0,  1.0}};
    JUNO_RQUAT_F64_T res = Juno_RQuat_F64_Add(q0, q1);
    double truth[4] = {5.0, 5.0, 5.0, 5.0};
    for(uint8_t i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_DOUBLE(truth[i], res.arr[i]);
    }
}

static void test_rquat_f64_sub(void)
{
    JUNO_RQUAT_F64_T q0 = {{5.0, 6.0, 7.0, 8.0}};
    JUNO_RQUAT_F64_T q1 = {{1.0, 2.0, 3.0, 4.0}};
    JUNO_RQUAT_F64_T res = Juno_RQuat_F64_Sub(q0, q1);
    double truth[4] = {4.0, 4.0, 4.0, 4.0};
    for(uint8_t i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_DOUBLE(truth[i], res.arr[i]);
    }
}

static void test_rquat_f64_mult_scalar(void)
{
    JUNO_RQUAT_F64_T q  = {{1.0, -2.0, 3.0, -4.0}};
    double scalar    = 2.5;
    JUNO_RQUAT_F64_T res = Juno_RQuat_F64_Mult(q, scalar);
    double truth[4] = { 2.5, -5.0, 7.5, -10.0 };
    for(uint8_t i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_DOUBLE(truth[i], res.arr[i]);
    }
}

static void test_rquat_f64_hamprod(void)
{
    // use identity quaternion so q * identity == q
    JUNO_RQUAT_F64_T qId = {{1.0, 0.0, 0.0, 0.0}};
    JUNO_RQUAT_F64_T q   = {{2.0, -1.0,  3.0, -4.0}};
    JUNO_RQUAT_F64_T res = Juno_RQuat_F64_HamProd(qId, q);
    double truth[4] = {2.0, -1.0, 3.0, -4.0};
    for(uint8_t i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_DOUBLE(truth[i], res.arr[i]);
    }
}

static void test_rquat_f64_hamprod_general(void)
{
	// General case check against canonical Hamilton product
	// q0 = (1,2,3,4), q1 = (5,6,7,8) -> (-60, 12, 30, 24)
	JUNO_RQUAT_F64_T q0 = {{1.0, 2.0, 3.0, 4.0}};
	JUNO_RQUAT_F64_T q1 = {{5.0, 6.0, 7.0, 8.0}};
	JUNO_RQUAT_F64_T res = Juno_RQuat_F64_HamProd(q0, q1);
	double truth[4] = { -60.0, 12.0, 30.0, 24.0 };
	for(uint8_t i = 0; i < 4; i++) {
		TEST_ASSERT_EQUAL_DOUBLE(truth[i], res.arr[i]);
	}
}

static void test_rquat_f64_conj(void)
{
    JUNO_RQUAT_F64_T q = {{1.0, 2.0, -3.0, 4.0}};
    JUNO_RQUAT_F64_T res = Juno_RQuat_F64_Conj(q);
    double truth[4] = {1.0, -2.0, 3.0, -4.0};
    for(uint8_t i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_DOUBLE(truth[i], res.arr[i]);
    }
}

static void test_rquat_f64_l2norm2(void)
{
    JUNO_RQUAT_F64_T q = {{1.0, 2.0, 3.0, 4.0}};
    double n2 = Juno_RQuat_F64_L2Norm2(q);
    // 1^2 + 2^2 + 3^2 + 4^2 = 30
    TEST_ASSERT_EQUAL_DOUBLE(30.0, n2);
}

static void test_rquat_f64_l2norm(void)
{
    JUNO_RQUAT_F64_T q = {{1.0, 2.0, 3.0, 4.0}};
    double n = Juno_RQuat_F64_L2Norm2(q);
    // sqrt(30) ≈ 5.477225575
    TEST_ASSERT_DOUBLE_WITHIN(1e-12, 30.0, n);
}

static void test_rquat_f64_recip(void)
{
    JUNO_RQUAT_F64_T q = {{1.0, 2.0, 3.0, 4.0}};
    JUNO_RQUAT_F64_T res = Juno_RQuat_F64_Recip(q);
    double inv30 = 1.0 / 30.0;
    double truth[4] = {  inv30, -2.0*inv30, -3.0*inv30, -4.0*inv30 };
    for(uint8_t i = 0; i < 4; i++) {
        TEST_ASSERT_DOUBLE_WITHIN(1e-12, truth[i], res.arr[i]);
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
    RUN_TEST(test_rquat_f64_add);
    RUN_TEST(test_rquat_f64_sub);
    RUN_TEST(test_rquat_f64_mult_scalar);
    RUN_TEST(test_rquat_f64_hamprod);
	RUN_TEST(test_rquat_f64_hamprod_general);
    RUN_TEST(test_rquat_f64_conj);
    RUN_TEST(test_rquat_f64_l2norm2);
    RUN_TEST(test_rquat_f64_l2norm);
    RUN_TEST(test_rquat_f64_recip);
	return UNITY_END();
}
