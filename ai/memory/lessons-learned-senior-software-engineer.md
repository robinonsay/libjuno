# Lessons Learned — Senior Software Engineer
*Read before every task. Append new entries concisely.*

### 2026-04-14 — Chevrotain grammar bugs cause silent empty results, not parse errors
- `recoveryEnabled: true` masks grammar bugs — the visitor silently produces empty arrays.
- Always assert `parser.errors` is empty; do not rely on recovery for correctness.
- A visitor returning empty results for valid C input = red flag for a grammar bug.
- Check `AT_LEAST_ONE`/`MANY` rules for greedy token consumption and sibling-rule interference.
- CST keys never use numeric suffixes: `children["X"]` not `children["X2"]`.
