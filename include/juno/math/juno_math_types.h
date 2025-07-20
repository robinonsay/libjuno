#ifndef JUNO_MATH_TYPES_H
#define JUNO_MATH_TYPES_H

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

typedef JUNO_MATH_INT_TYPE JUNO_INT_T;
typedef JUNO_MATH_FLOAT_TYPE JUNO_FLOAT_T;

typedef union JUNO_MATH_VEC2F_TAG
{
    struct{JUNO_FLOAT_T i, j;};
    struct{JUNO_FLOAT_T x, y;};
    JUNO_FLOAT_T v[2];
} JUNO_MATH_VEC2F_T;

typedef union JUNO_MATH_VEC2I_TAG
{
    struct{JUNO_INT_T i, j;};
    struct{JUNO_INT_T x, y;};
    JUNO_INT_T v[2];
} JUNO_MATH_VEC2I_T;

typedef union JUNO_MATH_VEC3F_TAG
{
    struct{JUNO_FLOAT_T i, j, k;};
    struct{JUNO_FLOAT_T x, y, z;};
    JUNO_FLOAT_T v[3];
} JUNO_MATH_VEC3F_T;

typedef union JUNO_MATH_VEC3I_TAG
{
    struct{JUNO_INT_T i, j, k;};
    struct{JUNO_INT_T x, y, z;};
    JUNO_INT_T v[3];
} JUNO_MATH_VEC3I_T;

typedef union JUNO_MATH_VEC4F_TAG
{
    struct{JUNO_FLOAT_T i, j, k, l;};
    struct{JUNO_FLOAT_T x, y, z, w;};
    JUNO_FLOAT_T v[4];
} JUNO_MATH_VEC4F_T;

typedef union JUNO_MATH_VEC4I_TAG
{
    struct{JUNO_INT_T i, j, k, l;};
    struct{JUNO_INT_T x, y, z, w;};
    JUNO_INT_T v[4];
} JUNO_MATH_VEC4I_T;

typedef struct JUNO_MATH_M3X3F_TAG
{
    JUNO_FLOAT_T m[3][3];
} JUNO_MATH_M3X3F_T;

typedef struct JUNO_MATH_M4X4F_TAG
{
    JUNO_FLOAT_T m[4][4];
} JUNO_MATH_M4X4F_T;

typedef struct JUNO_MATH_M3X3I_TAG
{
    JUNO_INT_T m[3][3];
} JUNO_MATH_M3X3I_T;

typedef struct JUNO_MATH_M4X4I_TAG
{
    JUNO_INT_T m[4][4];
} JUNO_MATH_M4X4I_T;

typedef union JUNO_MATH_RQUATF_TAG
{
    struct{JUNO_FLOAT_T s, i, j, k;};
    JUNO_FLOAT_T q[4];
} JUNO_MATH_RQUATF_T;

typedef union JUNO_MATH_RQUATI_TAG
{
    struct{JUNO_INT_T s, i, j, k;};
    JUNO_INT_T q[4];
} JUNO_MATH_RQUATI_T;

#ifdef __cplusplus
}
#endif
#endif