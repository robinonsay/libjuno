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
#ifndef JUNO_MACROS_H
#define JUNO_MACROS_H

#include "juno/status.h"
#include <stdint.h>

/**
    Assert if `ptr` exists.
    For example `JUNO_ASSERT_EXISTS(ptMyPointerFoo)` or JUNO_ASSERT_EXISTS(ptMyPointerFoo && ptMyPointerBar)
    @param ptr A pointer or pointers concatinated with &&
*/
#define JUNO_ASSERT_EXISTS(ptr) \
if(!(ptr)) \
{ \
    return JUNO_STATUS_NULLPTR_ERROR; \
}

/**
    Assert if a module's dependencies exist
    @param ptr the Module dependecy or dependencies
    to assert (similar to `JUNO_ASSERT_EXISTS`)
    @param ptMod The module
    @param str The error message if `ptr` fails assertion
*/
#define JUNO_ASSERT_EXISTS_MODULE(ptr, ptMod, str) if(!(ptr)) \
{ \
    JUNO_FAIL_MODULE(JUNO_STATUS_NULLPTR_ERROR, ptMod, str); \
    return JUNO_STATUS_NULLPTR_ERROR; \
}

/**
    Assert the status is a success, if not perform `failOp`
    @param tStatus The status to assert
    @param failOp The failure operation
*/
#define JUNO_ASSERT_SUCCESS(tStatus, ...) if(tStatus != JUNO_STATUS_SUCCESS) \
{ \
    __VA_ARGS__; \
}


#endif // JUNO_MACROS_H
