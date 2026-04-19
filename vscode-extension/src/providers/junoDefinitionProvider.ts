// @{"req": ["REQ-VSCODE-002", "REQ-VSCODE-005", "REQ-VSCODE-006", "REQ-VSCODE-007", "REQ-VSCODE-016", "REQ-VSCODE-034", "REQ-VSCODE-035"]}
import * as vscode from 'vscode';
import { VtableResolver } from '../resolver/vtableResolver';
import { FailureHandlerResolver } from '../resolver/failureHandlerResolver';
import { NavigationIndex, ConcreteLocation } from '../parser/types';
import { StatusBarHelper } from './statusBarHelper';

/** Error message returned by VtableResolver when the line has no LibJuno pattern. */
const NO_PATTERN_MSG = 'No LibJuno API call pattern found at cursor position.';

/** Error message returned by FailureHandlerResolver when the line is unrelated. */
const NO_HANDLER_PATTERN_MSG = 'Line does not contain a failure handler reference.';

/**
 * VSCode DefinitionProvider that resolves LibJuno vtable call sites and
 * failure handler references to their concrete implementation locations.
 * Registered for C and C++ documents (REQ-VSCODE-007).
 *
 * Contract (REQ-VSCODE-034/REQ-VSCODE-035): provideDefinition returns all resolved
 * locations for any recognized call site. Single-implementation call sites navigate
 * directly; multi-implementation call sites surface VSCode's native peek widget so
 * the user can select the desired implementation.
 */
export class JunoDefinitionProvider implements vscode.DefinitionProvider {
    constructor(
        private readonly vtableResolver: VtableResolver,
        private readonly failureHandlerResolver: FailureHandlerResolver,
        private readonly index: NavigationIndex,
        private readonly statusBar: StatusBarHelper,
        private readonly log: (msg: string) => void = console.log
    ) {}

    // @{"req": ["REQ-VSCODE-005", "REQ-VSCODE-007", "REQ-VSCODE-034", "REQ-VSCODE-035"]}
    async provideDefinition(
        document: vscode.TextDocument,
        position: vscode.Position,
        _token: vscode.CancellationToken
    ): Promise<vscode.Location[] | undefined> {
        const lineText = document.lineAt(position.line).text;
        const file = document.uri.fsPath;
        const line = position.line + 1; // convert to 1-based
        const column = position.character;

        // 1. Try failure handler resolver first (design §6.2 priority).
        let result = this.failureHandlerResolver.resolve(file, line, column, lineText);

        // 2. Fall through to vtable resolver if failure handler pattern not present.
        if (!result.found && result.errorMsg === NO_HANDLER_PATTERN_MSG) {
            result = this.vtableResolver.resolve(file, line, column, lineText);
        }

        if (!result.found || result.locations.length === 0) {
            // Only surface an error when a recognizable pattern existed but resolution failed.
            const msg = result.errorMsg ?? '';
            if (msg && msg !== NO_PATTERN_MSG && msg !== NO_HANDLER_PATTERN_MSG) {
                this.statusBar.showError(`Could not resolve implementation — ${msg}`);
            }
            return undefined;
        }

        // 3. Return all resolved locations (REQ-VSCODE-034/035).
        //    Single impl: VSCode navigates directly. Multi-impl: VSCode shows the
        //    native multi-definition peek widget for implementation selection.
        return toVscodeLocations(result.locations);
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
