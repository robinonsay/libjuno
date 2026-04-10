# Skill: trace-check

## Purpose

Audit the traceability system for completeness and consistency. Identify
untraced requirements, orphaned tags, missing annotations, and broken
`uses`/`implements` links.

## When to Use

- Before generating documentation (pre-flight check)
- After adding new requirements or code
- During code review to verify traceability is maintained
- As a periodic health check on the traceability system

## Inputs Required

- **Scope** (optional): specific module or all modules (default: all)

## Instructions

### Coach Role

1. Define the audit checklist:
   - [ ] Every requirement in `requirements.json` has at least one `@{"req": ...}` in source
   - [ ] Every requirement with `verification_method: "Test"` has `@{"verify": ...}` in tests
   - [ ] No `@{"req": ...}` tags reference nonexistent requirement IDs
   - [ ] No `@{"verify": ...}` tags reference nonexistent requirement IDs
   - [ ] All `uses` links resolve to valid requirement IDs
   - [ ] All `implements` links resolve to valid requirement IDs
   - [ ] No circular `uses`/`implements` dependencies
   - [ ] Requirement IDs follow `REQ-<MODULE>-<NNN>` pattern
   - [ ] No duplicate requirement IDs across modules
2. Direct the Player to execute the audit.

### Player Role

3. Scan all `requirements/<module>/requirements.json` files.
4. Scan all source files (`src/`, `include/`) for `@{"req": [...]}` annotations.
5. Scan all test files (`tests/`) for `@{"verify": [...]}` annotations.
6. Cross-reference and produce:
   - **Untraced requirements**: REQ IDs with no code annotation
   - **Untested requirements**: REQ IDs (with verification_method=Test) with no test annotation
   - **Orphaned code tags**: `@{"req": ...}` referencing nonexistent REQ IDs
   - **Orphaned test tags**: `@{"verify": ...}` referencing nonexistent REQ IDs
   - **Broken links**: `uses`/`implements` pointing to nonexistent IDs
   - **Coverage statistics**: % traced, % tested
7. Submit report to Coach.

### Coach Verification

8. Review the report for accuracy.
9. Prioritize gaps by severity:
   - **Error**: orphaned tags, broken links, duplicate IDs
   - **Warning**: untraced requirements, untested requirements
   - **Info**: coverage statistics
10. **Present the report to the Program with recommended actions.**

## Constraints

- This is a read-only audit — do not modify any files.
- Report all issues; do not silently skip modules.
- Use the JSON schema from `ai/memory/traceability.md` as the validation reference.

## Output Format

```
## Traceability Audit Report

### Summary
- Total requirements: N
- Traced to code: N (X%)
- Traced to tests: N (X%)

### Errors
- [ERROR] Orphaned tag @{"req": ["REQ-XXX-999"]} in src/juno_xxx.c:42
- [ERROR] Broken link: REQ-YYY-001 uses REQ-ZZZ-001 (does not exist)

### Warnings
- [WARN] REQ-HEAP-003 has no code annotation
- [WARN] REQ-CRC-002 (verification_method=Test) has no test annotation

### Info
- Module HEAP: 5/5 traced, 4/5 tested
- Module CRC: 3/4 traced, 2/4 tested
```

## Example Invocation

> Use skill: trace-check
> Scope: all modules
