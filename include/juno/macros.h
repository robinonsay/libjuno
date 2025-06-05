/*
    MIT License

    Copyright (c) Year Robin A. Onsay

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

#define ASSERT_EXISTS(ptr) \
if(!(ptr)) \
{ \
    return JUNO_STATUS_NULLPTR_ERROR; \
}

#define ASSERT_EXISTS_MODULE(ptr, ptMod, str) if(!(ptr)) \
{ \
    FAIL_MODULE(JUNO_STATUS_NULLPTR_ERROR, ptMod, str); \
    return JUNO_STATUS_NULLPTR_ERROR; \
}

#define ASSERT_SUCCESS(tStatus, failOp) if(tStatus != JUNO_STATUS_SUCCESS) \
{ \
    failOp; \
}


#endif // JUNO_MACROS_H
