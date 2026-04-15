// @{"req": ["REQ-VSCODE-006"]}
import * as vscode from 'vscode';
import * as path from 'path';
import { ConcreteLocation } from '../parser/types';

interface QuickPickImpl extends vscode.QuickPickItem {
    location: ConcreteLocation;
}

/**
 * Displays a QuickPick list for selecting among multiple concrete
 * implementation locations (Section 6.3 / REQ-VSCODE-006).
 *
 * @param results Non-empty list of concrete implementation locations.
 * @returns The selected location, or `undefined` if the picker was dismissed.
 */
export async function showImplementationQuickPick(
    results: ConcreteLocation[]
): Promise<ConcreteLocation | undefined> {
    const items: QuickPickImpl[] = results.map(loc => ({
        label: loc.functionName,
        description: `${path.basename(loc.file)}:${loc.line}`,
        detail: loc.file,
        location: loc,
    }));

    const chosen = await vscode.window.showQuickPick(items, {
        placeHolder: 'Select an implementation to navigate to',
        matchOnDescription: true,
        matchOnDetail: true,
    });

    return chosen?.location;
}
