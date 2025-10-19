#include ".local/include/juno/app/app_api.h"
#include ".local/include/juno/log/log_api.h"
#include ".local/include/juno/sb/broker_api.h"
#include ".local/include/juno/time/time_api.h"
#include "engine_app/engine_app.h"
#include "juno/macros.h"
#include "juno/module.h"
#include "juno/status.h"
#include "system_manager_app/system_manager_app.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>

static JUNO_STATUS_T LogDebug(const JUNO_LOG_ROOT_T *ptJunoLog, const char *pcMsg, ...)
{
    (void) ptJunoLog;
    va_list tVarg;
    va_start(tVarg, pcMsg);
    char pcPrintStr[512] = {0};
    vsnprintf(pcPrintStr, sizeof(pcPrintStr), pcMsg, tVarg);
    printf("DEBUG: %s\n", pcPrintStr);
    va_end(tVarg);
    return JUNO_STATUS_SUCCESS;
}
static JUNO_STATUS_T LogInfo(const JUNO_LOG_ROOT_T *ptJunoLog, const char *pcMsg, ...)
{
    (void) ptJunoLog;
    va_list tVarg;
    va_start(tVarg, pcMsg);
    char pcPrintStr[512] = {0};
    vsnprintf(pcPrintStr, sizeof(pcPrintStr), pcMsg, tVarg);
    printf("INFO: %s\n", pcPrintStr);
    va_end(tVarg);
    return JUNO_STATUS_SUCCESS;
}
static JUNO_STATUS_T LogWarning(const JUNO_LOG_ROOT_T *ptJunoLog, const char *pcMsg, ...)
{
    (void) ptJunoLog;
    va_list tVarg;
    va_start(tVarg, pcMsg);
    char pcPrintStr[512] = {0};
    vsnprintf(pcPrintStr, sizeof(pcPrintStr), pcMsg, tVarg);
    printf("WARN: %s\n", pcPrintStr);
    va_end(tVarg);
    return JUNO_STATUS_SUCCESS;
}
static JUNO_STATUS_T LogError(const JUNO_LOG_ROOT_T *ptJunoLog, const char *pcMsg, ...)
{
    (void) ptJunoLog;
    va_list tVarg;
    va_start(tVarg, pcMsg);
    char pcPrintStr[512] = {0};
    vsnprintf(pcPrintStr, sizeof(pcPrintStr), pcMsg, tVarg);
    printf("ERROR: %s\n", pcPrintStr);
    va_end(tVarg);
    return JUNO_STATUS_SUCCESS;
}

/// Get the current time as specified by the implementation
static JUNO_TIMESTAMP_RESULT_T Now(const JUNO_TIME_ROOT_T *ptTime)
{
    JUNO_TIMESTAMP_RESULT_T tResult = {0};
    if(!ptTime)
    {
        tResult.tStatus = JUNO_STATUS_NULLPTR_ERROR;
        return tResult;
    }
    struct timespec tTime = {0};
    clock_gettime(CLOCK_MONOTONIC, &tTime);
    tResult = ptTime->ptApi->NanosToTimestamp(ptTime, tTime.tv_nsec);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    tResult.tOk.iSeconds = tTime.tv_sec;
    return tResult;
}
/// Sleep this thread until a specific time
static JUNO_STATUS_T SleepTo(const JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T tTimeToWakeup)
{
    (void) ptTime;
    (void) tTimeToWakeup;
    return JUNO_STATUS_SUCCESS;
}
/// Sleep this thread for a duration
static JUNO_STATUS_T Sleep(const JUNO_TIME_ROOT_T *ptTime, JUNO_TIMESTAMP_T tDuration)
{
    (void) ptTime;
    (void) tDuration;
    return JUNO_STATUS_SUCCESS;
}

static const JUNO_LOG_API_T gtMyLoggerApi ={
    LogDebug,
    LogInfo,
    LogWarning,
    LogError
};

static const JUNO_TIME_API_T gtTimeApi = JunoTime_TimeApiInit(Now, SleepTo, Sleep);

void FailureHandler(JUNO_STATUS_T tStatus, const char *pcMsg, JUNO_USER_DATA_T *pvUserData)
{
    (void) pvUserData;
    printf("FAILURE: %u | %s\n", tStatus, pcMsg);
}

int main(void)
{
    JUNO_TIME_ROOT_T tTime = {0};
    JUNO_LOG_ROOT_T tLogger = {0};
    JUNO_SB_PIPE_T *tRegistry[10] = {0};
    JUNO_SB_BROKER_ROOT_T tBroker = {0};
    ENGINE_APP_T tEngineApp = {0};
    SYSTEM_MANAGER_APP_T tSystemManagerApp = {0};
    JUNO_STATUS_T tStatus = JunoTime_TimeInit(&tTime, &gtTimeApi, FailureHandler, NULL);
    JUNO_ASSERT_SUCCESS(tStatus, return -1);
    tStatus = JunoLog_LogInit(&tLogger, &gtMyLoggerApi, FailureHandler, NULL);
    JUNO_ASSERT_SUCCESS(tStatus, return -1);
    tStatus = JunoSb_BrokerInit(&tBroker, tRegistry, 10, FailureHandler, NULL);
    JUNO_ASSERT_SUCCESS(tStatus, return -1);
    tStatus = EngineApp_Init(&tEngineApp, &tLogger, &tTime, &tBroker, FailureHandler, NULL);
    JUNO_ASSERT_SUCCESS(tStatus, return -1);
    tStatus = SystemManagerApp(&tSystemManagerApp, &tLogger, &tTime, &tBroker, FailureHandler, NULL);
    JUNO_ASSERT_SUCCESS(tStatus, return -1);
    JUNO_APP_ROOT_T *ptAppList[2] = {
        &tSystemManagerApp.tRoot,
        &tEngineApp.tRoot
    };

    for(size_t i = 0; i < 2; i++)
    {
        tStatus = ptAppList[i]->ptApi->OnInit(ptAppList[i]);
        JUNO_ASSERT_SUCCESS(tStatus, return -1);
    }
    size_t iCounter = 0;
    while(true)
    {
        ptAppList[iCounter]->ptApi->OnProcess(ptAppList[iCounter]);
        iCounter = (iCounter + 1) % 2;
    }
}
