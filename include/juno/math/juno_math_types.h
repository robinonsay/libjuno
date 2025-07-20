#ifndef JUNO_MATH_TYPES_H
#define JUNO_MATH_TYPES_H

#include "juno/module.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

#ifndef JUNO_MATH_INT_TYPE
#define JUNO_MATH_INT_TYPE int64_t
#endif

#ifndef JUNO_MATH_FLOAT_TYPE
#define JUNO_MATH_FLOAT_TYPE double
#endif

typedef union JUNO_MATH_VEC2_F64_TAG
{
    struct{double i, j;};
    struct{double x, y;};
    double arr[2];
} JUNO_MATH_VEC2_F64_T;

JUNO_MODULE_RESULT(JUNO_MATH_VEC2F_RESULT_T, JUNO_MATH_VEC2_F64_T);

typedef union JUNO_MATH_VEC2_I32_TAG
{
    struct{int32_t i, j;};
    struct{int32_t x, y;};
    int32_t arr[2];
} JUNO_MATH_VEC2_I32_T;

JUNO_MODULE_RESULT(JUNO_MATH_VEC2I_RESULT_T, JUNO_MATH_VEC2_I32_T);

typedef union JUNO_MATH_VEC3_F64_TAG
{
    struct{double i, j, k;};
    struct{double x, y, z;};
    double arr[3];
} JUNO_MATH_VEC3_F64_T;

JUNO_MODULE_RESULT(JUNO_MATH_VEC3F_RESULT_T, JUNO_MATH_VEC3_F64_T);

typedef union JUNO_MATH_VEC3_I32_TAG
{
    struct{int32_t i, j, k;};
    struct{int32_t x, y, z;};
    int32_t arr[3];
} JUNO_MATH_VEC3_I32_T;

JUNO_MODULE_RESULT(JUNO_MATH_VEC3I_RESULT_T, JUNO_MATH_VEC3_I32_T);

typedef union JUNO_MATH_VEC4_F64_TAG
{
    struct{double i, j, k, l;};
    struct{double x, y, z, w;};
    double arr[4];
} JUNO_MATH_VEC4_F64_T;

JUNO_MODULE_RESULT(JUNO_MATH_VEC4F_RESULT_T, JUNO_MATH_VEC4_F64_T);

typedef union JUNO_MATH_VEC4_I32_TAG
{
    struct{int32_t i, j, k, l;};
    struct{int32_t x, y, z, w;};
    int32_t arr[4];
} JUNO_MATH_VEC4_I32_T;

JUNO_MODULE_RESULT(JUNO_MATH_VEC4I_RESULT_T, JUNO_MATH_VEC4_I32_T);

typedef struct JUNO_MATH_M3X3_F64_TAG
{
    double mat[3][3];
} JUNO_MATH_M3X3_F64_T;

JUNO_MODULE_RESULT(JUNO_MATH_M3X3F_RESULT_T, JUNO_MATH_M3X3_F64_T);

typedef struct JUNO_MATH_M4X4_F64_TAG
{
    double mat[4][4];
} JUNO_MATH_M4X4_F64_T;

JUNO_MODULE_RESULT(JUNO_MATH_M4X4F_RESULT_T, JUNO_MATH_M4X4_F64_T);

typedef struct JUNO_MATH_M3X3_I32_TAG
{
    int32_t mat[3][3];
} JUNO_MATH_M3X3_I32_T;

JUNO_MODULE_RESULT(JUNO_MATH_M3X3I_RESULT_T, JUNO_MATH_M3X3_I32_T);

typedef struct JUNO_MATH_M4X4_I32_TAG
{
    int32_t mat[4][4];
} JUNO_MATH_M4X4_I32_T;

JUNO_MODULE_RESULT(JUNO_MATH_M4X4I_RESULT_T, JUNO_MATH_M4X4_I32_T);

typedef union JUNO_MATH_RQUAT_F64_TAG
{
    struct{double s, i, j, k;};
    double quat[4];
} JUNO_MATH_RQUAT_F64_T;

JUNO_MODULE_RESULT(JUNO_MATH_RQUATF_RESULT_T, JUNO_MATH_RQUAT_F64_T);

typedef union JUNO_MATH_RQUAT_I32_TAG
{
    struct{int32_t s, i, j, k;};
    int32_t quat[4];
} JUNO_MATH_RQUAT_I32_T;

JUNO_MODULE_RESULT(JUNO_MATH_RQUATI_RESULT_T, JUNO_MATH_RQUAT_I32_T);

#ifdef __cplusplus
}
#endif
#endif