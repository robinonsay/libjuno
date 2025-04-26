#ifndef JUNO_TABLE_H
#define JUNO_TABLE_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "juno/table/table_api.h"

/// Initialize a Juno Table with a buffer
/// @param ptTable The table manager to initialize
/// @param ptMemoryApi The memory api
/// @param ptBuff The table buffer
/// @param zBuffSize The table buffer size
/// @param ptAlloc The memory allocator
/// @param pfcnFailureHdlr The failure handler (can be null)
/// @param pvFailureUserData The failure handler user data (can be null)
/// @return Returns a `JUNO_STATUS_T` code
JUNO_STATUS_T Juno_TablePosixInit(
    JUNO_TABLE_MANAGER_T *ptTableManager,
    JUNO_TABLE_HDR_T *ptBuff,
    size_t zBuffSize,
    const char *pcTablePath,
    JUNO_FAILURE_HANDLER_T pfcnFailureHdlr,
    JUNO_USER_DATA_T *pvFailureUserData
);
/// Load the table
/// @param ptTableManager The Juno table manager to load
/// @return Returns a `JUNO_STATUS_T` code
JUNO_STATUS_T Juno_TablePosixLoad(JUNO_TABLE_MANAGER_T *ptTableManager);
/// Save the table
/// @param ptTableManager The Juno table manager to load
/// @return Returns a `JUNO_STATUS_T` code
JUNO_STATUS_T Juno_TablePosixSave(JUNO_TABLE_MANAGER_T *ptTableManager);
/// Set the table manager to a new table buffer
/// @param ptTableManager The Juno table manager to load
/// @param ptBuff The new table buffer
/// @param zBuffSize The table buffer size
/// @return Returns a `JUNO_STATUS_T` code
JUNO_STATUS_T Juno_TablePosixSetBuffer(JUNO_TABLE_MANAGER_T *ptTableManager, JUNO_TABLE_HDR_T *ptBuff, size_t zBuffSize);

/// Get the POSIX `JUNO_TABLE_API_T`
/// @return Return the `JUNO_TABLE_API_T`
const JUNO_TABLE_API_T * Juno_TablePosixApi(void);

#ifdef __cplusplus
}
#endif
#endif
