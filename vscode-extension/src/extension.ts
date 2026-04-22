// @{"req": ["REQ-VSCODE-001", "REQ-VSCODE-002", "REQ-VSCODE-007", "REQ-VSCODE-021"]}
import * as vscode from 'vscode';
import * as fs from 'fs';
import * as path from 'path';
import { WorkspaceIndexer } from './indexer/workspaceIndexer';
import { VtableResolver } from './resolver/vtableResolver';
import { FailureHandlerResolver } from './resolver/failureHandlerResolver';
import { JunoDefinitionProvider } from './providers/junoDefinitionProvider';
import { StatusBarHelper } from './providers/statusBarHelper';
import { showImplementationQuickPick } from './providers/quickPickHelper';
import { McpServer } from './mcp/mcpServer';
import { VtableTraceProvider } from './providers/vtableTraceProvider';

let mcpServer: McpServer | undefined;

// @{"req": ["REQ-VSCODE-001"]}
export async function activate(
    context: vscode.ExtensionContext,
    indexerFactory?: (root: string, excludes: string[]) => WorkspaceIndexer
): Promise<void> {
    const outputChannel = vscode.window.createOutputChannel('LibJuno');
    context.subscriptions.push(outputChannel);
    const log = (msg: string) => outputChannel.appendLine(msg);

    // 1. Get workspace root
    const folders = vscode.workspace.workspaceFolders;
    if (!folders || folders.length === 0) {
        log('[LibJuno] No workspace folders found; extension inactive.');
        return;
    }
    const workspaceRoot = folders[0].uri.fsPath;

    // 2. Read settings
    const config = vscode.workspace.getConfiguration('libjuno');
    const excludedDirs = config.get<string[]>('excludedDirectories', ['build', 'deps', '.libjuno']);
    const mcpServerPort = config.get<number>('mcpServerPort', 6543);

    // 3. Create indexer
    const factory = indexerFactory ?? ((r: string, ex: string[]) => new WorkspaceIndexer(r, ex));
    const indexer = factory(workspaceRoot, excludedDirs);

    // Status bar — created early so progress/errors can show
    const statusBar = new StatusBarHelper();
    context.subscriptions.push(statusBar);

    // 4. Load cache then full index, both shown with progress notification
    await vscode.window.withProgress(
        {
            location: vscode.ProgressLocation.Notification,
            title: 'LibJuno: Indexing workspace...',
            cancellable: false,
        },
        async () => {
            try {
                const cacheLoaded = await indexer.loadFromCache();
                if (!cacheLoaded) {
                    await indexer.fullIndex();
                }
            } catch (err) {
                log('[LibJuno] Indexing error: ' + (err instanceof Error ? (err.stack ?? String(err)) : String(err)));
                vscode.window.showErrorMessage(`LibJuno: Indexing failed — ${err instanceof Error ? (err.stack ?? String(err)) : String(err)}`);
            }
        }
    );

    const fileCount = indexer.index.localTypeInfo.size;
    statusBar.showIndexed(fileCount);

    // 5. Create resolvers from the shared index
    const vtableResolver = new VtableResolver(indexer.index);
    const failureHandlerResolver = new FailureHandlerResolver(indexer.index);

    // 6. Register JunoDefinitionProvider for C and C++ (REQ-VSCODE-007, REQ-VSCODE-021)
    context.subscriptions.push(
        vscode.languages.registerDefinitionProvider(
            [{ language: 'c' }, { language: 'cpp' }],
            new JunoDefinitionProvider(vtableResolver, failureHandlerResolver, indexer.index, statusBar, log)
        )
    );

    // 7. Command: libjuno.goToImplementation — direct resolver invocation at cursor
    context.subscriptions.push(
        vscode.commands.registerCommand('libjuno.goToImplementation', async () => {
            const editor = vscode.window.activeTextEditor;
            if (!editor) { return; }

            const position = editor.selection.active;
            const lineText = editor.document.lineAt(position.line).text;
            const file = editor.document.uri.fsPath;
            const line = position.line + 1; // convert to 1-based
            const column = position.character;

            let result = failureHandlerResolver.resolve(file, line, column, lineText);
            if (!result.found) {
                result = vtableResolver.resolve(file, line, column, lineText);
            }

            if (!result.found || result.locations.length === 0) {
                statusBar.showError(result.errorMsg ?? 'No implementation found.');
                return;
            }

            if (result.locations.length === 1) {
                const loc = result.locations[0];
                const pos = new vscode.Position(Math.max(0, loc.line - 1), 0);
                await vscode.window.showTextDocument(vscode.Uri.file(loc.file), {
                    selection: new vscode.Range(pos, pos),
                });
                return;
            }

            const selected = await showImplementationQuickPick(result.locations);
            if (selected) {
                const pos = new vscode.Position(Math.max(0, selected.line - 1), 0);
                await vscode.window.showTextDocument(vscode.Uri.file(selected.file), {
                    selection: new vscode.Range(pos, pos),
                });
            }
        })
    );

    // 8. Command: libjuno.showVtableTrace — show resolution trace in webview
    const traceProvider = new VtableTraceProvider(
        vtableResolver,
        indexer.index,
        statusBar,
        vscode.window.createWebviewPanel,
        vscode.window.showTextDocument
    );

    // @{"req": ["REQ-VSCODE-028", "REQ-VSCODE-029"]}
    context.subscriptions.push(
        vscode.commands.registerCommand('libjuno.showVtableTrace', () => {
            const editor = vscode.window.activeTextEditor;
            if (!editor) { return; }
            const pos = editor.selection.active;
            const lineText = editor.document.lineAt(pos.line).text;
            traceProvider.showTrace(
                editor.document.uri.fsPath,
                pos.line + 1,   // Convert 0-based to 1-based
                pos.character,
                lineText
            );
        })
    );

    // 9. Command: libjuno.reindexWorkspace — full re-scan with progress
    context.subscriptions.push(
        vscode.commands.registerCommand('libjuno.reindexWorkspace', async () => {
            await vscode.window.withProgress(
                {
                    location: vscode.ProgressLocation.Notification,
                    title: 'LibJuno: Re-indexing workspace...',
                    cancellable: false,
                },
                async () => {
                    try {
                        await indexer.fullIndex();
                        const count = indexer.index.localTypeInfo.size;
                        statusBar.showIndexed(count);
                    } catch (err) {
                        log('[LibJuno] Re-index error: ' + (err instanceof Error ? (err.stack ?? String(err)) : String(err)));
                        vscode.window.showErrorMessage(`LibJuno: Re-indexing failed — ${err instanceof Error ? (err.stack ?? String(err)) : String(err)}`);
                    }
                }
            );
        })
    );

    // 10. FileSystemWatcher for incremental updates (Section 9.3)
    const watcher = vscode.workspace.createFileSystemWatcher('**/*.{c,h,cpp,hpp,hh,cc}');

    // @{"req": ["REQ-VSCODE-042"]}
    watcher.onDidCreate((uri: vscode.Uri) => {
        indexer.reindexFile(uri.fsPath).catch(err => {
            log('[LibJuno] watcher onDidCreate error: ' + (err instanceof Error ? (err.stack ?? String(err)) : String(err)));
        });
    });

    watcher.onDidChange((uri: vscode.Uri) => {
        indexer.reindexFile(uri.fsPath).catch(err => {
            log('[LibJuno] watcher onDidChange error: ' + (err instanceof Error ? (err.stack ?? String(err)) : String(err)));
        });
    });

    // @{"req": ["REQ-VSCODE-043"]}
    watcher.onDidDelete((uri: vscode.Uri) => {
        indexer.removeFile(uri.fsPath);
    });

    context.subscriptions.push(watcher);

    // 11. Start embedded MCP server (Section 7)
    if (mcpServerPort > 0) {
        mcpServer = new McpServer(vtableResolver, failureHandlerResolver, indexer.index, log);
        mcpServer.start(mcpServerPort).then((actualPort) => {
            writeMcpDiscoveryFile(workspaceRoot, actualPort, log);
            log(`[LibJuno] MCP server started on port ${actualPort}`);
        }).catch((err) => {
            log('[LibJuno] MCP server failed to start: ' + (err instanceof Error ? (err.stack ?? String(err)) : String(err)));
        });
    }
}

export function deactivate(): void {
    if (mcpServer) {
        mcpServer.stop();
        mcpServer = undefined;
    }
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

/**
 * Writes the MCP discovery file to `.libjuno/mcp.json` so that AI agent
 * platforms can discover the embedded server (Section 7.4).
 */
function writeMcpDiscoveryFile(workspaceRoot: string, port: number, log: (msg: string) => void): void {
    const libJunoDir = path.join(workspaceRoot, '.libjuno');
    const mcpFile = path.join(libJunoDir, 'mcp.json');
    const content = JSON.stringify(
        {
            mcpServers: {
                libjuno: {
                    url: `http://127.0.0.1:${port}/mcp`,
                },
            },
        },
        null,
        2
    );
    try {
        fs.mkdirSync(libJunoDir, { recursive: true });
        fs.writeFileSync(mcpFile, content, 'utf8');
    } catch (err) {
        log('[LibJuno] Failed to write .libjuno/mcp.json: ' + (err instanceof Error ? (err.stack ?? String(err)) : String(err)));
    }
}
