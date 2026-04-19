# UDP Threads — Thread Module Test Cases

**Scope:** Unit tests for the thread module (`THREAD`) covering initialization,
Create, Stop, Join, error status, and failure handler invocation.

**Verification method:** Test
**Test framework:** Unity (C)
**Injection mechanism:** Vtable-injected test doubles; no mock framework.

**Thread entry test double pattern:** Tests that require an executing thread body
use a hand-crafted entry function that writes to a caller-owned flag or counter
variable passed via the `pvArg` argument pointer, then returns. The variable is
declared as a file-scope static (no heap allocation).

---

## Module Initialization

### TC-THREAD-001 — Init with valid root and vtable succeeds

**Requirement:** REQ-THREAD-012
**Category:** Unit — Happy Path
**Setup:**
- Declare a stack-allocated `JUNO_THREAD_ROOT_T` zeroed with `memset`.
- Declare a pointer to the production Linux vtable (or a hand-crafted test vtable
  where all three function pointers point to stub functions that return
  `JUNO_STATUS_SUCCESS`).
- Provide a no-op failure handler captured in a file-scope double struct.

**Action:** Call `JunoThread_Init(&root, pVtable, pFailureHandler)`.

**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- `root.pApi` equals `pVtable` (vtable pointer wired into root).
- `root.bStop` is `false`.
- The failure handler is NOT invoked (call count on the double remains 0).

**Teardown:** None required; no OS resources allocated.

---

### TC-THREAD-002 — Init with null root invokes failure handler and returns error

**Requirement:** REQ-THREAD-012, REQ-THREAD-013
**Category:** Unit — Error Path
**Setup:**
- Provide a valid vtable pointer.
- Provide a failure handler double with a `call_count` field and a captured
  `description` string pointer.

**Action:** Call `JunoThread_Init(NULL, pVtable, pFailureHandler)`.

**Expected:**
- Return value is exactly `JUNO_STATUS_NULLPTR_ERROR` (the LibJuno standard null-pointer guard code returned by `JUNO_ASSERT_EXISTS`). The failure handler spy is invoked exactly once.
- The description string passed to the failure handler is non-NULL and non-empty.

**Teardown:** None.

---

### TC-THREAD-003 — Init with null vtable invokes failure handler and returns error

**Requirement:** REQ-THREAD-012, REQ-THREAD-013
**Category:** Unit — Error Path
**Setup:**
- Declare a valid zeroed `JUNO_THREAD_ROOT_T`.
- Provide a failure handler double with a `call_count` field.
- Pass `NULL` as the vtable pointer.

**Action:** Call `JunoThread_Init(&root, NULL, pFailureHandler)`.

**Expected:**
- Return value is exactly `JUNO_STATUS_NULLPTR_ERROR`. The root's state is unchanged (stop flag still false, handle still 0). The failure handler spy is invoked exactly once.
- `root.pApi` remains `NULL` (vtable was NOT partially written).

**Teardown:** None.

---

## Create Operation

### TC-THREAD-004 — Create with valid entry function starts the thread

**Requirement:** REQ-THREAD-003
**Category:** Unit — Happy Path
**Setup:**
- Declare a file-scope `static volatile bool s_entry_ran = false`.
- Define a thread entry double:
  ```
  void *TestEntry(void *pvArg) {
      s_entry_ran = true;
      return NULL;
  }
  ```
- Initialize a `JUNO_THREAD_ROOT_T` via `JunoThread_Init` with the Linux vtable
  and a no-op failure handler.

**Action:** Call `root.pApi->Create(&root, TestEntry, NULL)`.

**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- After a short deterministic wait (e.g., call `Join` immediately after), `s_entry_ran`
  is `true`, confirming the entry function executed.
- `root.uHandle` is non-zero (OS thread handle populated).

**Teardown:** Join the thread to prevent resource leak.

---

### TC-THREAD-005 — Create with null entry function returns error

**Requirement:** REQ-THREAD-003, REQ-THREAD-010
**Category:** Unit — Error Path
**Setup:**
- Initialize a `JUNO_THREAD_ROOT_T` with a failure handler double.

**Action:** Call `root.pApi->Create(&root, NULL, NULL)`.

**Expected:**
- Return value is the module-defined null-pointer error status code (not merely
  `!= JUNO_STATUS_SUCCESS` — the exact code must be verified).
- Failure handler is invoked exactly once (`call_count == 1`).
- `root.uHandle` remains 0 (no OS thread was created).

**Teardown:** None.

---

### TC-THREAD-006 — Create on already-running root returns error (single thread per root)

**Requirement:** REQ-THREAD-004, REQ-THREAD-015
**Category:** Unit — Error Path
**Setup:**
- Declare a long-running entry double that spins while `root.bStop == false`:
  ```
  void *SpinEntry(void *pvArg) {
      JUNO_THREAD_ROOT_T *pRoot = pvArg;
      while (!pRoot->bStop) { /* spin */ }
      return NULL;
  }
  ```
- Initialize the root and call `Create` once successfully (first thread is running).
- Provide a failure handler double.

**Action:** Call `root.pApi->Create(&root, SpinEntry, &root)` a second time while
the first thread is still running.

**Expected:**
- Second `Create` return value is the module-defined "already running" error status
  code (exact code verified, not merely `!= JUNO_STATUS_SUCCESS`).
- Failure handler is invoked exactly once for the second call.
- The first thread continues running (handle unchanged, `s_entry_ran` state intact).

**Teardown:** Set `root.bStop = true` (or call `Stop`), then `Join`.

---

## Stop Operation

### TC-THREAD-007 — Stop sets the stop flag; entry function observes it and exits

**Requirement:** REQ-THREAD-006, REQ-THREAD-007
**Category:** Unit — Happy Path
**Setup:**
- Declare a file-scope `static volatile int s_loops = 0`.
- Define an entry double that reads `root.bStop` via the `pvArg` pointer:
  ```
  void *LoopEntry(void *pvArg) {
      JUNO_THREAD_ROOT_T *pRoot = pvArg;
      while (!pRoot->bStop) {
          s_loops++;
          /* small yield if available */
      }
      return NULL;
  }
  ```
- Initialize the root; call `Create(&root, LoopEntry, &root)`.
- Allow at least one loop iteration.

**Action:** Call `root.pApi->Stop(&root)`.

**Expected:**
- Return value of `Stop` is `JUNO_STATUS_SUCCESS`.
- `root.bStop` is `true` immediately after `Stop` returns.
- A subsequent `Join` completes without blocking indefinitely (thread observed
  the flag and exited its loop).
- `s_loops >= 1` confirms the entry ran at least once before stopping.

**Teardown:** Join the thread.

---

### TC-THREAD-008 — Stop on un-created root returns error

**Requirement:** REQ-THREAD-006, REQ-THREAD-010
**Category:** Unit — Error Path
**Setup:**
- Initialize a `JUNO_THREAD_ROOT_T` (handle is zero — thread was never created).
- Provide a failure handler double.

**Action:** Call `root.pApi->Stop(&root)`.

**Expected:**
- Return value is the module-defined "not running" or "invalid handle" error status
  code (exact code verified).
- Failure handler is invoked exactly once (`call_count == 1`).
- `root.bStop` is unchanged (remains `false`).

**Teardown:** None.

---

## Join Operation

### TC-THREAD-009 — After Stop, Join blocks until thread exits and returns success

**Requirement:** REQ-THREAD-005
**Category:** Unit — Happy Path
**Setup:**
- Use the same spinning `LoopEntry` double from TC-THREAD-007.
- Initialize root, call `Create`, call `Stop`.

**Action:** Call `root.pApi->Join(&root)`.

**Expected:**
- Return value is `JUNO_STATUS_SUCCESS`.
- Call returns (does not block indefinitely) — verified implicitly by the test
  completing within a fixed timeout enforced by the test runner.
- `root.uHandle` is reset to 0 (or the module-defined sentinel) after a successful
  join, indicating the root is ready for reuse.

**Teardown:** None; join already completed.

---

### TC-THREAD-010 — Join on un-created root returns error

**Requirement:** REQ-THREAD-005, REQ-THREAD-010
**Category:** Unit — Error Path
**Setup:**
- Initialize a `JUNO_THREAD_ROOT_T` with handle = 0 (never created).
- Provide a failure handler double.

**Action:** Call `root.pApi->Join(&root)`.

**Expected:**
- Return value is the module-defined "not running" or "invalid handle" error status
  code (exact code verified, not merely `!= JUNO_STATUS_SUCCESS`).
- Failure handler is invoked exactly once (`call_count == 1`).

**Teardown:** None.

---

## Error Status and Failure Handler

### TC-THREAD-011 — Failed pthread_create causes failure handler invocation

**Requirement:** REQ-THREAD-010, REQ-THREAD-013
**Category:** Unit — Injected Failure
**Setup:** Initialize a thread root with a failing `Create` vtable stub: the stub records that it was called and returns `JUNO_STATUS_ERROR` immediately, but does NOT invoke the failure handler. A failure handler spy (records call count and description) is wired into the root. The thread handle field in the root is zero before the call.

**Action:** Call `JunoThread_Create(ptRoot, ValidEntryFn, NULL)` (the module's production dispatch entry point with the failing vtable injected).

**Expected:** Return value is `JUNO_STATUS_ERROR`. The failure handler spy is invoked exactly once by the *module's* dispatch logic (not the stub) — the module detects the vtable's error return and calls the handler. The thread handle field in the root remains zero (no OS thread was created). No real pthreads resources are acquired.

**Teardown:** No OS resources acquired; no cleanup needed.

---

### TC-THREAD-012 — All error-returning operations return JUNO_STATUS_T (coverage roll-up)

**Requirement:** REQ-THREAD-010
**Category:** Unit — Analysis / Roll-up

**Setup:** Execute TC-THREAD-002, TC-THREAD-003, TC-THREAD-005, TC-THREAD-006,
TC-THREAD-008, TC-THREAD-010, and TC-THREAD-011. Each uses the setup described in
its own entry.

**Action:** Run the above TCs as a suite.

**Expected:** Each TC passes its own assertions, and in every case the module API
returns a value of type `JUNO_STATUS_T` (not `void` or a bare integer outside the
`JUNO_STATUS_T` domain). No additional test function is required; coverage of
REQ-THREAD-010 is provided by the union of the above TCs.

> **Traceability note:** When adding `@verify` annotations, tag any one of the
> above TCs with `REQ-THREAD-010`. Do not create an empty test function for this
> roll-up entry.

---

## Test Double Reference

| Double | Purpose | Fields |
|--------|---------|--------|
| `TEST_FAILURE_HANDLER_DOUBLE_T` | Captures failure handler invocations | `call_count`, `last_description` (const char *) |
| `TestEntry` (entry function) | Marks execution via `s_entry_ran` flag | File-scope `volatile bool` |
| `SpinEntry` (entry function) | Loops until `root->bStop` is true | Reads `JUNO_THREAD_ROOT_T.bStop` via arg |
| `LoopEntry` (entry function) | Counts loops and exits when stop flag set | File-scope `volatile int s_loops` |
| Failure-injecting vtable | Forces `Create` to return error | `fail_create` flag, `injected_status` field |

All test doubles are hand-crafted; no mock framework is used. All state variables
are file-scope statics or stack-allocated — no heap allocation.
