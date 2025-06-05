/*
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
#ifndef HASH_API_H
#define HASH_API_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "juno/status.h"

typedef struct JUNO_HASH_API_TAG JUNO_HASH_API_T;

struct JUNO_HASH_API_TAG
{

    JUNO_STATUS_T (*Hash)(const uint8_t *pcBuff, size_t zBuffSize, size_t *pzRetHash);
};


#ifdef __cplusplus
}
#endif
#endif
