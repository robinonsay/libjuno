# Skill: derive-requirements

## Purpose

Analyze existing source code and test files to derive requirements, generate or
update `requirements.json` files, and add traceability annotations (`@{"req": ...}`
and `@{"verify": ...}`) to source and test code.

## When to Use

- Bootstrapping requirements for a module that has code but no `requirements.json`
- Updating requirements after code changes
- Adding missing traceability tags to existing source/test functions

## Inputs Required

- **Module name** (e.g., `heap`, `memory`, `crc`)
- **Scope** (optional): specific files or the entire module

## Instructions

> **Software Lead**: See `ai/skills/software-lead.md` → Derive Requirements for planning and verification steps.

### Software Developer Role

1. After Project manager approval, generate/update `requirements/<module>/requirements.json`.
2. Add `// @{"req": ["REQ-MODULE-NNN"]}` tags above implementing functions in source.
3. Add `// @{"verify": ["REQ-MODULE-NNN"]}` tags above test functions.
4. Submit all changes to Software Lead for review.

## Constraints

- Never invent rationale — always ask the Project manager.
- Never assume a requirement exists unless it is directly evidenced by code behavior
  or test assertions.
- Follow the JSON schema defined in `ai/memory/traceability.md`.
- Follow all naming conventions in `ai/memory/coding-standards.md`.

## Output Format

- `requirements/<module>/requirements.json` — new or updated
- Modified source files with `@{"req": ...}` tags
- Modified test files with `@{"verify": ...}` tags
- Summary table of derived requirements with traceability status

## Example Invocation

> Use skill: derive-requirements
> Module: heap
