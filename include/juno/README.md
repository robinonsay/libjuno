# Dependency Injection

In embedded systems, code often grows into tightly coupled, monolithic blocks where a change in one component ripples through the entire system. This makes maintenance and reuse difficult, especially as projects scale or requirements evolve. **LibJuno** offers a lightweight, C99-compatible approach to achieve modularity and dependency injection (DI) in embedded projects. By using LibJuno’s macros and a clear pattern for defining “modules” (essentially self-contained components with defined APIs), you can:

* **Decouple** implementation details from consumers.
* **Enforce** clear contracts (via function pointers in an API struct).
* **Inject** dependencies at initialization time instead of hard-coding them.
* **Isolate** changes so that swapping one implementation doesn’t force you to rework unrelated code.

In this tutorial, we’ll walk through:

1. The basics of LibJuno’s module-system macros.
2. How to define a module API (header) and its implementation.
3. How to compose multiple modules with dependency injection.
4. A concrete example: building a “Car” that depends on an “Engine”, which in turn depends on either a gas tank or a battery.
5. Variations that demonstrate isolated changes (e.g., swapping a V6 engine for a V8, or adding a new turbocharged engine) without affecting consumers.

We’ll use the example code provided in the “gastank\_api.h”, “engine\_api.h”, “battery\_api.h”, “car\_api.h” files, plus their implementations, to illustrate these concepts. By the end, you’ll see how a modular, DI-driven design can keep your embedded software clean, testable, and maintainable.

---

## 1. LibJuno’s Module System: An Overview

At its core, LibJuno provides a set of macros to declare:

* A **module base type** (the “base” struct that holds an API pointer, failure-handler, and user data).
* A **module type** (which may be a union of a base plus derived “substructures”).
* A way to **derive** a new module from an existing one (i.e., to create a subtype).
* A consistent pattern for **failure handling** in every module.

Below is a quick breakdown of the key macros (all defined in `juno/module.h`):

```c
#define JUNO_MODULE_DECLARE(name)        typedef union name##_TAG name
#define JUNO_MODULE_BASE_DECLARE(name)   typedef struct name##_TAG name
#define JUNO_MODULE_DERIVE_DECLARE(name) JUNO_MODULE_BASE_DECLARE(name)

#define JUNO_MODULE_SUPER   tBase
#define JUNO_MODULE(name, API, base, derived) \
union name##_TAG                                     \
{                                                    \
    base JUNO_MODULE_SUPER;                          \
    derived                                          \
}

#define JUNO_MODULE_BASE(name, API, members)        \
struct name##_TAG                                    \
{                                                    \
    const API *ptApi;                                \
    members                                          \
    JUNO_FAILURE_HANDLER_T JUNO_FAILURE_HANDLER;     \
    JUNO_USER_DATA_T *JUNO_FAILURE_USER_DATA;        \
}

#define JUNO_MODULE_DERIVE(name, base, members)     \
struct name##_TAG                                    \
{                                                    \
    base JUNO_MODULE_SUPER;                          \
    members                                          \
}
```

1. **`JUNO_MODULE_BASE(...)`**: Defines the *base* layout for a module. Every module will have:

   * A pointer to an API struct (`ptApi`) that contains function pointers.
   * Any “base members” needed (e.g., local state fields).
   * A failure handler function pointer (`JUNO_FAILURE_HANDLER`) and user data (`JUNO_FAILURE_USER_DATA`).

2. **`JUNO_MODULE(...)`**: Creates a *union* type where:

   * The first member is the base struct (aliased via `JUNO_MODULE_SUPER`).
   * The other members are “derived” sub-structs you specify. Because it’s a union, all derived sub-structures overlay the same memory as the base.

3. **`JUNO_MODULE_DERIVE(...)`**: Defines a *derived* struct that contains a copy of the base (aliased as `tBase`) plus any extra members needed by the derived type.

4. **Failure Handling Macros**:

   * Every module gets a `JUNO_FAILURE_HANDLER` and `JUNO_FAILURE_USER_DATA` in its base.
   * `JUNO_FAIL_MODULE(status, ptMod, msg)` invokes the module’s failure handler if it exists, passing along a custom message.

These macros enforce a consistent layout and pattern for modules across your entire system. Now let’s see how they’re used in a concrete API.

---

## 2. Defining a Simple Module API: The Gas Tank

Imagine we need a “gas tank” module that allows setting and getting fuel level. We create:

* **`gastank_api.h`**: Declares the `GASTANK_T` module and its API.
* **`gastank_impl.h` / `gastank_impl.c`**: (Not shown in full) provides the implementation for our default gas-tank.

### 2.1. gastank_api.h

```c
#ifndef GASTANK_API_H
#define GASTANK_API_H

#include "juno/status.h"
#include "juno/module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GASTANK_API_TAG GASTANK_API_T;

// Declare a `GASTANK_T` module type
JUNO_MODULE_DECLARE(GASTANK_T);
JUNO_MODULE_BASE_DECLARE(GASTANK_BASE_T);

// Define the _base_ of the GasTank module:
//  - `ptApi` will point to GASTANK_API_T
//  - `int iFuelLevel;` is our internal state
//  - `JUNO_FAILURE_HANDLER_T` + `JUNO_USER_DATA_T *` follow
JUNO_MODULE_BASE(
    GASTANK_BASE_T,
    GASTANK_API_T,
    int iFuelLevel;
);

struct GASTANK_API_TAG
{
    JUNO_STATUS_T (*SetFuel)(GASTANK_T *ptGastank, int iFuelLevel);
    JUNO_STATUS_T (*GetFuel)(GASTANK_T *ptGastank, int *piFuelLevel);
};

#ifdef __cplusplus
}
#endif
#endif // GASTANK_API_H
```

**Key points**:

* We use `JUNO_MODULE_DECLARE(GASTANK_T)` to create:

  ```c
  typedef union GASTANK_T_TAG GASTANK_T;
  ```

  which means `GASTANK_T` is an opaque union type (details come from the base or any derived form).
* We use `JUNO_MODULE_BASE(GASTANK_BASE_T, GASTANK_API_T, int iFuelLevel;)` to define:

  ```c
  struct GASTANK_BASE_T_TAG {
      const GASTANK_API_T *ptApi;
      int iFuelLevel;
      JUNO_FAILURE_HANDLER_T JUNO_FAILURE_HANDLER;
      JUNO_USER_DATA_T *JUNO_FAILURE_USER_DATA;
  };
  ```
* Finally, `GASTANK_API_T` is a struct with two function pointers, one to set fuel, one to get it.

### 2.2. gastank_impl.h

```c
#ifndef GASTANK_IMPL_H
#define GASTANK_IMPL_H

#include "juno/module.h"
#include "juno/status.h"
#include "gastank_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef GASTANK_DEFAULT
JUNO_MODULE(GASTANK_T, GASTANK_API_T, GASTANK_BASE_T,
    // No extra “derived” members here—just use the base
);
#endif

// This function initializes a default GasTank instance.
// The caller must pass a pointer to uninitialized `GASTANK_T`,
// a failure handler, and optional user data.
JUNO_STATUS_T Gastank_ImplApi(
    GASTANK_T *ptGastank,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData);

#ifdef __cplusplus
}
#endif
#endif // GASTANK_IMPL_H
```

Inside the `.c` file (not shown in detail here), you’d do something like:

```c
static const GASTANK_API_T tGastankImplApi = {
    .SetFuel = Gastank_SetFuel_Impl,
    .GetFuel = Gastank_GetFuel_Impl
};

JUNO_STATUS_T Gastank_ImplApi(
    GASTANK_T *ptGastank,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData)
{
    ASSERT_EXISTS(ptGastank);
    GASTANK_BASE_T *pBase = (GASTANK_BASE_T *)(ptGastank);

    // Assign the API pointer into the base
    pBase->ptApi = &tGastankImplApi;
    pBase->JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    pBase->JUNO_FAILURE_USER_DATA = pvFailureUserData;

    // Initialize default internal fuel level (e.g., 0 liters)
    pBase->iFuelLevel = 0;
    return JUNO_STATUS_SUCCESS;
}

// ... Implementations for SetFuel/GetFuel that read/write pBase->iFuelLevel, call JUNO_FAIL_MODULE if invalid, etc.
```

By the end of this, any user of `GASTANK_T` can do:

```c
GASTANK_T myTank;
Gastank_ImplApi(&myTank, MyFailureHandler, NULL);

myTank.ptApi->SetFuel(&myTank, 50);
int level;
myTank.ptApi->GetFuel(&myTank, &level);
```

All interactions go through `ptBase->ptApi->FunctionName`, which hides implementation details behind the API struct.

---

## 3. Building an Engine Module: Deriving & Injecting Dependencies

Now let’s build an **Engine** module that depends on a gas tank (for V6/V8 engines) or on a battery (for electric engines). We’ll see how LibJuno supports deriving from a “base engine” and injecting the appropriate sub-module.

### 3.1. engine_api.h: The Base Engine

```c
#ifndef ENGINE_API_H
#define ENGINE_API_H

#include "juno/status.h"
#include "juno/module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ENGINE_API_TAG ENGINE_API_T;

JUNO_MODULE_DECLARE(ENGINE_T);
JUNO_MODULE_BASE_DECLARE(ENGINE_BASE_T);

// The base engine holds its API pointer, plus a rotor-speed field:
JUNO_MODULE_BASE(
    ENGINE_BASE_T,
    ENGINE_API_T,
    int iRpm;
);

struct ENGINE_API_TAG
{
    /// Start the Engine
    JUNO_STATUS_T (*Start)(ENGINE_T *ptEngine);
    /// Set the RPM of the engine
    JUNO_STATUS_T (*SetRPM)(ENGINE_T *ptEngine, int iRpm);
    /// Get the fuel-level of the engine
    JUNO_STATUS_T (*GetFuel)(ENGINE_T *ptEngine, int *piFuel);
    /// Stop the engine
    JUNO_STATUS_T (*Stop)(ENGINE_T *ptEngine);
};

#ifdef __cplusplus
}
#endif
#endif // ENGINE_API_H
```

#### What This Means

* `ENGINE_T` is an opaque union defined via `JUNO_MODULE_DECLARE`.
* `ENGINE_BASE_T` has:

  * `const ENGINE_API_T *ptApi;`
  * `int iRpm;`
  * A failure handler pointer and user data.
* `ENGINE_API_T` is the set of function pointers that every engine implementation must provide, e.g. `Start()`, `SetRPM()`, `GetFuel()`, `Stop()`.

### 3.2. Deriving a Gas-Powered Engine: engine_v6.h & engine_v8.h

For a gas engine, we need to store a pointer to a `GASTANK_T` so that when someone calls `GetFuel()`, we’ll delegate to the gas tank. We do this by “deriving” from `ENGINE_BASE_T`.

Example: **`engine_v6.h`**

```c
#ifndef ENGINE_V6_H
#define ENGINE_V6_H

#include "juno/module.h"
#include "juno/status.h"
#include "engine_api.h"
#include "gastank_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// Declare a new derived module type: ENGINE_V6_T
JUNO_MODULE_DERIVE_DECLARE(ENGINE_V6_T);

// Derive from ENGINE_BASE_T and add a GASTANK_T* member
JUNO_MODULE_DERIVE(
    ENGINE_V6_T,
    ENGINE_BASE_T,
    GASTANK_T *ptGastank;
);

#ifdef ENGINE_DEFAULT
/**
    This is the “default” v6 implementation for ENGINE_T.
    When you do:
      JUNO_MODULE(ENGINE_t, ENGINE_API_T, ENGINE_BASE_T, ENGINE_V6_T tEngineV6;)
    the union will overlay ENGINE_BASE_T (as tBase) with ENGINE_V6_T (with a ptGastank).
*/
JUNO_MODULE(
    ENGINE_T,
    ENGINE_API_T,
    ENGINE_BASE_T,
    ENGINE_V6_T tEngineV6;
);
#endif

// Initialization function: takes the un‐initialized ENGINE_T,
// a pointer to an existing GASTANK_T module, a failure handler, and user data.
JUNO_STATUS_T Engine_V6Api(
    ENGINE_T *ptEngine,
    GASTANK_T *ptGastank,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
);

#ifdef __cplusplus
}
#endif
#endif // ENGINE_V6_H
```

Similarly, **`engine_v8.h`** is identical except its type is `ENGINE_V8_T`. Both store `GASTANK_T *ptGastank` as their dependency.

#### Implementation Sketch (in engine_v6.c)

```c
static const ENGINE_API_T tEngineV6ImplApi = {
    .Start = EngineV6_Start_Impl,
    .SetRPM = EngineV6_SetRPM_Impl,
    .GetFuel = EngineV6_GetFuel_Impl,
    .Stop = EngineV6_Stop_Impl
};

JUNO_STATUS_T Engine_V6Api(
    ENGINE_T *ptEngine,
    GASTANK_T *ptGastank,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
) {
    ASSERT_EXISTS(ptEngine);
    ENGINE_V6_T *pV6 = (ENGINE_V6_T *)(ptEngine);

    // Assign the API pointer, failure handler, and user data
    pV6->JUNO_MODULE_SUPER.ptApi = &tEngineV6ImplApi;
    pV6->JUNO_MODULE_SUPER.JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    pV6->JUNO_MODULE_SUPER.JUNO_FAILURE_USER_DATA = pvFailureUserData;

    // Store our dependency
    pV6->ptGastank = ptGastank;

    // Optionally initialize iRpm = 0, etc.
    pBase->iRpm = 0;

    return JUNO_STATUS_SUCCESS;
}

// In EngineV6_GetFuel_Impl, you’d do something like:
//   Retrieve `GASTANK_T *tank = pV6->ptGastank;`
//   Return tank->ptApi->GetFuel(tank, piFuelLevel);
//   or JUNO_FAIL_MODULE if `ptGastank` is NULL, etc.
```

Notice how `ENGINE_T` is a union overlaying:

```c
union ENGINE_T_TAG {
    ENGINE_BASE_T tBase;
    ENGINE_V6_T tEngineV6;
};
```

* When you call `Engine_V6Api(&myEngine, &myTank, handler, NULL)`, you choose the “V6” variant, which means `ptEngine->ptApi` now points to `tEngineV6ImplApi`, and `ptEngine->tEngineV6.ptGastank` holds the injected `GASTANK_T *`.

A consumer only ever sees `ENGINE_T *ptEngine`, but under the hood, the memory contains either an `ENGINE_V6_T` or `ENGINE_V8_T` (depending on how you initialize it). The `ptApi` function pointers implement `GetFuel()` by delegating to `ptGastank`.

### 3.3. An Electric Engine: engine_electric.h

```c
#ifndef ENGINE_ELECTRIC_H
#define ENGINE_ELECTRIC_H

#include "juno/module.h"
#include "juno/status.h"
#include "engine_api.h"
#include "battery_api.h"

#ifdef __cplusplus
extern "C" {
#endif

JUNO_MODULE_DERIVE_DECLARE(ENGINE_ELECTRIC_T);

JUNO_MODULE_DERIVE(
    ENGINE_ELECTRIC_T,
    ENGINE_BASE_T,
    BATTERY_T *ptBattery;
);

#ifdef ENGINE_DEFAULT
/**
    Default “electric” engine incarnation.
*/
JUNO_MODULE(
    ENGINE_T,
    ENGINE_API_T,
    ENGINE_BASE_T,
    ENGINE_ELECTRIC_T tEngineElectric;
);
#endif

// Initialization function takes an existing BATTERY_T*
JUNO_STATUS_T Engine_ElectricApi(
    ENGINE_T *ptEngine,
    BATTERY_T *ptBattery,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
);

#ifdef __cplusplus
}
#endif
#endif // ENGINE_ELECTRIC_H
```

Implementation notes:

* The API function pointers (in `tEngineElectricImplApi`) implement `GetFuel()` by calling `Battery_GetVoltage()` (or similarly named). They might return an error if the battery is not sufficiently charged.

---

## 4. Wiring Everything Together: The Car Module & main.c

Now let’s see how a **`Car`** module consumes an `ENGINE_T`, but doesn’t care whether it’s V6, V8, or Electric—this is classic polymorphism via DI.

### 4.1. car_api.h

```c
#ifndef CAR_API_H
#define CAR_API_H

#include "juno/status.h"
#include "juno/module.h"
#include "engine_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CAR_API_TAG CAR_API_T;

JUNO_MODULE_DECLARE(CAR_T);
JUNO_MODULE_BASE_DECLARE(CAR_BASE_T);

// The Car’s base has:
//  - ptApi: pointer to CAR_API_T
//  - ENGINE_T *ptEngine: our dependency
//  - int iSpeed: internal state
//  - plus failure handler & data
JUNO_MODULE_BASE(
    CAR_BASE_T,
    CAR_API_T,
    ENGINE_T *ptEngine;
    int iSpeed;
);

struct CAR_API_TAG
{
    // Make the car go at some speed
    JUNO_STATUS_T (*Go)(CAR_T *ptCar, int iSpeed);
    // Stop the car
    JUNO_STATUS_T (*Stop)(CAR_T *ptCar);
};

#ifdef __cplusplus
}
#endif
#endif // CAR_API_H
```

### 4.2. car_impl.h

```c
#ifndef CAR_IMPL_H
#define CAR_IMPL_H

#include "juno/module.h"
#include "juno/status.h"
#include "car_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CAR_DEFAULT
/**
    Default Car implementation uses only the base—no derived fields here.
*/
JUNO_MODULE(
    CAR_T,
    CAR_API_T,
    CAR_BASE_T,
    // No extra derived members
);
#endif

// Initialization: inject a pre-initialized ENGINE_T*
JUNO_STATUS_T Car_ImplApi(
    CAR_T *ptCar,
    ENGINE_T *ptEngine,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
);

#ifdef __cplusplus
}
#endif
#endif // CAR_IMPL_H
```

In the `.c`:

```c
static const CAR_API_T tCarImplApi = {
    .Go = Car_Go_Impl,
    .Stop = Car_Stop_Impl
};

JUNO_STATUS_T Car_ImplApi(
    CAR_T *ptCar,
    ENGINE_T *ptEngine,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
) {
    ASSERT_EXISTS(ptCar);

    CAR_BASE_T *pBase = (CAR_BASE_T *)(ptCar);
    pBase->ptApi = &tCarImplApi;
    pBase->JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    pBase->JUNO_FAILURE_USER_DATA = pvFailureUserData;

    // Inject the engine pointer
    pBase->ptEngine = ptEngine;
    pBase->iSpeed = 0;
    return JUNO_STATUS_SUCCESS;
}

JUNO_STATUS_T Car_Go_Impl(CAR_T *ptCar, int iSpeed) {
    CAR_BASE_T *pBase = (CAR_BASE_T *)(ptCar);
    if (!pBase->ptEngine) {
        JUNO_FAIL_MODULE(JUNO_STATUS_NULLPTR_ERROR, ptCar, "No engine injected");
        return JUNO_STATUS_NULLPTR_ERROR;
    }

    // Call engine->Start(), engine->SetRPM(), etc., based on speed
    // Example: convert “speed” to “rpm” (just as a placeholder)
    int targetRpm = iSpeed * 50;
    pBase->ptEngine->ptApi->Start(pBase->ptEngine);
    pBase->ptEngine->ptApi->SetRPM(pBase->ptEngine, targetRpm);

    // Store iSpeed internally
    pBase->iSpeed = iSpeed;
    return JUNO_STATUS_SUCCESS;
}

JUNO_STATUS_T Car_Stop_Impl(CAR_T *ptCar) {
    CAR_BASE_T *pBase = (CAR_BASE_T *)(ptCar);
    if (!pBase->ptEngine) {
        JUNO_FAIL_MODULE(JUNO_STATUS_NULLPTR_ERROR, ptCar, "No engine injected");
        return JUNO_STATUS_NULLPTR_ERROR;
    }

    pBase->ptEngine->ptApi->Stop(pBase->ptEngine);
    pBase->iSpeed = 0;
    return JUNO_STATUS_SUCCESS;
}
```

### 4.3. Putting It All Together: main.c

```c
#include "battery_api.h"
#include "engine_api.h"
#include "gastank_api.h"
#include "juno/status.h"
#include <stdio.h>

#define BATTERY_DEFAULT
#include "battery_impl.h"
#define GASTANK_DEFAULT
#include "gastank_impl.h"
#define CAR_DEFAULT
#include "car_impl.h"

#include "engine_electric.h"
#include "engine_v6.h"
#include "engine_v8.h"

// We also define a “mega-engine” union type, combining all three variants:
JUNO_MODULE(
    ENGINE_T,
    ENGINE_API_T,
    ENGINE_BASE_T, 
    ENGINE_ELECTRIC_T tElectric;
    ENGINE_V6_T tV6;
    ENGINE_V8_T tV8;
);

void JunoFailureHandler(JUNO_STATUS_T tStatus, const char *pcMessage, JUNO_USER_DATA_T *pvUserData) {
    printf("Failed: %s\n", pcMessage);
}

int main(void) {
    // 1) Create and initialize a Battery
    BATTERY_T tBattery = {};
    JUNO_STATUS_T tStatus = Battery_ImplApi(&tBattery, JunoFailureHandler, NULL);
    if(tStatus) return -1;

    // 2) Create and initialize a GasTank (we’ll use it for both V6 & V8, by copying)
    GASTANK_T tV6Gastank = {};
    tStatus = Gastank_ImplApi(&tV6Gastank, JunoFailureHandler, NULL);
    if(tStatus) return -1;
    // Copy tV6Gastank into tV8Gastank so each engine has its own state
    GASTANK_T tV8Gastank = tV6Gastank;

    // 3) Create & initialize each kind of ENGINE
    ENGINE_T tElectricEngine = {};
    tStatus = Engine_ElectricApi(&tElectricEngine, &tBattery, JunoFailureHandler, NULL);
    if(tStatus) return -1;

    ENGINE_T tV6Engine = {};
    tStatus = Engine_V6Api(&tV6Engine, &tV6Gastank, JunoFailureHandler, NULL);
    if(tStatus) return -1;

    ENGINE_T tV8Engine = {};
    tStatus = Engine_V8Api(&tV8Engine, &tV8Gastank, JunoFailureHandler, NULL);
    if(tStatus) return -1;

    // 4) Create & initialize three Cars, each with its own engine
    CAR_T tElectricCar = {};
    tStatus = Car_ImplApi(&tElectricCar, &tElectricEngine, JunoFailureHandler, NULL);
    if(tStatus) return -1;

    CAR_T tV6Car = {};
    tStatus = Car_ImplApi(&tV6Car, &tV6Engine, JunoFailureHandler, NULL);
    if(tStatus) return -1;

    CAR_T tV8Car = {};
    tStatus = Car_ImplApi(&tV8Car, &tV8Engine, JunoFailureHandler, NULL);
    if(tStatus) return -1;

    // 5) Drive each car at 100 mph
    tElectricCar.ptApi->Go(&tElectricCar, 100);
    tV6Car.ptApi->Go(&tV6Car, 100);
    tV8Car.ptApi->Go(&tV8Car, 100);

    // … do other operations …

    // 6) Stop each car
    tElectricCar.ptApi->Stop(&tElectricCar);
    tV6Car.ptApi->Stop(&tV6Car);
    tV8Car.ptApi->Stop(&tV8Car);

    return 0;
}
```

#### Why This Is Modular

1. **Each module has a well-defined API** (e.g., `ENGINE_API_T`, `GASTANK_API_T`, `BATTERY_API_T`, `CAR_API_T`). Consumers only call via `ptBase->ptApi->FunctionName`.
2. **Dependencies are injected** at initialization:

   * The **engine** doesn’t internally create its own gas tank or battery; instead, it’s handed a pointer to an existing `GASTANK_T` or `BATTERY_T`.
   * The **car** doesn’t know how to construct an engine; it’s passed an existing `ENGINE_T`.
3. **Swapping implementations is trivial**: if you want a different kind of engine (say, a turbo V6), you just add a new derived `TURBOV6_T` and call its initializer in `main()`.

Because modules communicate only through the API pointers, **no other code needs changing** when you introduce a new engine variant.

---

## 5. Demonstrating Isolation of Changes: Adding a Turbocharged Engine

Imagine you now want a **Turbo V6** variant, which still needs a gas tank but behaves differently in its `Start`/`SetRPM` logic. Here’s how you’d proceed:

### 5.1. Add engine_turbov6.h

```c
#ifndef ENGINE_TURBOV6_H
#define ENGINE_TURBOV6_H

#include "juno/module.h"
#include "juno/status.h"
#include "engine_api.h"
#include "gastank_api.h"

#ifdef __cplusplus
extern "C" {
#endif

JUNO_MODULE_DERIVE_DECLARE(ENGINE_TURBOV6_T);

JUNO_MODULE_DERIVE(
    ENGINE_TURBOV6_T,
    ENGINE_BASE_T,
    GASTANK_T *ptGastank;
    // Additional fields for turbo (e.g., int iBoostPressure;)
    int iBoostPressure;
);

#ifdef ENGINE_DEFAULT
JUNO_MODULE(
    ENGINE_T,
    ENGINE_API_T,
    ENGINE_BASE_T,
    ENGINE_TURBOV6_T tEngineTurboV6;
);
#endif

JUNO_STATUS_T Engine_TurboV6Api(
    ENGINE_T *ptEngine,
    GASTANK_T *ptGastank,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
);

#ifdef __cplusplus
}
#endif
#endif // ENGINE_TURBOV6_H
```

### 5.2. Implement engine_turbov6.c

```c
static const ENGINE_API_T tEngineTurboV6ImplApi = {
    .Start = EngineTurboV6_Start_Impl,
    .SetRPM = EngineTurboV6_SetRPM_Impl,
    .GetFuel = EngineTurboV6_GetFuel_Impl,
    .Stop = EngineTurboV6_Stop_Impl
};

JUNO_STATUS_T Engine_TurboV6Api(
    ENGINE_T *ptEngine,
    GASTANK_T *ptGastank,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
) {
    ASSERT_EXISTS(ptEngine);
    ENGINE_TURBOV6_T *pTurbo = &ptEngine->tEngineTurboV6;
    ENGINE_BASE_T *pBase = &pTurbo->tBase;

    // Set up API & failure handler
    pBase->ptApi = &tEngineTurboV6ImplApi;
    pBase->JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    pBase->JUNO_FAILURE_USER_DATA = pvFailureUserData;

    // Inject the gas tank
    pTurbo->ptGastank = ptGastank;

    // Initialize turbo-specific fields
    pBase->iRpm = 0;
    pTurbo->iBoostPressure = 0;

    return JUNO_STATUS_SUCCESS;
}

// Implementation details: e.g., when Start() is called, spin up the turbo, check boost, etc.
```

### 5.3. Modify main.c to Use Turbo V6

```c
#include "battery_api.h"
#include "engine_api.h"
#include "gastank_api.h"
#include "juno/status.h"
#include <stdio.h>

#define BATTERY_DEFAULT
#include "battery_impl.h"
#define GASTANK_DEFAULT
#include "gastank_impl.h"
#define CAR_DEFAULT
#include "car_impl.h"

#include "engine_electric.h"
#include "engine_v6.h"
#include "engine_v8.h"

// Include our new turbo-V6 header
#include "engine_turbov6.h"

JUNO_MODULE(ENGINE_T, ENGINE_API_T, ENGINE_BASE_T,
    ENGINE_ELECTRIC_T tElectric;
    ENGINE_V6_T tV6;
    ENGINE_V8_T tV8;
    ENGINE_TURBOV6_T tTurboV6;   // <-- add here
);

void JunoFailureHandler(JUNO_STATUS_T tStatus, const char *pcMessage, JUNO_USER_DATA_T *pvUserData) {
    printf("Failed: %s\n", pcMessage);
}

int main(void) {
    // ... (battery and gastank setup as before) ...

    GASTANK_T tTurboGastank = tV6Gastank; // Copy the same GASTANK initial state
    ENGINE_T tTurboV6Engine = {};
    tStatus = Engine_TurboV6Api(&tTurboV6Engine, &tTurboGastank, JunoFailureHandler, NULL);
    if(tStatus) return -1;

    CAR_T tTurboCar = {};
    tStatus = Car_ImplApi(&tTurboCar, &tTurboV6Engine, JunoFailureHandler, NULL);
    if(tStatus) return -1;

    // Now we can drive the Turbo V6 car:
    tTurboCar.ptApi->Go(&tTurboCar, 120); // e.g. 120 mph
    tTurboCar.ptApi->Stop(&tTurboCar);

    return 0;
}
```

#### Why This Is Completely Isolated

* You **never changed** `car_impl.h` or `car_impl.c`. The Car still only calls `ENGINE_API_T` function pointers.
* You **never changed** `engine_v6.h` or `engine_v8.h`; they remain valid and usable.
* By adding `engine_turbov6.h` and its implementation, plus a new union member in `JUNO_MODULE(ENGINE_t, ENGINE_API_T,…)`, you gain a brand-new engine type.
* Everything else—batteries, gas tanks, cars—remains untouched. DI guarantees that each module only holds references to abstract interfaces (`ENGINE_API_T`, `GASTANK_API_T`, etc.), so new implementations slot in seamlessly.

---

## 6. Injecting an Entirely Different Fuel Source: “Hybrid” Engines

What if you want to create a **Hybrid Engine** that can run on gas *or* battery, depending on load? You could derive from the base engine and inject *both* a gas tank and a battery. Example:

### 6.1. engine_hybrid.h

```c
#ifndef ENGINE_HYBRID_H
#define ENGINE_HYBRID_H

#include "juno/module.h"
#include "juno/status.h"
#include "engine_api.h"
#include "gastank_api.h"
#include "battery_api.h"

#ifdef __cplusplus
extern "C" {
#endif

JUNO_MODULE_DERIVE_DECLARE(ENGINE_HYBRID_T);

JUNO_MODULE_DERIVE(
    ENGINE_HYBRID_T,
    ENGINE_BASE_T,
    GASTANK_T *ptGastank;   // for high-load (gas)
    BATTERY_T *ptBattery;   // for low-load (electric)
    int iHybridMode;        // 0 = electric, 1 = gas
);

#ifdef ENGINE_DEFAULT
JUNO_MODULE(
    ENGINE_T,
    ENGINE_BASE_T,
    ENGINE_HYBRID_T tEngineHybrid;
);
#endif

JUNO_STATUS_T Engine_HybridApi(
    ENGINE_T *ptEngine,
    GASTANK_T *ptGastank,
    BATTERY_T *ptBattery,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
);

#ifdef __cplusplus
}
#endif
#endif // ENGINE_HYBRID_H
```

### 6.2. engine_hybrid.c

```c
static const ENGINE_API_T tEngineHybridImplApi = {
    .Start    = EngineHybrid_Start_Impl,
    .SetRPM   = EngineHybrid_SetRPM_Impl,
    .GetFuel  = EngineHybrid_GetFuel_Impl,
    .Stop     = EngineHybrid_Stop_Impl
};

JUNO_STATUS_T Engine_HybridApi(
    ENGINE_T *ptEngine,
    GASTANK_T *ptGastank,
    BATTERY_T *ptBattery,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    JUNO_USER_DATA_T *pvFailureUserData
) {
    ASSERT_EXISTS(ptEngine);
    ENGINE_HYBRID_T *pHy = &ptEngine->tEngineHybrid;
    ENGINE_BASE_T *pBase = &pHy->tBase;

    pBase->ptApi = &tEngineHybridImplApi;
    pBase->JUNO_FAILURE_HANDLER = pfcnFailureHandler;
    pBase->JUNO_FAILURE_USER_DATA = pvFailureUserData;

    pHy->ptGastank = ptGastank;
    pHy->ptBattery = ptBattery;
    pHy->iHybridMode = 0; // default to electric mode

    pBase->iRpm = 0;
    return JUNO_STATUS_SUCCESS;
}

/* In EngineHybrid_GetFuel_Impl:
   if (pHy->iHybridMode == 0) 
       return pHy->ptBattery->ptApi->GetVoltage(...);
   else 
       return pHy->ptGastank->ptApi->GetFuel(...);
*/
```

### 6.3. Using the Hybrid Engine in main.c

```c
#include "battery_api.h"
#include "engine_api.h"
#include "gastank_api.h"
#include "juno/status.h"
#include <stdio.h>

#define BATTERY_DEFAULT
#include "battery_impl.h"
#define GASTANK_DEFAULT
#include "gastank_impl.h"
#define CAR_DEFAULT
#include "car_impl.h"
#include "engine_hybrid.h"

JUNO_MODULE(ENGINE_T, ENGINE_API_T, ENGINE_BASE_T,
    ENGINE_HYBRID_T tEngineHybrid;
    // Optionally: other engine types...
);

void JunoFailureHandler(JUNO_STATUS_T tStatus, const char *pcMessage, JUNO_USER_DATA_T *pvUserData) {
    printf("Failed: %s\n", pcMessage);
}

int main(void) {
    // Initialize a GasTank & Battery (as before)
    GASTANK_T myTank = {};
    JUNO_STATUS_T tStatus = Gastank_ImplApi(&myTank, JunoFailureHandler, NULL);
    if(tStatus) return -1;

    BATTERY_T myBattery = {};
    tStatus = Battery_ImplApi(&myBattery, JunoFailureHandler, NULL);
    if(tStatus) return -1;

    // Initialize a Hybrid Engine with both dependencies
    ENGINE_T tHybridEngine = {};
    tStatus = Engine_HybridApi(&tHybridEngine, &myTank, &myBattery, JunoFailureHandler, NULL);
    if(tStatus) return -1;

    CAR_T tHybridCar = {};
    tStatus = Car_ImplApi(&tHybridCar, &tHybridEngine, JunoFailureHandler, NULL);
    if(tStatus) return -1;

    // Drive at 60 mph (electric mode)
    tHybridEngine.tEngineHybrid.iHybridMode = 0; 
    tHybridCar.ptApi->Go(&tHybridCar, 60);

    // Now switch to gas mode for rapid acceleration
    tHybridEngine.tEngineHybrid.iHybridMode = 1;
    tHybridCar.ptApi->Stop(&tHybridCar);
    tHybridCar.ptApi->Go(&tHybridCar, 120);

    tHybridCar.ptApi->Stop(&tHybridCar);
    return 0;
}
```

Because `Car_ImplApi` only cares about `ENGINE_T *` (and calls functions from `ENGINE_API_T`), it doesn’t have to know whether the engine is “hybrid,” “electric,” “V6,” etc. All that complexity is hidden behind the `ptApi` function pointers.

---

## 7. How to Write Your Own Modules in LibJuno
You can use the `scripts/create_lib.py` script to auto-generate new LibJuno
libraries and modules for you. The generated code will have `TODO` statements
with instructions on implementation.

## 8. Why Dependency Injection Matters in Embedded Systems

1. **Testability**

   * You can easily substitute a real hardware interface (e.g., a physical SPI peripheral) with a “mock” module for unit tests. Since dependencies are passed in, your logic never directly touches `#ifdef`‐guarded hardware registers.

2. **Separation of Concerns**

   * Each module owns its own state and logic. The Car doesn’t need to know how the Engine calculates RPM, and the Engine doesn’t care how the Car uses it.

3. **Runtime Flexibility**

   * In some systems, you might want to switch between `ENGINE_V6` and `ENGINE_V8` depending on configuration or sensor input. Since they share the same `ENGINE_API_T` interface, you could hold a pointer to `ENGINE_T` and, at run time, assign either variant.

4. **Easier Maintenance & Upgrades**

   * Imagine you discover a bug in `GetFuel` for your V6 engine. You fix it in `engine_v6.c`, recompile that module, and relink. You didn’t have to recompile or touch Car, or GasTank, or any other part of the system.

5. **Optimized Resource Usage**

   * Because each module is strictly ANSI C (no RTTI, no dynamic allocation by default), the memory layout is clear. You know exactly how big each `ENGINE_T` is (it’s the size of the largest derived struct in the union), and each base struct has predictable alignment. No hidden vtables, no surprises.

---

## 9. Best Practices & Tips

* **Always Check Return Codes**
  Every `Foo_ImplApi(&instance, …)` returns a `JUNO_STATUS_T`. If it fails, you should call your failure handler immediately or abort.

* **Implement Comprehensive Failure Handlers**
  Pass a meaningful `JUNO_FAILURE_HANDLER_T` so you can trace what went wrong in the field. The macros `JUNO_FAIL_MODULE()` let you attach custom messages.

* **Keep Derived Structs Small**
  Since each derived struct is overlaid on the base in a `union`, the size of a module is the maximum size of any derived variant. If one variant has a large buffer, that buffer consumes memory even if other variants don’t use it. Structure your code so that modules that seldom co-exist don’t share the same union, or keep large buffers separate.

* **Document Your API**
  In each `<module>_api.h`, comment what each function pointer does, what side effects it might have, and any limitations (e.g., “Start() must be called before SetRPM()”).

* **Use `ASSERT_EXISTS` Liberally**
  In each function implementation, call `ASSERT_EXISTS(ptModule);` at the top. This expands to check if `ptModule` is non‐NULL and if the module’s `ptApi` matches the expected API. It prevents accidental misuse.

* **Avoid Global Variables**
  Inject everything. Even if there’s “only one” gas tank, pass it as a parameter instead of reading from a global. You’ll thank yourself when portability or testability matters.

* **Group Related Modules**
  If you have multiple variations of the same abstraction (e.g., several engine types), keep them in the same directory, name headers consistently (e.g., `engine_v6.h`, `engine_v8.h`, `engine_hybrid.h`), and avoid circular dependencies.

* **Be Wary of Deep Inheritance**
  While LibJuno allows multiple levels of derivation (`JUNO_MODULE_DERIVE(Child, ParentBase, members)`), deep chains can become confusing. Prefer composition over inheritance when possible (e.g., have a `Turbocharger_T` module that wraps a base engine rather than deriving a new engine type).

---

## 10. Conclusion

By following the patterns in this tutorial—defining clear module APIs, implementing initializers that inject dependencies, and consistently using the base‐and‐derived macros—you can write embedded C code that is:

* **Modular**: Each component lives in its own directory, has its own API, and only interacts through function pointers.
* **Flexible**: Swapping or upgrading one component doesn’t break or force changes in its consumers.
* **Testable**: You can inject fake or stub implementations for hardware interfaces or algorithms.
* **Maintainable**: Bugs or performance tweaks are isolated to the module in question.

LibJuno brings these benefits to C99 code without requiring C++ or dynamic allocation. Whether you’re building a simple control loop or a complex automotive system with dozens of possible configurations, using DI in a manner similar to this tutorial helps keep your codebase clear, robust, and future-proof.

Feel free to adapt the examples—add a new “Turbo + Electric” engine, introduce a “Diagnostics” module that periodically queries each submodule, or build a “Factory” that constructs entire subsystems at runtime based on configuration. As long as you keep each module’s responsibilities confined to its own API and inject dependencies at init time, you’ll maintain the modularity and maintainable structure you set out to achieve. Happy coding!
