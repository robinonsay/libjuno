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
    size_t zSize;
};

#ifdef __cplusplus
}
#endif
#endif
