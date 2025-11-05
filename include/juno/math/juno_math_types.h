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
 * @file juno_math_types.h
 * @brief Aggregated math type definitions (vectors, matrices, quaternions).
 * @ingroup juno_math
 * @details
 *  Convenience umbrella header that includes the structural math types from
 *  `juno_vec_types.h` (2D/3D/4D vectors in Cartesian/spherical/hyperspherical
 *  forms, matrices, and real quaternions).
 *
 *  Prefer including this header when you need only the type definitions. If
 *  you also want the inline vector/quaternion operations, include
 *  `juno_math.h` instead, which pulls in both this header and `juno_vec.h`.
 *
 * @code{.c}
 *  #include "juno/math/juno_math_types.h"
 *  // Access types like JUNO_VEC3_F64_T, JUNO_M4X4_F32_T, JUNO_RQUAT_F64_T
 * @endcode
 */
#ifndef JUNO_MATH_TYPES_H
#define JUNO_MATH_TYPES_H

// Re-export the core math types (vectors/matrices/quaternions)
#include "juno_vec_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif
