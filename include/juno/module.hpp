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
#ifndef JUNO_MODULE_HPP
#define JUNO_MODULE_HPP

#include "status.h"
#include <stdint.h>
#include "module.h"

#define JUNO_MODULE_ARG(...)    __VA_ARGS__

namespace juno
{

template<typename T>
struct RESULT_T
{
    JUNO_STATUS_T tStatus;
    T tSuccess;
};

template<typename T>
struct OPTION_T
{
    bool bIsSome;
    T tSome;
};

}

#endif // JUNO_MODULE_H
