> Part of: [Software Development Plan](index.md) — Sections 9-10

## 9. Appendix: File Structure (Target State)

```
vscode-extension/
├── src/
│   ├── parser/
│   │   ├── lexer.ts
│   │   ├── parser.ts
│   │   ├── visitor.ts
│   │   ├── types.ts
│   │   └── __tests__/
│   │       ├── lexer.test.ts                       (Phase 1 ✅)
│   │       ├── visitor-structs.test.ts              (Phase 1 ✅)
│   │       ├── visitor-vtable.test.ts               (Phases 1–2)
│   │       ├── visitor-functions.test.ts            (Phases 1–2)
│   │       ├── visitor-localtypeinfo.test.ts        (Phase 3 — NEW)
│   │       └── visitor-callsites.test.ts            (Phase 4 — NEW)
│   ├── indexer/
│   │   ├── navigationIndex.ts
│   │   ├── workspaceIndexer.ts
│   │   └── __tests__/
│   │       ├── navigationIndex.test.ts              (Phase 5 — NEW)
│   │       └── workspaceIndexer.test.ts             (Phases 11–12 — NEW)
│   ├── resolver/
│   │   ├── vtableResolver.ts
│   │   ├── failureHandlerResolver.ts
│   │   ├── resolverUtils.ts
│   │   └── __tests__/
│   │       ├── resolverUtils.test.ts                (Phase 6 — NEW)
│   │       ├── vtableResolver.test.ts               (Phase 7 — NEW)
│   │       └── failureHandlerResolver.test.ts       (Phase 8 — NEW)
│   ├── cache/
│   │   ├── cacheManager.ts
│   │   └── __tests__/
│   │       └── cacheManager.test.ts                (Phase 10 — NEW)
│   ├── providers/
│   │   ├── junoDefinitionProvider.ts
│   │   ├── quickPickHelper.ts
│   │   ├── statusBarHelper.ts
│   │   └── __tests__/
│   │       ├── junoDefinitionProvider.test.ts       (Phase 14 — NEW)
│   │       ├── quickPickHelper.test.ts              (Phase 15 — NEW)
│   │       └── statusBarHelper.test.ts             (Phase 15 — NEW)
│   ├── mcp/
│   │   ├── mcpServer.ts
│   │   └── __tests__/
│   │       └── mcpServer.test.ts                   (Phase 13 — NEW)
│   ├── __tests__/
│   │   ├── integration.test.ts                     (Phase 9 — NEW)
│   │   └── sprint17-regression.test.ts             (Phase 18 — NEW)
│   ├── __mocks__/
│   │   └── vscode.ts                               (Phase 14 — NEW)
│   └── extension.ts
├── design/
│   ├── design.md
│   └── test-cases.md
├── software-development-plan.md                    (this file)
├── jest.config.js
├── package.json
└── tsconfig.json
```

---

## 10. Review Findings Log

This section records all findings from the three review engineers (Systems Engineer, Quality Engineer, Senior Engineer) that reviewed version 1.0 of this plan. Each finding is listed with its severity, summary, and disposition.

### Review Findings Summary

| ID | Severity | Finding | Disposition | Incorporated In |
|----|---------|---------|------------|----------------|
| C1 | Critical | `apiCallSites` is dead-code data — populated by visitor but never consumed by indexer or resolver | Incorporated | Section 1.3 (design question), Phase 3 (new), Phase 4 (TC-P9 reduced to sample) |
| M1 | Major | Design §5.1 describes CST-based chain-walk; actual code uses 3 regex strategies (`macroRe`, `arrayRe`, `generalRe`) | Incorporated | Phase 7 scope rewritten; TC-RES-008 through TC-RES-011 added |
| M2 | Major | No cross-seam integration test between visitor output, indexer mergeInto, and resolver | Incorporated | Phase 9 added (TC-INT-001, TC-INT-002) |
| M3 | Major | `McpServer.start()` returns void; bound port not observable for port-0 tests | Incorporated | Phase 13 WI-13.0 source change listed as prerequisite |
| M4 | Major | `removeFileRecords` leaves stale entries in 5 flat maps; TC-CACHE-005 acceptance criteria incomplete | Incorporated | Phase 5 TC-IDX-005; Phase 10 TC-CACHE-005 updated with stale-map documentation |
| M5 | Major | REQ-VSCODE-021 has zero test cases | Incorporated | Phase 12 TC-FILE-001; Section 7 traceability table updated |
| M6 | Major | WorkspaceIndexer, NavigationIndex CRUD, and resolverUtils have no TC-\* IDs | Incorporated | Phases 5, 6, 11–12: TC-IDX-001–005, TC-UTIL-001–005, TC-WI-001–008 |
| M7 | Major | Phase 2a debug budget too low at 20%; Sprint 1 actuals were 40% | Incorporated | Phase 2 debug budget revised to 30% |
| M8 | Major | Cache corruption with matching version not tested | Incorporated | Phase 10 TC-CACHE-010 |
| m1 | Minor | `FailureHandlerResolver` has no column guard — any cursor triggers resolution | Incorporated | Phase 8 TC-FH-005a (column 0, LHS) and TC-FH-005b (RHS) |
| m2 | Minor | `junoDefinitionProvider.ts` hard-codes strings matching resolver error messages; drift risk | Incorporated | Phase 14 TC-VSC-008 |
| m3 | Minor | TC-CACHE-009 atomicity — concurrent write scenario undocumented | Incorporated | Phase 10 TC-CACHE-009: documented as single-caller only; concurrency handled by debouncing |
| m4 | Minor | TC-P9-201/202/203 and TC-RES-005 mislabeled in test-cases.md | Deferred | Section 7 Documentation Errata note |
| m5 | Minor | No Jest coverage threshold defined | Incorporated | Section 8 DoD; Phase 16 WI-16.4 (≥80% line coverage gate) |
| m6 | Minor | DoD "no dynamic memory allocation" inapplicable to TypeScript | Incorporated | Section 8 DoD rewritten with TypeScript-relevant constraints |
| m7 | Minor | No MCP stop/port release test | Incorporated | Phase 13 TC-MCP-013 |
| m8 | Minor | `resolveFailureHandlerRootType()` multi-module heuristic untested | Incorporated | Phase 12 TC-WI-008 |
| O1 | Observation | `visitor-vtable.test.ts` already has TC-P6-001/002; Phase 1 scope should include them | Incorporated | Section 2.2 test table and Phase 1 summary updated |
| O2 | Observation | Six requirements under-classified as "Demonstration" instead of "Test" | Deferred | Section 7 note; requirements.json update deferred to maintenance pass |
| O3 | Observation | MCP `file` parameter path not validated against workspace root | Incorporated | Phase 13 TC-MCP-014 |
| O4 | Observation | Failure handler silent fallthrough behavior undocumented | Incorporated | Phase 8 TC-FH-006 |

### Deferred Findings

| ID | Finding | Reason Deferred | Owner |
|----|---------|----------------|-------|
| O2 | requirements.json verification_method fields for REQ-VSCODE-006/-007/-016/-018/-019 should be "Test" | Low-priority documentation fix; no impact on test execution. Deferred to next requirements maintenance pass. | PM |
| m4 | TC-P9-201/202/203 and TC-RES-005 labels in test-cases.md incorrect | Cosmetic fix; no test execution impact. Deferred to next test-cases.md maintenance pass. | PM |

### Rejected Findings

None. All findings were either incorporated or deferred with documented rationale.

### Version 2.0 Verification Pass

The following additional findings from the Systems Engineer's v2.0 review were incorporated:

| ID | Severity | Finding | Disposition |
|----|---------|---------|------------|
| V2-1 | Error | `FailureHandlerResolver` pipeline not included in Phase 9 integration tests | Incorporated: TC-INT-003 added to Phase 9 |
| V2-2 | Error | TC-CACHE-007 (FSW → re-index) tests WorkspaceIndexer behavior, not CacheManager | Incorporated: moved to Phase 11 as TC-WI-004a |
| V2-3 | Warning | `parseIntermediates()` has no dedicated TC in Phase 6 | Incorporated: TC-UTIL-006 added to Phase 6 |
| V2-4 | Warning | Phase 14 has false prerequisite on Phase 13 (architecturally independent) | Incorporated: Phase 14 prerequisite changed to Phases 7+8; can parallel Phase 13 |
