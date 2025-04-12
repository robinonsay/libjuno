#ifndef JUNO_MAP_H
#define JUNO_MAP_H

#include "juno/map/map_types.h"
#include "juno/status.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Initialize a Juno Map
/// @param ptMap A pointer to the map to initalize
/// @param ptKeyTable The allocated memory for the keys
/// @param ptValueTable The allocated memory for the values
/// @param zCapacity The capacity of the map table
/// @param pfcnIsEqual A function to determine if two keys are equal
/// @param pfcnFailureHandler The failure handler
/// @param pvFailureUserData User data to provide to the failure handler
/// @return Returns a JUNO_STATUS_SUCCESS on success, and error otherwise
JUNO_STATUS_T Juno_MapInit(
    JUNO_MAP_T *ptMap,
    JUNO_MAP_KEY_T *ptKeyTable,
    JUNO_MAP_VALUE_T *ptValueTable,
    size_t zCapacity,
    JUNO_MAP_KEY_EQUAL_FCN_T pfcnIsEqual,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
);

/// Add a key,value pair to the map
/// @param ptMap A pointer to the map
/// @param ptKey A key to add
/// @param pvValue A value to add
/// @return Status of operation
JUNO_STATUS_T Juno_MapAdd(JUNO_MAP_T *ptMap, JUNO_MAP_KEY_T tKey, JUNO_MAP_VALUE_T tValue);

/// Remove a key,value pair from the map
/// @param ptMap A pointer to the map
/// @param tKey The key to remove
/// @return Status of operation
JUNO_STATUS_T Juno_MapRemove(JUNO_MAP_T *ptMap, JUNO_MAP_KEY_T tKey);

/// Get a value from the map using the key
/// @param ptMap A pointer to the map
/// @param tKey The key to use
/// @param ptRetVal The return value retrieved using the key
/// @return Status of operation
JUNO_STATUS_T Juno_MapGet(JUNO_MAP_T *ptMap, JUNO_MAP_KEY_T tKey, JUNO_MAP_VALUE_T *ptRetValue);

#ifdef __cplusplus
}
#endif
#endif
