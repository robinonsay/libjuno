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
 * @file macros.h
 * @brief Common assertion and helper macros for LibJuno modules.
 * @defgroup juno_macros Assertions and helpers
 * @details
 *  These macros provide lightweight, freestanding-friendly assertions and
 *  failure-notification hooks. They are designed to return error codes rather
 *  than aborting, in line with LibJuno's deterministic error handling.
 *
 *  @{
 */
#ifndef JUNO_MACROS_H
#define JUNO_MACROS_H

#include "juno/status.h"
#include <stdint.h>

/**
 * @def JUNO_ASSERT_EXISTS(ptr)
 * @ingroup juno_macros
 * @brief Returns JUNO_STATUS_NULLPTR_ERROR if the expression is falsy.
 * @details
 *  Use to validate required pointers or expressions. Typical usage within a
 *  function that returns JUNO_STATUS_T:
 *  @code{.c}
 *  JUNO_STATUS_T f(void *p) {
 *      JUNO_ASSERT_EXISTS(p);
 *      // safe to use p
 *      return JUNO_STATUS_SUCCESS;
 *  }
 *  @endcode
 * @param ptr Pointer/expression to validate (may combine with &&).
 */
#define JUNO_ASSERT_EXISTS(ptr) \
if(!(ptr)) \
{ \
    return JUNO_STATUS_NULLPTR_ERROR; \
}

/**
 * @def JUNO_ASSERT_EXISTS_MODULE(ptr, ptMod, str)
 * @ingroup juno_macros
 * @brief Like JUNO_ASSERT_EXISTS but also calls the module's failure handler.
 * @details Invokes `JUNO_FAIL_MODULE(JUNO_STATUS_NULLPTR_ERROR, ptMod, str)`
 *          before returning `JUNO_STATUS_NULLPTR_ERROR`.
 * @param ptr The dependency expression to validate.
 * @param ptMod The module instance providing the failure handler.
 * @param str Message passed to the failure handler upon failure.
 */
#define JUNO_ASSERT_EXISTS_MODULE(ptr, ptMod, str) if(!(ptr)) \
{ \
    JUNO_FAIL_MODULE(JUNO_STATUS_NULLPTR_ERROR, ptMod, str); \
    return JUNO_STATUS_NULLPTR_ERROR; \
}

/**
 * @def JUNO_ASSERT_SUCCESS(tStatus, ...)
 * @ingroup juno_macros
 * @brief Execute the provided failure operation(s) if status is not success.
 * @details
 *  Intended for early-returns and cleanup in error paths:
 *  @code{.c}
 *  JUNO_STATUS_T t = do_something();
 *  JUNO_ASSERT_SUCCESS(t, return t);
 *  @endcode
 *  The variadic statements are emitted verbatim inside the error branch;
 *  ensure they are valid in the enclosing scope.
 * @param tStatus Status value to check.
 * @param ... Statements to execute when tStatus != JUNO_STATUS_SUCCESS.
 */
#define JUNO_ASSERT_SUCCESS(tStatus, ...) if(tStatus != JUNO_STATUS_SUCCESS) \
{ \
    __VA_ARGS__; \
}

/** @} */

#endif // JUNO_MACROS_H
