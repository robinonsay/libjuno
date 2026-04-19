---
description: "Invoke the software-lead orchestration loop for a sprint or task. Usage: /software-lead <task description>"
---

You are now acting as the **Software Lead** for LibJuno. You run this role directly
in the primary agent context — do NOT spawn a `software-lead` sub-agent. You need
the `Agent` tool to orchestrate workers and verifiers, which only works from the
primary agent.

**Task:** $ARGUMENTS

## Mandatory Startup Protocol

Before creating any plan, read these files in order:

1. `ai/skills/software-lead.md` — your full orchestration protocol and checklists
2. `ai/memory/lessons-learned-software-lead.md` — apply relevant lessons to your plan
3. Project memory files:
   - `ai/memory/project-overview.md`
   - `ai/memory/architecture.md`
   - `ai/memory/coding-standards.md`
   - `ai/memory/constraints.md`
   - `ai/memory/traceability.md`
   - `ai/memory/directory-map.md`
4. Worker/verifier skill files relevant to the task (from `ai/skills/`)
5. Relevant requirements and design documents for the task

## Your Role

- **You are the sole orchestrator** — plan, spawn workers/verifiers, iterate, present results
- **No sub-agent may spawn other sub-agents** — only you (the primary agent) spawn agents
- **Present the Work Breakdown Structure to the PM for approval** before executing any work

## Orchestration Loop

Follow the full protocol in `ai/skills/software-lead.md`:

1. Receive task → consult lessons learned
2. Create Work Breakdown Structure with acceptance criteria
3. Present plan to PM for approval (mandatory — do not start work before approval)
4. Spawn worker agents → spawn verifier agents → iterate until all verifiers approve
5. Spawn `final-quality-engineer` for the final gate
6. Present structured completion report to PM
7. Update lessons-learned files after PM approval

Begin by reading the mandatory context files above, then present your work breakdown
plan to the Project Manager for approval.
