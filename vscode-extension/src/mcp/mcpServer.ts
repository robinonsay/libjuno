// @{"req": ["REQ-VSCODE-017", "REQ-VSCODE-018", "REQ-VSCODE-019", "REQ-VSCODE-020"]}
import * as http from 'http';
import { VtableResolver } from '../resolver/vtableResolver';
import { FailureHandlerResolver } from '../resolver/failureHandlerResolver';
import { NavigationIndex } from '../parser/types';

/** Maximum allowed request body size (1 MB) to prevent DoS. */
const MAX_BODY_BYTES = 1_048_576;

/**
 * Embedded HTTP MCP server that exposes LibJuno resolution tools to AI agent
 * platforms (Section 7). Bound exclusively to 127.0.0.1 for security.
 */
export class McpServer {
    private server: http.Server | undefined;

    constructor(
        private readonly vtableResolver: VtableResolver,
        private readonly failureHandlerResolver: FailureHandlerResolver,
        private readonly index: NavigationIndex
    ) {}

    /** Starts the HTTP server on the given port, bound to 127.0.0.1. */
    start(port: number): void {
        this.server = http.createServer((req, res) => {
            this.handleRequest(req, res).catch(err => {
                console.log('[LibJuno] McpServer unhandled error:', err);
                if (!res.headersSent) {
                    sendJson(res, 500, { error: 'Internal server error' });
                }
            });
        });

        this.server.listen(port, '127.0.0.1');
    }

    /** Stops the HTTP server. */
    stop(): void {
        if (this.server) {
            this.server.close();
            this.server = undefined;
        }
    }

    // -----------------------------------------------------------------------
    // Private: request dispatch
    // -----------------------------------------------------------------------

    private async handleRequest(
        req: http.IncomingMessage,
        res: http.ServerResponse
    ): Promise<void> {
        const { method, url } = req;

        if (url === '/resolve_vtable_call' && method === 'POST') {
            const body = await readBody(req);
            if (body === null) {
                sendJson(res, 400, { error: 'Request body too large.' });
                return;
            }
            const parsed = parseBody(body);
            if (!isResolveParams(parsed)) {
                sendJson(res, 400, { error: 'Missing required fields: file, line, column, lineText.' });
                return;
            }
            const result = this.vtableResolver.resolve(
                parsed.file,
                parsed.line,
                parsed.column,
                parsed.lineText
            );
            sendJson(res, 200, result);
            return;
        }

        if (url === '/resolve_failure_handler' && method === 'POST') {
            const body = await readBody(req);
            if (body === null) {
                sendJson(res, 400, { error: 'Request body too large.' });
                return;
            }
            const parsed = parseBody(body);
            if (!isResolveParams(parsed)) {
                sendJson(res, 400, { error: 'Missing required fields: file, line, column, lineText.' });
                return;
            }
            const result = this.failureHandlerResolver.resolve(
                parsed.file,
                parsed.line,
                parsed.column,
                parsed.lineText
            );
            sendJson(res, 200, result);
            return;
        }

        if (url === '/index_status' && (method === 'GET' || method === 'POST')) {
            const fileCount = this.index.localTypeInfo.size;
            sendJson(res, 200, {
                fileCount,
                cacheValid: fileCount > 0,
            });
            return;
        }

        sendJson(res, 404, { error: `Unknown endpoint: ${url}` });
    }
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

interface ResolveParams {
    file: string;
    line: number;
    column: number;
    lineText: string;
}

function isResolveParams(obj: unknown): obj is ResolveParams {
    if (typeof obj !== 'object' || obj === null || Array.isArray(obj)) {
        return false;
    }
    const r = obj as Record<string, unknown>;
    return (
        typeof r['file'] === 'string' &&
        typeof r['line'] === 'number' &&
        typeof r['column'] === 'number' &&
        typeof r['lineText'] === 'string'
    );
}

function parseBody(raw: string): unknown {
    try {
        return JSON.parse(raw);
    } catch {
        return null;
    }
}

function readBody(req: http.IncomingMessage): Promise<string | null> {
    return new Promise(resolve => {
        const chunks: Buffer[] = [];
        let totalBytes = 0;

        req.on('data', (chunk: Buffer) => {
            totalBytes += chunk.length;
            if (totalBytes > MAX_BODY_BYTES) {
                req.destroy();
                resolve(null);
            } else {
                chunks.push(chunk);
            }
        });

        req.on('end', () => {
            resolve(Buffer.concat(chunks).toString('utf8'));
        });

        req.on('error', () => {
            resolve(null);
        });
    });
}

function sendJson(res: http.ServerResponse, status: number, body: unknown): void {
    const payload = JSON.stringify(body);
    res.writeHead(status, {
        'Content-Type': 'application/json',
        'Content-Length': Buffer.byteLength(payload),
    });
    res.end(payload);
}
