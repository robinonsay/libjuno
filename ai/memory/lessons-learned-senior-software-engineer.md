# Lessons Learned — Senior Software Engineer

## How to Use This File

- **Read** this file before starting any task
- **Append** a new entry when a mistake or insight is discovered during work or verification
- **Keep entries concise**: what went wrong, root cause, and corrective action
- **Do not delete** old entries — they form institutional knowledge
- **Date** each entry for chronological tracking

## Lessons

### 2026-04-14 — Chevrotain grammar bugs cause silent empty results, not parse errors
**What happened:** The parser's `declarationSpecifiers` rule greedily consumed the declarator identifier as a type specifier. Because `recoveryEnabled: true` was set in the parser constructor, parse errors were silently recovered and the visitor produced empty arrays instead of crashing.
**Root cause:** Recovery mode masks grammar bugs. Code that appears to "work" (no exceptions thrown) actually produces garbage CST trees that the visitor silently ignores.
**Corrective action:** When reviewing Chevrotain parser/visitor code:
1. Check that `parser.errors` is asserted to be empty in tests — recovery should not be relied upon for correctness.
2. Any visitor method that returns empty results for valid C input is a red flag for a grammar bug.
3. Review `AT_LEAST_ONE` and `MANY` rules for greedy consumption of tokens that should belong to a sibling rule.
4. Review CST key access patterns (`children["X"]` vs `children["X2"]`) — Chevrotain does NOT use numeric suffixes for keys.
