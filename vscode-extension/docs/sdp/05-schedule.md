> Part of: [Software Development Plan](index.md) — Section 5

## 5. Sprint Schedule

### Sprint Cadence

Each sprint represents one orchestration cycle: plan → delegate → verify → fix → gate. Sprint length is adaptive based on work item complexity (not fixed calendar time).

### Schedule

| Sprint | Phase(s) | Focus | Key Deliverables | Debug Budget |
|--------|---------|-------|-----------------|-------------|
| 1 | Phase 1 ✅ | Parser Foundation | 4 test files, 98 tests, 3 bug fixes | 40% (actual) |
| 2 | Phase 2 ✅ | Visitor: Vtable Init Patterns | TC-P6/P7/P8 tests; TC-P10/P11 gap fill | 30% |
| 3 | Phases 3+5 ✅ | Visitor: Local Type Info + Navigation Index CRUD | TC-LTI-001–005; TC-IDX-001–005 | 35% |
| 4 | Phase 6 | Resolver Utilities | TC-UTIL-001–006 | 20% |
| 5 | Phase 7 | VtableResolver | TC-RES-001–011; regex boundary tests | 25% |
| 6 | Phase 8 | FailureHandlerResolver | TC-FH-001–006 including column guard and fallthrough edge cases | 20% |
| 7 | Phase 9 | Integration Seam | TC-INT-001–006; full pipeline smoke (no stubs) | 40% |
| 7 | Phase 10 ✅ | CacheManager | TC-CACHE-001, 002, 006, 009, 010, NEG-001, BND-001; 1 source fix | 25% |
| 9 | Phases 11+12 | WorkspaceIndexer Core + File Discovery | TC-WI-001–008; TC-FILE-001 | 25–35% |
| 10 | Phase 13 | MCP Server | TC-MCP-001–014; WI-13.0 source change | 20% |
| 11 | Phase 14 | VSCode Mocks & Definition Provider | TC-VSC-001–008; `__mocks__/vscode.ts` | 35% |
| 12 | Phase 15 | Error UX, QuickPick & StatusBar | TC-ERR-001–006; TC-QP-001–005 | 20% |
| 14 | Phase 16 ✅ | End-to-End Smoke & Final Quality | Full suite; coverage gate; `tsc --noEmit`; real C file smoke; API audit | 20% |
| 15 | Phase 17 ✅ | Parser: Production Header Compatibility | macroCallStatement rule, void macro arg fix, 29/29 headers clean, TC-MACRO-STMT-001–006, NEG-001, BND-001, 009, TC-MODROOT-VOID-001, TC-BULK-004 | 10% |
| 16 | — | Traceability Audit & SDP Closure | Traceability matrix (21/21 REQs), SDP phases marked COMPLETE | 5% |
| 17 | Phase 18 ✅ | Parser & Indexer: Production Source Compatibility | looksLikeCast, braced init, reindexFile deferred, EOF sentinel, FloatingLiteral; REG-17-001–010b | 15% |
| 18 | — | Cross-File Definition Resolution Bug Fix | `resolveDefinitionLocation()` fix; TC-WI-015–018 regression tests | 10% |
| 19 | Phase 19 ✅ | Go-to-Definition Bug: Parser C11 Unary Conformance | Parser `unaryExpression` fix (& \* + - ~ ! → castExpression); REQ-VSCODE-005 tightened; REQ-VSCODE-033 added; TC-CONF-001–022, TC-WI-019, TC-WI-020a/b (23 new tests); VSIX 0.1.4 | 15% |
| 20 | Phase 20 ✅ | Ctrl-Hover Underline & Multi-Impl UX Fix | REQ-VSCODE-034 revised; REQ-VSCODE-035 added; `provideDefinition` returns first Location for multi-impl (no premature peek); TC-QP-004 updated; TC-VSC-009 added; VSIX 0.1.5 | 10% |
| 21 | Phase 21 ✅ | Peek Widget Regression Fix | `provideDefinition` reverted to return all locations; REQ-VSCODE-035 revised (multi-impl peek); TC-QP-004 + TC-VSC-009 updated; VSIX 0.1.6 | 5% |

**Total: 21 sprints, 20 active phases (21 numbered; Phase 4 removed)**

### Sprint Entry Criteria

Before starting any sprint:
1. Previous sprint's full test suite passes (0 failures)
2. All source bugs from previous sprint are fixed and verified
3. Lessons learned from previous sprint are documented in `ai/memory/lessons-learned-software-developer.md`
4. PM has approved the sprint plan

### Sprint Exit Criteria

A sprint is complete when:
1. All planned test cases are implemented
2. All tests pass (including all prior sprints' tests)
3. Any source bugs discovered are fixed, verified, and documented
4. ESLint/TSLint clean (when configured)
5. **Requirements-traceability HARD gate (v2.1 Amendment A3-revised):** Starting Sprint 2, every requirement in `requirements.json` that has test cases written during or before the current sprint must have at least one test with a valid `// @{"verify": ["REQ-..."]}'` tag. Every `verify` tag in test files must reference a valid requirement ID. Sprint CANNOT exit until this passes. This ensures test adequacy — tests that pass but don't trace to requirements provide no verifiable coverage.
6. **Coverage checkpoint (v2.1 Amendment A2):** Starting Sprint 2, no production source module with tests below 85% line coverage. Flagged modules must be addressed or have a documented remediation plan.
7. Final quality engineer has approved the sprint output
8. PM has approved the sprint deliverables

---
