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
typedef union JUNO_TIME_TAG JUNO_TIME_T;
typedef struct JUNO_TIME_ROOT_TAG JUNO_TIME_ROOT_T;
typedef struct JUNO_TIMESTAMP_TAG JUNO_TIMESTAMP_T;

typedef uint64_t JUNO_TIME_SECONDS_T;
typedef uint64_t JUNO_TIME_MILLIS_T;
typedef uint64_t JUNO_TIME_MICROS_T;
typedef uint64_t JUNO_TIME_NANOS_T;
typedef uint64_t JUNO_TIME_SUBSECONDS_T;

/**
    @brief This is the Juno Time Module for all time time related
    operations. Some of the time API function implementations may
    be very implementation specific, so be consciencous of the implementation

    Certain operations regarding time math are implemented by LibJuno for module
    developers to include:
    * `JunoTime_AddTime`
    * `JunoTime_SubtractTime`
    * `JunoTime_TimestampToNanos`
    * `JunoTime_TimestampToMicros`
    * `JunoTime_TimestampToMillis`
    * `JunoTime_NanosToTimestamp`
    * `JunoTime_MicrosToTimestamp`
    * `JunoTime_MillisToTimestamp`
*/
struct JUNO_TIME_ROOT_TAG JUNO_MODULE_ROOT(JUNO_TIME_API_T, JUNO_MODULE_EMPTY);

struct JUNO_TIMESTAMP_TAG
{
    /// Seconds component of time
    JUNO_TIME_SECONDS_T iSeconds;
    /// Subseconds componenet of time.
    // `1 second = sizeof(1 << JUNO_TIME_SUBSECONDS_T) - 1 subseconds`
    JUNO_TIME_SUBSECONDS_T iSubSeconds;
};

struct JUNO_TIME_API_TAG
{
    /// Get the current time as specified by the implementation
    JUNO_STATUS_T (*Now)(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime);
    /// Perform addition with time
    JUNO_STATUS_T (*AddTime)(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime, JUNO_TIMESTAMP_T tTimeToAdd);
    /// Perform subtraction with time
    JUNO_STATUS_T (*SubtractTime)(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime, JUNO_TIMESTAMP_T tTimeToSubtract);
    /// Sleep this thread until a specific time
    JUNO_STATUS_T (*SleepTo)(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tTimeToWakeup);
    /// Sleep this thread for a duration
    JUNO_STATUS_T (*Sleep)(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tDuration);
    /// Convert a timestamp to nanoseconds
    JUNO_STATUS_T (*TimestampToNanos)(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tTime, JUNO_TIME_NANOS_T *piNanos);
    /// Convert a timestamp to microsconds
    JUNO_STATUS_T (*TimestampToMicros)(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tTime, JUNO_TIME_MICROS_T *piMicros);
    /// Convert a timestamp to milliseconds
    JUNO_STATUS_T (*TimestampToMillis)(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tTime, JUNO_TIME_MILLIS_T *piMillis);
    /// Convert nanoseconds to a timestamp
    JUNO_STATUS_T (*NanosToTimestamp)(JUNO_TIME_T *ptTime, JUNO_TIME_NANOS_T iNanos, JUNO_TIMESTAMP_T *ptRetTime);
    /// Convert microseconds to a timestamp
    JUNO_STATUS_T (*MicrosToTimestamp)(JUNO_TIME_T *ptTime, JUNO_TIME_MICROS_T iMicros, JUNO_TIMESTAMP_T *ptRetTime);
    /// Convert milliseconds to a timestamp
    JUNO_STATUS_T (*MillisToTimestamp)(JUNO_TIME_T *ptTime, JUNO_TIME_MILLIS_T iMillis, JUNO_TIMESTAMP_T *ptRetTime);
};

JUNO_STATUS_T JunoTime_AddTime(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime, JUNO_TIMESTAMP_T tTimeToAdd);
JUNO_STATUS_T JunoTime_SubtractTime(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime, JUNO_TIMESTAMP_T tTimeToSubtract);
JUNO_STATUS_T JunoTime_TimestampToNanos(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tTime, JUNO_TIME_NANOS_T *piNanos);
JUNO_STATUS_T JunoTime_TimestampToMicros(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tTime, JUNO_TIME_MICROS_T *piMicros);
JUNO_STATUS_T JunoTime_TimestampToMillis(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tTime, JUNO_TIME_MILLIS_T *piMillis);
JUNO_STATUS_T JunoTime_NanosToTimestamp(JUNO_TIME_T *ptTime, JUNO_TIME_NANOS_T iNanos, JUNO_TIMESTAMP_T *ptRetTime);
JUNO_STATUS_T JunoTime_MicrosToTimestamp(JUNO_TIME_T *ptTime, JUNO_TIME_MICROS_T iMicros, JUNO_TIMESTAMP_T *ptRetTime);
JUNO_STATUS_T JunoTime_MillisToTimestamp(JUNO_TIME_T *ptTime, JUNO_TIME_MILLIS_T iMillis, JUNO_TIMESTAMP_T *ptRetTime);

#ifdef __cplusplus
}
#endif
#endif // JUNO_TIME_API_H

