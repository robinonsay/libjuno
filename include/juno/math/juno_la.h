#ifndef JUNO_MATH_VEC_H
#define JUNO_MATH_VEC_H

#include <stddef.h>
#include "juno/module.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct JUNO_MATH_MAT_API_T JUNO_MATH_MAT_API_T;
JUNO_MODULE_DECLARE(JUNO_MATH_MAT_T);

typedef struct JUNO_MATH_MAT_ROOT_TAG JUNO_MODULE_ROOT(JUNO_MATH_MAT_API_T, JUNO_MODULE_EMPTY) JUNO_MATH_MAT_ROOT_T;


typedef struct JUNO_MATH_MAT_FLOAT_VEC3_TAG JUNO_MODULE_DERIVE(JUNO_MATH_MAT_ROOT_T,
    float i;
    float j;
    float k;
) JUNO_MATH_MAT_FLOAT_VEC3_T;

#ifdef __cplusplus
}
#endif
#endif
