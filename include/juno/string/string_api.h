#ifndef JUNO_STRING_API_H
#define JUNO_STRING_API_H
#include "juno/memory/memory_types.h"
#include "juno/status.h"
#include "juno/string/string_types.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>

typedef struct JUNO_STRING_API_TAG JUNO_STRING_API_T;

struct JUNO_STRING_API_TAG
{
    JUNO_STATUS_T (*Append)(JUNO_STRING_T *ptString, JUNO_STRING_T tString);
    JUNO_STATUS_T (*Find)(JUNO_STRING_T tString, JUNO_STRING_T tSearchString, JUNO_STRING_T *ptResult);
    JUNO_STATUS_T (*Split)(JUNO_STRING_T *ptString, JUNO_STRING_T tDelim, JUNO_STRING_T *ptArrStrs, size_t zArrLen);
};

#ifdef __cplusplus
}
#endif
#endif
