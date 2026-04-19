# LibJuno VSCode Extension — Test Case Specification

> This document has been split into smaller files for easier agent navigation.
> Full content is in [`../docs/test-cases/`](../docs/test-cases/).
> The index file also contains the Lessons-Learned Coverage Map, Summary Tables, and Requirements Coverage Matrix.

## Quick Navigation

| File | Content |
|------|---------|
| [../docs/test-cases/index.md](../docs/test-cases/index.md) | Index, Lessons-Learned Coverage Map, Summary Tables, Requirements Coverage Matrix |
| [../docs/test-cases/01-visitor-struct.md](../docs/test-cases/01-visitor-struct.md) | Sections 1-4: visitStructDefinition (MODULE_ROOT, DERIVE, TRAIT_ROOT, TRAIT_DERIVE) |
| [../docs/test-cases/02-visitor-api-vtable.md](../docs/test-cases/02-visitor-api-vtable.md) | Sections 5-8: API Struct Field Extraction, Vtable Declaration variants |
| [../docs/test-cases/03-chain-walk.md](../docs/test-cases/03-chain-walk.md) | Section 9: Chain-Walk Call Site Resolution |
| [../docs/test-cases/04-failure-handler.md](../docs/test-cases/04-failure-handler.md) | Section 10: visitFailureHandlerAssignment |
| [../docs/test-cases/05-function-definition.md](../docs/test-cases/05-function-definition.md) | Section 11: visitFunctionDefinition |
| [../docs/test-cases/06-e2e-resolution.md](../docs/test-cases/06-e2e-resolution.md) | Section 12: End-to-End Resolution Tests |
| [../docs/test-cases/07-vscode-integration.md](../docs/test-cases/07-vscode-integration.md) | Sections 13-14: VSCode Integration and Error UX Tests |
| [../docs/test-cases/08-mcp-cache.md](../docs/test-cases/08-mcp-cache.md) | Sections 15-16: MCP Server and Cache Tests |
| [../docs/test-cases/09-navigation-quickpick.md](../docs/test-cases/09-navigation-quickpick.md) | Sections 17-18: Failure Handler Navigation and QuickPick Tests |
| [../docs/test-cases/10-lexer-parser.md](../docs/test-cases/10-lexer-parser.md) | Sections 20-21: Lexer Token Boundary and Parser Error Recovery Tests |
| [../docs/test-cases/11-local-preprocessor-macro.md](../docs/test-cases/11-local-preprocessor-macro.md) | Sections 22-24: LocalTypeInfo, Preprocessor Directive, Standalone Macro Tests |
| [../docs/test-cases/12-system-e2e.md](../docs/test-cases/12-system-e2e.md) | Section 19: System-Level End-to-End Tests |
