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
 * @file status.h
 * @brief Status codes and failure-handling helpers for LibJuno.
 * @details
 *  This header defines the unified status type used across LibJuno along with
 *  a canonical set of status codes and small helper macros to invoke a
 *  user-provided failure handler. The design favors freestanding
 *  compatibility and deterministic error propagation.
 *
 *  Usage notes:
 *  - Functions that can fail should return JUNO_STATUS_T (or a JUNO_MODULE_RESULT
 *    wrapper that embeds a JUNO_STATUS_T alongside a value).
 *  - The failure handler is optional; when provided, helper macros will call it
 *    with the status and message but will not abort control flow.
 *
 * @defgroup juno_status Status and error handling
 * @brief Canonical status codes and failure callback hooks.
 * @details
 *   LibJuno uses a single 32-bit integer type for status propagation. Zero
 *   indicates success; non-zero values indicate specific failure conditions
 *   enumerated by the constants in this module. Optional failure callbacks let
 *   applications log or collect diagnostics without affecting control flow.
 */
#ifndef JUNO_STATUS_H
#define JUNO_STATUS_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/** @brief Canonical status type for LibJuno functions.
 *  @ingroup juno_status
 */
typedef int32_t JUNO_STATUS_T;
/** @defgroup juno_status_codes Status codes
 *  @ingroup juno_status
 *  @brief Well-defined error and success codes used by LibJuno.
 *  @{ 
 */
/** @brief Operation completed successfully. */
#define JUNO_STATUS_SUCCESS             0
/** @brief Unspecified error. */
#define JUNO_STATUS_ERR                 1
/** @brief A required pointer argument was NULL or invalid. */
#define JUNO_STATUS_NULLPTR_ERROR       2
/** @brief Memory allocation failed (hosted or platform-specific allocator). */
#define JUNO_STATUS_MEMALLOC_ERROR      3
/** @brief Memory free operation failed or was invalid. */
#define JUNO_STATUS_MEMFREE_ERROR       4
/** @brief Provided type or trait did not match the expected one. */
#define JUNO_STATUS_INVALID_TYPE_ERROR  5
/** @brief Provided size or alignment was invalid or unsupported. */
#define JUNO_STATUS_INVALID_SIZE_ERROR  6
/** @brief A fixed-capacity table/structure was full. */
#define JUNO_STATUS_TABLE_FULL_ERROR    7
/** @brief Requested element or key did not exist. */
#define JUNO_STATUS_DNE_ERROR           8
/** @brief Generic file I/O error on hosted platforms. */
#define JUNO_STATUS_FILE_ERROR          9
/** @brief Read operation failed or returned less than requested. */
#define JUNO_STATUS_READ_ERROR          10
/** @brief Write operation failed or returned less than requested. */
#define JUNO_STATUS_WRITE_ERROR         11
/** @brief CRC check failed or a CRC value was invalid. */
#define JUNO_STATUS_CRC_ERROR           12
/** @brief A reference identifier or handle was invalid. */
#define JUNO_STATUS_INVALID_REF_ERROR   13
/** @brief Resource cannot be freed while references are still active. */
#define JUNO_STATUS_REF_IN_USE_ERROR    14
/** @brief Input data failed validation. */
#define JUNO_STATUS_INVALID_DATA_ERROR  15
/** @brief Operation timed out. */
#define JUNO_STATUS_TIMEOUT_ERROR       16
/** @brief Index or pointer was out of bounds. */
#define JUNO_STATUS_OOB_ERROR           17
/** @} */

/** @brief Opaque user data type for failure callbacks.
 *  @ingroup juno_status
 */
typedef void JUNO_USER_DATA_T;
/**
 * @brief Failure handler callback signature.
 * @ingroup juno_status
 * @param tStatus Status code for the failure condition.
 * @param pcCustomMessage Optional, human-readable message with context (may be NULL).
 * @param pvUserData Opaque user data pointer supplied by the caller (may be NULL).
 */
typedef void (*JUNO_FAILURE_HANDLER_T)(JUNO_STATUS_T tStatus, const char *pcCustomMessage, JUNO_USER_DATA_T *pvUserData);

/** @brief Recommended maximum length for failure messages.
 *  @ingroup juno_status
 */
#ifndef JUNO_FAIL_MESSAGE_LEN
#define JUNO_FAIL_MESSAGE_LEN    256
#endif
/**
 * @def JUNO_FAIL(tStatus, pfcnFailureHandler, pvFailureUserData, pcMessage)
 * @brief Invoke a failure handler if provided.
 * @ingroup juno_status
 * @param tStatus Status value to report.
 * @param pfcnFailureHandler Failure handler function pointer (nullable).
 * @param pvFailureUserData Opaque user data pointer passed to the handler.
 * @param pcMessage Optional message describing the failure.
 * @note This macro does not alter control flow; callers should still return
 *       or handle the error as appropriate.
 */
#define JUNO_FAIL(tStatus, pfcnFailureHandler, pvFailureUserData, pcMessage) \
if(pfcnFailureHandler){pfcnFailureHandler(tStatus, pcMessage, pvFailureUserData);}

/**
 * @def JUNO_FAIL_MODULE(tStatus, ptMod, pcMessage)
 * @brief Invoke a module instance's failure handler if available.
 * @ingroup juno_status
 * @param tStatus Status value to report.
 * @param ptMod Pointer to a module instance (may be NULL).
 * @param pcMessage Optional message describing the failure.
 * @warning This macro expects the pointed-to object to expose a
 *          `JUNO_MODULE_SUPER` member with `JUNO_FAILURE_HANDLER` and
 *          `JUNO_FAILURE_USER_DATA` fields as defined by the module system.
 *          Use only with LibJuno module types.
 */
#define JUNO_FAIL_MODULE(tStatus, ptMod, pcMessage) \
if(ptMod && ptMod->JUNO_MODULE_SUPER.JUNO_FAILURE_HANDLER){ptMod->JUNO_MODULE_SUPER.JUNO_FAILURE_HANDLER(tStatus, pcMessage, ptMod->JUNO_MODULE_SUPER.JUNO_FAILURE_USER_DATA);}

/**
 * @def JUNO_FAIL_ROOT(tStatus, ptMod, pcMessage)
 * @brief Invoke a module root's failure handler if available.
 * @ingroup juno_status
 * @param tStatus Status value to report.
 * @param ptMod Pointer to a module root (may be NULL).
 * @param pcMessage Optional message describing the failure.
 */
#define JUNO_FAIL_ROOT(tStatus, ptMod, pcMessage) \
if(ptMod && ptMod->JUNO_FAILURE_HANDLER){ptMod->JUNO_FAILURE_HANDLER(tStatus, pcMessage, ptMod->JUNO_FAILURE_USER_DATA);}

#ifdef __cplusplus
}
#endif
#endif
