#ifndef JUNO_STRING_TYPES_H
#define JUNO_STRING_TYPES_H
#include "juno/macros.h"
#include "juno/memory/memory_types.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct JUNO_STRING_TAG JUNO_STRING_T;

struct JUNO_STRING_TAG
{
    JUNO_MEMORY_ALLOC_T *ptAlloc;
    JUNO_MEMORY_T tMemory;
    DECLARE_FAILURE_HANDLER;
};

#ifdef __cplusplus
}
#endif
#endif
