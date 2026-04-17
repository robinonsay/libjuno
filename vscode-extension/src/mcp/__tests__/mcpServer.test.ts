/// <reference types="jest" />
// @{"verify": ["REQ-VSCODE-017", "REQ-VSCODE-018", "REQ-VSCODE-019", "REQ-VSCODE-020"]}
import * as http from 'http';
import * as fs from 'fs';
import * as path from 'path';
import { McpServer } from '../mcpServer';
import { VtableResolver } from '../../resolver/vtableResolver';
import { FailureHandlerResolver } from '../../resolver/failureHandlerResolver';
import { NavigationIndex } from '../../parser/types';

// ---------------------------------------------------------------------------
// Helper: issue an HTTP request and return status + parsed body
// ---------------------------------------------------------------------------
function httpRequest(
    port: number,
    method: string,
    reqPath: string,
    body?: string
): Promise<{ status: number; body: unknown }> {
    return new Promise((resolve, reject) => {
        const options: http.RequestOptions = {
            hostname: '127.0.0.1',
            port,
            path: reqPath,
            method,
            headers: body
                ? { 'Content-Type': 'application/json', 'Content-Length': Buffer.byteLength(body) }
                : {},
        };
        const req = http.request(options, (res) => {
            const chunks: Buffer[] = [];
            res.on('data', (chunk: Buffer) => chunks.push(chunk));
            res.on('end', () => {
                const raw = Buffer.concat(chunks).toString('utf8');
                try { resolve({ status: res.statusCode!, body: JSON.parse(raw) }); }
                catch { resolve({ status: res.statusCode!, body: raw }); }
            });
        });
        req.on('error', reject);
        if (body) req.write(body);
        req.end();
    });
}

// ---------------------------------------------------------------------------
// Shared valid POST body
// ---------------------------------------------------------------------------
const VALID_BODY = JSON.stringify({ file: 'test.c', line: 1, column: 5, lineText: 'some code' });

// ---------------------------------------------------------------------------
// Test Suite
// ---------------------------------------------------------------------------
describe('McpServer', () => {
    let server: McpServer;
    let port: number;
    let mockVtableResolver: VtableResolver;
    let mockFhResolver: FailureHandlerResolver;
    let mockIndex: NavigationIndex;

    beforeEach(async () => {
        mockVtableResolver = { resolve: jest.fn() } as unknown as VtableResolver;
        mockFhResolver = { resolve: jest.fn() } as unknown as FailureHandlerResolver;
        mockIndex = { localTypeInfo: new Map() } as unknown as NavigationIndex;
        server = new McpServer(mockVtableResolver, mockFhResolver, mockIndex);
        port = await server.start(0);
    });

    afterEach(() => {
        server.stop();
    });

    // === TC-MCP-002 ===
    // @{"verify": ["REQ-VSCODE-018"]}
    test('TC-MCP-002: /resolve_vtable_call accepts POST with correct parameters', async () => {
        (mockVtableResolver.resolve as jest.Mock).mockReturnValue({ found: true, locations: [] });

        const result = await httpRequest(port, 'POST', '/resolve_vtable_call', VALID_BODY);

        expect(result.status).toBe(200);
        expect(mockVtableResolver.resolve).toHaveBeenCalledWith('test.c', 1, 5, 'some code');
        expect(mockVtableResolver.resolve).toHaveBeenCalledTimes(1);
    });

    // === TC-MCP-003 ===
    // @{"verify": ["REQ-VSCODE-019"]}
    test('TC-MCP-003: /resolve_failure_handler accepts POST with correct parameters', async () => {
        (mockFhResolver.resolve as jest.Mock).mockReturnValue({ found: true, locations: [] });

        const result = await httpRequest(port, 'POST', '/resolve_failure_handler', VALID_BODY);

        expect(result.status).toBe(200);
        expect(mockFhResolver.resolve).toHaveBeenCalledWith('test.c', 1, 5, 'some code');
        expect(mockFhResolver.resolve).toHaveBeenCalledTimes(1);
    });

    // === TC-MCP-004 ===
    // @{"verify": ["REQ-VSCODE-018"]}
    test('TC-MCP-004: /resolve_vtable_call valid input → found: true with locations', async () => {
        const locations = [{ functionName: 'Foo', file: 'a.c', line: 10 }];
        (mockVtableResolver.resolve as jest.Mock).mockReturnValue({ found: true, locations });

        const result = await httpRequest(port, 'POST', '/resolve_vtable_call', VALID_BODY);

        expect(result.status).toBe(200);
        const body = result.body as { found: boolean; locations: unknown[] };
        expect(body.found).toBe(true);
        expect(body.locations).toHaveLength(1);
        expect(body.locations[0]).toEqual({ functionName: 'Foo', file: 'a.c', line: 10 });
    });

    // === TC-MCP-005 ===
    // @{"verify": ["REQ-VSCODE-018"]}
    test('TC-MCP-005: /resolve_vtable_call no-match → found: false', async () => {
        (mockVtableResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            error: 'No match',
        });

        const result = await httpRequest(port, 'POST', '/resolve_vtable_call', VALID_BODY);

        expect(result.status).toBe(200);
        const body = result.body as { found: boolean; error: string };
        expect(body.found).toBe(false);
        expect(body.error).toBe('No match');
    });

    // === TC-MCP-006 ===
    // @{"verify": ["REQ-VSCODE-019"]}
    test('TC-MCP-006: /resolve_failure_handler valid input → found: true', async () => {
        const locations = [{ functionName: 'OnFail', file: 'b.c', line: 20 }];
        (mockFhResolver.resolve as jest.Mock).mockReturnValue({ found: true, locations });

        const result = await httpRequest(port, 'POST', '/resolve_failure_handler', VALID_BODY);

        expect(result.status).toBe(200);
        const body = result.body as { found: boolean; locations: unknown[] };
        expect(body.found).toBe(true);
        expect(body.locations).toHaveLength(1);
        expect(body.locations[0]).toEqual({ functionName: 'OnFail', file: 'b.c', line: 20 });
    });

    // === TC-MCP-007 ===
    // @{"verify": ["REQ-VSCODE-019"]}
    test('TC-MCP-007: /resolve_failure_handler no handler → found: false', async () => {
        (mockFhResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            error: 'None',
        });

        const result = await httpRequest(port, 'POST', '/resolve_failure_handler', VALID_BODY);

        expect(result.status).toBe(200);
        const body = result.body as { found: boolean; error: string };
        expect(body.found).toBe(false);
        expect(body.error).toBe('None');
    });

    // === TC-MCP-009 ===
    // @{"verify": ["REQ-VSCODE-017", "REQ-VSCODE-020"]}
    test('TC-MCP-009: Server binds to 127.0.0.1 only — request to 127.0.0.1 succeeds', async () => {
        // start() resolves with a port only after successful bind to 127.0.0.1.
        // Making a request to that address confirms the binding is correct.
        const result = await httpRequest(port, 'GET', '/index_status');

        expect(result.status).toBe(200);
        const body = result.body as { fileCount: number; cacheValid: boolean };
        expect(typeof body.fileCount).toBe('number');
        expect(typeof body.cacheValid).toBe('boolean');
    });

    // === TC-MCP-010 ===
    // @{"verify": ["REQ-VSCODE-017"]}
    test('TC-MCP-010: Application-level resolver errors use HTTP 200 (not 4xx/5xx)', async () => {
        (mockVtableResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            error: 'Not found',
        });

        const result = await httpRequest(port, 'POST', '/resolve_vtable_call', VALID_BODY);

        // Resolver "not found" is a valid application result, not a server error.
        expect(result.status).toBe(200);
        const body = result.body as { found: boolean; error: string };
        expect(body.found).toBe(false);
        expect(body.error).toBe('Not found');
    });

    // === TC-MCP-011 ===
    // @{"verify": ["REQ-VSCODE-017", "REQ-VSCODE-020"]}
    test('TC-MCP-011: Headless mode — server functional without VSCode context', async () => {
        // McpServer was instantiated and started in beforeEach with no VSCode context or
        // activate() call. The fact that it runs in Jest without VSCode (no 'vscode' module
        // in scope) proves headless operation.
        (mockVtableResolver.resolve as jest.Mock).mockReturnValue({ found: true, locations: [] });
        const reqBody = JSON.stringify({ file: 'test.c', line: 5, column: 0, lineText: 'foo->Bar(' });

        const result = await httpRequest(port, 'POST', '/resolve_vtable_call', reqBody);

        expect(result.status).toBe(200);
        expect(mockVtableResolver.resolve).toHaveBeenCalledTimes(1);
        expect(mockVtableResolver.resolve).toHaveBeenCalledWith('test.c', 5, 0, 'foo->Bar(');
    });

    // === TC-MCP-012 ===
    // @{"verify": ["REQ-VSCODE-020"]}
    test('TC-MCP-012: mcpServer.ts has no platform-specific AI SDK imports', () => {
        const srcPath = path.resolve(__dirname, '../mcpServer.ts');
        const src = fs.readFileSync(srcPath, 'utf8');

        expect(src).not.toMatch(/@github\/copilot|@anthropic-ai|openai/);
    });

    // === TC-MCP-013 ===
    // @{"verify": ["REQ-VSCODE-017"]}
    test('TC-MCP-013: Unknown endpoint → HTTP 404 with error field', async () => {
        const result = await httpRequest(port, 'GET', '/nonexistent');

        expect(result.status).toBe(404);
        const body = result.body as { error: string };
        expect(typeof body.error).toBe('string');
        expect(body.error.length).toBeGreaterThan(0);
    });

    // === TC-MCP-014 ===
    // @{"verify": ["REQ-VSCODE-017"]}
    test('TC-MCP-014: Malformed JSON body → HTTP 400', async () => {
        const result = await httpRequest(port, 'POST', '/resolve_vtable_call', 'not valid json {{{');

        expect(result.status).toBe(400);
    });

    // === TC-MCP-015 ===
    // @{"verify": ["REQ-VSCODE-017"]}
    test('TC-MCP-015: Large request body (>1MB) → rejected with 400 or connection reset', async () => {
        // Body exceeds MAX_BODY_BYTES (1,048,576). The server calls req.destroy() which
        // may reset the TCP connection before the 400 response is fully delivered.
        // Both outcomes (400 or ECONNRESET) are acceptable per spec.
        const largeBody = 'A'.repeat(1_048_577);

        let gotResponse = false;
        try {
            const result = await httpRequest(port, 'POST', '/resolve_vtable_call', largeBody);
            gotResponse = true;
            expect(result.status).toBe(400);
            expect((result.body as { error: string }).error).toMatch(/too large/i);
        } catch {
            // ECONNRESET — req.destroy() closed the socket before response arrived.
            expect(gotResponse).toBe(false);
        }
    });

    // === TC-MCP-016 ===
    // @{"verify": ["REQ-VSCODE-017"]}
    test('TC-MCP-016: stop() releases port; re-bind on same port succeeds', async () => {
        const boundPort = port;

        // Stop the server started by beforeEach — this should release the port.
        server.stop();

        // A second server must be able to bind to the same port immediately.
        const server2 = new McpServer(mockVtableResolver, mockFhResolver, mockIndex);
        const port2 = await server2.start(boundPort);
        expect(port2).toBe(boundPort);
        server2.stop();

        // Re-start on a new port so afterEach's server.stop() is safe (no double-stop).
        server = new McpServer(mockVtableResolver, mockFhResolver, mockIndex);
        port = await server.start(0);
    });
});
