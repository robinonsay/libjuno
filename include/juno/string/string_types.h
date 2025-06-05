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

#ifndef JUNO_STRING_TYPES_H
#define JUNO_STRING_TYPES_H
#include "juno/memory/memory_api.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>

typedef struct JUNO_STRING_TAG JUNO_STRING_T;

struct JUNO_STRING_TAG
{
    const char *pcBuff;
    size_t zLen;
    size_t zCapacity;
};

#ifdef __cplusplus
}
#endif
#endif
