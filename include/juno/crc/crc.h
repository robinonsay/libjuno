/**
   Copyright 2025 Robin A. Onsay

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

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
#define M1_32 0xffffffff
#define M2_32 0xffffff00

#define JUNO_ARC_CRC_INIT (0)
uint16_t Juno_CrcArcUpdate(uint16_t iCrc, const void *pcData, size_t zDataSize);


#define JUNO_BINHEX_CRC_INIT (0)
uint16_t Juno_CrcBinhexUpdate(uint16_t iCrc, const void *pcData, size_t zDataSize);


#define JUNO_CCITT_CRC_INIT (-1)
uint16_t Juno_CrcCcittUpdate(uint16_t iCrc, const void *pcData, size_t zDataSize);


#define JUNO_CCITT32_CRC_INIT (-1)
uint32_t Juno_CrcCcitt32Update(uint32_t iCrc, const void *pcData, size_t zDataSize);


#define JUNO_KERMIT_CRC_INIT (0)
uint32_t Juno_CrcKermitUpdate(uint32_t iCrc, const void *pcData, size_t zDataSize);

#define JUNO_ZIP_CRC_INIT (-1)
uint32_t Juno_CrcZipUpdate(uint32_t iCrc, const void *pcData, size_t zDataSize);

#ifdef __cplusplus
}
#endif
#endif
