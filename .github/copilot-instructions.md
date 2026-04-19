# Copilot Instructions — LibJuno

## Project Summary

LibJuno is a freestanding C11 embedded systems micro-framework with zero dynamic
memory allocation. It uses dependency injection via vtables and struct embedding.
All memory is caller-owned and injected.

## Key Rules

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

### Lessons Learned System

Each agent type has a persistent lessons-learned file at
`ai/memory/lessons-learned-<agent-type>.md`. Agents read their file before
starting any task and append new entries when mistakes are discovered.

## Available AI Agents

### Orchestrator

| Agent | Role | Model |
|-------|------|-------|
| `software-lead` | Sole orchestrator — plans, delegates, verifies, presents | Claude Opus 4.6 |

### Worker Agents

| Agent | Role | Model |
|-------|------|-------|
| `software-developer` | Write code, design, module scaffolding, documentation | Claude Sonnet 4.6 |
| `software-test-engineer` | Write tests, test doubles, coverage analysis | Claude Sonnet 4.6 |
| `software-requirements-engineer` | Write/derive requirements, traceability annotations | Claude Sonnet 4.6 |
| `junior-software-developer` | Boilerplate, repetitive edits, search-and-summarize | GPT-4o |

### Verifier Agents

| Agent | Role | Model |
|-------|------|-------|
| `software-quality-engineer` | Coding standards, naming, Doxygen, no dynamic allocation | Claude Sonnet 4.6 |
| `software-systems-engineer` | Architecture, DI patterns, module integration, design | Claude Sonnet 4.6 |
| `senior-software-engineer` | Code correctness, edge cases, security, error handling | Claude Sonnet 4.6 |
| `software-verification-engineer` | Traceability, requirements coverage, test coverage | Claude Sonnet 4.6 |
| `final-quality-engineer` | Overall product verification — final gate before PM | Claude Sonnet 4.6 |

## Traceability System

- Requirements live in `requirements/<module>/requirements.json`
- Source code tags: `// @{"req": ["REQ-MODULE-NNN"]}`
- Test function tags: `// @{"verify": ["REQ-MODULE-NNN"]}`
- `"uses"` points UP (to parent requirement)
- `"implements"` points DOWN (to child requirement)
- Verification methods: Test, Inspection, Analysis, Demonstration

## AI Skills

Each agent has a corresponding skill file at `ai/skills/<agent-name>.md`
containing detailed domain instructions. The Software Lead reads the relevant
skill file before delegating work.

| Skill | Purpose |
|-------|---------|
| `software-lead` | Orchestration loop, work breakdown, verification checklists, PM presentation format |
| `software-developer` | Implementation code, design proposals, module scaffolding, documentation |
| `software-test-engineer` | Test writing (C/Python/JS), test doubles, DI patterns, coverage |
| `software-requirements-engineer` | Requirements authoring, derivation, traceability annotations |
| `junior-software-developer` | Boilerplate, scaffolding, repetitive edits, search-and-summarize |
| `software-quality-engineer` | Standards compliance, naming, documentation quality verification |
| `software-systems-engineer` | Architecture, DI patterns, integration, design verification |
| `senior-software-engineer` | Correctness, edge cases, security, error handling verification |
| `software-verification-engineer` | Traceability audit, requirements/test coverage verification |
| `final-quality-engineer` | Final product gate — holistic quality check before PM presentation |

## Sprint Startup Protocol (MANDATORY)

At the beginning of every sprint, BEFORE creating or presenting a sprint plan,
the Software Lead MUST:

1. Re-read the software-lead agent/skill file (`ai/skills/software-lead.md`)
2. Read the worker and verifier skill files relevant to the sprint
3. Read the relevant requirements (e.g., `requirements/vscode-extension/requirements.json`)
4. Read the software design document (e.g., `vscode-extension/design/design.md`)
5. Read the test cases document (e.g., `vscode-extension/design/test-cases.md`)
6. Read the software development plan for the current sprint's phase(s)
7. Summarize each document's key points relevant to the sprint
8. Only THEN create and present the sprint plan to the PM

This protocol is non-negotiable. Skipping it leads to stale context, missed
constraints, and plans that contradict prior decisions.

## Memory Files

Detailed project knowledge is stored in `ai/memory/`:

| File                  | Contents                                        |
|-----------------------|-------------------------------------------------|
| `project-overview.md` | Project description, philosophy, module catalog |
| `coding-standards.md` | Naming, style, documentation, error handling    |
| `architecture.md`     | Module system, vtable DI, initialization pattern|
| `constraints.md`      | Hard technical and traceability constraints      |
| `traceability.md`     | Requirements JSON schema, annotation format     |
| `directory-map.md`    | Working directory reference for all commands    |

### Lessons Learned Files

Each agent type has a persistent lessons-learned file:

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

**Always read the relevant memory files before performing any task.**

## Post-Compaction Recovery Protocol

After every conversation compaction (context window reset), the **Software Lead**
must re-read the following files before resuming work:

1. **Agent & skill files**: `.github/agents/software-lead.md`, `ai/skills/software-lead.md`
2. **Lessons learned**: `ai/memory/lessons-learned-software-lead.md`
3. **Project memory**: `ai/memory/project-overview.md`, `ai/memory/architecture.md`,
   `ai/memory/coding-standards.md`, `ai/memory/constraints.md`, `ai/memory/traceability.md`
4. **Current project requirements & design**: For the VSCode extension:
   - `requirements/vscode-extension/requirements.json`
   - `vscode-extension/design/design.md`
   - `vscode-extension/design/test-cases.md`
5. **Reiterate plans**: Present both the high-level plan (all sprints) and the
   tactical plan (current sprint work breakdown) to the PM before resuming work.
