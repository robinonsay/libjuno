// @{"req": ["REQ-VSCODE-002", "REQ-VSCODE-005", "REQ-VSCODE-006", "REQ-VSCODE-007", "REQ-VSCODE-016"]}
import * as vscode from 'vscode';
import { VtableResolver } from '../resolver/vtableResolver';
import { FailureHandlerResolver } from '../resolver/failureHandlerResolver';
import { NavigationIndex, ConcreteLocation } from '../parser/types';
import { showImplementationQuickPick } from './quickPickHelper';
import { StatusBarHelper } from './statusBarHelper';

/** Error message returned by VtableResolver when the line has no LibJuno pattern. */
const NO_PATTERN_MSG = 'No LibJuno API call pattern found at cursor position.';

/** Error message returned by FailureHandlerResolver when the line is unrelated. */
const NO_HANDLER_PATTERN_MSG = 'Line does not contain a failure handler reference.';

/**
 * VSCode DefinitionProvider that resolves LibJuno vtable call sites and
 * failure handler references to their concrete implementation locations.
 * Registered for C and C++ documents (REQ-VSCODE-007).
 */
export class JunoDefinitionProvider implements vscode.DefinitionProvider {
    constructor(
        private readonly vtableResolver: VtableResolver,
        private readonly failureHandlerResolver: FailureHandlerResolver,
        private readonly index: NavigationIndex,
        private readonly statusBar: StatusBarHelper
    ) {}

    async provideDefinition(
        document: vscode.TextDocument,
        position: vscode.Position,
        token: vscode.CancellationToken
    ): Promise<vscode.Location[] | undefined> {
        const lineText = document.lineAt(position.line).text;
        const file = document.uri.fsPath;
        const line = position.line + 1; // convert to 1-based
        const column = position.character;

        // 2. Try failure handler resolver first (design §6.2 priority)
        let result = this.failureHandlerResolver.resolve(file, line, column, lineText);

        // 3. Fall through to vtable resolver if failure handler pattern not present
        if (!result.found && result.errorMsg === NO_HANDLER_PATTERN_MSG) {
            result = this.vtableResolver.resolve(file, line, column, lineText);
        }

        if (!result.found || result.locations.length === 0) {
            // Only surface an error when a recognizable pattern existed but resolution failed
            const msg = result.errorMsg ?? '';
            if (msg && msg !== NO_PATTERN_MSG && msg !== NO_HANDLER_PATTERN_MSG) {
                this.statusBar.showError(`Could not resolve implementation — ${msg}`);
            }
            return undefined;
        }

        // 4a. Single location: return directly for native VSCode navigation (REQ-VSCODE-005)
        if (result.locations.length === 1) {
            return toVscodeLocations(result.locations);
        }

        // 4b. Multiple locations: show QuickPick and navigate imperatively (REQ-VSCODE-006)
        token.onCancellationRequested(() => { /* QuickPick auto-dismisses on cancellation */ });
        const selected = await showImplementationQuickPick(result.locations);
        if (selected) {
            await navigateTo(selected);
        }
        return undefined;
    }
}

function toVscodeLocations(locations: ConcreteLocation[]): vscode.Location[] {
    return locations.map(loc =>
        new vscode.Location(
            vscode.Uri.file(loc.file),
            new vscode.Position(Math.max(0, loc.line - 1), 0)
        )
    );
}

async function navigateTo(loc: ConcreteLocation): Promise<void> {
    try {
        const uri = vscode.Uri.file(loc.file);
        const pos = new vscode.Position(Math.max(0, loc.line - 1), 0);
        await vscode.window.showTextDocument(uri, {
            selection: new vscode.Range(pos, pos),
        });
    } catch (err) {
        console.log('[LibJuno] Failed to navigate to definition:', err);
    }
}
