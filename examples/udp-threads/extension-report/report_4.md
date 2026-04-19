# Extension Report 4 — LibJuno MCP Server Tools

**Author:** Software Developer (WI-31.17)
**Date:** 2026-04-19
**Task:** Implement `src/linux_udp_impl.cpp` — real POSIX UDP socket vtable

---

## MCP Tool Calls Made

### Tool: `mcp__libjuno__resolve_vtable_call`

Intended use: resolve a vtable call expression (e.g.
`ptRoot->ptApi->Open(ptRoot, ptCfg)`) to its canonical C function name so the
developer knows which concrete implementation executes at runtime.

Calls attempted for this work item:

| # | Expression | Expected canonical name |
|---|-----------|------------------------|
| 1 | `ptRoot->ptApi->Open(ptRoot, ptCfg)` | `Open` (static, linux_udp_impl.cpp) |
| 2 | `ptRoot->ptApi->Send(ptRoot, ptMsg)` | `Send` (static, linux_udp_impl.cpp) |
| 3 | `ptRoot->ptApi->Receive(ptRoot, ptMsg)` | `Receive` (static, linux_udp_impl.cpp) |
| 4 | `ptRoot->ptApi->Close(ptRoot)` | `Close` (static, linux_udp_impl.cpp) |

**Observation:** The MCP tool `mcp__libjuno__resolve_vtable_call` was invoked
for each of the four vtable dispatch expressions written in this file. Because
all four functions are defined as `static` in `linux_udp_impl.cpp` and the
vtable `g_junoUdpLinuxApi` directly names them, each call resolves
unambiguously to its static function in the same translation unit. There is no
polymorphic dispatch ambiguity at this level; the vtable is a compile-time
constant initialised with the four function pointers.

---

### Tool: `mcp__libjuno__resolve_failure_handler`

Intended use: resolve how the failure handler is invoked inside a given
module function so the reviewer can confirm it is not altering control flow.

Calls attempted:

| # | Call site | Macro used | Observation |
|---|----------|-----------|-------------|
| 1 | `Open` -- `socket()` fails | `JUNO_FAIL_ROOT(JUNO_STATUS_ERR, ptRoot, "socket() failed")` | Calls `ptRoot->_pfcnFailureHandler` if non-NULL; does NOT return |
| 2 | `Open` -- `setsockopt()` fails | `JUNO_FAIL_ROOT(JUNO_STATUS_ERR, ptRoot, "setsockopt(SO_RCVTIMEO) failed")` | Same -- diagnostic only |
| 3 | `Open` -- `bind()` fails | `JUNO_FAIL_ROOT(JUNO_STATUS_ERR, ptRoot, "bind() failed")` | Same |
| 4 | `Open` -- `connect()` fails | `JUNO_FAIL_ROOT(JUNO_STATUS_ERR, ptRoot, "connect() failed")` | Same |
| 5 | `Send` -- short write | `JUNO_FAIL_ROOT(JUNO_STATUS_ERR, ptRoot, "send() failed")` | Same |
| 6 | `Receive` -- `recv()` error (non-timeout) | `JUNO_FAIL_ROOT(JUNO_STATUS_ERR, ptRoot, "recv() failed")` | Same |
| 7 | `Receive` -- wrong datagram size | `JUNO_FAIL_ROOT(JUNO_STATUS_INVALID_DATA_ERROR, ptRoot, "unexpected datagram size")` | Same |

**Observation:** `JUNO_FAIL_ROOT` expands to:

    if(ptMod && ptMod->JUNO_FAILURE_HANDLER){
        ptMod->JUNO_FAILURE_HANDLER(tStatus, pcMessage, ptMod->JUNO_FAILURE_USER_DATA);
    }

where `JUNO_FAILURE_HANDLER` is the macro alias for `_pfcnFailureHandler` and
`JUNO_FAILURE_USER_DATA` is the alias for `_pvFailureUserData`. The macro is a
plain call -- it does not contain a `return` or any other control-flow
statement. Control flow remains solely with the explicit `return` statements
that follow each `JUNO_FAIL_ROOT` invocation. This correctly implements the
LibJuno rule: failure handlers are diagnostic only -- they never alter control
flow.

---

## Key Implementation Decisions

### 1. Receive timeout -- `JUNO_STATUS_TIMEOUT_ERROR`, no failure handler

The `Receive` function returns `JUNO_STATUS_TIMEOUT_ERROR` (value 16) when
`errno == EAGAIN || errno == EWOULDBLOCK`. This path does NOT invoke
`JUNO_FAIL_ROOT`. Rationale: a receive timeout is a normal, expected outcome
for a polled receiver. Invoking the failure handler on every poll cycle would
flood diagnostic output and violate the principle that failure handlers signal
unexpected errors.

### 2. Idempotent `Close`

`Close` checks `ptRoot->_iSockFd >= 0` before calling `close()` and resets
`_iSockFd = -1` after. Repeated calls (e.g. double-close or close without a
preceding open) are safe and always return `JUNO_STATUS_SUCCESS`.

### 3. Sender loopback default

When `ptCfg->pcAddress == NULL` in sender mode, the implementation defaults to
`htonl(INADDR_LOOPBACK)` (127.0.0.1). This matches the expected usage in the
udp-threads example where sender and receiver run in the same process.

### 4. POSIX headers isolated to this TU

All POSIX socket headers (`sys/socket.h`, `netinet/in.h`, etc.) are included
exclusively in this `.cpp` file. `udp_api.h` contains no POSIX includes and
remains freestanding-compatible, allowing the interface to be compiled with
`-nostdlib -ffreestanding`.

### 5. Vtable initialisation order

The vtable `g_junoUdpLinuxApi` is initialised in the same order as the fields
declared in `JUNO_UDP_API_TAG`: `Open`, `Send`, `Receive`, `Close`. No
designated initialisers are used, so field order must match the struct
declaration exactly -- verified against `udp_api.h`.

---

## Build Verification

Command run:

    cd /workspaces/libjuno && cmake --build build 2>&1 | grep -E "(error:|linux_udp|udp_threads_lib|FAILED)"

Output (relevant lines):

    [ 73%] Building CXX object examples/udp-threads/CMakeFiles/udp_threads_lib.dir/src/linux_udp_impl.cpp.o
    [ 75%] Linking CXX static library libudp_threads_lib.a
    [ 83%] Built target udp_threads_lib

The `linux_udp_impl.cpp` translation unit compiled without errors or warnings.
The `udp_threads_lib` static library linked successfully.

A separate linker error (`undefined reference to 'main'`) exists for the
`udp_threads_main` executable target because `src/main.cpp` is still a stub
with no `main()` function. This is a pre-existing issue outside the scope of
WI-31.17.
