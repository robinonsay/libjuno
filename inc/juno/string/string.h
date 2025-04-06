#ifndef JUNO_STRING_H
#define JUNO_STRING_H
#include "juno/memory/memory_types.h"
#include "juno/status.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "juno/string/string_types.h"

/**
 * @brief Initializes a JUNO string.
 *
 * @param ptString Pointer to the JUNO string structure.
 * @param ptAlloc Memory allocator.
 * @param pfcnFailureHandler Failure handler callback.
 * @param pvUserData User data pointer.
 * @return JUNO_STATUS_T result status.
 */
JUNO_STATUS_T Juno_StringInit(
    JUNO_STRING_T *ptString,
    JUNO_MEMORY_ALLOC_T *ptAlloc,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvUserData
);

/**
 * @brief Creates a JUNO string from a C string.
 *
 * @param ptString Pointer to the JUNO string structure.
 * @param ptAlloc Memory allocator.
 * @param pcStr C-string input.
 * @param zLen Length of the input string.
 * @param pfcnFailureHandler Failure handler callback.
 * @param pvUserData User data pointer.
 * @return JUNO_STATUS_T result status.
 */
JUNO_STATUS_T Juno_StringFromCStr(
    JUNO_STRING_T *ptString,
    JUNO_MEMORY_ALLOC_T *ptAlloc,
    const char *pcStr,
    size_t zLen,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvUserData
);

/**
 * @brief Sets a new memory allocator for a JUNO string.
 *
 * @param ptString Pointer to the JUNO string structure.
 * @param ptAlloc New memory allocator.
 * @return JUNO_STATUS_T result status.
 */
JUNO_STATUS_T Juno_StringSetAlloc(JUNO_STRING_T *ptString, JUNO_MEMORY_ALLOC_T *ptAlloc);

/**
 * @brief Retrieves the size of the JUNO string.
 *
 * @param ptString Pointer to the JUNO string structure.
 * @param pzRetSize Pointer to variable to store the string size.
 * @return JUNO_STATUS_T result status.
 */
JUNO_STATUS_T Juno_StringGetSize(JUNO_STRING_T *ptString, size_t *pzRetSize);

/**
 * @brief Concatenates two JUNO strings.
 *
 * @param ptString1 Pointer to the first string.
 * @param ptString2 Pointer to the second string.
 * @return JUNO_STATUS_T result status.
 */
JUNO_STATUS_T Juno_StringConcat(JUNO_STRING_T *ptString1, JUNO_STRING_T *ptString2);

/**
 * @brief Frees resources associated with the JUNO string.
 *
 * @param ptString Pointer to the JUNO string structure.
 * @return JUNO_STATUS_T result status.
 */
JUNO_STATUS_T Juno_StringFree(JUNO_STRING_T *ptString);

#ifdef __cplusplus
}
#endif
#endif
