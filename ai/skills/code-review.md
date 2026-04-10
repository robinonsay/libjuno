# Skill: code-review

## Purpose

Review code changes against LibJuno's coding standards, architectural patterns,
traceability requirements, and project constraints. Catch bugs, style issues,
safety violations, and traceability gaps.

## When to Use

- Reviewing a new module or feature implementation
- Reviewing changes before merging to a main branch
- Verifying a refactoring preserves correctness and traceability

## Inputs Required

- **Files to review**: specific files, a git diff, or "all changed files"
- **Context** (optional): what the change is intended to accomplish

## Instructions

### Coach Role

1. Define the review checklist based on project standards:

   **Memory Safety**
   - [ ] No `malloc`, `calloc`, `realloc`, `free`
   - [ ] No heap-allocated memory
   - [ ] All memory is caller-owned and injected
   - [ ] Pointer validity checked before use

   **Language & Portability**
   - [ ] C11 compliant, freestanding-compatible
   - [ ] No platform-specific headers in library code
   - [ ] Compiles with `-Wall -Wextra -Werror -pedantic`

   **Naming Conventions**
   - [ ] Types: `SCREAMING_SNAKE_CASE_T`
   - [ ] Functions: `PascalCase` with module prefix
   - [ ] Variables: Hungarian notation (`tStatus`, `ptRoot`, `zSize`, etc.)
   - [ ] Macros: `SCREAMING_SNAKE_CASE` with `JUNO_` prefix
   - [ ] Private members: leading underscore

   **Architecture**
   - [ ] Module root / derivation / vtable pattern followed
   - [ ] Dependencies injected via init function
   - [ ] Verify function validates all preconditions
   - [ ] No global mutable state

   **Error Handling**
   - [ ] Returns `JUNO_STATUS_T` or `JUNO_MODULE_RESULT`
   - [ ] Uses `JUNO_ASSERT_*` macros for error propagation
   - [ ] No silent error swallowing
   - [ ] Failure handler is diagnostic-only

   **Documentation**
   - [ ] Doxygen comments on all public API elements
   - [ ] `@file`, `@brief`, `@param`, `@return` present
   - [ ] MIT License header at top of file

   **Traceability**
   - [ ] `@{"req": [...]}` tags on implementing functions
   - [ ] `@{"verify": [...]}` tags on test functions
   - [ ] Referenced REQ IDs exist in `requirements.json`
   - [ ] New code has corresponding requirements

2. Direct the Player to perform the review.

### Player Role

3. Read the files under review.
4. Apply each checklist item, noting violations with:
   - File path and line number
   - Violated rule
   - Severity: **Error** (must fix), **Warning** (should fix), **Info** (suggestion)
   - Suggested fix
5. Submit findings to Coach.

### Coach Verification

6. Review findings for accuracy and completeness.
7. Remove false positives.
8. Prioritize by severity.
9. **Present review to the Program with clear, actionable items.**
10. Ask the Program if they want any items auto-fixed.

## Constraints

- Do not auto-fix without Program approval.
- Reference specific rules from `ai/memory/coding-standards.md` and
  `ai/memory/constraints.md` when citing violations.
- Be precise about line numbers and specific code.
- Do not flag intentional patterns (e.g., `stdlib.h` usage in test files is OK).

## Output Format

```
## Code Review: <files>

### Summary
- Errors: N
- Warnings: N
- Info: N

### Findings

#### [ERROR] No dynamic allocation (src/juno_foo.c:42)
`malloc(sizeof(FOO_T))` — LibJuno forbids dynamic allocation.
**Fix**: Accept a caller-provided buffer via the init function.

#### [WARN] Missing traceability tag (src/juno_foo.c:78)
`JunoFoo_Insert` has no `@{"req": [...]}` annotation.
**Fix**: Add `// @{"req": ["REQ-FOO-002"]}` above the function.

#### [INFO] Consider adding @note for complexity (include/juno/foo_api.h:55)
`JunoFoo_Find` has O(n) complexity — document this in Doxygen.
```

## Example Invocation

> Use skill: code-review
> Files: src/juno_heap.c, include/juno/ds/heap_api.h
