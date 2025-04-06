#ifndef JUNO_STRING_H
#define JUNO_STRING_H

#include "juno/macros.h"
#include "juno/status.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct JUNO_STRING_TAG JUNO_STRING_T;
typedef struct JUNO_STRING_API_TAG JUNO_STRING_API_T;

struct JUNO_STRING_TAG
{
    char *pcString;
    size_t zLen;
    DECLARE_FAILURE_HANDLER;
};

struct JUNO_STRING_API_TAG
{
    JUNO_STATUS_T (*Init)(
        JUNO_STRING_T *ptString,
        char *pcString,
        size_t zLen,
        JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
        JUNO_USER_DATA_T *pvUserData
    );
    JUNO_STATUS_T (*Concat)(JUNO_STRING_T *ptString1, JUNO_STRING_T *ptString2);
};

const JUNO_STRING_API_T* Juno_StringApi(void);

#ifdef __cplusplus
}
#endif
#endif // JUNO_STRING_H
