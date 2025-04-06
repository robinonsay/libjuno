#ifndef JUNO_STRING_DIRECT_H
#define JUNO_STRING_DIRECT_H
#include "juno/status.h"
#include "juno/string.h"
#ifdef __cplusplus
extern "C" {
#endif

JUNO_STATUS_T Juno_StringInit(
    JUNO_STRING_T *ptString,
    char *pcString,
    size_t zLen,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvUserData
);

JUNO_STATUS_T Juno_StringConcat(JUNO_STRING_T *ptString1, JUNO_STRING_T *ptString2);
#ifdef __cplusplus
}
#endif
#endif
