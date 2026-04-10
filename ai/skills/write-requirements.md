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
- **Feature description** or high-level intent from the Program (user)
- **Parent requirements** (optional): higher-level REQ IDs this should trace to

## Instructions

### Coach Role

1. Read existing `requirements.json` files from related modules to establish
   the style, granularity, and language patterns used in this project.
2. Read `ai/memory/traceability.md` for the JSON schema and conventions.
3. Ask the Program to describe the feature or module in their own words.
4. Ask clarifying questions ONE AT A TIME:
   - What problem does this solve?
   - What are the inputs and outputs?
   - What are the failure modes?
   - Are there performance or memory constraints?
   - What existing modules does this depend on or integrate with?
5. Draft requirements in "shall" language with proposed:
   - IDs, titles, descriptions
   - Verification methods
   - `uses` links to parent requirements
6. **Present draft to Program for review.**
7. Ask the Program for **rationale** for each requirement.

### Player Role

8. After Program approval, create `requirements/<module>/requirements.json`.
9. Ensure the JSON is valid against the schema.
10. Submit to Coach for review.

### Coach Verification

11. Verify all IDs are unique and follow naming convention.
12. Verify all `uses`/`implements` links resolve to valid IDs.
13. Verify "shall" language is used consistently.
14. Verify rationale is present for every requirement (from Program, not invented).
15. **Present final output to Program for approval.**

## Constraints

- All rationale MUST come from the Program — never generate rationale.
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
