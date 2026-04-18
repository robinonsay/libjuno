/// <reference types="jest" />
// @{"verify": ["REQ-VSCODE-006"]}

import { showImplementationQuickPick } from '../quickPickHelper';
import * as vscode from 'vscode';
import { resetMocks } from '../../__mocks__/vscode';
import { ConcreteLocation } from '../../parser/types';

// ---------------------------------------------------------------------------
// Test data
// ---------------------------------------------------------------------------

const LOCATIONS: ConcreteLocation[] = [
    { functionName: 'OnStart', file: 'engine/src/engine_app.c', line: 128 },
    { functionName: 'OnStart', file: 'sys_manager/src/sys_manager_app.c', line: 77 },
];

// ---------------------------------------------------------------------------
// Test Suite
// ---------------------------------------------------------------------------

describe('showImplementationQuickPick', () => {
    beforeEach(() => {
        resetMocks();
    });

    // TC-QP-001
    // @{"verify": ["REQ-VSCODE-006"]}
    it('TC-QP-001: QuickPick items show function name as label', async () => {
        await showImplementationQuickPick(LOCATIONS);

        const items: vscode.QuickPickItem[] =
            (vscode.window.showQuickPick as jest.Mock).mock.calls[0][0];

        expect(items).toHaveLength(2);
        expect(items.every(item => item.label === 'OnStart')).toBe(true);
    });

    // TC-QP-002
    // @{"verify": ["REQ-VSCODE-006"]}
    it('TC-QP-002: QuickPick items show file:line as description', async () => {
        await showImplementationQuickPick(LOCATIONS);

        const items: vscode.QuickPickItem[] =
            (vscode.window.showQuickPick as jest.Mock).mock.calls[0][0];

        expect(items[0].description).toBe('engine_app.c:128');
        expect(items[1].description).toBe('sys_manager_app.c:77');
    });

    // TC-QP-003
    // @{"verify": ["REQ-VSCODE-006"]}
    it('TC-QP-003: QuickPick items show full file path as detail', async () => {
        await showImplementationQuickPick(LOCATIONS);

        const items: vscode.QuickPickItem[] =
            (vscode.window.showQuickPick as jest.Mock).mock.calls[0][0];

        expect(items[0].detail).toBe('engine/src/engine_app.c');
        expect(items[1].detail).toBe('sys_manager/src/sys_manager_app.c');
    });

    // TC-QP-004
    // @{"verify": ["REQ-VSCODE-006"]}
    it('TC-QP-004: Selecting a QuickPick item returns the selected location', async () => {
        const selectedItem = {
            label: 'OnStart',
            description: 'engine_app.c:128',
            detail: 'engine/src/engine_app.c',
            location: LOCATIONS[0],
        };
        (vscode.window.showQuickPick as jest.Mock).mockResolvedValue(selectedItem);

        const result = await showImplementationQuickPick(LOCATIONS);

        expect(result).toBe(LOCATIONS[0]);
    });

    // TC-QP-005
    // @{"verify": ["REQ-VSCODE-006"]}
    it('TC-QP-005: Cancelling QuickPick returns undefined', async () => {
        (vscode.window.showQuickPick as jest.Mock).mockResolvedValue(undefined);

        const result = await showImplementationQuickPick(LOCATIONS);

        expect(result).toBeUndefined();
    });
});
