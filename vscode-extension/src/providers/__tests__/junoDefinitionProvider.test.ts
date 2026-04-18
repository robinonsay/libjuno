/// <reference types="jest" />
// @{"verify": ["REQ-VSCODE-001", "REQ-VSCODE-002", "REQ-VSCODE-004", "REQ-VSCODE-006", "REQ-VSCODE-007", "REQ-VSCODE-013", "REQ-VSCODE-016", "REQ-VSCODE-034", "REQ-VSCODE-035"]}

import * as vscode from 'vscode';
import { createMockExtensionContext, resetMocks, cancellationToken } from '../../__mocks__/vscode';
import { activate } from '../../extension';
import { JunoDefinitionProvider } from '../junoDefinitionProvider';
import { VtableResolver } from '../../resolver/vtableResolver';
import { FailureHandlerResolver } from '../../resolver/failureHandlerResolver';
import { createEmptyIndex } from '../../indexer/navigationIndex';
import { NavigationIndex } from '../../parser/types';
import { StatusBarHelper } from '../statusBarHelper';

// Mock McpServer to avoid binding real network ports during tests.
jest.mock('../../mcp/mcpServer', () => ({
    McpServer: jest.fn().mockImplementation(() => ({
        start: jest.fn().mockResolvedValue(6543),
        stop: jest.fn(),
    })),
}));

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

function createMockDocument(lineText: string, filePath: string = '/test/file.c'): any {
    return {
        lineAt: jest.fn((_line: number) => ({ text: lineText })),
        uri: { fsPath: filePath },
    };
}

function createMockPosition(line: number, character: number): any {
    return { line, character };
}

// ---------------------------------------------------------------------------
// Group A: Activation & Registration Tests
// ---------------------------------------------------------------------------

describe('Extension Activation', () => {

    beforeEach(() => {
        resetMocks();
    });

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-VSC-001: activate completes without throwing', async () => {
        const context = createMockExtensionContext();
        await expect(activate(context)).resolves.toBeUndefined();
        // Verify activation actually registered subscriptions (prevents no-op stub from passing)
        expect(context.subscriptions.length).toBeGreaterThan(0);
    });

    // @{"verify": ["REQ-VSCODE-001", "REQ-VSCODE-007"]}
    it('TC-VSC-002: DefinitionProvider registered for c and cpp', async () => {
        const context = createMockExtensionContext();
        await activate(context);
        expect(vscode.languages.registerDefinitionProvider).toHaveBeenCalled();
        const [selectorArg] = (vscode.languages.registerDefinitionProvider as jest.Mock).mock.calls[0];
        expect(selectorArg).toContainEqual({ language: 'c' });
        expect(selectorArg).toContainEqual({ language: 'cpp' });
    });

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-VSC-007: Both libjuno.goToImplementation and libjuno.reindexWorkspace commands registered', async () => {
        const context = createMockExtensionContext();
        await activate(context);
        const registeredCommands = (vscode.commands.registerCommand as jest.Mock).mock.calls.map(
            (c: any[]) => c[0]
        );
        expect(registeredCommands).toContain('libjuno.goToImplementation');
        expect(registeredCommands).toContain('libjuno.reindexWorkspace');
        expect(vscode.commands.registerCommand).toHaveBeenCalledTimes(2);
    });

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-VSC-006: FileSystemWatcher registered with C/C++ glob pattern', async () => {
        const context = createMockExtensionContext();
        await activate(context);
        expect(vscode.workspace.createFileSystemWatcher).toHaveBeenCalledWith(
            '**/*.{c,h,cpp,hpp,hh,cc}'
        );
    });
});

// ---------------------------------------------------------------------------
// Group B: JunoDefinitionProvider Resolution Tests
// ---------------------------------------------------------------------------

describe('JunoDefinitionProvider', () => {
    let index: NavigationIndex;
    let vtableResolver: VtableResolver;
    let fhResolver: FailureHandlerResolver;
    let statusBar: StatusBarHelper;
    let provider: JunoDefinitionProvider;

    const NO_PATTERN_MSG = 'No LibJuno API call pattern found at cursor position.';
    const NO_HANDLER_PATTERN_MSG = 'Line does not contain a failure handler reference.';

    beforeEach(() => {
        resetMocks();
        index = createEmptyIndex();
        vtableResolver = new VtableResolver(index);
        fhResolver = new FailureHandlerResolver(index);
        statusBar = new StatusBarHelper();

        // Set up default spy behaviour — safe fallthrough defaults.
        jest.spyOn(vtableResolver, 'resolve').mockReturnValue({
            found: false,
            locations: [],
            errorMsg: NO_PATTERN_MSG,
        });
        jest.spyOn(fhResolver, 'resolve').mockReturnValue({
            found: false,
            locations: [],
            errorMsg: NO_HANDLER_PATTERN_MSG,
        });

        provider = new JunoDefinitionProvider(vtableResolver, fhResolver, index, statusBar, jest.fn());
    });

    afterEach(() => {
        statusBar.dispose();
    });

    // @{"verify": ["REQ-VSCODE-002", "REQ-VSCODE-007"]}
    it('TC-VSC-003: vtable call → returns Location array with correct file and 0-based line', async () => {
        const doc = createMockDocument('ptSelf->ptApi->Launch(ptSelf);');
        const pos = createMockPosition(0, 5);

        (fhResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            errorMsg: NO_HANDLER_PATTERN_MSG,
        });
        (vtableResolver.resolve as jest.Mock).mockReturnValue({
            found: true,
            locations: [{ functionName: 'Now', file: '/test/impl.c', line: 110 }],
        });

        const result = await provider.provideDefinition(doc, pos, cancellationToken);

        expect(result).toBeDefined();
        expect(Array.isArray(result)).toBe(true);
        expect(result!.length).toBe(1);
        // toVscodeLocations converts 1-based line 110 → 0-based Position(109, 0)
        expect((result![0] as any).uri.fsPath).toBe('/test/impl.c');
        expect((result![0] as any).range.start.line).toBe(109);
    });

    // @{"verify": ["REQ-VSCODE-016"]}
    it('TC-VSC-004: failure handler match → returns Location array with correct file and 0-based line', async () => {
        const doc = createMockDocument('ptSelf->pOnFailure = MyFailureHandler;');
        const pos = createMockPosition(0, 5);

        (fhResolver.resolve as jest.Mock).mockReturnValue({
            found: true,
            locations: [{ functionName: 'OnFailure', file: '/test/handler.c', line: 50 }],
        });

        const result = await provider.provideDefinition(doc, pos, cancellationToken);

        expect(result).toBeDefined();
        expect(Array.isArray(result)).toBe(true);
        expect(result!.length).toBe(1);
        expect((result![0] as any).uri.fsPath).toBe('/test/handler.c');
        // toVscodeLocations converts 1-based line 50 → 0-based Position(49, 0)
        expect((result![0] as any).range.start.line).toBe(49);
    });

    // @{"verify": ["REQ-VSCODE-007"]}
    it('TC-VSC-005: no pattern match from either resolver → returns undefined', async () => {
        const doc = createMockDocument('int x = 0;');
        const pos = createMockPosition(0, 0);

        (fhResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            errorMsg: NO_HANDLER_PATTERN_MSG,
        });
        (vtableResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            errorMsg: NO_PATTERN_MSG,
        });

        const result = await provider.provideDefinition(doc, pos, cancellationToken);
        expect(result).toBeUndefined();
    });

    // @{"verify": ["REQ-VSCODE-004", "REQ-VSCODE-013"]}
    it('TC-VSC-008: real resolver error → setStatusBarMessage called with LibJuno prefix and error text', async () => {
        const doc = createMockDocument("ptApp->ptApi->Launch(ptApp);");
        const pos = createMockPosition(0, 5);

        (fhResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            errorMsg: NO_HANDLER_PATTERN_MSG,
        });
        (vtableResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            errorMsg: "No implementation found for 'JUNO_APP_API_T::Launch'.",
        });

        await provider.provideDefinition(doc, pos, cancellationToken);

        const item = (vscode.window.createStatusBarItem as jest.Mock).mock.results[0].value;
        expect(item.text).toContain('LibJuno');
        expect(item.text).toContain("JUNO_APP_API_T::Launch");
    });

    // @{"verify": ["REQ-VSCODE-007"]}
    it('TC-VSC-NEG-001: non-vtable line → returns undefined and setStatusBarMessage NOT called', async () => {
        const doc = createMockDocument('int x = 0;');
        const pos = createMockPosition(0, 0);

        (fhResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            errorMsg: NO_HANDLER_PATTERN_MSG,
        });
        (vtableResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            errorMsg: NO_PATTERN_MSG,
        });

        const result = await provider.provideDefinition(doc, pos, cancellationToken);
        expect(result).toBeUndefined();
        // StatusBarHelper.showError() must NOT have been called for no-pattern messages.
        const item = (vscode.window.createStatusBarItem as jest.Mock).mock.results[0].value;
        expect(item.text).toBe('');
    });

    // @{"verify": ["REQ-VSCODE-002"]}
    it('TC-VSC-BND-001: provideDefinition at line 0, column 0 on empty line → no throw, returns undefined', async () => {
        const doc = createMockDocument('', '/test/file.c');
        const pos = createMockPosition(0, 0);

        (fhResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            errorMsg: NO_HANDLER_PATTERN_MSG,
        });
        (vtableResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            errorMsg: NO_PATTERN_MSG,
        });

        const result = await provider.provideDefinition(doc, pos, cancellationToken);
        expect(result).toBeUndefined();
    });

    // @{"verify": ["REQ-VSCODE-013"]}
    it('TC-ERR-004: showErrorMessage is never called for any resolution failure — even after 3 invocations', async () => {
        (fhResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            errorMsg: NO_HANDLER_PATTERN_MSG,
        });
        (vtableResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            errorMsg: "No implementation found for 'JUNO_APP_API_T::Launch'.",
        });

        const doc = createMockDocument("ptApp->ptApi->Launch(ptApp);");
        const pos = createMockPosition(0, 5);

        await provider.provideDefinition(doc, pos, cancellationToken);
        await provider.provideDefinition(doc, pos, cancellationToken);
        await provider.provideDefinition(doc, pos, cancellationToken);

        // Resolution failures must never surface a modal dialog — status bar only.
        expect(vscode.window.showErrorMessage).not.toHaveBeenCalled();
    });

    // @{"verify": ["REQ-VSCODE-004", "REQ-VSCODE-013"]}
    it('TC-ERR-005: setStatusBarMessage includes API type and field name in error text', async () => {
        (fhResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            errorMsg: NO_HANDLER_PATTERN_MSG,
        });
        (vtableResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            errorMsg: "No implementation found for 'JUNO_DS_HEAP_API_T::Insert'.",
        });

        const doc = createMockDocument("ptDs->ptApi->Insert(ptDs);");
        const pos = createMockPosition(0, 5);

        await provider.provideDefinition(doc, pos, cancellationToken);

        const item = (vscode.window.createStatusBarItem as jest.Mock).mock.results[0].value;
        expect(item.text).toContain('LibJuno');
        expect(item.text).toContain('JUNO_DS_HEAP_API_T');
        expect(item.text).toContain('Insert');
    });

    // @{"verify": ["REQ-VSCODE-006", "REQ-VSCODE-034", "REQ-VSCODE-035"]}
    it('TC-QP-004: multi-impl vtable call → provideDefinition returns all Locations; no QuickPick shown', async () => {
        const loc1 = { functionName: 'OnStart', file: '/workspace/engine/src/engine_app.c', line: 128 };
        const loc2 = { functionName: 'OnStart', file: '/workspace/sys_manager/src/sys_manager_app.c', line: 77 };

        (fhResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            errorMsg: NO_HANDLER_PATTERN_MSG,
        });
        (vtableResolver.resolve as jest.Mock).mockReturnValue({
            found: true,
            locations: [loc1, loc2],
        });

        const doc = createMockDocument('ptSelf->ptApi->OnStart(ptSelf);');
        const pos = createMockPosition(0, 5);

        const result = await provider.provideDefinition(doc, pos, cancellationToken);

        // Provider must NOT invoke QuickPick — that is the command's responsibility.
        expect(vscode.window.showQuickPick).not.toHaveBeenCalled();

        // Provider returns all Locations — VSCode shows its native peek widget for multi-impl.
        expect(result).toBeDefined();
        expect(Array.isArray(result)).toBe(true);
        expect(result!.length).toBe(2);
        expect((result![0] as any).uri.fsPath).toBe('/workspace/engine/src/engine_app.c');
        // toVscodeLocations converts 1-based line 128 → 0-based Position(127, 0)
        expect((result![0] as any).range.start.line).toBe(127);
        expect((result![1] as any).uri.fsPath).toBe('/workspace/sys_manager/src/sys_manager_app.c');
        // toVscodeLocations converts 1-based line 77 → 0-based Position(76, 0)
        expect((result![1] as any).range.start.line).toBe(76);

        // No imperative navigation — VSCode handles routing via the returned Location[].
        expect(vscode.window.showTextDocument).not.toHaveBeenCalled();
    });

    // @{"verify": ["REQ-VSCODE-035"]}
    it('TC-VSC-009: multi-impl (3 locations) → provideDefinition returns all 3 Locations', async () => {
        const locations = [
            { functionName: 'Reset', file: '/impl/a.c', line: 10 },
            { functionName: 'Reset', file: '/impl/b.c', line: 20 },
            { functionName: 'Reset', file: '/impl/c.c', line: 30 },
        ];

        (fhResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            errorMsg: NO_HANDLER_PATTERN_MSG,
        });
        (vtableResolver.resolve as jest.Mock).mockReturnValue({
            found: true,
            locations,
        });

        const doc = createMockDocument('ptSelf->ptApi->Reset(ptSelf);');
        const pos = createMockPosition(0, 5);

        const result = await provider.provideDefinition(doc, pos, cancellationToken);

        // All 3 Locations returned — VSCode shows its native multi-definition peek widget.
        expect(result).toBeDefined();
        expect(result!.length).toBe(3);
        expect((result![0] as any).uri.fsPath).toBe('/impl/a.c');
        // toVscodeLocations converts 1-based line 10 → 0-based Position(9, 0)
        expect((result![0] as any).range.start.line).toBe(9);
        expect((result![1] as any).uri.fsPath).toBe('/impl/b.c');
        expect((result![1] as any).range.start.line).toBe(19);
        expect((result![2] as any).uri.fsPath).toBe('/impl/c.c');
        expect((result![2] as any).range.start.line).toBe(29);
    });
});
