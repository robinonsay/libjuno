> Part of: [Software Design Document](index.md) — Sections 1-2

# LibJuno VSCode Extension — Software Design Document

**Date:** 2026-04-14  
**Module:** VSCODE  
**Requirements file:** `requirements/vscode-extension/requirements.json`

---

// @{"design": ["REQ-VSCODE-001"]}
## 1. Overview

The LibJuno VSCode Extension assists developers navigating LibJuno-based embedded C projects. LibJuno uses vtable-based dependency injection (DI) — function calls dispatched through `ptApi->Foo(...)` — that standard IDE tooling cannot resolve to their concrete implementations. This extension bridges that gap by building a workspace-wide navigation index and wiring it into VSCode's native Go to Definition system.

The extension also exposes its resolution capabilities to AI agent platforms via an embedded MCP (Model Context Protocol) server, enabling AI-assisted workflows to benefit from the same vtable resolution as manual developer workflows.

### Requirements in Scope

| ID | Title |
|----|-------|
| REQ-VSCODE-001 | VSCode Extension |
| REQ-VSCODE-002 | Vtable-Aware Go to Definition |
| REQ-VSCODE-003 | LibJuno API Pattern Recognition |
| REQ-VSCODE-004 | Graceful Error on Missing Implementation |
| REQ-VSCODE-005 | Single Implementation Navigation |
| REQ-VSCODE-006 | Multiple Implementation Selection |
| REQ-VSCODE-007 | Native Go to Definition Integration |
| REQ-VSCODE-008 | Module Root API Discovery |
| REQ-VSCODE-009 | Module Derivation Chain Resolution |
| REQ-VSCODE-010 | Designated Initializer Recognition |
| REQ-VSCODE-011 | Direct Assignment Recognition |
| REQ-VSCODE-012 | Positional Initializer Recognition |
| REQ-VSCODE-013 | Informative Non-Intrusive Error |
| REQ-VSCODE-014 | Trait Root API Discovery |
| REQ-VSCODE-015 | Trait Derivation Chain Resolution |
| REQ-VSCODE-016 | Failure Handler Navigation |
| REQ-VSCODE-017 | AI Agent Accessibility |
| REQ-VSCODE-018 | AI Vtable Resolution Access |
| REQ-VSCODE-019 | AI Failure Handler Resolution Access |
| REQ-VSCODE-020 | Platform-Agnostic AI Interface |
| REQ-VSCODE-021 | C and C++ File Type Support |
| REQ-VSCODE-022 | FAIL Macro Failure Handler Navigation |
| REQ-VSCODE-023 | JUNO_FAIL Direct Handler Resolution |
| REQ-VSCODE-024 | JUNO_FAIL_MODULE Handler Resolution |
| REQ-VSCODE-025 | JUNO_FAIL_ROOT Handler Resolution |
| REQ-VSCODE-026 | JUNO_ASSERT_EXISTS_MODULE Handler Resolution |
| REQ-VSCODE-027 | Vtable Resolution Trace View |
| REQ-VSCODE-028 | Trace View Activation via Keyboard |
| REQ-VSCODE-029 | Trace View Activation via Command Palette |
| REQ-VSCODE-030 | Trace View Call Site Node |
| REQ-VSCODE-031 | Trace View Composition Root Node |
| REQ-VSCODE-032 | Trace View Implementation Node |

---

// @{"design": ["REQ-VSCODE-001"]}
## 2. Design Approach

### 2.1 Technology Stack

- **Language:** TypeScript, targeting the VSCode Extension API
- **Runtime:** Node.js (bundled with VSCode)
- **C parsing:** Chevrotain-based context-free grammar parser. Chevrotain is a zero-runtime-dependency parser generator for TypeScript that runs natively in Node.js. It produces a concrete syntax tree (CST) that visitor methods can walk to extract all index data in a single pass. Chevrotain includes built-in error recovery support, and its pure TypeScript implementation allows LibJuno macros to be treated as first-class grammar constructs without requiring a build system or native binaries.
- **Navigation index:** In-memory data structures populated at activation and maintained incrementally via file watchers.
- **Persistence:** JSON file cache at `.libjuno/navigation-cache.json` in the workspace root. Prevents full re-scan on every activation.
- **AI interface:** Embedded MCP (Model Context Protocol) server. MCP is platform-agnostic and supported by GitHub Copilot, Claude, and other AI platforms, satisfying REQ-VSCODE-020.

### 2.2 Alternatives Considered

| Alternative | Rejected Because |
|-------------|-----------------|
| Regex-based text scanning | Fragile on edge cases: Allman-style braces, multiline parameter lists, and the multiple macro forms of LibJuno constructs. Required 11 separate patterns (P1–P11) plus a 5-strategy call-site system to approximate what a grammar handles uniformly. Could not track local variable types without backward regex scans spanning up to 200 lines. |
| Full C AST parser (libclang via Node.js FFI) | Requires native binaries and FFI bindings — not portable across macOS, Linux, and Windows without a build step. Cannot expand `JUNO_MODULE_ROOT` and related macros without the full build system and include paths. Chevrotain avoids both issues: it is pure TypeScript and handles LibJuno macros as first-class grammar constructs. |
| LSP extension (C language server extension) | LSP hooks cannot override symbol resolution for macro-generated struct fields. |
| Per-request file scan (no index) | Unacceptably slow for large workspaces; each Go to Definition would freeze the editor. |

---
