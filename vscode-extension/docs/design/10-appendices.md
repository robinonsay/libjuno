> Part of: [Software Design Document](index.md) — Appendices A-B

// @{"design": ["REQ-VSCODE-001", "REQ-VSCODE-021"]}
## Appendix A: Extension File Structure

```
vscode-extension/
  package.json              — extension manifest, commands, activationEvents
  tsconfig.json
  src/
    extension.ts            — activate(), register providers and commands
    parser/
      lexer.ts              — Chevrotain token definitions
      parser.ts             — Chevrotain grammar rules (CParser class)
      visitor.ts            — CST visitor (IndexBuildingVisitor)
      types.ts              — ParsedFile interface, record types, LocalTypeInfo
    indexer/
      workspaceIndexer.ts   — WorkspaceIndexer, file scan, FileSystemWatcher
      navigationIndex.ts    — NavigationIndex type and in-memory store
    resolver/
      vtableResolver.ts     — VtableResolver (chain-walk algorithm)
      failureHandlerResolver.ts
    vscode/
      junoDefinitionProvider.ts
      quickPickHelper.ts    — multiple result QuickPick
      statusBarHelper.ts    — non-intrusive error display
    providers/
      vtableTraceProvider.ts   — WebviewPanel trace view
    mcp/
      mcpServer.ts          — HTTP MCP server, tool registration
    cache/
      cacheManager.ts       — JSON read/write, hash comparison, atomic write
  design/
    design.md               — this document
    test-cases.md           — test case specification
```

## Appendix B: Key Type Summary

| TypeScript Type | Purpose |
|-----------------|---------|
| `ParsedFile` | Output of the Chevrotain parser + visitor for one file |
| `NavigationIndex` | In-memory index (Maps) populated by the CST visitor |
| `VtableResolutionResult` | Output of VtableResolver and FailureHandlerResolver |
| `ConcreteLocation` | `{ functionName, file, line, assignmentFile?, assignmentLine? }` — `line` is the function definition line; `assignmentFile`/`assignmentLine` are the composition root vtable assignment site |
| `FunctionDefinitionRecord` | `{ functionName, file, line, isStatic, signature? }` — one entry per definition site |
| `LocalTypeInfo` | Per-file local variable and parameter type maps, keyed per function |
| `TypeInfo` | `{ name, typeName, isPointer, isConst, isArray }` — one entry per variable/parameter |
| `CacheFile` | Serialized JSON cache schema (matches Section 4.2) |
| `TraceNode` | One node in the resolution trace: call site, composition root, or implementation |
