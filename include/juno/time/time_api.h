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
    This header contains the time library API
    @author
*/
#ifndef JUNO_TIME_API_H
#define JUNO_TIME_API_H
#include "juno/status.h"
#include "juno/module.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct JUNO_TIME_API_TAG JUNO_TIME_API_T;
JUNO_MODULE_DECLARE(JUNO_TIME_T);
JUNO_MODULE_BASE_DECLARE(JUNO_TIME_BASE_T);
typedef struct JUNO_TIMESTAMP_TAG JUNO_TIMESTAMP_T;

typedef uint64_t JUNO_TIME_SECONDS_T;
typedef uint64_t JUNO_TIME_MILLIS_T;
typedef uint64_t JUNO_TIME_MICROS_T;
typedef uint64_t JUNO_TIME_NANOS_T;

JUNO_MODULE_BASE(JUNO_TIME_BASE_T, JUNO_TIME_API_T, JUNO_MODULE_EMPTY);

struct JUNO_TIMESTAMP_TAG
{
    JUNO_TIME_SECONDS_T iSeconds;
    JUNO_TIME_MILLIS_T iMillis;
    JUNO_TIME_MICROS_T iMicros;
    JUNO_TIME_NANOS_T iNanos;
};

struct JUNO_TIME_API_TAG
{
    JUNO_STATUS_T (*GetTime)(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime);
    JUNO_STATUS_T (*AddTime)(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime, JUNO_TIMESTAMP_T tTimeToAdd);
    JUNO_STATUS_T (*SubtractTime)(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime, JUNO_TIMESTAMP_T tTimeToSubtract);
};

#ifdef __cplusplus
}
#endif
#endif // JUNO_TIME_API_H

