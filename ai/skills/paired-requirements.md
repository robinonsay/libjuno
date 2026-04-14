# Skill: paired-requirements

## Purpose

Author new requirements incrementally as **one half of a requirements-author
+ systems-engineer pair** operating under a "require a little, review a little"
discipline. The paired-requirements author works alongside a
`systems-engineer` agent, producing one requirement cluster at a time,
getting immediate consistency feedback, and cycling internally until each
cluster passes before moving to the next. The Software Lead reviews the pair's
completed summary report on a rolling basis.

This skill follows the same paired parallel philosophy as `paired-developer` +
`paired-tester`. The systems engineer plays the role of immediate reviewer,
keeping the feedback loop tight and preventing requirement drift from compounding
across a large batch.

## When to Use

Use this skill (instead of `write-requirements`) when the Software Lead is
running the **paired parallel model** for upstream work — i.e., authoring
requirements in fast, reviewable clusters before design or code begins.

Use `write-requirements` when requirements need to be authored in isolation
without a paired reviewer.

## Inputs Required

- **Module name** (new or existing)
- **Increment scope**: the specific capability or concern this cluster addresses
  (e.g., "initialization behavior", "error propagation", "capacity limits")
- **Feature description**: rationale and intent provided by the Software Lead
  (sourced from the Project Manager — never fabricated)
- **Parent requirements** (optional): higher-level REQ IDs this cluster traces to
- **Existing requirements**: paths to related `requirements.json` files for style/granularity reference

## How You Work With the Systems Engineer

You and the `systems-engineer` agent form a self-contained pair. The Software
Lead does not mediate individual cluster sub-cycles. You resolve feedback
within the pair and report to the Lead only when the full increment is
complete and every cluster has passed review.

### The Pair's Internal Cycle

```
You draft one requirement cluster
       │
       ▼
Systems engineer reviews the cluster
(consistency, "shall" language, traceability links, verification methods, IDs)
       │
       ▼
  Pass? ──Yes──► draft next cluster (repeat)
       │
      No
       │
  Author error? ──Yes──► you revise the cluster, re-submit for review
       │
      No (review error / ambiguity) ──► escalate to Lead
```

Repeat until every cluster in the increment's scope is drafted and reviewed.

### Escalation Rule

Resolve all requirement feedback within the pair. Only escalate to the Lead when:
- The feature description is ambiguous and rationale is needed from the Project Manager
- A requirement conflicts with an existing requirement that the pair cannot resolve

### What Is "One Cluster"?

A cluster is a small, cohesive group of 1–4 requirements that address the same
behavioral concern. Examples:
- All requirements related to module initialization
- All requirements specifying error return behavior for one operation
- All requirements for one public API function's contract

Do not draft all requirements for an entire module before any are reviewed.

### When the Increment Is Done

All clusters in scope are drafted and have passed systems-engineer review.
Co-author the **Pair Summary Report** with the systems engineer and submit
it to the Lead.

## Instructions

### Requirements Author Role

1. Read all context before drafting a single requirement:
   - Feature description and rationale from the Lead's brief
   - Existing `requirements.json` files from related modules — for granularity,
     "shall" language patterns, and ID conventions
   - `ai/memory/traceability.md` — JSON schema and annotation format
   - `ai/memory/coding-standards.md` — naming conventions

2. Select **one cluster** from the increment scope and draft its requirements:
   - Use "shall" language throughout
   - Assign `REQ-<MODULE>-<NNN>` IDs following the existing module's sequence
   - Choose a verification method (`Test`, `Inspection`, `Analysis`, `Demonstration`)
   - Add `uses` links to parent requirements where provided
   - Apply rationale provided by the Lead — never fabricate it

3. Hand off the cluster draft to the systems engineer for review.

4. On receiving feedback:
   - If the systems engineer identifies issues, revise and re-submit
   - Do not draft the next cluster until the current one passes

5. After all clusters pass, co-author the Pair Summary Report.

## Constraints

- DO NOT fabricate rationale — use only rationale provided by the Lead from the Project Manager
- DO NOT write design or implementation artifacts
- DO NOT draft requirements for concerns outside the assigned increment scope
- DO NOT skip the systems-engineer review cycle for any cluster
- ONLY report back to the Software Lead — not the Project Manager directly
- Follow the JSON schema in `ai/memory/traceability.md`

## Pair Summary Report Format

Co-author with the systems engineer and submit when the increment is done:

```
## Pair Summary — Requirements: <Increment Name>

### Requirements Authored
| REQ ID | Title | Verification Method | Parent (uses) |
|--------|-------|--------------------:|--------------|
| ...    | ...   | ...                 | ...           |

### Clusters Reviewed
- <Cluster name>: PASS
- <Cluster name>: PASS — <n> revisions

### Open Questions / Risks
- Any ambiguity encountered (flag — do not silently decide)
- Any conflict with existing requirements (flag)
```

## Output Format

Submit one **Pair Summary Report** to the Software Lead after all clusters are
complete and reviewed. Also include the updated or new `requirements/<module>/requirements.json`.
