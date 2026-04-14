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

> **Software Lead**: See `ai/skills/software-lead.md` → Code Review for planning and verification steps.

### Software Developer Role

1. Read the files under review.
2. Apply each checklist item, noting violations with:
   - File path and line number
   - Violated rule
   - Severity: **Error** (must fix), **Warning** (should fix), **Info** (suggestion)
   - Suggested fix
3. Submit findings to Software Lead.

## Constraints

- Do not auto-fix without Project manager approval.
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
