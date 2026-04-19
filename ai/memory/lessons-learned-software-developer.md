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

### 2026-04-19 — LibJuno Broker/Pipe API: correct usage patterns
- `JUNO_SB_BROKER_API_T` has only `Publish(ptBroker, tMid, tMsg: JUNO_POINTER_T)` and `RegisterSubscriber(ptBroker, ptPipe)` — NO `Dequeue` on the broker.
- `RegisterSubscriber` takes exactly 2 args: the broker pointer and the pipe pointer (NOT a MID; MID is set via `JunoSb_PipeInit`).
- Dequeue must call the pipe's embedded queue directly: `tPipe.tRoot.ptApi->Dequeue(&tPipe.tRoot, tReturn)`.
- Empty queue sentinel: `JUNO_STATUS_OOB_ERROR` (from `juno_buff_queue.c:72`) — NOT `JUNO_STATUS_EMPTY` (does not exist).
- `Publish` requires a `JUNO_POINTER_T` fat pointer: `JUNO_POINTER_T tPtr = JunoMemory_PointerInit(ptPointerApi, TYPE, &tVar)`.
- Pipe init sequence: `JunoSb_PipeInit(&tPipe, MID, ptArray, pfcnFailureHandler, pvUserData)` → `ptBroker->ptApi->RegisterSubscriber(ptBroker, &tPipe)`.

### 2026-04-19 — LibJuno module derivation: correct struct syntax
- `JUNO_MODULE_DERIVE` expands to a struct body. Do NOT write `struct TAG JUNO_MODULE_DERIVE(...);` as a statement.
- Correct derivation form: `struct JUNO_UDP_LINUX_TAG { JUNO_UDP_ROOT_T tRoot; };` with explicit brace body.
- Module union (expanded JUNO_MODULE macro form): `union TAG { ROOT_T tRoot; DERIVED_T tDerived; };`
