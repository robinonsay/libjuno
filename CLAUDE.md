# CLAUDE.md — LibJuno

## Project Summary

LibJuno is a lightweight C11 embedded systems micro-framework that provides common
capabilities and interfaces for embedded systems development. It is designed to be
freestanding-compatible, use zero dynamic memory allocation, and support dependency
injection via vtables as its core architectural paradigm. All memory is caller-owned
and injected. Version 1.0.1, MIT license.

## Repository Roots

| Sub-Project | Root Directory | Description |
|-------------|---------------|-------------|
| LibJuno C library | `/workspaces/libjuno` | C11 embedded micro-framework |
| VSCode Extension | `/workspaces/libjuno/vscode-extension` | TypeScript VS Code extension |
| Python scripts | `/workspaces/libjuno` | Utility and verification scripts |

### Command Reference

| Command | Working Directory | Purpose |
|---------|-------------------|---------|
| `cd build && cmake --build .` | `/workspaces/libjuno` | Build LibJuno C library |
| `cd build && ctest --output-on-failure` | `/workspaces/libjuno` | Run C unit tests (Unity) |
| `python3 scripts/verify_traceability.py` | `/workspaces/libjuno` | Verify traceability annotations |
| `npm test` | `/workspaces/libjuno/vscode-extension` | Run VSCode extension Jest tests |
| `npm run compile` | `/workspaces/libjuno/vscode-extension` | Compile TypeScript extension |

**Safe command patterns:**
```bash
# C build + test
cd /workspaces/libjuno && cd build && cmake --build . && ctest --output-on-failure

# VSCode Extension test
cd /workspaces/libjuno/vscode-extension && npm test

# Traceability verification
cd /workspaces/libjuno && python3 scripts/verify_traceability.py
```

**Common mistakes:** Running `npm test` from `/workspaces/libjuno` (wrong — must be in `vscode-extension/`). Running cmake from `vscode-extension/` (wrong — must be in project root).

## Key Technical Rules

1. **No dynamic allocation** — never use `malloc`, `calloc`, `realloc`, `free`.
2. **Freestanding C11** — all library code must compile with `-nostdlib -ffreestanding`.
3. **Module pattern** — use vtable/DI: module root → derivation → API struct → union.
4. **Error handling** — return `JUNO_STATUS_T` or `JUNO_MODULE_RESULT`. Use `JUNO_ASSERT_*` macros.
5. **Naming** — Types: `SCREAMING_SNAKE_T`, Functions: `PascalCase`, Variables: Hungarian notation.
6. **Documentation** — Doxygen on all public API elements.
7. **Testing** — Unity framework, vtable-injected test doubles, no mock framework.
8. **Traceability** — all requirements, code, and tests must be linked (see below).

## AI Workflow: Flat Orchestration Model

The AI operates as a **flat orchestration system** with one orchestrator,
worker agents, and verifier agents. **Only the Software Lead spawns agents.**
No sub-agent may spawn other sub-agents.

### Roles

- **Software Lead** (orchestrator): Plans tasks, creates work breakdowns,
  spawns worker and verifier agents, reviews output, iterates until quality
  gates pass, and presents final results to the Project Manager.
- **Worker Agents**: Execute bite-sized work items and report back to the
  Software Lead. They do NOT spawn sub-agents or interact with the PM.
- **Verifier Agents**: Check worker output for correctness, standards
  compliance, and completeness. They report back to the Software Lead with
  APPROVED or NEEDS CHANGES. They do NOT modify files.
- **Project Manager** (user): Provides domain knowledge, design rationale,
  and final approval. Has override authority on all AI decisions.

### Orchestration Loop

```
1. PM assigns task → Software Lead
2. Lead consults lessons-learned file
3. Lead creates Work Breakdown Structure with acceptance criteria
4. Lead presents plan to PM for approval
5. WORK LOOP (repeat until all verifiers approve):
   a. Lead spawns N worker agents (bite-sized briefs)
   b. Workers complete and report back
   c. Lead spawns N verifier agents to check worker output
   d. Verifiers report findings (APPROVED / NEEDS CHANGES)
   e. If NEEDS CHANGES → Lead spawns workers to fix → re-verify
6. Lead spawns Final Quality Engineer for overall product check
7. Lead presents structured completion report to PM
8. PM approves → done | PM requests changes → back to step 5
9. Lead updates lessons-learned files with insights
```

## Sprint Startup Protocol (MANDATORY)

At the beginning of every sprint, BEFORE creating or presenting a sprint plan,
the Software Lead MUST:

1. Re-read the software-lead agent file: `.claude/agents/software-lead.md`
2. Read the worker and verifier skill files relevant to the sprint (`ai/skills/`)
3. Read the relevant requirements (e.g., `requirements/vscode-extension/requirements.json`)
4. Read the software design document (e.g., `vscode-extension/design/design.md`)
5. Read the test cases document (e.g., `vscode-extension/design/test-cases.md`)
6. Read the software development plan for the current sprint's phase(s)
7. Summarize each document's key points relevant to the sprint
8. Only THEN create and present the sprint plan to the PM

This protocol is non-negotiable. Skipping it leads to stale context, missed
constraints, and plans that contradict prior decisions.

## Post-Compaction Recovery Protocol

After every conversation compaction (context window reset), the **Software Lead**
must re-read the following files before resuming work:

1. **Agent file**: `.claude/agents/software-lead.md`
2. **Lessons learned**: `ai/memory/lessons-learned-software-lead.md`
3. **Project memory**: `ai/memory/project-overview.md`, `ai/memory/architecture.md`,
   `ai/memory/coding-standards.md`, `ai/memory/constraints.md`, `ai/memory/traceability.md`
4. **Current project requirements & design**: For the VSCode extension:
   - `requirements/vscode-extension/requirements.json`
   - `vscode-extension/design/design.md`
   - `vscode-extension/design/test-cases.md`
5. **Reiterate plans**: Present both the high-level plan (all sprints) and the
   tactical plan (current sprint work breakdown) to the PM before resuming work.

## Available Agents

| Agent | Role Type | Model | Agent File |
|-------|-----------|-------|-----------|
| `software-lead` | Orchestrator | claude-opus-4-7 | `.claude/agents/software-lead.md` |
| `software-developer` | Worker | claude-sonnet-4-6 | `.claude/agents/software-developer.md` |
| `software-test-engineer` | Worker | claude-sonnet-4-6 | `.claude/agents/software-test-engineer.md` |
| `software-requirements-engineer` | Worker | claude-sonnet-4-6 | `.claude/agents/software-requirements-engineer.md` |
| `junior-software-developer` | Worker | claude-haiku-4-5-20251001 | `.claude/agents/junior-software-developer.md` |
| `software-quality-engineer` | Verifier | claude-sonnet-4-6 | `.claude/agents/software-quality-engineer.md` |
| `software-systems-engineer` | Verifier | claude-sonnet-4-6 | `.claude/agents/software-systems-engineer.md` |
| `senior-software-engineer` | Verifier | claude-sonnet-4-6 | `.claude/agents/senior-software-engineer.md` |
| `software-verification-engineer` | Verifier | claude-sonnet-4-6 | `.claude/agents/software-verification-engineer.md` |
| `final-quality-engineer` | Final Gate | claude-sonnet-4-6 | `.claude/agents/final-quality-engineer.md` |

## Memory Files

Detailed project knowledge is stored in `ai/memory/`:

| File | Contents |
|------|----------|
| `project-overview.md` | Project description, philosophy, module catalog |
| `coding-standards.md` | Naming, style, documentation, error handling |
| `architecture.md` | Module system, vtable DI, initialization pattern |
| `constraints.md` | Hard technical and traceability constraints |
| `traceability.md` | Requirements JSON schema, annotation format |
| `directory-map.md` | Working directory reference for all commands |

## Lessons Learned Files

Each agent type has a persistent lessons-learned file in `ai/memory/`:

| File | Agent |
|------|-------|
| `lessons-learned-software-lead.md` | Software Lead |
| `lessons-learned-software-developer.md` | Software Developer |
| `lessons-learned-software-test-engineer.md` | Software Test Engineer |
| `lessons-learned-software-requirements-engineer.md` | Software Requirements Engineer |
| `lessons-learned-junior-software-developer.md` | Junior Software Developer |
| `lessons-learned-software-quality-engineer.md` | Software Quality Engineer |
| `lessons-learned-software-systems-engineer.md` | Software Systems Engineer |
| `lessons-learned-senior-software-engineer.md` | Senior Software Engineer |
| `lessons-learned-software-verification-engineer.md` | Software Verification Engineer |
| `lessons-learned-final-quality-engineer.md` | Final Quality Engineer |

Always read the relevant lessons-learned file before performing any task.

## Skill Files

Each agent has a detailed skill file in `ai/skills/`:

| Skill File | Purpose |
|-----------|---------|
| `ai/skills/software-lead.md` | Orchestration loop, work breakdown, verification checklists, PM presentation format |
| `ai/skills/software-developer.md` | Implementation code, design proposals, module scaffolding, documentation |
| `ai/skills/software-test-engineer.md` | Test writing (C/Python/JS), test doubles, DI patterns, coverage |
| `ai/skills/software-requirements-engineer.md` | Requirements authoring, derivation, traceability annotations |
| `ai/skills/junior-software-developer.md` | Boilerplate, scaffolding, repetitive edits, search-and-summarize |
| `ai/skills/software-quality-engineer.md` | Standards compliance, naming, documentation quality verification |
| `ai/skills/software-systems-engineer.md` | Architecture, DI patterns, integration, design verification |
| `ai/skills/senior-software-engineer.md` | Correctness, edge cases, security, error handling verification |
| `ai/skills/software-verification-engineer.md` | Traceability audit, requirements/test coverage verification |
| `ai/skills/final-quality-engineer.md` | Final product gate — holistic quality check before PM presentation |

## Traceability System

- Requirements live in `requirements/<module>/requirements.json`
- Source code tags: `// @{"req": ["REQ-MODULE-NNN"]}`
- Test function tags: `// @{"verify": ["REQ-MODULE-NNN"]}`
- `"uses"` points UP (to parent requirement)
- `"implements"` points DOWN (to child requirement)
- Verification methods: Test, Inspection, Analysis, Demonstration

## Slash Commands

| Command | Purpose |
|---------|---------|
| `/software-lead` | Start the full orchestration loop for a sprint/task |
| `/worker` | Dispatch a single worker agent directly (bypass orchestration) |
| `/verify` | Run verifier agents on completed work output |
| `/final-quality` | Invoke the final quality gate before PM presentation |
