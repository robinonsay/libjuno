# UDP Module — Test Case Specifications

**Module:** UDP socket module (`JUNO_UDP_*`)
**Framework:** Unity (C11)
**Test double strategy:** Vtable-injected fake `JUNO_UDP_API_T` for unit tests; real loopback socket pair for integration tests.

---

## Scope

| REQ ID | Title | Verification |
|--------|-------|--------------|
| REQ-UDP-003 | Open Operation | Test |
| REQ-UDP-004 | Receiver Socket Bind | Test |
| REQ-UDP-005 | Sender Socket Connect | Test |
| REQ-UDP-006 | Send Operation | Test |
| REQ-UDP-007 | Receive Operation Blocking | Test |
| REQ-UDP-008 | Receive Timeout Status | Test |
| REQ-UDP-009 | Close Operation | Test |
| REQ-UDP-012 | API Error Status Return | Test |
| REQ-UDP-013 | Failure Handler Invocation on Error | Test |
| REQ-UDP-015 | Fixed Datagram Size | Test |
| REQ-UDP-016 | Module Initialization | Test |

---

## Common Setup Notes

**Fake vtable (unit tests):** A `TEST_UDP_API_T` double implementing the `JUNO_UDP_API_T` interface. Each function pointer records its call count, captures the last argument(s), and returns either `JUNO_STATUS_SUCCESS` or an injected error status when a failure flag is set. The fake never calls any POSIX socket API.

**Failure handler double:** A `TestFailureHandler` function that increments a call counter and records the last `JUNO_STATUS_T` it was passed. Stored in a file-scoped struct that is `memset` to zero in `setUp()`.

**Module under test instance:** A file-scoped `JUNO_UDP_ROOT_T s_tRoot` initialized in `setUp()` via the function under test (`Udp_Init`), with the fake vtable and failure handler double injected.

---

## Test Cases: Module Initialization (REQ-UDP-016)

### TC-UDP-001 — Init with valid root and vtable succeeds

**Requirement:** REQ-UDP-016
**Category:** Unit
**Setup:** `s_tRoot` is zeroed. A valid fake `JUNO_UDP_API_T` vtable pointer (`&s_tFakeApi`) is available. Failure handler double is registered.
**Action:** Call `Udp_Init(&s_tRoot, &s_tFakeApi, TestFailureHandler, NULL)`.
**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- `s_tRoot.ptApi` points to `&s_tFakeApi` (vtable is wired into the root).
- Failure handler double call count remains 0 (no error raised).
**Teardown:** None.

---

### TC-UDP-002 — Init with null root returns error and invokes failure handler

**Requirement:** REQ-UDP-016, REQ-UDP-012, REQ-UDP-013
**Category:** Unit
**Setup:** A valid fake vtable pointer is available. Failure handler double is ready (static context, no module root available since root is NULL).
**Action:** Call `Udp_Init(NULL, &s_tFakeApi, TestFailureHandler, NULL)`.
**Expected:**
- Return value is `JUNO_STATUS_NULLPTR_ERROR`.
- No state change (no root to corrupt).
- Failure handler is invoked exactly once with status `JUNO_STATUS_NULLPTR_ERROR`.
**Teardown:** None.

---

### TC-UDP-003 — Init with null vtable returns error and invokes failure handler

**Requirement:** REQ-UDP-016, REQ-UDP-012, REQ-UDP-013
**Category:** Unit
**Setup:** `s_tRoot` is zeroed. Failure handler double is registered.
**Action:** Call `Udp_Init(&s_tRoot, NULL, TestFailureHandler, NULL)`.
**Expected:**
- Return value is `JUNO_STATUS_NULLPTR_ERROR`.
- `s_tRoot.ptApi` is not set to a non-NULL value (root remains unmodified or zeroed).
- Failure handler is invoked exactly once with status `JUNO_STATUS_NULLPTR_ERROR`.
**Teardown:** None.

---

## Test Cases: Open Operation (REQ-UDP-003)

### TC-UDP-004 — Open receiver socket with valid config succeeds

**Requirement:** REQ-UDP-003, REQ-UDP-004
**Category:** Unit
**Setup:** `Udp_Init` called successfully with the fake vtable. Fake `Open` function pointer is configured to return `JUNO_STATUS_SUCCESS` and record a sentinel descriptor value (e.g., `iDescriptor = 5`).
**Action:** Call `ptRoot->ptApi->Open(&s_tRoot, &tCfg)` where `tCfg = { .pcAddress = "127.0.0.1", .uPort = 9000, .uTimeoutMs = 0 }`.
**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- Fake `Open` call count is 1.
- The descriptor field in the root (or implementation-specific derivation) holds the value set by the fake (not still an initial-zero/invalid sentinel), confirming the call reached the vtable and the descriptor was updated.
- Failure handler call count remains 0.
**Teardown:** None.

---

### TC-UDP-005 — Open sender socket with valid remote config succeeds

**Requirement:** REQ-UDP-003, REQ-UDP-005
**Category:** Unit
**Setup:** `Udp_Init` called successfully with the fake vtable. Fake `Open` function pointer is configured to return `JUNO_STATUS_SUCCESS` and set a sentinel descriptor value.
**Action:** Call `ptRoot->ptApi->Open(&s_tRoot, &tCfg)` where `tCfg = { .pcAddress = "127.0.0.1", .uPort = 9001, .uTimeoutMs = 0 }`.
**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- Fake `Open` call count is 1.
- The descriptor field in the root or derivation holds the sentinel value (not the invalid-sentinel), confirming the vtable dispatch reached the implementation and wrote back the handle.
- Failure handler call count remains 0.
**Teardown:** None.

---

### TC-UDP-006 — Open with null root returns error

**Requirement:** REQ-UDP-003, REQ-UDP-012, REQ-UDP-013
**Category:** Unit
**Setup:** Failure handler double is ready. No module root is available (root is NULL).
**Action:** Call the production module's Open dispatch — `ptRoot->ptApi->Open(NULL, &ptCfg)` where `ptRoot` is a validly initialized root with the fake vtable wired in. This causes the production dispatch to pass `NULL` as the root argument to the fake's `Open`, so the fake returns an error without performing any socket operation.
**Expected:** Return value is `JUNO_STATUS_NULLPTR_ERROR`. The fake vtable's `Open` call count is 1 (the dispatch reached the vtable but the fake detected NULL and returned an error). The root's internal socket descriptor is unchanged (still the invalid sentinel from init). The failure handler is invoked exactly once.
**Teardown:** None.

---

### TC-UDP-007 — Double open on already-open socket returns error

**Requirement:** REQ-UDP-003, REQ-UDP-016
**Category:** Unit
**Setup:** `Udp_Init` called successfully. Fake `Open` is configured to set a valid descriptor sentinel on first call. A second call to `Open` on the same root (with the descriptor already set to the sentinel, indicating open state) is configured to return `JUNO_STATUS_INVALID_REF_ERROR`.
**Action:** Call `ptRoot->ptApi->Open(&s_tRoot, &tCfg)` twice in sequence.
**Expected:**
- First call returns `JUNO_STATUS_SUCCESS`; descriptor set to sentinel.
- Second call returns `JUNO_STATUS_INVALID_REF_ERROR` (not `JUNO_STATUS_SUCCESS`).
- Fake `Open` call count is 2.
- Failure handler is invoked once (for the second, failing call) with status `JUNO_STATUS_INVALID_REF_ERROR`.
**Teardown:** None.

---

## Test Cases: Receiver Bind / Sender Connect (REQ-UDP-004, REQ-UDP-005)

### TC-UDP-008 — Receiver socket binds to local port (integration)

**Requirement:** REQ-UDP-004
**Category:** Integration
**Setup:** A real `LINUX_UDP_IMPL_T` (POSIX implementation) is initialized as a receiver with the real vtable. Config: `{ .pcAddress = "127.0.0.1", .uPort = 9100, .uTimeoutMs = 100 }`. A second real sender socket is separately initialized and configured to `{ .pcAddress = "127.0.0.1", .uPort = 9100, .uTimeoutMs = 0 }`.
**Action:** Call `Open` on the receiver, then immediately `Send` one `UDP_THREAD_MSG_T` from the sender to port 9100, then call `Receive` on the receiver.
**Expected:**
- Receiver `Open` returns `JUNO_STATUS_SUCCESS`.
- Sender `Send` returns `JUNO_STATUS_SUCCESS`.
- Receiver `Receive` returns `JUNO_STATUS_SUCCESS` and populates the output struct with the sent message contents (sequence number and payload match).
- This confirms the receiver socket was bound to the local port (a bound socket is reachable; an unbound one would not receive the datagram).
**Teardown:** Call `Close` on both receiver and sender sockets.

---

### TC-UDP-009 — Sender socket connects to remote address:port (integration)

**Requirement:** REQ-UDP-005
**Category:** Integration
**Setup:** A real receiver socket open on `127.0.0.1:9101` with timeout 200 ms. A real sender socket initialized with config `{ .pcAddress = "127.0.0.1", .uPort = 9101, .uTimeoutMs = 0 }`.
**Action:** Call `Open` on the sender (triggering the connect), then call `Send` with a populated `UDP_THREAD_MSG_T` (distinct sequence number = 42).
**Expected:**
- Sender `Open` returns `JUNO_STATUS_SUCCESS`.
- Sender `Send` returns `JUNO_STATUS_SUCCESS` (send succeeds because the socket is connected to the remote, requiring no per-call addressing).
- Receiver `Receive` returns `JUNO_STATUS_SUCCESS` and the output struct has `uSequenceNumber == 42`.
- This confirms the sender connected to the correct remote; a failed connect would result in a failed send or the wrong recipient.
**Teardown:** Call `Close` on both sockets.

---

## Test Cases: Send Operation (REQ-UDP-006, REQ-UDP-015)

### TC-UDP-010 — Send one message; receiver gets exactly sizeof(UDP_THREAD_MSG_T) bytes

**Requirement:** REQ-UDP-006, REQ-UDP-015
**Category:** Integration
**Setup:** Real receiver socket open on `127.0.0.1:9102` with timeout 200 ms. Real sender socket opened and connected to `127.0.0.1:9102`. A `UDP_THREAD_MSG_T` is populated with: `uSequenceNumber = 7`, `uTimestampSeconds = 100`, `uTimestampSubSeconds = 500`, `pucPayload` filled with the byte value `0xAB` repeated for all 64 bytes.
**Action:** Call `ptSenderRoot->ptApi->Send(ptSenderRoot, &tMsg)`.
**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- Receiver's `Receive` call returns `JUNO_STATUS_SUCCESS`.
- Received `UDP_THREAD_MSG_T` output struct fields match exactly: `uSequenceNumber == 7`, `uTimestampSeconds == 100`, `uTimestampSubSeconds == 500`, all 64 payload bytes equal `0xAB`.
- The datagram transferred is exactly `sizeof(UDP_THREAD_MSG_T)` bytes (verified by checking no truncation occurred — the received struct is fully populated, not partially zeroed).
**Teardown:** Close both sockets.

---

### TC-UDP-011 — Send with null message pointer returns error and invokes failure handler

**Requirement:** REQ-UDP-006, REQ-UDP-012, REQ-UDP-013
**Category:** Unit
**Setup:** `Udp_Init` called successfully with the fake vtable. Fake `Send` is configured to return `JUNO_STATUS_NULLPTR_ERROR` when `ptMsg` is NULL.
**Action:** Call `ptRoot->ptApi->Send(&s_tRoot, NULL)`.
**Expected:**
- Return value is `JUNO_STATUS_NULLPTR_ERROR`.
- Fake `Send` call count is 1.
- Failure handler is invoked exactly once with status `JUNO_STATUS_NULLPTR_ERROR`.
- No send data is transmitted (verifiable by checking the fake's captured argument is NULL and no bytes-sent side-effect counter is incremented).
**Teardown:** None.

---

## Test Cases: Receive Operation (REQ-UDP-007, REQ-UDP-008, REQ-UDP-015)

### TC-UDP-012 — Receive with datagram available returns success and populates output

**Requirement:** REQ-UDP-007, REQ-UDP-015
**Category:** Unit
**Setup:** `Udp_Init` called successfully with the fake vtable. Fake `Receive` is configured to: return `JUNO_STATUS_SUCCESS`, and write a known `UDP_THREAD_MSG_T` into the output pointer (`uSequenceNumber = 99`, `uTimestampSeconds = 200`, `uTimestampSubSeconds = 0`, `pucPayload` all set to `0xCD`). The output buffer `tMsgOut` is zero-initialized before the call.
**Action:** Call `ptRoot->ptApi->Receive(&s_tRoot, &tMsgOut)`.
**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- Fake `Receive` call count is 1.
- `tMsgOut.uSequenceNumber == 99`.
- `tMsgOut.uTimestampSeconds == 200`.
- `tMsgOut.uTimestampSubSeconds == 0`.
- All 64 bytes of `tMsgOut.pucPayload` equal `0xCD`.
- Failure handler call count remains 0.
**Teardown:** None.

---

### TC-UDP-013 — Receive times out when no sender is present

**Requirement:** REQ-UDP-007, REQ-UDP-008
**Category:** Integration
**Setup:** A real receiver socket initialized with `{ .pcAddress = "127.0.0.1", .uPort = 9103, .uTimeoutMs = 100 }` (100 ms timeout). No sender socket is created. Output buffer `tMsgOut` is zero-initialized.
**Action:** Call `ptReceiverRoot->ptApi->Receive(ptReceiverRoot, &tMsgOut)` and record the wall-clock time before and after.
**Expected:**
- Return value is `JUNO_STATUS_TIMEOUT_ERROR` (not `JUNO_STATUS_SUCCESS`, not `JUNO_STATUS_ERR`, not any other error code).
- The return occurs within ≤ 3 × uTimeoutMs milliseconds from the call to Receive (i.e., ≤ 300 ms for a 100 ms timeout), ensuring the implementation is not blocking indefinitely or ignoring the timeout setting.
- `tMsgOut` remains fully zero (no partial write occurred).
- Failure handler is NOT invoked (timeout is a normal, expected condition, not a hard error).
**Teardown:** Close receiver socket.

---

### TC-UDP-014 — Receive with null output pointer returns error and invokes failure handler

**Requirement:** REQ-UDP-007, REQ-UDP-012, REQ-UDP-013
**Category:** Unit
**Setup:** `Udp_Init` called successfully with the fake vtable. Fake `Receive` is configured to return `JUNO_STATUS_NULLPTR_ERROR` when `ptMsgOut` is NULL.
**Action:** Call `ptRoot->ptApi->Receive(&s_tRoot, NULL)`.
**Expected:**
- Return value is `JUNO_STATUS_NULLPTR_ERROR`.
- Fake `Receive` call count is 1.
- Failure handler is invoked exactly once with status `JUNO_STATUS_NULLPTR_ERROR`.
- No output buffer is modified (no output pointer was valid to write to).
**Teardown:** None.

---

## Test Cases: Close Operation (REQ-UDP-009)

### TC-UDP-015 — Close open socket succeeds and resets descriptor to invalid sentinel

**Requirement:** REQ-UDP-009
**Category:** Unit
**Setup:** `Udp_Init` called successfully with the fake vtable. Fake `Open` sets the descriptor to a valid sentinel (e.g., `5`). `Open` is called to place the root into open state. Fake `Close` is configured to return `JUNO_STATUS_SUCCESS` and reset the descriptor to the invalid sentinel (e.g., `-1`).
**Action:** Call `ptRoot->ptApi->Close(&s_tRoot)`.
**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- Fake `Close` call count is 1.
- The descriptor field in the root (or derivation) holds the invalid sentinel value (e.g., `-1`), confirming the reset occurred and future accidental reuse is detectable.
- Failure handler call count remains 0.
**Teardown:** None.

---

### TC-UDP-016 — Close already-closed socket returns error

**Requirement:** REQ-UDP-009
**Category:** Unit
**Setup:** `Udp_Init` called successfully with the fake vtable. The descriptor is at its initial invalid-sentinel state (socket was never opened, or `Close` has already run). Fake `Close` is configured to return `JUNO_STATUS_INVALID_REF_ERROR` when the descriptor is already at the invalid sentinel.
**Action:** Call `ptRoot->ptApi->Close(&s_tRoot)`.
**Expected:**
- Return value is `JUNO_STATUS_INVALID_REF_ERROR` (not `JUNO_STATUS_SUCCESS`).
- Fake `Close` call count is 1.
- Failure handler is invoked exactly once with status `JUNO_STATUS_INVALID_REF_ERROR`.
**Teardown:** None.

---

## Test Cases: Error Status Return and Failure Handler (REQ-UDP-012, REQ-UDP-013)

### TC-UDP-017 — Every API operation returns a JUNO_STATUS_T value (not void)

**Requirement:** REQ-UDP-012
**Category:** Unit
**Setup:** `Udp_Init` called successfully with the fake vtable. All four fake function pointers (`Open`, `Send`, `Receive`, `Close`) are configured to return `JUNO_STATUS_SUCCESS`.
**Action:** Call all four API functions in sequence: `Open`, `Send` (with a valid message), `Receive` (with a valid output buffer), `Close`.
**Expected:**
- Each call returns a value that can be compared to `JUNO_STATUS_SUCCESS` (i.e., the return type is `JUNO_STATUS_T` — confirmed at compile time by the function pointer signature in `JUNO_UDP_API_T`, and at run time by the fact that comparison to `JUNO_STATUS_SUCCESS` does not produce a compiler warning about void-result usage).
- All four returned values equal `JUNO_STATUS_SUCCESS`.
- This test serves as a living smoke test confirming the vtable signature contract is upheld; any change to `void` return types would break compilation.
**Teardown:** None.

---

### TC-UDP-018 — Failed open due to bad address invokes failure handler with non-null user data

**Requirement:** REQ-UDP-012, REQ-UDP-013
**Category:** Unit
**Setup:** `Udp_Init` called with the fake vtable and a non-NULL user data pointer (`pvUserData = &s_tUserDataSentinel`, a static variable). Fake `Open` is configured to return `JUNO_STATUS_ERR` (simulating a bad-address failure). Failure handler double captures both the status and the `pvUserData` pointer it receives.
**Action:** Call `ptRoot->ptApi->Open(&s_tRoot, &tCfg)` where `tCfg` contains an intentionally bad or empty address string.
**Expected:**
- Return value is `JUNO_STATUS_ERR`.
- Failure handler is invoked exactly once.
- The `pvUserData` argument received by the failure handler equals `&s_tUserDataSentinel` (not NULL, not a different pointer), confirming user data is correctly threaded from the module root to the failure handler call.
- The `tStatus` argument received by the failure handler equals `JUNO_STATUS_ERR`.
**Teardown:** None.

---

### TC-UDP-019 — Sequence number wraps at UINT32_MAX

**Requirement:** REQ-UDP-006, REQ-UDP-018  
**Category:** Unit  
**Setup:** Initialize SenderApp with a fake broker (records Publish calls) and a fake UDP vtable (records Send calls). Set the app's internal sequence counter to UINT32_MAX directly via test setup access.  
**Action:** Call `SenderApp_OnProcess(ptSenderApp)`.  
**Expected:** Return value is `JUNO_STATUS_SUCCESS`. The `UDP_THREAD_MSG_T` captured by the fake UDP Send call has `uSequenceNumber == 0` (wraps from UINT32_MAX to 0 via unsigned arithmetic). The message published to the fake broker has the same `uSequenceNumber == 0`. No assertion, crash, or trap occurs.  
**Teardown:** No OS resources acquired; no cleanup needed.

---

### TC-UDP-020 — OS socket creation failure propagates error

**Requirement:** REQ-UDP-003, REQ-UDP-012, REQ-UDP-013  
**Category:** Unit  
**Setup:** Initialize a UDP root with a fake vtable whose `Open` function is configured to return `JUNO_STATUS_ERROR` (simulating `socket()` returning -1 at the OS level). A failure handler spy is installed.  
**Action:** Call `ptRoot->ptApi->Open(ptRoot, &tCfg)`.  
**Expected:** Return value is `JUNO_STATUS_ERROR`. The root's internal socket descriptor is unchanged (invalid sentinel). The failure handler spy is invoked exactly once. No system `socket()`, `bind()`, or `connect()` calls are made (confirmed by the fake vtable recording zero delegated calls).  
**Teardown:** No OS resources acquired; no cleanup needed.
