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

#ifndef JUNO_TABLE_TYPES_H
#define JUNO_TABLE_TYPES_H
#include "juno/macros.h"
#include "juno/memory/memory_types.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct JUNO_TABLE_TAG JUNO_TABLE_MANAGER_T;
typedef struct JUNO_TABLE_HDR_TAG JUNO_TABLE_HDR_T;
typedef struct JUNO_TABLE_PRIVATE_TAG JUNO_TABLE_PRIVATE_T;

struct JUNO_TABLE_HDR_TAG
{
    /// CRC32 checksum
    uint32_t iCrc32;
};

struct JUNO_TABLE_TAG
{
    /// The path to the table
    const char *pcTablePath;
    /// The memory buffer for the table
    JUNO_TABLE_HDR_T *ptBuff;
    /// The memory buffer size
    size_t zBuffSize;
    /// The failure handler
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler;
    JUNO_USER_DATA_T *pvFailureUserData;
};

#ifdef __cplusplus
}
#endif
#endif
