#include "juno/macros.h"
#include "juno/status.h"
#include "juno/time/time_api.h"
#include <stdint.h>

static const JUNO_TIME_SUBSECONDS_T giSUBSECS_MAX = -1;

JUNO_STATUS_T JunoTime_AddTime(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime, JUNO_TIMESTAMP_T tTimeToAdd)
{
    ASSERT_EXISTS(ptTime && ptRetTime);
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

JUNO_STATUS_T JunoTime_SubtractTime(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime, JUNO_TIMESTAMP_T tTimeToSubtract)
{
    ASSERT_EXISTS(ptTime && ptRetTime);
    JUNO_TIME_ROOT_T *ptTimeRoot = (JUNO_TIME_ROOT_T *)(ptTime);
    // Check if we are going to result in negative time
    if(ptRetTime->iSeconds < tTimeToSubtract.iSeconds || (ptRetTime->iSeconds == 0 && ptRetTime->iSubSeconds < tTimeToSubtract.iSubSeconds))
    {
        ptRetTime->iSeconds = 0;
        ptRetTime->iSubSeconds = 0;
        JUNO_FAIL_ROOT(JUNO_STATUS_INVALID_DATA_ERROR, ptTimeRoot, "Subtracting invalid time");
        return JUNO_STATUS_INVALID_DATA_ERROR;
    }
    ptRetTime->iSeconds -= tTimeToSubtract.iSeconds;
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
    return JUNO_STATUS_SUCCESS;
}

JUNO_TIME_NANOS_RESULT_T JunoTime_TimestampToNanos(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tTime)
{
    JUNO_TIME_NANOS_RESULT_T tResult = {0};
    tResult.tStatus = JUNO_STATUS_NULLPTR_ERROR;
    if(!(ptTime))
    {
        return tResult;
    }
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    JUNO_TIME_ROOT_T *ptTimeRoot = (JUNO_TIME_ROOT_T *)(ptTime);
    const JUNO_TIME_NANOS_T iMAX_NANOS = -1;
    const JUNO_TIME_NANOS_T iNANOS_PER_SEC = 1000 * 1000 * 1000;
    // check if seconds -1 would overflow nanos
    if(tTime.iSeconds > iMAX_NANOS / iNANOS_PER_SEC - 1)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_DATA_ERROR;
        JUNO_FAIL_ROOT(tResult.tStatus, ptTimeRoot, "Overflow when converting nanos");
        return tResult;     
    }
    tResult.tSuccess = tTime.iSeconds * iNANOS_PER_SEC;
    tResult.tSuccess += ((double)tTime.iSubSeconds) / giSUBSECS_MAX * iNANOS_PER_SEC ;
    return tResult;
}

JUNO_TIME_MICROS_RESULT_T JunoTime_TimestampToMicros(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tTime)
{
    JUNO_TIME_MICROS_RESULT_T tResult = {0};
    tResult.tStatus = JUNO_STATUS_NULLPTR_ERROR;
    if(!(ptTime))
    {
        return tResult;
    }
    JUNO_TIME_ROOT_T *ptTimeRoot = (JUNO_TIME_ROOT_T *)(ptTime);
    const JUNO_TIME_MICROS_T iMAX_MICROS = -1;
    const JUNO_TIME_MICROS_T iMICROS_PER_SEC = 1000 * 1000;
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    // check if seconds -1 would overflow nanos
    if(tTime.iSeconds > iMAX_MICROS / iMICROS_PER_SEC - 1)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_DATA_ERROR;
        JUNO_FAIL_ROOT(tResult.tStatus, ptTimeRoot, "Overflow when converting nanos");
        return tResult;     
    }
    tResult.tSuccess = tTime.iSeconds * iMICROS_PER_SEC;
    tResult.tSuccess +=  (((double)tTime.iSubSeconds) * iMICROS_PER_SEC) / giSUBSECS_MAX;
    return tResult;
}

JUNO_TIME_MILLIS_RESULT_T JunoTime_TimestampToMillis(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tTime)
{
    JUNO_TIME_MILLIS_RESULT_T tResult = {0};
    tResult.tStatus = JUNO_STATUS_NULLPTR_ERROR;
    if(!(ptTime))
    {
        return tResult;
    }
    JUNO_TIME_ROOT_T *ptTimeRoot = (JUNO_TIME_ROOT_T *)(ptTime);
    const JUNO_TIME_MICROS_T iMAX_MILLIS = -1;
    const JUNO_TIME_MICROS_T iMILLIS_PER_SEC = 1000;
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    // check if seconds -1 would overflow nanos
    if(tTime.iSeconds > iMAX_MILLIS / iMILLIS_PER_SEC - 1)
    {
        tResult.tStatus = JUNO_STATUS_INVALID_DATA_ERROR;
        JUNO_FAIL_ROOT(tResult.tStatus, ptTimeRoot, "Overflow when converting nanos");
        return tResult;     
    }
    tResult.tSuccess = tTime.iSeconds * iMILLIS_PER_SEC;
    tResult.tSuccess += (((double)tTime.iSubSeconds) * iMILLIS_PER_SEC)/ giSUBSECS_MAX;
    return tResult;
}

JUNO_TIMESTAMP_RESULT_T JunoTime_NanosToTimestamp(JUNO_TIME_T *ptTime, JUNO_TIME_NANOS_T iNanos)
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
    tResult.tSuccess.iSeconds = iNanos / iNANOS_PER_SEC;
    tResult.tSuccess.iSubSeconds = (iNanos % iNANOS_PER_SEC) * iSUBSECS_PER_NANO;
    return tResult;
}

JUNO_TIMESTAMP_RESULT_T JunoTime_MicrosToTimestamp(JUNO_TIME_T *ptTime, JUNO_TIME_MICROS_T iMicros)
{
    JUNO_TIMESTAMP_RESULT_T tResult = {0};
    tResult.tStatus = JUNO_STATUS_NULLPTR_ERROR;
    if(!(ptTime))
    {
        return tResult;
    }
    const JUNO_TIME_NANOS_T iMICROS_PER_SEC = 1000 * 1000;
    const JUNO_TIME_SUBSECONDS_T iSUBSECS_PER_MICRO = giSUBSECS_MAX / iMICROS_PER_SEC;
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    tResult.tSuccess.iSeconds = iMicros / iMICROS_PER_SEC;
    tResult.tSuccess.iSubSeconds = (iMicros % iMICROS_PER_SEC) * iSUBSECS_PER_MICRO;
    return tResult;
}

JUNO_TIMESTAMP_RESULT_T JunoTime_MillisToTimestamp(JUNO_TIME_T *ptTime, JUNO_TIME_MILLIS_T iMillis)
{
    JUNO_TIMESTAMP_RESULT_T tResult = {0};
    tResult.tStatus = JUNO_STATUS_NULLPTR_ERROR;
    if(!(ptTime))
    {
        return tResult;
    }
    const JUNO_TIME_NANOS_T iMILLIS_PER_SEC = 1000;
    const JUNO_TIME_SUBSECONDS_T iSUBSECS_PER_MILLI = giSUBSECS_MAX / iMILLIS_PER_SEC;
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    tResult.tSuccess.iSeconds = iMillis / iMILLIS_PER_SEC;
    tResult.tSuccess.iSubSeconds = (iMillis % iMILLIS_PER_SEC) * iSUBSECS_PER_MILLI;
    return tResult;
}


JUNO_RESULT_F64_T JunoTime_TimestampToDouble(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tTimestamp)
{
    JUNO_RESULT_F64_T tResult = {0};
    tResult.tStatus = JUNO_STATUS_NULLPTR_ERROR;
    if(!(ptTime))
    {
        return tResult;
    }
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    tResult.tSuccess = tTimestamp.iSeconds;
    tResult.tSuccess += (double)(tTimestamp.iSubSeconds) / giSUBSECS_MAX;
    return tResult;
}

JUNO_TIMESTAMP_RESULT_T JunoTime_DoubleToTimestamp(JUNO_TIME_T *ptTime, double dTimestamp)
{
    JUNO_TIMESTAMP_RESULT_T tResult = {0};
    tResult.tStatus = JUNO_STATUS_NULLPTR_ERROR;
    if(!(ptTime))
    {
        return tResult;
    }
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    tResult.tSuccess.iSeconds = dTimestamp;
    tResult.tSuccess.iSubSeconds = (dTimestamp - tResult.tSuccess.iSeconds) * giSUBSECS_MAX;
    return tResult;
}

