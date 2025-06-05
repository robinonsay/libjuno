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

#ifndef HASH_API_H
#define HASH_API_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "juno/status.h"

typedef struct JUNO_HASH_API_TAG JUNO_HASH_API_T;

struct JUNO_HASH_API_TAG
{

    JUNO_STATUS_T (*Hash)(const uint8_t *pcBuff, size_t zBuffSize, size_t *pzRetHash);
};


#ifdef __cplusplus
}
#endif
#endif
