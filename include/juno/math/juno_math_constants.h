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
 * @file juno_math_constants.h
 * @brief Common math constants for LibJuno (double precision).
 * @defgroup juno_math Math Core
 * @details
 *  Provides frequently used mathematical constants as double-precision
 *  macros. These are intended for compile-time use and freestanding builds.
 */
#ifndef JUNO_CONST_H
#define JUNO_CONST_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def JUNO_PI
 * @ingroup juno_math
 * @brief Pi (π) in radians as a double-precision literal.
 */
#define JUNO_PI         3.141592653589793

/**
 * @def JUNO_HALF_PI
 * @ingroup juno_math
 * @brief Half pi (π/2) in radians as a double-precision literal.
 */
#define JUNO_HALF_PI    1.5707963267948966

/**
 * @def JUNO_QUART_PI
 * @ingroup juno_math
 * @brief Quarter pi (π/4) in radians as a double-precision literal.
 */
#define JUNO_QUART_PI   0.7853981633974483

/**
 * @def JUNO_E
 * @ingroup juno_math
 * @brief Euler's number e as a double-precision literal.
 */
#define JUNO_E          2.718281828459045

/**
 * @def ANG_EPSILON
 * @ingroup juno_math
 * @brief Small angle epsilon (radians) for approximate comparisons.
 */
#define ANG_EPSILON     1.0E-12

#ifdef __cplusplus
}
#endif
#endif
