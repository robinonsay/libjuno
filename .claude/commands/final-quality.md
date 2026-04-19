---
description: "Invoke the final quality gate before presenting work to the Project Manager. Runs build, tests, traceability script, and acceptance-criteria check. Usage: /final-quality <work item list or 'all'>"
---

You are invoking the **final quality gate** before presenting work to the Project Manager.

**Scope / Acceptance criteria:** $ARGUMENTS

Spawn the `final-quality-engineer` agent with the following brief:

1. **Acceptance criteria to verify:** `$ARGUMENTS` (or "all work items from the current sprint" if 'all' was specified)

2. **The final-quality-engineer MUST run these commands:**
   ```bash
   # Build verification (if C code was produced)
   cd /workspaces/libjuno && cd build && cmake --build . 2>&1

   # Test suite (if C code was produced)
   cd /workspaces/libjuno && cd build && ctest --output-on-failure

   # VSCode extension tests (if extension code was produced)
   cd /workspaces/libjuno/vscode-extension && npm test

   # Traceability verification (MANDATORY for all work involving code, tests, or requirements)
   cd /workspaces/libjuno && python3 scripts/verify_traceability.py
   ```

3. **The final-quality-engineer MUST check:**
   - Each acceptance criterion: MET or UNMET with evidence
   - Build cleanliness (zero errors and warnings under -Werror)
   - Test suite: all tests pass, zero failures
   - Traceability: `verify_traceability.py` exits with code 0
   - Cross-item consistency: no duplicate definitions, no incompatible vtable changes, no conflicting REQ IDs
   - No regressions in previously passing tests
   - Documentation accuracy (if docs were produced)

4. **Verdict:**
   - **APPROVED** — all checks pass, every acceptance criterion is MET
   - **REJECTED** — any check fails; list every blocking issue with file:line and description

5. **Context files to read:**
   - `ai/memory/architecture.md`, `ai/memory/coding-standards.md`, `ai/memory/constraints.md`, `ai/memory/traceability.md`
   - `ai/memory/lessons-learned-final-quality-engineer.md`

Return the Final Quality Assessment report to the user. If REJECTED, the findings will indicate what needs to be fixed before PM presentation.
