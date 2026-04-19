// @{"req": ["REQ-VSCODE-027", "REQ-VSCODE-028", "REQ-VSCODE-029", "REQ-VSCODE-030", "REQ-VSCODE-031", "REQ-VSCODE-032"]}
import * as fs from 'fs';
import * as vscode from 'vscode';
import { VtableResolver } from '../resolver/vtableResolver';
import { NavigationIndex, FunctionDefinitionRecord } from '../parser/types';
import { StatusBarHelper } from './statusBarHelper';

/**
 * A single node in the vtable resolution trace tree.
 */
interface TraceNode {
    type:   'call-site' | 'composition-root' | 'init-impl' | 'implementation';
    label:  string;
    file:   string;
    line:   number;
    detail: string;
}

/**
 * Full resolution trace from call site through composition root to
 * implementation (REQ-VSCODE-027). When compRootFile is resolved on the
 * ConcreteLocation, an optional initImpl node (the Init function body where
 * the vtable pointer is wired) is inserted between compositionRoot and
 * implementation, producing a 4-node chain.
 */
interface VtableTrace {
    callSite:        TraceNode;
    compositionRoot: TraceNode;
    initImpl?:       TraceNode;  // present only when both compRootFile and initCallFile are resolved
    implementation:  TraceNode;
}

/**
 * Escapes HTML special characters to prevent XSS when inserting user data
 * into the webview HTML template (§11.6).
 */
function escHtml(s: string): string {
    return s
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;');
}

/**
 * Generates a nonce string for the Content-Security-Policy inline script
 * allowlist (§11.6).
 */
function generateNonce(): string {
    return Buffer.from(Math.random().toString())
        .toString('base64')
        .replace(/[^a-zA-Z0-9]/g, '')
        .slice(0, 16);
}

/**
 * Builds the HTML for a single composition-root (+ optional init-impl) +
 * implementation subtree.
 */
function buildSubtreeHtml(compositionRoot: TraceNode, implementation: TraceNode, initImpl?: TraceNode): string {
    const initImplHtml = initImpl ? `
  <div class="trace-connector">│</div>
  <div class="trace-node init-impl">
    <span class="node-icon">⚙</span>
    <span class="node-label">Initialization Implementation</span>
    <div class="node-detail">
      <a href="#" data-file="${escHtml(initImpl.file)}" data-line="${initImpl.line}">${escHtml(initImpl.file)}:${initImpl.line}</a>
      <code>${escHtml(initImpl.detail)}</code>
    </div>
  </div>` : '';

    return `
  <div class="trace-connector">│</div>
  <div class="trace-node composition-root">
    <span class="node-icon">🔗</span>
    <span class="node-label">Composition Root</span>
    <div class="node-detail">
      <a href="#" data-file="${escHtml(compositionRoot.file)}" data-line="${compositionRoot.line}">${escHtml(compositionRoot.file)}:${compositionRoot.line}</a>
      <code>${escHtml(compositionRoot.detail)}</code>
    </div>
  </div>${initImplHtml}
  <div class="trace-connector">│</div>
  <div class="trace-node implementation">
    <span class="node-icon">⚡</span>
    <span class="node-label">Implementation</span>
    <div class="node-detail">
      <a href="#" data-file="${escHtml(implementation.file)}" data-line="${implementation.line}">${escHtml(implementation.file)}:${implementation.line}</a>
      <code>${escHtml(implementation.detail)}</code>
    </div>
  </div>`;
}

/**
 * Generates the full HTML document for the WebviewPanel (§11.3, §11.4, §11.6).
 */
function generateHtml(traces: VtableTrace[], nonce: string): string {
    const callSite = traces[0].callSite;

    let subtreesHtml: string;
    if (traces.length === 1) {
        subtreesHtml = buildSubtreeHtml(traces[0].compositionRoot, traces[0].implementation, traces[0].initImpl);
    } else {
        subtreesHtml = traces.map((trace, idx) => `
  <details open>
    <summary>Result ${idx + 1}: ${escHtml(trace.implementation.label)}</summary>${buildSubtreeHtml(trace.compositionRoot, trace.implementation, trace.initImpl)}
  </details>`).join('');
    }

    return `<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta http-equiv="Content-Security-Policy" content="default-src 'none'; script-src 'nonce-${nonce}'; style-src 'unsafe-inline';">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Vtable Resolution Trace</title>
  <style>
    body { font-family: var(--vscode-font-family, monospace); padding: 16px; }
    .trace-tree { margin: 0; padding: 0; }
    .trace-node { border: 1px solid var(--vscode-panel-border, #444); border-radius: 4px; padding: 8px 12px; margin: 4px 0; }
    .trace-node.call-site { border-left: 3px solid #569cd6; }
    .trace-node.composition-root { border-left: 3px solid #ce9178; }
    .trace-node.init-impl { border-left: 3px solid #dcdcaa; }
    .trace-node.implementation { border-left: 3px solid #4ec9b0; }
    .node-icon { margin-right: 6px; }
    .node-label { font-weight: bold; margin-right: 8px; }
    .node-detail { margin-top: 6px; font-size: 0.9em; }
    .node-detail a { display: block; color: var(--vscode-textLink-foreground, #4fc1ff); text-decoration: none; cursor: pointer; }
    .node-detail a:hover { text-decoration: underline; }
    .node-detail code { display: block; margin-top: 2px; color: var(--vscode-editor-foreground, #d4d4d4); }
    .trace-connector { color: var(--vscode-descriptionForeground, #888); padding: 2px 16px; }
    details { margin: 8px 0; }
    summary { cursor: pointer; font-weight: bold; padding: 4px 0; }
  </style>
</head>
<body>
  <h2>Vtable Resolution Trace</h2>
  <div class="trace-tree">
    <div class="trace-node call-site">
      <span class="node-icon">📍</span>
      <span class="node-label">Call Site</span>
      <div class="node-detail">
        <a href="#" data-file="${escHtml(callSite.file)}" data-line="${callSite.line}">${escHtml(callSite.file)}:${callSite.line}</a>
        <code>${escHtml(callSite.detail)}</code>
      </div>
    </div>${subtreesHtml}
  </div>
  <script nonce="${nonce}">
    (function() {
      const vscode = acquireVsCodeApi();
      document.addEventListener('click', function(e) {
        const target = e.target && e.target.closest('a[data-file]');
        if (!target) { return; }
        e.preventDefault();
        const file = target.getAttribute('data-file');
        const line = parseInt(target.getAttribute('data-line') || '1', 10);
        vscode.postMessage({ type: 'navigate', file: file, line: line });
      });
    })();
  </script>
</body>
</html>`;
}

/**
 * Provides the vtable resolution trace view (REQ-VSCODE-027).
 *
 * Builds an up-to-4-node tree (call site → composition root → [init impl] →
 * implementation) from the VtableResolver output and renders it in a VSCode
 * WebviewPanel. The init-impl node is only present when compRootFile is
 * resolved on the ConcreteLocation. File link clicks in the panel navigate
 * the editor to the referenced location.
 *
 * The `createWebviewPanel` and `showTextDocument` functions are injected via the
 * constructor to allow Jest testing without a live VSCode host.
 */
export class VtableTraceProvider {
    constructor(
        private readonly vtableResolver: VtableResolver,
        private readonly index: NavigationIndex,
        private readonly statusBar: StatusBarHelper,
        private readonly createWebviewPanel: typeof vscode.window.createWebviewPanel,
        private readonly showTextDocument: typeof vscode.window.showTextDocument
    ) {}

    // @{"req": ["REQ-VSCODE-027", "REQ-VSCODE-028", "REQ-VSCODE-029", "REQ-VSCODE-030", "REQ-VSCODE-031", "REQ-VSCODE-032"]}
    /**
     * Resolves the vtable call at the given cursor position and shows the full
     * resolution trace (up to 4 nodes) in a WebviewPanel.
     *
     * @param file     Absolute path of the current C source file.
     * @param line     1-based line number of the cursor.
     * @param column   0-based column number of the cursor.
     * @param lineText Full text of the source line at the cursor position.
     */
    showTrace(file: string, line: number, column: number, lineText: string): void {
        // Step 1 — Resolve the vtable call (§11.2 Step 1).
        const result = this.vtableResolver.resolve(file, line, column, lineText);
        if (!result.found) {
            this.statusBar.showError(result.errorMsg ?? 'Could not resolve vtable call.');
            return;
        }

        // Step 2 — Build call site node (REQ-VSCODE-030).
        const callSite: TraceNode = {
            type:   'call-site',
            label:  lineText.trim(),
            file,
            line,
            detail: lineText.trim(),
        };

        // Steps 3–4 — Build one VtableTrace per resolved location.
        const traces: VtableTrace[] = result.locations.map(location => {
            // Step 3 — Composition root node (REQ-VSCODE-031, REQ-VSCODE-036, REQ-VSCODE-037).
            // When compRootFile is set, use it as the true composition root (caller of Init, e.g.
            // main.c). Otherwise fall back to initCallFile ?? assignmentFile as before.
            const compRootFile = location.compRootFile ?? location.initCallFile ?? location.assignmentFile ?? 'unknown';
            const compRootLine = location.compRootLine ?? location.initCallLine ?? location.assignmentLine ?? 0;
            const compositionRoot: TraceNode = {
                type:   'composition-root',
                label:  location.functionName,
                file:   compRootFile,
                line:   compRootLine,
                detail: this.readSourceLine(compRootFile, compRootLine),
            };

            // Step 3b — Initialization implementation node (REQ-VSCODE-037).
            // Only present when compRootFile is resolved and initCallFile is known, and they
            // differ (i.e., the Init function body is in a different location than the caller).
            let initImpl: TraceNode | undefined;
            if (location.compRootFile && location.initCallFile) {
                const initImplFile = location.initCallFile;
                const initImplLine = location.initCallLine ?? 0;
                initImpl = {
                    type:   'init-impl',
                    label:  location.functionName,
                    file:   initImplFile,
                    line:   initImplLine,
                    detail: this.readSourceLine(initImplFile, initImplLine),
                };
            }

            // Step 4 — Implementation node (REQ-VSCODE-032).
            // Prefer FunctionDefinitionRecord.signature when available.
            const defRecords: FunctionDefinitionRecord[] | undefined =
                this.index.functionDefinitions.get(location.functionName);
            const signature = defRecords && defRecords.length > 0
                ? ((defRecords[0] as FunctionDefinitionRecord & { signature?: string }).signature ?? location.functionName)
                : location.functionName;

            const implementation: TraceNode = {
                type:   'implementation',
                label:  location.functionName,
                file:   location.file,
                line:   location.line,
                detail: signature,
            };

            return { callSite, compositionRoot, initImpl, implementation };
        });

        // Step 5 — Display the WebviewPanel (§11.2 Step 5, §11.3).
        const panel = this.createWebviewPanel(
            'libjunoVtableTrace',
            'Vtable Resolution Trace',
            vscode.ViewColumn.Beside,
            {
                enableScripts: true,
                retainContextWhenHidden: true,
            }
        );

        const nonce = generateNonce();
        panel.webview.html = generateHtml(traces, nonce);

        // Register message handler for file navigation clicks.
        panel.webview.onDidReceiveMessage((message: { type: string; file: string; line: number }) => {
            if (message.type === 'navigate') {
                this.showTextDocument(
                    vscode.Uri.file(message.file),
                    { selection: new vscode.Range(Math.max(0, message.line - 1), 0, Math.max(0, message.line - 1), 0) }
                );
            }
        });
    }

    /**
     * Reads the text of a single source line from disk (1-based line number).
     * Returns an empty string if the file cannot be read or the line does not exist.
     *
     * @param file Absolute path to the source file.
     * @param line 1-based line number to read.
     */
    private readSourceLine(file: string, line: number): string {
        try {
            const text = fs.readFileSync(file, 'utf8');
            const lines = text.split('\n');
            return (lines[line - 1] ?? '').trim();
        } catch {
            return '';
        }
    }
}
