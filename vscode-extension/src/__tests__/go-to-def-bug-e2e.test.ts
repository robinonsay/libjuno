/// <reference types="jest" />

/**
 * @file go-to-def-bug-e2e.test.ts
 *
 * TC-WI-020 — End-to-end regression test for the Go-to-Definition bug,
 * exercised through the real `WorkspaceIndexer` pipeline (parse → merge →
 * NavigationIndex.functionDefinitions).
 *
 * Two parts per PM directive:
 *   (a) TC-WI-020a — primary case using the controlled fixture at
 *       `test/fixtures/go-to-def-bug/`. Asserts that `FixtureImpl_Reset` and
 *       `FixtureImpl_Copy` resolve to their definition lines (48 and 40),
 *       not the vtable initializer line (34/35) or the forward-declaration
 *       line (28/29).
 *   (b) TC-WI-020b — smoke assertion on the real
 *       `examples/example_project/engine/src/engine_cmd_msg.c` file that
 *       `EngineCmdMsg_Reset` resolves to line 88 (the definition line, NOT
 *       the forward declaration on line 31 nor the vtable initializer on
 *       line 53). This locks the exact PM-reported symptom.
 *
 * Both cases MUST FAIL (RED) against the pre-fix parser and MUST PASS (GREEN)
 * after the Phase 2 grammar fix.
 */

import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';
import { WorkspaceIndexer } from '../indexer/workspaceIndexer';

const FIXTURE_DIR =
    '/workspaces/libjuno/vscode-extension/test/fixtures/go-to-def-bug';
const FIXTURE_HEADER = path.join(FIXTURE_DIR, 'fixture_api.h');
const FIXTURE_IMPL = path.join(FIXTURE_DIR, 'fixture_impl.c');

const REAL_ENGINE_CMD_MSG =
    '/workspaces/libjuno/examples/example_project/engine/src/engine_cmd_msg.c';

// @{"verify": ["REQ-VSCODE-005"]}
describe('Go-to-Definition E2E — Fixture + Real File', () => {

    let tempDir: string;

    beforeAll(() => {
        tempDir = fs.mkdtempSync(path.join(os.tmpdir(), 'libjuno-e2e-gotodef-'));
    });

    afterAll(() => {
        fs.rmSync(tempDir, { recursive: true, force: true });
    });

    // -----------------------------------------------------------------------
    // TC-WI-020a — Primary fixture case
    // -----------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-005"]}
    it('TC-WI-020a: WorkspaceIndexer resolves Reset and Copy to their definition lines in fixture_impl.c', async () => {
        const subDir = path.join(tempDir, 'fixture');
        fs.mkdirSync(subDir);

        const headerPath = path.join(subDir, 'fixture_api.h');
        const implPath = path.join(subDir, 'fixture_impl.c');
        fs.copyFileSync(FIXTURE_HEADER, headerPath);
        fs.copyFileSync(FIXTURE_IMPL, implPath);

        const indexer = new WorkspaceIndexer(subDir, []);
        // Index the header first so API struct definitions reach the impl file.
        await indexer.reindexFile(headerPath);
        await indexer.reindexFile(implPath);

        // FixtureImpl_Reset — line 48 in the fixture
        const resetDefs = indexer.index.functionDefinitions.get('FixtureImpl_Reset');
        expect(resetDefs).toBeDefined();
        expect(resetDefs!).toHaveLength(1);
        expect(resetDefs![0].line).toBe(48);
        expect(resetDefs![0].file).toBe(implPath);

        // FixtureImpl_Copy — line 40 in the fixture
        const copyDefs = indexer.index.functionDefinitions.get('FixtureImpl_Copy');
        expect(copyDefs).toBeDefined();
        expect(copyDefs!).toHaveLength(1);
        expect(copyDefs![0].line).toBe(40);
        expect(copyDefs![0].file).toBe(implPath);
    });

    // -----------------------------------------------------------------------
    // TC-WI-020b — Smoke assertion on the real engine_cmd_msg.c
    //
    // Locks the exact PM-reported symptom: ctrl-click on the `Reset` field
    // in the vtable initializer at line 53 must resolve to the function
    // definition on line 88, not to the forward declaration on line 31 nor
    // to the vtable initializer line itself.
    // -----------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-005"]}
    it('TC-WI-020b: WorkspaceIndexer resolves EngineCmdMsg_Reset to line 88 in real engine_cmd_msg.c', async () => {
        const subDir = path.join(tempDir, 'real');
        fs.mkdirSync(subDir);

        const copiedPath = path.join(subDir, 'engine_cmd_msg.c');
        fs.copyFileSync(REAL_ENGINE_CMD_MSG, copiedPath);

        const indexer = new WorkspaceIndexer(subDir, []);
        await indexer.reindexFile(copiedPath);

        // EngineCmdMsg_Reset — definition line is 88 in the real file.
        const defs = indexer.index.functionDefinitions.get('EngineCmdMsg_Reset');
        expect(defs).toBeDefined();
        expect(defs!.length).toBeGreaterThanOrEqual(1);

        // At least one record must point to the definition line (88), not the
        // forward declaration (31) or vtable initializer entry (53).
        const definitionLines = defs!.map((d) => d.line);
        expect(definitionLines).toContain(88);
    });
});
