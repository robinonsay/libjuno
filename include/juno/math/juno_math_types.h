#ifndef JUNO_MATH_TYPES_H
#define JUNO_MATH_TYPES_H

#include "juno/module.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct JUNO_MATH_API_T JUNO_MATH_API_T;
JUNO_MODULE_DECLARE(JUNO_MATH_T);

typedef struct JUNO_MATH_ROOT_TAG JUNO_MODULE_ROOT(JUNO_MATH_API_T, JUNO_MODULE_EMPTY) JUNO_MATH_ROOT_T;

#ifdef __cplusplus
}
#endif
#endif