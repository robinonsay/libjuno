# Lessons Learned — Software Test Engineer

## How to Use This File

- **Read** this file before starting any task
- **Append** a new entry when a mistake or insight is discovered during work or verification
- **Keep entries concise**: what went wrong, root cause, and corrective action
- **Do not delete** old entries — they form institutional knowledge
- **Date** each entry for chronological tracking

## Lessons

### 2026-04-14 — Tests must verify behavior, not just return status

**What went wrong:** Tests were written to pass rather than to verify the code under test.
Common patterns included: asserting only `JUNO_STATUS_SUCCESS` without checking outputs or state,
writing always-pass tests that a no-op stub would satisfy, and configuring test doubles to
return exactly the value the test then asserted (tautological doubles).

**Root cause:** Focusing on "make the test compile and run" rather than "specify what the code
must do and prove it does it."

**Corrective action (applied to all future tests):**
1. After every function call, ask: *"What did this function change or produce?"* — assert on it.
2. For mutating functions (push, write, insert, encode): assert the resulting state (count, contents, output buffer).
3. For output-parameter functions: assert the output parameter contains the expected value.
4. For error paths: assert the exact `JUNO_STATUS_*` error code, never just `!= SUCCESS`.
5. For test doubles with `call_count`: always assert the counter after the test scenario.
6. Before submitting: mentally replace the function under test with `return JUNO_STATUS_SUCCESS;` — if the test still passes, the test is defective.

### 2026-04-14 — Verify source code behavior empirically before writing tests
**What happened:** Four test files were written based on design docs and type definitions. All tests assumed the parser/visitor would produce certain records, but the source code had bugs that caused all extraction to return empty arrays. Result: 26 out of 26 non-lexer tests failed.
**Root cause:** Tests were written against expected behavior (from design docs) without first running a small diagnostic to confirm the source code actually produces that behavior.
**Corrective action:**
1. Before writing a test file, run a minimal diagnostic (e.g., `parseFile('/test.h', 'struct FOO_TAG JUNO_MODULE_ROOT(FOO_API_T, JUNO_MODULE_EMPTY);')`) and inspect the actual output.
2. If the source code returns unexpected results, report this to the Software Lead as a source bug — do NOT write tests that will fail due to known source bugs.
3. When the brief says "write tests for X," first confirm X works by running it. If it doesn't, stop and report back.

### 2026-04-17 — Jest test files require `/// <reference types="jest" />` directive

**What went wrong:** A new test file in `src/resolver/__tests__/` failed to compile with TS2593 ("Cannot find name 'describe'") even though `@types/jest` is installed.

**Root cause:** `tsconfig.json` has `"types": ["node"]` which excludes jest globals from the default type resolution. Existing tests work because they include `/// <reference types="jest" />` at the top of the file.

**Corrective action:** Always add `/// <reference types="jest" />` as the first line of every new TypeScript test file in this project.

### 2026-04-14 — Chevrotain CST property names differ from what design docs suggest
**What happened:** Test expectations used property names like `variableName`, `variableType` from approximate descriptions, but the actual TypeScript interfaces use `apiType`, `field`, `functionName`, etc.
**Root cause:** Tests were written from design-doc descriptions rather than reading the actual `types.ts` interface definitions.
**Corrective action:** Always read the actual TypeScript interface definitions in `types.ts` before writing test expectations. Use `toMatchObject()` with the exact property names from the interface, not approximations from design docs.

### 2026-04-17 — Always use absolute directory paths when running test commands
**What happened:** Agents ran `npm test` or `npx jest` from the project root instead of `vscode-extension/`, or ran `ctest` from `vscode-extension/` instead of the project root.
**Root cause:** Commands used relative paths without verifying the current working directory.
**Corrective action:** Read `ai/memory/directory-map.md` before running any test command. Use absolute paths: `cd /workspaces/libjuno/vscode-extension && npm test` for extension tests, `cd /workspaces/libjuno && cd build && ctest --output-on-failure` for C tests.
