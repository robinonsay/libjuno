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

#ifndef JUNO_TYPES_H
#define JUNO_TYPES_H
#include "module.h"
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

JUNO_MODULE_RESULT(JUNO_RESULT_BOOL_T, bool);
JUNO_MODULE_RESULT(JUNO_RESULT_UINT32_T, uint32_t);
JUNO_MODULE_RESULT(JUNO_RESULT_F64_T, double);

#ifdef __cplusplus
}
#endif
#endif
