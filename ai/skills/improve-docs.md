````markdown
# Skill: improve-docs

## Purpose

Iteratively evaluate and improve Software Design Documentation (SDD, SRS, RTM)
using a closed-loop two-agent system: an **Evaluator** (Systems Engineer) scores
the documentation and identifies gaps, and an **Implementer** (Documentation
Engineer) executes the required changes. The loop repeats until convergence
criteria are met.

This workflow mirrors formal iterative design review cycles used in
high-reliability environments (NPR 7150.2 style), adapted for LibJuno's
architecture: vtable DI, module root/derivation, fat pointers, trait roots,
and zero-allocation constraints.

## When to Use

- Improving existing SDD, SRS, or RTM quality after initial generation
- Preparing documentation for a formal review or release
- Recovering documentation that has drifted from the codebase
- Systematically closing traceability, clarity, or completeness gaps

## Inputs Required

- **Document type**: `sdd`, `srs`, `rtm`, or `all`
- **Scope** (optional): specific module(s) or all (default: all)
- **Max iterations** (optional): cap on loop iterations (default: 5)

## System Overview

Two cooperating agents orchestrated by the Coach:

### 1. Evaluator Agent (Systems Engineer)

- Scores documentation on a 50-point rubric
- Identifies design gaps and missing content
- Produces actionable change items (`DOC-###`)
- Tracks score deltas across iterations

### 2. Implementation Agent (Documentation Engineer)

- Consumes the Evaluator report
- Executes all `DOC-###` changes precisely
- Produces updated documentation and a change log
- Validates internal consistency before re-evaluation

### Loop Behavior

```
Evaluator → Implementation → Evaluator → Implementation → ...
```

Continues until **convergence criteria** are met or max iterations reached.

---

## Instructions

### Coach Role

1. Read the relevant memory files:
   - `ai/memory/architecture.md` — module system, vtable DI, initialization
   - `ai/memory/coding-standards.md` — naming, documentation standards
   - `ai/memory/constraints.md` — hard technical constraints
   - `ai/memory/traceability.md` — requirements schema, annotation format
2. Read the current documentation under evaluation.
3. Read the source code, headers, and requirements for in-scope modules.
4. **Ask the Program** for any missing context before starting the loop:
   - Are there known documentation gaps to prioritize?
   - Are there pending architectural changes?
   - Any design rationale not yet captured?
5. Orchestrate the Evaluator → Implementation loop (see Loop Process below).
6. After convergence or max iterations, **present final results to the Program**.

---

## Loop Process

### Step 1 — Evaluation (Evaluator Agent)

Score the current documentation using the rubric below and produce:
- Scorecard (0–50)
- Gap analysis
- Actionable changes (`DOC-###`)
- Delta analysis (from iteration 2 onward)

### Step 2 — Implementation (Implementation Agent)

Consume the Evaluator report and:
- Execute all `DOC-###` actions
- Update documentation files directly
- Produce a change log

### Step 3 — Re-evaluation (Evaluator Agent)

Run the Evaluator again on the updated documentation.

### Step 4 — Convergence Check

**Stop** if ALL of the following are true:
- Overall score ≥ 45/50
- No category score < 4/5
- No high-priority `DOC-###` actions remain

Otherwise, continue the loop.

---

## Evaluator Rubric (50 points, 10 categories × 5 points)

Each category scored 0–5:

| # | Category | What to Evaluate | LibJuno-Specific Criteria |
|---|----------|-----------------|--------------------------|
| 1 | **Architecture Representation** | System decomposition, module relationships, subsystem boundaries | Module root/derivation/vtable pattern documented; dependency graph accurate; subsystem boundaries (ds, memory, crc, etc.) clear |
| 2 | **Interface Design** | API contracts, vtable layouts, function signatures, preconditions | Vtable structs fully documented; Init/Verify pattern shown; all `@param`/`@return` present; pointer API (fat pointer) explained |
| 3 | **Data Design** | Struct layouts, memory ownership, result/option types | All struct members described with Hungarian notation; `JUNO_MODULE_RESULT`/`JUNO_MODULE_OPTION` usage shown; memory ownership explicit |
| 4 | **Behavioral Design** | State machines, algorithms, control flow, error paths | FSM transition tables present; algorithm complexity noted; `JUNO_ASSERT_*` error propagation paths documented |
| 5 | **Traceability** | Requirements ↔ Code ↔ Tests ↔ Design linkage | `@{"design": [...]}` tags present; REQ IDs cross-referenced; `uses`/`implements` hierarchy visible; RTM consistency |
| 6 | **Diagrams & Visualizations** | Mermaid/PlantUML diagrams for architecture, data flow, sequences | Module dependency diagram; vtable dispatch sequence; initialization flow; memory ownership diagram |
| 7 | **Clarity & Readability** | Non-expert comprehensibility, consistent terminology, examples | LibJuno-specific terms defined (fat pointer, trait root, module union); examples use project naming conventions |
| 8 | **Completeness** | All modules covered, no missing sections, no placeholder content | Every module in the catalog has a detailed design section; no TODO/TBD/placeholder text |
| 9 | **Technical Accuracy** | Documentation matches actual code, no stale content | Struct layouts match headers; API signatures match declarations; initialization patterns match source |
| 10 | **Standards Compliance** | IEEE 1016 (SDD) / IEEE 830 (SRS) section structure | Correct section numbering; required sections present; proper use of "shall" language in requirements |

### Scoring Guide

| Score | Meaning |
|-------|---------|
| 0 | Missing entirely |
| 1 | Present but critically incomplete or wrong |
| 2 | Partially present, significant gaps |
| 3 | Adequate, minor gaps or unclear areas |
| 4 | Good, only minor improvements needed |
| 5 | Excellent, meets all criteria |

---

## Evaluator Output Format

```markdown
# Documentation Evaluation Report — Iteration N

## 1. Scorecard

| # | Category | Score (0–5) | Notes |
|---|----------|-------------|-------|
| 1 | Architecture Representation | X | ... |
| 2 | Interface Design | X | ... |
| 3 | Data Design | X | ... |
| 4 | Behavioral Design | X | ... |
| 5 | Traceability | X | ... |
| 6 | Diagrams & Visualizations | X | ... |
| 7 | Clarity & Readability | X | ... |
| 8 | Completeness | X | ... |
| 9 | Technical Accuracy | X | ... |
| 10 | Standards Compliance | X | ... |
| **Total** | | **XX/50** | |

## 2. Score Delta Analysis (Iteration 2+)

| Category | Previous | Current | Delta | Notes |
|----------|----------|---------|-------|-------|
| Architecture Representation | X | Y | +/- | ... |
| ... | | | | |

### Delta Summary
- **Improved Areas**: ...
- **Regressions**: ...
- **Unchanged Weaknesses**: ...

## 3. Gap Analysis

### Critical Gaps (must fix)
- ...

### Moderate Gaps (should fix)
- ...

### Minor Gaps (nice to fix)
- ...

## 4. Actionable Changes

| ID | Priority | Category | Location | Required Change | Expected Outcome |
|----|----------|----------|----------|-----------------|------------------|
| DOC-001 | High | Architecture | sdd.adoc §2 | ... | ... |
| DOC-002 | High | Traceability | modules/heap.adoc | ... | ... |
| ... | | | | | |
```

---

## Implementation Agent Process

### Step 1: Parse Actions

For each `DOC-###` item, understand:
- Location (file and section)
- Required modification
- Expected outcome

### Step 2: Apply Changes

- Modify documentation files directly using edit tools
- Add missing sections
- Rewrite unclear content
- Insert Mermaid/PlantUML diagrams where specified
- Add `@{"design": [...]}` traceability tags where needed

### Step 3: Validate Internally

Ensure:
- No contradictions introduced between sections
- Diagrams match text descriptions
- Document structure remains consistent
- Traceability tags reference valid REQ IDs
- LibJuno naming conventions followed in all examples

### Implementation Output Format

```markdown
# Documentation Update Report — Iteration N

## 1. Summary
- Files modified: N
- Sections added: N
- Diagrams added: N
- Traceability tags added: N

## 2. Actions Executed

| ID | Status | Notes |
|----|--------|-------|
| DOC-001 | Complete | ... |
| DOC-002 | Complete | ... |
| DOC-003 | Deferred | Requires Program input on design rationale |

## 3. Files Modified
- `docs/sdd/sdd.adoc` — updated §2 architecture overview
- `docs/sdd/modules/heap.adoc` — added vtable layout diagram
- ...

## 4. Deviations
- DOC-###: <reason not completed>

## 5. Assumptions
- <assumptions made during implementation>
```

---

## Iteration Report Format

Each iteration produces a summary:

```markdown
## Iteration N

### Score
- Previous: XX/50
- Current: YY/50
- Delta: +/-ZZ

### Remaining Gaps
- <unresolved issues>

### Actions Executed
- DOC-001 through DOC-NNN completed

### Next Actions
- DOC-NNN+1 through DOC-MMM pending

### Convergence Status
- [ ] Score ≥ 45/50
- [ ] All categories ≥ 4/5
- [ ] No high-priority actions remain
- **Result**: Continue / Converged ✅
```

---

## Constraints

### Evaluation Rules
- Never skip evaluation — always score before and after changes
- Never assume improvements worked — verify via scoring
- Maintain strict traceability across iterations
- Score against the actual code, not aspirational state
- LibJuno-specific patterns (vtable DI, fat pointers, trait roots) must be
  understood natively by the Evaluator — do not penalize for C-native DI
  patterns that differ from OOP languages

### Implementation Rules
- Do NOT reinterpret intent — follow `DOC-###` actions exactly
- Do NOT skip items unless impossible (document as deviation)
- Do NOT introduce new design decisions without Program approval
- Do NOT fabricate design rationale — ask the Program
- Ensure output is ready for re-evaluation
- All documentation edits must preserve existing traceability tags
- Diagrams must use Mermaid or PlantUML syntax compatible with AsciiDoc

### General Rules
- Design rationale MUST come from the Program — never fabricate it
- If the Evaluator identifies a code–documentation mismatch, the **code is
  authoritative** (Agile: code is the single source of truth)
- Maximum 5 iterations by default (ask Program to continue if needed)
- Each iteration must show measurable progress (delta > 0) or request
  Program input to unblock

---

## Example Invocation

> Use skill: improve-docs
> Document: sdd
> Scope: all modules

### Example Loop Trace

**Iteration 1**
- Score: **22/50**
- Critical gaps: architecture diagram missing, FSMs undocumented, no interface
  contracts for 8 modules
- Actions: DOC-001 → DOC-015

**Iteration 2**
- Score: **37/50** (Δ +15)
- Remaining: traceability tags incomplete, data flow unclear for broker module
- Actions: DOC-016 → DOC-025

**Iteration 3**
- Score: **46/50** (Δ +9) ✅
- All categories ≥ 4, no high-priority actions
- **Converged — present to Program for approval**
````
