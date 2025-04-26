#ifndef JUNO_TABLE_API_H
#define JUNO_TABLE_API_H
#include "juno/memory/memory_types.h"
#include "juno/table/table_types.h"
#include "juno/status.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct JUNO_TABLE_API_TAG JUNO_TABLE_API_T;

struct JUNO_TABLE_API_TAG
{
    /// Initialize a Juno Table with a buffer
    /// @param ptTable The table manager to initialize
    /// @param ptMemoryApi The memory api
    /// @param ptBuff The table buffer
    /// @param zBuffSize The table buffer size
    /// @param ptAlloc The memory allocator
    /// @param pfcnFailureHdlr The failure handler (can be null)
    /// @param pvFailureUserData The failure handler user data (can be null)
    /// @return Returns a `JUNO_STATUS_T` code
    JUNO_STATUS_T (*Init)(
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
    JUNO_STATUS_T (*Load)(JUNO_TABLE_MANAGER_T *ptTableManager);
    /// Save the table
    /// @param ptTableManager The Juno table manager to load
    /// @return Returns a `JUNO_STATUS_T` code
    JUNO_STATUS_T (*Save)(JUNO_TABLE_MANAGER_T *ptTableManager);
    /// Set the table manager to a new table buffer
    /// @param ptTableManager The Juno table manager to load
    /// @param ptBuff The new table buffer
    /// @param zBuffSize The table buffer size
    /// @return Returns a `JUNO_STATUS_T` code
    JUNO_STATUS_T (*SetBuffer)(JUNO_TABLE_MANAGER_T *ptTableManager, JUNO_TABLE_HDR_T *ptBuff, size_t zBuffSize);
};

#ifdef __cplusplus
}
#endif
#endif
