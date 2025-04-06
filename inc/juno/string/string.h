#ifndef JUNO_STRING_H
#define JUNO_STRING_H
#include "juno/memory/memory_types.h"
#include "juno/status.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "juno/string/string_types.h"

JUNO_STATUS_T Juno_StringInit(
    JUNO_STRING_T *ptString,
    JUNO_MEMORY_ALLOC_T *ptAlloc,
    size_t zLen,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvUserData
);
JUNO_STATUS_T Juno_StringFromCStr(
    JUNO_STRING_T *ptString,
    JUNO_MEMORY_ALLOC_T *ptAlloc,
    const char *pcStr,
    size_t zLen,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvUserData
);
JUNO_STATUS_T Juno_StringSetAlloc(JUNO_STRING_T *ptString, JUNO_MEMORY_ALLOC_T *ptAlloc);
JUNO_STATUS_T Juno_StringGetSize(JUNO_STRING_T *ptString, size_t *pzRetSize);
/// Concat two strings together
/// @param ptString1 The first string
/// @param ptString2 The second string
/// @param zNewSize The size of the new string. If the size is 0, the new size will be inferred
JUNO_STATUS_T Juno_StringConcat(JUNO_STRING_T *ptString1, JUNO_STRING_T *ptString2, size_t zNewSize);
JUNO_STATUS_T Juno_StringFree(JUNO_STRING_T *ptString);
#ifdef __cplusplus
}
#endif
#endif
