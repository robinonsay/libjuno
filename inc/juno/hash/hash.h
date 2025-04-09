#ifndef HASH_H
#define HASH_H
#include "juno/status.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

JUNO_STATUS_T Juno_HashDjB2(const uint8_t *pcBuff, size_t zBuffSize, size_t *pzRetHash);

#ifdef __cplusplus
}
#endif
#endif

