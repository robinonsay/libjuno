#ifndef HASH_H
#define HASH_H
#include "juno/status.h"
#include <stddef.h>
#include <stdint.h>
#include "juno/hash/hash_api.h"
#ifdef __cplusplus
extern "C" {
#endif

JUNO_STATUS_T Juno_HashDjB2(const uint8_t *pcBuff, size_t zBuffSize, size_t *pzRetHash);

const JUNO_HASH_API_T* Juno_HashDjB2Api(void);
#ifdef __cplusplus
}
#endif
#endif

