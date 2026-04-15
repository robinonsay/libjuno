# Skill: senior-software-engineer

## Purpose

Perform deep technical review of code and designs for algorithmic correctness,
edge case handling, security, error handling patterns, and overall code quality.
This is the final line of defense for technical correctness in LibJuno work products.

## When to Use

- When performing deep code review on implementation code
- When verifying algorithmic correctness and edge case handling
- When auditing error handling patterns for completeness
- When checking for security vulnerabilities in C code
- When reviewing design documents for technical soundness

**Always read the agent file** (`.github/agents/senior-software-engineer.agent.md`)
alongside this file for the full verification protocol.

---

## Code Correctness Checklist

### Error Handling (Severity: Error)

LibJuno uses a structured error handling system. Every deviation is a potential
silent failure or undefined behavior.

| Rule | Check | Correct Form |
|------|-------|--------------|
| Return type | All fallible functions return `JUNO_STATUS_T` or `JUNO_MODULE_RESULT` | `JUNO_STATUS_T FunctionName(...)` |
| NULL checks | Pointer params checked at entry | `JUNO_ASSERT_EXISTS(ptParam)` — returns error status if NULL |
| Status propagation | Sub-call status checked and propagated | `JUNO_ASSERT_SUCCESS(SubFunction(...))` — returns on failure |
| Result extraction | Result values extracted with status check | `JUNO_ASSERT_OK(tResult)` — returns error if status != success |
| Option extraction | Option values extracted with presence check | `JUNO_ASSERT_SOME(tOption)` — returns error if not present |
| No silent swallowing | Every error path returns a non-success status | Never ignore return value of a fallible function |
| Failure handler | Diagnostic-only callback, never alters control flow | `_pfcnFailureHandler` called for logging, then error still returned |
| Verify at entry | All public functions call Verify first | First operation in function body |

### Algorithmic Correctness (Severity: Error)

| Rule | What to Check |
|------|---------------|
| Loop bounds | No off-by-one: `i < zSize` not `i <= zSize` for 0-based indexing |
| Pointer arithmetic | Stays within allocated bounds, correct element size |
| Index calculation | Correct for the data structure (e.g., heap parent/child formulas) |
| State consistency | Struct invariants maintained after every mutation |
| Termination | All loops provably terminate |
| Overflow | Size calculations checked before use: `a + b` doesn't wrap |
| Division | Divisor checked for zero before dividing |
| Comparison | Correct operator, correct operands, correct type |
| Return value | Function returns what it promises in all paths |

### Edge Cases (Severity: Error if crash/UB, Warning if wrong result)

| Edge Case | What to Check |
|-----------|---------------|
| NULL inputs | Handled by `JUNO_ASSERT_EXISTS` at public API boundary |
| Zero size | `zSize == 0` does not cause division by zero, empty iteration, or underflow |
| Maximum values | `SIZE_MAX`, `UINT32_MAX` — no overflow in arithmetic |
| Empty collection | Operations on empty queue/stack/heap/map return correct status |
| Single element | Push+pop, insert+remove on single-element collection works correctly |
| Full capacity | Operations at capacity return correct status, no buffer overrun |
| First/last | First and last element operations in arrays/lists correct |
| Repeated operations | Multiple init, multiple push, idempotent operations behave correctly |
| Self-referential | Module passed as its own dependency (should fail gracefully if invalid) |

---

## Security Checklist

### Memory Safety (Severity: Error)

| Rule | What to Check |
|------|---------------|
| Buffer overflow | All writes verified: `iIndex < zCapacity` before `buffer[iIndex] = value` |
| Read out of bounds | All reads verified: index within valid range |
| Integer overflow | Size calculations: `a + b >= a` check, or `a <= SIZE_MAX - b` before `a + b` |
| Uninitialized reads | All struct members set before first read; no partial init |
| Cast safety | No narrowing casts that lose data, no pointer type punning UB |
| Pointer validity | Pointers checked before dereference (via Verify or ASSERT_EXISTS) |

### Input Validation (Severity: Error at boundaries, Warning internally)

| Rule | What to Check |
|------|---------------|
| Public API boundary | All parameters validated: NULL, range, size |
| Internal functions | Trust already-verified data from public entry point |
| Configuration values | Capacity, sizes, counts validated for sanity |

---

## Design Review Checklist

### Technical Soundness (Severity: Error for flawed, Warning for suboptimal)

| Rule | What to Check |
|------|---------------|
| Algorithm choice | Appropriate for the problem; correct time/space complexity |
| Error handling | Uses `JUNO_STATUS_T` / `JUNO_MODULE_RESULT` patterns |
| Memory model | All memory caller-owned; no hidden allocation |
| Failure modes | Identified explicitly; each has a defined error status |
| Complexity | Appropriate for embedded context (no unnecessary O(n²) when O(n) possible) |

### Design Quality (Severity: Warning)

| Rule | What to Check |
|------|---------------|
| No over-engineering | Design is minimal and sufficient for requirements |
| Rationale | Key decisions have rationale, sourced from PM |
| Consistency | Matches existing LibJuno module API style |
| Testability | Design is testable via DI (dependencies injectable, state observable) |
| Extensibility | Module pattern allows future derivations without breaking existing code |

---

## Verdict Criteria

### APPROVED — All of the following:

- Zero **Error** severity findings
- Zero **Warning** severity findings (or only pre-existing issues outside scope)
- Algorithm is correct for all valid inputs and documented edge cases
- Error handling is complete — no silent failures
- No security vulnerabilities

### NEEDS CHANGES — Any of the following:

- One or more **Error** severity findings (correctness, security, error handling)
- One or more **Warning** severity findings in work under review
- Algorithm has provable incorrect behavior for some input
- Error path exists that silently drops a failure status
- Buffer overflow or integer overflow possible

**Info** severity findings do not block approval but should be reported.

---

## Output Format Template

```
## Senior Software Engineer — Verification Report

### Summary
- **Verdict:** APPROVED | NEEDS CHANGES
- **Files Reviewed:** <list of files>
- **Errors:** <count>
- **Warnings:** <count>
- **Info:** <count>

### Findings

| # | Severity | File:Line | Rule | Description |
|---|----------|-----------|------|-------------|
| 1 | Error    | <file>:<line> | <category> | <what is wrong> |
| 2 | Warning  | <file>:<line> | <category> | <what is wrong> |
| 3 | Info     | <file>:<line> | <category> | <suggestion> |

### Details

<For each Error and Warning finding, describe:
 - What is wrong and why it matters
 - Concrete example: input → expected behavior → actual behavior
 - What the correct form should be
 - Which rule or pattern is violated>

### Checklist Summary

- Error Handling: PASS / FAIL (<count> issues)
- Algorithmic Correctness: PASS / FAIL (<count> issues)
- Edge Cases: PASS / FAIL (<count> issues)
- Security: PASS / FAIL (<count> issues)
- Code Quality: PASS / FAIL (<count> issues)
- Design Review: PASS / FAIL / N/A (<count> issues)
```
