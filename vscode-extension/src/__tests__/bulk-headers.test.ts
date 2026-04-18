/// <reference types="jest" />
// @{"verify": ["REQ-VSCODE-001", "REQ-VSCODE-003"]}

import * as fs from 'fs';
import * as path from 'path';
import { parseFile } from '../parser/visitor';
import { WorkspaceIndexer } from '../indexer/workspaceIndexer';

const INCLUDE_DIR = '/workspaces/libjuno/include/juno';

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

function collectHeaders(dir: string): string[] {
    const results: string[] = [];
    for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
        const full = path.join(dir, entry.name);
        if (entry.isDirectory()) {
            results.push(...collectHeaders(full));
        } else if (entry.isFile() && entry.name.endsWith('.h')) {
            results.push(full);
        }
    }
    return results;
}

const headers = collectHeaders(INCLUDE_DIR);

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

describe('Bulk Real-World Header Indexing', () => {

    it('TC-BULK-001: all production headers parse without throwing', () => {
        expect(headers.length).toBeGreaterThan(0);
        for (const h of headers) {
            const text = fs.readFileSync(h, 'utf8');
            expect(() => parseFile(h, text)).not.toThrow();
        }
    });

    it('TC-BULK-002: combined extraction meets minimum thresholds', () => {
        let totalRoots = 0;
        let totalApis = 0;
        let totalDerivations = 0;

        for (const h of headers) {
            const text = fs.readFileSync(h, 'utf8');
            const result = parseFile(h, text);
            totalRoots += result.moduleRoots.length;
            totalApis += result.apiStructDefinitions.length;
            totalDerivations += result.derivations.length;
        }

        expect(totalRoots).toBeGreaterThanOrEqual(15);
        expect(totalApis).toBeGreaterThanOrEqual(18);
        expect(totalDerivations).toBeGreaterThanOrEqual(2);
    });

    it('TC-BULK-003: WorkspaceIndexer.fullIndex on include directory completes without error', async () => {
        const indexer = new WorkspaceIndexer(INCLUDE_DIR, []);
        await expect(indexer.fullIndex()).resolves.not.toThrow();
        expect(indexer.index.moduleRoots.size).toBeGreaterThanOrEqual(15);
    }, 30000);
});
