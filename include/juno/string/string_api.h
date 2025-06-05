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

#ifndef JUNO_STRING_API_H
#define JUNO_STRING_API_H
#include "juno/memory/memory_types.h"
#include "juno/status.h"
#include "juno/string/string_types.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>

typedef struct JUNO_STRING_API_TAG JUNO_STRING_API_T;

struct JUNO_STRING_API_TAG
{
    JUNO_STATUS_T (*Append)(JUNO_STRING_T *ptString, JUNO_STRING_T tString);
    JUNO_STATUS_T (*Find)(JUNO_STRING_T tString, JUNO_STRING_T tSearchString, JUNO_STRING_T *ptResult);
    JUNO_STATUS_T (*Split)(JUNO_STRING_T *ptString, JUNO_STRING_T tDelim, JUNO_STRING_T *ptArrStrs, size_t zArrLen);
};

#ifdef __cplusplus
}
#endif
#endif
