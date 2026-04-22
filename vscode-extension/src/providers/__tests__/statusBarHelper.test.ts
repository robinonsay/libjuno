/// <reference types="jest" />
// @{"verify": ["REQ-VSCODE-004", "REQ-VSCODE-013"]}

import { StatusBarHelper } from '../statusBarHelper';
import * as vscode from 'vscode';
import { resetMocks } from '../../__mocks__/vscode';

describe('StatusBarHelper', () => {
    let helper: StatusBarHelper | undefined;

    beforeEach(() => {
        resetMocks();
        helper = undefined;
    });

    afterEach(() => {
        if (helper) {
            helper.dispose();
        }
        jest.useRealTimers();
    });

    // TC-ERR-001
    // @{"verify": ["REQ-VSCODE-004", "REQ-VSCODE-013"]}
    it('TC-ERR-001: Status bar message is displayed when showError() is called (non-intrusive)', () => {
        jest.useFakeTimers();
        helper = new StatusBarHelper();
        const item = (vscode.window.createStatusBarItem as jest.Mock).mock.results[0].value;

        helper.showError("No implementation found for 'JUNO_APP_API_T::Launch'.");

        expect(item.text).toContain('LibJuno');
        expect(item.text).toContain("No implementation found for 'JUNO_APP_API_T::Launch'.");
        expect(item.show).toHaveBeenCalled();
        expect(vscode.window.showErrorMessage).not.toHaveBeenCalled();
    });

    // TC-ERR-002
    // @{"verify": ["REQ-VSCODE-013"]}
    it('TC-ERR-002: Status bar message auto-clears after 5 seconds (text restoration)', () => {
        jest.useFakeTimers();
        helper = new StatusBarHelper();
        const item = (vscode.window.createStatusBarItem as jest.Mock).mock.results[0].value;

        helper.showIndexed(10);
        helper.showError('test error');

        // Error text is showing immediately after showError
        expect(item.text).toContain('warning');

        // At 4999ms: error text still showing
        jest.advanceTimersByTime(4999);
        expect(item.text).toContain('warning');

        // At 5000ms: text restored to the previous indexed display
        jest.advanceTimersByTime(1);
        expect(item.text).toBe('$(check) LibJuno: Indexed 10 files');
    });

    // TC-ERR-003
    // @{"verify": ["REQ-VSCODE-013"]}
    it('TC-ERR-003: Information message on repeated failure within 10 seconds', () => {
        jest.useFakeTimers();
        helper = new StatusBarHelper();

        // First error at T=0: showInformationMessage must NOT be called
        helper.showError('Error A');
        expect(vscode.window.showInformationMessage).not.toHaveBeenCalled();

        // Advance to T=8000ms (within the 10s window)
        jest.advanceTimersByTime(5000); // auto-clear fires
        jest.advanceTimersByTime(3000); // now T=8000

        // Second error at T=8000: within 10s window, showInformationMessage MUST be called
        helper.showError('Error B');
        expect(vscode.window.showInformationMessage).toHaveBeenCalledWith('Error B', 'Show Details');
    });
});
