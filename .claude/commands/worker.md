---
description: "Dispatch a single worker agent directly (bypass full orchestration loop). Usage: /worker <agent-type> <brief>. Agent types: software-developer, software-test-engineer, software-requirements-engineer, junior-software-developer"
---

You are dispatching a **direct worker agent** task (bypassing the full orchestration loop).

**Arguments:** $ARGUMENTS

Parse the arguments to extract:
1. **Agent type** — one of: `software-developer`, `software-test-engineer`, `software-requirements-engineer`, `junior-software-developer`
2. **Task brief** — the remaining text describing what the worker should do

Spawn the named worker agent with the brief. The worker should:
- Read its lessons-learned file (`ai/memory/lessons-learned-<agent-type>.md`) before starting
- Read any context files it needs to understand the task
- Execute the work and report back with deliverables

**Use this command for:**
- One-off mechanical tasks where the full orchestration loop is overhead
- Direct work item dispatch when you already know which agent is appropriate
- Quick tasks that don't need a full Work Breakdown Structure

**If you need the full orchestration loop** (WBS, PM approval, verifier agents, final quality gate), use `/software-lead` instead.

Return the worker's output directly to the user.
