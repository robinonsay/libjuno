> Part of: [Software Design Document](index.md) — Section 4

# Section 4: Thread Module Design

// @{"design": ["REQ-THREAD-001", "REQ-THREAD-002"]}
## 4.1 Purpose and Scope

The Thread module (`JUNO_THREAD_T`) provides a freestanding C11 interface for creating,
stopping, and joining a single OS thread. It encapsulates POSIX pthreads behind a vtable
so that application code depends only on the abstract interface — not on any platform
header.

The module uses a two-layer design:

- **C11 freestanding interface layer** — The public header (`juno/thread.h`) defines the
  root struct (`JUNO_THREAD_ROOT_T`), the vtable struct (`JUNO_THREAD_API_T`), and the
  `JunoThread_Init` prototype. It includes only `<stdint.h>`, `<stddef.h>`, `<stdbool.h>`,
  `juno/status.h`, `juno/module.h`, and `juno/types.h`. No OS or POSIX headers appear here.

- **C++ Linux/POSIX implementation layer** — A single `.cpp` translation unit includes
  `<pthread.h>` and provides the concrete vtable (`JunoThread_LinuxApi`) whose function
  pointers call `pthread_create` and `pthread_join`. This is the only translation unit in
  the module that contains OS-specific code.

The module supports a **cooperative shutdown model**: rather than forcibly terminating a
thread, the caller calls `Stop` to set a flag in the module root, and the thread entry
function periodically reads that flag and exits its loop voluntarily. The caller then calls
`Join` to wait for the thread to finish.

---

// @{"design": ["REQ-THREAD-001", "REQ-THREAD-007"]}
## 4.2 Data Structures

### 4.2.1 `JUNO_THREAD_ROOT_T` — Module Root

```c
struct JUNO_THREAD_ROOT_TAG {
    const JUNO_THREAD_API_T  *ptApi;               // vtable pointer (injected by Init)
    JUNO_FAILURE_HANDLER_T    _pfcnFailureHandler;  // diagnostic callback (may be NULL)
    void                     *_pvFailureUserData;   // user data passed to failure handler
    uintptr_t                 _uHandle;             // opaque OS thread handle; 0 = not running
    volatile bool             bStop;               // cooperative shutdown flag
};
```

**Field notes:**

| Field | Type | Underscore | Rationale |
|-------|------|-----------|-----------|
| `ptApi` | `const JUNO_THREAD_API_T *` | No | Public — required by every vtable call |
| `_pfcnFailureHandler` | `JUNO_FAILURE_HANDLER_T` | Yes | Implementation detail; callers do not read it |
| `_pvFailureUserData` | `void *` | Yes | Implementation detail; callers do not read it |
| `_uHandle` | `uintptr_t` | Yes | Implementation detail; stores `pthread_t` cast to integer; callers must not inspect or modify it |
| `bStop` | `volatile bool` | **No** | **Public** — the thread entry function reads `ptRoot->bStop` directly in its scheduling loop. Because the entry function is caller-supplied code outside the module, this field must be part of the public interface. The `volatile` qualifier ensures reads are not optimized away across thread contexts. |

The `_uHandle` field equals `0` when no thread is running. This is the **not-running
sentinel**. `Init` sets it to `0`; `Create` sets it to the `pthread_t` handle cast to
`uintptr_t`; `Join` resets it to `0` after the thread exits.

### 4.2.2 `JUNO_THREAD_API_T` — Vtable

```c
typedef struct JUNO_THREAD_API_TAG {
    JUNO_STATUS_T (*Create)(JUNO_THREAD_ROOT_T *ptRoot,
                            void *(*pfcnEntry)(void *),
                            void *pvArg);
    JUNO_STATUS_T (*Stop)(JUNO_THREAD_ROOT_T *ptRoot);
    JUNO_STATUS_T (*Join)(JUNO_THREAD_ROOT_T *ptRoot);
} JUNO_THREAD_API_T;
```

**Function pointer descriptions:**

| Pointer | Signature | Description |
|---------|-----------|-------------|
| `Create` | `(ptRoot, pfcnEntry, pvArg) → JUNO_STATUS_T` | Creates and starts a new OS thread executing `pfcnEntry(pvArg)`. `pfcnEntry` is a standard POSIX thread entry function (`void *(*)(void *)`). `pvArg` is the argument forwarded to the entry; callers typically pass `ptRoot` itself so the entry can read `bStop`. |
| `Stop` | `(ptRoot) → JUNO_STATUS_T` | Sets `ptRoot->bStop = true`. Does not send signals or cancel the thread. |
| `Join` | `(ptRoot) → JUNO_STATUS_T` | Blocks until the managed thread exits, then resets `_uHandle` to `0`. |

---

// @{"design": ["REQ-THREAD-012"]}
## 4.3 Module Initialization

```c
JUNO_STATUS_T JunoThread_Init(
    JUNO_THREAD_ROOT_T          *ptRoot,            // caller-owned root storage
    const JUNO_THREAD_API_T     *ptApi,             // vtable (Linux impl or test double)
    JUNO_FAILURE_HANDLER_T       pfcnFailureHandler, // diagnostic callback (may be NULL)
    void                        *pvFailureUserData   // user data for failure handler
);
```

**Initialization sequence:**

1. Guard `ptRoot` with `JUNO_ASSERT_EXISTS(ptRoot)` — returns `JUNO_STATUS_NULLPTR_ERROR`
   if NULL.
2. Guard `ptApi` with `JUNO_ASSERT_EXISTS(ptApi)` — returns `JUNO_STATUS_NULLPTR_ERROR`
   if NULL.
3. Wire the vtable: `ptRoot->ptApi = ptApi`.
4. Initialize the thread handle sentinel: `ptRoot->_uHandle = 0`.
5. Initialize the stop flag: `ptRoot->bStop = false`.
6. Store the failure handler: `ptRoot->_pfcnFailureHandler = pfcnFailureHandler`.
7. Store the failure handler user data: `ptRoot->_pvFailureUserData = pvFailureUserData`.
8. Return `JUNO_STATUS_SUCCESS`.

The failure handler parameter (`pfcnFailureHandler`) may be NULL. When it is NULL, the
module skips the handler invocation step on errors. This allows callers to omit diagnostic
output when it is not needed.

The vtable pointer (`ptApi`) may point to the Linux pthreads implementation vtable
(`JunoThread_LinuxApi`) in production or to a test-double vtable during unit testing. This
is the vtable-injection point that decouples application code from the OS.

---

// @{"design": ["REQ-THREAD-003", "REQ-THREAD-004", "REQ-THREAD-005", "REQ-THREAD-006", "REQ-THREAD-015"]}
## 4.4 Vtable Operations — Algorithms

### 4.4.1 Create (REQ-THREAD-003, REQ-THREAD-004, REQ-THREAD-015)

```
Create(ptRoot, pfcnEntry, pvArg):
  Verify(ptRoot)                                   // JUNO_ASSERT_EXISTS(ptRoot)
  Verify(pfcnEntry)                                // JUNO_ASSERT_EXISTS(pfcnEntry)
  if ptRoot->_uHandle != 0:
    invoke ptRoot->_pfcnFailureHandler(...)        // diagnostic only
    return JUNO_STATUS_ALREADY_RUNNING             // REQ-THREAD-015
  result = pthread_create(&tid, NULL, pfcnEntry, pvArg)
  if result != 0:
    invoke ptRoot->_pfcnFailureHandler(...)        // diagnostic only
    return JUNO_STATUS_ERROR
  ptRoot->_uHandle = (uintptr_t)tid               // store opaque handle
  return JUNO_STATUS_SUCCESS
```

The `_uHandle != 0` check enforces the single-thread-per-root constraint (REQ-THREAD-004).
If the caller attempts to create a second thread before joining the first, `Create` returns
`JUNO_STATUS_ALREADY_RUNNING` without touching the existing thread (REQ-THREAD-015).

### 4.4.2 Stop (REQ-THREAD-006, REQ-THREAD-007)

```
Stop(ptRoot):
  Verify(ptRoot)                                   // JUNO_ASSERT_EXISTS(ptRoot)
  if ptRoot->_uHandle == 0:
    invoke ptRoot->_pfcnFailureHandler(...)        // diagnostic only
    return JUNO_STATUS_INVALID_HANDLE              // no thread to stop
  ptRoot->bStop = true                             // cooperative shutdown signal
  return JUNO_STATUS_SUCCESS
```

`Stop` does **not** call `pthread_cancel`, send signals, or otherwise force thread
termination. It only sets the `bStop` flag. The thread entry function is responsible for
observing the flag in its scheduling loop and exiting cooperatively. This design avoids
resource-leak risks associated with forced cancellation and gives the thread an opportunity
to clean up its own state before exiting.

### 4.4.3 Join (REQ-THREAD-005)

```
Join(ptRoot):
  Verify(ptRoot)                                   // JUNO_ASSERT_EXISTS(ptRoot)
  if ptRoot->_uHandle == 0:
    invoke ptRoot->_pfcnFailureHandler(...)        // diagnostic only
    return JUNO_STATUS_INVALID_HANDLE              // no thread to join
  result = pthread_join((pthread_t)ptRoot->_uHandle, NULL)
  if result != 0:
    invoke ptRoot->_pfcnFailureHandler(...)        // diagnostic only
    return JUNO_STATUS_ERROR
  ptRoot->_uHandle = 0                            // reset to not-running sentinel
  return JUNO_STATUS_SUCCESS
```

After a successful `Join`, `_uHandle` is reset to `0`. This means the root is ready for
another `Create` call if the caller wants to restart a thread using the same root instance.

---

// @{"design": ["REQ-THREAD-006", "REQ-THREAD-007"]}
## 4.5 Cooperative Shutdown Protocol

The cooperative shutdown model is the protocol by which the caller signals a thread to exit
and then waits for it to do so cleanly.

**Protocol steps:**

1. The caller calls `Create(ptRoot, pfcnEntry, ptRoot)`, passing `ptRoot` as `pvArg`.
   The thread entry function receives `pvArg` and casts it back to `JUNO_THREAD_ROOT_T *`.

2. The thread entry function reads `ptRoot->bStop` in its scheduling loop. While
   `bStop == false`, it executes its scheduler major frame (e.g., calls
   `ptSch->ptApi->Execute(ptSch)`). When `bStop` becomes `true`, it exits the loop and
   returns.

3. The caller (from any thread) calls `Stop(ptRoot)`, which sets `ptRoot->bStop = true`.
   The `volatile` qualifier ensures the write is visible to the thread that reads the flag.

4. The caller calls `Join(ptRoot)`, which blocks until the thread entry function returns.
   After `Join` returns successfully, it is safe to free or reuse the `JUNO_THREAD_ROOT_T`
   storage (subject to the memory ownership rules in Section 4.8).

**Typical thread entry function:**

```c
void *ThreadEntry(void *pvArg)
{
    JUNO_THREAD_ROOT_T *ptRoot = (JUNO_THREAD_ROOT_T *)pvArg;
    while (!ptRoot->bStop) {
        // execute scheduler major frame
        ptSch->ptApi->Execute(ptSch);
    }
    return NULL;
}
```

The entry function does not need to include any Thread module headers beyond access to
`JUNO_THREAD_ROOT_T`. Because `bStop` is a public field (no underscore), the entry function
may read it directly without violating the module's encapsulation boundary.

---

// @{"design": ["REQ-THREAD-010", "REQ-THREAD-013"]}
## 4.6 Error Handling Strategy

All three vtable operations (`Create`, `Stop`, `Join`) and `JunoThread_Init` return
`JUNO_STATUS_T`. The following error codes are used by the Thread module:

| Condition | Error Returned | Operation(s) |
|-----------|---------------|-------------|
| `ptRoot` is NULL | `JUNO_STATUS_NULLPTR_ERROR` | All |
| `ptApi` is NULL | `JUNO_STATUS_NULLPTR_ERROR` | `Init` |
| `pfcnEntry` is NULL | `JUNO_STATUS_NULLPTR_ERROR` | `Create` |
| Thread already running (`_uHandle != 0`) | `JUNO_STATUS_ALREADY_RUNNING` | `Create` |
| No thread running (`_uHandle == 0`) | `JUNO_STATUS_INVALID_HANDLE` | `Stop`, `Join` |
| `pthread_create` returns non-zero | `JUNO_STATUS_ERROR` | `Create` |
| `pthread_join` returns non-zero | `JUNO_STATUS_ERROR` | `Join` |

**NULL pointer guards** use `JUNO_ASSERT_EXISTS(ptr)`, which returns
`JUNO_STATUS_NULLPTR_ERROR` immediately without invoking the failure handler (the handler
pointer itself may be uninitialized at that point).

**Failure handler invocation** occurs before returning any non-success status (except for
the NULL-pointer guard path, where the handler pointer cannot be trusted). The failure
handler is diagnostic only — it must never alter control flow. The operation returns its
error status regardless of what the handler does.

---

// @{"design": ["REQ-THREAD-008", "REQ-THREAD-009", "REQ-THREAD-011", "REQ-THREAD-014"]}
## 4.7 Interface Header Constraints

The two-layer separation keeps OS headers confined to implementation translation units.

**Allowed includes in the public interface header (`juno/thread.h`):**

| Header | Purpose |
|--------|---------|
| `<stdint.h>` | `uintptr_t` for the opaque thread handle |
| `<stddef.h>` | `NULL`, `size_t` |
| `<stdbool.h>` | `bool`, `true`, `false` for `bStop` |
| `juno/status.h` | `JUNO_STATUS_T` return type |
| `juno/module.h` | `JUNO_FAILURE_HANDLER_T`, module macros |
| `juno/types.h` | Shared LibJuno types |

**Forbidden in the public interface header:**

| Header | Reason Forbidden |
|--------|-----------------|
| `<pthread.h>` | POSIX — not available on freestanding targets |
| `<sys/types.h>` | POSIX — platform-specific |
| Any other OS/POSIX header | Breaks freestanding portability |

The POSIX `pthread_t` type is stored inside `JUNO_THREAD_ROOT_T` as `uintptr_t`
(`_uHandle`). The Linux implementation casts `pthread_t` to `uintptr_t` on store (in
`Create`) and casts it back on use (in `Join`). This is the mechanism that keeps the OS
type out of the freestanding public interface — the interface sees only an opaque integer
that happens to hold a thread handle.

**Two-layer separation summary:**

| Layer | File Type | `pthread.h` | `malloc` / `free` |
|-------|-----------|-------------|-------------------|
| Interface | `.h` (C11 freestanding) | Forbidden | Forbidden |
| Implementation | `.cpp` (C++11 Linux) | Allowed | Forbidden |

No dynamic allocation is permitted in either layer (REQ-THREAD-008).

---

// @{"design": ["REQ-THREAD-001", "REQ-THREAD-008"]}
## 4.8 Memory Ownership

| Resource | Who Allocates | Who Frees | Lifetime Requirement |
|----------|--------------|-----------|---------------------|
| `JUNO_THREAD_ROOT_T` | Caller (composition root) | Caller | Must remain valid from `Init` until after `Join` returns. The running thread holds a pointer to this root (via `pvArg`); the root must not be destroyed while the thread is executing. |
| `JUNO_THREAD_API_T` (vtable) | Caller (static const) | Never freed | Must remain valid for the lifetime of the root. Vtables are always `const` and typically statically allocated. |
| Thread entry function `pvArg` | Caller | Caller | If the caller passes `ptRoot` as `pvArg`, the root's required lifetime is inherently sufficient — it must outlive the thread anyway. If the caller passes a different buffer, that buffer must also remain valid until the thread exits. |
| Module-internal buffers | None | N/A | The Thread module allocates no internal storage. Zero bytes are allocated by any Thread module translation unit. |

**Key ownership rule:** The caller must not reuse or destroy `JUNO_THREAD_ROOT_T` storage
until `Join` has returned successfully. Destroying the root while a thread is executing
causes undefined behavior because the thread entry function holds a live pointer to it.

The correct teardown sequence is:

```
Stop(ptRoot)   → signal the thread to exit
Join(ptRoot)   → wait until the thread has exited (root is now safe to reuse/destroy)
```
