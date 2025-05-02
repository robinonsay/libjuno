#include "juno/macros.h"
#include "juno/memory/memory.h"
#include "juno/memory/memory_types.h"
#include "juno/string/string_api.h"
#include "juno/status.h"
#include "juno/string/string.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static inline JUNO_STATUS_T Juno_StringValidate(JUNO_STRING_T *ptString)
{
    // Validate that the string pointer and its allocator are not NULL.
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    if(!(ptString && ptString->ptAlloc))
    {
        // Invalid input detected.
        tStatus = JUNO_STATUS_NULLPTR_ERROR;
    }
    return tStatus;
}

JUNO_STATUS_T Juno_StringInit(
    JUNO_STRING_T *ptString,
    JUNO_MEMORY_ALLOC_T *ptAlloc,
    const char *pcStr,
    size_t zLen,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvUserData
)
{
    // Verify input pointers.
    ASSERT_EXISTS((ptString && pcStr));
    // Initialize the string structure.
    ptString->ptAlloc = ptAlloc;
    ptString->pfcnFailureHandler = pfcnFailureHandler;
    ptString->pvUserData = pvUserData;
    ptString->tMemory.zSize = zLen;
    ptString->zLen = zLen;
    // Request memory allocation for the string.
    JUNO_STATUS_T tStatus = Juno_MemoryGet(ptString->ptAlloc, &ptString->tMemory, zLen);
    ASSERT_SUCCESS(tStatus, return tStatus);
    // Copy the C-string into the allocated memory.
    memcpy(ptString->tMemory.pvAddr, pcStr, zLen);
    return tStatus;
}

JUNO_STATUS_T Juno_StringSetAlloc(JUNO_STRING_T *ptString, JUNO_MEMORY_ALLOC_T *ptAlloc)
{
    // Ensure the string pointer is valid.
    ASSERT_EXISTS(ptString);
    JUNO_STATUS_T tStatus = Juno_StringValidate(ptString);
    ASSERT_SUCCESS(tStatus, return tStatus);
    // Set the new memory allocator.
    ptString->ptAlloc = ptAlloc;
    return tStatus;
}

JUNO_STATUS_T Juno_StringLength(JUNO_STRING_T *ptString, size_t *pzRetSize)
{
    // Verify the string pointer.
    ASSERT_EXISTS(ptString);
    JUNO_STATUS_T tStatus = Juno_StringValidate(ptString);
    ASSERT_SUCCESS(tStatus, return tStatus);
    // Retrieve the size from the memory structure.
    *pzRetSize = ptString->zLen;
    return tStatus;
}

JUNO_STATUS_T Juno_StringAppend(JUNO_STRING_T *ptString, const char *pcCstr, size_t zLen)
{
    // Verify the string pointer.
    ASSERT_EXISTS((ptString && pcCstr));
    JUNO_STATUS_T tStatus = Juno_StringValidate(ptString);
    ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = Juno_MemoryUpdate(ptString->ptAlloc, &ptString->tMemory, zLen + ptString->zLen);
    ASSERT_SUCCESS(tStatus, return tStatus);
    char *pcEndStr = ptString->tMemory.pvAddr;
    pcEndStr = &pcEndStr[ptString->zLen];
    memcpy(pcEndStr, pcCstr, zLen);
    return tStatus;   
}

JUNO_STATUS_T Juno_StringConcat(JUNO_STRING_T *ptString1, JUNO_STRING_T *ptString2)
{
    // Ensure both string pointers are valid.
    ASSERT_EXISTS((ptString1 && ptString2));
    JUNO_STATUS_T tStatus = Juno_StringValidate(ptString1);
    ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = Juno_StringValidate(ptString2);
    ASSERT_SUCCESS(tStatus, return tStatus);
    // Calculate size for the concatenated result.
    size_t zNewSize = ptString1->zLen + ptString2->zLen;
    // Request new memory based on the new size.
    tStatus = Juno_MemoryUpdate(ptString1->ptAlloc, &ptString1->tMemory, zNewSize);
    ASSERT_SUCCESS(tStatus, return tStatus);
    char *pcEndStr = ptString1->tMemory.pvAddr;
    pcEndStr = &pcEndStr[ptString1->zLen];
    // Append the second string after the first.
    memcpy(pcEndStr, ptString2->tMemory.pvAddr, ptString2->zLen);
    return tStatus;
}

JUNO_STATUS_T Juno_StringFree(JUNO_STRING_T *ptString)
{
    // Verify the input string.
    ASSERT_EXISTS(ptString);
    JUNO_STATUS_T tStatus = Juno_StringValidate(ptString);
    ASSERT_SUCCESS(tStatus, return tStatus);
    // Free the memory allocated for the string.
    tStatus = Juno_MemoryPut(ptString->ptAlloc, &ptString->tMemory);
    return tStatus;
}

// API structure and accessor remain unchanged.
static const JUNO_STRING_API_T tJuno_StringApi =
{
    .Init = Juno_StringInit,
    .SetAlloc = Juno_StringSetAlloc,
    .Length = Juno_StringLength,
    .Concat = Juno_StringConcat,
    .Append = Juno_StringAppend,
    .Free = Juno_StringFree
};

const JUNO_STRING_API_T* Juno_StringApi(void)
{
    // Return the API structure for external access.
    return &tJuno_StringApi;
}
