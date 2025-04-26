#include "juno/crc/crc.h"
#include "ccitt32.h"
#include <stdio.h>

uint32_t Juno_CrcCcitt32Update(uint32_t iCrc, const void *pcData, size_t zDataSize)
{

    register uint32_t crc = iCrc;
    register const uint8_t *cp = pcData;
    register size_t cnt = zDataSize;

    while(cnt--) {
	crc=((crc<<8)&M2_32)^ccitt32_crctab[((crc>>24)&0xff)^*cp++];
    }

    return(crc);
}
