// Include JUNO macros for assertions and error handling
#include "juno/crc/crc.h"
#include "juno/macros.h"
// Include status codes used throughout JUNO
#include "juno/status.h"
// POSIX table API declarations
#include "juno/table/table_posix.h"
// Table data structures and typedefs
#include "juno/table/table_types.h"
#include <errno.h>                // System error numbers
#include <stdint.h>
#include <stdio.h>                // File I/O routines
#include <string.h>               // strerror() and string ops
#include <unistd.h>

// Validate ensures manager and its required fields are non‑NULL
static inline JUNO_STATUS_T Validate(JUNO_TABLE_MANAGER_T *ptTableManager)
{
    // If manager or its path/buffer pointers are invalid, return error
    if(!ptTableManager && ptTableManager->pcTablePath && ptTableManager->ptBuff)
    {
        return JUNO_STATUS_NULLPTR_ERROR;
    }
    // All checks passed
    return JUNO_STATUS_SUCCESS;
}

// Initialize the POSIX table manager with buffer, size, path, and failure handler
JUNO_STATUS_T Juno_TablePosixInit(
    JUNO_TABLE_MANAGER_T *ptTableManager,
    JUNO_TABLE_HDR_T *ptBuff,
    size_t zBuffSize,
    const char *pcTablePath,
    JUNO_FAILURE_HANDLER_T pfcnFailureHdlr,
    JUNO_USER_DATA_T *pvFailureUserData
)
{
    // Default status to null‐pointer error
    JUNO_STATUS_T tStatus = JUNO_STATUS_NULLPTR_ERROR;
    // Check if a table manager was provided
    if(!ptTableManager)
    {
        return tStatus;
    }
    // Store file path, buffer pointer, and buffer size
    ptTableManager->pcTablePath = pcTablePath;
    ptTableManager->ptBuff = ptBuff;
    ptTableManager->zBuffSize = zBuffSize;
    // Register failure callback and user data
    ptTableManager->pfcnFailureHandler = pfcnFailureHdlr;
    ptTableManager->pvUserData = pvFailureUserData;
    // Validate setup before returning
    tStatus = Validate(ptTableManager);
    return tStatus;
}

// Load table data from disk into the provided buffer
JUNO_STATUS_T Juno_TablePosixLoad(JUNO_TABLE_MANAGER_T *ptTableManager)
{
    // Ensure manager is valid
    JUNO_STATUS_T tStatus = Validate(ptTableManager);
    ASSERT_SUCCESS(tStatus, return tStatus);
    if(access(ptTableManager->pcTablePath, F_OK))
    {
        tStatus = JUNO_STATUS_DNE_ERROR;
        const char *pcErrMsg = strerror(errno);
        FAIL(tStatus, ptTableManager->pfcnFailureHandler, ptTableManager->pvUserData, pcErrMsg);
        return tStatus;
    }
    // Open table file for reading in binary mode
    FILE *ptFile = fopen(ptTableManager->pcTablePath, "rb");
    if(!ptFile)
    {
        tStatus = JUNO_STATUS_FILE_ERROR;
        const char *pcErrMsg = strerror(errno);
        FAIL(tStatus, ptTableManager->pfcnFailureHandler, ptTableManager->pvUserData, pcErrMsg);
        return tStatus;
    }
    // Read buffer and handle any I/O or size mismatches
    size_t zNumElms = fread(ptTableManager->ptBuff, ptTableManager->zBuffSize, 1, ptFile);
    int iErrStatus = ferror(ptFile);
    fclose(ptFile);
    if(!zNumElms)
    {
        if(zNumElms == 0 && iErrStatus)
        {
            tStatus = JUNO_STATUS_READ_ERROR;
            const char *pcErrMsg = strerror(errno);
            FAIL(tStatus, ptTableManager->pfcnFailureHandler, ptTableManager->pvUserData, pcErrMsg);
            return tStatus;
        }
        else
        {
            tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
            FAIL(
                tStatus,
                ptTableManager->pfcnFailureHandler,
                ptTableManager->pvUserData,
                "Invalid size for table"
            );
            return tStatus;
        }
    }
    uint32_t iCrc = Juno_CrcZipUpdate(JUNO_ZIP_CRC_INIT, &ptTableManager->ptBuff[1], ptTableManager->zBuffSize - sizeof(JUNO_TABLE_HDR_T));
    if(iCrc != ptTableManager->ptBuff->iCrc32)
    {
        tStatus = JUNO_STATUS_CRC_ERROR;
        FAIL(
            tStatus,
            ptTableManager->pfcnFailureHandler,
            ptTableManager->pvUserData,
            "Invalid CRC"
        );
    }
    return tStatus;
}

// Save the current buffer contents to disk as the table file
JUNO_STATUS_T Juno_TablePosixSave(JUNO_TABLE_MANAGER_T *ptTableManager)
{
    // Ensure manager is valid
    JUNO_STATUS_T tStatus = Validate(ptTableManager);
    ASSERT_SUCCESS(tStatus, return tStatus);
    // Calculate the CRC
    ptTableManager->ptBuff->iCrc32 = Juno_CrcZipUpdate(JUNO_ZIP_CRC_INIT, &ptTableManager->ptBuff[1], ptTableManager->zBuffSize - sizeof(JUNO_TABLE_HDR_T));
    // Open table file for writing in binary mode
    FILE *ptFile = fopen(ptTableManager->pcTablePath, "wb+");
    if(!ptFile)
    {
        tStatus = JUNO_STATUS_FILE_ERROR;
        const char *pcErrMsg = strerror(errno);
        FAIL(tStatus, ptTableManager->pfcnFailureHandler, ptTableManager->pvUserData, pcErrMsg);
        return tStatus;
    }
    // Write buffer and handle write errors or size mismatches
    size_t zNumElms = fwrite(ptTableManager->ptBuff, ptTableManager->zBuffSize, 1, ptFile);
    int iErrStatus = ferror(ptFile);
    fclose(ptFile);
    if(!zNumElms)
    {
        if(zNumElms == 0 && iErrStatus)
        {
            tStatus = JUNO_STATUS_WRITE_ERROR;
            const char *pcErrMsg = strerror(errno);
            FAIL(tStatus, ptTableManager->pfcnFailureHandler, ptTableManager->pvUserData, pcErrMsg);
            return tStatus;
        }
        else
        {
            tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
            FAIL(
                tStatus,
                ptTableManager->pfcnFailureHandler,
                ptTableManager->pvUserData,
                "Invalid size for table"
            );
            return tStatus;
        }
    }
    return tStatus;

}

// Update the table manager’s buffer pointer and its size only
JUNO_STATUS_T Juno_TablePosixSetBuffer(JUNO_TABLE_MANAGER_T *ptTableManager, JUNO_TABLE_HDR_T *ptBuff)
{
    // Ensure manager is valid before updating
    JUNO_STATUS_T tStatus = Validate(ptTableManager);
    ASSERT_SUCCESS(tStatus, return tStatus);
    // Assign new buffer and size
    ptTableManager->ptBuff = ptBuff;
    return tStatus;
}

// Define static API struct mapping to the above functions
static const JUNO_TABLE_API_T tTableApi = {
    .Init = Juno_TablePosixInit,
    .Load = Juno_TablePosixLoad,
    .Save = Juno_TablePosixSave,
    .SetBuffer = Juno_TablePosixSetBuffer
};

// Expose the POSIX table API to external callers
const JUNO_TABLE_API_T * Juno_TablePosixApi(void)
{
    return &tTableApi;
}

