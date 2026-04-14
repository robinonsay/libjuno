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

> **Software Lead**: See `ai/skills/software-lead.md` → Trace Check for planning and verification steps.

### Software Developer Role

1. Scan all `requirements/<module>/requirements.json` files.
2. Scan all source files (`src/`, `include/`) for `@{"req": [...]}` annotations.
3. Scan build system files (`CMakeLists.txt`, `cmake/`) for `@{"req": [...]}` annotations.
4. Scan all test files (`tests/`) for `@{"verify": [...]}` annotations.
5. Scan all design files (`docs/sdd/`) for `@{"design": [...]}` annotations.
6. Cross-reference and produce:
   - **Untraced requirements**: REQ IDs with no code annotation
   - **Undesigned requirements**: REQ IDs with no design annotation
   - **Untested requirements**: REQ IDs (with verification_method=Test) with no test annotation
   - **Orphaned code tags**: `@{"req": ...}` referencing nonexistent REQ IDs
   - **Orphaned test tags**: `@{"verify": ...}` referencing nonexistent REQ IDs
   - **Orphaned design tags**: `@{"design": ...}` referencing nonexistent REQ IDs
   - **Broken links**: `uses`/`implements` pointing to nonexistent IDs
   - **Coverage statistics**: % traced, % designed, % tested
7. Submit report to Software Lead.

## Lessons Learned

1. **Trace to the enforcement mechanism.** System-level requirements enforced
   by compiler flags or build configuration must be traced to the build system
   file (e.g., `CMakeLists.txt`), not to a header that merely benefits from
   the constraint. The annotation belongs where the requirement is **enforced**.

2. **The scanner must cover all traceable file types.** Annotations may
   legitimately appear in `.c`, `.h`, `.adoc`, `CMakeLists.txt`, or `.cmake`
   files. The tag pattern must support both `//` and `#` comment styles.

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
- Traced to design: N (X%)
- Traced to tests: N (X%)

### Errors
- [ERROR] Orphaned tag @{"req": ["REQ-XXX-999"]} in src/juno_xxx.c:42
- [ERROR] Broken link: REQ-YYY-001 uses REQ-ZZZ-001 (does not exist)

### Warnings
- [WARN] REQ-HEAP-003 has no code annotation
- [WARN] REQ-HEAP-003 has no design annotation
- [WARN] REQ-CRC-002 (verification_method=Test) has no test annotation

### Info
- Module HEAP: 5/5 coded, 5/5 designed, 4/5 tested
- Module CRC: 3/4 coded, 4/4 designed, 2/4 tested
```

## Example Invocation

> Use skill: trace-check
> Scope: all modules
