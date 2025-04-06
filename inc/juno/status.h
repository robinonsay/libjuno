#ifndef JUNO_STATUS_H
#define JUNO_STATUS_H
#include <stdio.h>
#include <string.h>

typedef enum JUNO_STATUS_TAG
{
    JUNO_STATUS_SUCCESS             = 0,
    JUNO_STATUS_ERR                 = 1,
    JUNO_STATUS_NULLPTR_ERROR       = 2,
    JUNO_STATUS_MEMALLOC_ERROR      = 3,
    JUNO_STATUS_MEMFREE_ERROR       = 4,
    JUNO_STATUS_INVALID_TYPE_ERROR  = 5,
    JUNO_STATUS_INVALID_SIZE_ERROR  = 6,
} JUNO_STATUS_T;

typedef void JUNO_USER_DATA_T;
typedef void (*JUNO_FAILURE_HANDLER_T)(JUNO_STATUS_T tStatus, const char *pcCustomMessage, JUNO_USER_DATA_T *pvUserData);

#ifndef FAIL_MESSAGE_LEN
#define FAIL_MESSAGE_LEN    256
#endif
#define FAIL(tStatus, pfcnFailureHandler, pvUserData, ...)\
char pcMessage[FAIL_MESSAGE_LEN] = {0};\
snprintf(pcMessage, FAIL_MESSAGE_LEN, __VA_ARGS__);\
if(pfcnFailureHandler){pfcnFailureHandler(tStatus, pcMessage, pvUserData);}

#endif
