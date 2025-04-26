#include "juno/crc/crc.h"
#include "ccitt32.h"
#include <stdio.h>

uint32_t Juno_Ccitt32UpdateCrc(uint32_t iCrc, void *pcData, size_t zDataSize)
{

    register uint32_t crc = iCrc;
    register unsigned char *cp = pcData;
    register size_t cnt = zDataSize;

    while(cnt--) {
	crc=((crc<<8)&M2_32)^ccitt32_crctab[((crc>>24)&0xff)^*cp++];
    }

    return(crc);
}
