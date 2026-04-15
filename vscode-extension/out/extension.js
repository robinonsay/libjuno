"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    var desc = Object.getOwnPropertyDescriptor(m, k);
    if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
      desc = { enumerable: true, get: function() { return m[k]; } };
    }
    Object.defineProperty(o, k2, desc);
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || (function () {
    var ownKeys = function(o) {
        ownKeys = Object.getOwnPropertyNames || function (o) {
            var ar = [];
            for (var k in o) if (Object.prototype.hasOwnProperty.call(o, k)) ar[ar.length] = k;
            return ar;
        };
        return ownKeys(o);
    };
    return function (mod) {
        if (mod && mod.__esModule) return mod;
        var result = {};
        if (mod != null) for (var k = ownKeys(mod), i = 0; i < k.length; i++) if (k[i] !== "default") __createBinding(result, mod, k[i]);
        __setModuleDefault(result, mod);
        return result;
    };
})();
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = activate;
exports.deactivate = deactivate;
// @{"req": ["REQ-VSCODE-001", "REQ-VSCODE-002", "REQ-VSCODE-007", "REQ-VSCODE-021"]}
const vscode = __importStar(require("vscode"));
const fs = __importStar(require("fs"));
const path = __importStar(require("path"));
const workspaceIndexer_1 = require("./indexer/workspaceIndexer");
const vtableResolver_1 = require("./resolver/vtableResolver");
const failureHandlerResolver_1 = require("./resolver/failureHandlerResolver");
const junoDefinitionProvider_1 = require("./providers/junoDefinitionProvider");
const statusBarHelper_1 = require("./providers/statusBarHelper");
const quickPickHelper_1 = require("./providers/quickPickHelper");
const mcpServer_1 = require("./mcp/mcpServer");
let mcpServer;
// @{"req": ["REQ-VSCODE-001"]}
async function activate(context) {
    // 1. Get workspace root
    const folders = vscode.workspace.workspaceFolders;
    if (!folders || folders.length === 0) {
        console.log('[LibJuno] No workspace folders found; extension inactive.');
        return;
    }
    const workspaceRoot = folders[0].uri.fsPath;
    // 2. Read settings
    const config = vscode.workspace.getConfiguration('libjuno');
    const excludedDirs = config.get('excludedDirectories', ['build', 'deps', '.libjuno']);
    const mcpServerPort = config.get('mcpServerPort', 6543);
    // 3. Create indexer
    const indexer = new workspaceIndexer_1.WorkspaceIndexer(workspaceRoot, excludedDirs);
    // Status bar — created early so progress/errors can show
    const statusBar = new statusBarHelper_1.StatusBarHelper();
    context.subscriptions.push(statusBar);
    // 4. Load cache then full index, both shown with progress notification
    await vscode.window.withProgress({
        location: vscode.ProgressLocation.Notification,
        title: 'LibJuno: Indexing workspace...',
        cancellable: false,
    }, async () => {
        try {
            const cacheLoaded = await indexer.loadFromCache();
            if (!cacheLoaded) {
                await indexer.fullIndex();
            }
        }
        catch (err) {
            console.log('[LibJuno] Indexing error:', err);
            vscode.window.showErrorMessage(`LibJuno: Indexing failed — ${String(err)}`);
        }
    });
    const fileCount = indexer.index.localTypeInfo.size;
    statusBar.showIndexed(fileCount);
    // 5. Create resolvers from the shared index
    const vtableResolver = new vtableResolver_1.VtableResolver(indexer.index);
    const failureHandlerResolver = new failureHandlerResolver_1.FailureHandlerResolver(indexer.index);
    // 6. Register JunoDefinitionProvider for C and C++ (REQ-VSCODE-007, REQ-VSCODE-021)
    context.subscriptions.push(vscode.languages.registerDefinitionProvider([{ language: 'c' }, { language: 'cpp' }], new junoDefinitionProvider_1.JunoDefinitionProvider(vtableResolver, failureHandlerResolver, indexer.index)));
    // 7. Command: libjuno.goToImplementation — direct resolver invocation at cursor
    context.subscriptions.push(vscode.commands.registerCommand('libjuno.goToImplementation', async () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            return;
        }
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
            vscode.window.setStatusBarMessage(`$(warning) LibJuno: ${result.errorMsg ?? 'No implementation found.'}`, 5000);
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
        const selected = await (0, quickPickHelper_1.showImplementationQuickPick)(result.locations);
        if (selected) {
            const pos = new vscode.Position(Math.max(0, selected.line - 1), 0);
            await vscode.window.showTextDocument(vscode.Uri.file(selected.file), {
                selection: new vscode.Range(pos, pos),
            });
        }
    }));
    // 8. Command: libjuno.reindexWorkspace — full re-scan with progress
    context.subscriptions.push(vscode.commands.registerCommand('libjuno.reindexWorkspace', async () => {
        await vscode.window.withProgress({
            location: vscode.ProgressLocation.Notification,
            title: 'LibJuno: Re-indexing workspace...',
            cancellable: false,
        }, async () => {
            try {
                await indexer.fullIndex();
                const count = indexer.index.localTypeInfo.size;
                statusBar.showIndexed(count);
            }
            catch (err) {
                console.log('[LibJuno] Re-index error:', err);
                vscode.window.showErrorMessage(`LibJuno: Re-indexing failed — ${String(err)}`);
            }
        });
    }));
    // 9. FileSystemWatcher for incremental updates (Section 9.3)
    const watcher = vscode.workspace.createFileSystemWatcher('**/*.{c,h,cpp,hpp,hh,cc}');
    watcher.onDidCreate((uri) => {
        indexer.reindexFile(uri.fsPath).catch(err => {
            console.log('[LibJuno] watcher onDidCreate error:', err);
        });
    });
    watcher.onDidChange((uri) => {
        indexer.reindexFile(uri.fsPath).catch(err => {
            console.log('[LibJuno] watcher onDidChange error:', err);
        });
    });
    watcher.onDidDelete((uri) => {
        indexer.removeFile(uri.fsPath);
    });
    context.subscriptions.push(watcher);
    // 10. Start embedded MCP server (Section 7)
    if (mcpServerPort > 0) {
        mcpServer = new mcpServer_1.McpServer(vtableResolver, failureHandlerResolver, indexer.index);
        try {
            mcpServer.start(mcpServerPort);
            writeMcpDiscoveryFile(workspaceRoot, mcpServerPort);
            console.log(`[LibJuno] MCP server started on port ${mcpServerPort}`);
        }
        catch (err) {
            console.log('[LibJuno] MCP server failed to start:', err);
        }
    }
}
function deactivate() {
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
function writeMcpDiscoveryFile(workspaceRoot, port) {
    const libJunoDir = path.join(workspaceRoot, '.libjuno');
    const mcpFile = path.join(libJunoDir, 'mcp.json');
    const content = JSON.stringify({
        mcpServers: {
            libjuno: {
                url: `http://127.0.0.1:${port}/mcp`,
            },
        },
    }, null, 2);
    try {
        fs.mkdirSync(libJunoDir, { recursive: true });
        fs.writeFileSync(mcpFile, content, 'utf8');
    }
    catch (err) {
        console.log('[LibJuno] Failed to write .libjuno/mcp.json:', err);
    }
}
//# sourceMappingURL=extension.js.map