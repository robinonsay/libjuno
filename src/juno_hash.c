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
