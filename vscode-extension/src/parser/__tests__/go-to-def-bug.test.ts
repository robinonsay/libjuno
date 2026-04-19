/// <reference types="jest" />

/**
 * @file go-to-def-bug.test.ts
 *
 * TC-WI-019 — Parser unit regression test for the Go-to-Definition bug.
 *
 * When a function body contains:
 *   - `*(TYPE *) ptr = (TYPE){0};`        (compound-literal assignment), or
 *   - `*(TYPE *) ptr = *(TYPE *) src;`    (dereference-of-cast assignment),
 *
 * and is preceded by a `JUNO_ASSERT_SUCCESS(expr, return tStatus);` macro
 * invocation, the Chevrotain parser's `unaryExpression` rule (which conflated
 * the C11 §6.5.3 `++`/`--` and `& * + - ~ !` recursion targets) failed to
 * consume the function body, causing the indexer to drop the
 * `FunctionDefinitionRecord` entirely.
 *
 * The fixture at `test/fixtures/go-to-def-bug/fixture_impl.c` reproduces the
 * exact shape reported by the PM. Ground truth line numbers:
 *   - `FixtureImpl_Copy`  definition on line 40
 *   - `FixtureImpl_Reset` definition on line 48
 *
 * This test MUST FAIL (RED) against the pre-fix parser and MUST PASS (GREEN)
 * after the Phase 2 grammar fix.
 */

import * as fs from 'fs';
import { parseFileWithDefs } from '../visitor';

const FIXTURE_PATH =
    '/workspaces/libjuno/vscode-extension/test/fixtures/go-to-def-bug/fixture_impl.c';

// @{"verify": ["REQ-VSCODE-005"]}
describe('Go-to-Definition Bug — Parser Fixture', () => {

    // @{"verify": ["REQ-VSCODE-005"]}
    it('TC-WI-019: parseFileWithDefs records FixtureImpl_Reset at line 48 and FixtureImpl_Copy at line 40', () => {
        const text = fs.readFileSync(FIXTURE_PATH, 'utf8');
        const result = parseFileWithDefs(FIXTURE_PATH, text);

        expect(Array.isArray(result.functionDefs)).toBe(true);

        // FixtureImpl_Copy — line 40
        const copyDefs = result.functionDefs.filter(
            (def) => def.functionName === 'FixtureImpl_Copy',
        );
        expect(copyDefs).toHaveLength(1);
        expect(copyDefs[0].line).toBe(40);
        expect(copyDefs[0].isStatic).toBe(true);
        expect(copyDefs[0].file).toBe(FIXTURE_PATH);

        // FixtureImpl_Reset — line 48
        const resetDefs = result.functionDefs.filter(
            (def) => def.functionName === 'FixtureImpl_Reset',
        );
        expect(resetDefs).toHaveLength(1);
        expect(resetDefs[0].line).toBe(48);
        expect(resetDefs[0].isStatic).toBe(true);
        expect(resetDefs[0].file).toBe(FIXTURE_PATH);
    });
});
