> Part of: [Software Design Document](index.md) — Section 5

# Section 5: Application Designs

// @{"design": ["REQ-UDPAPP-002", "REQ-UDPAPP-008", "REQ-UDPAPP-011", "REQ-UDPAPP-016"]}
## 5.1 Overview

The `udp-threads` example defines four application types, each implementing the LibJuno
`JUNO_APP_API_T` vtable. Two apps run on Thread 1 and two run on Thread 2:

| App | Thread | Role |
|-----|--------|------|
| `SenderApp` | Thread 1 | Builds and transmits `UDP_THREAD_MSG_T` messages over UDP; publishes locally to Thread 1's broker |
| `MonitorApp` | Thread 1 | Subscribes to Thread 1's broker; logs messages produced by SenderApp |
| `UdpBridgeApp` | Thread 2 | Receives UDP datagrams from the network; publishes each received message to Thread 2's broker |
| `ProcessorApp` | Thread 2 | Subscribes to Thread 2's broker; processes messages bridged from Thread 1 via UDP |

All four apps share the same structural pattern:

- All implement `JUNO_APP_API_T` (`OnStart`, `OnProcess`, `OnExit`).
- All embed `JUNO_APP_ROOT_T` as the **first** member of their concrete struct, enabling
  safe upcast from `JUNO_APP_ROOT_T *` to the concrete app pointer.
- All receive their dependencies (UDP module instance, broker instance) as injected pointers
  at `Init` time. No app allocates any resource itself.
- All app struct instances are stack- or statically-allocated by the composition root.

### 5.1.1 UDPTH_MSG_MID

`UDPTH_MSG_MID` is a compile-time constant of the message identifier type used by the
LibJuno software bus broker (`JUNO_SB_BROKER_T`). It identifies the single message topic
shared by all four apps across both threads. SenderApp publishes under `UDPTH_MSG_MID` on
Broker 1; MonitorApp subscribes to `UDPTH_MSG_MID` on Broker 1. UdpBridgeApp publishes
under `UDPTH_MSG_MID` on Broker 2; ProcessorApp subscribes to `UDPTH_MSG_MID` on Broker 2.
Because Thread 1 and Thread 2 each have their own independent broker instance, using the
same constant on both threads does not create cross-thread coupling.

---

// @{"design": ["REQ-UDPAPP-002", "REQ-UDPAPP-003", "REQ-UDPAPP-004", "REQ-UDPAPP-005", "REQ-UDPAPP-006", "REQ-UDPAPP-007"]}
## 5.2 SenderApp

SenderApp runs on Thread 1. On each scheduler cycle it builds a `UDP_THREAD_MSG_T` with a
monotonically incrementing sequence counter, publishes it to Thread 1's broker so MonitorApp
can observe it locally, and transmits it via the UDP module to the loopback address so
UdpBridgeApp on Thread 2 can receive it.

### 5.2.1 Struct Definition

```c
struct SENDER_APP_TAG {
    JUNO_APP_ROOT_T        tRoot;      /* MUST be first — upcast target */
    JUNO_UDP_ROOT_T       *ptUdp;      /* injected: UDP module (sender role) */
    JUNO_SB_BROKER_ROOT_T *ptBroker;  /* injected: Thread 1's broker */
    uint32_t               _uSeqNum;  /* internal: monotonic sequence counter */
};
typedef struct SENDER_APP_TAG SENDER_APP_T;
```

`tRoot` must be the first member. The scheduler holds a `JUNO_APP_ROOT_T *` and dispatches
`ptApp->ptApi->OnProcess(ptApp)`. The concrete app recovers its full state by casting:
`SENDER_APP_T *ptSender = (SENDER_APP_T *)ptApp`.

`_uSeqNum` is private state (leading underscore per naming convention). It is initialized
to zero in `OnStart` and incremented before each transmission in `OnProcess`.

### 5.2.2 Init Function

```c
JUNO_STATUS_T SenderApp_Init(
    SENDER_APP_T              *ptApp,
    const JUNO_APP_API_T      *ptApi,             /* vtable */
    JUNO_UDP_ROOT_T           *ptUdp,             /* UDP module (sender role) */
    JUNO_SB_BROKER_ROOT_T     *ptBroker,          /* Thread 1's broker */
    JUNO_FAILURE_HANDLER_T     pfcnFailureHandler, /* diagnostic failure callback */
    void                      *pvFailureUserData   /* user data passed to failure handler */
);
```

`Init` wires `ptApp->tRoot.ptApi = ptApi`, stores `ptUdp` and `ptBroker`, stores the
failure handler and user data into `tRoot`, initializes `_uSeqNum = 0`, and verifies that
no injected pointer is NULL. All caller-allocated storage; `SenderApp_Init` does not
allocate any memory.

### 5.2.3 Lifecycle Algorithms

**OnStart** (REQ-UDPAPP-003)

Opens the UDP sender socket and initializes the sequence counter:

```
JUNO_UDP_CFG_T tCfg = {
    .pcAddress    = "127.0.0.1",
    .uPort        = 9000,
    .uTimeoutMs   = 0,
    .bIsReceiver  = false
};
ptUdp->ptApi->Open(ptUdp, &tCfg);
_uSeqNum = 0;
```

`bIsReceiver = false` causes the platform implementation to call `connect()` rather than
`bind()`, directing outgoing datagrams to 127.0.0.1:9000.

**OnProcess** (REQ-UDPAPP-004, REQ-UDPAPP-005, REQ-UDPAPP-006)

Builds the message, publishes it locally, then transmits it via UDP:

```
UDP_THREAD_MSG_T tMsg;
tMsg.uSeqNum         = ++ptSender->_uSeqNum;
tMsg.uTimestampSec   = 0;   /* populated from time module when available */
tMsg.uTimestampSubSec = 0;
memset(tMsg.arrPayload, 0, sizeof(tMsg.arrPayload));

/* Wrap &tMsg in a fat pointer (JUNO_POINTER_T) before publishing */
JUNO_POINTER_T tPtr = JunoMemory_PointerInit(ptPointerApi, UDP_THREAD_MSG_T, &tMsg);
tStatus = ptBroker->ptApi->Publish(ptBroker, UDPTH_MSG_MID, tPtr);
if (tStatus != JUNO_STATUS_SUCCESS) return tStatus;
tStatus = ptUdp->ptApi->Send(ptUdp, &tMsg);
/* Note: If Publish succeeds but Send subsequently fails, Broker 1 will have received
   the message but Thread 2 will not. This is acceptable behavior for this example;
   a production implementation might consider rollback or retry semantics. */
```

`tMsg` is stack-allocated on each invocation. The pre-increment (`++_uSeqNum`) ensures the
sequence number starts at 1 and increases monotonically. `Publish` receives a `JUNO_POINTER_T`
fat pointer wrapping `&tMsg`; the broker copies the message data internally via the pointer
API's `Copy` operation. `Send` receives the raw `&tMsg` pointer.

**OnExit** (REQ-UDPAPP-007)

Closes the UDP sender socket:

```
ptUdp->ptApi->Close(ptUdp);
```

---

// @{"design": ["REQ-UDPAPP-008", "REQ-UDPAPP-009", "REQ-UDPAPP-010"]}
## 5.3 MonitorApp

MonitorApp runs on Thread 1. It subscribes to Thread 1's broker and, on each cycle,
dequeues and logs all messages that SenderApp has published during that cycle.

### 5.3.1 Struct Definition

```c
struct MONITOR_APP_TAG {
    JUNO_APP_ROOT_T           tRoot;     /* MUST be first */
    JUNO_SB_BROKER_ROOT_T    *ptBroker;  /* injected: Thread 1's broker */
    JUNO_SB_PIPE_T            tPipe;     /* subscription pipe — embedded, no heap */
};
typedef struct MONITOR_APP_TAG MONITOR_APP_T;
```

`JUNO_SB_PIPE_T` is the subscription pipe type from the LibJuno software bus module. It is
embedded directly in the app struct — no separate allocation is needed. The broker does not
own the pipe; the app struct owns it (caller-owned memory model). The broker stores a
pointer to `tPipe` after registration in `OnStart`.

### 5.3.2 Init Function

```c
JUNO_STATUS_T MonitorApp_Init(
    MONITOR_APP_T              *ptApp,
    const JUNO_APP_API_T       *ptApi,             /* vtable */
    JUNO_SB_BROKER_ROOT_T      *ptBroker,          /* Thread 1's broker */
    JUNO_FAILURE_HANDLER_T      pfcnFailureHandler,
    void                       *pvFailureUserData
);
```

`Init` wires the vtable, stores `ptBroker`, stores the failure handler, and verifies all
pointers. The pipe (`tPipe`) is zero-initialized by the caller's static or stack allocation;
`MonitorApp_Init` does not populate it — that is deferred to `OnStart`.

### 5.3.3 Lifecycle Algorithms

**OnStart** (REQ-UDPAPP-009)

Initializes the subscription pipe for `UDPTH_MSG_MID` and registers it with Thread 1's broker:

```
/* Step 1: Initialize the pipe, associating it with UDPTH_MSG_MID and its backing array */
JunoSb_PipeInit(&tPipe, UDPTH_MSG_MID, ptPipeArray, pfcnFailureHandler, pvFailureUserData);

/* Step 2: Register the initialized pipe with the broker */
ptBroker->ptApi->RegisterSubscriber(ptBroker, &tPipe);
```

After these calls the broker will enqueue a copy of every message published under
`UDPTH_MSG_MID` into `ptMonitor->tPipe`. `ptPipeArray` is a `JUNO_DS_ARRAY_ROOT_T *`
injected at `Init` time and must outlive the pipe registration.

**OnProcess** (REQ-UDPAPP-010)

Dequeues and processes all available messages in the subscription pipe:

```
loop:
    UDP_THREAD_MSG_T tMsg;
    JUNO_POINTER_T tReturn = JunoMemory_PointerInit(ptPointerApi, UDP_THREAD_MSG_T, &tMsg);
    JUNO_STATUS_T tStatus = tPipe.tRoot.ptApi->Dequeue(&tPipe.tRoot, tReturn);
    if (tStatus == JUNO_STATUS_OOB_ERROR) break;   /* empty queue — drain complete */
    if (tStatus != JUNO_STATUS_SUCCESS) return tStatus;
    /* process/log tMsg — e.g., print uSeqNum, uTimestampSec */
```

`tMsg` is stack-allocated on each loop iteration. Dequeue is performed directly on the
pipe's embedded queue via `tPipe.tRoot.ptApi->Dequeue`. The loop exits on `JUNO_STATUS_OOB_ERROR`,
which is the queue's normal signal that the pipe is drained. Any other non-success status
is propagated to the scheduler.

**OnExit**

No resources to release. The broker manages the pipe registration lifetime; the embedded
`tPipe` is released automatically when the app struct goes out of scope (or the process exits).

---

// @{"design": ["REQ-UDPAPP-011", "REQ-UDPAPP-012", "REQ-UDPAPP-013", "REQ-UDPAPP-014", "REQ-UDPAPP-015"]}
## 5.4 UdpBridgeApp

UdpBridgeApp runs on Thread 2. It opens a UDP receiver socket bound to port 9000 and on
each cycle attempts to receive one datagram. If a datagram arrives it is published to
Thread 2's broker. If the receive times out the app returns success immediately without
publishing. This bridges the inter-thread UDP path into the Thread 2 software bus.

### 5.4.1 Struct Definition

```c
struct UDP_BRIDGE_APP_TAG {
    JUNO_APP_ROOT_T        tRoot;      /* MUST be first */
    JUNO_UDP_ROOT_T       *ptUdp;      /* injected: UDP module (receiver role) */
    JUNO_SB_BROKER_ROOT_T *ptBroker;  /* injected: Thread 2's broker */
};
typedef struct UDP_BRIDGE_APP_TAG UDP_BRIDGE_APP_T;
```

UdpBridgeApp holds no internal mutable state beyond the injected pointers. The sequence
number and timestamp are carried inside the received `UDP_THREAD_MSG_T`; UdpBridgeApp
forwards them unchanged to Broker 2.

### 5.4.2 Init Function

```c
JUNO_STATUS_T UdpBridgeApp_Init(
    UDP_BRIDGE_APP_T              *ptApp,
    const JUNO_APP_API_T          *ptApi,             /* vtable */
    JUNO_UDP_ROOT_T               *ptUdp,             /* UDP module (receiver role) */
    JUNO_SB_BROKER_ROOT_T         *ptBroker,          /* Thread 2's broker */
    JUNO_FAILURE_HANDLER_T         pfcnFailureHandler,
    void                          *pvFailureUserData
);
```

`Init` wires the vtable, stores `ptUdp` and `ptBroker`, stores the failure handler, and
verifies all pointers.

### 5.4.3 Lifecycle Algorithms

**OnStart** (REQ-UDPAPP-012)

Opens the UDP receiver socket bound to port 9000:

```
JUNO_UDP_CFG_T tCfg = {
    .pcAddress    = "127.0.0.1",
    .uPort        = 9000,
    .uTimeoutMs   = 100,
    .bIsReceiver  = true
};
ptUdp->ptApi->Open(ptUdp, &tCfg);
```

`bIsReceiver = true` causes the platform implementation to call `bind()` on port 9000,
making the socket ready to accept incoming datagrams. `uTimeoutMs = 100` configures a
100 ms receive timeout via `SO_RCVTIMEO`, enabling the periodic scheduler to remain
responsive without blocking indefinitely.

**OnProcess** (REQ-UDPAPP-013, REQ-UDPAPP-014)

Attempts to receive one UDP datagram and, on success, publishes it to Thread 2's broker:

```
UDP_THREAD_MSG_T tMsg;
JUNO_STATUS_T tStatus = ptUdp->ptApi->Receive(ptUdp, &tMsg);
if (tStatus == JUNO_STATUS_TIMEOUT)
{
    return JUNO_STATUS_SUCCESS;   /* normal: no datagram arrived this cycle */
}
if (tStatus != JUNO_STATUS_SUCCESS)
{
    return tStatus;               /* propagate unexpected errors */
}

/* Wrap &tMsg in a fat pointer (JUNO_POINTER_T) before publishing */
JUNO_POINTER_T tPtr = JunoMemory_PointerInit(ptPointerApi, UDP_THREAD_MSG_T, &tMsg);
tStatus = ptBroker->ptApi->Publish(ptBroker, UDPTH_MSG_MID, tPtr);
if (tStatus != JUNO_STATUS_SUCCESS) return tStatus;
return JUNO_STATUS_SUCCESS;
```

The timeout case is explicitly treated as a non-error. On a loopback path the timeout
fires whenever SenderApp's cycle rate is slower than UdpBridgeApp's polling rate, or
when the system has just started. Propagating `JUNO_STATUS_TIMEOUT` as success prevents
the scheduler from treating a quiet moment as a fault.

**OnExit** (REQ-UDPAPP-015)

Closes the UDP receiver socket:

```
ptUdp->ptApi->Close(ptUdp);
```

---

// @{"design": ["REQ-UDPAPP-016", "REQ-UDPAPP-017", "REQ-UDPAPP-018"]}
## 5.5 ProcessorApp

ProcessorApp runs on Thread 2. It subscribes to Thread 2's broker and, on each cycle,
dequeues and processes all messages that UdpBridgeApp has published. It mirrors the
MonitorApp pattern on Thread 1.

### 5.5.1 Struct Definition

```c
struct PROCESSOR_APP_TAG {
    JUNO_APP_ROOT_T           tRoot;     /* MUST be first */
    JUNO_SB_BROKER_ROOT_T    *ptBroker;  /* injected: Thread 2's broker */
    JUNO_SB_PIPE_T            tPipe;     /* subscription pipe — embedded, no heap */
};
typedef struct PROCESSOR_APP_TAG PROCESSOR_APP_T;
```

`JUNO_SB_PIPE_T` is embedded directly in the struct (same pattern as MonitorApp). The
composition root owns the `PROCESSOR_APP_T` instance; the pipe is part of that allocation.

### 5.5.2 Init Function

```c
JUNO_STATUS_T ProcessorApp_Init(
    PROCESSOR_APP_T              *ptApp,
    const JUNO_APP_API_T         *ptApi,             /* vtable */
    JUNO_SB_BROKER_ROOT_T        *ptBroker,          /* Thread 2's broker */
    JUNO_FAILURE_HANDLER_T        pfcnFailureHandler,
    void                         *pvFailureUserData
);
```

`Init` wires the vtable, stores `ptBroker`, stores the failure handler, and verifies all
pointers. Pipe initialization is deferred to `OnStart`.

### 5.5.3 Lifecycle Algorithms

**OnStart** (REQ-UDPAPP-017)

Initializes the subscription pipe for `UDPTH_MSG_MID` and registers it with Thread 2's broker:

```
/* Step 1: Initialize the pipe, associating it with UDPTH_MSG_MID and its backing array */
JunoSb_PipeInit(&tPipe, UDPTH_MSG_MID, ptPipeArray, pfcnFailureHandler, pvFailureUserData);

/* Step 2: Register the initialized pipe with the broker */
ptBroker->ptApi->RegisterSubscriber(ptBroker, &tPipe);
```

After these calls the broker will enqueue a copy of every message published under
`UDPTH_MSG_MID` into `ptProcessor->tPipe`. `ptPipeArray` is a `JUNO_DS_ARRAY_ROOT_T *`
injected at `Init` time and must outlive the pipe registration.

**OnProcess** (REQ-UDPAPP-018)

Dequeues and processes all available messages from the pipe:

```
loop:
    UDP_THREAD_MSG_T tMsg;
    JUNO_POINTER_T tReturn = JunoMemory_PointerInit(ptPointerApi, UDP_THREAD_MSG_T, &tMsg);
    JUNO_STATUS_T tStatus = tPipe.tRoot.ptApi->Dequeue(&tPipe.tRoot, tReturn);
    if (tStatus == JUNO_STATUS_OOB_ERROR) break;   /* empty queue — drain complete */
    if (tStatus != JUNO_STATUS_SUCCESS) return tStatus;
    /* process tMsg — e.g., validate sequence number, measure latency */
```

`tMsg` is stack-allocated on each iteration. Dequeue is performed directly on the pipe's
embedded queue via `tPipe.tRoot.ptApi->Dequeue`. The loop drains the pipe on every
scheduler cycle. `JUNO_STATUS_OOB_ERROR` is the queue's normal empty-drain signal and is not
treated as an application error.

**OnExit**

No resources to release. The embedded `tPipe` is released when the app struct goes out
of scope (or the process exits).

---

// @{"design": ["REQ-UDPAPP-002", "REQ-UDPAPP-008", "REQ-UDPAPP-011", "REQ-UDPAPP-016"]}
## 5.6 Memory Ownership

All four application structs are allocated by the composition root (stack-allocated or
declared with static storage duration in `main.cpp`). No app struct is heap-allocated.

| Object | Owner | Lifetime |
|--------|-------|---------|
| `SENDER_APP_T` | Composition root | Entire program duration |
| `MONITOR_APP_T` | Composition root | Entire program duration |
| `UDP_BRIDGE_APP_T` | Composition root | Entire program duration |
| `PROCESSOR_APP_T` | Composition root | Entire program duration |
| `JUNO_UDP_ROOT_T` (sender) | Composition root | Exceeds SenderApp lifetime |
| `JUNO_UDP_ROOT_T` (receiver) | Composition root | Exceeds UdpBridgeApp lifetime |
| `JUNO_SB_BROKER_ROOT_T` (Broker 1) | Composition root | Exceeds Thread 1 app lifetimes |
| `JUNO_SB_BROKER_ROOT_T` (Broker 2) | Composition root | Exceeds Thread 2 app lifetimes |
| `JUNO_SB_PIPE_T` in MonitorApp | Embedded in `MONITOR_APP_T` | Same as app struct |
| `JUNO_SB_PIPE_T` in ProcessorApp | Embedded in `PROCESSOR_APP_T` | Same as app struct |
| `UDP_THREAD_MSG_T` (OnProcess) | Stack (local variable) | Single OnProcess invocation |

Key ownership rules:

1. **Injected dependencies outlive their dependents.** The composition root initializes
   the UDP module and broker instances before initializing the apps that depend on them,
   and tears them down after the apps have exited via `OnExit`.

2. **`JUNO_SB_PIPE_T` is embedded — no separate allocation.** Embedding the pipe in the
   app struct avoids any separate allocation and ensures the pipe lifetime matches the app
   lifetime exactly.

3. **`UDP_THREAD_MSG_T` is stack-allocated per `OnProcess` call.** No message buffer is
   retained between cycles. The broker copies message data internally on `Publish`; the
   stack variable is safe to discard after the call returns.

4. **No dynamic allocation anywhere.** `malloc`, `calloc`, `realloc`, and `free` are
   never used. All storage is visible at compile time in the composition root.
