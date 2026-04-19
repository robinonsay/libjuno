> Part of: [Software Design Document](index.md) — Section 3

# Section 3: UDP Socket Module Design

---

## 3.1 Purpose and Scope

// @{"design": ["REQ-UDP-001", "REQ-UDP-002"]}

The UDP socket module provides a portable, freestanding-compatible C11 interface for
transmitting and receiving fixed-size UDP datagrams. It encapsulates all socket
lifecycle operations (open, send, receive, close) behind a vtable-based dependency
injection interface, enabling test doubles to replace the real POSIX implementation
without modifying application or thread code.

The module follows a strict two-layer design:

- **Interface layer** — a freestanding C11 header (`udp_api.h`) that declares the root
  struct, vtable, and configuration types. This layer has no POSIX or OS-specific
  includes and can be compiled with `-nostdlib -ffreestanding`.
- **Implementation layer** — a Linux/POSIX C++ translation unit (`linux_udp_impl.cpp`)
  that provides the concrete vtable satisfying the interface. This is the only file in
  the module that includes POSIX socket headers.

This separation means application code and thread logic depend only on the C11
interface. Porting to a different OS requires only a new implementation file; the
interface and all code that uses it remain unchanged.

---

## 3.2 Data Structures

// @{"design": ["REQ-UDP-001", "REQ-UDP-017", "REQ-UDP-018"]}

### 3.2.1 `UDP_THREAD_MSG_T` — Message Structure

The message structure is the single unit of data transmitted or received per datagram.
Every Send and Receive call transfers exactly one `UDP_THREAD_MSG_T` as an atomic
datagram.

```c
typedef struct UDP_THREAD_MSG_TAG {
    uint32_t uSeqNum;           /* monotonically increasing sequence number (wraps at UINT32_MAX) */
    uint32_t uTimestampSec;     /* timestamp: whole seconds                                       */
    uint32_t uTimestampSubSec;  /* timestamp: sub-second component (units defined by application) */
    uint8_t  arrPayload[64];    /* fixed-size application payload                                 */
} UDP_THREAD_MSG_T;
/* sizeof(UDP_THREAD_MSG_T) = 76 bytes (3 × uint32_t = 12 bytes + 64 × uint8_t = 64 bytes)      */
```

Field descriptions:

| Field | Type | Size (bytes) | Description |
|-------|------|-------------|-------------|
| `uSeqNum` | `uint32_t` | 4 | Monotonically increasing sender counter; wraps at `UINT32_MAX` via unsigned arithmetic |
| `uTimestampSec` | `uint32_t` | 4 | Whole-second component of the sender's timestamp |
| `uTimestampSubSec` | `uint32_t` | 4 | Sub-second component of the sender's timestamp |
| `arrPayload` | `uint8_t[64]` | 64 | Fixed-size application-defined payload (`arrPayload` — fixed-size payload array; the `arr` prefix is used for static array fields) |
| **Total** | | **76** | |

Note: The `arr` prefix is used for fixed-size array fields (an extension to the project Hungarian notation table defined in `ai/memory/coding-standards.md`).

The fixed, compile-time-known size is essential: both Send and Receive use
`sizeof(UDP_THREAD_MSG_T)` as the byte count, ensuring datagrams are never truncated
and no partial reads or writes are possible at the message boundary.

---

### 3.2.2 `JUNO_UDP_CFG_T` — Configuration Structure

The configuration structure is passed to `Open` by the caller. It carries all
information needed to configure and bind or connect the socket. The caller allocates
this struct and it need only remain valid for the duration of the `Open` call.

```c
typedef struct JUNO_UDP_CFG_TAG {
    const char *pcAddress;   /* IPv4 address string, e.g., "127.0.0.1"; NULL or "0.0.0.0" for receiver */
    uint16_t    uPort;       /* UDP port number (host byte order)                                       */
    uint32_t    uTimeoutMs;  /* receive timeout in milliseconds; 0 = no timeout (blocking)             */
    bool        bIsReceiver; /* true = bind to local port (receiver); false = connect to remote (sender) */
} JUNO_UDP_CFG_T;
```

The `bIsReceiver` field distinguishes the two socket roles (REQ-UDP-004, REQ-UDP-005)
within a single `Open` implementation:

- When `bIsReceiver == true`, the implementation calls `bind()` to register the socket
  on the local port, enabling incoming datagrams to be received.
- When `bIsReceiver == false`, the implementation calls `connect()` to associate the
  socket with the remote address and port, enabling `Send` calls without per-call
  addressing.

---

### 3.2.3 `JUNO_UDP_ROOT_T` — Module Root Structure

The root structure is the module instance. It is caller-allocated (stack or static)
and injected into every API call. It holds the vtable pointer and all instance state.

The root is defined using the `JUNO_MODULE_ROOT` macro, which takes the vtable type as
its first argument and any additional root-level members as variadic arguments. Because
the UDP module has exactly one Linux implementation, the socket descriptor `_iSockFd` is
placed directly in the root via the variadic argument rather than in a separate
derivation. This keeps the design simple — callers work entirely with `JUNO_UDP_ROOT_T`
and need not know about a derivation struct.

```c
typedef struct JUNO_UDP_ROOT_TAG JUNO_MODULE_ROOT(JUNO_UDP_API_T, intptr_t _iSockFd;) JUNO_UDP_ROOT_T;
```

Which expands to the following layout:

```c
typedef struct JUNO_UDP_ROOT_TAG {
    const JUNO_UDP_API_T       *ptApi;               /* vtable pointer — wired by Init, never NULL after init */
    JUNO_FAILURE_HANDLER_T      _pfcnFailureHandler;  /* diagnostic callback; invoked on any error            */
    JUNO_USER_DATA_T           *_pvFailureUserData;   /* opaque user data pointer threaded to failure handler  */
    intptr_t                    _iSockFd;             /* opaque socket descriptor; -1 = invalid/closed        */
} JUNO_UDP_ROOT_T;
```

Field descriptions:

| Field | Type | Description |
|-------|------|-------------|
| `ptApi` | `const JUNO_UDP_API_T *` | Vtable pointer injected by `JunoUdp_Init`; the sole dispatch mechanism |
| `_pfcnFailureHandler` | `JUNO_FAILURE_HANDLER_T` | Diagnostic callback invoked before any error return; never alters control flow |
| `_pvFailureUserData` | `void *` | Opaque pointer passed verbatim to the failure handler |
| `_iSockFd` | `intptr_t` | Platform socket descriptor; sentinel value `-1` means socket is closed/invalid |

The `_iSockFd` sentinel of `-1` is the open/closed state discriminator used by `Send`,
`Receive`, and `Close` to detect whether `Open` has been called. Using `intptr_t`
ensures the field is wide enough to hold any file descriptor on 32-bit and 64-bit
platforms without casting.

---

### 3.2.4 `JUNO_UDP_API_T` — Vtable (Function Pointer Table)

The vtable struct defines the interface contract. Every concrete UDP implementation
(Linux POSIX, test double) provides a statically allocated `JUNO_UDP_API_T` whose
function pointers are wired into a root by `JunoUdp_Init`.

```c
typedef struct JUNO_UDP_API_TAG {
    JUNO_STATUS_T (*Open)   (JUNO_UDP_ROOT_T *ptRoot, const JUNO_UDP_CFG_T *ptCfg);
    JUNO_STATUS_T (*Send)   (JUNO_UDP_ROOT_T *ptRoot, const UDP_THREAD_MSG_T *ptMsg);
    JUNO_STATUS_T (*Receive)(JUNO_UDP_ROOT_T *ptRoot, UDP_THREAD_MSG_T *ptMsg);
    JUNO_STATUS_T (*Close)  (JUNO_UDP_ROOT_T *ptRoot);
} JUNO_UDP_API_T;
```

All four operations take the module root as their first argument, providing access to
the instance's socket descriptor and failure handler. All return `JUNO_STATUS_T`,
enabling uniform error propagation throughout the call chain.

---

// @{"design": ["REQ-UDP-014"]}
### 3.2.5 `JUNO_UDP_LINUX_T` — Linux Derivation

```c
// @{"design": ["REQ-UDP-014"]}
// Linux POSIX derivation — embeds root as first member (JUNO_MODULE_SUPER pattern)
struct JUNO_UDP_LINUX_TAG {
    JUNO_UDP_ROOT_T tRoot;  // MUST be first member — enables safe upcast from root
    // No additional fields: _iSockFd is in root via JUNO_MODULE_ROOT variadic
};
typedef struct JUNO_UDP_LINUX_TAG JUNO_UDP_LINUX_T;
```

---

### 3.2.6 `JUNO_UDP_T` — Module Union

The module union is the type-safe polymorphic handle used by callers. It is defined
as a union (expanded form of the `JUNO_MODULE(...)` pattern):

```c
// Type-safe polymorphic handle for callers
union JUNO_UDP_TAG {
    JUNO_UDP_ROOT_T  tRoot;
    JUNO_UDP_LINUX_T tLinux;
};
typedef union JUNO_UDP_TAG JUNO_UDP_T;
```

For this single-implementation example module, the union contains the root and the
Linux derivation. The caller allocates `JUNO_UDP_T` and passes `&tUdp.tRoot` to
`JunoUdp_Init`. If a second implementation were added (e.g., a loopback test stub
with internal state), it would be added as a union member alongside `tRoot`, following
the standard `JUNO_MODULE_DERIVE` pattern.

---

## 3.3 Module Initialization

// @{"design": ["REQ-UDP-016"]}

The `JunoUdp_Init` function wires a concrete vtable into a caller-provided root and
validates all injected dependencies.

```c
JUNO_STATUS_T JunoUdp_Init(
    JUNO_UDP_ROOT_T            *ptRoot,             /* caller-owned root storage; must be non-NULL */
    const JUNO_UDP_API_T       *ptApi,              /* vtable (Linux impl or test double); must be non-NULL */
    JUNO_FAILURE_HANDLER_T      pfcnFailureHandler, /* diagnostic callback; may be NULL */
    void                       *pvFailureUserData   /* opaque user data for the handler; may be NULL */
);
```

Initialization sequence:

1. Guard `ptRoot` with `JUNO_ASSERT_EXISTS(ptRoot)` — returns `JUNO_STATUS_NULLPTR_ERROR` if NULL.
2. Guard `ptApi` with `JUNO_ASSERT_EXISTS(ptApi)` — returns `JUNO_STATUS_NULLPTR_ERROR` if NULL.
3. Store `ptApi` into `ptRoot->ptApi`.
4. Store `pfcnFailureHandler` into `ptRoot->_pfcnFailureHandler` (NULL is allowed; absence of a handler is valid).
5. Store `pvFailureUserData` into `ptRoot->_pvFailureUserData`.
6. Initialize `ptRoot->_iSockFd = -1` (invalid sentinel — no socket is open after init).
7. Return `JUNO_STATUS_SUCCESS`.

The `_iSockFd` sentinel ensures that `Send`, `Receive`, and `Close` can reliably detect
the uninitialized/closed state without an additional boolean flag, and that a freshly
initialized but unopened root never passes a stale descriptor to the OS.

---

## 3.4 Vtable Operations — Algorithms

// @{"design": ["REQ-UDP-003", "REQ-UDP-004", "REQ-UDP-005", "REQ-UDP-006", "REQ-UDP-007", "REQ-UDP-008", "REQ-UDP-009", "REQ-UDP-015"]}

### 3.4.1 Open (REQ-UDP-003, REQ-UDP-004, REQ-UDP-005)

`Open` allocates the OS socket resource and configures it as either a receiver (bind)
or sender (connect) based on `ptCfg->bIsReceiver`.

```
Open(ptRoot, ptCfg):
  Verify(ptRoot)                                       // JUNO_ASSERT_EXISTS(ptRoot), JUNO_ASSERT_EXISTS(ptRoot->ptApi)
  JUNO_ASSERT_EXISTS(ptCfg)                            // ptCfg must be non-NULL
  if ptRoot->_iSockFd != -1:
    invoke failure handler(JUNO_STATUS_INVALID_REF_ERROR)
    return JUNO_STATUS_INVALID_REF_ERROR               // already open; double-open is an error

  fd = socket(AF_INET, SOCK_DGRAM, 0)
  if fd < 0:
    invoke failure handler(JUNO_STATUS_ERROR)
    return JUNO_STATUS_ERROR

  if ptCfg->uTimeoutMs > 0:
    tv = { .tv_sec  = ptCfg->uTimeoutMs / 1000,
           .tv_usec = (ptCfg->uTimeoutMs % 1000) * 1000 }
    result = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))
    if result < 0:
      invoke failure handler(JUNO_STATUS_ERROR)
      // Open continues; socket is still functional, only receive timeout is affected
      // Open does NOT return an error for setsockopt failure

  if ptCfg->bIsReceiver:
    local_addr = { AF_INET, htons(ptCfg->uPort), INADDR_ANY }
    result = bind(fd, &local_addr, sizeof(local_addr))
    if result < 0:
      close(fd)
      invoke failure handler(JUNO_STATUS_ERROR)
      return JUNO_STATUS_ERROR
  else:
    remote_addr = { AF_INET, htons(ptCfg->uPort), inet_addr(ptCfg->pcAddress) }
    result = connect(fd, &remote_addr, sizeof(remote_addr))
    if result < 0:
      close(fd)
      invoke failure handler(JUNO_STATUS_ERROR)
      return JUNO_STATUS_ERROR

  ptRoot->_iSockFd = (intptr_t)fd
  return JUNO_STATUS_SUCCESS
```

Note: When `Open` fails after `socket()` succeeds, the raw file descriptor is
immediately `close()`d before returning to prevent resource leaks. The root's
`_iSockFd` is only written on success, preserving the `-1` sentinel on failure.

---

### 3.4.2 Send (REQ-UDP-006, REQ-UDP-015)

`Send` transmits exactly one `UDP_THREAD_MSG_T` as a single UDP datagram. Because the
socket was configured with `connect()` during `Open`, `sendto()` is called with NULL
destination (relying on the connected address). Alternatively, `send()` may be used
directly on a connected UDP socket.

```
Send(ptRoot, ptMsg):
  Verify(ptRoot)                                       // JUNO_ASSERT_EXISTS(ptRoot)
  JUNO_ASSERT_EXISTS(ptMsg)                            // ptMsg must be non-NULL
  if ptRoot->_iSockFd == -1:
    invoke failure handler(JUNO_STATUS_INVALID_REF_ERROR)
    return JUNO_STATUS_INVALID_REF_ERROR               // socket not open

  bytes_sent = sendto(
    (int)ptRoot->_iSockFd,
    ptMsg,
    sizeof(UDP_THREAD_MSG_T),                          // exactly 76 bytes; never more, never less
    0,
    NULL,                                              // NULL: use connected address from Open
    0
  )
  if bytes_sent != (ssize_t)sizeof(UDP_THREAD_MSG_T):
    invoke failure handler(JUNO_STATUS_ERROR)
    return JUNO_STATUS_ERROR

  return JUNO_STATUS_SUCCESS
```

The size check `bytes_sent != sizeof(UDP_THREAD_MSG_T)` guards against partial sends.
For UDP datagrams, a partial send is abnormal (the kernel either sends all bytes or
returns an error), but the check is retained for correctness and defensive programming.

---

### 3.4.3 Receive (REQ-UDP-007, REQ-UDP-008, REQ-UDP-015)

`Receive` blocks until one complete `UDP_THREAD_MSG_T` datagram is received or the
configured timeout elapses. The blocking duration is bounded by the `SO_RCVTIMEO`
socket option set during `Open`.

```
Receive(ptRoot, ptMsg):
  Verify(ptRoot)                                       // JUNO_ASSERT_EXISTS(ptRoot)
  JUNO_ASSERT_EXISTS(ptMsg)                            // output buffer must be non-NULL
  if ptRoot->_iSockFd == -1:
    invoke failure handler(JUNO_STATUS_INVALID_REF_ERROR)
    return JUNO_STATUS_INVALID_REF_ERROR               // socket not open

  bytes_recv = recvfrom(
    (int)ptRoot->_iSockFd,
    ptMsg,
    sizeof(UDP_THREAD_MSG_T),                          // exactly 76 bytes requested
    0,
    NULL,                                              // source address not captured
    NULL
  )

  if bytes_recv < 0:
    if errno == EAGAIN or errno == EWOULDBLOCK:
      // timeout elapsed; this is a normal expected condition
      return JUNO_STATUS_TIMEOUT                       // NOT an error; failure handler is NOT invoked
    else:
      invoke failure handler(JUNO_STATUS_ERROR)
      return JUNO_STATUS_ERROR                         // unexpected socket error

  if bytes_recv != (ssize_t)sizeof(UDP_THREAD_MSG_T):
    invoke failure handler(JUNO_STATUS_ERROR)
    return JUNO_STATUS_ERROR                           // wrong datagram size; discard and signal error

  return JUNO_STATUS_SUCCESS
```

The timeout path (`EAGAIN`/`EWOULDBLOCK`) returns `JUNO_STATUS_TIMEOUT` without
invoking the failure handler. Timeout is a normal, anticipated condition for a
receiver thread polling for messages; it must not be treated as a diagnostic event.
All other negative-return paths invoke the failure handler before returning
`JUNO_STATUS_ERROR`.

---

### 3.4.4 Close (REQ-UDP-009)

`Close` releases the OS socket resource and resets `_iSockFd` to the invalid sentinel.
If the socket is already closed (descriptor is `-1`), `Close` returns
`JUNO_STATUS_SUCCESS` without invoking the failure handler. This makes `Close`
idempotent and safe to call from an `OnExit` handler even if `Open` was never called.

```
Close(ptRoot):
  Verify(ptRoot)                                       // JUNO_ASSERT_EXISTS(ptRoot)
  if _iSockFd == -1:
    return JUNO_STATUS_SUCCESS                         // already closed — idempotent

  result = close((int)ptRoot->_iSockFd)
  ptRoot->_iSockFd = -1                               // reset sentinel regardless of close() result

  if result < 0:
    invoke failure handler(JUNO_STATUS_ERROR)
    return JUNO_STATUS_ERROR

  return JUNO_STATUS_SUCCESS
```

The `_iSockFd` reset to `-1` is performed unconditionally before inspecting the
`close()` return value. This prevents double-close bugs even if the caller ignores the
error return: the descriptor is invalidated the moment `close()` is called.

---

## 3.5 Error Handling Strategy

// @{"design": ["REQ-UDP-012", "REQ-UDP-013"]}

All four vtable operations and the `Init` function return `JUNO_STATUS_T` (a `int32_t`
typedef). A return value of `JUNO_STATUS_SUCCESS` (0) indicates success; any other
value indicates failure.

Error handling rules:

1. **NULL pointer guards** — `JUNO_ASSERT_EXISTS(ptr)` checks every pointer argument
   before use. On NULL, it invokes the failure handler (if non-NULL) with
   `JUNO_STATUS_NULLPTR_ERROR` and returns that status to the caller.

2. **Failure handler invocation** — The failure handler is invoked immediately before
   returning any non-success status. The handler receives the error status and the
   user data pointer stored in the root. The handler is **diagnostic only** — it must
   never modify control flow, call `longjmp`, or throw exceptions.

3. **Timeout distinguished from error** — `JUNO_STATUS_TIMEOUT` is returned by
   `Receive` when `recvfrom` returns `EAGAIN`/`EWOULDBLOCK`. This is the only
   non-success status that does **not** invoke the failure handler, because timeout is
   an expected, normal operating condition rather than a fault.

4. **Socket state guard** — Operations that require an open socket (`Send`, `Receive`,
   `Close`) check `_iSockFd != -1` before proceeding. If the socket is not open, they
   invoke the failure handler with `JUNO_STATUS_INVALID_REF_ERROR`.

5. **No silent swallowing** — Every error path from a POSIX call that cannot be
   recovered from is propagated upward with a non-success status.

Status code summary:

| Status Code | Meaning | Failure Handler Invoked |
|-------------|---------|------------------------|
| `JUNO_STATUS_SUCCESS` | Operation succeeded | No |
| `JUNO_STATUS_NULLPTR_ERROR` | A required pointer argument was NULL | Yes |
| `JUNO_STATUS_INVALID_REF_ERROR` | Socket descriptor in invalid state | Yes |
| `JUNO_STATUS_ERROR` | POSIX call failure (socket, bind, connect, send, recv) | Yes |
| `JUNO_STATUS_TIMEOUT` | `recvfrom` returned `EAGAIN`/`EWOULDBLOCK` | No |

---

## 3.6 Interface Header Constraints

// @{"design": ["REQ-UDP-010", "REQ-UDP-011", "REQ-UDP-014"]}

The public interface header (`udp_api.h`) is restricted to freestanding-safe includes
only. POSIX and OS-specific headers are confined exclusively to the Linux implementation
translation unit.

### Permitted includes in `udp_api.h`

```c
#include <stdint.h>    /* uint32_t, uint8_t, uint16_t, intptr_t */
#include <stddef.h>    /* size_t, NULL */
#include <stdbool.h>   /* bool */
#include "juno/status.h"
#include "juno/module.h"
#include "juno/types.h"
```

### Forbidden includes in `udp_api.h`

The following headers must **never** appear in the interface header:

- `<sys/socket.h>` — POSIX socket API
- `<netinet/in.h>` — `sockaddr_in`, `in_addr`
- `<arpa/inet.h>` — `inet_addr`, `htons`
- `<unistd.h>` — `close`
- `<errno.h>` — `errno`, `EAGAIN`, `EWOULDBLOCK`
- Any platform-specific header

### Two-layer separation

| Layer | File Type | Compiled With | POSIX Headers | Dynamic Allocation |
|-------|-----------|---------------|---------------|--------------------|
| Interface | `.h` (C11) | `-nostdlib -ffreestanding` | Forbidden | Forbidden |
| Implementation | `.cpp` (C++11) | Host Linux toolchain | Allowed (required) | Forbidden |

This table enforces that the interface header can be included in any freestanding
translation unit (bare-metal, RTOS, embedded kernel) without modification. The
implementation layer is Linux-specific by design; porting requires a new `.cpp` file,
not any change to the interface.

The prohibition on dynamic allocation (`malloc`/`calloc`/`realloc`/`free`) applies to
**both** layers. The POSIX implementation uses only stack-local `struct sockaddr_in`
and `struct timeval` variables; all module state lives in the caller-owned root.

---

## 3.7 Memory Ownership

// @{"design": ["REQ-UDP-001", "REQ-UDP-010"]}

All memory is caller-owned and injected. The module allocates nothing.

| Object | Allocator | Lifetime Requirement | Module Access |
|--------|-----------|---------------------|---------------|
| `JUNO_UDP_ROOT_T` | Caller (stack or static) | Must outlive all API calls on this root | Read/write (stores vtable ptr, failure handler, socket descriptor) |
| `JUNO_UDP_API_T` (vtable) | Implementation or test (static) | Must outlive all API calls on any root using it | Read-only (function pointers dispatched, never modified) |
| `JUNO_UDP_CFG_T` | Caller (stack or static) | Must be valid only for the duration of the `Open` call | Read-only (fields read during Open, no pointer stored after return) |
| `UDP_THREAD_MSG_T` (Send) | Caller | Must be valid only for the duration of the `Send` call | Read-only (bytes copied into datagram buffer by OS) |
| `UDP_THREAD_MSG_T` (Receive) | Caller | Must be valid only for the duration of the `Receive` call | Write-only (OS writes received bytes into the caller's buffer) |

No module-internal buffers of any kind are used. The module holds zero allocated
memory between calls. The only persistent state is `_iSockFd` (an integer) stored
inside the caller-allocated root.

Lifetime diagram:

```
Caller scope:
  JUNO_UDP_ROOT_T tRoot;                 // allocated here; lifetime = enclosing scope
  JUNO_UDP_CFG_T  tCfg = { ... };        // allocated here; only needed until Open returns
  UDP_THREAD_MSG_T tMsg;                  // allocated here; reused per Send/Receive call

  JunoUdp_Init(&tRoot, &s_tLinuxApi, NULL, NULL);
  tRoot.ptApi->Open(&tRoot, &tCfg);      // tCfg may go out of scope after this line
  tRoot.ptApi->Send(&tRoot, &tMsg);      // tMsg only needs to be valid during this call
  tRoot.ptApi->Receive(&tRoot, &tMsg);   // module writes into tMsg; tMsg is caller-owned
  tRoot.ptApi->Close(&tRoot);
  // tRoot goes out of scope here; _iSockFd is already -1 (closed)
```
