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
#ifndef JUNO_MODULE_H
#define JUNO_MODULE_H

#include "juno/status.h"
#include <stdint.h>
/**
    Declare a juno module implemented as a union
    @name The name of the module type
*/
#define JUNO_MODULE_DECLARE(name)   typedef union name##_TAG name
/**
    Declare a base implementation for the module implemented as a struct
    @param name The name of the base implementation type
*/
#define JUNO_MODULE_BASE_DECLARE(name)   typedef struct name##_TAG name
/**
    Declare a derivation of a juno module implemented as a struct
    @param name The name of the derived module type
*/
#define JUNO_MODULE_DERIVE_DECLARE(name)   JUNO_MODULE_BASE_DECLARE(name)

/**
    Alias for the failure handler for a module
*/
#define JUNO_FAILURE_HANDLER    _pfcnFailureHandler
/**
    Alias for the failure handler user data for a module
*/
#define JUNO_FAILURE_USER_DATA    _pvFailurUserData
/**
    Empty macro that indicates a module
    implementation has no additional members 
*/
#define JUNO_MODULE_EMPTY
/**
    Alias for the module base implementation
*/
#define JUNO_MODULE_SUPER   tBase

/**
    Define a juno module. This needs to be done in
    the composition root. This is where users define
    all possible module implementations for a module.
    @param name The name of the module as declared
    @param API The name of the API type for the module
    @param base The name of the base implementation type
    for the module as declared
    @param derived The derived modules seperated by `;`

*/
#define JUNO_MODULE(name, API, base, derived) \
union name##_TAG \
{ \
    const API *ptApi; \
    base JUNO_MODULE_SUPER; \
    derived \
}

/**
    Implement a base for a module
    @param name The name of the module base implementation as declared
    @param API The API type for the module
    @param members The member components of the module base implementation
*/
#define JUNO_MODULE_BASE(name, API, members) \
struct name##_TAG \
{ \
    const API *ptApi; \
    members \
    JUNO_FAILURE_HANDLER_T JUNO_FAILURE_HANDLER; \
    JUNO_USER_DATA_T *JUNO_FAILURE_USER_DATA; \
}

/**
    Implement a derivation of a module
    @param name The name of the module derivation as declared
    @param base The name of the base implementation for the module as declared
    @param members The member components of the module derivation
*/
#define JUNO_MODULE_DERIVE(name, base, members) \
struct name##_TAG \
{ \
    base JUNO_MODULE_SUPER; \
    members \
}

/**
    Get the API pointer from the module
    @param ptModule The module pointer
    @param MODULE_BASE_NAME The base type of the module
*/
#define JUNO_MODULE_GET_API(ptModule, MODULE_BASE_NAME) ((const MODULE_BASE_NAME *)ptModule)->ptApi

#endif // JUNO_MODULE_H
