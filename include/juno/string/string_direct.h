/*
    MIT License

    Copyright (c) 2025 Robin A. Onsay

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.
*/

/**
    This API has been generated by LibJuno:
    https://www.robinonsay.com/libjuno/
*/

/**
    This header contains the juno_string library API
    @author
*/
#ifndef JUNO_STRING_DIRECT_H
#define JUNO_STRING_DIRECT_H
#include "juno/memory/memory_api.h"
#include "juno/status.h"
#include "juno/string/string_api.h"
#ifdef __cplusplus
extern "C"
{
#endif


/// Initializes the module and resources for juno_string
JUNO_STATUS_T JunoString_Init(JUNO_STRING_T *ptJunoString, JUNO_MEMORY_ALLOC_T *ptAlloc, const char *pcCStr, size_t zCStrSize);
JUNO_STATUS_T JunoString_Append(JUNO_STRING_T *ptJunoString, JUNO_STRING_T *ptNewJunoString);
JUNO_STATUS_T JunoString_AppendCStr(JUNO_STRING_T *ptJunoString, const char *pcCStr, size_t zCStrSize);
JUNO_STATUS_T JunoString_GetSize(JUNO_STRING_T *ptJunoString, size_t *pzSize);
/// Frees resources allocated by juno_string
JUNO_STATUS_T JunoString_Free(JUNO_STRING_T *ptJunoString);

#ifdef __cplusplus
}
#endif
#endif // JUNO_STRING_DIRECT_H

