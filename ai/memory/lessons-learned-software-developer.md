# Lessons Learned — Software Developer
*Read before every task. Append new entries concisely.*

### 2026-04-14 — Chevrotain CONSUME2 does NOT produce a "Token2" CST key
- `CONSUME2(Token)` → `children["Token"][1]`, not `children["Token2"]`.
- `CONSUME(T)`→`[0]`, `CONSUME2(T)`→`[1]`, `CONSUME3(T)`→`[2]`; same for SUBRULE.
- Dump `Object.keys(node.children)` in a diagnostic test before writing visitor code.

### 2026-04-14 — declarationSpecifiers must not greedily consume declarator identifiers
- `AT_LEAST_ONE` with `Identifier` as a typeSpecifier alternative will consume function names.
- Add a GATE: treat an Identifier as a type name only if followed by `*`, another Identifier, or `(`.

### 2026-04-14 — Chevrotain GATE inside AT_LEAST_ONE doesn't prevent loop entry; use MANY with GATE
- Inner GATEs don't block loop entry — Chevrotain pre-computes FIRST sets independently.
- Fix: `MANY({ GATE: () => ..., DEF: () => this.OR([...]) })`.
- The GATE must also cover non-Identifier specifier tokens (Const, Void, etc.).

### 2026-04-17 — Always use absolute directory paths when running commands
- Read `ai/memory/directory-map.md` before any terminal command.
- C: `cd /workspaces/libjuno && cd build && cmake --build .`
- Extension: `cd /workspaces/libjuno/vscode-extension && npm test`
