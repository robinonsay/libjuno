#ifndef JUNO_MATH_NARR_H
#define JUNO_MATH_NARR_H

#include "juno/module.h"
#include "juno/types.h"
#include <stddef.h>
#include <stdint.h>
#include "juno_math_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/// The linear algebra API
typedef struct JUNO_MATH_NARR_API_T JUNO_MATH_NARR_API_T;
JUNO_MODULE_DECLARE(JUNO_MATH_NARR_T);

/// The NARR Root
typedef struct JUNO_MATH_NARR_ROOT_TAG JUNO_MODULE_ROOT(JUNO_MATH_NARR_API_T,

) JUNO_MATH_NARR_ROOT_T;

/// Get a value at an index of an narr
#define Juno_MathNarr_Get(narr, nIndex, mIndex)  narr.data[nIndex * narr.shape.m + mIndex]
/// Get a float value at an index of an narr
#define Juno_MathNarrf_Get(narr, nIndex, mIndex) Juno_MathNarr_Get(narr, nIndex, mIndex)
/// Get a double value at an index of an narr
#define Juno_MathNarrd_Get(narr, nIndex, mIndex) Juno_MathNarr_Get(narr, nIndex, mIndex)
/// Get a int8 value at an index of an narr
#define Juno_MathNarri8_Get(narr, nIndex, mIndex) Juno_MathNarr_Get(narr, nIndex, mIndex)
/// Get a int16 value at an index of an narr
#define Juno_MathNarri16_Get(narr, nIndex, mIndex) Juno_MathNarr_Get(narr, nIndex, mIndex)
/// Get a int32 value at an index of an narr
#define Juno_MathNarri32_Get(narr, nIndex, mIndex) Juno_MathNarr_Get(narr, nIndex, mIndex)
/// Get a int64 value at an index of an narr
#define Juno_MathNarri64_Get(narr, nIndex, mIndex) Juno_MathNarr_Get(narr, nIndex, mIndex)

typedef struct JUNO_MATH_NARR_INDEX_T
{
    size_t n;
    size_t m;
} JUNO_MATH_NARR_INDEX_T;

/// A Float NARR
typedef struct JUNO_MATH_NARRF_TAG
{
    float *data;
    JUNO_MATH_NARR_INDEX_T shape;
} JUNO_MATH_NARRF_T;

/// Create a new NARRF
static inline JUNO_MATH_NARRF_T Juno_MathNarrf_New(float data[], JUNO_MATH_NARR_INDEX_T shape)
{
    JUNO_MATH_NARRF_T tVec = {data, shape};
    return tVec;
}

/// A double NARR
typedef struct JUNO_MATH_NARRD_TAG
{
    double *data;
    JUNO_MATH_NARR_INDEX_T shape;
} JUNO_MATH_NARRD_T;

/// Create a new double NARR
static inline JUNO_MATH_NARRD_T Juno_MathNarrd_New(double data[], JUNO_MATH_NARR_INDEX_T shape)
{
    JUNO_MATH_NARRD_T tVec = {data, shape};
    return tVec;
}

/// A int8 NARR
typedef struct JUNO_MATH_NARRI8_TAG
{
    int8_t *data;
    JUNO_MATH_NARR_INDEX_T shape;
} JUNO_MATH_NARRI8_T;

/// Create a new int8 NARR
static inline JUNO_MATH_NARRI8_T Juno_MathNarri8_New(int8_t data[], JUNO_MATH_NARR_INDEX_T shape)
{
    JUNO_MATH_NARRI8_T tVec = {data, shape};
    return tVec;
}

/// A int16 NARR
typedef struct JUNO_MATH_NARRI16_TAG
{
    int16_t *data;
    JUNO_MATH_NARR_INDEX_T shape;
} JUNO_MATH_NARRI16_T;

/// Create a new int16 NARR
static inline JUNO_MATH_NARRI16_T Juno_MathNarri16_New(int16_t data[], JUNO_MATH_NARR_INDEX_T shape)
{
    JUNO_MATH_NARRI16_T tVec = {data, shape};
    return tVec;
}

/// A int32 NARR
typedef struct JUNO_MATH_NARRI32_TAG
{
    int32_t *data;
    JUNO_MATH_NARR_INDEX_T shape;
} JUNO_MATH_NARRI32_T;

/// Create a new int32 NARR
static inline JUNO_MATH_NARRI32_T Juno_MathNarri32_New(int32_t data[], JUNO_MATH_NARR_INDEX_T shape)
{
    JUNO_MATH_NARRI32_T tVec = {data, shape};
    return tVec;
}

/// A int64 NARR
typedef struct JUNO_MATH_NARRI64_TAG
{
    int64_t *data;
    JUNO_MATH_NARR_INDEX_T shape;
} JUNO_MATH_NARRI64_T;

/// Create a new int64 NARR
static inline JUNO_MATH_NARRI64_T Juno_MathNarri64_New(int64_t data[], JUNO_MATH_NARR_INDEX_T shape)
{
    JUNO_MATH_NARRI64_T tVec = {data, shape};
    return tVec;
}


#ifdef __cplusplus
}
#endif
#endif
