#include "juno/crc/crc.h"
#include <stdio.h>
#include "zip.h"

uint32_t Juno_CrcZipUpdate(uint32_t iCrc, const void *pcData, size_t zDataSize)
{
    register uint32_t crc = iCrc;
    register const uint8_t *cp = pcData;
    register size_t cnt = zDataSize;

    while(cnt--) {
	crc=((crc>>8)&M1_32)^zip_crctab[(crc&0xff)^*cp++];
    }

    return(crc ^ 0xFFFFFFFF);
}
