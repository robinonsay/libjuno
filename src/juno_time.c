#include "juno/macros.h"
#include "juno/status.h"
#include "juno/time/time_api.h"

const JUNO_TIME_SUBSECONDS_T giSUBSECS_MAX = -1;

JUNO_STATUS_T JunoTime_AddTime(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime, JUNO_TIMESTAMP_T tTimeToAdd)
{
    ASSERT_EXISTS(ptTime && ptRetTime);
    ptRetTime->iSeconds += tTimeToAdd.iSeconds;
    JUNO_TIME_SUBSECONDS_T iDifference = (giSUBSECS_MAX) - 1 - ptRetTime->iSubSeconds;
    if(tTimeToAdd.iSubSeconds < iDifference)
    {
        ptRetTime->iSubSeconds += tTimeToAdd.iSubSeconds;
    }
    else
    {
        ptRetTime->iSeconds += 1;
        ptRetTime->iSubSeconds = tTimeToAdd.iSubSeconds - iDifference;
    }
    return JUNO_STATUS_SUCCESS;
}

JUNO_STATUS_T JunoTime_SubtractTime(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime, JUNO_TIMESTAMP_T tTimeToSubtract)
{
    ASSERT_EXISTS(ptTime && ptRetTime);
    JUNO_TIME_ROOT_T *ptTimeRoot = (JUNO_TIME_ROOT_T *)(ptTime);
    if(ptRetTime->iSeconds < tTimeToSubtract.iSeconds)
    {
        ptRetTime->iSeconds = 0;
        ptRetTime->iSubSeconds = 0;
        if(ptTimeRoot->_pfcnFailureHandler)
            ptTimeRoot->_pfcnFailureHandler(JUNO_STATUS_INVALID_DATA_ERROR, "Subtracting invalid time", ptTimeRoot->_pvFailurUserData);
        return JUNO_STATUS_INVALID_DATA_ERROR;
    }
    if(ptRetTime->iSubSeconds < tTimeToSubtract.iSubSeconds)
    {
        ptRetTime->iSeconds -= 1;
        ptRetTime->iSubSeconds = giSUBSECS_MAX - (tTimeToSubtract.iSubSeconds - ptRetTime->iSubSeconds);
    }
    else
    {
        ptRetTime->iSubSeconds -= tTimeToSubtract.iSubSeconds;
    }
    return JUNO_STATUS_SUCCESS;
}

JUNO_STATUS_T JunoTime_TimestampToNanos(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tTime, JUNO_TIME_NANOS_T *piNanos)
{

}

JUNO_STATUS_T JunoTime_TimestampToMicros(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tTime, JUNO_TIME_MICROS_T *piMicros)
{

}

JUNO_STATUS_T JunoTime_TimestampToMillis(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tTime, JUNO_TIME_MILLIS_T *piMillis)
{

}
