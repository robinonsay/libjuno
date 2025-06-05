/**
    MIT License

    Copyright (c) Year Robin A. Onsay

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

/***/

#ifndef JUNO_STRING_TYPES_H
#define JUNO_STRING_TYPES_H
#include "juno/memory/memory_api.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>

typedef struct JUNO_STRING_TAG JUNO_STRING_T;

struct JUNO_STRING_TAG
{
    const char *pcBuff;
    size_t zLen;
    size_t zCapacity;
};

#ifdef __cplusplus
}
#endif
#endif
