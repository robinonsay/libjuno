#ifndef JUNO_H
#define JUNO_H

#include "juno_math_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
    Add two vecf's together
*/
static inline JUNO_VEC2_F64_T Juno_Vec2_f64_Add(JUNO_VEC2_F64_T tVec0, JUNO_VEC2_F64_T tVec1)
{
    tVec0.arr[0] += tVec1.arr[0];
    tVec0.arr[1] += tVec1.arr[1];
    return tVec0;
}


/**
    Subtract to vec2f's
    @param tVec0 The vec to subtract from
    @param tVec1 The vec to subtract
*/
static inline JUNO_VEC2_F64_T Juno_Vec2_f64_Sub(JUNO_VEC2_F64_T tVec0, JUNO_VEC2_F64_T tVec1)
{
    tVec0.arr[0] -= tVec1.arr[0];
    tVec0.arr[1] -= tVec1.arr[1];
    return tVec0;
}

/**
    Multiply scalar with vec2f's
*/
static inline JUNO_VEC2_F64_T Juno_Vec2_f64_Mult(JUNO_VEC2_F64_T tVec0, double dScalar)
{
    tVec0.arr[0] *= dScalar;
    tVec0.arr[1] *= dScalar;
    return tVec0;
}

/**
    Dot product of two vec2f's
*/
static inline double Juno_Vec2_f64_Dot(JUNO_VEC2_F64_T tVec0, JUNO_VEC2_F64_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[0] + tVec0.arr[1] * tVec1.arr[1];
}

/**
    The cross product of two vec2f's.
    The result is a psedoscalar
*/
static inline double Juno_Vec2_f64_Cross(JUNO_VEC2_F64_T tVec0, JUNO_VEC2_F64_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[1] - tVec0.arr[1] * tVec1.arr[0];
}

/// Add two vec2i's
static inline JUNO_VEC2_I32_T Juno_Vec2_i32_Add(JUNO_VEC2_I32_T tVec0, JUNO_VEC2_I32_T tVec1)
{
    tVec0.arr[0] += tVec1.arr[0];
    tVec0.arr[1] += tVec1.arr[1];
    return tVec0;
}


/**
    Subtract to vec2i's
    @param tVec0 The vec to subtract from
    @param tVec1 The vec to subtract
*/
static inline JUNO_VEC2_I32_T Juno_Vec2_i32_Sub(JUNO_VEC2_I32_T tVec0, JUNO_VEC2_I32_T tVec1)
{
    tVec0.arr[0] -= tVec1.arr[0];
    tVec0.arr[1] -= tVec1.arr[1];
    return tVec0;
}

/**
    Multiply scalar with vec2f's
*/
static inline JUNO_VEC2_I32_T Juno_Vec2_i32_Mult(JUNO_VEC2_I32_T tVec0, int32_t dScalar)
{
    tVec0.arr[0] *= dScalar;
    tVec0.arr[1] *= dScalar;
    return tVec0;
}

/**
    Dot product of two vec2i's
*/
static inline int32_t Juno_Vec2_i32_Dot(JUNO_VEC2_I32_T tVec0, JUNO_VEC2_I32_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[0] + tVec0.arr[1] * tVec1.arr[1];
}

/**
    The cross product of two vec2i's.
    The result is a psedoscalar
*/
static inline int32_t Juno_Vec2_i32_Cross(JUNO_VEC2_I32_T tVec0, JUNO_VEC2_I32_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[1] - tVec0.arr[1] * tVec1.arr[0];
}

/**
    Add two vecf's together
*/
static inline JUNO_VEC3_F64_T Juno_Vec3_f64_Add(JUNO_VEC3_F64_T tVec0, JUNO_VEC3_F64_T tVec1)
{
    tVec0.arr[0] += tVec1.arr[0];
    tVec0.arr[1] += tVec1.arr[1];
    tVec0.arr[2] += tVec1.arr[2];
    return tVec0;
}


/**
    Subtract to vec2f's
    @param tVec0 The vec to subtract from
    @param tVec1 The vec to subtract
*/
static inline JUNO_VEC3_F64_T Juno_Vec3_f64_Sub(JUNO_VEC3_F64_T tVec0, JUNO_VEC3_F64_T tVec1)
{
    tVec0.arr[0] -= tVec1.arr[0];
    tVec0.arr[1] -= tVec1.arr[1];
    tVec0.arr[2] -= tVec1.arr[2];
    return tVec0;
}

/**
    Multiply scalar with vec2f's
*/
static inline JUNO_VEC3_F64_T Juno_Vec3_f64_Mult(JUNO_VEC3_F64_T tVec0, double dScalar)
{
    tVec0.arr[0] *= dScalar;
    tVec0.arr[1] *= dScalar;
    tVec0.arr[2] *= dScalar;
    return tVec0;
}

/**
    Dot product of two vec2f's
*/
static inline double Juno_Vec3_f64_Dot(JUNO_VEC3_F64_T tVec0, JUNO_VEC3_F64_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[0] + tVec0.arr[1] * tVec1.arr[1] + tVec0.arr[2] * tVec1.arr[2];
}

/**
    The cross product of two vec2f's.
    The result is a psedoscalar
*/
static inline JUNO_VEC3_F64_T Juno_Vec3_f64_Cross(JUNO_VEC3_F64_T tVec0, JUNO_VEC3_F64_T tVec1)
{
    JUNO_VEC3_F64_T tRes = {
        .arr[0] = tVec0.arr[1] * tVec1.arr[2] - tVec0.arr[2] * tVec1.arr[1],
        .arr[1] = tVec0.arr[2] * tVec1.arr[0] - tVec0.arr[0] * tVec1.arr[2],
        .arr[2] = tVec0.arr[0] * tVec1.arr[1] - tVec0.arr[1] * tVec1.arr[0]
    };
    return tRes;
}

/// Add two vec2i's
static inline JUNO_VEC3_I32_T Juno_Vec3_i32_Add(JUNO_VEC3_I32_T tVec0, JUNO_VEC3_I32_T tVec1)
{
    tVec0.arr[0] += tVec1.arr[0];
    tVec0.arr[1] += tVec1.arr[1];
    tVec0.arr[2] += tVec1.arr[2];
    return tVec0;
}


/**
    Subtract to vec2i's
    @param tVec0 The vec to subtract from
    @param tVec1 The vec to subtract
*/
static inline JUNO_VEC3_I32_T Juno_Vec3_i32_Sub(JUNO_VEC3_I32_T tVec0, JUNO_VEC3_I32_T tVec1)
{
    tVec0.arr[0] -= tVec1.arr[0];
    tVec0.arr[1] -= tVec1.arr[1];
    tVec0.arr[2] -= tVec1.arr[2];
    return tVec0;
}

/**
    Multiply scalar with vec2f's
*/
static inline JUNO_VEC3_I32_T Juno_Vec3_i32_Mult(JUNO_VEC3_I32_T tVec0, int32_t dScalar)
{
    tVec0.arr[0] *= dScalar;
    tVec0.arr[1] *= dScalar;
    tVec0.arr[2] *= dScalar;
    return tVec0;
}

/**
    Dot product of two vec2i's
*/
static inline int32_t Juno_Vec3_i32_Dot(JUNO_VEC3_I32_T tVec0, JUNO_VEC3_I32_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[0] + tVec0.arr[1] * tVec1.arr[1] + tVec0.arr[2] * tVec1.arr[2];
}

/**
    The cross product of two vec2i's.
    The result is a psedoscalar
*/
static inline JUNO_VEC3_I32_T Juno_Vec3_i32_Cross(JUNO_VEC3_I32_T tVec0, JUNO_VEC3_I32_T tVec1)
{
    JUNO_VEC3_I32_T tRes = {
        .arr[0] = tVec0.arr[1] * tVec1.arr[2] - tVec0.arr[2] * tVec1.arr[1],
        .arr[1] = tVec0.arr[2] * tVec1.arr[0] - tVec0.arr[0] * tVec1.arr[2],
        .arr[2] = tVec0.arr[0] * tVec1.arr[1] - tVec0.arr[1] * tVec1.arr[0]
    };
    return tRes;
}

#ifdef __cplusplus
}
#endif
#endif