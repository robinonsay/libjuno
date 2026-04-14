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

## AI Workflow: Software Lead / Software Developer / Project Manager

The AI operates in two roles:

- **Software Lead**: Plans tasks, sets acceptance criteria, delegates work to
  Software Developer sub-agents, verifies quality, and asks the Project Manager
  (user) for domain knowledge and approval.
- **Software Developer**: Executes the task following the Software Lead's plan
  and project constraints.

The **Project Manager** (user) provides:
- Design rationale and domain knowledge
- Final approval on all outputs
- Override authority on any AI decision

The Software Lead and Software Developers must **proactively ask the Project Manager**
rather than make assumptions. This avoids hallucinations, miscommunication, and
incorrect decisions.

## Traceability System

- Requirements live in `requirements/<module>/requirements.json`
- Source code tags: `// @{"req": ["REQ-MODULE-NNN"]}`
- Test function tags: `// @{"verify": ["REQ-MODULE-NNN"]}`
- `"uses"` points UP (to parent requirement)
- `"implements"` points DOWN (to child requirement)
- Verification methods: Test, Inspection, Analysis, Demonstration

## Available AI Skills

Invoke a skill by saying: **"Use skill: `<skill-name>`"**

| Skill                  | Purpose                                                    |
|------------------------|------------------------------------------------------------|
| `software-lead`        | Software Lead planning, delegation, and verification guide |
| `derive-requirements`  | Extract requirements from existing code and tests          |
| `write-requirements`   | Author new requirements before code exists                 |
| `write-tests`          | Generate Unity tests with traceability tags                |
| `generate-docs`        | Produce SRS (IEEE 830), RTM with consistency validation    |
| `generate-sdd`         | Produce SDD (IEEE 1016) from code + user rationale         |
| `write-design`         | Propose software designs from requirements before code     |
| `trace-check`          | Audit traceability completeness and consistency            |
| `code-review`          | Review code against all project standards                  |
| `write-module`         | Scaffold a new module with all conventions                 |
| `improve-docs`         | Closed-loop iterative evaluation & improvement of docs     |

## Memory Files

Detailed project knowledge is stored in `ai/memory/`:

| File                  | Contents                                        |
|-----------------------|-------------------------------------------------|
| `project-overview.md` | Project description, philosophy, module catalog |
| `coding-standards.md` | Naming, style, documentation, error handling    |
| `architecture.md`     | Module system, vtable DI, initialization pattern|
| `constraints.md`      | Hard technical and traceability constraints      |
| `traceability.md`     | Requirements JSON schema, annotation format     |

**Always read the relevant memory files before performing any task.**
