---
description: "Invoke the software-lead orchestration loop for a sprint or task. Usage: /software-lead <task description>"
---

You are initiating the **Software Lead orchestration loop** for the following task:

**Task:** $ARGUMENTS

Spawn the `software-lead` agent with this task and the following mandatory context:

1. **Sprint Startup Protocol is MANDATORY** — before planning, the software-lead MUST:
   - Read `ai/memory/lessons-learned-software-lead.md`
   - Read `ai/memory/project-overview.md`, `ai/memory/architecture.md`, `ai/memory/coding-standards.md`, `ai/memory/constraints.md`, `ai/memory/traceability.md`, `ai/memory/directory-map.md`
   - Read relevant worker/verifier skill files from `ai/skills/`
   - Read relevant requirements and design documents for the task

2. **The software-lead is the sole orchestrator** — it plans, spawns workers and verifiers, iterates, runs the final quality gate, and presents results.

3. **Orchestration loop steps:**
   - Create a Work Breakdown Structure with acceptance criteria
   - Present plan to PM (the user) for approval before starting
   - Spawn worker agents with detailed briefs
   - Spawn verifier agents to check worker output
   - Iterate until all verifiers approve
   - Spawn `final-quality-engineer` for the final gate
   - Present structured completion report to PM

4. **No sub-agent may spawn other sub-agents** — only software-lead spawns agents.

Begin by reading the mandatory context files, then present your work breakdown plan to the Project Manager for approval before executing any work.
