# Lessons Learned — Software Developer

## How to Use This File

- **Read** this file before starting any task
- **Append** a new entry when a mistake or insight is discovered during work or verification
- **Keep entries concise**: what went wrong, root cause, and corrective action
- **Do not delete** old entries — they form institutional knowledge
- **Date** each entry for chronological tracking

## Lessons

### 2026-04-14 — Chevrotain CONSUME2 does NOT produce a "Token2" CST key
**What happened:** `visitor.ts` used `tok(c, "Identifier2")` to access the identifier from `CONSUME2(Identifier)` in the `structOrUnionSpecifier` rule. This returned `undefined` because Chevrotain stores all CONSUMEs of the same token type under the same key (e.g., `"Identifier"`) as an array, indexed by position.
**Root cause:** Misunderstanding of Chevrotain's CST key naming. `CONSUME2(Identifier)` stores in `children["Identifier"][1]`, not `children["Identifier2"]`.
**Corrective action:** When accessing CST children from a Chevrotain parser:
- `CONSUME(Token)` → `children["Token"][0]`
- `CONSUME2(Token)` → `children["Token"][1]`
- `CONSUME3(Token)` → `children["Token"][2]`
- Similarly for SUBRULE/SUBRULE2/etc.
Always write a small diagnostic test that dumps `Object.keys(node.children)` to verify before writing visitor code.

### 2026-04-14 — C parser declarationSpecifiers must not greedily consume declarator identifiers
**What happened:** The `declarationSpecifiers` rule uses `AT_LEAST_ONE` with `typeSpecifier` → `Identifier` as one alternative. For input like `static JUNO_STATUS_T OnStart(...)`, after consuming `static` and `JUNO_STATUS_T`, it also consumed `OnStart` as another type specifier, breaking function definition parsing.
**Root cause:** No lookahead to distinguish "this Identifier is a type name" from "this Identifier starts a declarator." In C, this is the classic typedef ambiguity.
**Corrective action:** Add a GATE or lookahead predicate to the Identifier alternative in `typeSpecifier` that checks whether the current context allows another type specifier. Common approach: limit to at most one Identifier type specifier per declarationSpecifiers, or use GATE to check if the Identifier is followed by a declarator-starting token (star, another Identifier, LParen).

### 2026-04-14 — Chevrotain GATE inside AT_LEAST_ONE does NOT prevent loop entry; use MANY with GATE instead
**What happened:** Placing a GATE on an Identifier alternative *inside* `typeSpecifier` within an `AT_LEAST_ONE` loop still caused errors. Even though the GATE returned false, the loop entered the iteration because Chevrotain's pre-computed FIRST sets include `Identifier`.
**Root cause:** Chevrotain computes implicit lookahead sets for `AT_LEAST_ONE`/`MANY` loops independently of any inner GATEs. When the current token is in the FIRST set, the loop body is entered regardless of inner GATEs. If GATE then blocks the only matching alternative, a parse error is thrown.
**Corrective action:** Move the GATE to the loop itself using `MANY({ GATE: ..., DEF: ... })` instead of `AT_LEAST_ONE`. When doing this, the GATE must also replicate the FIRST-set check for non-Identifier tokens (otherwise non-specifier tokens like `*` will incorrectly enter the loop). Pattern:
```typescript
this.MANY({
    GATE: () => {
        const la1 = this.LA(1);
        if (!tokenMatcher(la1, Identifier)) {
            // Only enter for tokens that can actually start a specifier
            return tokenMatcher(la1, Const) || tokenMatcher(la1, Void) || /* ...all specifier tokens... */;
        }
        // Identifier: only a type name if followed by another Identifier, *, qualifier, or (*
        const la2 = this.LA(2);
        return tokenMatcher(la2, Identifier) || tokenMatcher(la2, Star) || ...;
    },
    DEF: () => { this.OR([...]); },
});
```

### 2026-04-17 — Always use absolute directory paths when running commands
**What happened:** Agents ran build/test commands from the wrong working directory, causing failures.
**Root cause:** Commands used relative paths (`cd build`) without ensuring the current directory was correct.
**Corrective action:** Read `ai/memory/directory-map.md` before running any terminal command. Always use absolute `cd` paths: `cd /workspaces/libjuno && cd build && cmake --build .` for C code, `cd /workspaces/libjuno/vscode-extension && npm test` for the extension.
