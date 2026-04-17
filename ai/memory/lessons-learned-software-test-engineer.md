# Lessons Learned — Software Test Engineer
*Read before every task. Append new entries concisely.*

### 2026-04-14 — Tests must verify behavior, not just return status
- After every call, assert what changed (state, output buffer, output params, call_count).
- For error paths: assert the exact `JUNO_STATUS_*` code, never just `!= SUCCESS`.
- Sanity check: if replacing the function under test with `return JUNO_STATUS_SUCCESS;` still passes the test, the test is defective.

### 2026-04-14 — Verify source code behavior empirically before writing tests
- Run a minimal diagnostic first to confirm source actually produces expected output.
- If source returns wrong results, report to Lead as a source bug — do NOT write failing tests.
- "Write tests for X" means: run X first, confirm it works, then write tests.

### 2026-04-14 — Always add `/// <reference types="jest" />` to new TS test files
- `tsconfig.json` has `"types": ["node"]` — jest globals excluded without the directive.
- Add `/// <reference types="jest" />` as the FIRST line of every new TypeScript test file.

### 2026-04-14 — Read actual TypeScript interfaces before writing test expectations
- Use exact property names from `types.ts`, not approximations from design docs.
- Use `toMatchObject()` with verified property names.

### 2026-04-17 — Always use absolute directory paths when running test commands
- Read `ai/memory/directory-map.md` before any test command.
- Extension: `cd /workspaces/libjuno/vscode-extension && npm test`
- C lib: `cd /workspaces/libjuno && cd build && ctest --output-on-failure`

### 2026-04-17 — Verify regex match boundaries empirically before writing boundary tests
- Run `node -e` to exec the exact regexes against test lines and log m.index + m[0].length.
- The column boundary check is `column >= m.index && column < m.index + m[0].length` (exclusive end).
- For vtableResolver.ts strategies: macroRe=[0,48), arrayRe=[4,33), generalRe=[4,22) for the shared test lines.
- Always verify one-past-end is actually outside ALL strategy matches (e.g. col 33 is end of both arrayRe AND generalRe).

### 2026-04-17 — JUNO_MODULE_SUPER does NOT match generalRe
- `JUNO_MODULE_SUPER(ptSelf, MY_ROOT_T)->DoThing(` — the `(` immediately after the word prevents generalRe from connecting to the `->field(` part.
- Result: all 3 strategies return no match → resolver returns "No LibJuno API call pattern found".
- Test this as found=false → documents actual behavior, not a bug.

### 2026-04-17 — Read entire test file before adding new tests; file may be larger than first read
- `read_file` with endLine=150 only reads 150 lines; use `wc -l` to check actual length first.
- If file has pre-existing content from a prior session, the new describe blocks may duplicate IDs.
- Import `TypeInfo` explicitly when used in `new Map<string, TypeInfo>()` type annotations.

### 2026-04-17 — WorkspaceIndexer deferred positional mechanism: producer pipeline is missing
- `DeferredPositional` interface and `resolveDeferred()` exist but the visitor's `extractPositionalVtable()` silently returns (does not push deferred entries) when the API struct is not in the same file.
- Cross-file positional initializers are silently dropped — test only the same-file path (which works).
- Report this to the Lead as a known implementation gap.

### 2026-04-17 — resolveFailureHandlerRootType() has multi-module disambiguation bug
- When two module roots are in the same file, both failure handlers are attributed to the FIRST root type found in localTypeInfo.functionParameters iteration order.
- Test the single-module (unambiguous) case only; document multi-module limitation in test comment.
