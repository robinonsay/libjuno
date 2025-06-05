/**
   Copyright 2025 Robin A. Onsay

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef JUNO_MAP_API_H
#define JUNO_MAP_API_H
#include "juno/memory/memory_types.h"
#include "juno/status.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "juno/map/map_types.h"

typedef struct JUNO_MAP_API_TAG JUNO_MAP_API_T;

struct JUNO_MAP_API_TAG
{
    /// Initialize a Juno Map
    /// @param ptMap A pointer to the map to initialize
    /// @param ptKeyTable The allocated memory for the keys
    /// @param ptValueTable The allocated memory for the values
    /// @param zKeySize The size of the key
    /// @param zCapacity The capacity of the map table
    /// @param pfcnIsEqual A function to determine if two keys are equal
    /// @param pfcnFailureHandler The failure handler
    /// @param pvFailureUserData User data to provide to the failure handler
    /// @return Returns a JUNO_STATUS_SUCCESS on success, and error otherwise
    JUNO_STATUS_T (*Init)(
        JUNO_MAP_T *ptMap,
        const JUNO_HASH_API_T *ptHashApi,
        JUNO_MEMORY_T *ptKeyTable,
        JUNO_MEMORY_T *ptValueTable,
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
    JUNO_STATUS_T (*Set)(JUNO_MAP_T *ptMap, JUNO_MEMORY_T tKey, JUNO_MEMORY_T tValue);
    
    /// Remove a key,value pair from the map
    /// @param ptMap A pointer to the map
    /// @param tKey The key to remove
    /// @return Status of operation
    JUNO_STATUS_T (*Remove)(JUNO_MAP_T *ptMap, JUNO_MEMORY_T tKey);
    
    /// Get a value from the map using the key
    /// @param ptMap A pointer to the map
    /// @param tKey The key to use
    /// @param ptRetVal The return value retrieved using the key
    /// @return Status of operation
    /// Returns `JUNO_STATUS_DNE_ERROR` if the key is not in the map
    JUNO_STATUS_T (*Get)(JUNO_MAP_T *ptMap, JUNO_MEMORY_T tKey, JUNO_MEMORY_T *ptRetValue);
};


#ifdef __cplusplus
}
#endif
#endif


