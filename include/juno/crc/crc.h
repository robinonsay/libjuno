#ifndef JUNO_CRC_H
#define JUNO_CRC_H
#include <stdio.h>
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
#define M1_16 0xff
#define M2_16 0xff00
#define M1_32 0xffffffff
#define M2_32 0xffffff00

#define JUNO_ARC_CRC_INIT (0)
uint16_t Juno_CrcArcUpdate(uint16_t iCrc, const void *pcData, size_t zDataSize);


#define JUNO_BINHEX_CRC_INIT (0)
uint16_t Juno_CrcBinhexUpdate(uint16_t iCrc, const void *pcData, size_t zDataSize);


#define JUNO_CCITT_CRC_INIT (-1)
uint16_t Juno_CrcCcittUpdate(uint16_t iCrc, const void *pcData, size_t zDataSize);


#define JUNO_CCITT32_CRC_INIT (-1)
uint32_t Juno_CrcCcitt32Update(uint32_t iCrc, const void *pcData, size_t zDataSize);


#define JUNO_KERMIT_CRC_INIT (0)
uint32_t Juno_CrcKermitUpdate(uint32_t iCrc, const void *pcData, size_t zDataSize);

#define JUNO_ZIP_CRC_INIT (-1)
uint32_t Juno_CrcZipUpdate(uint32_t iCrc, const void *pcData, size_t zDataSize);

#ifdef __cplusplus
}
#endif
#endif
