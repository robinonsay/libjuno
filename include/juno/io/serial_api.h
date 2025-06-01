/**
    This header contains the juno_serial library API
    @author
*/
#ifndef JUNO_SERIAL_API_H
#define JUNO_SERIAL_API_H
#include "juno/status.h"
#include "juno/time/time_api.h"
#include <stddef.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct JUNO_SERIAL_TAG JUNO_SERIAL_T;

JUNO_STATUS_T JunoSerial_Read(JUNO_SERIAL_T *ptSerial, char *pcBuff, size_t zBuffSize);
JUNO_STATUS_T JunoSerial_TryRead(JUNO_SERIAL_T *ptSerial, char *pcBuff, size_t zBuffSize, JUNO_TIME_MICROS_T iTimeoutUs);
JUNO_STATUS_T JunoSerial_Write(JUNO_SERIAL_T *ptSerial, const void *pcBuff, size_t zBuffSize);
JUNO_STATUS_T JunoSerial_Poll(JUNO_SERIAL_T *ptSerial, bool *pbRetHasData, JUNO_TIME_MICROS_T iTimeoutUs);

#ifdef __cplusplus
}
#endif
#endif // JUNO_SERIAL_API_H
