#ifndef JUNO_STRING_API_H
#define JUNO_STRING_API_H
#include "juno/status.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "juno/string/string_types.h"

typedef struct JUNO_STRING_API_TAG JUNO_STRING_API_T;

struct JUNO_STRING_API_TAG
{
    JUNO_STATUS_T (*Init)(
        JUNO_STRING_T *ptString,
        JUNO_MEMORY_ALLOC_T *ptAlloc,
        size_t zLen,
        JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
        JUNO_USER_DATA_T *pvUserData
    );
    JUNO_STATUS_T (*FromCStr)(
        JUNO_STRING_T *ptString,
        JUNO_MEMORY_ALLOC_T *ptAlloc,
        const char *pcStr,
        size_t zLen,
        JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
        JUNO_USER_DATA_T *pvUserData
    );
    JUNO_STATUS_T (*SetAlloc)(JUNO_STRING_T *ptString, JUNO_MEMORY_ALLOC_T *ptAlloc);
    JUNO_STATUS_T (*GetSize)(JUNO_STRING_T *ptString, size_t *pzRetSize);
    /// Concat two strings together
    /// @param ptString1 The first string
    /// @param ptString2 The second string
    /// @param zNewSize The size of the new string. If the size is 0, the new size will be inferred
    JUNO_STATUS_T (*Concat)(JUNO_STRING_T *ptString1, JUNO_STRING_T *ptString2, size_t zNewSize);
    JUNO_STATUS_T (*Free)(JUNO_STRING_T *ptString);

};

const JUNO_STRING_API_T* Juno_StringApi(void);

#ifdef __cplusplus
}
#endif
#endif
