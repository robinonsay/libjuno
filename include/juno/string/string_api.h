#ifndef JUNO_STRING_API_H
#define JUNO_STRING_API_H
#include "juno/status.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "juno/string/string_types.h"

typedef struct JUNO_STRING_API_TAG JUNO_STRING_API_T;

struct JUNO_STRING_API_TAG
{
    /// @brief Creates a JUNO string from a C string.
    /// @param ptString Pointer to the JUNO string structure.
    /// @param ptAlloc Memory allocator.
    /// @param pcStr C-string input.
    /// @param zLen Length of the input string.
    /// @param pfcnFailureHandler Failure handler callback.
    /// @param pvUserData User data pointer.
    /// @return JUNO_STATUS_T result status.
    JUNO_STATUS_T (*Init)(
        JUNO_STRING_T *ptString,
        JUNO_MEMORY_ALLOC_T *ptAlloc,
        const char *pcStr,
        size_t zLen,
        JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
        JUNO_USER_DATA_T *pvUserData
    );
    /// @brief Sets a new memory allocator for a JUNO string.
    /// @param ptString Pointer to the JUNO string structure.
    /// @param ptAlloc New memory allocator.
    /// @return JUNO_STATUS_T result status.
    JUNO_STATUS_T (*SetAlloc)(JUNO_STRING_T *ptString, JUNO_MEMORY_ALLOC_T *ptAlloc);
    /// @brief Retrieves the size of the JUNO string.
    /// @param ptString Pointer to the JUNO string structure.
    /// @param pzRetSize Pointer to variable to store the string size.
    /// @return JUNO_STATUS_T result status.
    JUNO_STATUS_T (*Length)(JUNO_STRING_T *ptString, size_t *pzRetSize);
    /// Concat two strings together
    /// @param ptString1 The first string
    /// @param ptString2 The second string
    /// @param zNewSize The size of the new string. If the size is 0, the new size will be inferred
    JUNO_STATUS_T (*Concat)(JUNO_STRING_T *ptString1, JUNO_STRING_T *ptString2);
    /// Writes a C-String to the Juno String
    /// @param ptString The juno string to write to
    /// @param pcCstr The C string
    /// @param zLen The length of the C string
    /// @return JUNO_STATUS_T result status
    JUNO_STATUS_T (*Append)(JUNO_STRING_T *ptString, const char *pcCstr, size_t zLen);
    /// @brief Frees resources associated with the JUNO string.
    /// @param ptString Pointer to the JUNO string structure.
    /// @return JUNO_STATUS_T result status.
    JUNO_STATUS_T (*Free)(JUNO_STRING_T *ptString);

};


#ifdef __cplusplus
}
#endif
#endif
