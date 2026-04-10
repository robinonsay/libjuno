# Skill: generate-sdd

## Purpose

Generate a Software Design Document (SDD) following IEEE 1016 structure,
derived from the source code with design rationale provided by the Program (user).

## When to Use

- Producing a formal SDD for a release or review
- Documenting the design of new or existing modules
- Updating the SDD after architectural changes

## Inputs Required

- **Modules** (optional): specific modules or all (default: all)
- **Output format** (optional): `adoc`, `html`, `pdf`, or `all` (default: `all`)

## Instructions

### Coach Role

1. Read the codebase to extract design information:
   - Module headers: type definitions, vtable layouts, API contracts
   - Source files: algorithm implementations, static vtable wiring
   - Module relationships: which modules depend on which
   - `requirements.json` files: to cross-reference design ↔ requirements
2. Identify design elements per module:
   - Purpose and responsibility
   - Data structures (struct layouts with member descriptions)
   - Interface contracts (function signatures, preconditions, postconditions)
   - Vtable layout and polymorphic dispatch pattern
   - Memory ownership model
   - Error handling behavior
3. Identify **missing design rationale** — the "why" behind design decisions.
4. **Ask the Program for design rationale**, ONE topic at a time:
   - Why was this data structure chosen?
   - Why this initialization pattern?
   - Why these specific dependencies?
   - What trade-offs were considered?
5. Draft the SDD outline and present to the Program.

### Player Role

6. After Program approval, generate the SDD following IEEE 1016:

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
   - 3.N.6: Design rationale (from Program)
   - 3.N.7: Requirements traceability (cross-reference to REQ IDs)

   **Section 4: Data Design**
   - Common types (Result, Option, Pointer, Status)
   - Memory ownership model

   **Section 5: Interface Design**
   - Module initialization contracts
   - Vtable dispatch pattern
   - Trait system (JUNO_TRAIT_ROOT)

7. Generate AsciiDoc source.
8. Invoke `asciidoctor` / `asciidoctor-pdf` for HTML/PDF.
9. Submit to Coach for review.

### Coach Verification

10. Verify IEEE 1016 section structure is followed.
11. Verify all modules are covered.
12. Verify design descriptions are accurate to the code.
13. Verify design rationale is present (from Program, not fabricated).
14. Verify requirement cross-references are valid.
15. Verify HTML and PDF render correctly.
16. **Present final output to Program for approval.**

## Constraints

- Design rationale MUST come from the Program — never fabricate it.
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
