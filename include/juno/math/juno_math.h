#ifndef JUNO_H
#define JUNO_H

#include "juno_math_types.h"
#include <stdint.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#define Juno_Pow2(d)    (d*d)
#define Juno_Pow3(d)    (d*d*d)
#define Juno_Pow4(d)    (d*d*d*d)

/**
    Add two vecf's together
*/
static inline JUNO_VEC2_F64_T Juno_Vec2_F64_Add(JUNO_VEC2_F64_T tVec0, JUNO_VEC2_F64_T tVec1)
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
static inline JUNO_VEC2_F64_T Juno_Vec2_F64_Sub(JUNO_VEC2_F64_T tVec0, JUNO_VEC2_F64_T tVec1)
{
    tVec0.arr[0] -= tVec1.arr[0];
    tVec0.arr[1] -= tVec1.arr[1];
    return tVec0;
}

/**
    Multiply scalar with vec2f's
*/
static inline JUNO_VEC2_F64_T Juno_Vec2_F64_Mult(JUNO_VEC2_F64_T tVec0, double dScalar)
{
    tVec0.arr[0] *= dScalar;
    tVec0.arr[1] *= dScalar;
    return tVec0;
}

/**
    Dot product of two vec2f's
*/
static inline double Juno_Vec2_F64_Dot(JUNO_VEC2_F64_T tVec0, JUNO_VEC2_F64_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[0] + tVec0.arr[1] * tVec1.arr[1];
}

/**
    The cross product of two vec2f's.
    The result is a psedoscalar
*/
static inline double Juno_Vec2_F64_Cross(JUNO_VEC2_F64_T tVec0, JUNO_VEC2_F64_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[1] - tVec0.arr[1] * tVec1.arr[0];
}

/**
    Get L2 norm of vector
*/
static inline double Juno_Vec2_F64_L2Norm(JUNO_VEC2_F64_T tVec0)
{

    return sqrt(Juno_Pow2(tVec0.arr[0]) + Juno_Pow2(tVec0.arr[1]));
}

/**
    Add two vecf's together
*/
static inline JUNO_VEC2_F32_T Juno_Vec2_F32_Add(JUNO_VEC2_F32_T tVec0, JUNO_VEC2_F32_T tVec1)
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
static inline JUNO_VEC2_F32_T Juno_Vec2_F32_Sub(JUNO_VEC2_F32_T tVec0, JUNO_VEC2_F32_T tVec1)
{
    tVec0.arr[0] -= tVec1.arr[0];
    tVec0.arr[1] -= tVec1.arr[1];
    return tVec0;
}

/**
    Multiply scalar with vec2f's
*/
static inline JUNO_VEC2_F32_T Juno_Vec2_F32_Mult(JUNO_VEC2_F32_T tVec0, float dScalar)
{
    tVec0.arr[0] *= dScalar;
    tVec0.arr[1] *= dScalar;
    return tVec0;
}

/**
    Dot product of two vec2f's
*/
static inline float Juno_Vec2_F32_Dot(JUNO_VEC2_F32_T tVec0, JUNO_VEC2_F32_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[0] + tVec0.arr[1] * tVec1.arr[1];
}

/**
    The cross product of two vec2f's.
    The result is a psedoscalar
*/
static inline float Juno_Vec2_F32_Cross(JUNO_VEC2_F32_T tVec0, JUNO_VEC2_F32_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[1] - tVec0.arr[1] * tVec1.arr[0];
}

/**
    Get L2 norm of vector
*/
static inline float Juno_Vec2_F32_L2Norm(JUNO_VEC2_F32_T tVec0)
{

    return sqrtf(Juno_Pow2(tVec0.arr[0]) + Juno_Pow2(tVec0.arr[1]));
}

/// Add two vec2i's
static inline JUNO_VEC2_I32_T Juno_Vec2_I32_Add(JUNO_VEC2_I32_T tVec0, JUNO_VEC2_I32_T tVec1)
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
static inline JUNO_VEC2_I32_T Juno_Vec2_I32_Sub(JUNO_VEC2_I32_T tVec0, JUNO_VEC2_I32_T tVec1)
{
    tVec0.arr[0] -= tVec1.arr[0];
    tVec0.arr[1] -= tVec1.arr[1];
    return tVec0;
}

/**
    Multiply scalar with vec2f's
*/
static inline JUNO_VEC2_I32_T Juno_Vec2_I32_Mult(JUNO_VEC2_I32_T tVec0, int32_t dScalar)
{
    tVec0.arr[0] *= dScalar;
    tVec0.arr[1] *= dScalar;
    return tVec0;
}

/**
    Dot product of two vec2i's
*/
static inline int32_t Juno_Vec2_I32_Dot(JUNO_VEC2_I32_T tVec0, JUNO_VEC2_I32_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[0] + tVec0.arr[1] * tVec1.arr[1];
}

/**
    The cross product of two vec2i's.
    The result is a psedoscalar
*/
static inline int32_t Juno_Vec2_I32_Cross(JUNO_VEC2_I32_T tVec0, JUNO_VEC2_I32_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[1] - tVec0.arr[1] * tVec1.arr[0];
}

/**
    Get L2 norm of vector
*/
static inline float Juno_Vec2_I32_L2Norm(JUNO_VEC2_I32_T tVec0)
{

    return sqrtf(Juno_Pow2(tVec0.arr[0]) + Juno_Pow2(tVec0.arr[1]));
}

/**
    Add two vecf's together
*/
static inline JUNO_VEC3_F64_T Juno_Vec3_F64_Add(JUNO_VEC3_F64_T tVec0, JUNO_VEC3_F64_T tVec1)
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
static inline JUNO_VEC3_F64_T Juno_Vec3_F64_Sub(JUNO_VEC3_F64_T tVec0, JUNO_VEC3_F64_T tVec1)
{
    tVec0.arr[0] -= tVec1.arr[0];
    tVec0.arr[1] -= tVec1.arr[1];
    tVec0.arr[2] -= tVec1.arr[2];
    return tVec0;
}

/**
    Multiply scalar with vec2f's
*/
static inline JUNO_VEC3_F64_T Juno_Vec3_F64_Mult(JUNO_VEC3_F64_T tVec0, double dScalar)
{
    tVec0.arr[0] *= dScalar;
    tVec0.arr[1] *= dScalar;
    tVec0.arr[2] *= dScalar;
    return tVec0;
}

/**
    Dot product of two vec2f's
*/
static inline double Juno_Vec3_F64_Dot(JUNO_VEC3_F64_T tVec0, JUNO_VEC3_F64_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[0] + tVec0.arr[1] * tVec1.arr[1] + tVec0.arr[2] * tVec1.arr[2];
}

/**
    The cross product of two vec2f's.
    The result is a psedoscalar
*/
static inline JUNO_VEC3_F64_T Juno_Vec3_F64_Cross(JUNO_VEC3_F64_T tVec0, JUNO_VEC3_F64_T tVec1)
{
    JUNO_VEC3_F64_T tRes = {{
        tRes.arr[0] = tVec0.arr[1] * tVec1.arr[2] - tVec0.arr[2] * tVec1.arr[1],
        tRes.arr[1] = tVec0.arr[2] * tVec1.arr[0] - tVec0.arr[0] * tVec1.arr[2],
        tRes.arr[2] = tVec0.arr[0] * tVec1.arr[1] - tVec0.arr[1] * tVec1.arr[0]
    }};
    return tRes;
}

/**
    Get L2 norm of vector
*/
static inline double Juno_Vec3_F64_L2Norm(JUNO_VEC3_F64_T tVec0)
{

    return sqrtf(Juno_Pow2(tVec0.arr[0]) + Juno_Pow2(tVec0.arr[1]) + Juno_Pow2(tVec0.arr[2]));
}

/**
    Add two vecf's together
*/
static inline JUNO_VEC3_F32_T Juno_Vec3_F32_Add(JUNO_VEC3_F32_T tVec0, JUNO_VEC3_F32_T tVec1)
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
static inline JUNO_VEC3_F32_T Juno_Vec3_F32_Sub(JUNO_VEC3_F32_T tVec0, JUNO_VEC3_F32_T tVec1)
{
    tVec0.arr[0] -= tVec1.arr[0];
    tVec0.arr[1] -= tVec1.arr[1];
    tVec0.arr[2] -= tVec1.arr[2];
    return tVec0;
}

/**
    Multiply scalar with vec2f's
*/
static inline JUNO_VEC3_F32_T Juno_Vec3_F32_Mult(JUNO_VEC3_F32_T tVec0, float dScalar)
{
    tVec0.arr[0] *= dScalar;
    tVec0.arr[1] *= dScalar;
    tVec0.arr[2] *= dScalar;
    return tVec0;
}

/**
    Dot product of two vec2f's
*/
static inline float Juno_Vec3_F32_Dot(JUNO_VEC3_F32_T tVec0, JUNO_VEC3_F32_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[0] + tVec0.arr[1] * tVec1.arr[1] + tVec0.arr[2] * tVec1.arr[2];
}

/**
    The cross product of two vec2f's.
    The result is a psedoscalar
*/
static inline JUNO_VEC3_F32_T Juno_Vec3_F32_Cross(JUNO_VEC3_F32_T tVec0, JUNO_VEC3_F32_T tVec1)
{
    JUNO_VEC3_F32_T tRes = {{
        tRes.arr[0] = tVec0.arr[1] * tVec1.arr[2] - tVec0.arr[2] * tVec1.arr[1],
        tRes.arr[1] = tVec0.arr[2] * tVec1.arr[0] - tVec0.arr[0] * tVec1.arr[2],
        tRes.arr[2] = tVec0.arr[0] * tVec1.arr[1] - tVec0.arr[1] * tVec1.arr[0]
    }};
    return tRes;
}

/**
    Get L2 norm of vector
*/
static inline float Juno_Vec3_F32_L2Norm(JUNO_VEC3_F32_T tVec0)
{

    return sqrtf(Juno_Pow2(tVec0.arr[0]) + Juno_Pow2(tVec0.arr[1]) + Juno_Pow2(tVec0.arr[2]));
}

/// Add two vec2i's
static inline JUNO_VEC3_I32_T Juno_Vec3_I32_Add(JUNO_VEC3_I32_T tVec0, JUNO_VEC3_I32_T tVec1)
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
static inline JUNO_VEC3_I32_T Juno_Vec3_I32_Sub(JUNO_VEC3_I32_T tVec0, JUNO_VEC3_I32_T tVec1)
{
    tVec0.arr[0] -= tVec1.arr[0];
    tVec0.arr[1] -= tVec1.arr[1];
    tVec0.arr[2] -= tVec1.arr[2];
    return tVec0;
}

/**
    Multiply scalar with vec2f's
*/
static inline JUNO_VEC3_I32_T Juno_Vec3_I32_Mult(JUNO_VEC3_I32_T tVec0, int32_t dScalar)
{
    tVec0.arr[0] *= dScalar;
    tVec0.arr[1] *= dScalar;
    tVec0.arr[2] *= dScalar;
    return tVec0;
}

/**
    Dot product of two vec2i's
*/
static inline int32_t Juno_Vec3_I32_Dot(JUNO_VEC3_I32_T tVec0, JUNO_VEC3_I32_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[0] + tVec0.arr[1] * tVec1.arr[1] + tVec0.arr[2] * tVec1.arr[2];
}

/**
    The cross product of two vec2i's.
    The result is a psedoscalar
*/
static inline JUNO_VEC3_I32_T Juno_Vec3_I32_Cross(JUNO_VEC3_I32_T tVec0, JUNO_VEC3_I32_T tVec1)
{
    JUNO_VEC3_I32_T tRes = {{
        tVec0.arr[1] * tVec1.arr[2] - tVec0.arr[2] * tVec1.arr[1],
        tVec0.arr[2] * tVec1.arr[0] - tVec0.arr[0] * tVec1.arr[2],
        tVec0.arr[0] * tVec1.arr[1] - tVec0.arr[1] * tVec1.arr[0]
    }};
    return tRes;
}

/**
    Get L2 norm of vector
*/
static inline float Juno_Vec3_I32_L2Norm(JUNO_VEC3_I32_T tVec0)
{

    return sqrtf(Juno_Pow2(tVec0.arr[0]) + Juno_Pow2(tVec0.arr[1]) + Juno_Pow2(tVec0.arr[2]));
}

#ifdef __cplusplus
}
#endif
#endif
