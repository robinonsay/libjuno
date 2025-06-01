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
typedef struct JUNO_TIMESTAMP_TAG JUNO_TIMESTAMP_T;

typedef uint64_t JUNO_TIME_SECONDS_T;
typedef uint32_t JUNO_TIME_MILLIS_T;
typedef uint32_t JUNO_TIME_MICROS_T;
typedef uint32_t JUNO_TIME_NANOS_T;

JUNO_MODULE(JUNO_TIME_T, JUNO_MODULE_EMPTY);

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

