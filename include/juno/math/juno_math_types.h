#ifndef JUNO_TYPES_H
#define JUNO_TYPES_H

#include "juno/module.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

#ifndef JUNO_INT_TYPE
#define JUNO_INT_TYPE int64_t
#endif

#ifndef JUNO_FLOAT_TYPE
#define JUNO_FLOAT_TYPE double
#endif
/**
 * @struct JUNO_VEC2_F64_POLAR_TAG
 * @brief 2D vector in polar coordinates (double precision).
 *
 * Contains radial distance and angle in radians.
 */
typedef struct JUNO_VEC2_F64_POLAR_TAG
{
    struct {
        double r;   /**< Radial distance from the origin. */ 
        double phi; /**< Angle in radians from the x-axis. */ 
    };
} JUNO_VEC2_F64_POLAR_T;

/**
 * @struct JUNO_VEC2_F64_CART_TAG
 * @brief 2D vector in Cartesian coordinates (double precision).
 *
 * Contains x and y components.
 */
typedef struct JUNO_VEC2_F64_CART_TAG
{
    struct {
        double x; /**< X component. */
        double y; /**< Y component. */
    };
} JUNO_VEC2_F64_CART_T;

/**
 * @union JUNO_VEC2_F64_TAG
 * @brief 2D vector union supporting Cartesian, polar, and array access (double precision).
 *
 * - tCart: Cartesian form.
 * - tPolar: Polar form.
 * - arr: Raw component array [0]=x/r, [1]=y/phi.
 */
typedef union JUNO_VEC2_F64_TAG
{
    JUNO_VEC2_F64_CART_T  tCart;   /**< Cartesian coordinates. */
    JUNO_VEC2_F64_POLAR_T tPolar;  /**< Polar coordinates. */
    double                arr[2];  /**< Raw component array. */
} JUNO_VEC2_F64_T;

/**
 * @typedef JUNO_VEC2_F64_POLAR_RESULT_T
 * @brief Result type for functions returning JUNO_VEC2_F64_POLAR_T.
 */
JUNO_MODULE_RESULT(JUNO_VEC2_F64_POLAR_RESULT_T, JUNO_VEC2_F64_POLAR_T);

/**
 * @typedef JUNO_VEC2_F64_CART_RESULT_T
 * @brief Result type for functions returning JUNO_VEC2_F64_CART_T.
 */
JUNO_MODULE_RESULT(JUNO_VEC2_F64_CART_RESULT_T, JUNO_VEC2_F64_CART_T);

/**
 * @typedef JUNO_VEC2_F64_RESULT_T
 * @brief Result type for functions returning JUNO_VEC2_F64_T.
 */
JUNO_MODULE_RESULT(JUNO_VEC2_F64_RESULT_T, JUNO_VEC2_F64_T);

/**
 * @struct JUNO_VEC2_I32_POLAR_TAG
 * @brief 2D vector in polar coordinates (32-bit integer).
 *
 * Contains radial distance and angle.
 */
typedef struct JUNO_VEC2_I32_POLAR_TAG
{
    struct {
        int32_t r;   /**< Radial distance from the origin. */
        int32_t phi; /**< Angle unit (e.g., degrees or custom). */
    };
} JUNO_VEC2_I32_POLAR_T;

/**
 * @struct JUNO_VEC2_I32_CART_TAG
 * @brief 2D vector in Cartesian coordinates (32-bit integer).
 *
 * Contains x and y components.
 */
typedef struct JUNO_VEC2_I32_CART_TAG
{
    struct {
        int32_t x; /**< X component. */
        int32_t y; /**< Y component. */
    };
} JUNO_VEC2_I32_CART_T;

/**
 * @union JUNO_VEC2_I32_TAG
 * @brief 2D vector union supporting Cartesian, polar, and array access (32-bit integer).
 *
 * - tCart: Cartesian form.
 * - tPolar: Polar form.
 * - arr: Raw component array [0]=x/r, [1]=y/phi.
 */
typedef union JUNO_VEC2_I32_TAG
{
    JUNO_VEC2_I32_CART_T  tCart;   /**< Cartesian coordinates. */
    JUNO_VEC2_I32_POLAR_T tPolar;  /**< Polar coordinates. */
    int32_t               arr[2];  /**< Raw component array. */
} JUNO_VEC2_I32_T;

/**
 * @typedef JUNO_VEC2_I32_POLAR_RESULT_T
 * @brief Result type for functions returning JUNO_VEC2_I32_POLAR_T.
 */
JUNO_MODULE_RESULT(JUNO_VEC2_I32_POLAR_RESULT_T, JUNO_VEC2_I32_POLAR_T);

/**
 * @typedef JUNO_VEC2_I32_CART_RESULT_T
 * @brief Result type for functions returning JUNO_VEC2_I32_CART_T.
 */
JUNO_MODULE_RESULT(JUNO_VEC2_I32_CART_RESULT_T, JUNO_VEC2_I32_CART_T);

/**
 * @typedef JUNO_VEC2_I32_RESULT_T
 * @brief Result type for functions returning JUNO_VEC2_I32_T.
 */
JUNO_MODULE_RESULT(JUNO_VEC2_I32_RESULT_T, JUNO_VEC2_I32_T);

/**
 * @struct JUNO_VEC3_F64_POLAR_TAG
 * @brief 3D vector in spherical coordinates (double precision).
 *
 * Contains radius, azimuthal angle (phi), and polar angle (theta).
 */
typedef struct JUNO_VEC3_F64_POLAR_TAG
{
    struct {
        double r;     /**< Radial distance from the origin. */
        double phi;   /**< Azimuthal angle in radians. */
        double theta; /**< Polar angle in radians. */
    };
} JUNO_VEC3_F64_POLAR_T;

/**
 * @struct JUNO_VEC3_F64_CART_TAG
 * @brief 3D vector in Cartesian coordinates (double precision).
 *
 * Contains x, y, and z components.
 */
typedef struct JUNO_VEC3_F64_CART_TAG
{
    struct {
        double x; /**< X component. */
        double y; /**< Y component. */
        double z; /**< Z component. */
    };
} JUNO_VEC3_F64_CART_T;

/**
 * @union JUNO_VEC3_F64_TAG
 * @brief 3D vector union supporting Cartesian, spherical, and array access (double precision).
 *
 * - tCart: Cartesian form.
 * - tPolar: Spherical form.
 * - arr: Raw component array [0]=x/r, [1]=y/phi, [2]=z/theta.
 */
typedef union JUNO_VEC3_F64_TAG
{
    JUNO_VEC3_F64_CART_T  tCart;   /**< Cartesian coordinates. */
    JUNO_VEC3_F64_POLAR_T tPolar;  /**< Spherical coordinates. */
    double                arr[3];  /**< Raw component array. */
} JUNO_VEC3_F64_T;

/**
 * @typedef JUNO_VEC3_F64_POLAR_RESULT_T
 * @brief Result type for functions returning JUNO_VEC3_F64_POLAR_T.
 */
JUNO_MODULE_RESULT(JUNO_VEC3_F64_POLAR_RESULT_T, JUNO_VEC3_F64_POLAR_T);

/**
 * @typedef JUNO_VEC3_F64_CART_RESULT_T
 * @brief Result type for functions returning JUNO_VEC3_F64_CART_T.
 */
JUNO_MODULE_RESULT(JUNO_VEC3_F64_CART_RESULT_T, JUNO_VEC3_F64_CART_T);

/**
 * @typedef JUNO_VEC3F_RESULT_T
 * @brief Result type for functions returning JUNO_VEC3_F64_T.
 */
JUNO_MODULE_RESULT(JUNO_VEC3F_RESULT_T, JUNO_VEC3_F64_T);

/**
 * @struct JUNO_VEC3_I32_POLAR_TAG
 * @brief 3D vector in spherical coordinates (32-bit integer).
 *
 * Contains radius, azimuthal angle, and polar angle.
 */
typedef struct JUNO_VEC3_I32_POLAR_TAG
{
    struct {
        int32_t r;     /**< Radial distance from the origin. */
        int32_t phi;   /**< Azimuthal angle unit. */
        int32_t theta; /**< Polar angle unit. */
    };
} JUNO_VEC3_I32_POLAR_T;

/**
 * @struct JUNO_VEC3_I32_CART_TAG
 * @brief 3D vector in Cartesian coordinates (32-bit integer).
 *
 * Contains x, y, and z components.
 */
typedef struct JUNO_VEC3_I32_CART_TAG
{
    struct {
        int32_t x; /**< X component. */
        int32_t y; /**< Y component. */
        int32_t z; /**< Z component. */
    };
} JUNO_VEC3_I32_CART_T;

/**
 * @union JUNO_VEC3_I32_TAG
 * @brief 3D vector union supporting Cartesian, spherical, and array access (32-bit integer).
 *
 * - tCart: Cartesian form.
 * - tPolar: Spherical form.
 * - arr: Raw component array [0]=x/r, [1]=y/phi, [2]=z/theta.
 */
typedef union JUNO_VEC3_I32_TAG
{
    JUNO_VEC3_I32_CART_T  tCart;   /**< Cartesian coordinates. */
    JUNO_VEC3_I32_POLAR_T tPolar;  /**< Spherical coordinates. */
    int32_t               arr[3];  /**< Raw component array. */
} JUNO_VEC3_I32_T;

/**
 * @typedef JUNO_VEC3_I32_POLAR_RESULT_T
 * @brief Result type for functions returning JUNO_VEC3_I32_POLAR_T.
 */
JUNO_MODULE_RESULT(JUNO_VEC3_I32_POLAR_RESULT_T, JUNO_VEC3_I32_POLAR_T);

/**
 * @typedef JUNO_VEC3_I32_CART_RESULT_T
 * @brief Result type for functions returning JUNO_VEC3_I32_CART_T.
 */
JUNO_MODULE_RESULT(JUNO_VEC3_I32_CART_RESULT_T, JUNO_VEC3_I32_CART_T);

/**
 * @typedef JUNO_VEC3_I32_RESULT_T
 * @brief Result type for functions returning JUNO_VEC3_I32_T.
 */
JUNO_MODULE_RESULT(JUNO_VEC3_I32_RESULT_T, JUNO_VEC3_I32_T);

/**
 * @struct JUNO_VEC4_F64_POLAR_TAG
 * @brief 4D vector in hyperspherical coordinates (double precision).
 *
 * Contains radial distance and three angles.
 */
typedef struct JUNO_VEC4_F64_POLAR_TAG
{
    struct {
        double r;     /**< Radial distance from the origin. */
        double phi;   /**< First angular coordinate. */
        double theta; /**< Second angular coordinate. */
        double rho;   /**< Third angular coordinate. */
    };
} JUNO_VEC4_F64_POLAR_T;

/**
 * @struct JUNO_VEC4_F64_CART_TAG
 * @brief 4D vector in Cartesian coordinates (double precision).
 *
 * Contains x, y, z, and w components.
 */
typedef struct JUNO_VEC4_F64_CART_TAG
{
    struct {
        double x; /**< X component. */
        double y; /**< Y component. */
        double z; /**< Z component. */
        double w; /**< W component. */
    };
} JUNO_VEC4_F64_CART_T;

/**
 * @union JUNO_VEC4_F64_TAG
 * @brief 4D vector union supporting Cartesian, hyperspherical, and array access (double precision).
 *
 * - tCart: Cartesian form.
 * - tPolar: Hyperspherical form.
 * - arr: Raw component array [0]=x/r, [1]=y/phi, [2]=z/theta, [3]=w/rho.
 */
typedef union JUNO_VEC4_F64_TAG
{
    JUNO_VEC4_F64_CART_T  tCart;   /**< Cartesian coordinates. */
    JUNO_VEC4_F64_POLAR_T tPolar;  /**< Hyperspherical coordinates. */
    double                arr[4];  /**< Raw component array. */
} JUNO_VEC4_F64_T;

/**
 * @typedef JUNO_VEC4F_RESULT_T
 * @brief Result type for functions returning JUNO_VEC3_F64_T (per original macro invocation).
 */
JUNO_MODULE_RESULT(JUNO_VEC4F_RESULT_T, JUNO_VEC3_F64_T);

/**
 * @struct JUNO_VEC4_I32_POLAR_TAG
 * @brief 4D vector in hyperspherical coordinates (32-bit integer).
 *
 * Contains radial distance and three angles.
 */
typedef struct JUNO_VEC4_I32_POLAR_TAG
{
    struct {
        int32_t r;     /**< Radial distance from the origin. */
        int32_t phi;   /**< First angular coordinate. */
        int32_t theta; /**< Second angular coordinate. */
        int32_t rho;   /**< Third angular coordinate. */
    };
} JUNO_VEC4_I32_POLAR_T;

/**
 * @struct JUNO_VEC4_I32_CART_TAG
 * @brief 4D vector in Cartesian coordinates (32-bit integer).
 *
 * Contains x, y, z, and w components.
 */
typedef struct JUNO_VEC4_I32_CART_TAG
{
    struct {
        int32_t x; /**< X component. */
        int32_t y; /**< Y component. */
        int32_t z; /**< Z component. */
        int32_t w; /**< W component. */
    };
} JUNO_VEC4_I32_CART_T;

/**
 * @union JUNO_VEC4_I32_TAG
 * @brief 4D vector union supporting Cartesian, hyperspherical, and array access (32-bit integer).
 *
 * - tCart: Cartesian form.
 * - tPolar: Hyperspherical form.
 * - arr: Raw component array [0]=x/r, [1]=y/phi, [2]=z/theta, [3]=w/rho.
 */
typedef union JUNO_VEC4_I32_TAG
{
    JUNO_VEC4_I32_CART_T  tCart;   /**< Cartesian coordinates. */
    JUNO_VEC4_I32_POLAR_T tPolar;  /**< Hyperspherical coordinates. */
    int32_t               arr[4];  /**< Raw component array. */
} JUNO_VEC4_I32_T;

/**
 * @typedef JUNO_VEC4_I32_POLAR_RESULT_T
 * @brief Result type for functions returning JUNO_VEC4_I32_POLAR_T.
 */
JUNO_MODULE_RESULT(JUNO_VEC4_I32_POLAR_RESULT_T, JUNO_VEC4_I32_POLAR_T);

/**
 * @typedef JUNO_VEC4_I32_CART_RESULT_T
 * @brief Result type for functions returning JUNO_VEC4_I32_CART_T.
 */
JUNO_MODULE_RESULT(JUNO_VEC4_I32_CART_RESULT_T, JUNO_VEC4_I32_CART_T);

/**
 * @typedef JUNO_VEC4_I32_RESULT_T
 * @brief Result type for functions returning JUNO_VEC4_I32_T.
 */
JUNO_MODULE_RESULT(JUNO_VEC4_I32_RESULT_T, JUNO_VEC4_I32_T);

/**
 * @struct JUNO_M3X3_F64_TAG
 * @brief 3×3 matrix of double-precision values.
 */
typedef struct JUNO_M3X3_F64_TAG
{
    double mat[3][3]; /**< Row-major 3×3 matrix. */
} JUNO_M3X3_F64_T;

/**
 * @typedef JUNO_M3X3F_RESULT_T
 * @brief Result type for functions returning JUNO_M3X3_F64_T.
 */
JUNO_MODULE_RESULT(JUNO_M3X3F_RESULT_T, JUNO_M3X3_F64_T);

/**
 * @struct JUNO_M4X4_F64_TAG
 * @brief 4×4 matrix of double-precision values.
 */
typedef struct JUNO_M4X4_F64_TAG
{
    double mat[4][4]; /**< Row-major 4×4 matrix. */
} JUNO_M4X4_F64_T;

/**
 * @typedef JUNO_M4X4F_RESULT_T
 * @brief Result type for functions returning JUNO_M4X4_F64_T.
 */
JUNO_MODULE_RESULT(JUNO_M4X4F_RESULT_T, JUNO_M4X4_F64_T);

/**
 * @struct JUNO_M3X3_I32_TAG
 * @brief 3×3 matrix of 32-bit integers.
 */
typedef struct JUNO_M3X3_I32_TAG
{
    int32_t mat[3][3]; /**< Row-major 3×3 matrix. */
} JUNO_M3X3_I32_T;

/**
 * @typedef JUNO_M3X3I_RESULT_T
 * @brief Result type for functions returning JUNO_M3X3_I32_T.
 */
JUNO_MODULE_RESULT(JUNO_M3X3I_RESULT_T, JUNO_M3X3_I32_T);

/**
 * @struct JUNO_M4X4_I32_TAG
 * @brief 4×4 matrix of 32-bit integers.
 */
typedef struct JUNO_M4X4_I32_TAG
{
    int32_t mat[4][4]; /**< Row-major 4×4 matrix. */
} JUNO_M4X4_I32_T;

/**
 * @typedef JUNO_M4X4I_RESULT_T
 * @brief Result type for functions returning JUNO_M4X4_I32_T.
 */
JUNO_MODULE_RESULT(JUNO_M4X4I_RESULT_T, JUNO_M4X4_I32_T);

/**
 * @union JUNO_RQUAT_F64_TAG
 * @brief Right-handed quaternion (double precision).
 *
 * Can be accessed as individual components or raw array.
 */
typedef union JUNO_RQUAT_F64_TAG
{
    struct {
        double s; /**< Scalar part. */
        double i; /**< First vector part. */
        double j; /**< Second vector part. */
        double k; /**< Third vector part. */
    };
    double quat[4]; /**< Raw quaternion array [s,i,j,k]. */
} JUNO_RQUAT_F64_T;

/**
 * @typedef JUNO_RQUATF_RESULT_T
 * @brief Result type for functions returning JUNO_RQUAT_F64_T.
 */
JUNO_MODULE_RESULT(JUNO_RQUATF_RESULT_T, JUNO_RQUAT_F64_T);

/**
 * @union JUNO_RQUAT_I32_TAG
 * @brief Right-handed quaternion (32-bit integer).
 *
 * Can be accessed as individual components or raw array.
 */
typedef union JUNO_RQUAT_I32_TAG
{
    struct {
        int32_t s; /**< Scalar part. */
        int32_t i; /**< First vector part. */
        int32_t j; /**< Second vector part. */
        int32_t k; /**< Third vector part. */
    };
    int32_t quat[4]; /**< Raw quaternion array [s,i,j,k]. */
} JUNO_RQUAT_I32_T;

/**
 * @typedef JUNO_RQUATI_RESULT_T
 * @brief Result type for functions returning JUNO_RQUAT_I32_T.
 */
JUNO_MODULE_RESULT(JUNO_RQUATI_RESULT_T, JUNO_RQUAT_I32_T);

#ifdef __cplusplus
}
#endif
#endif
