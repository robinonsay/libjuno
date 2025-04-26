#include "juno/crc/crc.h"
#include <stdio.h>
#include "zip.h"

uint32_t Juno_ZipUpdateCrc(uint32_t iCrc, void *pcData, size_t zDataSize)
{
    register uint32_t crc = iCrc;
    register const uint8_t *data = pcData;
    register size_t cnt = zDataSize;
    for (size_t i = 0; i < cnt; i++) {
        uint8_t lookup_index = (uint8_t)(crc ^ data[i]);
        crc = (crc >> 8) ^ zip_crctab[lookup_index];
    }
    return(crc ^ 0xFFFFFFFF);
}
