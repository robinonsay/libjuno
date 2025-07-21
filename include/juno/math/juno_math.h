#ifndef JUNO_MATH_H
#define JUNO_MATH_H

#include "juno_math_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
    Add two vecf's together
*/
static inline JUNO_MATH_VEC2_F64_T Juno_MathVec2_f64_Add(JUNO_MATH_VEC2_F64_T tVec0, JUNO_MATH_VEC2_F64_T tVec1)
{
    tVec0.i += tVec1.i;
    tVec0.j += tVec1.j;
    return tVec0;
}


/**
    Subtract to vec2f's
    @param tVec0 The vec to subtract from
    @param tVec1 The vec to subtract
*/
static inline JUNO_MATH_VEC2_F64_T Juno_MathVec2_f64_Sub(JUNO_MATH_VEC2_F64_T tVec0, JUNO_MATH_VEC2_F64_T tVec1)
{
    tVec0.i -= tVec1.i;
    tVec0.j -= tVec1.j;
    return tVec0;
}

/**
    Multiply scalar with vec2f's
*/
static inline JUNO_MATH_VEC2_F64_T Juno_MathVec2_f64_Mult(JUNO_MATH_VEC2_F64_T tVec0, double dScalar)
{
    tVec0.i *= dScalar;
    tVec0.j *= dScalar;
    return tVec0;
}

/**
    Dot product of two vec2f's
*/
static inline double Juno_MathVec2_f64_Dot(JUNO_MATH_VEC2_F64_T tVec0, JUNO_MATH_VEC2_F64_T tVec1)
{
    return tVec0.i * tVec1.i + tVec0.j * tVec1.j;
}

/**
    The cross product of two vec2f's.
    The result is a psedoscalar
*/
static inline double Juno_MathVec2_f64_Cross(JUNO_MATH_VEC2_F64_T tVec0, JUNO_MATH_VEC2_F64_T tVec1)
{
    return tVec0.i * tVec1.j - tVec0.j * tVec1.i;
}

/// Add two vec2i's
static inline JUNO_MATH_VEC2_I32_T Juno_MathVec2_i32_Add(JUNO_MATH_VEC2_I32_T tVec0, JUNO_MATH_VEC2_I32_T tVec1)
{
    tVec0.i += tVec1.i;
    tVec0.j += tVec1.j;
    return tVec0;
}


/**
    Subtract to vec2i's
    @param tVec0 The vec to subtract from
    @param tVec1 The vec to subtract
*/
static inline JUNO_MATH_VEC2_I32_T Juno_MathVec2_i32_Sub(JUNO_MATH_VEC2_I32_T tVec0, JUNO_MATH_VEC2_I32_T tVec1)
{
    tVec0.i -= tVec1.i;
    tVec0.j -= tVec1.j;
    return tVec0;
}

/**
    Multiply scalar with vec2f's
*/
static inline JUNO_MATH_VEC2_I32_T Juno_MathVec2_i32_Mult(JUNO_MATH_VEC2_I32_T tVec0, int32_t dScalar)
{
    tVec0.i *= dScalar;
    tVec0.j *= dScalar;
    return tVec0;
}

/**
    Dot product of two vec2i's
*/
static inline int32_t Juno_MathVec2_i32_Dot(JUNO_MATH_VEC2_I32_T tVec0, JUNO_MATH_VEC2_I32_T tVec1)
{
    return tVec0.i * tVec1.i + tVec0.j * tVec1.j;
}

/**
    The cross product of two vec2i's.
    The result is a psedoscalar
*/
static inline int32_t Juno_MathVec2_i32_Cross(JUNO_MATH_VEC2_I32_T tVec0, JUNO_MATH_VEC2_I32_T tVec1)
{
    return tVec0.i * tVec1.j - tVec0.j * tVec1.i;
}

/**
    Add two vecf's together
*/
static inline JUNO_MATH_VEC3_F64_T Juno_MathVec3_f64_Add(JUNO_MATH_VEC3_F64_T tVec0, JUNO_MATH_VEC3_F64_T tVec1)
{
    tVec0.i += tVec1.i;
    tVec0.j += tVec1.j;
    tVec0.k += tVec1.k;
    return tVec0;
}


/**
    Subtract to vec2f's
    @param tVec0 The vec to subtract from
    @param tVec1 The vec to subtract
*/
static inline JUNO_MATH_VEC3_F64_T Juno_MathVec3_f64_Sub(JUNO_MATH_VEC3_F64_T tVec0, JUNO_MATH_VEC3_F64_T tVec1)
{
    tVec0.i -= tVec1.i;
    tVec0.j -= tVec1.j;
    tVec0.k -= tVec1.k;
    return tVec0;
}

/**
    Multiply scalar with vec2f's
*/
static inline JUNO_MATH_VEC3_F64_T Juno_MathVec3_f64_Mult(JUNO_MATH_VEC3_F64_T tVec0, double dScalar)
{
    tVec0.i *= dScalar;
    tVec0.j *= dScalar;
    tVec0.k *= dScalar;
    return tVec0;
}

/**
    Dot product of two vec2f's
*/
static inline double Juno_MathVec3_f64_Dot(JUNO_MATH_VEC3_F64_T tVec0, JUNO_MATH_VEC3_F64_T tVec1)
{
    return tVec0.i * tVec1.i + tVec0.j * tVec1.j + tVec0.k * tVec1.k;
}

/**
    The cross product of two vec2f's.
    The result is a psedoscalar
*/
static inline JUNO_MATH_VEC3_F64_T Juno_MathVec3_f64_Cross(JUNO_MATH_VEC3_F64_T tVec0, JUNO_MATH_VEC3_F64_T tVec1)
{
    JUNO_MATH_VEC3_F64_T tRes = {
        .i = tVec0.j * tVec1.k - tVec0.k * tVec1.j,
        .j = tVec0.k * tVec1.i - tVec0.i * tVec1.k,
        .k = tVec0.i * tVec1.j - tVec0.j * tVec1.i
    };
    return tRes;
}

/// Add two vec2i's
static inline JUNO_MATH_VEC3_I32_T Juno_MathVec3_i32_Add(JUNO_MATH_VEC3_I32_T tVec0, JUNO_MATH_VEC3_I32_T tVec1)
{
    tVec0.i += tVec1.i;
    tVec0.j += tVec1.j;
    tVec0.k += tVec1.k;
    return tVec0;
}


/**
    Subtract to vec2i's
    @param tVec0 The vec to subtract from
    @param tVec1 The vec to subtract
*/
static inline JUNO_MATH_VEC3_I32_T Juno_MathVec3_i32_Sub(JUNO_MATH_VEC3_I32_T tVec0, JUNO_MATH_VEC3_I32_T tVec1)
{
    tVec0.i -= tVec1.i;
    tVec0.j -= tVec1.j;
    tVec0.k -= tVec1.k;
    return tVec0;
}

/**
    Multiply scalar with vec2f's
*/
static inline JUNO_MATH_VEC3_I32_T Juno_MathVec3_i32_Mult(JUNO_MATH_VEC3_I32_T tVec0, int32_t dScalar)
{
    tVec0.i *= dScalar;
    tVec0.j *= dScalar;
    tVec0.k *= dScalar;
    return tVec0;
}

/**
    Dot product of two vec2i's
*/
static inline int32_t Juno_MathVec3_i32_Dot(JUNO_MATH_VEC3_I32_T tVec0, JUNO_MATH_VEC3_I32_T tVec1)
{
    return tVec0.i * tVec1.i + tVec0.j * tVec1.j + tVec0.k * tVec1.k;
}

/**
    The cross product of two vec2i's.
    The result is a psedoscalar
*/
static inline JUNO_MATH_VEC3_I32_T Juno_MathVec3_i32_Cross(JUNO_MATH_VEC3_I32_T tVec0, JUNO_MATH_VEC3_I32_T tVec1)
{
    JUNO_MATH_VEC3_I32_T tRes = {
        .i = tVec0.j * tVec1.k - tVec0.k * tVec1.j,
        .j = tVec0.k * tVec1.i - tVec0.i * tVec1.k,
        .k = tVec0.i * tVec1.j - tVec0.j * tVec1.i
    };
    return tRes;
}

#ifdef __cplusplus
}
#endif
#endif