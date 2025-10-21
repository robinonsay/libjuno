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

#include "juno/macros.h"
#include "juno/status.h"
#include "juno/time/time_api.h"
#include <stdint.h>

static const JUNO_TIME_SUBSECONDS_T giSUBSECS_MAX = (~(JUNO_TIME_SUBSECONDS_T)0);
static const JUNO_TIME_SECONDS_T giSECS_MAX = (~(JUNO_TIME_SECONDS_T)0);

JUNO_STATUS_T JunoTime_AddTime(const JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime, JUNO_TIMESTAMP_T tTimeToAdd)
{
    JUNO_ASSERT_EXISTS(ptTime && ptRetTime);
    // Add seconds
    ptRetTime->iSeconds += tTimeToAdd.iSeconds;
    // Get the difference from the max value and the current subseconds
    JUNO_TIME_SUBSECONDS_T iDifference = giSUBSECS_MAX - ptRetTime->iSubSeconds;
    // Check if we're adding less than the difference
    if(tTimeToAdd.iSubSeconds < iDifference)
    {
        // We are so add the time
        ptRetTime->iSubSeconds += tTimeToAdd.iSubSeconds;
    }
    else
    {
        // We are not to increment seconds and add overflow to subseconds
        ptRetTime->iSeconds += 1;
        ptRetTime->iSubSeconds = tTimeToAdd.iSubSeconds - iDifference;
    }
    return JUNO_STATUS_SUCCESS;
}

JUNO_STATUS_T JunoTime_SubtractTime(const JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime, JUNO_TIMESTAMP_T tTimeToSubtract)
{
    JUNO_ASSERT_EXISTS(ptTime && ptRetTime);
    JUNO_TIME_ROOT_T *ptTimeRoot = (JUNO_TIME_ROOT_T *)(ptTime);
    // Check if we are going to result in negative time
    if(ptRetTime->iSeconds < tTimeToSubtract.iSeconds || (ptRetTime->iSeconds == tTimeToSubtract.iSeconds && ptRetTime->iSubSeconds < tTimeToSubtract.iSubSeconds))
    {
        ptRetTime->iSeconds = 0;
        ptRetTime->iSubSeconds = 0;
        JUNO_FAIL_ROOT(JUNO_STATUS_INVALID_DATA_ERROR, ptTimeRoot, "Subtracting invalid time");
        return JUNO_STATUS_INVALID_DATA_ERROR;
    }
    // Check if we need to decrement seconds
    if(ptRetTime->iSubSeconds < tTimeToSubtract.iSubSeconds)
    {
        // Decrement seconds and subtract
        ptRetTime->iSeconds -= 1;
        ptRetTime->iSubSeconds = giSUBSECS_MAX - (tTimeToSubtract.iSubSeconds - ptRetTime->iSubSeconds);
    }
    else
    {
        // Just subtract
        ptRetTime->iSubSeconds -= tTimeToSubtract.iSubSeconds;
    }
    ptRetTime->iSeconds -= tTimeToSubtract.iSeconds;
    return JUNO_STATUS_SUCCESS;
}

JUNO_TIME_NANOS_RESULT_T JunoTime_TimestampToNanos(const JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T tTime)
{
    JUNO_TIME_NANOS_RESULT_T tResult = {0};
    tResult.tStatus = JUNO_STATUS_NULLPTR_ERROR;
    if(!(ptTime))
    {
        return tResult;
    }
    JUNO_TIME_ROOT_T *ptTimeRoot = (JUNO_TIME_ROOT_T *)(ptTime);
    const JUNO_TIME_NANOS_T iMAX_NANOS = (JUNO_TIME_NANOS_T)-1;
    const JUNO_TIME_NANOS_T iNANOS_PER_SEC = 1000ULL * 1000ULL * 1000ULL;

    // Compute fractional contribution using integer math
    JUNO_TIME_NANOS_T iFrac = ((JUNO_TIME_NANOS_T)tTime.iSubSeconds * iNANOS_PER_SEC) / giSUBSECS_MAX;

    // Guard seconds multiplication overflow
    if (tTime.iSeconds > iMAX_NANOS / iNANOS_PER_SEC)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_DATA_ERROR;
        JUNO_FAIL_ROOT(tResult.tStatus, ptTimeRoot, "Overflow when converting nanos (seconds)");
        return tResult;
    }
    JUNO_TIME_NANOS_T iBase = (JUNO_TIME_NANOS_T)tTime.iSeconds * iNANOS_PER_SEC;

    // Guard addition overflow
    if (iFrac > iMAX_NANOS - iBase)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_DATA_ERROR;
        JUNO_FAIL_ROOT(tResult.tStatus, ptTimeRoot, "Overflow when converting nanos (fraction)");
        return tResult;
    }

    tResult.tOk = iBase + iFrac;
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    return tResult;
}

JUNO_TIME_MICROS_RESULT_T JunoTime_TimestampToMicros(const JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T tTime)
{
    JUNO_TIME_MICROS_RESULT_T tResult = {0};
    tResult.tStatus = JUNO_STATUS_NULLPTR_ERROR;
    if(!(ptTime))
    {
        return tResult;
    }
    JUNO_TIME_ROOT_T *ptTimeRoot = (JUNO_TIME_ROOT_T *)(ptTime);
    const JUNO_TIME_MICROS_T iMAX_MICROS = (JUNO_TIME_MICROS_T)-1;
    const JUNO_TIME_MICROS_T iMICROS_PER_SEC = 1000ULL * 1000ULL;

    JUNO_TIME_MICROS_T iFrac = ((JUNO_TIME_MICROS_T)tTime.iSubSeconds * iMICROS_PER_SEC) / giSUBSECS_MAX;

    if (tTime.iSeconds > iMAX_MICROS / iMICROS_PER_SEC)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_DATA_ERROR;
        JUNO_FAIL_ROOT(tResult.tStatus, ptTimeRoot, "Overflow when converting micros (seconds)");
        return tResult;
    }
    JUNO_TIME_MICROS_T iBase = (JUNO_TIME_MICROS_T)tTime.iSeconds * iMICROS_PER_SEC;

    if (iFrac > iMAX_MICROS - iBase)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_DATA_ERROR;
        JUNO_FAIL_ROOT(tResult.tStatus, ptTimeRoot, "Overflow when converting micros (fraction)");
        return tResult;
    }

    tResult.tOk = iBase + iFrac;
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    return tResult;
}

JUNO_TIME_MILLIS_RESULT_T JunoTime_TimestampToMillis(const JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T tTime)
{
    JUNO_TIME_MILLIS_RESULT_T tResult = {0};
    tResult.tStatus = JUNO_STATUS_NULLPTR_ERROR;
    if(!(ptTime))
    {
        return tResult;
    }
    JUNO_TIME_ROOT_T *ptTimeRoot = (JUNO_TIME_ROOT_T *)(ptTime);
    const JUNO_TIME_MILLIS_T iMAX_MILLIS = (JUNO_TIME_MILLIS_T)-1;
    const JUNO_TIME_MILLIS_T iMILLIS_PER_SEC = 1000ULL;

    JUNO_TIME_MILLIS_T iFrac = ((JUNO_TIME_MILLIS_T)tTime.iSubSeconds * iMILLIS_PER_SEC) / giSUBSECS_MAX;

    if (tTime.iSeconds > iMAX_MILLIS / iMILLIS_PER_SEC)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_DATA_ERROR;
        JUNO_FAIL_ROOT(tResult.tStatus, ptTimeRoot, "Overflow when converting millis (seconds)");
        return tResult;
    }
    JUNO_TIME_MILLIS_T iBase = (JUNO_TIME_MILLIS_T)tTime.iSeconds * iMILLIS_PER_SEC;

    if (iFrac > iMAX_MILLIS - iBase)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_DATA_ERROR;
        JUNO_FAIL_ROOT(tResult.tStatus, ptTimeRoot, "Overflow when converting millis (fraction)");
        return tResult;
    }

    tResult.tOk = iBase + iFrac;
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    return tResult;
}

JUNO_TIMESTAMP_RESULT_T JunoTime_NanosToTimestamp(const JUNO_TIME_ROOT_T *ptTime, JUNO_TIME_NANOS_T iNanos)
{
    JUNO_TIMESTAMP_RESULT_T tResult = {0};
    tResult.tStatus = JUNO_STATUS_NULLPTR_ERROR;
    if(!(ptTime))
    {
        return tResult;
    }
    const JUNO_TIME_NANOS_T iNANOS_PER_SEC = 1000 * 1000 * 1000;
    const JUNO_TIME_SUBSECONDS_T iSUBSECS_PER_NANO = giSUBSECS_MAX / iNANOS_PER_SEC;
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    tResult.tOk.iSeconds = iNanos / iNANOS_PER_SEC;
    tResult.tOk.iSubSeconds = (iNanos % iNANOS_PER_SEC) * iSUBSECS_PER_NANO;
    return tResult;
}

JUNO_TIMESTAMP_RESULT_T JunoTime_MicrosToTimestamp(const JUNO_TIME_ROOT_T *ptTime, JUNO_TIME_MICROS_T iMicros)
{
    JUNO_TIMESTAMP_RESULT_T tResult = {0};
    tResult.tStatus = JUNO_STATUS_NULLPTR_ERROR;
    if(!(ptTime))
    {
        return tResult;
    }
    const JUNO_TIME_MICROS_T iMICROS_PER_SEC = 1000 * 1000;
    const JUNO_TIME_SUBSECONDS_T iSUBSECS_PER_MICRO = giSUBSECS_MAX / iMICROS_PER_SEC;
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    tResult.tOk.iSeconds = iMicros / iMICROS_PER_SEC;
    tResult.tOk.iSubSeconds = (iMicros % iMICROS_PER_SEC) * iSUBSECS_PER_MICRO;
    return tResult;
}

JUNO_TIMESTAMP_RESULT_T JunoTime_MillisToTimestamp(const JUNO_TIME_ROOT_T *ptTime, JUNO_TIME_MILLIS_T iMillis)
{
    JUNO_TIMESTAMP_RESULT_T tResult = {0};
    tResult.tStatus = JUNO_STATUS_NULLPTR_ERROR;
    if(!(ptTime))
    {
        return tResult;
    }
    const JUNO_TIME_MILLIS_T iMILLIS_PER_SEC = 1000;
    const JUNO_TIME_SUBSECONDS_T iSUBSECS_PER_MILLI = giSUBSECS_MAX / iMILLIS_PER_SEC;
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    tResult.tOk.iSeconds = iMillis / iMILLIS_PER_SEC;
    tResult.tOk.iSubSeconds = (iMillis % iMILLIS_PER_SEC) * iSUBSECS_PER_MILLI;
    return tResult;
}


JUNO_RESULT_F64_T JunoTime_TimestampToDouble(const JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T tTimestamp)
{
    JUNO_RESULT_F64_T tResult = {0};
    tResult.tStatus = JUNO_STATUS_NULLPTR_ERROR;
    if(!(ptTime))
    {
        return tResult;
    }
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    tResult.tOk = tTimestamp.iSeconds;
    tResult.tOk += (double)(tTimestamp.iSubSeconds) / giSUBSECS_MAX;
    return tResult;
}

JUNO_TIMESTAMP_RESULT_T JunoTime_DoubleToTimestamp(const JUNO_TIME_ROOT_T *ptTime, double dTimestamp)
{
    JUNO_TIMESTAMP_RESULT_T tResult = {0};
    tResult.tStatus = JUNO_STATUS_NULLPTR_ERROR;
    if(!(ptTime))
    {
        return tResult;
    }
    if(dTimestamp < 0 || dTimestamp > giSECS_MAX)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_DATA_ERROR;
        return tResult;
    }
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    tResult.tOk.iSeconds = dTimestamp;
    if(dTimestamp - tResult.tOk.iSeconds < 1.0 - 1.0 / giSUBSECS_MAX)
    {
        tResult.tOk.iSubSeconds = (dTimestamp - tResult.tOk.iSeconds) * giSUBSECS_MAX;
    }
    else
    {
        tResult.tOk.iSubSeconds = 0;
        tResult.tOk.iSeconds += 1;
    }
    return tResult;
}

