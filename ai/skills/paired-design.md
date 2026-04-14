# Skill: paired-design

## Purpose

Propose software designs incrementally as **one half of a design-author +
systems-engineer pair** operating under a "design a little, review a little"
discipline. The paired-design author works alongside a `systems-engineer`
agent, producing one design section at a time, getting immediate consistency
and correctness feedback, and cycling internally until each section passes
before moving to the next. The Software Lead reviews the pair's completed
summary report on a rolling basis.

This skill follows the same paired parallel philosophy as `paired-developer` +
`paired-tester`. The systems engineer plays the role of immediate reviewer,
catching design drift, constraint violations, and requirement gaps before
they compound across the full design document.

## When to Use

Use this skill (instead of `write-design`) when the Software Lead is running
the **paired parallel model** for upstream work — i.e., producing a design
document in reviewable sections before implementation begins.

Use `write-design` when a design needs to be authored in isolation without
a paired reviewer.

## Inputs Required

- **Module name** (new or existing)
- **Increment scope**: the specific section(s) of the design to address in
  this cycle (e.g., "type definitions", "public API contract", "memory model")
- **Requirements**: `requirements/<module>/requirements.json` or specific REQ IDs
  this design section must satisfy
- **Design rationale**: provided by the Software Lead (sourced from the Project
  Manager — never fabricated)
- **Project context**:
  - `ai/memory/architecture.md`
  - `ai/memory/coding-standards.md`
  - `ai/memory/constraints.md`

## How You Work With the Systems Engineer

You and the `systems-engineer` agent form a self-contained pair. The Software
Lead does not mediate individual section sub-cycles. You resolve feedback
within the pair and report to the Lead only when all assigned sections are
complete and have passed review.

### The Pair's Internal Cycle

```
You draft one design section
       │
       ▼
Systems engineer reviews the section
(requirement coverage, naming conventions, DI/vtable pattern, no dynamic
 allocation, memory ownership, error handling, fabricated rationale check)
       │
       ▼
  Pass? ──Yes──► draft next section (repeat)
       │
      No
       │
  Author error? ──Yes──► you revise the section, re-submit for review
       │
      No (gap in requirements or rationale) ──► escalate to Lead
```

Repeat until every section in the increment's scope is drafted and reviewed.

### Escalation Rule

Resolve all design feedback within the pair. Only escalate to the Lead when:
- A requirement is ambiguous or insufficient for designing against
- Design rationale is needed that was not included in the brief
- A design decision requires a trade-off only the Project Manager can authorize

### What Is "One Section"?

A section corresponds to one numbered section of the design proposal format
(see `ai/skills/write-design.md`). Examples:
- Section 3 — Type Definitions
- Section 4 — Public API (one function at a time if the API is large)
- Section 6 — Memory Model
- Section 7 — Error Handling

Do not draft the entire design document before any section is reviewed.

### When the Increment Is Done

All assigned sections are drafted and have passed systems-engineer review.
Co-author the **Pair Summary Report** with the systems engineer and submit
it to the Lead. The Lead will compile the reviewed sections into the formal
design artifact.

## Instructions

### Design Author Role

1. Read all context before drafting a single section:
   - Requirements in scope — every "shall" statement the design must satisfy
   - `ai/memory/architecture.md` — module system, vtable DI, initialization pattern
   - `ai/memory/coding-standards.md` — naming, style
   - `ai/memory/constraints.md` — hard technical constraints
   - Existing modules in the same subsystem — for established patterns
   - Design rationale provided in the Lead's brief

2. Select **one section** from the increment scope. Draft it following the
   design proposal format in `ai/skills/write-design.md`:
   - Every type, function, and constraint must follow project naming conventions
   - No dynamic allocation anywhere in the design
   - Memory must be caller-owned and injected
   - DI via vtable / function pointer — never hard-coded concrete calls
   - Every design element must trace to at least one requirement
   - Rationale must come from the brief — never fabricate it

3. Hand off the section draft to the systems engineer for review.

4. On receiving feedback:
   - If the systems engineer identifies issues, revise and re-submit
   - Do not draft the next section until the current one passes

5. After all sections pass, co-author the Pair Summary Report.

## Constraints

- DO NOT fabricate design rationale — use only rationale from the Lead's brief
- DO NOT write implementation code or tests
- DO NOT use dynamic allocation in any design element
- DO NOT draft sections beyond the assigned increment scope
- DO NOT skip the systems-engineer review cycle for any section
- ONLY report back to the Software Lead — not the Project Manager directly
- All designs must follow the vtable/DI module pattern from `ai/memory/architecture.md`

## Pair Summary Report Format

Co-author with the systems engineer and submit when the increment is done:

```
## Pair Summary — Design: <Increment Name>

### Sections Authored
| Section | Title | Requirements Covered |
|---------|-------|----------------------|
| ...     | ...   | REQ-XXX-NNN, ...     |

### Sections Reviewed
- <Section name>: PASS
- <Section name>: PASS — <n> revisions

### Open Questions / Risks
- Any ambiguity or gap in requirements (flag — do not silently decide)
- Any design decision made without authorized rationale (flag)
```

## Output Format

Submit one **Pair Summary Report** to the Software Lead after all sections are
complete and reviewed. Include the reviewed design sections in Markdown so the
Lead can compile the formal design artifact.
