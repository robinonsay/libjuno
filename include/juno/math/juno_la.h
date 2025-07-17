#ifndef JUNO_MATH_VEC_H
#define JUNO_MATH_VEC_H

#include "juno/module.h"
#include "juno/types.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct JUNO_MATH_MAT_API_T JUNO_MATH_MAT_API_T;
JUNO_MODULE_DECLARE(JUNO_MATH_MAT_T);

typedef struct JUNO_MATH_MAT_ROOT_TAG JUNO_MODULE_ROOT(JUNO_MATH_MAT_API_T,
    size_t iNDim;
    size_t iMDim;
) JUNO_MATH_MAT_ROOT_T;

typedef struct JUNO_MATH_MAT_NxM_TAG JUNO_MODULE_DERIVE(JUNO_MATH_MAT_ROOT_T,
    float tMat[];
) JUNO_MATH_MAT_NxM_T;

#define JUNO_MATH_MAT_NxM(TYPE_NAME_T, N, M) \
typedef struct TYPE_NAME_T JUNO_MODULE_DERIVE(JUNO_MATH_MAT_ROOT_T, \
    float tMat[N][M]; \
) TYPE_NAME_T;

struct JUNO_MATH_MAT_API_T
{
    JUNO_RESULT_BOOL_T (*Equals)(JUNO_MATH_MAT_T *ptMat1, JUNO_MATH_MAT_T *ptMat2);
};

#ifdef __cplusplus
}
#endif
#endif
