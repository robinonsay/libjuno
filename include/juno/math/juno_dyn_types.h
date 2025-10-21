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

#ifndef JUNO_DYN_TYPES_H
#define JUNO_DYN_TYPES_H

#include "juno/module.h"
#include "juno/status.h"
#include "juno/time/time_api.h"
#include "juno_vec_types.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct JUNO_KSTATE_F64_TAG
{
    JUNO_TIMESTAMP_T tTime;
    JUNO_VEC3_F64_T pos;
    JUNO_VEC3_F64_T vel;
    JUNO_VEC3_F64_T accel;
    JUNO_VEC3_F64_T w;
    JUNO_RQUAT_F64_T att;
} JUNO_KSTATE_F64_T;

JUNO_MODULE_RESULT(JUNO_KSTATE_F64_RESULT_T, JUNO_KSTATE_F64_T);

typedef union JUNO_KMAT_TAG JUNO_KMAT_T;
typedef struct JUNO_KMAT_API_TAG JUNO_KMAT_API_T;

typedef struct JUNO_KMAT_ROOT_TAG JUNO_MODULE_ROOT(JUNO_KMAT_API_T,
    JUNO_TIME_ROOT_T *ptTime;
    JUNO_KSTATE_F64_T tState;
)JUNO_KMAT_ROOT_T;

struct JUNO_KMAT_API_TAG
{
    /// Update the state of the kinematic system
    JUNO_STATUS_T (*UpdateWithDeltas)(JUNO_KMAT_T *ptKmat, JUNO_VEC3_F64_T dVel, JUNO_VEC3_F64_T dAng, JUNO_TIMESTAMP_T tTimestamp);
};

#ifdef __cplusplus
}
#endif
#endif
