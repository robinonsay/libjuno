#ifndef JUNO_TABLE_TYPES_H
#define JUNO_TABLE_TYPES_H
#include "juno/macros.h"
#include "juno/memory/memory_types.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct JUNO_TABLE_TAG JUNO_TABLE_T;
typedef struct JUNO_TABLE_HDR_TAG JUNO_TABLE_HDR_T;

struct JUNO_TABLE_HDR_TAG
{
    /// The size of the table
    size_t zTableSize;
    /// The number of elements;
    size_t zLen;
    /// CRC32 checksum
    uint32_t iCrc32;
};

struct JUNO_TABLE_TAG
{
    /// The path to the table
    const char *pcTablePath;
    /// The memory buffer for the table
    JUNO_TABLE_HDR_T *ptBuff;
    DECLARE_FAILURE_HANDLER;
};

#ifdef __cplusplus
}
#endif
#endif
