#ifndef JUNO_MATH_H
#define JUNO_MATH_H

#include "juno/module.h"
#include "juno_math_types.h"
#include "juno_math_constants.h"
#include "juno_narr.h"

#ifdef __cplusplus
extern "C" {
#endif

struct JUNO_MATH_API_T
{
    const JUNO_MATH_NARR_API_T *ptLaApi;
};

#ifdef __cplusplus
}
#endif
#endif