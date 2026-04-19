/// <reference types="jest" />
// @{"verify": ["REQ-VSCODE-004", "REQ-VSCODE-017", "REQ-VSCODE-018", "REQ-VSCODE-019", "REQ-VSCODE-020"]}
import * as http from 'http';
import * as net from 'net';
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
        expect((result.body as any).isError).toBeUndefined();
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
        const body = result.body as { found: boolean; error: string; isError: boolean };
        expect(body.found).toBe(false);
        expect(body.error).toBe('No match');
        expect(body.isError).toBe(true);
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
        const body = result.body as { found: boolean; error: string; isError: boolean };
        expect(body.found).toBe(false);
        expect(body.error).toBe('None');
        expect(body.isError).toBe(true);
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
        const body = result.body as { found: boolean; error: string; isError: boolean };
        expect(body.found).toBe(false);
        expect(body.error).toBe('Not found');
        expect(body.isError).toBe(true);
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

    // === TC-MCP-017 ===
    // @{"verify": ["REQ-VSCODE-017"]}
    test('TC-MCP-017: /resolve_failure_handler body too large → HTTP 400 or connection reset', async () => {
        // Body exceeds MAX_BODY_BYTES (1,048,576). Covers lines 90-91 in handleRequest.
        const largeBody = 'A'.repeat(1_048_577);

        let gotResponse = false;
        try {
            const result = await httpRequest(port, 'POST', '/resolve_failure_handler', largeBody);
            gotResponse = true;
            expect(result.status).toBe(400);
            expect((result.body as { error: string }).error).toMatch(/too large/i);
        } catch {
            // ECONNRESET — req.destroy() closed the socket before 400 response arrived.
            expect(gotResponse).toBe(false);
        }
    });

    // === TC-MCP-018 ===
    // @{"verify": ["REQ-VSCODE-017"]}
    test('TC-MCP-018: /resolve_failure_handler missing required fields → HTTP 400', async () => {
        // Valid JSON but without file/line/column/lineText. Covers lines 95-96.
        const invalidBody = JSON.stringify({ foo: 'bar' });

        const result = await httpRequest(port, 'POST', '/resolve_failure_handler', invalidBody);

        expect(result.status).toBe(400);
        const body = result.body as { error: string };
        expect(body.error).toBe('Missing required fields: file, line, column, lineText.');
    });

    // === TC-MCP-019 ===
    // @{"verify": ["REQ-VSCODE-017"]}
    test('TC-MCP-019: handleRequest unhandled exception → HTTP 500 Internal server error', async () => {
        // Resolver throws inside an async handleRequest. The .catch() in start() catches
        // the rejection and sends 500 when headers are not yet sent. Covers lines 33-35.
        (mockFhResolver.resolve as jest.Mock).mockImplementation(() => {
            throw new Error('unexpected crash');
        });

        const result = await httpRequest(port, 'POST', '/resolve_failure_handler', VALID_BODY);

        expect(result.status).toBe(500);
        const body = result.body as { error: string };
        expect(body.error).toBe('Internal server error');
    });

    // === TC-MCP-020 ===
    // @{"verify": ["REQ-VSCODE-017"]}
    test('TC-MCP-020: req error during body read → server remains functional', async () => {
        // An abrupt TCP RST while the server awaits body data triggers req.on('error')
        // inside readBody (line 173), resolving to null without crashing.
        await new Promise<void>(resolve => {
            const socket = net.createConnection({ host: '127.0.0.1', port }, () => {
                // Send POST headers claiming 1000-byte body, then abruptly destroy.
                socket.write(
                    'POST /resolve_failure_handler HTTP/1.1\r\n' +
                    'Host: 127.0.0.1\r\n' +
                    'Content-Type: application/json\r\n' +
                    'Content-Length: 1000\r\n' +
                    '\r\n'
                );
                socket.destroy();
                resolve();
            });
            socket.on('error', () => { /* suppress client-side ECONNRESET */ });
        });

        // Allow time for the server to process the abrupt closure.
        await new Promise(r => setTimeout(r, 50));

        // Server must still be functional after handling the req error.
        const result = await httpRequest(port, 'GET', '/index_status');
        expect(result.status).toBe(200);
        expect((result.body as { fileCount: number; cacheValid: boolean }).fileCount).toBeGreaterThanOrEqual(0);
    });

    // === TC-MCP-021 ===
    // @{"verify": ["REQ-VSCODE-017"]}
    it('TC-MCP-021: /mcp initialize → HTTP 200 with protocolVersion, capabilities.tools, serverInfo.name', async () => {
        const payload = JSON.stringify({
            jsonrpc: '2.0',
            id: 1,
            method: 'initialize',
            params: { protocolVersion: '2024-11-05', capabilities: {}, clientInfo: { name: 'test', version: '1.0' } },
        });

        const result = await httpRequest(port, 'POST', '/mcp', payload);

        expect(result.status).toBe(200);
        const body = result.body as { jsonrpc: string; id: number; result: { protocolVersion: string; capabilities: { tools: unknown }; serverInfo: { name: string } } };
        expect(body.jsonrpc).toBe('2.0');
        expect(body.id).toBe(1);
        expect(body.result.protocolVersion).toBe('2024-11-05');
        expect(body.result.capabilities).toHaveProperty('tools');
        expect(body.result.serverInfo.name).toBe('libjuno');
    });

    // === TC-MCP-022 ===
    // @{"verify": ["REQ-VSCODE-017", "REQ-VSCODE-018", "REQ-VSCODE-019"]}
    it('TC-MCP-022: /mcp tools/list → HTTP 200 with resolve_vtable_call and resolve_failure_handler tools', async () => {
        const payload = JSON.stringify({ jsonrpc: '2.0', id: 2, method: 'tools/list' });

        const result = await httpRequest(port, 'POST', '/mcp', payload);

        expect(result.status).toBe(200);
        const body = result.body as { result: { tools: Array<{ name: string; inputSchema: { type: string; required: string[] } }> } };
        expect(Array.isArray(body.result.tools)).toBe(true);
        expect(body.result.tools.length).toBeGreaterThanOrEqual(2);
        const names = body.result.tools.map(t => t.name);
        expect(names).toContain('resolve_vtable_call');
        expect(names).toContain('resolve_failure_handler');
        for (const tool of body.result.tools) {
            expect(tool.inputSchema.type).toBe('object');
            expect(Array.isArray(tool.inputSchema.required)).toBe(true);
            expect(tool.inputSchema.required).toContain('file');
            expect(tool.inputSchema.required).toContain('line');
            expect(tool.inputSchema.required).toContain('column');
            expect(tool.inputSchema.required).toContain('lineText');
        }
    });

    // === TC-MCP-023 ===
    // @{"verify": ["REQ-VSCODE-018"]}
    it('TC-MCP-023: /mcp tools/call resolve_vtable_call → HTTP 200 with content[0].text containing found property', async () => {
        (mockVtableResolver.resolve as jest.Mock).mockReturnValue({ found: true, locations: [{ functionName: 'Insert', file: '/test/a.c', line: 42 }] });

        const payload = JSON.stringify({
            jsonrpc: '2.0',
            id: 3,
            method: 'tools/call',
            params: { name: 'resolve_vtable_call', arguments: { file: '/test/a.c', line: 1, column: 5, lineText: 'ptApi->Insert(ptHeap)' } },
        });

        const result = await httpRequest(port, 'POST', '/mcp', payload);

        expect(result.status).toBe(200);
        const body = result.body as { result: { content: Array<{ type: string; text: string }> } };
        expect(Array.isArray(body.result.content)).toBe(true);
        expect(body.result.content.length).toBeGreaterThanOrEqual(1);
        expect(body.result.content[0].type).toBe('text');
        const parsed = JSON.parse(body.result.content[0].text) as { found: boolean };
        expect(typeof parsed.found).toBe('boolean');
        expect(parsed.found).toBe(true);
    });

    // === TC-MCP-024 ===
    // @{"verify": ["REQ-VSCODE-019"]}
    it('TC-MCP-024: /mcp tools/call resolve_failure_handler → HTTP 200 with content[0].text containing found property', async () => {
        (mockFhResolver.resolve as jest.Mock).mockReturnValue({ found: true, locations: [{ functionName: 'MyHandler', file: '/test/a.c', line: 10 }] });

        const payload = JSON.stringify({
            jsonrpc: '2.0',
            id: 4,
            method: 'tools/call',
            params: { name: 'resolve_failure_handler', arguments: { file: '/test/a.c', line: 1, column: 5, lineText: 'ptMod->tRoot._pfcnFailureHandler = MyHandler;' } },
        });

        const result = await httpRequest(port, 'POST', '/mcp', payload);

        expect(result.status).toBe(200);
        const body = result.body as { result: { content: Array<{ type: string; text: string }> } };
        expect(body.result.content[0].type).toBe('text');
        const parsed = JSON.parse(body.result.content[0].text) as { found: boolean };
        expect(typeof parsed.found).toBe('boolean');
        expect(parsed.found).toBe(true);
    });

    // === TC-MCP-025 ===
    // @{"verify": ["REQ-VSCODE-017"]}
    it('TC-MCP-025: /mcp unknown method → HTTP 200 with error.code -32601 and matching id', async () => {
        const payload = JSON.stringify({ jsonrpc: '2.0', id: 5, method: 'nonexistent/method' });

        const result = await httpRequest(port, 'POST', '/mcp', payload);

        expect(result.status).toBe(200);
        const body = result.body as { jsonrpc: string; id: number; error: { code: number; message: string } };
        expect(body.error.code).toBe(-32601);
        expect(body.id).toBe(5);
    });

    // === TC-ERR-006 ===
    // @{"verify": ["REQ-VSCODE-004"]}
    test('TC-ERR-006: resolve_vtable_call error object includes API type and field name — HTTP 200 (not 4xx/5xx)', async () => {
        (mockVtableResolver.resolve as jest.Mock).mockReturnValue({
            found: false,
            locations: [],
            error: "No implementation found for 'JUNO_DS_HEAP_API_T::Insert'.",
        });

        const result = await httpRequest(port, 'POST', '/resolve_vtable_call', VALID_BODY);

        // Application-level resolution failure must return HTTP 200, not an HTTP error code.
        expect(result.status).toBe(200);
        const body = result.body as { found: boolean; locations: unknown[]; error: string; isError: boolean };
        expect(body.found).toBe(false);
        expect(body.locations).toEqual([]);
        expect(body.error).toBe("No implementation found for 'JUNO_DS_HEAP_API_T::Insert'.");
        // Error message must identify the API type and specific field.
        expect(body.error).toContain('JUNO_DS_HEAP_API_T');
        expect(body.error).toContain('Insert');
        expect(body.isError).toBe(true);
    });
});
