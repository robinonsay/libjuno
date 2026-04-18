/// <reference types="jest" />
// @{"verify": ["REQ-VSCODE-001", "REQ-VSCODE-002", "REQ-VSCODE-006", "REQ-VSCODE-021"]}

/**
 * @file extension-branches.test.ts
 *
 * Targeted branch-coverage tests for src/extension.ts.
 *
 * Branches covered:
 *   Branch 0 arm 0  — activate: no workspace folders -> early return
 *   Branch 2 arm 1  — activate: cacheLoaded=true -> fullIndex skipped
 *   Branch 3 arm 0  — goToImplementation: no active editor -> early return
 *   Branch 3 arm 1  — goToImplementation: editor present
 *   Branch 4 arm 0  — goToImplementation: FH not found -> call VT resolver
 *   Branch 4 arm 1  — goToImplementation: FH found -> skip VT resolver
 *   Branch 5 arm 0  — goToImplementation: not found -> show error
 *   Branch 5 arm 1  — goToImplementation: found, locations >= 1 -> navigate
 *   Branch 6 arm 0  — goToImplementation: !result.found short-circuits the ||
 *   Branch 6 arm 1  — goToImplementation: found=true but locations empty
 *   Branch 7 arm 0  — goToImplementation: result.errorMsg is present
 *   Branch 7 arm 1  — goToImplementation: result.errorMsg is undefined -> default msg
 *   Branch 8 arm 0  — goToImplementation: single location -> direct navigate
 *   Branch 8 arm 1  — goToImplementation: multiple locations -> QuickPick
 *   Branch 9 arm 0  — goToImplementation: QuickPick selection -> navigate
 *   Branch 9 arm 1  — goToImplementation: QuickPick cancelled -> no navigate
 *   Branch 10 arm 1 — activate: mcpServerPort <= 0 -> MCP server not started
 *   Branch 11 arm 0 — deactivate: mcpServer defined -> stop() called
 *   Branch 11 arm 1 — deactivate: mcpServer undefined -> no-op
 */

import * as vscode from 'vscode';
import * as fs from 'fs';
import { createMockExtensionContext, resetMocks } from '../__mocks__/vscode';
import { activate, deactivate } from '../extension';

// ---------------------------------------------------------------------------
// Module-level capture variables — updated each time activate() creates instances
// ---------------------------------------------------------------------------

let _capturedFhInstance: { resolve: jest.Mock } | undefined;
let _capturedVtInstance: { resolve: jest.Mock } | undefined;
let _capturedMcpInstance: { start: jest.Mock; stop: jest.Mock } | undefined;
let _capturedIndexerInstance: {
    loadFromCache: jest.Mock;
    fullIndex: jest.Mock;
    reindexFile: jest.Mock;
    removeFile: jest.Mock;
    index: { localTypeInfo: Map<any, any> };
} | undefined;

/** Controls what loadFromCache resolves to in the mocked WorkspaceIndexer. */
let _indexerLoadFromCacheResult = false;

// ---------------------------------------------------------------------------
// Module mocks — factories capture instances into module-level variables
// ---------------------------------------------------------------------------

jest.mock('../mcp/mcpServer', () => ({
    McpServer: jest.fn().mockImplementation(() => {
        _capturedMcpInstance = {
            start: jest.fn().mockResolvedValue(6543),
            stop: jest.fn(),
        };
        return _capturedMcpInstance;
    }),
}));

jest.mock('../indexer/workspaceIndexer', () => ({
    WorkspaceIndexer: jest.fn().mockImplementation(() => {
        _capturedIndexerInstance = {
            loadFromCache: jest.fn().mockImplementation(
                () => Promise.resolve(_indexerLoadFromCacheResult)
            ),
            fullIndex: jest.fn().mockResolvedValue(undefined),
            reindexFile: jest.fn().mockResolvedValue(undefined),
            removeFile: jest.fn(),
            index: { localTypeInfo: new Map() },
        };
        return _capturedIndexerInstance;
    }),
}));

jest.mock('../resolver/vtableResolver', () => ({
    VtableResolver: jest.fn().mockImplementation(() => {
        _capturedVtInstance = {
            resolve: jest.fn().mockReturnValue({
                found: false,
                locations: [],
                errorMsg: 'No LibJuno API call pattern found at cursor position.',
            }),
        };
        return _capturedVtInstance;
    }),
}));

jest.mock('../resolver/failureHandlerResolver', () => ({
    FailureHandlerResolver: jest.fn().mockImplementation(() => {
        _capturedFhInstance = {
            resolve: jest.fn().mockReturnValue({
                found: false,
                locations: [],
                errorMsg: 'Line does not contain a failure handler reference.',
            }),
        };
        return _capturedFhInstance;
    }),
}));

// Mock fs so writeMcpDiscoveryFile doesn't touch the real filesystem.
// mkdirSync and writeFileSync are jest.fn()s; tests can make mkdirSync throw.
jest.mock('fs', () => ({
    ...jest.requireActual('fs'),
    mkdirSync: jest.fn(),
    writeFileSync: jest.fn(),
}));

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

function createMockEditor(
    filePath: string,
    line: number,
    character: number,
    lineText: string
): any {
    return {
        selection: { active: { line, character } },
        document: {
            lineAt: jest.fn((_l: number) => ({ text: lineText })),
            uri: { fsPath: filePath },
        },
    };
}

async function activateAndGetCommand(commandId: string): Promise<(...args: any[]) => any> {
    const context = createMockExtensionContext();
    await activate(context);
    const calls = (vscode.commands.registerCommand as jest.Mock).mock.calls;
    const entry = calls.find((c: any[]) => c[0] === commandId);
    return entry[1];
}

// ---------------------------------------------------------------------------
// Tests: Activation edge cases
// ---------------------------------------------------------------------------

describe('Extension Activation — edge cases', () => {

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-EXT-001: no workspace folders -> early return, no subscriptions pushed', async () => {
        resetMocks();
        (vscode.workspace as any).workspaceFolders = [];
        const context = createMockExtensionContext();

        await activate(context);

        // No subscriptions pushed because activate returned early (Branch 0 arm 0)
        expect(context.subscriptions.length).toBe(0);
    });

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-EXT-002: loadFromCache returns true -> fullIndex not called (Branch 2 arm 1)', async () => {
        resetMocks();
        _indexerLoadFromCacheResult = true;
        const context = createMockExtensionContext();

        await activate(context);

        expect(_capturedIndexerInstance!.loadFromCache).toHaveBeenCalledTimes(1);
        expect(_capturedIndexerInstance!.fullIndex).not.toHaveBeenCalled();

        _indexerLoadFromCacheResult = false; // restore default
    });

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-EXT-003: mcpServerPort=0 -> McpServer constructor never called (Branch 10 arm 1)', async () => {
        resetMocks();
        (vscode.workspace.getConfiguration as jest.Mock).mockImplementation((_section?: string) => ({
            get: jest.fn((key: string, defaultVal?: any) => {
                if (key === 'mcpServerPort') { return 0; }
                return defaultVal;
            }),
        }));
        _capturedMcpInstance = undefined;

        const context = createMockExtensionContext();
        await activate(context);

        // McpServer was never instantiated (skipped by if (mcpServerPort > 0))
        expect(_capturedMcpInstance).toBeUndefined();
    });

});

// ---------------------------------------------------------------------------
// Tests: libjuno.goToImplementation command branches
// ---------------------------------------------------------------------------

describe('libjuno.goToImplementation — command branches', () => {

    let goToImpl: (...args: any[]) => any;

    beforeEach(async () => {
        jest.useFakeTimers();
        resetMocks();
        goToImpl = await activateAndGetCommand('libjuno.goToImplementation');

        // Defaults: both resolvers return not-found
        _capturedFhInstance!.resolve.mockReturnValue({
            found: false,
            locations: [],
            errorMsg: 'Line does not contain a failure handler reference.',
        });
        _capturedVtInstance!.resolve.mockReturnValue({
            found: false,
            locations: [],
            errorMsg: 'No LibJuno API call pattern found at cursor position.',
        });
    });

    afterEach(() => {
        jest.useRealTimers();
    });

    // @{"verify": ["REQ-VSCODE-002"]}
    it('TC-EXT-010: no active editor -> early return, no showTextDocument (Branch 3 arm 0)', async () => {
        vscode.window.activeTextEditor = undefined;

        await goToImpl();

        expect(vscode.window.showTextDocument).not.toHaveBeenCalled();
        expect(vscode.window.showErrorMessage).not.toHaveBeenCalled();
    });

    // @{"verify": ["REQ-VSCODE-002", "REQ-VSCODE-021"]}
    it('TC-EXT-011: FH found, 1 result -> skip VT resolver, navigate directly (Branches 3/4/5/8)', async () => {
        _capturedFhInstance!.resolve.mockReturnValue({
            found: true,
            locations: [{ file: '/src/handler.c', line: 42, functionName: 'Handler' }],
        });
        vscode.window.activeTextEditor = createMockEditor('/src/test.c', 0, 5, 'ptSelf->pOnFailure = Handler;');

        await goToImpl();

        // FH found -> VT resolver NOT called (Branch 4 arm 1)
        expect(_capturedVtInstance!.resolve).not.toHaveBeenCalled();
        // Single location -> direct navigate (Branch 5 arm 1, Branch 8 arm 0)
        expect(vscode.window.showTextDocument).toHaveBeenCalledTimes(1);
        const uriArg = (vscode.window.showTextDocument as jest.Mock).mock.calls[0][0];
        expect(uriArg.fsPath).toBe('/src/handler.c');
        const optsArg = (vscode.window.showTextDocument as jest.Mock).mock.calls[0][1];
        expect(optsArg.selection.start.line).toBe(41); // 1-based 42 -> 0-based 41
    });

    // @{"verify": ["REQ-VSCODE-002", "REQ-VSCODE-013"]}
    it('TC-EXT-012: FH not found, VT not found, errorMsg set -> statusBar error (Branches 4/5/6/7 arm 0)', async () => {
        _capturedFhInstance!.resolve.mockReturnValue({ found: false, locations: [], errorMsg: 'no fh' });
        _capturedVtInstance!.resolve.mockReturnValue({ found: false, locations: [], errorMsg: 'Specific VT error msg' });
        vscode.window.activeTextEditor = createMockEditor('/src/test.c', 0, 5, 'int x = 0;');

        await goToImpl();

        // VT resolver called because FH not found (Branch 4 arm 0)
        expect(_capturedVtInstance!.resolve).toHaveBeenCalledTimes(1);
        expect(vscode.window.showTextDocument).not.toHaveBeenCalled();
        // statusBar.showError sets item.text (Branch 5 arm 0, Branch 6 arm 0, Branch 7 arm 0)
        const item = (vscode.window.createStatusBarItem as jest.Mock).mock.results[0].value;
        expect(item.text).toContain('Specific VT error msg');
    });

    // @{"verify": ["REQ-VSCODE-002", "REQ-VSCODE-013"]}
    it('TC-EXT-013: found=true, empty locations, no errorMsg -> default message (Branches 6 arm 1, 7 arm 1)', async () => {
        // found=true but locations=[] -> second operand of || triggers (Branch 6 arm 1)
        // errorMsg undefined -> ?? yields default string (Branch 7 arm 1)
        _capturedVtInstance!.resolve.mockReturnValue({ found: true, locations: [] });
        vscode.window.activeTextEditor = createMockEditor('/src/test.c', 0, 5, 'ptSelf->ptApi->Launch(ptSelf);');

        await goToImpl();

        expect(vscode.window.showTextDocument).not.toHaveBeenCalled();
        const item = (vscode.window.createStatusBarItem as jest.Mock).mock.results[0].value;
        expect(item.text).toContain('No implementation found.');
    });

    // @{"verify": ["REQ-VSCODE-002", "REQ-VSCODE-006"]}
    it('TC-EXT-014: found, 2 results, QuickPick selected -> navigate (Branches 8 arm 1, 9 arm 0)', async () => {
        _capturedVtInstance!.resolve.mockReturnValue({
            found: true,
            locations: [
                { file: '/src/a.c', line: 10, functionName: 'FuncA' },
                { file: '/src/b.c', line: 20, functionName: 'FuncB' },
            ],
        });
        // showImplementationQuickPick wraps locations in QuickPickItems with a `location` prop;
        // returning items[0] means chosen.location = { file: '/src/a.c', line: 10, ... }
        (vscode.window.showQuickPick as jest.Mock).mockImplementation(async (items: any[]) => items[0]);
        vscode.window.activeTextEditor = createMockEditor('/src/test.c', 0, 5, 'ptSelf->ptApi->Launch(ptSelf);');

        await goToImpl();

        // QuickPick shown (Branch 8 arm 1)
        expect(vscode.window.showQuickPick).toHaveBeenCalled();
        // Navigation occurred (Branch 9 arm 0)
        expect(vscode.window.showTextDocument).toHaveBeenCalledTimes(1);
        const uriArg = (vscode.window.showTextDocument as jest.Mock).mock.calls[0][0];
        expect(uriArg.fsPath).toBe('/src/a.c');
        const optsArg = (vscode.window.showTextDocument as jest.Mock).mock.calls[0][1];
        expect(optsArg.selection.start.line).toBe(9); // 1-based 10 -> 0-based 9
    });

    // @{"verify": ["REQ-VSCODE-002", "REQ-VSCODE-006"]}
    it('TC-EXT-015: found, 2 results, QuickPick cancelled -> no navigation (Branch 9 arm 1)', async () => {
        _capturedVtInstance!.resolve.mockReturnValue({
            found: true,
            locations: [
                { file: '/src/a.c', line: 10, functionName: 'FuncA' },
                { file: '/src/b.c', line: 20, functionName: 'FuncB' },
            ],
        });
        (vscode.window.showQuickPick as jest.Mock).mockResolvedValue(undefined); // user dismissed
        vscode.window.activeTextEditor = createMockEditor('/src/test.c', 0, 5, 'ptSelf->ptApi->Launch(ptSelf);');

        await goToImpl();

        expect(vscode.window.showQuickPick).toHaveBeenCalled();
        // Cancelled -> no navigation (Branch 9 arm 1)
        expect(vscode.window.showTextDocument).not.toHaveBeenCalled();
    });

});

// ---------------------------------------------------------------------------
// Tests: deactivate() branches
// ---------------------------------------------------------------------------

describe('deactivate()', () => {

    beforeEach(() => {
        resetMocks();
        // Clear any mcpServer state left by previous tests (safe no-op if already undefined)
        deactivate();
    });

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-EXT-020: deactivate when mcpServer is undefined -> no throw (Branch 11 arm 1)', () => {
        // beforeEach called deactivate() -> mcpServer=undefined.
        // Call again -> if(mcpServer) is false -> Branch 11 arm 1.
        _capturedMcpInstance = undefined;
        expect(() => deactivate()).not.toThrow();
        // stop was not called
        expect(_capturedMcpInstance).toBeUndefined();
    });

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-EXT-021: deactivate when mcpServer is set -> stop() called once (Branch 11 arm 0)', async () => {
        _capturedMcpInstance = undefined;
        const context = createMockExtensionContext();
        await activate(context);

        expect(_capturedMcpInstance).toBeDefined();
        const instance = _capturedMcpInstance!;

        deactivate();

        expect(instance.stop).toHaveBeenCalledTimes(1);
    });

});

// ---------------------------------------------------------------------------
// Tests: libjuno.reindexWorkspace command branches (lines 116-129)
// ---------------------------------------------------------------------------

describe('libjuno.reindexWorkspace — command branches', () => {

    beforeEach(() => {
        resetMocks();
    });

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-EXT-030: reindexWorkspace happy path -> fullIndex called, statusBar reflects indexed count', async () => {
        const cmd = await activateAndGetCommand('libjuno.reindexWorkspace');
        _capturedIndexerInstance!.fullIndex.mockClear(); // clear activation-time invocation
        _capturedIndexerInstance!.index.localTypeInfo.set('TYPE_A_T', {} as any);

        await cmd();

        expect(_capturedIndexerInstance!.fullIndex).toHaveBeenCalledTimes(1);
        const item = (vscode.window.createStatusBarItem as jest.Mock).mock.results[0].value;
        expect(item.text).toContain('Indexed 1 files');
    });

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-EXT-031: reindexWorkspace error path -> showErrorMessage called with thrown error text', async () => {
        const cmd = await activateAndGetCommand('libjuno.reindexWorkspace');
        const injectedErr = new Error('disk full');
        _capturedIndexerInstance!.fullIndex.mockRejectedValueOnce(injectedErr);
        (vscode.window.showErrorMessage as jest.Mock).mockClear();

        await cmd();

        expect(vscode.window.showErrorMessage).toHaveBeenCalledTimes(1);
        const msg = (vscode.window.showErrorMessage as jest.Mock).mock.calls[0][0] as string;
        expect(msg).toContain('disk full');
    });

});

// ---------------------------------------------------------------------------
// Tests: FileSystemWatcher callbacks (lines 140-141, 146-147, 152)
// ---------------------------------------------------------------------------

describe('FileSystemWatcher callbacks', () => {

    let consoleLogSpy: jest.SpyInstance;

    beforeEach(() => {
        resetMocks();
        consoleLogSpy = jest.spyOn(console, 'log').mockImplementation(() => {});
    });

    afterEach(() => {
        consoleLogSpy.mockRestore();
    });

    async function activateAndGetWatcher(): Promise<any> {
        const context = createMockExtensionContext();
        await activate(context);
        return (vscode.workspace.createFileSystemWatcher as jest.Mock).mock.results[0].value;
    }

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-EXT-032: watcher onDidCreate fires -> reindexFile called with uri fsPath', async () => {
        const watcher = await activateAndGetWatcher();

        watcher._onDidCreateCallback({ fsPath: '/workspace/new.c' });

        expect(_capturedIndexerInstance!.reindexFile).toHaveBeenCalledWith('/workspace/new.c');
        expect(_capturedIndexerInstance!.reindexFile).toHaveBeenCalledTimes(1);
    });

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-EXT-033: watcher onDidChange fires -> reindexFile called with uri fsPath', async () => {
        const watcher = await activateAndGetWatcher();

        watcher._onDidChangeCallback({ fsPath: '/workspace/changed.c' });

        expect(_capturedIndexerInstance!.reindexFile).toHaveBeenCalledWith('/workspace/changed.c');
        expect(_capturedIndexerInstance!.reindexFile).toHaveBeenCalledTimes(1);
    });

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-EXT-034: watcher onDidDelete fires -> removeFile called with uri fsPath', async () => {
        const watcher = await activateAndGetWatcher();

        watcher._onDidDeleteCallback({ fsPath: '/workspace/deleted.c' });

        expect(_capturedIndexerInstance!.removeFile).toHaveBeenCalledWith('/workspace/deleted.c');
        expect(_capturedIndexerInstance!.removeFile).toHaveBeenCalledTimes(1);
    });

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-EXT-037: watcher onDidCreate reindexFile rejects -> catch logs error (line 141)', async () => {
        const watcher = await activateAndGetWatcher();
        _capturedIndexerInstance!.reindexFile.mockRejectedValueOnce(new Error('read error'));

        watcher._onDidCreateCallback({ fsPath: '/workspace/new.c' });
        await Promise.resolve(); // flush .catch() microtask

        expect(consoleLogSpy).toHaveBeenCalledWith(
            '[LibJuno] watcher onDidCreate error:',
            expect.any(Error)
        );
    });

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-EXT-038: watcher onDidChange reindexFile rejects -> catch logs error (line 147)', async () => {
        const watcher = await activateAndGetWatcher();
        _capturedIndexerInstance!.reindexFile.mockRejectedValueOnce(new Error('write error'));

        watcher._onDidChangeCallback({ fsPath: '/workspace/changed.c' });
        await Promise.resolve(); // flush .catch() microtask

        expect(consoleLogSpy).toHaveBeenCalledWith(
            '[LibJuno] watcher onDidChange error:',
            expect.any(Error)
        );
    });

});

// ---------------------------------------------------------------------------
// Tests: MCP server start and writeMcpDiscoveryFile (lines 164, 200)
// ---------------------------------------------------------------------------

describe('MCP server start and writeMcpDiscoveryFile', () => {

    let consoleLogSpy: jest.SpyInstance;

    beforeEach(() => {
        resetMocks();
        // Explicitly configure mcpServerPort > 0 so the MCP branch always runs
        (vscode.workspace.getConfiguration as jest.Mock).mockImplementation((_section?: string) => ({
            get: jest.fn((key: string, defaultVal?: any) => {
                if (key === 'mcpServerPort') { return 6543; }
                return defaultVal;
            }),
        }));
        consoleLogSpy = jest.spyOn(console, 'log').mockImplementation(() => {});
    });

    afterEach(() => {
        consoleLogSpy.mockRestore();
    });

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-EXT-035: mcpServerPort > 0 -> McpServer instantiated and start() called with port', async () => {
        const context = createMockExtensionContext();
        await activate(context);
        await Promise.resolve(); // drain any remaining microtasks

        expect(_capturedMcpInstance).toBeDefined();
        expect(_capturedMcpInstance!.start).toHaveBeenCalledWith(6543);
    });

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-EXT-036: writeMcpDiscoveryFile mkdirSync throws -> catch block logs error (line 200)', async () => {
        // Make mkdirSync throw BEFORE activate() so the spy is active when the .then() microtask fires
        (fs.mkdirSync as jest.Mock).mockImplementationOnce(() => {
            throw new Error('EACCES: permission denied');
        });

        const context = createMockExtensionContext();
        await activate(context);
        await Promise.resolve(); // drain .then() microtask if not yet flushed

        expect(consoleLogSpy).toHaveBeenCalledWith(
            '[LibJuno] Failed to write .libjuno/mcp.json:',
            expect.any(Error)
        );
    });

    // @{"verify": ["REQ-VSCODE-001"]}
    it('TC-EXT-039: start() rejects -> .catch() callback logs MCP error (line 164)', async () => {
        // Override McpServer mock so that start() rejects for this test's instantiation
        const McpServerMock = jest.requireMock('../mcp/mcpServer').McpServer as jest.Mock;
        McpServerMock.mockImplementationOnce(() => {
            _capturedMcpInstance = {
                start: jest.fn().mockRejectedValueOnce(new Error('port in use')),
                stop: jest.fn(),
            };
            return _capturedMcpInstance;
        });

        const context = createMockExtensionContext();
        await activate(context);
        await Promise.resolve(); // drain .catch() microtask

        expect(consoleLogSpy).toHaveBeenCalledWith(
            '[LibJuno] MCP server failed to start:',
            expect.any(Error)
        );
    });

});
