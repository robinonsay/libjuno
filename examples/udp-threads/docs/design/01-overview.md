> Part of: [Software Design Document](index.md) — Sections 1-2

# UDP Threads Example — Software Design Document

**Date:** 2026-04-19
**Module:** UDPAPP / UDP / THREAD
**Requirements files:**
- `examples/udp-threads/requirements/app/requirements.json`
- `examples/udp-threads/requirements/udp/requirements.json`
- `examples/udp-threads/requirements/thread/requirements.json`

---

// @{"design": ["REQ-UDPAPP-001", "REQ-UDPAPP-002", "REQ-UDPAPP-003", "REQ-UDPAPP-004", "REQ-UDPAPP-005", "REQ-UDPAPP-006", "REQ-UDPAPP-007", "REQ-UDPAPP-008", "REQ-UDPAPP-009", "REQ-UDPAPP-010", "REQ-UDPAPP-011", "REQ-UDPAPP-012", "REQ-UDPAPP-013", "REQ-UDPAPP-014", "REQ-UDPAPP-015", "REQ-UDPAPP-016", "REQ-UDPAPP-017", "REQ-UDPAPP-018", "REQ-UDPAPP-019", "REQ-UDPAPP-020", "REQ-UDPAPP-021", "REQ-UDPAPP-022", "REQ-UDPAPP-023"]}
## 1. Overview

The `udp-threads` example demonstrates LibJuno's vtable/DI pattern applied to a realistic
inter-thread communication scenario. It uses two independent OS threads, a per-thread cyclic
scheduler (`JUNO_SCH_T`), a per-thread software bus broker (`JUNO_SB_BROKER_T`), and a UDP
socket module (`JUNO_UDP_T`) as the inter-thread transport. The composition root wires all
module instances together using LibJuno's caller-owns-memory, vtable-injected dependency model.

### 1.1 Requirements in Scope

| Requirement ID | Title |
|----------------|-------|
| REQ-UDP-001 | UDP Module Root Structure |
| REQ-UDP-002 | UDP API Vtable Interface |
| REQ-UDP-003 | Open Operation |
| REQ-UDP-004 | Receiver Socket Bind |
| REQ-UDP-005 | Sender Socket Connect |
| REQ-UDP-006 | Send Operation |
| REQ-UDP-007 | Receive Operation Blocking |
| REQ-UDP-008 | Receive Timeout Status |
| REQ-UDP-009 | Close Operation |
| REQ-UDP-010 | No Dynamic Memory Allocation |
| REQ-UDP-011 | Freestanding Interface Header |
| REQ-UDP-012 | API Error Status Return |
| REQ-UDP-013 | Failure Handler Invocation on Error |
| REQ-UDP-014 | Linux POSIX Implementation |
| REQ-UDP-015 | Fixed Datagram Size |
| REQ-UDP-016 | Module Initialization |
| REQ-UDP-017 | Socket Configuration Structure |
| REQ-UDP-018 | Message Structure Definition |
| REQ-THREAD-001 | Thread Module Root Structure |
| REQ-THREAD-002 | Thread API Vtable Interface |
| REQ-THREAD-003 | Create Operation |
| REQ-THREAD-004 | Single Thread Per Root |
| REQ-THREAD-005 | Join Operation |
| REQ-THREAD-006 | Stop Operation |
| REQ-THREAD-007 | Stop Flag Readable by Thread Entry |
| REQ-THREAD-008 | No Dynamic Allocation |
| REQ-THREAD-009 | Freestanding Interface — No Platform Headers |
| REQ-THREAD-010 | Error Status Return |
| REQ-THREAD-011 | Linux Pthreads Implementation |
| REQ-THREAD-012 | Module Initialization |
| REQ-THREAD-013 | Failure Handler Invocation on Error |
| REQ-THREAD-014 | Allowed Interface Header Dependencies |
| REQ-THREAD-015 | Error on Double Create |
| REQ-UDPAPP-001 | Example Project Overview |
| REQ-UDPAPP-002 | SenderApp Lifecycle Interface |
| REQ-UDPAPP-003 | SenderApp OnStart — Open UDP Sender Socket |
| REQ-UDPAPP-004 | SenderApp OnProcess — Publish to Thread 1 Broker |
| REQ-UDPAPP-005 | SenderApp OnProcess — Transmit via UDP |
| REQ-UDPAPP-006 | SenderApp Sequence Counter |
| REQ-UDPAPP-007 | SenderApp OnExit — Close UDP Sender Socket |
| REQ-UDPAPP-008 | MonitorApp Lifecycle Interface |
| REQ-UDPAPP-009 | MonitorApp OnStart — Register Subscription |
| REQ-UDPAPP-010 | MonitorApp OnProcess — Dequeue and Log Messages |
| REQ-UDPAPP-011 | UdpBridgeApp Lifecycle Interface |
| REQ-UDPAPP-012 | UdpBridgeApp OnStart — Open UDP Receiver Socket |
| REQ-UDPAPP-013 | UdpBridgeApp OnProcess — Receive UDP Datagram |
| REQ-UDPAPP-014 | UdpBridgeApp OnProcess — Publish to Thread 2 Broker |
| REQ-UDPAPP-015 | UdpBridgeApp OnExit — Close UDP Receiver Socket |
| REQ-UDPAPP-016 | ProcessorApp Lifecycle Interface |
| REQ-UDPAPP-017 | ProcessorApp OnStart — Register Subscription |
| REQ-UDPAPP-018 | ProcessorApp OnProcess — Dequeue and Process Messages |
| REQ-UDPAPP-019 | Composition Root — Static Allocation |
| REQ-UDPAPP-020 | Composition Root — Two-Thread Topology |
| REQ-UDPAPP-021 | Composition Root — Scheduler Configuration |
| REQ-UDPAPP-022 | Composition Root — Broker Isolation |
| REQ-UDPAPP-023 | Composition Root — Graceful Shutdown |

---

// @{"design": ["REQ-UDPAPP-001", "REQ-UDPAPP-019", "REQ-UDP-011", "REQ-UDP-012", "REQ-UDP-013", "REQ-UDP-014", "REQ-THREAD-009", "REQ-THREAD-010", "REQ-THREAD-011", "REQ-THREAD-013", "REQ-THREAD-014"]}
## 2. Design Approach

### 2.1 Technology Stack

| Layer | Technology |
|-------|-----------|
| Interface layer | C11 freestanding headers (`<stdint.h>`, `<stddef.h>`, `<stdbool.h>`, `juno/status.h`, `juno/module.h`, `juno/types.h`) |
| Implementation layer | C++ translation units (.cpp); Linux/POSIX-specific OS headers confined here |
| Scheduling | LibJuno `JUNO_SCH_T` cyclic scheduler — one instance per thread |
| Pub/sub messaging | LibJuno `JUNO_SB_BROKER_T` software bus broker — one instance per thread |
| Application interface | LibJuno `JUNO_APP_T` — all four applications implement `JUNO_APP_API_T` |
| Thread lifecycle | POSIX pthreads (`pthread_create`, `pthread_join`) wrapped by `JUNO_THREAD_T` |
| UDP transport | POSIX sockets (`socket`, `bind`, `sendto`, `recvfrom`, `close`) wrapped by `JUNO_UDP_T` |
| Network path | Fixed loopback address 127.0.0.1, fixed port 9000 |
| Message type | `UDP_THREAD_MSG_T` — compile-time fixed layout, 76 bytes total |

### 2.2 Architecture Overview

The design uses two layers. The **freestanding C11 interface layer** defines module roots,
vtable structs, and public API headers that contain no platform-specific types. This layer
is portable to any embedded target. The **C++ implementation layer** contains Linux/POSIX
translation units that satisfy the vtable contracts using `pthread.h` and POSIX socket APIs.
OS-specific headers never appear in the public interface, satisfying REQ-UDP-011,
REQ-THREAD-009, and REQ-THREAD-014.

```
Thread 1                              Thread 2
┌─────────────────────────┐          ┌──────────────────────────────┐
│ SenderApp               │          │ UdpBridgeApp                 │
│  - build UDP_THREAD_MSG │ ──UDP──► │  - receive UDP datagram      │
│  - publish to Broker1   │          │  - publish to Broker2        │
│  - send via UDP module  │          │                              │
├─────────────────────────┤          ├──────────────────────────────┤
│ MonitorApp              │          │ ProcessorApp                 │
│  - subscribe Broker1    │          │  - subscribe Broker2         │
│  - log messages         │          │  - process messages          │
└─────────────────────────┘          └──────────────────────────────┘
         ▲                                       ▲
  JUNO_SCH_T (Thread 1)              JUNO_SCH_T (Thread 2)
  JUNO_SB_BROKER_T (Broker 1)       JUNO_SB_BROKER_T (Broker 2)
  JUNO_THREAD_ROOT_T (Thread 1)     JUNO_THREAD_ROOT_T (Thread 2)
```

SenderApp and MonitorApp share Thread 1's scheduler and broker. SenderApp builds a
`UDP_THREAD_MSG_T`, publishes it to Broker 1 (so MonitorApp can observe it locally), and
transmits it via the UDP module. UdpBridgeApp on Thread 2 receives the datagram and publishes
it to Broker 2. ProcessorApp subscribes to Broker 2 and processes each received message,
completing the end-to-end inter-thread data path.

### 2.3 Module Dependency Graph

```
UDPAPP (composition root)
  ├── JUNO_THREAD_T (thread lifecycle)
  ├── JUNO_SCH_T (cyclic scheduling)
  ├── JUNO_SB_BROKER_T (pub/sub bus)
  ├── JUNO_APP_T (application interface)
  └── JUNO_UDP_T (UDP transport)
```

The composition root owns all module instances (stack or static). It initializes each module
by passing caller-allocated storage and injecting vtable pointers. No module allocates any
resource on behalf of the caller.

### 2.4 Design Constraints

The following hard constraints govern all design decisions in this example:

1. **No dynamic memory allocation** — `malloc`, `calloc`, `realloc`, and `free` are
   prohibited everywhere. All module instances are stack- or statically-allocated and owned
   by the composition root (REQ-UDP-010, REQ-THREAD-008, REQ-UDPAPP-019).

2. **Freestanding C11 interface headers** — UDP and THREAD public headers include only
   `<stdint.h>`, `<stddef.h>`, `<stdbool.h>`, `juno/status.h`, `juno/module.h`, and
   `juno/types.h`. No POSIX or OS-specific headers appear in the public API
   (REQ-UDP-011, REQ-THREAD-009, REQ-THREAD-014).

3. **C++ implementation layer** — Linux pthreads and POSIX socket code lives exclusively
   in `.cpp` translation units. This is the only place OS headers (`pthread.h`,
   `sys/socket.h`, etc.) appear (REQ-UDP-014, REQ-THREAD-011).

4. **Caller-owned memory** — all module root structs are allocated by the caller (the
   composition root) and passed by pointer. Modules never own their own storage.

5. **`JUNO_STATUS_T` error reporting** — every fallible API operation returns
   `JUNO_STATUS_T`. Callers inspect this value; they do not query out-of-band state
   (REQ-UDP-012, REQ-THREAD-010).

6. **Failure handler invocation on error** — when an API operation encounters an error,
   the module invokes the failure handler stored in the module root before returning the
   error status. The failure handler is diagnostic only and does not alter control flow
   (REQ-UDP-013, REQ-THREAD-013).

### 2.5 Alternatives Considered

**Shared memory between threads** — Using a shared ring buffer guarded by a mutex would
eliminate the UDP socket overhead. This approach was not selected because it requires
synchronization primitives (mutex, condition variable) that are outside the scope of
LibJuno's current module set. The UDP transport is self-contained and demonstrates a wider
range of LibJuno modules.

**TCP instead of UDP** — TCP provides reliable, ordered delivery and would eliminate the
need to handle dropped datagrams. It was not selected because connection-state management
(connect/accept lifecycle, half-close handling) adds complexity unnecessary for a loopback
path where packet loss does not occur in practice. UDP Send/Receive with a fixed datagram
size is simpler to implement and easier to understand as an example.

**Global variables for module instances** — Placing module roots in global variables would
simplify the composition root. This approach was rejected because it violates LibJuno's
no-global-mutable-state principle, which ensures that all state is explicitly visible as
caller-owned memory and dependency-injected, enabling testability and portability.
