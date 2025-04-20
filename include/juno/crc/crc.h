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

extern unsigned long arc_crcinit;
unsigned long Juno_ArcUpdateCrc(unsigned long icrc, unsigned char *icp, size_t icnt);


extern unsigned long binhex_crcinit;
unsigned long Juno_BinhexUpdateCrc(unsigned long icrc, unsigned char *icp, size_t icnt);


extern unsigned long ccitt_crcinit;
unsigned long Juno_CcittUpdateCrc(unsigned long icrc, unsigned char *icp, size_t icnt);


extern unsigned long ccitt32_crcinit;
unsigned long Juno_Ccitt32UpdateCrc(unsigned long icrc, unsigned char *icp, size_t icnt);


extern unsigned long kermit_crcinit;
unsigned long Juno_KermitUpdateCrc(unsigned long icrc, unsigned char *icp, size_t icnt);

extern unsigned long zip_crcinit;
unsigned long Juno_ZipUpdateCrc(unsigned long icrc, unsigned char *icp, size_t icnt);

#ifdef __cplusplus
}
#endif
#endif
