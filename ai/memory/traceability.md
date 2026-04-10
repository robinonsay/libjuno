# Traceability System — LibJuno

## Overview

LibJuno uses convention-based, bi-directional traceability linking:

```
Requirements ↔ Architecture ↔ Design ↔ Code ↔ Tests
```

Code and tests are the **single source of truth** (Agile methodology).
Formal artifacts (SRS, SDD, RTM) are **derived from** the codebase.

## Requirements Schema

Each module has a `requirements.json` file located at:

```
requirements/<module>/requirements.json
```

### JSON Schema

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
    "module": {
      "type": "string",
      "description": "Module identifier (e.g., HEAP, MEMORY, CRC)"
    },
    "requirements": {
      "type": "array",
      "items": {
        "type": "object",
        "required": ["id", "title", "description", "rationale", "verification_method"],
        "properties": {
          "id": {
            "type": "string",
            "pattern": "^REQ-[A-Z]+-[0-9]{3}$",
            "description": "Unique requirement ID (e.g., REQ-HEAP-001)"
          },
          "title": {
            "type": "string",
            "description": "Short requirement title"
          },
          "description": {
            "type": "string",
            "description": "Full requirement statement (shall language)"
          },
          "rationale": {
            "type": "string",
            "description": "Why this requirement exists (provided by Program/user)"
          },
          "verification_method": {
            "type": "string",
            "enum": ["Test", "Inspection", "Analysis", "Demonstration"],
            "description": "How this requirement is verified"
          },
          "uses": {
            "type": "array",
            "items": { "type": "string" },
            "description": "Higher-level requirement IDs this supports (points UP)"
          },
          "implements": {
            "type": "array",
            "items": { "type": "string" },
            "description": "Lower-level requirement IDs that realize this (points DOWN)"
          }
        }
      }
    }
  }
}
```

### Example

```json
{
  "module": "HEAP",
  "requirements": [
    {
      "id": "REQ-HEAP-001",
      "title": "Heap Initialization",
      "description": "The heap module shall initialize from a caller-provided array and pointer API without dynamic allocation.",
      "rationale": "Supports freestanding environments where malloc is unavailable.",
      "verification_method": "Test",
      "uses": ["REQ-SYS-001"],
      "implements": ["REQ-HEAP-002", "REQ-HEAP-003"]
    }
  ]
}
```

## Code Annotations

### Source code (`.c`, `.h` files)

Tag functions that implement a requirement:

```c
// @{"req": ["REQ-HEAP-001"]}
JUNO_STATUS_T JunoDs_Heap_Init(JUNO_DS_HEAP_ROOT_T *ptHeap, ...) {
    ...
}
```

### Test code (`.c`, `.cpp` files)

Tag test functions that verify a requirement:

```c
// @{"verify": ["REQ-HEAP-001"]}
void test_heap_init_success(void) {
    ...
}
```

### Placement Rules

- Tags are placed **immediately above the function definition** (one line above).
- A single function may trace to **multiple requirements**.
- Tags use standard C line comments (`//`).

## Hierarchy Model (Two-Tier)

```
High-Level Requirement (e.g., REQ-SYS-001)
    │
    │  "uses" (child points to parent)
    ▼
Detailed Requirement (e.g., REQ-HEAP-001)
```

- **`uses`**: This requirement supports/derives from a higher-level requirement.
- **`implements`**: This requirement is realized by lower-level requirements.

## Generated Artifacts

| Artifact | Standard   | Format              | Tool                        |
|----------|------------|---------------------|-----------------------------|
| SRS      | IEEE 830   | AsciiDoc, HTML, PDF | `scripts/generate_docs.py`  |
| SDD      | IEEE 1016  | AsciiDoc, HTML, PDF | `scripts/generate_docs.py`  |
| RTM      | Custom     | AsciiDoc, HTML, PDF | `scripts/generate_docs.py`  |

### RTM Matrix Columns

| Requirement | Code Location | Test Location | Verification Method | Status |
|-------------|--------------|---------------|---------------------|--------|

### Consistency Validation

The Python tool validates:
- Every requirement in `requirements.json` has at least one code annotation
- Every requirement has at least one test annotation (if verification = Test)
- No orphaned code/test tags referencing nonexistent requirements
- All `uses`/`implements` links resolve to valid requirement IDs
- Warnings are produced for traceability gaps
