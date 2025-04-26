#include "juno/crc/crc.h"
#include <stdint.h>
#include <stdio.h>
#include "binhex.h"

uint16_t Juno_CrcBinhexUpdate(uint16_t iCrc, const void *pcData, size_t zDataSize)
{
    register uint16_t crc = iCrc;
    register const uint8_t *cp = pcData;
    register size_t cnt = zDataSize;

    while(cnt--) {
	crc=((crc<<8)&M2_16)^binhex_crctab[((crc>>8)&0xff)^*cp++];
    }

    return(crc);
}
