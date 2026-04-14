# Skill: write-requirements

## Purpose

Author new requirements for a module or feature that does not yet have
implementation code, using existing modules and requirements as context
for style, granularity, and structure.

## When to Use

- Planning a new module before writing code
- Adding requirements for a new feature to an existing module
- Formalizing high-level system requirements

## Inputs Required

- **Module name** (new or existing)
- **Feature description** or high-level intent from the Project manager (user)
- **Parent requirements** (optional): higher-level REQ IDs this should trace to

## Instructions

> **Software Lead**: See `ai/skills/software-lead.md` → Write Requirements for planning and verification steps.

### Software Developer Role

1. After Project manager approval, create `requirements/<module>/requirements.json`.
2. Ensure the JSON is valid against the schema.
3. Submit to Software Lead for review.

## Constraints

- All rationale MUST come from the Project manager — never generate rationale.
- Use existing requirements as exemplars for granularity and language.
- Requirements must be testable/verifiable by the stated verification method.
- Follow the JSON schema in `ai/memory/traceability.md`.

## Output Format

- `requirements/<module>/requirements.json` — new file
- Summary table of new requirements

## Example Invocation

> Use skill: write-requirements
> Module: ringbuffer
> Description: I need a fixed-size circular buffer for sensor data
