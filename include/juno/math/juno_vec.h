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

/**
 * @file juno_vec.h
 * @brief Inline vector and quaternion operations for Juno math types.
 * @ingroup juno_math
 * @details
 *  Provides basic arithmetic (add/sub/scale), dot/cross products, and L2
 *  norms (often squared) for 2D and 3D vectors across double/float/int32
 *  precisions, plus quaternion operations (add/sub/scale, Hamilton product,
 *  conjugate, reciprocal). See function comments for squared vs true norm.
 */

#ifndef JUNO_VEC_H
#define JUNO_VEC_H

#include "juno/math/juno_vec_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def Juno_Pow2
 * @brief Square of a value.
 * @param d Input value.
 * @return d*d
 */
#define Juno_Pow2(d) (d*d)
/**
 * @def Juno_Pow3
 * @brief Cube of a value.
 * @param d Input value.
 * @return d*d*d
 */
#define Juno_Pow3(d) (d*d*d)
/**
 * @def Juno_Pow4
 * @brief Fourth power of a value.
 * @param d Input value.
 * @return d*d*d*d
 */
#define Juno_Pow4(d) (d*d*d*d)

/**
 * @brief Add two 2D vectors (double).
 * @param tVec0 First vector (augend).
 * @param tVec1 Second vector (addend).
 * @return Component-wise sum tVec0 + tVec1.
 */
static inline JUNO_VEC2_F64_T Juno_Vec2_F64_Add(JUNO_VEC2_F64_T tVec0, JUNO_VEC2_F64_T tVec1)
{
    tVec0.arr[0] += tVec1.arr[0];
    tVec0.arr[1] += tVec1.arr[1];
    return tVec0;
}


/**
 * @brief Subtract two 2D vectors (double).
 * @param tVec0 Minuend vector (from which tVec1 is subtracted).
 * @param tVec1 Subtrahend vector.
 * @return Component-wise difference tVec0 - tVec1.
 */
static inline JUNO_VEC2_F64_T Juno_Vec2_F64_Sub(JUNO_VEC2_F64_T tVec0, JUNO_VEC2_F64_T tVec1)
{
    tVec0.arr[0] -= tVec1.arr[0];
    tVec0.arr[1] -= tVec1.arr[1];
    return tVec0;
}

/**
 * @brief Scale a 2D vector by a scalar (double).
 * @param tVec0 Input vector.
 * @param dScalar Scalar multiplier.
 * @return Component-wise product tVec0 * dScalar.
 */
static inline JUNO_VEC2_F64_T Juno_Vec2_F64_Mult(JUNO_VEC2_F64_T tVec0, double dScalar)
{
    tVec0.arr[0] *= dScalar;
    tVec0.arr[1] *= dScalar;
    return tVec0;
}

/**
 * @brief Dot product of two 2D vectors (double).
 * @return tVec0.x*tVec1.x + tVec0.y*tVec1.y
 */
static inline double Juno_Vec2_F64_Dot(JUNO_VEC2_F64_T tVec0, JUNO_VEC2_F64_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[0] + tVec0.arr[1] * tVec1.arr[1];
}

/**
 * @brief 2D cross product (double).
 * @details Returns the pseudoscalar z-component of the 3D cross product of
 *          (x0, y0, 0) × (x1, y1, 0): x0*y1 - y0*x1.
 */
static inline double Juno_Vec2_F64_Cross(JUNO_VEC2_F64_T tVec0, JUNO_VEC2_F64_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[1] - tVec0.arr[1] * tVec1.arr[0];
}

/**
 * @brief Squared L2 norm of a 2D vector (double).
 * @return x^2 + y^2
 */
static inline double Juno_Vec2_F64_L2Norm2(JUNO_VEC2_F64_T tVec0)
{

    return Juno_Pow2(tVec0.arr[0]) + Juno_Pow2(tVec0.arr[1]);
}

/**
 * @brief Add two 2D vectors (float).
 */
static inline JUNO_VEC2_F32_T Juno_Vec2_F32_Add(JUNO_VEC2_F32_T tVec0, JUNO_VEC2_F32_T tVec1)
{
    tVec0.arr[0] += tVec1.arr[0];
    tVec0.arr[1] += tVec1.arr[1];
    return tVec0;
}


/**
 * @brief Subtract two 2D vectors (float).
 */
static inline JUNO_VEC2_F32_T Juno_Vec2_F32_Sub(JUNO_VEC2_F32_T tVec0, JUNO_VEC2_F32_T tVec1)
{
    tVec0.arr[0] -= tVec1.arr[0];
    tVec0.arr[1] -= tVec1.arr[1];
    return tVec0;
}

/**
 * @brief Scale a 2D vector by a scalar (float).
 */
static inline JUNO_VEC2_F32_T Juno_Vec2_F32_Mult(JUNO_VEC2_F32_T tVec0, float dScalar)
{
    tVec0.arr[0] *= dScalar;
    tVec0.arr[1] *= dScalar;
    return tVec0;
}

/**
 * @brief Dot product of two 2D vectors (float).
 */
static inline float Juno_Vec2_F32_Dot(JUNO_VEC2_F32_T tVec0, JUNO_VEC2_F32_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[0] + tVec0.arr[1] * tVec1.arr[1];
}

/**
 * @brief 2D cross product (float); returns pseudoscalar z-component.
 */
static inline float Juno_Vec2_F32_Cross(JUNO_VEC2_F32_T tVec0, JUNO_VEC2_F32_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[1] - tVec0.arr[1] * tVec1.arr[0];
}

/**
 * @brief Squared L2 norm of a 2D vector (float).
 */
static inline float Juno_Vec2_F32_L2Norm2(JUNO_VEC2_F32_T tVec0)
{

    return Juno_Pow2(tVec0.arr[0]) + Juno_Pow2(tVec0.arr[1]);
}

/**
 * @brief Add two 2D vectors (int32).
 */
static inline JUNO_VEC2_I32_T Juno_Vec2_I32_Add(JUNO_VEC2_I32_T tVec0, JUNO_VEC2_I32_T tVec1)
{
    tVec0.arr[0] += tVec1.arr[0];
    tVec0.arr[1] += tVec1.arr[1];
    return tVec0;
}


/**
 * @brief Subtract two 2D vectors (int32).
 */
static inline JUNO_VEC2_I32_T Juno_Vec2_I32_Sub(JUNO_VEC2_I32_T tVec0, JUNO_VEC2_I32_T tVec1)
{
    tVec0.arr[0] -= tVec1.arr[0];
    tVec0.arr[1] -= tVec1.arr[1];
    return tVec0;
}

/**
 * @brief Scale a 2D vector by a scalar (int32).
 */
static inline JUNO_VEC2_I32_T Juno_Vec2_I32_Mult(JUNO_VEC2_I32_T tVec0, int32_t dScalar)
{
    tVec0.arr[0] *= dScalar;
    tVec0.arr[1] *= dScalar;
    return tVec0;
}

/**
 * @brief Dot product of two 2D vectors (int32).
 */
static inline int32_t Juno_Vec2_I32_Dot(JUNO_VEC2_I32_T tVec0, JUNO_VEC2_I32_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[0] + tVec0.arr[1] * tVec1.arr[1];
}

/**
 * @brief 2D cross product (int32); returns pseudoscalar z-component.
 */
static inline int32_t Juno_Vec2_I32_Cross(JUNO_VEC2_I32_T tVec0, JUNO_VEC2_I32_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[1] - tVec0.arr[1] * tVec1.arr[0];
}

/**
 * @brief Squared L2 norm of a 2D vector (int32 inputs; float return).
 * @return (float)(x^2 + y^2)
 */
static inline float Juno_Vec2_I32_L2Norm2(JUNO_VEC2_I32_T tVec0)
{

    return Juno_Pow2(tVec0.arr[0]) + Juno_Pow2(tVec0.arr[1]);
}

/**
 * @brief Add two 3D vectors (double).
 */
static inline JUNO_VEC3_F64_T Juno_Vec3_F64_Add(JUNO_VEC3_F64_T tVec0, JUNO_VEC3_F64_T tVec1)
{
    tVec0.arr[0] += tVec1.arr[0];
    tVec0.arr[1] += tVec1.arr[1];
    tVec0.arr[2] += tVec1.arr[2];
    return tVec0;
}


/**
 * @brief Subtract two 3D vectors (double).
 */
static inline JUNO_VEC3_F64_T Juno_Vec3_F64_Sub(JUNO_VEC3_F64_T tVec0, JUNO_VEC3_F64_T tVec1)
{
    tVec0.arr[0] -= tVec1.arr[0];
    tVec0.arr[1] -= tVec1.arr[1];
    tVec0.arr[2] -= tVec1.arr[2];
    return tVec0;
}

/**
 * @brief Scale a 3D vector by a scalar (double).
 */
static inline JUNO_VEC3_F64_T Juno_Vec3_F64_Mult(JUNO_VEC3_F64_T tVec0, double dScalar)
{
    tVec0.arr[0] *= dScalar;
    tVec0.arr[1] *= dScalar;
    tVec0.arr[2] *= dScalar;
    return tVec0;
}

/**
 * @brief Dot product of two 3D vectors (double).
 */
static inline double Juno_Vec3_F64_Dot(JUNO_VEC3_F64_T tVec0, JUNO_VEC3_F64_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[0] + tVec0.arr[1] * tVec1.arr[1] + tVec0.arr[2] * tVec1.arr[2];
}

/**
 * @brief 3D cross product (double).
 * @details Returns a vector orthogonal to inputs: (x0,y0,z0)×(x1,y1,z1).
 */
static inline JUNO_VEC3_F64_T Juno_Vec3_F64_Cross(JUNO_VEC3_F64_T tVec0, JUNO_VEC3_F64_T tVec1)
{
    JUNO_VEC3_F64_T tRes = {{
        tVec0.arr[1] * tVec1.arr[2] - tVec0.arr[2] * tVec1.arr[1],
        tVec0.arr[2] * tVec1.arr[0] - tVec0.arr[0] * tVec1.arr[2],
        tVec0.arr[0] * tVec1.arr[1] - tVec0.arr[1] * tVec1.arr[0]
    }};
    return tRes;
}

/**
 * @brief Squared L2 norm of a 3D vector (double).
 */
static inline double Juno_Vec3_F64_L2Norm2(JUNO_VEC3_F64_T tVec0)
{

    return Juno_Pow2(tVec0.arr[0]) + Juno_Pow2(tVec0.arr[1]) + Juno_Pow2(tVec0.arr[2]);
}

/**
 * @brief Add two 3D vectors (float).
 */
static inline JUNO_VEC3_F32_T Juno_Vec3_F32_Add(JUNO_VEC3_F32_T tVec0, JUNO_VEC3_F32_T tVec1)
{
    tVec0.arr[0] += tVec1.arr[0];
    tVec0.arr[1] += tVec1.arr[1];
    tVec0.arr[2] += tVec1.arr[2];
    return tVec0;
}


/**
 * @brief Subtract two 3D vectors (float).
 */
static inline JUNO_VEC3_F32_T Juno_Vec3_F32_Sub(JUNO_VEC3_F32_T tVec0, JUNO_VEC3_F32_T tVec1)
{
    tVec0.arr[0] -= tVec1.arr[0];
    tVec0.arr[1] -= tVec1.arr[1];
    tVec0.arr[2] -= tVec1.arr[2];
    return tVec0;
}

/**
 * @brief Scale a 3D vector by a scalar (float).
 */
static inline JUNO_VEC3_F32_T Juno_Vec3_F32_Mult(JUNO_VEC3_F32_T tVec0, float dScalar)
{
    tVec0.arr[0] *= dScalar;
    tVec0.arr[1] *= dScalar;
    tVec0.arr[2] *= dScalar;
    return tVec0;
}

/**
 * @brief Dot product of two 3D vectors (float).
 */
static inline float Juno_Vec3_F32_Dot(JUNO_VEC3_F32_T tVec0, JUNO_VEC3_F32_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[0] + tVec0.arr[1] * tVec1.arr[1] + tVec0.arr[2] * tVec1.arr[2];
}

/**
 * @brief 3D cross product (float).
 */
static inline JUNO_VEC3_F32_T Juno_Vec3_F32_Cross(JUNO_VEC3_F32_T tVec0, JUNO_VEC3_F32_T tVec1)
{
    JUNO_VEC3_F32_T tRes = {{
        tVec0.arr[1] * tVec1.arr[2] - tVec0.arr[2] * tVec1.arr[1],
        tVec0.arr[2] * tVec1.arr[0] - tVec0.arr[0] * tVec1.arr[2],
        tVec0.arr[0] * tVec1.arr[1] - tVec0.arr[1] * tVec1.arr[0]
    }};
    return tRes;
}

/**
 * @brief Squared L2 norm of a 3D vector (float).
 */
static inline float Juno_Vec3_F32_L2Norm2(JUNO_VEC3_F32_T tVec0)
{

    return (Juno_Pow2(tVec0.arr[0]) + Juno_Pow2(tVec0.arr[1]) + Juno_Pow2(tVec0.arr[2]));
}

/**
 * @brief Add two 3D vectors (int32).
 */
static inline JUNO_VEC3_I32_T Juno_Vec3_I32_Add(JUNO_VEC3_I32_T tVec0, JUNO_VEC3_I32_T tVec1)
{
    tVec0.arr[0] += tVec1.arr[0];
    tVec0.arr[1] += tVec1.arr[1];
    tVec0.arr[2] += tVec1.arr[2];
    return tVec0;
}


/**
 * @brief Subtract two 3D vectors (int32).
 */
static inline JUNO_VEC3_I32_T Juno_Vec3_I32_Sub(JUNO_VEC3_I32_T tVec0, JUNO_VEC3_I32_T tVec1)
{
    tVec0.arr[0] -= tVec1.arr[0];
    tVec0.arr[1] -= tVec1.arr[1];
    tVec0.arr[2] -= tVec1.arr[2];
    return tVec0;
}

/**
 * @brief Scale a 3D vector by a scalar (int32).
 */
static inline JUNO_VEC3_I32_T Juno_Vec3_I32_Mult(JUNO_VEC3_I32_T tVec0, int32_t dScalar)
{
    tVec0.arr[0] *= dScalar;
    tVec0.arr[1] *= dScalar;
    tVec0.arr[2] *= dScalar;
    return tVec0;
}

/**
 * @brief Dot product of two 3D vectors (int32).
 */
static inline int32_t Juno_Vec3_I32_Dot(JUNO_VEC3_I32_T tVec0, JUNO_VEC3_I32_T tVec1)
{
    return tVec0.arr[0] * tVec1.arr[0] + tVec0.arr[1] * tVec1.arr[1] + tVec0.arr[2] * tVec1.arr[2];
}

/**
 * @brief 3D cross product (int32).
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
 * @brief Squared L2 norm of a 3D vector (int32 inputs; float return).
 */
static inline float Juno_Vec3_I32_L2Norm2(JUNO_VEC3_I32_T tVec0)
{

    return (Juno_Pow2(tVec0.arr[0]) + Juno_Pow2(tVec0.arr[1]) + Juno_Pow2(tVec0.arr[2]));
}

/**
 * @brief Add two real quaternions (double).
 * @details Component-wise addition using [s, i, j, k] order.
 */
static inline JUNO_RQUAT_F64_T Juno_RQuat_F64_Add(JUNO_RQUAT_F64_T q0, JUNO_RQUAT_F64_T q1)
{
    q0.arr[0] += q1.arr[0];
    q0.arr[1] += q1.arr[1];
    q0.arr[2] += q1.arr[2];
    q0.arr[3] += q1.arr[3];
    return q0;
}


/**
 * @brief Subtract two real quaternions (double).
 */
static inline JUNO_RQUAT_F64_T Juno_RQuat_F64_Sub(JUNO_RQUAT_F64_T q0, JUNO_RQUAT_F64_T q1)
{
    q0.arr[0] -= q1.arr[0];
    q0.arr[1] -= q1.arr[1];
    q0.arr[2] -= q1.arr[2];
    q0.arr[3] -= q1.arr[3];
    return q0;
}

/**
 * @brief Scale a real quaternion by a scalar (double).
 */
static inline JUNO_RQUAT_F64_T Juno_RQuat_F64_Mult(JUNO_RQUAT_F64_T q0, double dScalar)
{
    q0.arr[0] *= dScalar;
    q0.arr[1] *= dScalar;
    q0.arr[2] *= dScalar;
    q0.arr[3] *= dScalar;
    return q0;
}

/**
 * @brief Hamilton product of two right-handed real quaternions (double).
 * @details
 *  Given q0 = (s0, i0, j0, k0) and q1 = (s1, i1, j1, k1) with i^2 = j^2 = k^2 = ijk = -1
 *  and component order [s, i, j, k], the product q = q0 ⊗ q1 is:
 *   - s' = s0*s1 − i0*i1 − j0*j1 − k0*k1
 *   - i' = s0*i1 + i0*s1 + j0*k1 − k0*j1
 *   - j' = s0*j1 − i0*k1 + j0*s1 + k0*i1
 *   - k' = s0*k1 + i0*j1 − j0*i1 + k0*s1
 */
static inline JUNO_RQUAT_F64_T Juno_RQuat_F64_HamProd(JUNO_RQUAT_F64_T q0, JUNO_RQUAT_F64_T q1)
{
    JUNO_RQUAT_F64_T tRes = {{
        q0.tQuat.s * q1.tQuat.s  -  q0.tQuat.i * q1.tQuat.i  -  q0.tQuat.j * q1.tQuat.j  -  q0.tQuat.k * q1.tQuat.k,
        q0.tQuat.s * q1.tQuat.i  +  q0.tQuat.i * q1.tQuat.s  +  q0.tQuat.j * q1.tQuat.k  -  q0.tQuat.k * q1.tQuat.j,
        q0.tQuat.s * q1.tQuat.j  -  q0.tQuat.i * q1.tQuat.k  +  q0.tQuat.j * q1.tQuat.s  +  q0.tQuat.k * q1.tQuat.i,
        q0.tQuat.s * q1.tQuat.k  +  q0.tQuat.i * q1.tQuat.j  -  q0.tQuat.j * q1.tQuat.i  +  q0.tQuat.k * q1.tQuat.s
    }};
    return tRes;
}

/**
 * @brief Conjugate of a right-handed real quaternion (double).
 * @details Negates the vector part: (s, i, j, k) -> (s, -i, -j, -k).
 */
static inline JUNO_RQUAT_F64_T Juno_RQuat_F64_Conj(JUNO_RQUAT_F64_T q0)
{
    q0.tQuat.i = -q0.tQuat.i;
    q0.tQuat.j = -q0.tQuat.j;
    q0.tQuat.k = -q0.tQuat.k;
    return q0;
}

/**
 * @brief Squared L2 norm of a quaternion (double).
 */
static inline double Juno_RQuat_F64_L2Norm2(JUNO_RQUAT_F64_T q0)
{
    return Juno_Pow2(q0.arr[0]) + Juno_Pow2(q0.arr[1]) + Juno_Pow2(q0.arr[2]) + Juno_Pow2(q0.arr[3]);
}


/**
 * @brief Multiplicative inverse (reciprocal) of a quaternion (double).
 * @details Returns conj(q) / ||q||^2. Caller must ensure non-zero norm.
 */
static inline JUNO_RQUAT_F64_T Juno_RQuat_F64_Recip(JUNO_RQUAT_F64_T q0)
{
    return Juno_RQuat_F64_Mult(Juno_RQuat_F64_Conj(q0), 1/Juno_RQuat_F64_L2Norm2(q0));
}

#ifdef __cplusplus
}
#endif
#endif
