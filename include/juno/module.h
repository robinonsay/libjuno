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
#ifndef JUNO_MODULE_H
#define JUNO_MODULE_H

#include "juno/status.h"
#include <stdint.h>
#include <stdbool.h>
/**
 * @file module.h
 * @brief Module system and dependency injection primitives for LibJuno.
 * @defgroup juno_module Module system and DI
 * @details
 *  LibJuno implements dependency injection (DI) via a small module system.
 *  A module consists of:
 *  - A module root: common, freestanding members shared by all implementations.
 *  - One or more derived implementations: embed the root as their first member
 *    and add implementation-specific state and dependencies.
 *  - A module union: a union of the root and all derived implementations,
 *    enabling safe up-casts to the root.
 *
 *  Key properties:
 *  - Every derived implementation starts with the root, allowing safe up-casts
 *    to the root type for API dispatch and failure handling.
 *  - The root contains `ptApi` and failure handler fields and should avoid
 *    hosted dependencies to preserve freestanding portability.
 *  - APIs (vtables) receive a pointer to the root (or compatible view) for
 *    dispatch.
 *
 *  Example (defining a module union):
 *  @code{.c}
 *  union MY_MODULE_T JUNO_MODULE(MY_MODULE_API_T, MY_MODULE_ROOT_T,
 *      MY_MODULE_DERIVATION_1_T;
 *      MY_MODULE_DERIVATION_2_T;
 *      MY_MODULE_DERIVATION_3_T;
 *  );
 *  @endcode
 *  @{ 
 */


/**
 * @def JUNO_MODULE_DECLARE(NAME_T)
 * @brief Forward-declare a Juno module union type.
 * @param NAME_T The module union type name.
 */
#define JUNO_MODULE_DECLARE(NAME_T)   typedef union NAME_T NAME_T
/**
 * @def JUNO_MODULE_ROOT_DECLARE(NAME_T)
 * @brief Forward-declare a module root struct type.
 * @param NAME_T The root struct type name.
 */
#define JUNO_MODULE_ROOT_DECLARE(NAME_T)   typedef struct NAME_T NAME_T
/**
 * @def JUNO_MODULE_DERIVE_DECLARE(NAME_T)
 * @brief Forward-declare a derived module struct type.
 * @param NAME_T The derived struct type name.
 */
#define JUNO_MODULE_DERIVE_DECLARE(NAME_T)   JUNO_MODULE_ROOT_DECLARE(NAME_T)

/** @brief Member name alias for a module's failure handler.
 *  @ingroup juno_module
 */
#define JUNO_FAILURE_HANDLER    _pfcnFailureHandler
/** @brief Member name alias for failure handler user data.
 *  @ingroup juno_module
 */
#define JUNO_FAILURE_USER_DATA    _pvFailureUserData
/** @brief Helper for module definitions with no additional members.
 *  @ingroup juno_module
 */
#define JUNO_MODULE_EMPTY

/** @brief Pass-through argument pack helper for module macros.
 *  @ingroup juno_module
 */
#define JUNO_MODULE_ARG(...)    __VA_ARGS__

/** @brief Standard member name for the embedded module root.
 *  @ingroup juno_module
 */
#define JUNO_MODULE_SUPER   tRoot

/**
 * @def JUNO_MODULE(API_T, ROOT_T, ...)
 * @brief Define a module union consisting of the root and all derivations.
 * @ingroup juno_module
 * @param API_T The API (vtable) type for the module.
 * @param ROOT_T The root struct type for the module.
 * @param ... One or more derived struct members, each provided as a full
 *            struct member declaration and separated by semicolons in the
 *            invocation. Use JUNO_MODULE_EMPTY if there are no derivations.
 * @note The first member of the union is always the root, accessible via
 *       the alias JUNO_MODULE_SUPER.
 */
#define JUNO_MODULE(API_T, ROOT_T, ...) \
{ \
    ROOT_T JUNO_MODULE_SUPER; \
    __VA_ARGS__ \
}

/**
 * @def JUNO_MODULE_ROOT(API_T, ...)
 * @brief Implement a module root struct containing ptApi and failure fields.
 * @ingroup juno_module
 * @param API_T The API (vtable) type for the module.
 * @param ... Additional root members (freestanding types recommended).
 * @note Expands to a struct body whose first fields are:
 *       `const API_T *ptApi;`, failure handler pointer, and user-data pointer,
 *       followed by user-specified members.
 */
#define JUNO_MODULE_ROOT(API_T, ...) \
{ \
    const API_T *ptApi; \
    JUNO_FAILURE_HANDLER_T JUNO_FAILURE_HANDLER; \
    JUNO_USER_DATA_T *JUNO_FAILURE_USER_DATA; \
    __VA_ARGS__ \
}

/**
 * @def JUNO_TRAIT_ROOT(API_T, ...)
 * @brief Define a trait root carrying only the API pointer and members.
 * @ingroup juno_module
 * @param API_T The API (vtable) type for the trait.
 * @param ... Additional members for the trait root.
 * @note Traits omit failure handler fields and only carry the API pointer.
 */
#define JUNO_TRAIT_ROOT(API_T, ...) \
{ \
    const API_T *ptApi; \
    __VA_ARGS__ \
}

/**
 * @def JUNO_MODULE_DERIVE(ROOT_T, ...)
 * @brief Implement a derived module embedding the root as the first member.
 * @ingroup juno_module
 * @param ROOT_T The module root type.
 * @param ... Implementation-specific members.
 * @note The first member is always the embedded root, accessible via
 *       JUNO_MODULE_SUPER.
 */
#define JUNO_MODULE_DERIVE(ROOT_T, ...) \
{ \
    ROOT_T JUNO_MODULE_SUPER; \
    __VA_ARGS__ \
}

/** @brief Alias to define a trait derivation with the standard layout.
 *  @ingroup juno_module
 */
#define JUNO_TRAIT_DERIVE(ROOT_T, ...) JUNO_MODULE_DERIVE(ROOT_T, __VA_ARGS__)

/**
 * @def JUNO_MODULE_GET_API(ptModule, ROOT_T)
 * @brief Retrieve the API pointer from a module (via its root view).
 * @ingroup juno_module
 * @param ptModule Pointer to the module instance (module union or any
 *                 derived/root view).
 * @param ROOT_T The root type of the module.
 * @return The `const API_T*` stored in the root's `ptApi` field.
 * @warning This cast assumes the root is the first member of any derived
 *          or union representation, which is guaranteed when using the
 *          macros in this header.
 */
#define JUNO_MODULE_GET_API(ptModule, ROOT_T) ((const ROOT_T *)ptModule)->ptApi


/**
 * @def JUNO_MODULE_RESULT(NAME_T, OK_T)
 * @brief Define a result type combining a status and a success payload.
 * @ingroup juno_module
 * @param NAME_T Name of the result struct to define.
 * @param OK_T Type of the success payload contained in the result.
 * @details The generated struct has fields:
 *          - `JUNO_STATUS_T tStatus;`
 *          - `OK_T tOk;`
 */
#define JUNO_MODULE_RESULT(NAME_T, OK_T) \
typedef struct NAME_T \
{ \
    JUNO_STATUS_T tStatus; \
    OK_T tOk; \
} NAME_T

/** @def JUNO_OK(result)
 *  @brief Access the success payload from a result produced by JUNO_MODULE_RESULT.
 *  @ingroup juno_module
 */
#define JUNO_OK(result) result.tOk
/** @def JUNO_ASSERT_OK(result, ...)
 *  @brief Execute the provided statements if `result.tStatus` is not success.
 *  @ingroup juno_module
 *  @details This is a convenience wrapper over JUNO_ASSERT_SUCCESS. Typical usage:
 *  @code
 *  MY_RESULT_T r = DoThing();
 *  JUNO_ASSERT_OK(r, return r.tStatus);
 *  use(JUNO_OK(r));
 *  @endcode
 */
#define JUNO_ASSERT_OK(result, ...) JUNO_ASSERT_SUCCESS(result.tStatus, __VA_ARGS__)
/** @def JUNO_OK_RESULT(value)
 *  @brief Construct a result indicating success with the provided payload.
 *  @ingroup juno_module
 */
#define JUNO_OK_RESULT(value) {JUNO_STATUS_SUCCESS, value}
/** @def JUNO_ERR_RESULT(err, value)
 *  @brief Construct a result indicating error with a payload (may be default-initialized).
 *  @ingroup juno_module
 */
#define JUNO_ERR_RESULT(err, value) {err, value}
/**
 * @def JUNO_MODULE_OPTION(NAME_T, SOME_T)
 * @brief Define an option type combining a presence flag and a payload.
 * @ingroup juno_module
 * @param NAME_T Name of the option struct to define.
 * @param SOME_T Type of the payload when present.
 * @details The generated struct has fields:
 *          - `bool bIsSome;`
 *          - `SOME_T tSome;`
 */
#define JUNO_MODULE_OPTION(NAME_T, SOME_T) \
typedef struct NAME_T \
{ \
    bool bIsSome; \
    SOME_T tSome; \
} NAME_T

/** @def JUNO_SOME(result)
 *  @brief Access the payload from an option produced by JUNO_MODULE_OPTION.
 *  @ingroup juno_module
 */
#define JUNO_SOME(result) result.tSome
/** @def JUNO_ASSERT_SOME(result, ...)
 *  @brief Execute the provided statements if `result.bIsSome` is false.
 *  @ingroup juno_module
 *  @details Typical usage:
 *  @code
 *  MY_OPTION_T o = MaybeGet();
 *  JUNO_ASSERT_SOME(o, return JUNO_STATUS_DNE_ERROR);
 *  use(JUNO_SOME(o));
 *  @endcode
 */
#define JUNO_ASSERT_SOME(result, ...) if(!result.bIsSome){ \
    __VA_ARGS__  \
}
/** @def JUNO_SOME_OPTION(value)
 *  @brief Construct an option in the present state with the provided payload.
 *  @ingroup juno_module
 */
#define JUNO_SOME_OPTION(value) {true, value}
/** @def JUNO_NONE_OPTION(default_value)
 *  @brief Construct an option in the empty state, carrying a default payload.
 *  @ingroup juno_module
 */
#define JUNO_NONE_OPTION(default_value) {false, default_value}

#endif // JUNO_MODULE_H
/** @} */
