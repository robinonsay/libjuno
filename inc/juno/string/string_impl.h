#ifndef JUNO_STRING_IMPL_H
#ifdef __cplusplus
extern "C" {
#endif
#include "juno/string/string_direct.h"

static JUNO_STATUS_T Juno_StringValidate(JUNO_STRING_T *ptString)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    if(!(ptString && ptString->pcString))
    {
        tStatus = JUNO_STATUS_NULLPTR_ERROR;
    }
    return tStatus;
}

JUNO_STATUS_T Juno_StringInit(
    JUNO_STRING_T *ptString,
    char *pcString,
    size_t zLen,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvUserData
)
{
    if(!(ptString && pcString))
    {
        return JUNO_STATUS_NULLPTR_ERROR;
    }
    ptString->pcString = pcString;
    ptString->zLen = zLen;
    ptString->pfcnFailureHandler = pfcnFailureHandler;
    ptString->pvUserData = pvUserData;
    return JUNO_STATUS_SUCCESS;
}

JUNO_STATUS_T Juno_StringConcat(JUNO_STRING_T *ptString1, JUNO_STRING_T *ptString2)
{
    ASSERT_EXISTS((ptString1 && ptString2));
    JUNO_STATUS_T tStatus = Juno_StringValidate(ptString1);
    ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = Juno_StringValidate(ptString2);
    ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}

static const JUNO_STRING_API_T tJuno_StringApi =
{
    .Init = Juno_StringInit,
    .Concat = Juno_StringConcat
};

const JUNO_STRING_API_T* Juno_StringApi(void)
{
    return &tJuno_StringApi;
}

#ifdef __cplusplus
}
#endif
#endif // JUNO_STRING_IMPL_H
