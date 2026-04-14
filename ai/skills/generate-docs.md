# Skill: generate-docs

## Purpose

Generate formal engineering documents (SRS, SDD, RTM) from the codebase's
requirements, source code, and test annotations. Validate traceability
consistency and produce AsciiDoc, HTML, and PDF outputs.

## When to Use

- Generating or regenerating the SRS, SDD, or RTM after code/requirements changes
- Validating traceability completeness before a release
- Producing documentation artifacts for review or delivery

## Inputs Required

- **Document type**: `srs`, `sdd`, `rtm`, or `all`
- **Output format** (optional): `adoc`, `html`, `pdf`, or `all` (default: `all`)
- **Modules** (optional): specific modules, or all (default: all)

## Instructions

> **Software Lead**: See `ai/skills/software-lead.md` → Generate Docs for planning and verification steps.

### Software Developer Role

1. Run the document generation tool.
2. For **SRS** (IEEE 830 structure):
   - Section 1: Introduction (purpose, scope, definitions)
   - Section 2: Overall Description (product perspective, constraints, assumptions)
   - Section 3: Specific Requirements (organized by module)
   - Each requirement includes: ID, title, description, rationale, verification method
   - Traceability links shown inline
3. For **SDD** (IEEE 1016 structure):
   - System architecture overview (module dependency relationships)
   - Per-module design: purpose, data structures, API, vtable layout
   - Interface descriptions (public API contracts)
   - Data structure descriptions (struct layouts, memory ownership)
   - Design rationale sections (content from Project manager — ask if missing)
4. For **RTM**:
   - Single matrix: Requirement → Code Location → Test Location → Verification Method → Status
   - Coverage summary (percentage of requirements with code + test traces)
   - Gap report (requirements missing traces)
5. Submit generated documents to Software Lead for review.

## Tool Specification (`scripts/generate_docs.py`)

The Python tool must:
- Scan `requirements/` for all `requirements.json` files
- Scan `src/`, `include/` for `@{"req": [...]}` annotations (`.c`, `.h` files)
- Scan `tests/` for `@{"verify": [...]}` annotations
- Scan build system files (`CMakeLists.txt`, `cmake/`) for `@{"req": [...]}` annotations
- Support both `//` (C/C++) and `#` (CMake/Python) comment-style annotations
- Parse and cross-reference all data
- Validate consistency (report warnings/errors)
- Generate AsciiDoc output files
- Invoke `asciidoctor` for HTML output
- Invoke `asciidoctor-pdf` for PDF output
- Accept CLI arguments: `--type srs|sdd|rtm|all`, `--format adoc|html|pdf|all`,
  `--module <name>`, `--output-dir <path>`

## Lessons Learned

1. **Trace to the enforcement mechanism, not the design philosophy.** When a
   requirement is enforced by compiler flags (e.g., `-nostdlib -ffreestanding`),
   trace it to the build system file that sets those flags — not to a header
   that conceptually embodies the constraint. The trace must point to the thing
   that **makes the requirement true**.

2. **Fix the tooling, don't work around it.** If the scanner can't see a file
   where a trace legitimately belongs, improve the scanner. Never move a trace
   to a less-accurate location just to satisfy the tool.

3. **Verify all call sites after modifying a shared function.** If a utility
   function like `scan_annotations()` is called from multiple places (e.g., a
   helper wrapper AND `main()`), update every call site — not just the wrapper.

## Constraints

- SDD design rationale MUST come from the Project manager — never fabricate it.
- IEEE 830 and IEEE 1016 section structures must be followed.
- Consistency validation must run before document generation.
- Default AsciiDoc theme (no custom styling required).
- Tool must live in `scripts/` directory.

## Output Format

- `docs/srs/` — SRS in .adoc, .html, .pdf
- `docs/sdd/` — SDD in .adoc, .html, .pdf
- `docs/rtm/` — RTM in .adoc, .html, .pdf
- Console: consistency validation report

## Example Invocation

> Use skill: generate-docs
> Type: all
> Format: html, pdf
