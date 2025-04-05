#include "juno/string.h"
#include "juno/macros.h"
#include "juno/status.h"
#include <stddef.h>

static JUNO_STATUS_T Validate(JUNO_STRING_T *ptString)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    if(!(ptString && ptString->pcString))
    {
        tStatus = JUNO_STATUS_NULLPTR_ERROR;
    }
    return tStatus;
}

static JUNO_STATUS_T Init(
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

static JUNO_STATUS_T Concat(JUNO_STRING_T *ptString1, JUNO_STRING_T *ptString2)
{
    ASSERT_EXISTS((ptString1 && ptString2));
    JUNO_STATUS_T tStatus = Validate(ptString1);
    ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = Validate(ptString2);
    ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}

static const JUNO_STRING_API_T tApi =
{
    .Init = Init,
    .Concat = Concat
};

const JUNO_STRING_API_T* Juno_StringImpl(void)
{
    return &tApi;
}
