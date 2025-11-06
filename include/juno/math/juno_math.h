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
 * @file juno_math.h
 * @brief Convenience header: math types plus inline vector/quaternion ops.
 * @ingroup juno_math
 * @details
 *  Umbrella include that aggregates:
 *   - `juno_math_types.h` — structural type definitions (vectors, matrices, real quaternions)
 *   - `juno_vec.h` — inline operations (add/sub/scale, dot/cross, norms, quaternion algebra)
 *
 *  Use this header when you want both the math types and the inline utilities in one place.
 *  If you only need type definitions without inline operations, prefer `juno_math_types.h`.
 *
 *  Quaternions use the component order [s, i, j, k] and the Hamilton product
 *  follows the right-handed convention (i^2 = j^2 = k^2 = ijk = -1). See
 *  `Juno_RQuat_F64_HamProd` in `juno_vec.h` for the exact formula.
 *
 * @code{.c}
 *  #include "juno/math/juno_math.h"
 *  JUNO_VEC3_F64_T a = {{1.0, 2.0, 3.0}};
 *  JUNO_VEC3_F64_T b = {{4.0, 5.0, 6.0}};
 *  JUNO_VEC3_F64_T c = Juno_Vec3_F64_Add(a, b); // {5, 7, 9}
 * @endcode
 */

#ifndef JUNO_MATH_H
#define JUNO_MATH_H

#include "juno_math_types.h"
#include "juno_vec.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif
