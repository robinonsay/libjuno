# Skill: software-requirements-engineer

## Purpose

Author new requirements, derive requirements from existing code and tests, manage traceability annotations, and maintain requirements.json files. Ensure full bidirectional traceability between requirements, source code, and test code.

## When to Use

- Writing new requirements for a module before code is written
- Deriving requirements from an existing public API and test suite
- Adding or updating `// @{"req": [...]}` and `// @{"verify": [...]}` traceability tags
- Creating or maintaining `requirements/<module>/requirements.json` files
- Validating traceability completeness (every requirement tested, every test traced)

## Inputs Required

The Software Lead's brief must contain:

- **Task type**: "author new" or "derive from code" or "add traceability tags"
- **Module name**: Which module to work on (maps to `requirements/<module>/`)
- **PM rationale**: Design intent and rationale from the Project Manager (for authoring)
- **Scope**: Which functions/features/requirements to focus on
- **Context files**: Paths to relevant headers, source, tests, existing requirements
- **Parent requirements**: Any higher-level requirement IDs that the new requirements refine

## Instructions

### Authoring New Requirements

When writing requirements before implementation exists:

1. **Survey existing requirements** — read `requirements/<module>/requirements.json` if it exists. Note the style, granularity, ID numbering, and how `uses`/`implements` are used. If the module has no requirements yet, survey a neighboring module for conventions.

2. **Understand the PM's intent** — the brief contains the PM's design rationale. Every requirement must trace back to a PM decision or design goal. Do not invent rationale.

3. **Draft requirements in "shall" language**:
   - Pattern: `"The <module> module shall <verb> <object> <condition>."`
   - One behavior per requirement. If a sentence contains "and" linking two distinct behaviors, split it into two requirements.
   - Be precise about inputs, outputs, error conditions, and side effects.
   - Avoid implementation details — describe *what*, not *how*.

4. **Assign IDs**:
   - Format: `REQ-<MODULE>-<NNN>` (e.g., `REQ-QUEUE-001`, `REQ-HEAP-012`).
   - Module name is uppercase.
   - Numbers are zero-padded to three digits.
   - Assign sequentially. If existing requirements go up to 005, start at 006.

5. **Assign verification methods** — choose the most appropriate:
   - **Test**: The requirement can be verified by running a test (default for behavioral requirements).
   - **Inspection**: The requirement is structural and verified by code review (e.g., "shall not use dynamic allocation").
   - **Analysis**: The requirement is verified by formal or mathematical analysis (e.g., "shall have O(1) complexity").
   - **Demonstration**: The requirement is verified by running the system and observing behavior (e.g., "shall produce output on the serial console").

6. **Add traceability links**:
   - `"uses"`: Points **UP** to parent requirements. If this requirement refines `REQ-SYS-010`, then `"uses": ["REQ-SYS-010"]`.
   - `"implements"`: Points **DOWN** to child requirements. If this requirement is fulfilled by `REQ-QUEUE-001A` and `REQ-QUEUE-001B`, then `"implements": ["REQ-QUEUE-001A", "REQ-QUEUE-001B"]`.
   - Use empty arrays `[]` when no links exist.

7. **Include rationale** — copy the PM's rationale verbatim into the `"rationale"` field. If the brief does not provide rationale for a specific requirement, set `"rationale": ""` and add it to open questions.

### Deriving Requirements from Code

When extracting requirements from an existing codebase:

1. **Analyze the public API**:
   - Read the public header: `include/juno/<module>.h`
   - For each public function, document: name, parameters, return type, preconditions (asserts), postconditions.
   - For each public type, document: fields, valid states, invariants.

2. **Analyze existing tests**:
   - Read test files: `tests/test_<module>.c`
   - For each test function, extract what behavior is being verified.
   - Note test assertions — each assertion reveals an expected behavior.

3. **Extract behavioral requirements**:
   - Each public function's contract → one or more requirements.
   - Each error condition handled → one error-handling requirement.
   - Each test scenario without a matching requirement → candidate requirement.

4. **Draft requirements** — use "shall" language that matches the observed behavior. Be faithful to what the code actually does, not what you think it should do.

5. **Apply rationale** — the Software Lead's brief includes PM rationale. Attach rationale to each derived requirement. If rationale is not available for a specific behavior, flag it.

6. **Cross-reference**:
   - Every public API function → at least one requirement.
   - Every test assertion → at least one requirement.
   - Flag any untested requirements or untraced tests.

### Traceability Annotations

#### Source Code Tags (`@req`)

Place the tag on the line immediately above the function definition:

```c
// @{"req": ["REQ-MODULE-001"]}
JUNO_STATUS_T Module_Init(MODULE_T *pModule, const MODULE_CFG_T *pCfg)
{
    ...
}
```

Rules:
- One tag per function (may list multiple REQ IDs).
- Tag goes above the return type, not above the Doxygen comment.
- Only tag functions that directly implement the requirement's behavior.

#### Test Code Tags (`@verify`)

Place the tag on the line immediately above the test function definition:

```c
// @{"verify": ["REQ-MODULE-001"]}
static void test_module_init_success(void)
{
    ...
}
```

Rules:
- One tag per test function (may list multiple REQ IDs).
- A requirement may be verified by multiple test functions.
- Every requirement with `verification_method: "Test"` must have at least one `@verify` tag somewhere in the test suite.

#### Validation Checklist

After adding tags, verify:
- [ ] Every requirement with `verification_method: "Test"` has at least one `@verify` tag.
- [ ] Every `@req` tag references a valid ID in requirements.json.
- [ ] Every `@verify` tag references a valid ID in requirements.json.
- [ ] No orphaned tags (IDs that don't exist in requirements.json).
- [ ] `uses`/`implements` links are bidirectionally consistent.

### Requirements JSON Schema

Reference: `ai/memory/traceability.md`

Each requirement object:

```json
{
    "id": "REQ-MODULE-001",
    "title": "Short descriptive title",
    "description": "The <module> module shall <behavior>.",
    "rationale": "PM-provided rationale explaining why this requirement exists.",
    "verification_method": "Test",
    "uses": ["REQ-PARENT-001"],
    "implements": ["REQ-CHILD-001"]
}
```

Field rules:
- `id`: `REQ-<MODULE>-<NNN>` — uppercase, zero-padded three digits.
- `title`: Brief noun-phrase title (< 80 chars).
- `description`: "shall" language. One behavior. No implementation details.
- `rationale`: From PM only. Empty string `""` if not provided (flag as open question).
- `verification_method`: Exactly one of `"Test"`, `"Inspection"`, `"Analysis"`, `"Demonstration"`.
- `uses`: Array of parent REQ IDs (points UP). `[]` if none.
- `implements`: Array of child REQ IDs (points DOWN). `[]` if none.

File location: `requirements/<module>/requirements.json`

The file is a JSON object with a top-level `"requirements"` array:

```json
{
    "requirements": [
        { "id": "REQ-MODULE-001", ... },
        { "id": "REQ-MODULE-002", ... }
    ]
}
```

## Constraints

- Do **NOT** fabricate rationale — only use what came from the PM via the Software Lead.
- Do **NOT** write implementation code or tests.
- Do **NOT** interact with the PM directly — report questions to the Software Lead.
- Do **NOT** assume requirements without evidence from the API, tests, or PM.
- Do **NOT** modify source/test code except to add traceability annotation tags.
- Do **NOT** create compound requirements — split "A and B" into two requirements.
- Do **NOT** include implementation details in requirement descriptions — describe *what*, not *how*.
- **DO** match the style and granularity of existing requirements in the project.
- **DO** ensure bidirectional consistency of `uses`/`implements` links.
- **DO** flag missing rationale as open questions rather than inventing it.

## Output Format

Return to the Software Lead:

1. **requirements.json** — the created or modified requirements file (complete, valid JSON).
2. **Annotated files** — any source or test files with added `@req` or `@verify` tags (show the diff or full file).
3. **Summary table**:

| REQ ID | Title | Verification | Uses | Implements | Rationale Provided? |
|--------|-------|-------------|------|------------|---------------------|
| REQ-MODULE-001 | Module initialization | Test | REQ-SYS-010 | — | Yes |
| REQ-MODULE-002 | Error handling on write | Test | REQ-SYS-010 | — | No (flagged) |

4. **Open questions** — anything ambiguous, missing rationale, unclear scope, or requiring PM clarification.
