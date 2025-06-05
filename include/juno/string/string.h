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
