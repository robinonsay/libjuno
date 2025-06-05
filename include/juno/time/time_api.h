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

/**
    This header contains the time library API
    @author
*/
#ifndef JUNO_TIME_API_H
#define JUNO_TIME_API_H
#include "juno/status.h"
#include "juno/module.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct JUNO_TIME_API_TAG JUNO_TIME_API_T;
JUNO_MODULE_DECLARE(JUNO_TIME_T);
JUNO_MODULE_BASE_DECLARE(JUNO_TIME_BASE_T);
typedef struct JUNO_TIMESTAMP_TAG JUNO_TIMESTAMP_T;

typedef uint64_t JUNO_TIME_SECONDS_T;
typedef uint32_t JUNO_TIME_MILLIS_T;
typedef uint32_t JUNO_TIME_MICROS_T;
typedef uint32_t JUNO_TIME_NANOS_T;

JUNO_MODULE_BASE(JUNO_TIME_BASE_T, JUNO_TIME_API_T, JUNO_MODULE_EMPTY);

struct JUNO_TIMESTAMP_TAG
{
    JUNO_TIME_SECONDS_T iSeconds;
    JUNO_TIME_MILLIS_T iMillis;
    JUNO_TIME_MICROS_T iMicros;
    JUNO_TIME_NANOS_T iNanos;
};

struct JUNO_TIME_API_TAG
{
    JUNO_STATUS_T (*GetTime)(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime);
};

#ifdef __cplusplus
}
#endif
#endif // JUNO_TIME_API_H

