# Skill: write-design

## Purpose

Propose software designs that satisfy documented requirements, before
implementation code is written. The design covers data structures, vtable
layouts, API interfaces, algorithms, memory ownership, error handling
strategy, and module relationships — producing a design proposal that the
Project Manager (user) can review, refine, and approve.

## When to Use

- Designing a new module or feature before writing code
- Proposing an architecture for a set of requirements
- Evaluating alternative design approaches for a feature
- Updating a design after requirements change

## Inputs Required

- **Module name** (new or existing)
- **Requirements**: `requirements/<module>/requirements.json` or specific REQ IDs
- **Scope** (optional): full module design, specific feature, or design update
- **Constraints** (optional): additional constraints beyond project defaults

## Instructions

> **Software Lead**: See `ai/skills/software-lead.md` → Write Design for planning and verification steps.

### Software Developer Role

1. After Project Manager approval of the design proposal, produce the
   formal design artifact:
   - Create `docs/designs/<module>_design.md` with the approved design
   - Include requirements traceability matrix (which design element
     satisfies which requirement)
   - Include any approved diagrams or data flow descriptions
2. Submit to Software Lead for verification.

## Design Proposal Format

The design proposal presented to the Project Manager should contain:

### 1. Overview

- Module name, subsystem, and one-paragraph summary
- Requirements in scope (list REQ IDs and titles)

### 2. Design Approach

- High-level approach and rationale
- Alternatives considered and why they were rejected (with Project Manager input)

### 3. Type Definitions

- Module root struct layout (members, sizes, purposes)
- Module derivation struct (embedding root, additional state)
- API struct (vtable): function pointer signatures with parameter descriptions
- Result / Option types if applicable
- Any supporting types (enums, config structs)

### 4. Public API

For each public function:
- Signature (return type, parameters with Hungarian notation)
- Brief description
- Preconditions and postconditions
- Which requirement(s) it satisfies

### 5. Vtable Layout

- Function pointer table with each slot described
- Default implementation behavior
- How test doubles will override via vtable injection

### 6. Memory Model

- What memory the caller must provide
- Struct sizes and alignment considerations
- Buffer ownership and lifetime rules

### 7. Error Handling

- Failure modes and corresponding status codes
- Assertion points (entry validation)
- Failure handler integration (module root pattern)

### 8. Module Dependencies

- Which existing modules are required (API types referenced)
- Integration points (how vtables are composed / injected)

### 9. Algorithm Descriptions

- Key algorithms with step-by-step logic
- Time and space complexity (if relevant)

### 10. Requirements Traceability

| Requirement ID | Design Element | How Satisfied |
|----------------|---------------|---------------|
| REQ-XXX-001    | ...           | ...           |

## Constraints

- Design rationale MUST come from the Project Manager — never fabricate it.
- All designs must follow the vtable/DI module pattern.
- No dynamic allocation in any design element.
- All proposed types and functions must follow naming conventions.
- Memory must be caller-owned and injected.
- Designs must be testable via vtable-injected test doubles.
- Every requirement in scope must be traceable to at least one design element.
- If requirements are ambiguous or insufficient, flag them as gaps rather
  than making assumptions.

## Output Format

- Design proposal (Markdown, presented inline for review)
- `docs/designs/<module>_design.md` — formal design artifact (after approval)
- Requirements traceability matrix (which design element → which requirement)
- List of any requirement gaps or ambiguities found

## Example Invocation

> Use skill: write-design
> Module: ringbuffer
> Requirements: requirements/ringbuffer/requirements.json
> Scope: full module design
