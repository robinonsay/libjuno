"use strict";
/**
 * start-mcp-server.ts
 *
 * Standalone (headless) MCP server launcher for LibJuno.
 * Builds the workspace navigation index and starts the MCP HTTP server
 * without requiring VSCode to be running.
 *
 * Usage:
 *   npx ts-node scripts/start-mcp-server.ts [--port PORT] [--root ROOT]
 *
 * Options:
 *   --port PORT   Port to listen on (default: 6543)
 *   --root ROOT   Workspace root directory to index (default: process.cwd())
 *
 * The server exposes the same MCP endpoint as the VSCode extension:
 *   http://127.0.0.1:<PORT>/mcp
 *
 * Sub-agents connect via the URL registered in .claude/settings.json.
 * Keep this process running for the duration of a dev session.
 */
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
const path = __importStar(require("path"));
const workspaceIndexer_1 = require("../src/indexer/workspaceIndexer");
const vtableResolver_1 = require("../src/resolver/vtableResolver");
const failureHandlerResolver_1 = require("../src/resolver/failureHandlerResolver");
const mcpServer_1 = require("../src/mcp/mcpServer");
// ---------------------------------------------------------------------------
// Argument parsing
// ---------------------------------------------------------------------------
function parseArgs(argv) {
    let port = 6543;
    let root = process.cwd();
    for (let i = 0; i < argv.length; i++) {
        if (argv[i] === '--port' && i + 1 < argv.length) {
            const parsed = parseInt(argv[i + 1], 10);
            if (!isNaN(parsed) && parsed > 0 && parsed <= 65535) {
                port = parsed;
            }
            else {
                console.error(`[LibJuno] Invalid port value: ${argv[i + 1]}. Using default 6543.`);
            }
            i++;
        }
        else if (argv[i] === '--root' && i + 1 < argv.length) {
            root = path.resolve(argv[i + 1]);
            i++;
        }
    }
    return { port, root };
}
// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
async function main() {
    // Skip the first two entries: node executable and script path.
    const { port, root } = parseArgs(process.argv.slice(2));
    console.log(`[LibJuno] Starting headless MCP server.`);
    console.log(`[LibJuno] Workspace root: ${root}`);
    // 1. Build the workspace navigation index.
    const excludedDirs = ['build', 'deps', '.libjuno'];
    const indexer = new workspaceIndexer_1.WorkspaceIndexer(root, excludedDirs);
    console.log(`[LibJuno] Indexing workspace...`);
    await indexer.fullIndex();
    const fileCount = indexer.index.localTypeInfo.size;
    console.log(`[LibJuno] Indexed ${fileCount} files.`);
    // 2. Create resolvers from the shared index.
    const vtableResolver = new vtableResolver_1.VtableResolver(indexer.index);
    const failureHandlerResolver = new failureHandlerResolver_1.FailureHandlerResolver(indexer.index);
    // 3. Start the MCP server.
    const server = new mcpServer_1.McpServer(vtableResolver, failureHandlerResolver, indexer.index);
    const actualPort = await server.start(port);
    console.log(`[LibJuno] MCP server running on http://127.0.0.1:${actualPort}/mcp`);
    // 4. Keep the process alive indefinitely.
    // Handle graceful shutdown on SIGINT / SIGTERM.
    const shutdown = () => {
        console.log('\n[LibJuno] Shutting down MCP server...');
        server.stop();
        indexer.dispose();
        process.exit(0);
    };
    process.on('SIGINT', shutdown);
    process.on('SIGTERM', shutdown);
}
main().catch((err) => {
    console.error('[LibJuno] Fatal error:', err instanceof Error ? err.stack ?? String(err) : String(err));
    process.exit(1);
});
//# sourceMappingURL=start-mcp-server.js.map