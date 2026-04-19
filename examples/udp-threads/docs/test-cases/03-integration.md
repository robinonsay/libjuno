# UDP Threads — Integration Test Cases

**Scope:** Integration and application-level tests for the four application
lifecycles (SenderApp, MonitorApp, UdpBridgeApp, ProcessorApp) and the
composition root (scheduler configuration and graceful shutdown).

**Verification method:** Test
**Test framework:** Unity (C)
**Injection mechanism:** Vtable-injected test doubles for broker, UDP module,
and scheduler. No mock framework — all doubles are hand-crafted structs.

**Test double naming conventions:**
- Broker double: `TEST_BROKER_DOUBLE_T` / `s_broker_double`
- UDP double: `TEST_UDP_DOUBLE_T` / `s_udp_double`
- Scheduler double: `TEST_SCHEDULER_DOUBLE_T` / `s_scheduler_double`
- Thread double: `TEST_THREAD_DOUBLE_T` / `s_thread_double`

Each double carries a `call_count` per operation and injectable failure flags.
All doubles are file-scope statics; no heap allocation.

---

## SenderApp Tests

### TC-INT-001 — SenderApp OnStart opens the UDP sender socket with the correct config

**Requirement:** REQ-UDPAPP-003
**Category:** Integration — Happy Path
**Setup:**
- Declare a `TEST_UDP_DOUBLE_T` with `open_call_count = 0`, `captured_config`
  (a copy of the `JUNO_UDP_CONFIG_T` passed to `Open`), and `fail_open = false`.
- Wire the UDP double's vtable into a `JUNO_UDP_ROOT_T`.
- Construct a `SenderApp` instance injected with the UDP double root.
- Declare a no-op broker double (SenderApp does not call broker during `OnStart`).

**Action:** Call `SenderApp_OnStart(&senderApp)`.

**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- `s_udp_double.open_call_count == 1`.
- `s_udp_double.captured_config.address` equals `"127.0.0.1"` (the compile-time
  loopback constant).
- `s_udp_double.captured_config.port` equals `9000`.
- Broker double `publish_call_count` remains `0`.

**Teardown:** None; no real socket opened.

---

### TC-INT-002 — SenderApp OnProcess publishes to broker and transmits via UDP in one cycle

**Requirement:** REQ-UDPAPP-004, REQ-UDPAPP-005
**Category:** Integration — Happy Path
**Setup:**
- Initialize SenderApp via `OnStart` against the UDP double (open succeeds).
- Reset `s_udp_double.send_call_count = 0` and `s_broker_double.publish_call_count = 0`.
- Capture the last message published (`s_broker_double.last_mid` and
  `s_broker_double.last_payload` fields in the double).
- Capture the last UDP send payload (`s_udp_double.last_send_payload`).

**Action:** Call `SenderApp_OnProcess(&senderApp)` once.

**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- `s_broker_double.publish_call_count == 1`.
- `s_broker_double.last_mid == UDPTH_MSG_MID`.
- `s_udp_double.send_call_count == 1`.
- `s_udp_double.last_send_payload` contains a `UDP_THREAD_MSG_T` with
  `sequence_number == 1` (first cycle, starting from 0 + increment).
- The payload published to the broker and the payload sent via UDP represent
  the same message (same sequence number and content).

**Teardown:** None.

---

### TC-INT-003 — SenderApp OnProcess increments sequence number monotonically across multiple cycles

**Requirement:** REQ-UDPAPP-006
**Category:** Integration — Happy Path / Boundary
**Setup:**
- Initialize SenderApp via `OnStart`.
- Record `last_sequence` from the UDP double's captured send payload after each cycle.

**Action:** Call `SenderApp_OnProcess(&senderApp)` three consecutive times.

**Expected:**
- After cycle 1: sequence number in sent payload is N (initial value + 1).
- After cycle 2: sequence number is N + 1.
- After cycle 3: sequence number is N + 2.
- Each successive sequence number is exactly one greater than the previous
  (strictly monotonically increasing with step 1).
- `s_udp_double.send_call_count == 3`.
- `s_broker_double.publish_call_count == 3`.

**Teardown:** None.

---

### TC-INT-004 — SenderApp OnExit closes the UDP sender socket exactly once

**Requirement:** REQ-UDPAPP-007
**Category:** Integration — Happy Path
**Setup:**
- Initialize SenderApp via `OnStart` (open call recorded).
- Reset `s_udp_double.close_call_count = 0`.

**Action:** Call `SenderApp_OnExit(&senderApp)`.

**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- `s_udp_double.close_call_count == 1`.
- No additional `open`, `send`, or `publish` calls are made during `OnExit`.

**Teardown:** None.

---

## MonitorApp Tests

### TC-INT-005 — MonitorApp OnStart registers a subscription for UDPTH_MSG_MID

**Requirement:** REQ-UDPAPP-009
**Category:** Integration — Happy Path
**Setup:**
- Declare a `TEST_BROKER_DOUBLE_T` with `register_call_count = 0` and
  `captured_mid` (the MID passed to `RegisterSubscriber`).
- Construct a `MonitorApp` instance injected with the broker double.

**Action:** Call `MonitorApp_OnStart(&monitorApp)`.

**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- `s_broker_double.register_call_count == 1`.
- `s_broker_double.captured_mid == UDPTH_MSG_MID`.
- No broker `Publish` or `Dequeue` calls are made during `OnStart`.

**Teardown:** None.

---

### TC-INT-006 — MonitorApp OnProcess dequeues and processes an available message

**Requirement:** REQ-UDPAPP-010
**Category:** Integration — Happy Path
**Setup:**
- Initialize MonitorApp via `OnStart` (subscription registered).
- Pre-load the broker double's dequeue stub to return one `UDP_THREAD_MSG_T`
  with `sequence_number = 42` on the first call, then return "no message"
  (e.g., `JUNO_STATUS_EMPTY` or equivalent) on the second call, so the loop
  terminates.
- Track a `dequeue_call_count` and a `processed_sequence` field in the double.

**Action:** Call `MonitorApp_OnProcess(&monitorApp)`.

**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- `s_broker_double.dequeue_call_count >= 1`.
- The message with `sequence_number == 42` was processed (observable via a
  captured log call count in the double, or by inspecting
  `s_broker_double.last_dequeued_sequence == 42`).
- `OnProcess` returns normally (does not block or loop infinitely).

**Teardown:** None.

---

### TC-INT-007 — MonitorApp OnProcess with empty pipe returns normally without blocking

**Requirement:** REQ-UDPAPP-010
**Category:** Integration — Edge Case
**Setup:**
- Initialize MonitorApp via `OnStart`.
- Configure the broker double's dequeue stub to always return "no message"
  (empty pipe status) on the first call.

**Action:** Call `MonitorApp_OnProcess(&monitorApp)`.

**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- `s_broker_double.dequeue_call_count == 1` (attempted once, saw empty, stopped).
- No assertion failure, crash, or indefinite blocking.

**Teardown:** None.

---

## UdpBridgeApp Tests

### TC-INT-008 — UdpBridgeApp OnStart opens the UDP receiver socket bound to port 9000

**Requirement:** REQ-UDPAPP-012
**Category:** Integration — Happy Path
**Setup:**
- Declare a `TEST_UDP_DOUBLE_T` with `open_call_count = 0` and `captured_config`.
- Construct a `UdpBridgeApp` instance injected with the UDP double and a no-op
  broker double.

**Action:** Call `UdpBridgeApp_OnStart(&udpBridgeApp)`.

**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- `s_udp_double.open_call_count == 1`.
- `s_udp_double.captured_config.port == 9000`.
- The config indicates a receiver/bind mode (as distinct from the sender config
  in TC-INT-001 — specific field name verified against the `JUNO_UDP_CONFIG_T`
  definition in `juno/udp.h`).
- Broker double `register_call_count` remains `0`.

**Teardown:** None.

---

### TC-INT-009 — UdpBridgeApp OnProcess receives a datagram and publishes it to broker

**Requirement:** REQ-UDPAPP-013, REQ-UDPAPP-014
**Category:** Integration — Happy Path
**Setup:**
- Initialize UdpBridgeApp via `OnStart`.
- Configure the UDP double's `Receive` stub to return `JUNO_STATUS_SUCCESS` and
  populate the output buffer with a `UDP_THREAD_MSG_T` having `sequence_number = 7`.
- Track `s_broker_double.publish_call_count` and `s_broker_double.last_mid` and
  `s_broker_double.last_payload_sequence`.

**Action:** Call `UdpBridgeApp_OnProcess(&udpBridgeApp)`.

**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- `s_udp_double.receive_call_count == 1`.
- `s_broker_double.publish_call_count == 1`.
- `s_broker_double.last_mid == UDPTH_MSG_MID`.
- `s_broker_double.last_payload_sequence == 7` (the received payload was
  forwarded intact to the broker).

**Teardown:** None.

---

### TC-INT-010 — UdpBridgeApp OnProcess with receive timeout does not publish to broker

**Requirement:** REQ-UDPAPP-013
**Category:** Integration — Edge Case
**Setup:**
- Initialize UdpBridgeApp via `OnStart`.
- Configure the UDP double's `Receive` stub to return the module-defined timeout
  status (e.g., `JUNO_STATUS_TIMEOUT` or the project-specific equivalent) without
  writing to the output buffer.

**Action:** Call `UdpBridgeApp_OnProcess(&udpBridgeApp)`.

**Expected:**
- Return value is `JUNO_STATUS_SUCCESS` (timeout is a normal operating condition,
  not a fatal error).
- `s_udp_double.receive_call_count == 1`.
- `s_broker_double.publish_call_count == 0` (no publish on timeout).

**Teardown:** None.

---

### TC-INT-011 — UdpBridgeApp OnExit closes the UDP receiver socket exactly once

**Requirement:** REQ-UDPAPP-015
**Category:** Integration — Happy Path
**Setup:**
- Initialize UdpBridgeApp via `OnStart`.
- Reset `s_udp_double.close_call_count = 0`.

**Action:** Call `UdpBridgeApp_OnExit(&udpBridgeApp)`.

**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- `s_udp_double.close_call_count == 1`.
- No additional `receive`, `publish`, or `open` calls are made during `OnExit`.

**Teardown:** None.

---

## ProcessorApp Tests

### TC-INT-012 — ProcessorApp OnStart registers a subscription for UDPTH_MSG_MID on Thread 2 broker

**Requirement:** REQ-UDPAPP-017
**Category:** Integration — Happy Path
**Setup:**
- Declare a separate `TEST_BROKER_DOUBLE_T` instance representing Thread 2's
  broker (distinct from the Thread 1 broker double used in TC-INT-005).
- Construct a `ProcessorApp` instance injected with Thread 2's broker double.

**Action:** Call `ProcessorApp_OnStart(&processorApp)`.

**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- `s_t2_broker_double.register_call_count == 1`.
- `s_t2_broker_double.captured_mid == UDPTH_MSG_MID`.
- Thread 1 broker double is unaffected (`register_call_count` remains at its
  prior value, confirming broker isolation).

**Teardown:** None.

---

### TC-INT-013 — ProcessorApp OnProcess dequeues and processes an available message

**Requirement:** REQ-UDPAPP-018
**Category:** Integration — Happy Path
**Setup:**
- Initialize ProcessorApp via `OnStart`.
- Pre-load the Thread 2 broker double's dequeue stub to return one
  `UDP_THREAD_MSG_T` with `sequence_number = 99` on the first call, then
  return "no message" on subsequent calls.
- Track `s_t2_broker_double.dequeue_call_count` and record the processed
  sequence number (via a captured field or a file-scope `s_processed_sequence`
  static variable updated by the process callback).

**Action:** Call `ProcessorApp_OnProcess(&processorApp)`.

**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- `s_t2_broker_double.dequeue_call_count >= 1`.
- The message with `sequence_number == 99` was processed (confirmed by
  `s_processed_sequence == 99` or equivalent observable state).
- `OnProcess` returns normally.

**Teardown:** None.

---

## Composition Root Tests

### TC-INT-014 — Composition root assigns correct applications to each thread's scheduler

**Requirement:** REQ-UDPAPP-021
**Category:** Integration — Happy Path
**Setup:**
- Declare thread doubles for Thread 1 and Thread 2, each with a
  `captured_scheduler_table` pointer and `captured_app_count` field populated
  when the thread double's `Create` stub is called.
- Declare scheduler doubles for both threads that capture their application
  table (array of `JUNO_APP_API_T *` pointers) and count when initialized.
- Initialize the composition root with both thread doubles and both scheduler
  doubles injected (no real pthreads created during this test).

**Action:** Call the composition root initialization function (or directly inspect
the statically configured scheduler tables if the composition root uses
compile-time constant tables).

**Expected:**
- Thread 1 scheduler table contains exactly 2 application entries.
- Thread 1 scheduler table entry 0 points to `SenderApp`'s API vtable.
- Thread 1 scheduler table entry 1 points to `MonitorApp`'s API vtable.
- Thread 2 scheduler table contains exactly 2 application entries.
- Thread 2 scheduler table entry 0 points to `UdpBridgeApp`'s API vtable.
- Thread 2 scheduler table entry 1 points to `ProcessorApp`'s API vtable.
- No cross-assignment: SenderApp and MonitorApp are NOT in Thread 2's table;
  UdpBridgeApp and ProcessorApp are NOT in Thread 1's table.

**Teardown:** None (no real threads created).

---

### TC-INT-015 — Composition root gracefully shuts down both threads without deadlock

**Requirement:** REQ-UDPAPP-023
**Category:** Integration — System / Happy Path
**Setup:**
- Use the real Linux thread and UDP implementations (no doubles for this test —
  real loopback UDP and real pthreads).
- Configure both threads with a short scheduler period (e.g., 10 ms) so the
  test completes quickly.
- Start the composition root (both threads running, schedulers ticking).
- Allow a brief settling period (e.g., 3 scheduler cycles on each thread, ~30 ms).

**Action:** Invoke the composition root shutdown sequence:
1. Call `Thread_Stop` on Thread 1's root.
2. Call `Thread_Stop` on Thread 2's root.
3. Call `Thread_Join` on Thread 1's root.
4. Call `Thread_Join` on Thread 2's root.

**Expected:**
- Both `Stop` calls return `JUNO_STATUS_SUCCESS`.
- Both `Join` calls return `JUNO_STATUS_SUCCESS`.
- Join timeout enforcement: each `JunoThread_Join` call is wrapped by the test harness in a helper that internally calls `pthread_timedjoin_np` with a 2-second absolute deadline. If either Join does not return within 2 seconds, the test fails immediately with a "Join timeout — possible deadlock" message. Before calling Stop, the harness confirms that each scheduler has completed at least one full Execute cycle by checking a call counter injected into the scheduler's time source double (minimum 1 confirmed cycle per scheduler). This prevents stopping threads before applications have run OnStart.
- After both joins complete, the system is in a clean state: no threads running,
  no sockets open (UdpBridgeApp and SenderApp `OnExit` callbacks have been
  called by their respective schedulers, or confirmed via their close call counts
  if UDP doubles are layered back in for post-shutdown inspection).
- No assertion failures, crashes, or deadlocks during the shutdown sequence.

**Teardown:**
- Verify the loopback UDP port (9000) is no longer bound after the test
  (attempt `bind` on port 9000 from within the test process; it should succeed,
  confirming the socket was released).

---

## Test Double Reference

| Double | Purpose | Key Fields |
|--------|---------|-----------|
| `TEST_UDP_DOUBLE_T` | Replaces UDP module vtable | `open_call_count`, `send_call_count`, `receive_call_count`, `close_call_count`, `captured_config`, `last_send_payload`, `receive_return_status`, `receive_output_msg`, `fail_open`, `fail_send`, `fail_receive` |
| `TEST_BROKER_DOUBLE_T` | Replaces broker vtable | `register_call_count`, `publish_call_count`, `dequeue_call_count`, `captured_mid`, `last_mid`, `last_payload_sequence`, `dequeue_messages[]`, `dequeue_message_count` |
| `TEST_THREAD_DOUBLE_T` | Replaces thread vtable for composition root | `create_call_count`, `stop_call_count`, `join_call_count`, `captured_scheduler_table`, `captured_app_count` |
| `TEST_SCHEDULER_DOUBLE_T` | Captures scheduler init params | `init_call_count`, `app_table`, `app_count`, `period_ms` |

All doubles are file-scope statics. No heap allocation. Doubles are reset with
`memset` in `setUp`. Broker dequeue sequences use a static message array with
an index counter (not a dynamic queue).

---

## Requirement Coverage Summary

| REQ ID | Covered By |
|--------|-----------|
| REQ-UDPAPP-003 | TC-INT-001 |
| REQ-UDPAPP-004 | TC-INT-002 |
| REQ-UDPAPP-005 | TC-INT-002 |
| REQ-UDPAPP-006 | TC-INT-003 |
| REQ-UDPAPP-007 | TC-INT-004 |
| REQ-UDPAPP-009 | TC-INT-005 |
| REQ-UDPAPP-010 | TC-INT-006, TC-INT-007 |
| REQ-UDPAPP-012 | TC-INT-008 |
| REQ-UDPAPP-013 | TC-INT-009, TC-INT-010 |
| REQ-UDPAPP-014 | TC-INT-009 |
| REQ-UDPAPP-015 | TC-INT-011 |
| REQ-UDPAPP-017 | TC-INT-012 |
| REQ-UDPAPP-018 | TC-INT-013 |
| REQ-UDPAPP-021 | TC-INT-014 |
| REQ-UDPAPP-023 | TC-INT-015 |
