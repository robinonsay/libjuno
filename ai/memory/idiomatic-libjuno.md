# Idiomatic LibJuno — Reference for AI Agents

*Read this file before implementing or reviewing any LibJuno C code.*
*All examples are derived from authoritative PM-authored source files.*

---

## 1. Module System: Root / Derivation / Union / Vtable

LibJuno's dependency-injection model has four cooperating pieces: the **root**,
the **derivation**, the **union**, and the **vtable (API struct)**. Together they
replicate the Rust trait + concrete-impl idiom in C11 with zero dynamic allocation.

### 1.1 `JUNO_MODULE_ROOT(API_T, ...)` — the root struct body

`JUNO_MODULE_ROOT` expands to a struct body (braces included). It injects three
mandatory members plus any extra cross-platform fields you supply:

```c
// Expansion of JUNO_MODULE_ROOT(API_T, ...extra...):
// (shows what module.h writes as member identifiers, before preprocessing)
{
    const API_T             *ptApi;                  // vtable pointer
    JUNO_FAILURE_HANDLER_T   JUNO_FAILURE_HANDLER;   // diagnostic callback
    JUNO_USER_DATA_T        *JUNO_FAILURE_USER_DATA; // opaque user ptr
    /* ...extra... */
}
```

> **Note on member-name aliases:** `JUNO_FAILURE_HANDLER` and `JUNO_FAILURE_USER_DATA` are
> `#define`d in `module.h` to `_pfcnFailureHandler` and `_pvFailureUserData` respectively.
> After preprocessing the struct fields are actually named `_pfcnFailureHandler` and
> `_pvFailureUserData`. Both forms compile identically — `ptRoot->JUNO_FAILURE_HANDLER`
> and `ptRoot->_pfcnFailureHandler` access the same field. This document uses the
> **source-level alias form** (`JUNO_FAILURE_HANDLER` / `JUNO_FAILURE_USER_DATA`) as
> the canonical style, matching the engine_app.c gold standard.

Usage — declare the struct tag, then assign the body in one statement:

```c
// From app_api.h — root with no extra fields
typedef struct JUNO_APP_ROOT_TAG JUNO_APP_ROOT_T;
struct JUNO_APP_ROOT_TAG JUNO_MODULE_ROOT(JUNO_APP_API_T, JUNO_MODULE_EMPTY);

// From thread_api.h — root with cooperative-shutdown flag only
typedef struct JUNO_THREAD_ROOT_TAG JUNO_THREAD_ROOT_T;
struct JUNO_THREAD_ROOT_TAG JUNO_MODULE_ROOT(JUNO_THREAD_API_T,
    volatile bool bStop;       // cooperative-shutdown flag
);
// OS handle lives in the platform derivation — see thread_linux.h
```

**Rule:** Every field inside `JUNO_MODULE_ROOT(...)` beyond the injected triple
MUST be freestanding-compatible (no POSIX types, no `pthread_t`, no `int fd`).
OS-specific fields belong exclusively in derivations (see section 2).

### 1.2 `JUNO_MODULE_DERIVE(ROOT_T, ...)` — a concrete derivation

`JUNO_MODULE_DERIVE` expands to a struct body embedding the root as the **first
member** (`tRoot`, aliased by `JUNO_MODULE_SUPER`). This guarantees that a pointer
to any derivation can be safely up-cast to the root type.

```c
// Expansion:
{
    ROOT_T tRoot;   // always first; JUNO_MODULE_SUPER is an alias for tRoot
    /* ...extra... */
}
```

Usage:

```c
// From engine_app.h — app derivation with injected deps and owned state
typedef struct ENGINE_APP_TAG ENGINE_APP_T;
struct ENGINE_APP_TAG JUNO_MODULE_DERIVE(JUNO_APP_ROOT_T,
    const JUNO_LOG_ROOT_T    *ptLogger;
    const JUNO_TIME_ROOT_T   *ptTime;
    JUNO_SB_BROKER_ROOT_T    *ptBroker;
    ENGINE_CMD_MSG_T          ptArrCmdBuffer[ENGINE_CMD_MSG_PIPE_DEPTH];
    ENGINE_CMD_MSG_ARRAY_T    tCmdArray;
    JUNO_SB_PIPE_T            tCmdPipe;
    float                     fCurrentRpm;
);
```

**Critical syntax note (from lessons learned):** `JUNO_MODULE_DERIVE` expands to
a struct body, NOT a complete statement. The typedef+struct tag must appear before
the macro:

```c
// CORRECT
typedef struct FOO_TAG FOO_T;
struct FOO_TAG JUNO_MODULE_DERIVE(BAR_ROOT_T,
    int iField;
);

// WRONG — do not write:
struct FOO_TAG JUNO_MODULE_DERIVE(BAR_ROOT_T, int iField;);   // missing typedef
struct FOO_TAG { JUNO_MODULE_DERIVE(...); };                  // extra braces
```

The safe up-cast from root to derivation inside a vtable callback:

```c
static JUNO_STATUS_T OnStart(JUNO_APP_ROOT_T *ptJunoApp)
{
    // Cast from root pointer to concrete type — safe because tRoot is first member
    ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp);
    ...
}
```

### 1.3 `JUNO_MODULE(API_T, ROOT_T, ...)` — the union

The module union is the type callers allocate. It holds the root and all derivations
as overlapping union members, ensuring storage is large enough for any concrete
implementation.

```c
// From udp_api.h
union JUNO_UDP_TAG JUNO_MODULE(JUNO_UDP_API_T, JUNO_UDP_ROOT_T,
    JUNO_UDP_LINUX_T tLinux;
);
typedef union JUNO_UDP_TAG JUNO_UDP_T;
```

The expansion is:

```c
union JUNO_UDP_TAG
{
    JUNO_UDP_ROOT_T  tRoot;    // always present; JUNO_MODULE_SUPER alias
    JUNO_UDP_LINUX_T tLinux;
};
```

Caller usage — always allocate the union, pass `&t<Module>.tRoot` to `Init` and
all subsequent API calls:

```c
// From main.c
static JUNO_UDP_T tUdp = {0};
JunoUdp_Init(&tUdp.tRoot, &g_junoUdpLinuxApi, FailureHandler, NULL);
tUdp.tRoot.ptApi->Open(&tUdp.tRoot, &tCfg);
```

### 1.4 `JUNO_MODULE_EMPTY` — no extra root members

When a root has no additional members beyond the injected triple, pass
`JUNO_MODULE_EMPTY` (expands to nothing):

```c
// From app_api.h
struct JUNO_APP_ROOT_TAG JUNO_MODULE_ROOT(JUNO_APP_API_T, JUNO_MODULE_EMPTY);
```

### 1.5 `JUNO_MODULE_SUPER` / `tRoot` — accessing the embedded root

`JUNO_MODULE_SUPER` is a macro alias for the literal identifier `tRoot`. Both forms
are identical. Prefer `tRoot` in code for clarity; `JUNO_MODULE_SUPER` appears in
macro definitions:

```c
// In a derivation, access root members via .tRoot
ptEngineApp->tRoot.ptApi = &tEngineAppApi;
ptEngineApp->tRoot.JUNO_FAILURE_HANDLER = pfcnFailureHandler;   // alias form
// Equivalent expanded form (both compile identically):
// ptEngineApp->tRoot._pfcnFailureHandler = pfcnFailureHandler;

// In the union, the root member is also named tRoot
ptAppList[i] = &tEngineApp.tRoot;   // from main.c
```

---

## 2. Two-Header Pattern for Platform Modules

Platform-dependent modules split across two headers:

### 2.1 `*_api.h` — freestanding interface

Contains ONLY freestanding-compatible declarations:
- Forward `typedef` declarations
- Root struct (`JUNO_MODULE_ROOT`) with freestanding-only extra fields
- Vtable struct (`struct MODULE_API_TAG { ... }`)
- Module union (`union MODULE_TAG JUNO_MODULE(...)`)
- `Init` function declaration (generic, takes `const API_T *ptApi`)
- NO POSIX headers (`<pthread.h>`, `<sys/socket.h>`, `<unistd.h>`, etc.)

Example from `udp_api.h`:

```c
#include <stdint.h>
#include <stdbool.h>
#include "juno/status.h"
#include "juno/module.h"
// NO <sys/socket.h>, NO <netinet/in.h>

// _iSockFd belongs in the Linux derivation (udp_linux.h), not in the root
struct JUNO_UDP_ROOT_TAG JUNO_MODULE_ROOT(JUNO_UDP_API_T, JUNO_MODULE_EMPTY);

struct JUNO_UDP_API_TAG
{
    JUNO_STATUS_T (*Open)   (JUNO_UDP_ROOT_T *ptRoot, const JUNO_UDP_CFG_T *ptCfg);
    JUNO_STATUS_T (*Send)   (JUNO_UDP_ROOT_T *ptRoot, const UDP_THREAD_MSG_T *ptMsg);
    JUNO_STATUS_T (*Receive)(JUNO_UDP_ROOT_T *ptRoot, UDP_THREAD_MSG_T *ptMsg);
    JUNO_STATUS_T (*Close)  (JUNO_UDP_ROOT_T *ptRoot);
};

union JUNO_UDP_TAG JUNO_MODULE(JUNO_UDP_API_T, JUNO_UDP_ROOT_T,
    JUNO_UDP_LINUX_T tLinux;
);
typedef union JUNO_UDP_TAG JUNO_UDP_T;

JUNO_STATUS_T JunoUdp_Init(
    JUNO_UDP_ROOT_T       *ptRoot,
    const JUNO_UDP_API_T  *ptApi,
    JUNO_FAILURE_HANDLER_T pfcnFailureHandler,
    void                  *pvFailureUserData
);
```

### 2.2 Platform-specific header or TU — OS-specific derivation

OS-specific fields (`pthread_t`, socket fd, etc.) that CANNOT be placed in a
freestanding root belong in the derivation struct. In the UDP example the
`_iSockFd` is `intptr_t` (freestanding), so the Linux derivation needs no
additional fields — it uses `JUNO_MODULE_EMPTY`:

```c
// From udp_api.h — Linux derivation with no extra fields
struct JUNO_UDP_LINUX_TAG JUNO_MODULE_DERIVE(JUNO_UDP_ROOT_T, JUNO_MODULE_EMPTY);
typedef struct JUNO_UDP_LINUX_TAG JUNO_UDP_LINUX_T;
```

If the OS handle cannot be expressed as a freestanding type (e.g., raw `pthread_t`),
place it in a derivation that lives in a platform-specific header (not included by
freestanding code):

```c
// Hypothetical linux-specific header (not freestanding)
#include <pthread.h>
struct JUNO_THREAD_LINUX_TAG JUNO_MODULE_DERIVE(JUNO_THREAD_ROOT_T,
    pthread_t _tNativeHandle;
);
```

**Key rule:** OS-specific fields belong in derivations, NOT in the root. The root carries
only freestanding-compatible state that all implementations share (e.g., `volatile bool bStop`
for cooperative shutdown). An implementation that doesn't use a handle or file-descriptor
system should not be forced to carry one.

---

## 3. RAII Lifecycle for Platform Modules

For platform modules (UDP, Thread, etc.) the resource lifecycle is:

1. **`Init`** — wires vtable, stores failure handler, sets sentinel values (e.g.,
   `_iSockFd = -1`, `_uHandle = 0`). Does NOT open the OS resource.
2. **Vtable `Open` / `Create`** — opens the OS resource (socket, thread).
3. **Vtable `Close` / `Stop` + `Join`** — releases the OS resource.

From `udp_api.h` documentation:

```
JunoUdp_Init(&tUdp.tRoot, &g_junoUdpLinuxApi, NULL, NULL);  // wire vtable
tUdp.tRoot.ptApi->Open(&tUdp.tRoot, &tCfg);                 // acquire resource
// ... use ...
tUdp.tRoot.ptApi->Close(&tUdp.tRoot);                        // release resource
```

From `thread_linux.h` documentation (RAII — LinuxInit wires vtable AND spawns thread in one call):

```c
JunoThread_LinuxInit(&tThread, MyEntryFunction, &tThread.tRoot, NULL, NULL);
// ... thread runs, reads tThread.tRoot.bStop ...
tThread.tRoot.ptApi->Stop(&tThread.tRoot);   // sets bStop = true
tThread.tRoot.ptApi->Join(&tThread.tRoot);   // blocks until thread exits
```

**Anti-pattern:** Do NOT put `Open`/`Create` inside `Init`. Init only wires
metadata (vtable, sentinel state). Resource acquisition is the vtable's job.

---

## 4. Vtable Design — Capabilities Only ("Rust Trait")

The vtable expresses WHAT a module CAN DO, not HOW it is created or destroyed from
a platform perspective. Design function pointers around generic capabilities:

| Module | Vtable capabilities |
|--------|---------------------|
| UDP    | `Open`, `Send`, `Receive`, `Close` |
| Thread | `Create`, `Stop`, `Join` |
| App    | `OnStart`, `OnProcess`, `OnExit` |
| Broker | `Publish`, `RegisterSubscriber` |
| Logger | `LogDebug`, `LogInfo`, `LogWarning`, `LogError` |

All vtable function pointers take the module root as their first argument and
return `JUNO_STATUS_T`:

```c
struct JUNO_APP_API_TAG
{
    JUNO_STATUS_T (*OnStart)  (JUNO_APP_ROOT_T *ptJunoApp);
    JUNO_STATUS_T (*OnProcess)(JUNO_APP_ROOT_T *ptJunoApp);
    JUNO_STATUS_T (*OnExit)   (JUNO_APP_ROOT_T *ptJunoApp);
};
```

The vtable is `static const` inside the POSIX translation unit and is wired
automatically by the platform init function (e.g., `JunoUdp_LinuxInit`).
Callers never reference it by name.

---

## 5. Application Pattern — The Gold Standard (`engine_app.c`)

### 5.1 Header (`engine_app.h`) — what to declare

The header declares:
1. The concrete struct via `JUNO_MODULE_DERIVE(JUNO_APP_ROOT_T, ...)`.
2. The `Init` function.
3. NOTHING ELSE — no lifecycle function declarations, no vtable extern.

```c
// engine_app.h (condensed)
typedef struct ENGINE_APP_TAG ENGINE_APP_T;
struct ENGINE_APP_TAG JUNO_MODULE_DERIVE(JUNO_APP_ROOT_T,
    const JUNO_LOG_ROOT_T    *ptLogger;
    const JUNO_TIME_ROOT_T   *ptTime;
    JUNO_SB_BROKER_ROOT_T    *ptBroker;
    ENGINE_CMD_MSG_T          ptArrCmdBuffer[ENGINE_CMD_MSG_PIPE_DEPTH];
    ENGINE_CMD_MSG_ARRAY_T    tCmdArray;
    JUNO_SB_PIPE_T            tCmdPipe;
    float                     fCurrentRpm;
);

JUNO_STATUS_T EngineApp_Init(
    ENGINE_APP_T             *ptEngineApp,
    const JUNO_LOG_ROOT_T    *ptLogger,
    const JUNO_TIME_ROOT_T   *ptTime,
    JUNO_SB_BROKER_ROOT_T    *ptBroker,
    JUNO_FAILURE_HANDLER_T    pfcnFailureHandler,
    JUNO_USER_DATA_T         *pvUserData
);
```

Forbidden in the header:
- `extern const JUNO_APP_API_T tEngineAppApi;` — the vtable is internal to the `.c` file.
- `JUNO_STATUS_T OnStart(...)` — lifecycle functions are `static` in the `.c` file.
- `_pfcnFailureHandler` / `_pvFailureUserData` fields inside the derivation — these
  already exist in `JUNO_APP_ROOT_T` via `JUNO_MODULE_ROOT`.

### 5.2 Source (`engine_app.c`) — canonical structure

**Step 1: Forward-declare static lifecycle functions and `Verify`.**

```c
static inline JUNO_STATUS_T Verify(JUNO_APP_ROOT_T *ptJunoApp);
static JUNO_STATUS_T OnStart(JUNO_APP_ROOT_T *ptJunoApp);
static JUNO_STATUS_T OnProcess(JUNO_APP_ROOT_T *ptJunoApp);
static JUNO_STATUS_T OnExit(JUNO_APP_ROOT_T *ptJunoApp);
```

**Step 2: Define the vtable as a `static const` inside the `.c` file.**

```c
static const JUNO_APP_API_T tEngineAppApi = {
    .OnStart  = OnStart,
    .OnProcess = OnProcess,
    .OnExit   = OnExit
};
```

This vtable is `static` — invisible outside the translation unit. Callers never
reference it by name.

**Step 3: Implement `Verify`.**

`Verify` receives the root pointer (as the vtable signature demands), performs
three checks in order, and returns a status:

```c
static inline JUNO_STATUS_T Verify(JUNO_APP_ROOT_T *ptJunoApp)
{
    // (a) Root pointer non-null
    JUNO_ASSERT_EXISTS(ptJunoApp);

    // (b) Cast to concrete type; assert all dependencies non-null
    ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp);
    JUNO_ASSERT_EXISTS_MODULE(
        ptEngineApp &&
        ptEngineApp->tRoot.ptApi &&
        ptEngineApp->ptLogger &&
        ptEngineApp->ptTime &&
        ptEngineApp->ptBroker,
        ptEngineApp,
        "Module does not have all dependencies"
    );

    // (c) Assert the vtable pointer matches the internal static vtable
    if (ptEngineApp->tRoot.ptApi != &tEngineAppApi)
    {
        JUNO_FAIL_MODULE(JUNO_STATUS_INVALID_TYPE_ERROR, ptEngineApp,
                         "Module has invalid API");
        return JUNO_STATUS_INVALID_TYPE_ERROR;
    }
    return JUNO_STATUS_SUCCESS;
}
```

The vtable-identity check (`ptApi == &tEngineAppApi`) is the mechanism that
prevents accidentally passing a different module type through a `JUNO_APP_ROOT_T *`.

**Step 4: Implement `Init`.**

`Init` receives the concrete type directly (not the root). It:
1. Guards the module pointer.
2. Casts to the concrete type.
3. Wires `ptApi` to the internal static vtable.
4. Stores `JUNO_FAILURE_HANDLER` and `JUNO_FAILURE_USER_DATA` into the root.
5. Stores all injected dependencies.
6. Calls `Verify` to confirm everything is wired.

```c
JUNO_STATUS_T EngineApp_Init(
    ENGINE_APP_T             *ptJunoApp,
    const JUNO_LOG_ROOT_T    *ptLogger,
    const JUNO_TIME_ROOT_T   *ptTime,
    JUNO_SB_BROKER_ROOT_T    *ptBroker,
    JUNO_FAILURE_HANDLER_T    pfcnFailureHandler,
    JUNO_USER_DATA_T         *pvFailureUserData
)
{
    JUNO_ASSERT_EXISTS(ptJunoApp);
    ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp);
    ptEngineApp->tRoot.ptApi              = &tEngineAppApi;
    ptEngineApp->tRoot.JUNO_FAILURE_HANDLER      = pfcnFailureHandler;
    ptEngineApp->tRoot.JUNO_FAILURE_USER_DATA    = pvFailureUserData;
    ptEngineApp->ptLogger = ptLogger;
    ptEngineApp->ptTime   = ptTime;
    ptEngineApp->ptBroker = ptBroker;
    JUNO_STATUS_T tStatus = Verify(&ptJunoApp->tRoot);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}
```

**Step 5: Every lifecycle function calls `Verify` first.**

```c
static JUNO_STATUS_T OnStart(JUNO_APP_ROOT_T *ptJunoApp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    tStatus = Verify(ptJunoApp);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus)
    ENGINE_APP_T *ptEngineApp = (ENGINE_APP_T *)(ptJunoApp);
    // ... implementation ...
    return tStatus;
}
```

Pattern: `Verify` → cast → access dependencies via local convenience pointers →
use `JUNO_ASSERT_SUCCESS` to propagate errors → return.

**Step 6: Accessing the failure handler in lifecycle functions.**

Because `JUNO_FAILURE_HANDLER` lives in the root, access it through the root member
(NOT the derivation):

```c
// In OnStart — the pointer passed is JUNO_APP_ROOT_T *ptJunoApp
tStatus = JunoSb_PipeInit(
    &ptEngineApp->tCmdPipe,
    ENGINE_CMD_MSG_MID,
    &ptEngineApp->tCmdArray.tRoot,
    ptJunoApp->JUNO_FAILURE_HANDLER,     // alias for _pfcnFailureHandler; from root
    ptJunoApp->JUNO_FAILURE_USER_DATA    // alias for _pvFailureUserData; from root
);
```

### 5.3 Composition root (`main.c`) — caller perspective

```c
// Allocate all modules as static (or stack) storage — no malloc
static JUNO_TIME_ROOT_T       tTime           = {0};
static JUNO_LOG_ROOT_T        tLogger         = {0};
static JUNO_SB_BROKER_ROOT_T  tBroker         = {0};
static ENGINE_APP_T           tEngineApp      = {0};

// Initialize infrastructure first
JUNO_STATUS_T tStatus;
tStatus = JunoTime_TimeInit(&tTime, &gtTimeApi, FailureHandler, NULL);
JUNO_ASSERT_SUCCESS(tStatus, return -1);

// Initialize application — dependencies passed as pointers
tStatus = EngineApp_Init(&tEngineApp, &tLogger, &tTime, &tBroker,
                         FailureHandler, NULL);
JUNO_ASSERT_SUCCESS(tStatus, return -1);

// Dispatch via vtable through the root
JUNO_APP_ROOT_T *ptAppList[] = { &tEngineApp.tRoot };
ptAppList[0]->ptApi->OnStart(ptAppList[0]);
while (true) {
    ptAppList[0]->ptApi->OnProcess(ptAppList[0]);
}
```

---

## 6. Naming Conventions

| Category | Convention | Example |
|----------|-----------|---------|
| Types | `SCREAMING_SNAKE_CASE_T` | `ENGINE_APP_T`, `JUNO_UDP_API_T` |
| Struct tags | `SCREAMING_SNAKE_CASE_TAG` | `ENGINE_APP_TAG`, `JUNO_UDP_ROOT_TAG` |
| Public functions | `PascalCase` with module prefix | `EngineApp_Init`, `JunoUdp_Init` |
| Static functions | `PascalCase` (shorter) | `Verify`, `OnStart`, `OnProcess` |
| Macros | `SCREAMING_SNAKE_CASE` with `JUNO_` | `JUNO_ASSERT_EXISTS`, `JUNO_MODULE_ROOT` |
| Pointer variables | `pt` prefix | `ptEngineApp`, `ptLogger` |
| Struct variables | `t` prefix | `tStatus`, `tEngineCmd` |
| Size variables | `z` prefix | `zSize`, `zRegistryCapacity` |
| Bool variables | `b` prefix | `bStop`, `bIsReceiver`, `bIsSome` |
| Array/count variables | `i` prefix | `iCounter`, `iMsgId` |
| Unsigned variables | `u` prefix | `uSeqNum`, `uHandle` |
| Float variables | `f` prefix | `fCurrentRpm` |
| Char pointer | `pc` prefix | `pcAddress`, `pcMsg` |
| Void pointer | `pv` prefix | `pvFailureUserData`, `pvAddr` |
| Function pointer | `pfcn` prefix | `pfcnFailureHandler`, `pfcnEntry` |
| Static fixed arrays | `arr` prefix | `arrPayload`, `ptArrCmdBuffer` |
| Private members | leading underscore | `_iSockFd`, `_uHandle`, `_uSeqNum` |

**Module-prefix separator:** use `_` between module family and function name:
`JunoUdp_Init`, `JunoThread_Init`, `JunoSb_BrokerInit`, `EngineApp_Init`.

---

## 7. Error Handling

### 7.1 Return type

All fallible functions return `JUNO_STATUS_T` (`int32_t`):

```c
typedef int32_t JUNO_STATUS_T;
#define JUNO_STATUS_SUCCESS             0
#define JUNO_STATUS_ERR                 1
#define JUNO_STATUS_NULLPTR_ERROR       2
#define JUNO_STATUS_INVALID_TYPE_ERROR  5
#define JUNO_STATUS_TIMEOUT_ERROR       16
#define JUNO_STATUS_OOB_ERROR           17
// ... etc.
```

### 7.2 Null-guard macros

```c
// Returns JUNO_STATUS_NULLPTR_ERROR if ptr is falsy. No failure handler invoked.
JUNO_ASSERT_EXISTS(ptr);
JUNO_ASSERT_EXISTS(ptA && ptB && ptC);   // compound guard

// Same, but also invokes ptMod's failure handler with a message.
JUNO_ASSERT_EXISTS_MODULE(ptr, ptMod, "descriptive message");
```

### 7.3 Status propagation

```c
// Expands to: if (tStatus != JUNO_STATUS_SUCCESS) { __VA_ARGS__; }
// Control flow depends entirely on __VA_ARGS__ — no implicit fall-through.
// With return — exits the function on failure:
JUNO_STATUS_T tStatus = SomeFunction(...);
JUNO_ASSERT_SUCCESS(tStatus, return tStatus);

// With goto — jumps to cleanup label on failure (used in engine_app.c):
JUNO_ASSERT_SUCCESS(tStatus, goto exit);
```

### 7.4 Failure handler invocation

The failure handler is diagnostic only — it does NOT alter control flow. Always
invoke it BEFORE returning a non-success status:

```c
// On a derived module pointer (accesses tRoot.handler internally)
JUNO_FAIL_MODULE(tStatus, ptDerivedModule, "Error message");
return tStatus;

// On a root pointer directly
JUNO_FAIL_ROOT(tStatus, ptRoot, "Error message");
return tStatus;

// Raw form (rarely used directly)
JUNO_FAIL(tStatus, pfcnHandler, pvUserData, "Error message");
```

Expansions from `status.h`:

```c
#define JUNO_FAIL_MODULE(tStatus, ptMod, pcMessage) \
    if(ptMod && ptMod->JUNO_MODULE_SUPER.JUNO_FAILURE_HANDLER){ \
        ptMod->JUNO_MODULE_SUPER.JUNO_FAILURE_HANDLER( \
            tStatus, pcMessage, ptMod->JUNO_MODULE_SUPER.JUNO_FAILURE_USER_DATA); }

#define JUNO_FAIL_ROOT(tStatus, ptMod, pcMessage) \
    if(ptMod && ptMod->JUNO_FAILURE_HANDLER){ \
        ptMod->JUNO_FAILURE_HANDLER(tStatus, pcMessage, ptMod->JUNO_FAILURE_USER_DATA); }
```

### 7.5 Result and Option types

For functions that return a value plus a status, use `JUNO_MODULE_RESULT`:

```c
// Define a result type
JUNO_MODULE_RESULT(JUNO_TIMESTAMP_RESULT_T, JUNO_TIMESTAMP_T);
// Expands to:
// typedef struct JUNO_TIMESTAMP_RESULT_T { JUNO_STATUS_T tStatus; JUNO_TIMESTAMP_T tOk; } ...;

// Usage
JUNO_TIMESTAMP_RESULT_T tResult = ptTime->ptApi->Now(ptTime);
JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult.tStatus);
JUNO_TIMESTAMP_T tTs = tResult.tOk;   // or JUNO_OK(tResult)
```

For optional returns, use `JUNO_MODULE_OPTION`:

```c
JUNO_MODULE_OPTION(JUNO_OPTION_POINTER_T, JUNO_POINTER_T);
// Expands to:
// typedef struct { bool bIsSome; JUNO_POINTER_T tSome; } JUNO_OPTION_POINTER_T;

JUNO_ASSERT_SOME(tOption, return JUNO_STATUS_ERR);
JUNO_POINTER_T tPtr = JUNO_SOME(tOption);
```

---

## 8. Forbidden Patterns

### 8.1 Dynamic allocation

Never use `malloc`, `calloc`, `realloc`, or `free`. All storage is caller-owned
and injected. Pass buffers as pointers with their sizes.

### 8.2 Lifecycle function declarations in headers

App-pattern lifecycle functions (`OnStart`, `OnProcess`, `OnExit`) are `static`
inside the `.c` file. They MUST NOT be declared in the header:

```c
// WRONG — in engine_app.h
JUNO_STATUS_T OnStart(JUNO_APP_ROOT_T *ptJunoApp);   // must not appear in header

// CORRECT — in engine_app.c only
static JUNO_STATUS_T OnStart(JUNO_APP_ROOT_T *ptJunoApp);
```

### 8.3 Vtable extern in headers for the app pattern

The internal static vtable of an application module is invisible to callers:

```c
// WRONG — in engine_app.h
extern const JUNO_APP_API_T tEngineAppApi;   // must not appear in header

// CORRECT — in engine_app.c, no extern, static only
static const JUNO_APP_API_T tEngineAppApi = { .OnStart = OnStart, ... };
```

Note: Platform modules (UDP, Thread) DO expose their vtable extern in the
freestanding header because callers must pass it to `Init`. App modules do not.

### 8.4 Passing `ptApi` to app Init functions

For the app pattern, `Init` wires the internal vtable itself. Callers do NOT pass
`ptApi` to `EngineApp_Init` (contrast with `JunoUdp_Init` which does take `ptApi`):

```c
// WRONG — app Init does not take ptApi
EngineApp_Init(&tEngineApp, &tEngineAppApi, ...);

// CORRECT
EngineApp_Init(&tEngineApp, &tLogger, &tTime, &tBroker, FailureHandler, NULL);
```

### 8.5 OS-specific fields in the root

POSIX/OS types must not appear in the root struct body:

```c
// WRONG — pthread_t is not freestanding
struct JUNO_THREAD_ROOT_TAG JUNO_MODULE_ROOT(JUNO_THREAD_API_T,
    pthread_t _tHandle;   // forbidden in root
);

// CORRECT — root has only freestanding, cross-platform fields
struct JUNO_THREAD_ROOT_TAG JUNO_MODULE_ROOT(JUNO_THREAD_API_T,
    volatile bool bStop;   // cross-platform cooperative-shutdown flag
);

// OS handle belongs in the platform derivation (thread_linux.h — not freestanding)
struct JUNO_THREAD_LINUX_TAG JUNO_MODULE_DERIVE(JUNO_THREAD_ROOT_T,
    pthread_t _tHandle;   // POSIX-only; confined to the platform TU
);
```

### 8.6 Duplicate failure handler fields in derivations

`JUNO_MODULE_ROOT` already injects `JUNO_FAILURE_HANDLER` and `JUNO_FAILURE_USER_DATA`.
Never redeclare these in a derivation:

```c
// WRONG
struct ENGINE_APP_TAG JUNO_MODULE_DERIVE(JUNO_APP_ROOT_T,
    JUNO_FAILURE_HANDLER_T _pfcnFailureHandler;   // duplicate — already in root
    ...
);

// CORRECT — derivation only adds new fields
struct ENGINE_APP_TAG JUNO_MODULE_DERIVE(JUNO_APP_ROOT_T,
    const JUNO_LOG_ROOT_T *ptLogger;
    ...
);
```

### 8.7 Struct tag syntax errors with module macros

`JUNO_MODULE_DERIVE` and `JUNO_MODULE_ROOT` expand to a struct body (opening and
closing braces). The `struct TAG` declaration precedes them; no extra braces:

```c
// WRONG — extra braces wrap the macro
struct FOO_TAG { JUNO_MODULE_DERIVE(BAR_ROOT_T, int x;) };

// WRONG — semicolon after macro as if it were a statement
struct FOO_TAG JUNO_MODULE_DERIVE(BAR_ROOT_T, int x;);

// CORRECT
typedef struct FOO_TAG FOO_T;
struct FOO_TAG JUNO_MODULE_DERIVE(BAR_ROOT_T,
    int x;
);
```

---

## 9. Complete Worked Example: Minimal Module

Below is a minimal but complete example of a custom module following all conventions.
It is synthetic (not from source) but illustrates every required piece.

### Header (`include/mymod/mymod_api.h`)

```c
#ifndef MYMOD_API_H
#define MYMOD_API_H
#include "juno/status.h"
#include "juno/module.h"
#include "juno/types.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct MYMOD_ROOT_TAG MYMOD_ROOT_T;
typedef struct MYMOD_API_TAG  MYMOD_API_T;
typedef struct MYMOD_IMPL_TAG MYMOD_IMPL_T;
typedef union  MYMOD_TAG      MYMOD_T;

// Vtable — what the module can do
struct MYMOD_API_TAG
{
    JUNO_STATUS_T (*Process)(MYMOD_ROOT_T *ptRoot, uint32_t uInput);
};

// Root — freestanding fields only
struct MYMOD_ROOT_TAG JUNO_MODULE_ROOT(MYMOD_API_T,
    uint32_t _uState;
);

// Concrete derivation — implementation-specific fields
struct MYMOD_IMPL_TAG JUNO_MODULE_DERIVE(MYMOD_ROOT_T,
    const JUNO_LOG_ROOT_T *ptLogger;   // injected dependency
);

// Union — holds root + all derivations
union MYMOD_TAG JUNO_MODULE(MYMOD_API_T, MYMOD_ROOT_T,
    MYMOD_IMPL_T tImpl;
);

// Init function
JUNO_STATUS_T MyMod_Init(
    MYMOD_IMPL_T            *ptMod,
    const JUNO_LOG_ROOT_T   *ptLogger,
    JUNO_FAILURE_HANDLER_T   pfcnFailureHandler,
    void                    *pvFailureUserData
);

#ifdef __cplusplus
}
#endif
#endif
```

### Source (`src/mymod.c`)

```c
#include "mymod/mymod_api.h"
#include "juno/macros.h"
#include "juno/log/log_api.h"

// Forward declarations
static inline JUNO_STATUS_T Verify(MYMOD_ROOT_T *ptRoot);
static JUNO_STATUS_T Process(MYMOD_ROOT_T *ptRoot, uint32_t uInput);

// Internal vtable — static, never exposed in header
static const MYMOD_API_T tMyModApi = {
    .Process = Process
};

static inline JUNO_STATUS_T Verify(MYMOD_ROOT_T *ptRoot)
{
    JUNO_ASSERT_EXISTS(ptRoot);
    MYMOD_IMPL_T *ptImpl = (MYMOD_IMPL_T *)(ptRoot);
    JUNO_ASSERT_EXISTS_MODULE(
        ptImpl && ptImpl->tRoot.ptApi && ptImpl->ptLogger,
        ptImpl,
        "MyMod missing dependencies"
    );
    return JUNO_STATUS_SUCCESS;
}

JUNO_STATUS_T MyMod_Init(
    MYMOD_IMPL_T            *ptMod,
    const JUNO_LOG_ROOT_T   *ptLogger,
    JUNO_FAILURE_HANDLER_T   pfcnFailureHandler,
    void                    *pvFailureUserData
)
{
    JUNO_ASSERT_EXISTS(ptMod);
    ptMod->tRoot.ptApi              = &tMyModApi;
    ptMod->tRoot.JUNO_FAILURE_HANDLER    = pfcnFailureHandler;
    ptMod->tRoot.JUNO_FAILURE_USER_DATA  = pvFailureUserData;
    ptMod->tRoot._uState            = 0;
    ptMod->ptLogger                 = ptLogger;
    JUNO_STATUS_T tStatus = Verify(&ptMod->tRoot);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}

static JUNO_STATUS_T Process(MYMOD_ROOT_T *ptRoot, uint32_t uInput)
{
    JUNO_STATUS_T tStatus = Verify(ptRoot);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    MYMOD_IMPL_T *ptImpl = (MYMOD_IMPL_T *)(ptRoot);
    ptImpl->tRoot._uState += uInput;
    return JUNO_STATUS_SUCCESS;
}
```

### Composition root usage

```c
static MYMOD_T tMyMod = {0};
JUNO_STATUS_T tStatus = MyMod_Init(&tMyMod.tImpl,
                                   &tLogger, FailureHandler, NULL);
JUNO_ASSERT_SUCCESS(tStatus, return -1);
tMyMod.tRoot.ptApi->Process(&tMyMod.tRoot, 42);
```

---

## 10. Quick-Reference Checklist

Use this before submitting any LibJuno C implementation:

- [ ] Root struct uses `JUNO_MODULE_ROOT(API_T, ...)` with freestanding-only extra fields
- [ ] `JUNO_MODULE_EMPTY` used when root has no extra fields
- [ ] Derivation struct uses `JUNO_MODULE_DERIVE(ROOT_T, ...)` with `typedef` before the struct
- [ ] Union uses `union TAG JUNO_MODULE(API_T, ROOT_T, DERIVED_T tDerived;)`
- [ ] Vtable function pointers all take `ROOT_T *ptRoot` as first argument, return `JUNO_STATUS_T`
- [ ] `Init` wires `ptApi`, stores `JUNO_FAILURE_HANDLER` and `JUNO_FAILURE_USER_DATA`, calls `Verify` last
- [ ] `Verify` checks: (a) root non-null, (b) dependencies non-null via `JUNO_ASSERT_EXISTS_MODULE`, (c) `ptApi` identity check for app modules
- [ ] Every public/vtable function calls `Verify` as first action
- [ ] `JUNO_FAIL_MODULE` or `JUNO_FAIL_ROOT` called before every non-success return
- [ ] No `malloc`/`calloc`/`realloc`/`free`
- [ ] OS-specific types belong in derivations, not in the root; root carries only freestanding-compatible fields
- [ ] No duplicate `JUNO_FAILURE_HANDLER`/`JUNO_FAILURE_USER_DATA` in derivations
- [ ] App lifecycle functions (`OnStart` etc.) are `static` in `.c`, absent from header
- [ ] App vtable is `static const` in `.c`, no `extern` in header
- [ ] App `Init` does NOT take `ptApi` as a parameter
- [ ] All types `SCREAMING_SNAKE_CASE_T`, functions `PascalCase`, variables Hungarian
- [ ] Private members have leading underscore
- [ ] `// @{"req": ["REQ-MODULE-NNN"]}` traceability tag above each implementing function
