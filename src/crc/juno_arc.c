#include "juno/crc/crc.h"
#include <stdint.h>
#include <stdio.h>
#include "arc.h"

uint16_t Juno_CrcArcUpdate(uint16_t iCrc, const void *pcData, size_t zDataSize)
{
    register uint16_t crc = iCrc;
    register const uint8_t *cp = pcData;
    register size_t cnt = zDataSize;

    while(cnt--) {
	crc=((crc>>8)&M1_16)^arc_crctab[(crc&0xff)^*cp++];
    }

    return(crc);
}
