#ifndef JUNO_CRC_API_H
#define JUNO_CRC_API_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stddef.h>

typedef struct JUNO_CRC_API_TAG JUNO_CRC_API_T;

struct JUNO_CRC_API_TAG
{
    unsigned long (*Crc)(unsigned long iCrc, unsigned char *pcData, size_t zDataSize);
};

const JUNO_CRC_API_T * Juno_CrcArcApi(void);
const JUNO_CRC_API_T * Juno_CrcBinhexApi(void);
const JUNO_CRC_API_T * Juno_CrcCcittApi(void);
const JUNO_CRC_API_T * Juno_CrcCcitt32Api(void);
const JUNO_CRC_API_T * Juno_CrcKermitApi(void);
const JUNO_CRC_API_T * Juno_CrcZipApi(void);

#ifdef __cplusplus
}
#endif
#endif
