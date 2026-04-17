/// <reference types="jest" />
// Manual mock for the 'vscode' module.
// Provides all VSCode API stubs required by the LibJuno extension.
// Do NOT import from the real 'vscode' module here.

// ---------------------------------------------------------------------------
// Minimal type aliases (avoid importing real vscode types)
// ---------------------------------------------------------------------------

type AnyFn = (...args: any[]) => any;

// ---------------------------------------------------------------------------
// Classes
// ---------------------------------------------------------------------------

export class Position {
    constructor(public readonly line: number, public readonly character: number) {}
}

export class Range {
    constructor(
        public readonly start: Position,
        public readonly end: Position
    ) {}
}

export class Location {
    constructor(
        public readonly uri: Uri,
        public readonly range: Position | Range
    ) {}
}

export class Uri {
    readonly fsPath: string;
    readonly scheme: string;
    readonly path: string;

    private constructor(fsPath: string) {
        this.fsPath = fsPath;
        this.scheme = 'file';
        this.path = fsPath;
    }

    static file(filePath: string): Uri {
        return new Uri(filePath);
    }
}

// ---------------------------------------------------------------------------
// Enums
// ---------------------------------------------------------------------------

export const StatusBarAlignment = {
    Left: 1 as const,
    Right: 2 as const,
};

export const ProgressLocation = {
    SourceControl: 1 as const,
    Window: 10 as const,
    Notification: 15 as const,
};

// ---------------------------------------------------------------------------
// Internal helpers — re-created on resetMocks()
// ---------------------------------------------------------------------------

/** Shared FileSystemWatcher mock. Exposes stored callbacks for test use. */
function _createWatcherMock(): any {
    const watcher: any = {
        _onDidCreateCallback: undefined as AnyFn | undefined,
        _onDidChangeCallback: undefined as AnyFn | undefined,
        _onDidDeleteCallback: undefined as AnyFn | undefined,
        onDidCreate: jest.fn((cb: AnyFn) => {
            watcher._onDidCreateCallback = cb;
            return { dispose: jest.fn() };
        }),
        onDidChange: jest.fn((cb: AnyFn) => {
            watcher._onDidChangeCallback = cb;
            return { dispose: jest.fn() };
        }),
        onDidDelete: jest.fn((cb: AnyFn) => {
            watcher._onDidDeleteCallback = cb;
            return { dispose: jest.fn() };
        }),
        dispose: jest.fn(),
    };
    return watcher;
}

function _createStatusBarItemMock(): any {
    return {
        text: '',
        tooltip: '',
        show: jest.fn(),
        hide: jest.fn(),
        dispose: jest.fn(),
    };
}

// ---------------------------------------------------------------------------
// Namespace: vscode.languages
// ---------------------------------------------------------------------------

export const languages = {
    registerDefinitionProvider: jest.fn(() => ({ dispose: jest.fn() })),
};

// ---------------------------------------------------------------------------
// Namespace: vscode.commands
// ---------------------------------------------------------------------------

export const commands = {
    registerCommand: jest.fn(() => ({ dispose: jest.fn() })),
};

// ---------------------------------------------------------------------------
// Namespace: vscode.window
// ---------------------------------------------------------------------------

export const window = {
    showQuickPick: jest.fn().mockResolvedValue(undefined),
    showTextDocument: jest.fn().mockResolvedValue(undefined),
    showErrorMessage: jest.fn().mockResolvedValue(undefined),
    showInformationMessage: jest.fn().mockResolvedValue(undefined),
    setStatusBarMessage: jest.fn(() => ({ dispose: jest.fn() })),
    createStatusBarItem: jest.fn(() => _createStatusBarItemMock()),
    withProgress: jest.fn((options: any, task: AnyFn) =>
        task(
            { report: jest.fn() },
            { isCancellationRequested: false, onCancellationRequested: jest.fn() }
        )
    ),
    activeTextEditor: undefined as any,
};

// ---------------------------------------------------------------------------
// Namespace: vscode.workspace
// ---------------------------------------------------------------------------

export const workspace = {
    workspaceFolders: [
        { uri: { fsPath: '/test-workspace' }, name: 'test', index: 0 },
    ] as any[],
    getConfiguration: jest.fn((_section?: string) => ({
        get: jest.fn((key: string, defaultVal?: any) => defaultVal),
    })),
    createFileSystemWatcher: jest.fn(() => _createWatcherMock()),
};

// ---------------------------------------------------------------------------
// CancellationToken mock
// ---------------------------------------------------------------------------

export const cancellationToken = {
    isCancellationRequested: false,
    onCancellationRequested: jest.fn(),
};

// ---------------------------------------------------------------------------
// ExtensionContext helper
// ---------------------------------------------------------------------------

export function createMockExtensionContext(): any {
    return {
        subscriptions: [],
        extensionPath: '/mock/extension',
        storagePath: '/mock/storage',
        globalStoragePath: '/mock/globalStorage',
        logPath: '/mock/log',
        extensionUri: Uri.file('/mock/extension'),
        globalState: {
            get: jest.fn(),
            update: jest.fn(),
            keys: jest.fn(() => [] as string[]),
        },
        workspaceState: {
            get: jest.fn(),
            update: jest.fn(),
            keys: jest.fn(() => [] as string[]),
        },
        asAbsolutePath: jest.fn((p: string) => `/mock/extension/${p}`),
        extensionMode: 3,
    };
}

// ---------------------------------------------------------------------------
// resetMocks — call in beforeEach to restore all spies to initial state
// ---------------------------------------------------------------------------

export function resetMocks(): void {
    // Clear call history for all tracked jest.fn() instances
    jest.clearAllMocks();
    cancellationToken.isCancellationRequested = false;

    // Restore default implementations that clearAllMocks does not touch
    languages.registerDefinitionProvider.mockReturnValue({ dispose: jest.fn() });
    commands.registerCommand.mockReturnValue({ dispose: jest.fn() });

    window.showQuickPick.mockResolvedValue(undefined);
    window.showTextDocument.mockResolvedValue(undefined);
    window.showErrorMessage.mockResolvedValue(undefined);
    window.showInformationMessage.mockResolvedValue(undefined);
    window.setStatusBarMessage.mockReturnValue({ dispose: jest.fn() });
    window.createStatusBarItem.mockImplementation(() => _createStatusBarItemMock());
    window.withProgress.mockImplementation((options: any, task: AnyFn) =>
        task(
            { report: jest.fn() },
            { isCancellationRequested: false, onCancellationRequested: jest.fn() }
        )
    );
    window.activeTextEditor = undefined;

    workspace.workspaceFolders = [
        { uri: { fsPath: '/test-workspace' }, name: 'test', index: 0 },
    ];
    workspace.getConfiguration.mockImplementation((_section?: string) => ({
        get: jest.fn((key: string, defaultVal?: any) => defaultVal),
    }));
    workspace.createFileSystemWatcher.mockImplementation(() => _createWatcherMock());
}
