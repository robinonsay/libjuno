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

/**
    This API has been generated by LibJuno:
    https://www.robinonsay.com/libjuno/
*/

/**
    This header contains the juno_map impl implementation
    @author
*/
#ifndef JUNO_MAP_IMPL_H
#define JUNO_MAP_IMPL_H
#include "juno/module.h"
#include "juno/status.h"
#include "juno/map/map_api.h"
#ifdef __cplusplus
extern "C"
{
#endif


typedef struct JUNO_MAP_IMPL_TAG JUNO_MAP_IMPL_T;

struct JUNO_MAP_IMPL_TAG JUNO_MODULE_DERIVE(JUNO_MAP_ROOT_T,
    /*
    
    TODO: Include implementation specific members here
    
    */
);

#ifdef JUNO_MAP_DEFAULT
/**
    This is the default implementation for `JUNO_MAP_T`.
    If you want to use the default implementation for `JUNO_MAP_T`
    use `#define JUNO_MAP_DEFAULT` prior to including
    `#include "juno_map_impl.h"`

    Note: If you are implementing a derived module you will need
    to implement `JUNO_MAP_IMPL`.
*/
union JUNO_MAP_TAG JUNO_MODULE(JUNO_MAP_API_T, JUNO_MAP_ROOT_T,
    JUNO_MAP_IMPL_T tJunoMapImpl;
);
#endif

JUNO_STATUS_T JunoMap_ImplApi(
    JUNO_MAP_T *ptJunoMap,
    JUNO_HASH_T *ptHash,
    JUNO_MEMORY_T *ptKeyTable,
    JUNO_MEMORY_T *ptValueTable,
    size_t zCapacity,
    JUNO_MAP_KEY_EQUAL_FCN_T pfcnIsEqual,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
);
#ifdef __cplusplus
}
#endif
#endif // JUNO_MAP_IMPL_H

