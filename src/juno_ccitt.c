/*
    MIT License

    Copyright (c) 2025 Robin A. Onsay

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.
*/
#include "juno/crc/crc.h"
#include <stdint.h>
#include "crc/ccitt.h"

uint16_t Juno_CrcCcittUpdate(uint16_t iCrc, const void *pcData, size_t zDataSize)
{
    if(!(pcData && zDataSize))
    {
        return iCrc;
    }
    register uint16_t crc = iCrc;
    register const uint8_t *cp = pcData;
    register size_t cnt = zDataSize;

    while(cnt--) {
	crc=((crc<<8)&M2_16)^ccitt_crctab[((crc>>8)&0xff)^*cp++];
    }

    return(crc);
}
