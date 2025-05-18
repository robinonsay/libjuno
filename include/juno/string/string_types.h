#ifndef JUNO_STRING_TYPES_H
#define JUNO_STRING_TYPES_H
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
    const char *pcCStr;
    size_t zSize;
};

#ifdef __cplusplus
}
#endif
#endif
