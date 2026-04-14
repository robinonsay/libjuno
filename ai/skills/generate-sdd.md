# Skill: generate-sdd

## Purpose

Generate a Software Design Document (SDD) following IEEE 1016 structure,
derived from the source code with design rationale provided by the Project manager (user).

## When to Use

- Producing a formal SDD for a release or review
- Documenting the design of new or existing modules
- Updating the SDD after architectural changes

## Inputs Required

- **Modules** (optional): specific modules or all (default: all)
- **Output format** (optional): `adoc`, `html`, `pdf`, or `all` (default: `all`)

## Instructions

> **Software Lead**: See `ai/skills/software-lead.md` → Generate SDD for planning and verification steps.

### Software Developer Role

1. After Project manager approval, generate the SDD following IEEE 1016:

   **Section 1: Introduction**
   - Purpose, scope, definitions, references

   **Section 2: System Architecture**
   - Module dependency overview
   - Subsystem decomposition (ds, memory, crc, etc.)
   - Key architectural patterns (vtable DI, module root/derivation)

   **Section 3: Detailed Design (per module)**
   - 3.N.1: Purpose and responsibility
   - 3.N.2: Data structures (struct layouts, member descriptions)
   - 3.N.3: Interface design (API functions, vtable layout)
   - 3.N.4: Algorithm descriptions
   - 3.N.5: Error handling
   - 3.N.6: Design rationale (from Project manager)
   - 3.N.7: Requirements traceability (cross-reference to REQ IDs)

   **Section 4: Data Design**
   - Common types (Result, Option, Pointer, Status)
   - Memory ownership model

   **Section 5: Interface Design**
   - Module initialization contracts
   - Vtable dispatch pattern
   - Trait system (JUNO_TRAIT_ROOT)

2. Generate AsciiDoc source.
3. Invoke `asciidoctor` / `asciidoctor-pdf` for HTML/PDF.
4. Submit to Software Lead for review.

## Constraints

- Design rationale MUST come from the Project manager — never fabricate it.
- All design descriptions must be derived from actual code, not assumed.
- IEEE 1016 section structure must be followed.
- Cross-references to requirement IDs must be valid.
- If a module lacks requirements, flag it as a gap rather than inventing requirements.

## Output Format

- `docs/sdd/sdd.adoc` — AsciiDoc source
- `docs/sdd/sdd.html` — HTML output
- `docs/sdd/sdd.pdf` — PDF output
- Console: list of modules documented, any gaps found

## Example Invocation

> Use skill: generate-sdd
> Modules: all
> Format: all
