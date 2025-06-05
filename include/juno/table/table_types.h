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
#ifndef JUNO_TABLE_TYPES_H
#define JUNO_TABLE_TYPES_H
#include "juno/macros.h"
#include "juno/memory/memory_types.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct JUNO_TABLE_TAG JUNO_TABLE_MANAGER_T;
typedef struct JUNO_TABLE_HDR_TAG JUNO_TABLE_HDR_T;
typedef struct JUNO_TABLE_PRIVATE_TAG JUNO_TABLE_PRIVATE_T;

struct JUNO_TABLE_HDR_TAG
{
    /// CRC32 checksum
    uint32_t iCrc32;
};

struct JUNO_TABLE_TAG
{
    /// The path to the table
    const char *pcTablePath;
    /// The memory buffer for the table
    JUNO_TABLE_HDR_T *ptBuff;
    /// The memory buffer size
    size_t zBuffSize;
    /// The failure handler
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler;
    JUNO_USER_DATA_T *pvFailureUserData;
};

#ifdef __cplusplus
}
#endif
#endif
