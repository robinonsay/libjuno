#include "juno/macros.h"
#include "juno/memory/memory.h"
#include "juno/memory/memory_types.h"
#include "juno/status.h"
#include "juno/string/string.h"
#include "juno/string/string_api.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static JUNO_STATUS_T Juno_StringValidate(JUNO_STRING_T *ptString)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    if(!(ptString && ptString->ptAlloc))
    {
        tStatus = JUNO_STATUS_NULLPTR_ERROR;
    }
    return tStatus;
}

JUNO_STATUS_T Juno_StringInit(
    JUNO_STRING_T *ptString,
    JUNO_MEMORY_ALLOC_T *ptAlloc,
    size_t zLen,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvUserData
)
{
    if(!(ptString && ptAlloc))
    {
        return JUNO_STATUS_NULLPTR_ERROR;
    }
    ptString->ptAlloc = ptAlloc;
    ptString->pfcnFailureHandler = pfcnFailureHandler;
    ptString->pvUserData = pvUserData;
    ptString->tMemory.zSize = zLen;
    return Juno_MemoryGet(ptString->ptAlloc, &ptString->tMemory);
}

JUNO_STATUS_T Juno_StringFromCStr(
    JUNO_STRING_T *ptString,
    JUNO_MEMORY_ALLOC_T *ptAlloc,
    const char *pcStr,
    size_t zLen,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvUserData
)
{
    if(!(ptString && ptAlloc))
    {
        return JUNO_STATUS_NULLPTR_ERROR;
    }
    ptString->ptAlloc = ptAlloc;
    ptString->pfcnFailureHandler = pfcnFailureHandler;
    ptString->pvUserData = pvUserData;
    ptString->tMemory.zSize = zLen;
    JUNO_STATUS_T tStatus = Juno_MemoryGet(ptString->ptAlloc, &ptString->tMemory);
    ASSERT_SUCCESS(tStatus, return tStatus);
    memcpy(ptString->tMemory.pvAddr, pcStr, zLen);
    return tStatus;

}

JUNO_STATUS_T Juno_StringSetAlloc(JUNO_STRING_T *ptString, JUNO_MEMORY_ALLOC_T *ptAlloc)
{
    ASSERT_EXISTS(ptString);
    JUNO_STATUS_T tStatus = Juno_StringValidate(ptString);
    ASSERT_SUCCESS(tStatus, return tStatus);
    ptString->ptAlloc = ptAlloc;
    return tStatus;
}

JUNO_STATUS_T Juno_StringGetSize(JUNO_STRING_T *ptString, size_t *pzRetSize)
{
    ASSERT_EXISTS(ptString);
    JUNO_STATUS_T tStatus = Juno_StringValidate(ptString);
    ASSERT_SUCCESS(tStatus, return tStatus);
    *pzRetSize = ptString->tMemory.zSize;
    return tStatus;
}

JUNO_STATUS_T Juno_StringConcat(JUNO_STRING_T *ptString1, JUNO_STRING_T *ptString2, size_t zNewSize)
{
    ASSERT_EXISTS((ptString1 && ptString2));
    JUNO_STATUS_T tStatus = Juno_StringValidate(ptString1);
    ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = Juno_StringValidate(ptString2);
    ASSERT_SUCCESS(tStatus, return tStatus);
    if(zNewSize > 0 && zNewSize < ptString1->tMemory.zSize + ptString2->tMemory.zSize)
    {
        tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        FAIL(tStatus, ptString1->pfcnFailureHandler, ptString1->pvUserData,
        "Failed to concat strings. New size is too small");
        return tStatus;
    }
    if(!zNewSize)
    {
        zNewSize = ptString1->tMemory.zSize + ptString2->tMemory.zSize;
    }
    JUNO_MEMORY_T tNewMemory = {0};
    tNewMemory.zSize = zNewSize;
    tStatus = Juno_MemoryGet(ptString1->ptAlloc, &tNewMemory);
    ASSERT_SUCCESS(tStatus, return tStatus);
    uint8_t *ptNewStr = tNewMemory.pvAddr;
    memcpy(ptNewStr, ptString1->tMemory.pvAddr, ptString1->tMemory.zSize);
    ptNewStr = &ptNewStr[ptString1->tMemory.zSize];
    memcpy(ptNewStr, ptString2->tMemory.pvAddr, ptString2->tMemory.zSize);
    tStatus = Juno_MemoryPut(ptString1->ptAlloc, &ptString1->tMemory);
    ASSERT_SUCCESS(tStatus, {
        Juno_MemoryPut(ptString1->ptAlloc, &tNewMemory);
        return tStatus;
    });
    ptString1->tMemory = tNewMemory;
    return tStatus;
}


JUNO_STATUS_T Juno_StringFree(JUNO_STRING_T *ptString)
{
    ASSERT_EXISTS(ptString);
    JUNO_STATUS_T tStatus = Juno_StringValidate(ptString);
    ASSERT_SUCCESS(tStatus, return tStatus);
    tStatus = Juno_MemoryPut(ptString->ptAlloc, &ptString->tMemory);
    return tStatus;
}

static const JUNO_STRING_API_T tJuno_StringApi =
{
    .Init = Juno_StringInit,
    .FromCStr = Juno_StringFromCStr,
    .SetAlloc = Juno_StringSetAlloc,
    .GetSize = Juno_StringGetSize,
    .Concat = Juno_StringConcat,
    .Free = Juno_StringFree
};

const JUNO_STRING_API_T* Juno_StringApi(void)
{
    return &tJuno_StringApi;
}

