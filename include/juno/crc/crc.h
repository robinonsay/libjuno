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
#define M1_32 0xffffff
#define M2_32 0xffffff00

extern const unsigned long giJUNO_ARC_CRC_INIT;
unsigned long Juno_ArcUpdateCrc(unsigned long iCrc, unsigned char *pcData, size_t zDataSize);


extern const unsigned long giJUNO_BINHEX_CRC_INIT;
unsigned long Juno_BinhexUpdateCrc(unsigned long iCrc, unsigned char *pcData, size_t zDataSize);


extern const unsigned long giJUNO_CCITT_CRC_INIT;
unsigned long Juno_CcittUpdateCrc(unsigned long iCrc, unsigned char *pcData, size_t zDataSize);


extern const unsigned long giJUNO_CCITT32_CRC_INIT;
unsigned long Juno_Ccitt32UpdateCrc(unsigned long iCrc, unsigned char *pcData, size_t zDataSize);


extern const unsigned long giJUNO_KERMIT_CRC_INIT;
unsigned long Juno_KermitUpdateCrc(unsigned long iCrc, unsigned char *pcData, size_t zDataSize);

extern const unsigned long giJUNO_ZIP_CRsC_INIT;
unsigned long Juno_ZipUpdateCrc(unsigned long iCrc, unsigned char *pcData, size_t zDataSize);

#ifdef __cplusplus
}
#endif
#endif
