#include "juno/hash/hash_api.h"
#include "juno/hash/hash.h"
#include "juno/macros.h"


JUNO_STATUS_T Juno_HashDjB2(const uint8_t *pcBuff, size_t zBuffSize, size_t *pzRetHash)
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


static const JUNO_HASH_API_T tDjB2HashApi =
{
    .Hash = Juno_HashDjB2
};

const JUNO_HASH_API_T* Juno_HashDjB2Api(void)
{
    return &tDjB2HashApi;
}
