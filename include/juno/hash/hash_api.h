#ifndef HASH_API_H
#define HASH_API_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "juno/status.h"

typedef struct JUNO_HASH_API_TAG JUNO_HASH_API_T;

struct JUNO_HASH_API_TAG
{

    JUNO_STATUS_T (*Hash)(const uint8_t *pcBuff, size_t zBuffSize, size_t *pzRetHash);
};


#ifdef __cplusplus
}
#endif
#endif
