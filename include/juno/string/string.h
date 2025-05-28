#ifndef JUNO_STRING_H
#define JUNO_STRING_H
#include "juno/string/string_api.h"
#include "juno/string/string_types.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>

JUNO_STATUS_T Juno_StringAppend(JUNO_STRING_T *ptString, JUNO_STRING_T tString);
JUNO_STATUS_T Juno_StringFind(JUNO_STRING_T tString, JUNO_STRING_T tSearchString, JUNO_STRING_T *ptResult);
JUNO_STATUS_T Juno_StringSplit(JUNO_STRING_T *ptString, JUNO_STRING_T tDelim, JUNO_STRING_T *ptArrStrs, size_t zArrLen);

const JUNO_STRING_API_T * Juno_StringApi(void);

#ifdef __cplusplus
}
#endif
#endif
