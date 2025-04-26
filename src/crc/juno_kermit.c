#include "juno/crc/crc.h"
#include <stdint.h>
#include <stdio.h>
#include "kermit.h"

uint32_t Juno_KermitUpdateCrc(uint32_t iCrc, void *pcData, size_t zDataSize)
{
    register uint32_t crc = iCrc;
    register unsigned char *cp = pcData;
    register size_t cnt = zDataSize;

    while(cnt--) {
	crc=((crc>>8)&M1_16)^kermit_crctab[(crc&0xff)^*cp++];
    }

    return(crc);
}
