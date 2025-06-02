/**
    This header contains the juno_io_serial library API
    @author
*/
#ifndef JUNO_IO_SERIAL_API_H
#define JUNO_IO_SERIAL_API_H
#include "juno/status.h"
#include "juno/module.h"
#include "juno/time/time_api.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct JUNO_IO_SERIAL_API_TAG JUNO_IO_SERIAL_API_T;
typedef uint32_t JUNO_IO_SERIAL_BAUDRATE_T;
JUNO_MODULE_DECLARE(JUNO_IO_SERIAL_T);

JUNO_MODULE(JUNO_IO_SERIAL_T,
    JUNO_IO_SERIAL_BAUDRATE_T iBaud;
);

struct JUNO_IO_SERIAL_API_TAG
{
    /// Initializes the module and resources for juno_io_serial
    JUNO_STATUS_T (*Init)(
        JUNO_IO_SERIAL_T *ptJunoIoSerial,
        JUNO_IO_SERIAL_BAUDRATE_T iBaud,
        JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
        JUNO_USER_DATA_T *pvFailureUserData
    );
    /// Read from the serial device until `zBuffSize` is read
    JUNO_STATUS_T (*Read)(JUNO_IO_SERIAL_T *ptJunoIoSerial, char *pcBuff, size_t zBuffSize);
    /// Attempt to read from serial device for `iTimeoutMs`.
    /// Returns `JUNO_STATUS_TIMEOUT_ERROR` buffer could not be filled
    JUNO_STATUS_T (*TryRead)(JUNO_IO_SERIAL_T *ptJunoIoSerial, char *pcBuff, size_t zBuffSize, JUNO_TIME_MILLIS_T iTimeoutMs);
    /// Write a buffer to the serial device
    JUNO_STATUS_T (*Write)(JUNO_IO_SERIAL_T *ptJunoIoSerial, const void *pcBuff, size_t zBuffSize);
    /// Poll the serial device for data for `iTimeoutMs`
    JUNO_STATUS_T (*Poll)(JUNO_IO_SERIAL_T *ptJunoIoSerial, bool *pbHasData, JUNO_TIME_MILLIS_T iTimeoutMs);
    /// Frees resources allocated by juno_io_serial
    JUNO_STATUS_T (*Free)(JUNO_IO_SERIAL_T *ptJunoIoSerial);
};

#ifdef __cplusplus
}
#endif
#endif // JUNO_IO_SERIAL_API_H
