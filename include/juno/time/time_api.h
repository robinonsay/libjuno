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
 * @file include/juno/time/time_api.h
 * @brief Juno Time module API and common time math helpers.
 *
 * This header declares the Juno Time module interface (JUNO_TIME_API_T) and
 * provides reference implementations for common time math and conversions
 * (add/subtract timestamps; convert between timestamp and nanos/micros/millis
 * and double).
 *
 * Notes on timestamp representation:
 * - JUNO_TIMESTAMP_T uses an integer seconds field (iSeconds) and an
 *   unsigned fractional field (iSubSeconds).
 * - The fractional field is interpreted as a fixed-point fraction of a second
 *   over the full-scale value giSUBSECS_MAX (all-ones of the type). That is,
 *   fraction = (double)iSubSeconds / giSUBSECS_MAX.
 * - Representations are not normalized: iSubSeconds == giSUBSECS_MAX is
 *   equivalent to adding 1 to iSeconds and setting iSubSeconds to 0. Callers
 *   should avoid constructing such non-canonical values unless they plan to
 *   normalize.
 *
 * Conversion/rounding behavior:
 * - Conversions from timestamp to integer nanos/micros/millis truncate the
 *   fractional part toward zero (flooring).
 * - Conversions from double to timestamp use truncation for the integer
 *   seconds and for the fractional-to-subseconds mapping, except for the
 *   maximal fractional bucket: when the fractional part is within one
 *   subseconds unit of 1.0 (i.e., frac >= 1 - 1/giSUBSECS_MAX), the result is
 *   rounded up to the next whole second with subseconds set to 0. Negative
 *   inputs are not supported. Inputs must satisfy
 *   0.0 <= seconds < max(JUNO_TIME_SECONDS_T).
 *
 * Error handling:
 * - Subtraction that would result in negative time returns
 *   JUNO_STATUS_INVALID_DATA_ERROR and saturates the result to 0 seconds and
 *   0 subseconds.
 * - Conversion functions detect overflow and return
 *   JUNO_STATUS_INVALID_DATA_ERROR.
 *
 * @author Robin Onsay
 */
#ifndef JUNO_TIME_API_H
#define JUNO_TIME_API_H
#include "juno/status.h"
#include "juno/module.h"
#include "juno/types.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct JUNO_TIME_API_TAG JUNO_TIME_API_T;
typedef struct JUNO_TIME_ROOT_TAG JUNO_TIME_ROOT_T;
typedef struct JUNO_TIMESTAMP_TAG JUNO_TIMESTAMP_T;

typedef uint64_t JUNO_TIME_SECONDS_T;
typedef uint64_t JUNO_TIME_MILLIS_T;
typedef uint64_t JUNO_TIME_MICROS_T;
typedef uint64_t JUNO_TIME_NANOS_T;
typedef uint32_t JUNO_TIME_SUBSECONDS_T;

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
    /// Whole seconds component of time
    JUNO_TIME_SECONDS_T iSeconds;
    /// Fractional component of time represented over full-scale giSUBSECS_MAX
    /// i.e., fractional_seconds = (double)iSubSeconds / giSUBSECS_MAX
    JUNO_TIME_SUBSECONDS_T iSubSeconds;
};

JUNO_MODULE_RESULT(JUNO_TIMESTAMP_RESULT_T, JUNO_TIMESTAMP_T);
JUNO_MODULE_RESULT(JUNO_TIME_SECONDS_RESULT_T, JUNO_TIME_SECONDS_T);
JUNO_MODULE_RESULT(JUNO_TIME_MILLIS_RESULT_T, JUNO_TIME_MILLIS_T);
JUNO_MODULE_RESULT(JUNO_TIME_MICROS_RESULT_T, JUNO_TIME_MICROS_T);
JUNO_MODULE_RESULT(JUNO_TIME_NANOS_RESULT_T, JUNO_TIME_NANOS_T);
JUNO_MODULE_RESULT(JUNO_TIME_SUBSECONDS_RESULT_T, JUNO_TIME_SUBSECONDS_T);

struct JUNO_TIME_API_TAG
{
    /// Get the current time as specified by the implementation
    JUNO_TIMESTAMP_RESULT_T (*Now)(JUNO_TIME_ROOT_T *ptTime);
    /// Perform addition with time
    JUNO_STATUS_T (*AddTime)(JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime, JUNO_TIMESTAMP_T tTimeToAdd);
    /// Perform subtraction with time
    JUNO_STATUS_T (*SubtractTime)(JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime, JUNO_TIMESTAMP_T tTimeToSubtract);
    /// Sleep this thread until a specific time
    JUNO_STATUS_T (*SleepTo)(JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T tTimeToWakeup);
    /// Sleep this thread for a duration
    JUNO_STATUS_T (*Sleep)(JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T tDuration);
    /// Convert a timestamp to nanoseconds
    JUNO_TIME_NANOS_RESULT_T (*TimestampToNanos)(JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T tTime);
    /// Convert a timestamp to microseconds
    JUNO_TIME_MICROS_RESULT_T (*TimestampToMicros)(JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T tTime);
    /// Convert a timestamp to milliseconds
    JUNO_TIME_MILLIS_RESULT_T (*TimestampToMillis)(JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T tTime);
    /// Convert nanoseconds to a timestamp
    JUNO_TIMESTAMP_RESULT_T (*NanosToTimestamp)(JUNO_TIME_ROOT_T *ptTime, JUNO_TIME_NANOS_T iNanos);
    /// Convert microseconds to a timestamp
    JUNO_TIMESTAMP_RESULT_T (*MicrosToTimestamp)(JUNO_TIME_ROOT_T *ptTime, JUNO_TIME_MICROS_T iMicros);
    /// Convert milliseconds to a timestamp
    JUNO_TIMESTAMP_RESULT_T (*MillisToTimestamp)(JUNO_TIME_ROOT_T *ptTime, JUNO_TIME_MILLIS_T iMillis);
    /// Convert a timestamp to a double
    JUNO_RESULT_F64_T (*TimestampToDouble)(JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T tTimestamp);
    /// Convert a double to a timestamp
    /// Behavior:
    /// - Returns JUNO_STATUS_INVALID_DATA_ERROR if dTimestamp < 0.0 or
    ///   dTimestamp is not representable (>= max(JUNO_TIME_SECONDS_T)).
    /// - Otherwise, seconds are truncated from dTimestamp and the fractional
    ///   part is mapped to subseconds by truncation, except when the fraction
    ///   is within one subseconds unit of 1.0 (>= 1 - 1/giSUBSECS_MAX), in
    ///   which case the result rounds up to the next second with subseconds=0.
    JUNO_TIMESTAMP_RESULT_T (*DoubleToTimestamp)(JUNO_TIME_ROOT_T *ptTime, double dTimestamp);
};


/**
 * @brief Add a duration to a timestamp in-place.
 * @param ptTime Module pointer (used for error reporting).
 * @param ptRetTime In/out timestamp to be updated.
 * @param tTimeToAdd Duration to add.
 * @return JUNO_STATUS_SUCCESS on success.
 */
JUNO_STATUS_T JunoTime_AddTime(JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime, JUNO_TIMESTAMP_T tTimeToAdd);

/**
 * @brief Subtract a duration from a timestamp in-place.
 * If the subtraction would underflow (negative time), the result is saturated
 * to 0 seconds and 0 subseconds, and JUNO_STATUS_INVALID_DATA_ERROR is
 * returned.
 */
JUNO_STATUS_T JunoTime_SubtractTime(JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime, JUNO_TIMESTAMP_T tTimeToSubtract);

/** Conversions from timestamp to integer durations (truncates fractional) */
JUNO_TIME_NANOS_RESULT_T JunoTime_TimestampToNanos(JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T tTime);
JUNO_TIME_MICROS_RESULT_T JunoTime_TimestampToMicros(JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T tTime);
JUNO_TIME_MILLIS_RESULT_T JunoTime_TimestampToMillis(JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T tTime);

/** Conversions from integer durations to timestamp */
JUNO_TIMESTAMP_RESULT_T JunoTime_NanosToTimestamp(JUNO_TIME_ROOT_T *ptTime, JUNO_TIME_NANOS_T iNanos);
JUNO_TIMESTAMP_RESULT_T JunoTime_MicrosToTimestamp(JUNO_TIME_ROOT_T *ptTime, JUNO_TIME_MICROS_T iMicros);
JUNO_TIMESTAMP_RESULT_T JunoTime_MillisToTimestamp(JUNO_TIME_ROOT_T *ptTime, JUNO_TIME_MILLIS_T iMillis);

/** Conversions between timestamp and double (truncating semantics) */
JUNO_RESULT_F64_T JunoTime_TimestampToDouble(JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T tTimestamp);
JUNO_TIMESTAMP_RESULT_T JunoTime_DoubleToTimestamp(JUNO_TIME_ROOT_T *ptTime, double dTimestamp);

#define JUNO_TIME_NEW_API(Now, SleepTo, Sleep) \
{ \
    Now, \
    JunoTime_AddTime, \
    JunoTime_SubtractTime, \
    SleepTo, \
    Sleep, \
    JunoTime_TimestampToNanos, \
    JunoTime_TimestampToMicros, \
    JunoTime_TimestampToMillis, \
    JunoTime_NanosToTimestamp, \
    JunoTime_MicrosToTimestamp, \
    JunoTime_MillisToTimestamp, \
    JunoTime_TimestampToDouble, \
    JunoTime_DoubleToTimestamp, \
}

static inline bool JunoTime_TimestampGreaterThan(JUNO_TIMESTAMP_T tLeft, JUNO_TIMESTAMP_T tRight)
{
    return (tLeft.iSeconds > tRight.iSeconds) || (tLeft.iSeconds == tRight.iSeconds && tLeft.iSubSeconds > tRight.iSubSeconds);
}

static inline bool JunoTime_TimestampLessThan(JUNO_TIMESTAMP_T tLeft, JUNO_TIMESTAMP_T tRight)
{
    return (tLeft.iSeconds < tRight.iSeconds) || (tLeft.iSeconds == tRight.iSeconds && tLeft.iSubSeconds < tRight.iSubSeconds);
}

static inline bool JunoTime_TimestampEquals(JUNO_TIMESTAMP_T tLeft, JUNO_TIMESTAMP_T tRight)
{
    return (tLeft.iSeconds == tRight.iSeconds) && (tLeft.iSubSeconds == tRight.iSubSeconds);
}

#ifdef __cplusplus
}
#endif
#endif // JUNO_TIME_API_H

