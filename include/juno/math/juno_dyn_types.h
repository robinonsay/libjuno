#ifndef JUNO_DYN_TYPES_H
#define JUNO_DYN_TYPES_H

#include "juno_vec_types.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct JUNO_KSTATE_F64_TAG
{
    JUNO_VEC3_F64_T pos;
    JUNO_VEC3_F64_T vel;
    JUNO_VEC3_F64_T accel;
    JUNO_VEC3_F64_T w;
    JUNO_RQUAT_F64_T att;
} JUNO_KSTATE_F64_T;

typedef struct JUNO_KSTATE_F32_TAG
{
    JUNO_VEC3_F32_T pos;
    JUNO_VEC3_F32_T vel;
    JUNO_VEC3_F32_T accel;
    JUNO_VEC3_F32_T w;
    JUNO_RQUAT_F32_T att;
} JUNO_KSTATE_F32_T;

#ifdef __cplusplus
}
#endif
#endif
