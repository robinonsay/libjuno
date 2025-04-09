#ifndef HASH_H
#define HASH_H
#include "juno/macros.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "juno/status.h"

typedef enum JUNO_HASH_TYPE_TAG
{
    JUNO_HASH_TYPE_RESERVED = 0,
    JUNO_HASH_TYPE_DJB2     = 1,
} JUNO_HASH_TYPE_T;

inline JUNO_STATUS_T Juno_HashDjB2(const uint8_t *pcBuff, size_t zBuffSize, size_t *pzRetHash)
{
    ASSERT_EXISTS((pcBuff && pzRetHash));
    size_t zHash = 5381;
    for(size_t i = 0; i < zBuffSize; i++)
    {
        zHash = ((zHash << 5) + zHash) + pcBuff[i];
    }
    *pzRetHash = zHash;
    return JUNO_STATUS_SUCCESS;
}

inline JUNO_STATUS_T Juno_Hash(JUNO_HASH_TYPE_T tType, const uint8_t *pcBuff, size_t zBuffSize, size_t *pzRetSize)
{
    switch (tType)
    {
        case JUNO_HASH_TYPE_DJB2:
        return Juno_HashDjB2(pcBuff, zBuffSize, pzRetSize);
        default:
        return JUNO_STATUS_INVALID_TYPE_ERROR;
    }
}

#ifdef __cplusplus
}
#endif
#endif
