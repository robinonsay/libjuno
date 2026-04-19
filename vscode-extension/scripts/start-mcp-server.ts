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

import * as path from 'path';
import { WorkspaceIndexer } from '../src/indexer/workspaceIndexer';
import { VtableResolver } from '../src/resolver/vtableResolver';
import { FailureHandlerResolver } from '../src/resolver/failureHandlerResolver';
import { McpServer } from '../src/mcp/mcpServer';

// ---------------------------------------------------------------------------
// Argument parsing
// ---------------------------------------------------------------------------

function parseArgs(argv: string[]): { port: number; root: string } {
    let port = 6543;
    let root = process.cwd();

    for (let i = 0; i < argv.length; i++) {
        if (argv[i] === '--port' && i + 1 < argv.length) {
            const parsed = parseInt(argv[i + 1], 10);
            if (!isNaN(parsed) && parsed > 0 && parsed <= 65535) {
                port = parsed;
            } else {
                console.error(`[LibJuno] Invalid port value: ${argv[i + 1]}. Using default 6543.`);
            }
            i++;
        } else if (argv[i] === '--root' && i + 1 < argv.length) {
            root = path.resolve(argv[i + 1]);
            i++;
        }
    }

    return { port, root };
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

async function main(): Promise<void> {
    // Skip the first two entries: node executable and script path.
    const { port, root } = parseArgs(process.argv.slice(2));

    console.log(`[LibJuno] Starting headless MCP server.`);
    console.log(`[LibJuno] Workspace root: ${root}`);

    // 1. Build the workspace navigation index.
    const excludedDirs = ['build', 'deps', '.libjuno'];
    const indexer = new WorkspaceIndexer(root, excludedDirs);

    console.log(`[LibJuno] Indexing workspace...`);
    await indexer.fullIndex();

    const fileCount = indexer.index.localTypeInfo.size;
    console.log(`[LibJuno] Indexed ${fileCount} files.`);

    // 2. Create resolvers from the shared index.
    const vtableResolver = new VtableResolver(indexer.index);
    const failureHandlerResolver = new FailureHandlerResolver(indexer.index);

    // 3. Start the MCP server.
    const server = new McpServer(vtableResolver, failureHandlerResolver, indexer.index);
    const actualPort = await server.start(port);

    console.log(`[LibJuno] MCP server running on http://127.0.0.1:${actualPort}/mcp`);

    // 4. Keep the process alive indefinitely.
    // Handle graceful shutdown on SIGINT / SIGTERM.
    const shutdown = (): void => {
        console.log('\n[LibJuno] Shutting down MCP server...');
        server.stop();
        indexer.dispose();
        process.exit(0);
    };

    process.on('SIGINT', shutdown);
    process.on('SIGTERM', shutdown);
}

main().catch((err: unknown) => {
    console.error('[LibJuno] Fatal error:', err instanceof Error ? err.stack ?? String(err) : String(err));
    process.exit(1);
});
