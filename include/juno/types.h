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
 * @file types.h
 * @brief Common module result type aliases used throughout LibJuno.
 * @defgroup juno_types Common result types
 * @details
 *  This header provides convenience specializations of the generic
 *  `JUNO_MODULE_RESULT(NAME_T, OK_T)` macro for frequently used payload types.
 *  These result types are used to return a status alongside a typed value in a
 *  single aggregate, promoting deterministic error handling without exceptions.
 *
 *  Usage example:
 *  @code{.c}
 *  JUNO_RESULT_UINT32_T r = ReadRegister();
 *  JUNO_ASSERT_OK(r, return r.tStatus);
 *  uint32_t value = JUNO_OK(r);
 *  @endcode
 *  @{
 */
#ifndef JUNO_TYPES_H
#define JUNO_TYPES_H
#include "juno/module.h"
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** @brief Result type carrying a bool payload.
 *  @ingroup juno_types
 */
JUNO_MODULE_RESULT(JUNO_RESULT_BOOL_T, bool);
/** @brief Result type carrying a 32-bit unsigned integer payload.
 *  @ingroup juno_types
 */
JUNO_MODULE_RESULT(JUNO_RESULT_UINT32_T, uint32_t);
/** @brief Result type carrying a double-precision floating-point payload.
 *  @ingroup juno_types
 */
JUNO_MODULE_RESULT(JUNO_RESULT_F64_T, double);
/** @brief Result type carrying a size_t payload.
 *  @ingroup juno_types
 */
JUNO_MODULE_RESULT(JUNO_RESULT_SIZE_T, size_t);
/** @brief Result type carrying a void* payload.
 *  @ingroup juno_types
 */
JUNO_MODULE_RESULT(JUNO_RESULT_VOID_PTR_T, void *);
/** @} */

#ifdef __cplusplus
}
#endif
#endif
