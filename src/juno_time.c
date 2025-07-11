#include "juno/macros.h"
#include "juno/status.h"
#include "juno/time/time_api.h"

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

JUNO_STATUS_T JunoTime_TimestampToNanos(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tTime, JUNO_TIME_NANOS_T *piNanos)
{
    ASSERT_EXISTS(ptTime && piNanos);
    JUNO_TIME_ROOT_T *ptTimeRoot = (JUNO_TIME_ROOT_T *)(ptTime);
    const JUNO_TIME_NANOS_T iMAX_NANOS = -1;
    const JUNO_TIME_NANOS_T iNANOS_PER_SEC = 1000 * 1000 * 1000;
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    // check if seconds -1 would overflow nanos
    if(tTime.iSeconds > iMAX_NANOS / iNANOS_PER_SEC - 1)
    {
        tStatus = JUNO_STATUS_INVALID_DATA_ERROR;
        JUNO_FAIL_ROOT(tStatus, ptTimeRoot, "Overflow when converting nanos");
        return tStatus;     
    }
    *piNanos = tTime.iSeconds * iNANOS_PER_SEC;
    *piNanos += tTime.iSubSeconds / giSUBSECS_MAX * iNANOS_PER_SEC;
    return tStatus;
}

JUNO_STATUS_T JunoTime_TimestampToMicros(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tTime, JUNO_TIME_MICROS_T *piMicros)
{
    ASSERT_EXISTS(ptTime && piMicros);
    JUNO_TIME_ROOT_T *ptTimeRoot = (JUNO_TIME_ROOT_T *)(ptTime);
    const JUNO_TIME_MICROS_T iMAX_MICROS = -1;
    const JUNO_TIME_MICROS_T iMICROS_PER_SEC = 1000 * 1000;
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    // check if seconds -1 would overflow nanos
    if(tTime.iSeconds > iMAX_MICROS / iMICROS_PER_SEC - 1)
    {
        tStatus = JUNO_STATUS_INVALID_DATA_ERROR;
        JUNO_FAIL_ROOT(tStatus, ptTimeRoot, "Overflow when converting nanos");
        return tStatus;     
    }
    *piMicros = tTime.iSeconds * iMICROS_PER_SEC;
    *piMicros +=  tTime.iSubSeconds / giSUBSECS_MAX * iMICROS_PER_SEC;
    return tStatus;
}

JUNO_STATUS_T JunoTime_TimestampToMillis(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tTime, JUNO_TIME_MILLIS_T *piMillis)
{
    ASSERT_EXISTS(ptTime && piMillis);
    JUNO_TIME_ROOT_T *ptTimeRoot = (JUNO_TIME_ROOT_T *)(ptTime);
    const JUNO_TIME_MICROS_T iMAX_MILLIS = -1;
    const JUNO_TIME_MICROS_T iMILLIS_PER_SEC = 1000;
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    // check if seconds -1 would overflow nanos
    if(tTime.iSeconds > iMAX_MILLIS / iMILLIS_PER_SEC - 1)
    {
        tStatus = JUNO_STATUS_INVALID_DATA_ERROR;
        JUNO_FAIL_ROOT(tStatus, ptTimeRoot, "Overflow when converting nanos");
        return tStatus;     
    }
    *piMillis = tTime.iSeconds * iMILLIS_PER_SEC;
    *piMillis += tTime.iSubSeconds / giSUBSECS_MAX * iMILLIS_PER_SEC;
    return tStatus;
}

JUNO_STATUS_T JunoTime_NanosToTimestamp(JUNO_TIME_T *ptTime, JUNO_TIME_NANOS_T iNanos, JUNO_TIMESTAMP_T *ptRetTime)
{
    ASSERT_EXISTS(ptTime && ptRetTime);
    const JUNO_TIME_NANOS_T iNANOS_PER_SEC = 1000 * 1000 * 1000;
    const JUNO_TIME_SUBSECONDS_T iSUBSECS_PER_NANO = giSUBSECS_MAX / iNANOS_PER_SEC;
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    ptRetTime->iSeconds = iNanos / iNANOS_PER_SEC;
    ptRetTime->iSubSeconds = (iNanos % iNANOS_PER_SEC) * iSUBSECS_PER_NANO;
    return tStatus;
}
JUNO_STATUS_T JunoTime_MicrosToTimestamp(JUNO_TIME_T *ptTime, JUNO_TIME_MICROS_T iMicros, JUNO_TIMESTAMP_T *ptRetTime)
{
    ASSERT_EXISTS(ptTime && ptRetTime);
    const JUNO_TIME_NANOS_T iMICROS_PER_SEC = 1000 * 1000;
    const JUNO_TIME_SUBSECONDS_T iSUBSECS_PER_MICRO = giSUBSECS_MAX / iMICROS_PER_SEC;
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    ptRetTime->iSeconds = iMicros / iMICROS_PER_SEC;
    ptRetTime->iSubSeconds = (iMicros % iMICROS_PER_SEC) * iSUBSECS_PER_MICRO;
    return tStatus;
}
JUNO_STATUS_T JunoTime_MillisToTimestamp(JUNO_TIME_T *ptTime, JUNO_TIME_MILLIS_T iMillis, JUNO_TIMESTAMP_T *ptRetTime)
{
    ASSERT_EXISTS(ptTime && ptRetTime);
    const JUNO_TIME_NANOS_T iMILLIS_PER_SEC = 1000;
    const JUNO_TIME_SUBSECONDS_T iSUBSECS_PER_MILLI = giSUBSECS_MAX / iMILLIS_PER_SEC;
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    ptRetTime->iSeconds = iMillis / iMILLIS_PER_SEC;
    ptRetTime->iSubSeconds = (iMillis % iMILLIS_PER_SEC) * iSUBSECS_PER_MILLI;
    return tStatus;
}
