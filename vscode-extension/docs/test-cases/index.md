# LibJuno VSCode Extension — Test Case Specification

**Date:** 2026-04-14  
**Module:** VSCODE  
**Status:** Draft — for translation into Jest tests

All inputs are verbatim text from the LibJuno codebase (noted by file and line) unless explicitly
marked *(synthetic)*. Column numbers are 1-based. All test inputs and expected records are derived
from the Chevrotain parser/visitor architecture described in `vscode-extension/design/design.md`
Section 3.

---

## Lessons-Learned Coverage Map

| # | Lesson | Test Cases Covering It |
|---|--------|------------------------|
| 1 | Indirect API pointer (`ptLoggerApi->LogInfo`) — Category 3 (Direct API pointer) | TC-P9-301, TC-P9-302, TC-P9-303, TC-RES-001 |
| 2 | Array subscript in receiver (`ptAppList[i]->ptApi->OnStart`) — Category 1 strip `[i]` | TC-P9-103, TC-P9-104, TC-RES-002 |
| 3 | Chained member access (`ptEngineApp->ptBroker->ptApi->RegisterSubscriber`) | TC-P9-105, TC-RES-003 |
| 4 | Dot-accessed `ptApi` (`tReturn.ptApi->Copy`, `tPtrResult.tOk.ptApi->Copy`) | TC-P9-201, TC-P9-202, TC-RES-005 |
| 5 | Non-`ptApi` API member names (`ptHeap->ptHeapPointerApi->Compare`) — Category 4 (Named API member) | TC-P9-401, TC-P9-402, TC-RES-004 |
| 6 | Static function definitions — visitFunctionDefinition must capture `{` not `;` | TC-P11-005, TC-P11-006 |
| 7 | Positional initializers — visitVtableDeclaration (positional) must zip by field order from visitStructDefinition | TC-P8-001 through TC-P8-004 |
| 8 | Function definition varied return types — visitFunctionDefinition | TC-P11-001 through TC-P11-005 |

---

## Document Structure

| File | Content |
|------|---------|
| [01-visitor-struct.md](01-visitor-struct.md) | Sections 1-4: visitStructDefinition (MODULE_ROOT, DERIVE, TRAIT_ROOT, TRAIT_DERIVE) |
| [02-visitor-api-vtable.md](02-visitor-api-vtable.md) | Sections 5-8: API Struct Field Extraction, Vtable Declaration variants |
| [03-chain-walk.md](03-chain-walk.md) | Section 9: Chain-Walk Call Site Resolution |
| [04-failure-handler.md](04-failure-handler.md) | Section 10: visitFailureHandlerAssignment; Section 10b: FAIL Macro Failure Handler Navigation |
| [05-function-definition.md](05-function-definition.md) | Section 11: visitFunctionDefinition |
| [06-e2e-resolution.md](06-e2e-resolution.md) | Section 12: End-to-End Resolution Tests |
| [07-vscode-integration.md](07-vscode-integration.md) | Sections 13-14: VSCode Integration and Error UX Tests |
| [08-mcp-cache.md](08-mcp-cache.md) | Sections 15-16: MCP Server and Cache Tests |
| [09-navigation-quickpick.md](09-navigation-quickpick.md) | Sections 17-18: Failure Handler Navigation and QuickPick Tests |
| [10-lexer-parser.md](10-lexer-parser.md) | Sections 20-21: Lexer Token Boundary and Parser Error Recovery Tests |
| [11-local-preprocessor-macro.md](11-local-preprocessor-macro.md) | Sections 22-24: LocalTypeInfo, Preprocessor Directive, Standalone Macro Tests |
| [12-system-e2e.md](12-system-e2e.md) | Section 19: System-Level End-to-End Tests |
| [13-vtable-trace-view.md](13-vtable-trace-view.md) | Section 25: Vtable Resolution Trace View |
| [14-file-system-events.md](14-file-system-events.md) | Section 26: File System Event Handling Tests |

---

## Implementation Notes for Jest Developer

1. **Test doubles:** Use a plain `NavigationIndex` object pre-populated with the "Index state" from each
   TC-RES test. Inject it into the `VtableResolver` constructor. No file system access needed for
   resolution tests.

2. **LocalTypeInfo injection:** For chain-walk tests (TC-P9-101 through TC-P9-603), pre-populate
   `LocalTypeInfo` with the variable/parameter type entries described in each test case's "Index state"
   section. Inject it alongside the `NavigationIndex`. Do not read actual files in unit tests.

3. **TC-P11-003 (Allman braces):** The Chevrotain grammar handles Allman brace style natively
   (whitespace-insensitive). Write the test expecting a MATCH. No special workaround is needed.

4. **TC-P10-002 (macro form):** The `JunoFailureHandler` alternation token
   (`/JUNO_FAILURE_HANDLER\b|_pfcnFailureHandler\b/`) closes the previously identified gap.
   Write the test expecting a MATCH. No separate "gap reminder" test is needed.

5. **Column numbers:** Jest tests should compute column numbers using `line.indexOf(fieldName)` on
   the test input string rather than hardcoding, to avoid brittleness if test strings are reformatted.

6. **Chain-walk category coverage:** For TC-P9-303, write an explicit test asserting that the
   chain-walk resolves `ptApi` as a direct API pointer (Category 3) rather than as a member access
   chain (Category 1). This prevents a regression where both resolution paths could claim the same
   call site.

---

## Summary Table (Sections 1-12)

| Test Case ID | Visitor / Section | Scenario | Category | Requirement |
|-------------|---------|----------|----------|-------------|
| TC-P1-001 | visitStructDefinition | JUNO_LOG_ROOT_TAG, JUNO_MODULE_EMPTY | — | REQ-VSCODE-008 |
| TC-P1-002 | visitStructDefinition | JUNO_DS_HEAP_ROOT_TAG, extra members | — | REQ-VSCODE-008 |
| TC-P1-003 | visitStructDefinition | JUNO_SB_BROKER_ROOT_TAG | — | REQ-VSCODE-008 |
| TC-P1-004 | visitStructDefinition | Negative: JUNO_MODULE_DERIVE | — | REQ-VSCODE-008 |
| TC-P2-001 | visitStructDefinition | ENGINE_APP_TAG derives JUNO_APP_ROOT_T | — | REQ-VSCODE-009 |
| TC-P2-002 | visitStructDefinition | JUNO_SB_PIPE_TAG derives JUNO_DS_QUEUE_ROOT_T | — | REQ-VSCODE-009 |
| TC-P2-003 | visitStructDefinition | Negative: JUNO_MODULE_ROOT | — | REQ-VSCODE-009 |
| TC-P3-001 | visitStructDefinition | JUNO_POINTER_TAG, JUNO_TRAIT_ROOT | — | REQ-VSCODE-014 |
| TC-P3-002 | visitStructDefinition | Negative: JUNO_MODULE_ROOT | — | REQ-VSCODE-014 |
| TC-P3-003 | visitStructDefinition | Negative: JUNO_MODULE_DERIVE | — | REQ-VSCODE-014 |
| TC-P4-001 | visitStructDefinition | MY_POINTER_IMPL_TAG, JUNO_TRAIT_DERIVE (synthetic) | — | REQ-VSCODE-015 |
| TC-P4-002 | visitStructDefinition | Negative: JUNO_TRAIT_ROOT (synthetic) | — | REQ-VSCODE-015 |
| TC-P5-001 | visitStructDefinition | JUNO_DS_HEAP_API_T: [Insert, Heapify, Pop] | — | REQ-VSCODE-012 |
| TC-P5-002 | visitStructDefinition | JUNO_LOG_API_T: [LogDebug, LogInfo, LogWarning, LogError] | — | REQ-VSCODE-012 |
| TC-P5-003 | visitStructDefinition | JUNO_DS_HEAP_POINTER_API_T: [Compare, Swap] | — | REQ-VSCODE-012 |
| TC-P5-004 | visitStructDefinition | JUNO_POINTER_API_T: [Copy, Reset] | — | REQ-VSCODE-012 |
| TC-P5-005 | visitStructDefinition | JUNO_SB_BROKER_API_T: [Publish, RegisterSubscriber] | — | REQ-VSCODE-012 |
| TC-P6-001 | visitVtableDeclaration | Designated: tEngineAppApi (OnStart, OnProcess, OnExit) | — | REQ-VSCODE-010 |
| TC-P6-002 | visitVtableDeclaration | Negative: positional initializer → zero designated matches | — | REQ-VSCODE-010 |
| TC-P6-003 | visitVtableDeclaration | Negative: non-API-T type variable | — | REQ-VSCODE-010 |
| TC-P7-001 | visitVtableDeclaration | Direct assignment tHeapApi.Insert (synthetic) | — | REQ-VSCODE-011 |
| TC-P7-002 | visitVtableDeclaration | Disambiguation vs P6 (synthetic) | — | REQ-VSCODE-011 |
| TC-P8-001 | visitVtableDeclaration | Positional: JUNO_DS_HEAP_API_T, juno_heap.c | — | REQ-VSCODE-012 |
| TC-P8-002 | visitVtableDeclaration | Positional: JUNO_SB_BROKER_API_T, static functions | — | REQ-VSCODE-012 |
| TC-P8-003 | visitVtableDeclaration | Positional: JUNO_LOG_API_T, main.c | — | REQ-VSCODE-012 |
| TC-P8-004 | visitVtableDeclaration | Negative: designated initializer → no fallthrough to P8 | — | REQ-VSCODE-012 |
| TC-P9-001 | Chain-walk | Now, cursor confirmed | — | REQ-VSCODE-003 |
| TC-P9-002 | Chain-walk | LogInfo via indirect variable | — | REQ-VSCODE-003 |
| TC-P9-003 | Chain-walk | Negative: plain member access, no `(` | — | REQ-VSCODE-003 |
| TC-P9-004 | Chain-walk | Negative: cursor on whitespace | — | REQ-VSCODE-003 |
| TC-P9-005 | Chain-walk | Negative: free function call | — | REQ-VSCODE-003 |
| TC-P9-101 | Chain-walk | Simple pointer: ptTime->ptApi->Now | Cat. 1 | REQ-VSCODE-003 |
| TC-P9-102 | Chain-walk | Simple pointer: ptBroker->ptApi->Publish | Cat. 1 | REQ-VSCODE-003 |
| TC-P9-103 | Chain-walk | Array subscript: ptAppList[i]->ptApi->OnStart | Cat. 1 | REQ-VSCODE-003 |
| TC-P9-104 | Chain-walk | Array subscript (var index): ptAppList[iCounter]->ptApi->OnProcess | Cat. 1 | REQ-VSCODE-003 |
| TC-P9-105 | Chain-walk | Chained member: ptEngineApp->ptBroker->ptApi->RegisterSubscriber | Cat. 1 | REQ-VSCODE-003 |
| TC-P9-201 | Chain-walk | Dot-accessed: tReturn.ptApi->Copy | Cat. 1 | REQ-VSCODE-003 |
| TC-P9-202 | Chain-walk | Nested dot: tPtrResult.tOk.ptApi->Copy | Cat. 1 | REQ-VSCODE-003 |
| TC-P9-203 | Chain-walk | Dot-accessed: tItem.ptApi->Copy | Cat. 1 | REQ-VSCODE-003 |
| TC-P9-301 | Chain-walk | Indirect: ptLoggerApi->LogInfo | Cat. 3 | REQ-VSCODE-003 |
| TC-P9-302 | Chain-walk | Indirect: ptCmdPipeApi->Dequeue | Cat. 3 | REQ-VSCODE-003 |
| TC-P9-303 | Chain-walk | Indirect: ptApi->GetAt (generic name) | Cat. 3 | REQ-VSCODE-003 |
| TC-P9-401 | Chain-walk | Non-ptApi member: ptHeapPointerApi->Compare | Cat. 4 | REQ-VSCODE-003 |
| TC-P9-402 | Chain-walk | Non-ptApi member: ptHeapPointerApi->Swap | Cat. 4 | REQ-VSCODE-003 |
| TC-P9-501 | Chain-walk | Macro: JUNO_MODULE_GET_API()->Insert (synthetic) | Cat. 5 | REQ-VSCODE-003 |
| TC-P9-601 | Chain-walk | Fallback: unknown receiver, unique field | Cat. 6 | REQ-VSCODE-003 |
| TC-P9-602 | Chain-walk | Fallback: unknown receiver, shared field name | Cat. 6 | REQ-VSCODE-003,006 |
| TC-P9-603 | Chain-walk | Fallback: unknown field → not found | Cat. 6 | REQ-VSCODE-003,004 |
| TC-P10-001 | visitFailureHandlerAssignment | Direct: ptHeap->_pfcnFailureHandler | — | REQ-VSCODE-016 |
| TC-P10-002 | visitFailureHandlerAssignment | Macro form: JUNO_FAILURE_HANDLER (alternation token) | — | REQ-VSCODE-016 |
| TC-P10-003 | visitFailureHandlerAssignment | Negative: ptApi assignment | — | REQ-VSCODE-016 |
| TC-P11-001 | visitFunctionDefinition | Match: static LogInfo (varied return type) | — | REQ-VSCODE-003 |
| TC-P11-002 | visitFunctionDefinition | Match: non-static JunoDs_Heap_Insert | — | REQ-VSCODE-003 |
| TC-P11-003 | visitFunctionDefinition | Allman braces: static inline Verify (grammar handles natively) | — | REQ-VSCODE-003 |
| TC-P11-004 | visitFunctionDefinition | Match: void FailureHandler | — | REQ-VSCODE-003 |
| TC-P11-005 | visitFunctionDefinition | Match: static result-type Now | — | REQ-VSCODE-003 |
| TC-P11-006 | visitFunctionDefinition | No match: forward declaration (`;`) | — | REQ-VSCODE-003 |
| TC-P11-007 | visitFunctionDefinition | No match: header prototype (`;`) | — | REQ-VSCODE-003 |
| TC-RES-001 | E2E Resolution | LogInfo via indirect API pointer (Cat. 3) | Cat. 3 | REQ-VSCODE-005 |
| TC-RES-002 | E2E Resolution | OnStart via array subscript (Cat. 1 + QuickPick) | Cat. 1 | REQ-VSCODE-006 |
| TC-RES-003 | E2E Resolution | RegisterSubscriber via chained member (Cat. 1) | Cat. 1 | REQ-VSCODE-005 |
| TC-RES-004 | E2E Resolution | Compare via non-ptApi member (Cat. 4) | Cat. 4 | REQ-VSCODE-005 |
| TC-RES-005 | E2E Resolution | Copy via dot-accessed ptApi (Cat. 2) | Cat. 1 | REQ-VSCODE-005 |
| TC-RES-006 | E2E Resolution | No implementation found — error path | Cat. 6 | REQ-VSCODE-004,013 |
| TC-RES-007 | E2E Resolution | Multiple implementations — QuickPick | Cat. 1 | REQ-VSCODE-006 |

---

## Summary Table (Sections 13-18)

| Test Case ID | Section | Scenario | Requirement(s) |
|-------------|---------|----------|----------------|
| TC-VSC-001 | VSCode | Extension activates on C file open | REQ-VSCODE-001 |
| TC-VSC-002 | VSCode | DefinitionProvider registered for `c` and `cpp` | REQ-VSCODE-001, REQ-VSCODE-007 |
| TC-VSC-003 | VSCode | F12 on vtable call site returns location | REQ-VSCODE-002, REQ-VSCODE-007 |
| TC-VSC-004 | VSCode | F12 on non-vtable call returns undefined (fallthrough) | REQ-VSCODE-007 |
| TC-VSC-005 | VSCode | `libjuno.goToImplementation` command registered | REQ-VSCODE-001 |
| TC-VSC-006 | VSCode | `libjuno.reindexWorkspace` clears cache and rebuilds | REQ-VSCODE-001 |
| TC-VSC-007 | VSCode | `provideDefinition` returns undefined without throw on unresolvable site | REQ-VSCODE-002, REQ-VSCODE-004 |
| TC-ERR-001 | Error UX | Status bar shown on failure (non-intrusive) | REQ-VSCODE-004, REQ-VSCODE-013 |
| TC-ERR-002 | Error UX | Status bar auto-clears after 5 seconds | REQ-VSCODE-013 |
| TC-ERR-003 | Error UX | Information message on repeated failure within 10 s | REQ-VSCODE-013 |
| TC-ERR-004 | Error UX | No modal dialog on resolution failure | REQ-VSCODE-013 |
| TC-ERR-005 | Error UX | Error message includes specific API type and field name | REQ-VSCODE-004, REQ-VSCODE-013 |
| TC-ERR-006 | Error UX | MCP returns proper error object, not HTTP error code | REQ-VSCODE-004 |
| TC-MCP-001 | MCP | Server starts on extension activation | REQ-VSCODE-017 |
| TC-MCP-002 | MCP | `resolve_vtable_call` registered with correct schema | REQ-VSCODE-018 |
| TC-MCP-003 | MCP | `resolve_failure_handler` registered with correct schema | REQ-VSCODE-019 |
| TC-MCP-004 | MCP | `resolve_vtable_call` valid input → found: true + locations | REQ-VSCODE-018 |
| TC-MCP-005 | MCP | `resolve_vtable_call` no-match → found: false + error | REQ-VSCODE-018 |
| TC-MCP-006 | MCP | `resolve_failure_handler` valid input → found: true | REQ-VSCODE-019 |
| TC-MCP-007 | MCP | `resolve_failure_handler` no handler → found: false | REQ-VSCODE-019 |
| TC-MCP-008 | MCP | `.libjuno/mcp.json` written on activation | REQ-VSCODE-017 |
| TC-MCP-009 | MCP | Server binds to 127.0.0.1 only | REQ-VSCODE-017, REQ-VSCODE-020 |
| TC-MCP-010 | MCP | Error responses use `isError: true`, not HTTP error codes | REQ-VSCODE-017 |
| TC-MCP-011 | MCP | MCP tools work headless (no VSCode UI) | REQ-VSCODE-017, REQ-VSCODE-020 |
| TC-MCP-012 | MCP | No platform-specific AI SDK imports in MCP source | REQ-VSCODE-020 |
| TC-CACHE-001 | Cache | Cache file created on first index | REQ-VSCODE-001 |
| TC-CACHE-002 | Cache | Cache loaded on activation when valid | REQ-VSCODE-001 |
| TC-CACHE-003 | Cache | Stale file triggers re-index of that file only | REQ-VSCODE-001 |
| TC-CACHE-004 | Cache | New file indexed and added to cache | REQ-VSCODE-001 |
| TC-CACHE-005 | Cache | Deleted file removed from cache and index | REQ-VSCODE-001 |
| TC-CACHE-006 | Cache | Version mismatch triggers full re-index | REQ-VSCODE-001 |
| TC-CACHE-007 | Cache | FileSystemWatcher triggers re-index on file change | REQ-VSCODE-001 |
| TC-CACHE-008 | Cache | Debounced write: rapid saves → single cache write | REQ-VSCODE-001 |
| TC-CACHE-009 | Cache | Cache write is atomic (temp file + rename) | REQ-VSCODE-001 |
| TC-FH-001 | Failure Handler | _pfcnFailureHandler assignment resolves to handler | REQ-VSCODE-016 |
| TC-FH-002 | Failure Handler | JUNO_FAILURE_HANDLER macro form resolves (gap fix) | REQ-VSCODE-016 |
| TC-FH-003 | Failure Handler | Multiple assignments → QuickPick shown | REQ-VSCODE-016 |
| TC-FH-004 | Failure Handler | No assignment → error shown | REQ-VSCODE-016 |
| TC-FAIL-001 | FAIL Macro Nav | JUNO_FAIL: bare handler name found in functionDefinitions | REQ-VSCODE-023 |
| TC-FAIL-002 | FAIL Macro Nav | JUNO_FAIL: unknown handler name not in functionDefinitions | REQ-VSCODE-023 |
| TC-FAIL-003 | FAIL Macro Nav | JUNO_FAIL_MODULE: derived type walks chain → handler found | REQ-VSCODE-024 |
| TC-FAIL-004 | FAIL Macro Nav | JUNO_FAIL_MODULE: no handler registered for resolved root type | REQ-VSCODE-024 |
| TC-FAIL-005 | FAIL Macro Nav | JUNO_FAIL_ROOT: root pointer directly → no chain walk → found | REQ-VSCODE-025 |
| TC-FAIL-006 | FAIL Macro Nav | JUNO_FAIL_ROOT: no handler registered for root type | REQ-VSCODE-025 |
| TC-FAIL-007 | FAIL Macro Nav | JUNO_ASSERT_EXISTS_MODULE: derived → walks chain → found | REQ-VSCODE-026 |
| TC-FAIL-008 | FAIL Macro Nav | JUNO_ASSERT_EXISTS_MODULE: no handler registered | REQ-VSCODE-026 |
| TC-FAIL-009 | FAIL Macro Nav | Non-macro line falls through to §5.3 algorithm | REQ-VSCODE-022 |
| TC-FAIL-010 | FAIL Macro Nav | JUNO_FAIL: nested parentheses in arg[1] — gracefully unresolvable | REQ-VSCODE-023 |
| TC-FAIL-011 | FAIL Macro Nav | JUNO_FAIL_MODULE: cast expression in arg[1] stripped to bare identifier | REQ-VSCODE-024 |
| TC-FAIL-012 | FAIL Macro Nav | JUNO_FAIL_MODULE: two-level derivation chain walks to root type | REQ-VSCODE-024 |
| TC-QP-001 | QuickPick | Items show function name as label | REQ-VSCODE-006 |
| TC-QP-002 | QuickPick | Items show file:line as description | REQ-VSCODE-006 |
| TC-QP-003 | QuickPick | Items show workspace-relative path as detail | REQ-VSCODE-006 |
| TC-QP-004 | QuickPick | Selecting item navigates to correct file and line | REQ-VSCODE-006 |
| TC-QP-005 | QuickPick | Cancelling QuickPick does not navigate | REQ-VSCODE-006 |
| TC-SYS-001 | System E2E | LogInfo via indirect API pointer — real file, chain-walk Cat. 3 | REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005 |
| TC-SYS-002 | System E2E | RegisterSubscriber via chained member access — real file, chain-walk Cat. 1 | REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005 |
| TC-SYS-003 | System E2E | Now via standard ptApi access — real file, chain-walk Cat. 1 | REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005 |
| TC-SYS-004 | System E2E | Publish via standard ptApi access — real file, chain-walk Cat. 1 | REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005 |
| TC-SYS-005 | System E2E | OnStart via array subscript — multiple locations, real file | REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-006 |
| TC-SYS-006 | System E2E | SubtractTime via ptApi — non-static library function, real file | REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005 |
| TC-SYS-007 | System E2E | Dequeue via dot-chained ptApi — positional vtable, real file | REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005, REQ-VSCODE-012 |
| TC-SYS-008 | System E2E | JUNO_FAILURE_HANDLER macro form — navigates to FailureHandler in main.c | REQ-VSCODE-016 |
| TC-SYS-009 | System E2E | OnProcess via array subscript with variable index — multiple locations, real file | REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-006 |
| TC-SYS-010 | System E2E | LogInfo cross-file resolution from system_manager_app.c — real file | REQ-VSCODE-002, REQ-VSCODE-003, REQ-VSCODE-005 |
| TC-SYS-011 | System E2E | Extension activation and index build with real example project | REQ-VSCODE-001 |
| TC-SYS-012 | System E2E | Non-vtable call passes through to default C language provider | REQ-VSCODE-007 |
| TC-SYS-013 | System E2E | Designated initializer indexing — engine_app.c vtable, all 3 fields via MCP | REQ-VSCODE-010 |
| TC-SYS-014 | System E2E | Positional initializer indexing — main.c log API, all 4 fields via MCP | REQ-VSCODE-012 |

---

## Summary Table (Section 25)

| Test Case ID | Section | Scenario | Requirement(s) |
|-------------|---------|----------|----------------|
| TC-TRACE-001 | Vtable Trace View | Single impl: 3-node trace (call-site, composition-root, implementation) | REQ-VSCODE-030, REQ-VSCODE-031, REQ-VSCODE-032 |
| TC-TRACE-002 | Vtable Trace View | Multi-impl: 2 subtrees with shared call-site node | REQ-VSCODE-027 |
| TC-TRACE-003 | Vtable Trace View | Resolver found:false → StatusBarHelper.showError(); no WebviewPanel | REQ-VSCODE-027 |
| TC-TRACE-004 | Vtable Trace View | Call-site node: label from lineText; file = currentFile; line = cursorLine | REQ-VSCODE-030 |
| TC-TRACE-005 | Vtable Trace View | Composition-root node: file = assignmentFile; line = assignmentLine | REQ-VSCODE-031 |
| TC-TRACE-006 | Vtable Trace View | Implementation node: label = functionName; detail = FunctionDefinitionRecord.signature | REQ-VSCODE-032 |
| TC-TRACE-007 | Vtable Trace View | HTML escaping: file path with `<script>` tag → HTML-escaped in panel (XSS prevention) | REQ-VSCODE-027 |
| TC-TRACE-008 | Vtable Trace View | CSP nonce: same nonce in Content-Security-Policy meta tag and script tag | REQ-VSCODE-027 |
| TC-TRACE-009 | Vtable Trace View | File link click: postMessage {type:'navigate', file, line} → showTextDocument | REQ-VSCODE-027 |
| TC-TRACE-010 | Vtable Trace View | Command `libjuno.showVtableTrace` registered during activation | REQ-VSCODE-028, REQ-VSCODE-029 |
| TC-TRACE-011 | Vtable Trace View | Keybinding: package.json has Ctrl+Shift+T → libjuno.showVtableTrace with when clause | REQ-VSCODE-028 |
| TC-TRACE-012 | Vtable Trace View | Context menu: package.json has editor/context entry for libjuno.showVtableTrace | REQ-VSCODE-029 |
| TC-TRACE-013 | Vtable Trace View | WebviewPanel options: enableScripts:true and retainContextWhenHidden:true | REQ-VSCODE-027 |
| TC-TRACE-014 | Vtable Trace View | No external resources: HTML contains no http:// or https:// URLs | REQ-VSCODE-027 |
| TC-TRACE-015 | Vtable Trace View | Empty locations (found:false) → error shown, no panel; JUNO_MODULE_SUPER edge case | REQ-VSCODE-027 |

---

## Summary Table (Section 26)

| Test Case ID | Section | Scenario | Requirement(s) |
|-------------|---------|----------|----------------|
| TC-FSE-001 | File System Events | `onDidCreate` watcher registered on activation | REQ-VSCODE-042 |
| TC-FSE-002 | File System Events | New `.c` file created — vtable assignment indexed | REQ-VSCODE-042 |
| TC-FSE-003 | File System Events | New `.h` file created — module root record indexed | REQ-VSCODE-042 |
| TC-FSE-004 | File System Events | Negative: non-C file created — glob excludes non-C extensions | REQ-VSCODE-042 |
| TC-FSE-005 | File System Events | `onDidDelete` watcher registered on activation | REQ-VSCODE-043 |
| TC-FSE-006 | File System Events | Existing file deleted — all index records for that file removed | REQ-VSCODE-043 |
| TC-FSE-007 | File System Events | Deleted file's hash removed from cache and save scheduled | REQ-VSCODE-043 |
| TC-FSE-008 | File System Events | Delete sole implementation file — resolver returns `found: false` | REQ-VSCODE-042, REQ-VSCODE-043 |
| TC-FSE-009 | File System Events | Boundary: file created then immediately deleted (`readFile` throws ENOENT) — no crash, no stale entry | REQ-VSCODE-042 |

---

## Requirements Coverage Matrix

| Requirement ID | Title | Test Case(s) |
|----------------|-------|--------------|
| REQ-VSCODE-001 | VSCode Extension | TC-VSC-001, TC-VSC-002, TC-VSC-005, TC-VSC-006, TC-CACHE-001 through TC-CACHE-009, TC-SYS-011 |
| REQ-VSCODE-002 | Vtable-Aware Go to Definition | TC-VSC-003, TC-VSC-007, TC-SYS-001 through TC-SYS-007, TC-SYS-009, TC-SYS-010, TC-SYS-012 |
| REQ-VSCODE-003 | LibJuno API Pattern Recognition | TC-P9-001 through TC-P9-603, TC-P11-001 through TC-P11-007, TC-RES-001 through TC-RES-007, TC-SYS-001 through TC-SYS-007, TC-SYS-009, TC-SYS-010, TC-SYS-013, TC-SYS-014, TC-LEX-001 through TC-LEX-010, TC-ERR-PARSE-001 through TC-ERR-PARSE-005, TC-LOCAL-001 through TC-LOCAL-007, TC-PP-001 through TC-PP-004, TC-DECL-001 through TC-DECL-005 |
| REQ-VSCODE-004 | Graceful Error on Missing Implementation | TC-P9-603, TC-RES-006, TC-VSC-007, TC-ERR-001, TC-ERR-005, TC-ERR-006 |
| REQ-VSCODE-005 | Single Implementation Navigation | TC-RES-001, TC-RES-003, TC-RES-004, TC-RES-005, TC-SYS-001, TC-SYS-002, TC-SYS-003, TC-SYS-004, TC-SYS-006, TC-SYS-007, TC-SYS-010 |
| REQ-VSCODE-006 | Multiple Implementation Selection | TC-P9-602, TC-RES-002, TC-RES-007, TC-QP-001 through TC-QP-005, TC-SYS-005, TC-SYS-009 |
| REQ-VSCODE-007 | Native Go to Definition Integration | TC-VSC-002, TC-VSC-003, TC-VSC-004, TC-SYS-012 |
| REQ-VSCODE-008 | Module Root API Discovery | TC-P1-001 through TC-P1-004 |
| REQ-VSCODE-009 | Module Derivation Chain Resolution | TC-P2-001 through TC-P2-003, TC-LEX-007 |
| REQ-VSCODE-010 | Designated Initializer Recognition | TC-P6-001 through TC-P6-003, TC-SYS-013 |
| REQ-VSCODE-011 | Direct Assignment Recognition | TC-P7-001, TC-P7-002 |
| REQ-VSCODE-012 | Positional Initializer Recognition | TC-P5-001 through TC-P5-005, TC-P8-001 through TC-P8-004, TC-SYS-007, TC-SYS-014 |
| REQ-VSCODE-013 | Informative Non-Intrusive Error | TC-RES-006, TC-ERR-001 through TC-ERR-005 |
| REQ-VSCODE-014 | Trait Root API Discovery | TC-P3-001 through TC-P3-003 |
| REQ-VSCODE-015 | Trait Derivation Chain Resolution | TC-P4-001, TC-P4-002 |
| REQ-VSCODE-016 | Failure Handler Navigation | TC-P10-001 through TC-P10-003, TC-FH-001 through TC-FH-004, TC-SYS-008, TC-LEX-004, TC-LEX-005, TC-LEX-006, TC-LEX-008 |
| REQ-VSCODE-017 | AI Agent Accessibility | TC-MCP-001, TC-MCP-008, TC-MCP-009, TC-MCP-010, TC-MCP-011 |
| REQ-VSCODE-018 | AI Vtable Resolution Access | TC-MCP-002, TC-MCP-004, TC-MCP-005 |
| REQ-VSCODE-019 | AI Failure Handler Resolution Access | TC-MCP-003, TC-MCP-006, TC-MCP-007 |
| REQ-VSCODE-020 | Platform-Agnostic AI Interface | TC-MCP-009, TC-MCP-011, TC-MCP-012 |
| REQ-VSCODE-022 | FAIL Macro Failure Handler Navigation | TC-FAIL-001 through TC-FAIL-012 |
| REQ-VSCODE-023 | JUNO_FAIL Direct Handler Resolution | TC-FAIL-001, TC-FAIL-002, TC-FAIL-010 |
| REQ-VSCODE-024 | JUNO_FAIL_MODULE Handler Resolution | TC-FAIL-003, TC-FAIL-004, TC-FAIL-011, TC-FAIL-012 |
| REQ-VSCODE-025 | JUNO_FAIL_ROOT Handler Resolution | TC-FAIL-005, TC-FAIL-006 |
| REQ-VSCODE-026 | JUNO_ASSERT_EXISTS_MODULE Handler Resolution | TC-FAIL-007, TC-FAIL-008 |
| REQ-VSCODE-027 | Vtable Resolution Trace View | TC-TRACE-002, TC-TRACE-003, TC-TRACE-007, TC-TRACE-008, TC-TRACE-009, TC-TRACE-013, TC-TRACE-014, TC-TRACE-015 |
| REQ-VSCODE-028 | Trace View Activation via Keyboard | TC-TRACE-010, TC-TRACE-011 |
| REQ-VSCODE-029 | Trace View Activation via Command Palette | TC-TRACE-010, TC-TRACE-012 |
| REQ-VSCODE-030 | Trace View Call Site Node | TC-TRACE-001, TC-TRACE-004 |
| REQ-VSCODE-031 | Trace View Composition Root Node | TC-TRACE-001, TC-TRACE-005 |
| REQ-VSCODE-032 | Trace View Implementation Node | TC-TRACE-001, TC-TRACE-006 |
| REQ-VSCODE-042 | Automatic Indexing of Newly Created Files | TC-FSE-001, TC-FSE-002, TC-FSE-003, TC-FSE-004, TC-FSE-008, TC-FSE-009 |
| REQ-VSCODE-043 | Automatic Index Cleanup on File Deletion | TC-FSE-005, TC-FSE-006, TC-FSE-007, TC-FSE-008 |

---
