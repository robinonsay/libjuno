---
description: "Run verifier agents on completed work output. Usage: /verify <files or scope description>. Spawns appropriate verifiers based on the work type and returns consolidated findings."
---

You are running **verifier agents** on completed work.

**Scope:** $ARGUMENTS

Determine the appropriate verifier mix based on the scope description, then spawn the relevant verifiers in parallel:

| Verifier | Use When |
|----------|----------|
| `software-quality-engineer` | Code, tests, or docs were produced — checks standards/naming/Doxygen |
| `software-systems-engineer` | Module code, requirements JSON, or design docs — checks architecture/DI/structure |
| `senior-software-engineer` | Implementation code — checks correctness/edge cases/security |
| `software-verification-engineer` | Any code, tests, or requirements — checks traceability completeness |

**Default behavior** (if scope doesn't indicate otherwise): spawn all four specialist verifiers in parallel.

**For each verifier you spawn**, include in the brief:
- The files to review (from `$ARGUMENTS` or inferred from scope description)
- The acceptance criteria if known, or "verify against project standards" if not specified
- Relevant context: `ai/memory/coding-standards.md`, `ai/memory/architecture.md`, `ai/memory/constraints.md`

**After all verifiers complete**, consolidate their verdicts:
- If ALL approve → report: ✓ All verifiers approved
- If ANY report NEEDS CHANGES → list all findings by severity and verifier
- Recommend next steps (which fixes are needed, which worker type should fix them)

Return the consolidated verification report to the user.
