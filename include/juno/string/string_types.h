#ifndef JUNO_STRING_TYPES_H
#define JUNO_STRING_TYPES_H
#include "juno/macros.h"
#include "juno/memory/memory_types.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif


/// @brief Represents a JUNO string.
/// This structure encapsulates the memory allocation, underlying memory storage,
/// and associated failure handling for a string.
typedef struct JUNO_STRING_TAG JUNO_STRING_T;

struct JUNO_STRING_TAG
{
    /// The allocator for the string
    JUNO_MEMORY_ALLOC_T *ptAlloc;
    /// The memory location of the string
    JUNO_MEMORY_T tMemory;
    /// The length of the string
    size_t zLen;
    /// Failure handler
    DECLARE_FAILURE_HANDLER;
};

#ifdef __cplusplus
}
#endif
#endif
