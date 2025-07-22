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

#include "status.h"
#include <stdint.h>

/**DOC
    ## Overview
    LibJuno implements dependency injection through modules
    and API traits. This is very similar to the Rust paradigm.
    There are 3 components to modules within LibJuno:
    
    * The module
    * The module root
    * The module derivations

    ## The Module Root
    Before talking about the module itself we need to go over the
    module root.

    * The module root is a common set of member
      variables shared across all module derivations and
      implementations.
    * When a module is derived there is a
      compile-time gurantee that the module can be safely up-cast
      to the module root.
        * All derivations of the module can up-cast
          to the module root.
    * The module root always provides the following as defined by
      the macros:
        * `ptApi` -- A pointer to the API traits
        * `_pfcnFailureHandler` -- A failure handler callback
        * `_pvFailurUserData` -- User defined data pointer to
          provide to the failure handler callback
    * **Members within the module root should be freestanding**
        * The real power within DI is to isolate dependencies
        * The module root should not contain any dependencies.
          Ideally the module root should only contain types
          supported by your freestanding C standard, like C99
          or C11. This enables highly portable modules.
        * Implementation details and dependencies belong in the
          module derivations
    
    ## Implementing a Module: Derivations

    * Concrete implementations for a module are created
      through deriving the module.
    * A derivation is guranteed to have the module root as the
      the first member. Module derivations can always be safely
      up-cast to the module root.
    * The module derivations can contain implementation specific
      details, dependencies, and other private memebers.
    * You can think of the members placed within the derivation
      as "private" in C++ terms
    
    ## The Module
    Now with a background on the module root and module derivations,
    we can talk about the module.

    * The module is a union of all possible derivations, including
      the root.
    * The module is forward-declared. Module implementations will
      provide a default implementation of the module that can be
      overridden when `..._CUSTOM` is defined.
        * The user of the module would define `..._CUSTOM` if they
          want polymorphic behavior.
    * API traits are provided a pointer to the module.

    ### How to Define The Module Union

    Below is an example of defining a module union:
    ```c
    union MY_MODULE_T JUNO_MODULE(MY_MODULE_API_T, MY_MODULE_ROOT_T,
        MY_MODULE_DERIVATION_1_T;
        MY_MODULE_DERIVATION_2_T;
        MY_MODULE_DERIVATION_3_T;
    );
    ```
*/


/**
    Declare a juno module implemented as a union
    @name The name of the module type
*/
#define JUNO_MODULE_DECLARE(NAME_T)   typedef union NAME_T NAME_T
/**
    Declare a root implementation for the module implemented as a struct
    @param name The name of the root implementation type
*/
#define JUNO_MODULE_ROOT_DECLARE(NAME_T)   typedef struct NAME_T NAME_T
/**
    Declare a derivation of a juno module implemented as a struct
    @param name The name of the derived module type
*/
#define JUNO_MODULE_DERIVE_DECLARE(NAME_T)   JUNO_MODULE_ROOT_DECLARE(NAME_T)

/**
    Alias for the failure handler for a module
*/
#define JUNO_FAILURE_HANDLER    _pfcnFailureHandler
/**
    Alias for the failure handler user data for a module
*/
#define JUNO_FAILURE_USER_DATA    _pvFailurUserData
/**
    Empty macro that indicates a module
    implementation has no additional members 
*/
#define JUNO_MODULE_EMPTY
/**
    Alias for the module root implementation.
    All modules can call `.tRoot` to access the
    module root.
*/
#define JUNO_MODULE_SUPER   tRoot

/**
    Define a juno module. This needs to be done in
    the composition root. This is where users define
    all possible module implementations for a module.

    Example:
    ```
        union MY_MODULE_T JUNO_MODULE(MY_MODULE_API_T, MY_MODULE_ROOT_T,
            MY_MODULE_DERIVATION_1_T;
            MY_MODULE_DERIVATION_2_T;
            MY_MODULE_DERIVATION_3_T;
        );
    ```
    @param name The name of the module as declared
    @param API The name of the API type for the module
    @param root The name of the root implementation type
    for the module as declared
    @param derived The derived modules seperated by `;`

*/
#define JUNO_MODULE(API_T, ROOT_T, derived) \
{ \
    const API_T *ptApi; \
    ROOT_T JUNO_MODULE_SUPER; \
    derived \
}

/**
    Implement a root for a module
    Example:
    ```
        struct MY_MODULE_ROOT_T JUNO_MODULE(MY_MODULE_API_T,
            uint32_t iExampleRoot1Member;
            uint32_t iExampleRoot2Member;
            uint32_t iExampleRoot3Member;
        );
    ```
    @param name The name of the module root implementation as declared
    @param API The API type for the module
    @param members The member components of the module root implementation
*/
#define JUNO_MODULE_ROOT(API_T, members) \
{ \
    const API_T *ptApi; \
    members \
    JUNO_FAILURE_HANDLER_T JUNO_FAILURE_HANDLER; \
    JUNO_USER_DATA_T *JUNO_FAILURE_USER_DATA; \
}

/**
    Implement a derivation of a module
    Example:
    ```
        struct MY_MODULE_DERIVED_T JUNO_MODULE(MY_MODULE_ROOT_T,
            int iExampleSpecificMember;
            SOME_DEPENEDENCY_T tSomeDep;
        );
    ```
    @param name The name of the module derivation as declared
    @param root The name of the root implementation for the module as declared
    @param members The member components of the module derivation
*/
#define JUNO_MODULE_DERIVE(ROOT_T, members) \
{ \
    ROOT_T JUNO_MODULE_SUPER; \
    members \
}

/**
    Get the API pointer from the module
    @param ptModule The module pointer
    @param MODULE_ROOT_NAME The root type of the module
*/
#define JUNO_MODULE_GET_API(ptModule, ROOT_T) ((const ROOT_T *)ptModule)->ptApi


/**
 * @def JUNO_MODULE_RESULT(NAME_T, SUCCESS_T)
 * @brief Defines a result type combining a status and a success payload.
 * @param NAME_T Name of the result struct to define.
 * @param SUCCESS_T Type of the success payload contained in the result.
 */
#define JUNO_MODULE_RESULT(NAME_T, SUCCESS_T) \
typedef struct NAME_T \
{ \
    JUNO_STATUS_T tStatus; \
    SUCCESS_T tSuccess; \
} NAME_T

/**
 * @def JUNO_MODULE_OPTION(NAME_T, SUCCESS_T)
 * @brief Defines an option type combining a flag to indicate some and a success payload.
 * @param NAME_T Name of the result struct to define.
 * @param SUCCESS_T Type of the success payload contained in the result.
 */
#define JUNO_MODULE_OPTION(NAME_T, SOME_T) \
typedef struct NAME_T \
{ \
    bool bIsSome; \
    SOME_T tSome; \
} NAME_T;

#endif // JUNO_MODULE_H
