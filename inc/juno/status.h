#ifndef JUNO_STATUS_H
#define JUNO_STATUS_H
#include <stdio.h>
#include <string.h>

typedef enum JUNO_STATUS_TAG
{
    JUNO_STATUS_SUCCESS         = 0,
    JUNO_STATUS_NULLPTR_ERROR   = 1,
} JUNO_STATUS_T;

typedef void JUNO_USER_DATA_T;
typedef void (*JUNO_FAILURE_HANDLER_T)(JUNO_STATUS_T tStatus, const char *pcCustomMessage, JUNO_USER_DATA_T *pvUserData);

#ifndef FAIL_MESSAGE_LEN
#define FAIL_MESSAGE_LEN    256
#endif
#define FAIL(tStatus, pfcnFailureHandler, pvUserData, format, ...)\
const char pcMessage[FAIL_MESSAGE_LEN] = {};\
snprintf(pcMessage, FAIL_MESSAGE_LEN, format, __VA_ARGS__);\
pfcnFailureHandler(tStatus, pcMessage, pvUserData);

#endif
