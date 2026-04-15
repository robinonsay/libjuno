// @{"req": ["REQ-VSCODE-013", "REQ-VSCODE-001"]}
import * as vscode from 'vscode';

/**
 * Manages the LibJuno status bar item for indexed file counts and transient
 * resolution-failure messages (Section 8.1).
 */
export class StatusBarHelper implements vscode.Disposable {
    private readonly item: vscode.StatusBarItem;
    private clearTimer: ReturnType<typeof setTimeout> | undefined;

    constructor() {
        this.item = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left);
        this.item.show();
    }

    /**
     * Shows "LibJuno: Indexed N files" in the status bar.
     * @param fileCount Number of indexed source files.
     */
    showIndexed(fileCount: number): void {
        this.cancelClear();
        this.item.text = `$(check) LibJuno: Indexed ${fileCount} files`;
        this.item.tooltip = 'LibJuno workspace index is ready';
        this.item.show();
    }

    /**
     * Shows an error icon and message. The message auto-clears after 5 seconds,
     * restoring the last indexed-count display (Section 8.1).
     * @param message Human-readable error description.
     */
    showError(message: string): void {
        this.cancelClear();
        const previous = this.item.text;
        this.item.text = `$(warning) LibJuno: ${message}`;
        this.item.show();
        this.clearTimer = setTimeout(() => {
            this.item.text = previous;
        }, 5000);
    }

    dispose(): void {
        this.cancelClear();
        this.item.dispose();
    }

    private cancelClear(): void {
        if (this.clearTimer !== undefined) {
            clearTimeout(this.clearTimer);
            this.clearTimer = undefined;
        }
    }
}
