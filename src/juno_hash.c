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
#include "juno/hash/hash_djb2.h"
#include "juno/macros.h"

static const JUNO_HASH_API_T tJunoHashDjb2Api;

static inline JUNO_STATUS_T Verify(JUNO_HASH_T *ptJunoHash)
{
    JUNO_ASSERT_EXISTS(ptJunoHash);
    JUNO_HASH_DJB2_T *ptJunoHashDjb2 = (JUNO_HASH_DJB2_T *)(ptJunoHash);
    JUNO_ASSERT_EXISTS_MODULE(
        ptJunoHash && ptJunoHashDjb2->JUNO_MODULE_SUPER.ptApi
        /* TODO: Assert other dependencies and members here using &&*/,
        ptJunoHashDjb2,
        "Module does not have all dependencies"
    );
    if(ptJunoHashDjb2->JUNO_MODULE_SUPER.ptApi != &tJunoHashDjb2Api)
    {
        JUNO_FAIL_MODULE(JUNO_STATUS_INVALID_TYPE_ERROR, ptJunoHashDjb2, "Module has invalid API");
        return JUNO_STATUS_INVALID_TYPE_ERROR;
    }
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T Hash(JUNO_HASH_T *ptJunoHash, const uint8_t *pcBuff, size_t zBuffSize, size_t *pzRetHash)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptJunoHash);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus)
    size_t zHash = 5381;
    for(size_t i = 0; i < zBuffSize; i++)
    {
        zHash = ((zHash << 5) + zHash) + pcBuff[i];
    }
    *pzRetHash = zHash;
    return JUNO_STATUS_SUCCESS;
}


static const JUNO_HASH_API_T tJunoHashDjb2Api = {
    .Hash = Hash
};

JUNO_STATUS_T JunoHash_Djb2Api(JUNO_HASH_T *ptJunoHash, JUNO_FAILURE_HANDLER_T pfcnFailureHandler, JUNO_USER_DATA_T *pvFailureUserData)
{
    JUNO_ASSERT_EXISTS(ptJunoHash);
    JUNO_HASH_DJB2_T *ptJunoHashDjb2 = (JUNO_HASH_DJB2_T *)(ptJunoHash);
    ptJunoHashDjb2->JUNO_MODULE_SUPER.ptApi = &tJunoHashDjb2Api;
    ptJunoHashDjb2->JUNO_MODULE_SUPER.JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    ptJunoHashDjb2->JUNO_MODULE_SUPER.JUNO_FAILURE_USER_DATA = pvFailureUserData;
    JUNO_STATUS_T tStatus = Verify(ptJunoHash);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}
