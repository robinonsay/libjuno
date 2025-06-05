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

#ifndef JUNO_MAP_TYPES_H
#define JUNO_MAP_TYPES_H

#include "juno/hash/hash_api.h"
#include "juno/macros.h"
#include "juno/memory/memory_types.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JUNO_MAP_TAG JUNO_MAP_T;
typedef bool (*JUNO_MAP_KEY_EQUAL_FCN_T)(JUNO_MEMORY_T ptKey1, JUNO_MEMORY_T ptKey2);

struct JUNO_MAP_TAG
{
    const JUNO_HASH_API_T *ptHashApi;
    JUNO_MEMORY_T *ptMapKeys;
    JUNO_MEMORY_T *ptMapValues;
    size_t zCapacity;
    size_t zLenHashTable;
    JUNO_MAP_KEY_EQUAL_FCN_T pfcnIsEqual;
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler;            ///< Macro to declare a failure handler.
    JUNO_USER_DATA_T *pvFailureUserData;
};

#ifdef __cplusplus
}
#endif
#endif

