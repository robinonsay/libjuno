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
exports.McpServer = void 0;
// @{"req": ["REQ-VSCODE-017", "REQ-VSCODE-018", "REQ-VSCODE-019", "REQ-VSCODE-020"]}
const http = __importStar(require("http"));
/** Maximum allowed request body size (1 MB) to prevent DoS. */
const MAX_BODY_BYTES = 1048576;
/**
 * Embedded HTTP MCP server that exposes LibJuno resolution tools to AI agent
 * platforms (Section 7). Bound exclusively to 127.0.0.1 for security.
 */
class McpServer {
    constructor(vtableResolver, failureHandlerResolver, index) {
        this.vtableResolver = vtableResolver;
        this.failureHandlerResolver = failureHandlerResolver;
        this.index = index;
    }
    /** Starts the HTTP server on the given port, bound to 127.0.0.1. */
    start(port) {
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
    stop() {
        if (this.server) {
            this.server.close();
            this.server = undefined;
        }
    }
    // -----------------------------------------------------------------------
    // Private: request dispatch
    // -----------------------------------------------------------------------
    async handleRequest(req, res) {
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
            const result = this.vtableResolver.resolve(parsed.file, parsed.line, parsed.column, parsed.lineText);
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
            const result = this.failureHandlerResolver.resolve(parsed.file, parsed.line, parsed.column, parsed.lineText);
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
exports.McpServer = McpServer;
function isResolveParams(obj) {
    if (typeof obj !== 'object' || obj === null || Array.isArray(obj)) {
        return false;
    }
    const r = obj;
    return (typeof r['file'] === 'string' &&
        typeof r['line'] === 'number' &&
        typeof r['column'] === 'number' &&
        typeof r['lineText'] === 'string');
}
function parseBody(raw) {
    try {
        return JSON.parse(raw);
    }
    catch {
        return null;
    }
}
function readBody(req) {
    return new Promise(resolve => {
        const chunks = [];
        let totalBytes = 0;
        req.on('data', (chunk) => {
            totalBytes += chunk.length;
            if (totalBytes > MAX_BODY_BYTES) {
                req.destroy();
                resolve(null);
            }
            else {
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
function sendJson(res, status, body) {
    const payload = JSON.stringify(body);
    res.writeHead(status, {
        'Content-Type': 'application/json',
        'Content-Length': Buffer.byteLength(payload),
    });
    res.end(payload);
}
//# sourceMappingURL=mcpServer.js.map