/// <reference types="jest" />
// @{"verify": ["REQ-VSCODE-027", "REQ-VSCODE-028", "REQ-VSCODE-029", "REQ-VSCODE-030", "REQ-VSCODE-031", "REQ-VSCODE-032"]}

import * as path from 'path';
import * as fs from 'fs';
import * as vscode from 'vscode';
import { VtableTraceProvider } from '../vtableTraceProvider';
import { NavigationIndex, FunctionDefinitionRecord } from '../../parser/types';
import { VtableResolver } from '../../resolver/vtableResolver';
import { StatusBarHelper } from '../statusBarHelper';

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

function createMinimalIndex(
    functionDefinitions?: Map<string, FunctionDefinitionRecord[]>
): NavigationIndex {
    return {
        moduleRoots: new Map(),
        traitRoots: new Map(),
        derivationChain: new Map(),
        apiStructFields: new Map(),
        vtableAssignments: new Map(),
        failureHandlerAssignments: new Map(),
        apiMemberRegistry: new Map(),
        functionDefinitions: functionDefinitions ?? new Map(),
        localTypeInfo: new Map(),
        initCallIndex: new Map(),
    };
}

function createFakePanel() {
    return {
        webview: {
            html: '',
            onDidReceiveMessage: jest.fn(),
            postMessage: jest.fn(),
        },
        dispose: jest.fn(),
        onDidDispose: jest.fn(),
    };
}

// ---------------------------------------------------------------------------
// Test Suite
// ---------------------------------------------------------------------------

describe('VtableTraceProvider', () => {
    let mockResolver: jest.Mocked<Pick<VtableResolver, 'resolve'>>;
    let mockStatusBar: { showError: jest.Mock; dispose: jest.Mock };
    let mockCreateWebviewPanel: jest.Mock;
    let mockShowTextDocument: jest.Mock;
    let fakePanel: ReturnType<typeof createFakePanel>;

    beforeEach(() => {
        fakePanel = createFakePanel();

        mockResolver = {
            resolve: jest.fn(),
        };

        mockStatusBar = {
            showError: jest.fn(),
            dispose: jest.fn(),
        };

        mockCreateWebviewPanel = jest.fn().mockReturnValue(fakePanel);
        mockShowTextDocument = jest.fn().mockResolvedValue(undefined);
    });

    afterEach(() => {
        jest.clearAllMocks();
    });

    // -------------------------------------------------------------------------
    // TC-TRACE-001: Single implementation produces a 3-node trace
    // -------------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-030"]}
    it('TC-TRACE-001: single implementation produces a 3-node trace with correct HTML', () => {
        mockResolver.resolve.mockReturnValue({
            found: true,
            locations: [{
                functionName: 'JunoDs_BuffQueue_Dequeue',
                file: 'src/juno_buff_queue.c',
                line: 112,
                assignmentFile: 'examples/example_project/engine_app.c',
                assignmentLine: 45,
            }],
        });

        const funcDefRecord: FunctionDefinitionRecord = {
            functionName: 'JunoDs_BuffQueue_Dequeue',
            file: 'src/juno_buff_queue.c',
            line: 112,
            isStatic: false,
            signature: 'JUNO_STATUS_T JunoDs_BuffQueue_Dequeue(JUNO_DS_BUFF_QUEUE_ROOT_T *ptQueue, JUNO_POINTER_T *ptOut)',
        };

        const index = createMinimalIndex(
            new Map([['JunoDs_BuffQueue_Dequeue', [funcDefRecord]]])
        );

        const provider = new VtableTraceProvider(
            mockResolver as unknown as VtableResolver,
            index,
            mockStatusBar as unknown as StatusBarHelper,
            mockCreateWebviewPanel,
            mockShowTextDocument
        );

        provider.showTrace(
            'examples/example_project/engine_app.c',
            223,
            22,
            '    tStatus = ptCmdPipeApi->Dequeue(ptCmdPipe, &tMsg);'
        );

        // Panel created exactly once
        expect(mockCreateWebviewPanel).toHaveBeenCalledTimes(1);

        // HTML contains function name and assignment file
        const html: string = fakePanel.webview.html;
        expect(html).toContain('JunoDs_BuffQueue_Dequeue');
        expect(html).toContain('examples/example_project/engine_app.c');
    });

    // -------------------------------------------------------------------------
    // TC-TRACE-002: Multiple implementations produce subtrees with shared call-site
    // -------------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-027"]}
    it('TC-TRACE-002: two implementations produce HTML containing both function names', () => {
        mockResolver.resolve.mockReturnValue({
            found: true,
            locations: [
                {
                    functionName: 'EngineApp_OnStart',
                    file: 'src/engine_app.c',
                    line: 88,
                    assignmentFile: 'examples/example_project/main.c',
                    assignmentLine: 60,
                },
                {
                    functionName: 'SystemManagerApp_OnStart',
                    file: 'src/system_manager_app.c',
                    line: 74,
                    assignmentFile: 'examples/example_project/main.c',
                    assignmentLine: 67,
                },
            ],
        });

        const index = createMinimalIndex(
            new Map([
                ['EngineApp_OnStart', [{ functionName: 'EngineApp_OnStart', file: 'src/engine_app.c', line: 88, isStatic: false }]],
                ['SystemManagerApp_OnStart', [{ functionName: 'SystemManagerApp_OnStart', file: 'src/system_manager_app.c', line: 74, isStatic: false }]],
            ])
        );

        const provider = new VtableTraceProvider(
            mockResolver as unknown as VtableResolver,
            index,
            mockStatusBar as unknown as StatusBarHelper,
            mockCreateWebviewPanel,
            mockShowTextDocument
        );

        provider.showTrace(
            'examples/example_project/main.c',
            120,
            30,
            '    ptAppList[i]->ptApi->OnStart(ptAppList[i]);'
        );

        expect(mockCreateWebviewPanel).toHaveBeenCalledTimes(1);

        const html: string = fakePanel.webview.html;
        expect(html).toContain('EngineApp_OnStart');
        expect(html).toContain('SystemManagerApp_OnStart');
    });

    // -------------------------------------------------------------------------
    // TC-TRACE-003: Resolver returns found:false — error shown, panel not created
    // -------------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-027"]}
    it('TC-TRACE-003: resolver found:false shows error and does NOT create a panel', () => {
        mockResolver.resolve.mockReturnValue({
            found: false,
            locations: [],
            errorMsg: "No implementation found for 'JUNO_APP_API_T::OnStart'.",
        });

        const provider = new VtableTraceProvider(
            mockResolver as unknown as VtableResolver,
            createMinimalIndex(),
            mockStatusBar as unknown as StatusBarHelper,
            mockCreateWebviewPanel,
            mockShowTextDocument
        );

        provider.showTrace('any/file.c', 1, 1, '    ptApp->ptApi->OnStart(ptApp);');

        expect(mockStatusBar.showError).toHaveBeenCalledTimes(1);
        expect(mockStatusBar.showError).toHaveBeenCalledWith(
            expect.stringContaining('OnStart')
        );
        expect(mockCreateWebviewPanel).not.toHaveBeenCalled();
    });

    // -------------------------------------------------------------------------
    // TC-TRACE-004: Call-site node fields are populated from cursor context
    // -------------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-030"]}
    it('TC-TRACE-004: call-site node HTML contains the source file path and call expression', () => {
        mockResolver.resolve.mockReturnValue({
            found: true,
            locations: [{
                functionName: 'Insert',
                file: 'src/juno_heap.c',
                line: 200,
                assignmentFile: 'src/my_module.c',
                assignmentLine: 10,
            }],
        });

        const provider = new VtableTraceProvider(
            mockResolver as unknown as VtableResolver,
            createMinimalIndex(),
            mockStatusBar as unknown as StatusBarHelper,
            mockCreateWebviewPanel,
            mockShowTextDocument
        );

        provider.showTrace(
            'src/my_module.c',
            77,
            18,
            '    eStatus = ptHeap->ptApi->Insert(ptHeap, tValue);'
        );

        expect(mockCreateWebviewPanel).toHaveBeenCalledTimes(1);

        const html: string = fakePanel.webview.html;
        // Call-site file appears in the HTML (HTML-escaped but no special chars in this path)
        expect(html).toContain('src/my_module.c');
        // The call expression text appears in the HTML
        expect(html).toContain('Insert');
    });

    // -------------------------------------------------------------------------
    // TC-TRACE-005: Composition-root node uses assignmentFile and assignmentLine
    // -------------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-031"]}
    it('TC-TRACE-005: composition-root node HTML contains assignmentFile and assignmentLine', () => {
        mockResolver.resolve.mockReturnValue({
            found: true,
            locations: [{
                functionName: 'JunoDs_Heap_Insert',
                file: 'src/juno_heap.c',       // implementation file
                line: 259,                      // implementation line
                assignmentFile: 'wiring/root.c', // composition root
                assignmentLine: 55,              // composition root line
            }],
        });

        const provider = new VtableTraceProvider(
            mockResolver as unknown as VtableResolver,
            createMinimalIndex(),
            mockStatusBar as unknown as StatusBarHelper,
            mockCreateWebviewPanel,
            mockShowTextDocument
        );

        provider.showTrace(
            'examples/example_project/main.c',
            90,
            25,
            '    ptHeap->ptApi->Insert(ptHeap, tValue);'
        );

        const html: string = fakePanel.webview.html;

        // Composition-root file and line appear
        expect(html).toContain('wiring/root.c');
        expect(html).toContain('55');

        // Implementation line also appears
        expect(html).toContain('259');
    });

    // -------------------------------------------------------------------------
    // TC-TRACE-006: Implementation node uses FunctionDefinitionRecord.signature
    // -------------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-032"]}
    it('TC-TRACE-006: implementation node HTML contains the full function signature from index', () => {
        const signature = 'static JUNO_STATUS_T Publish(JUNO_BROKER_ROOT_T *ptBroker, JUNO_BROKER_TOPIC_T tTopic, JUNO_POINTER_T tData)';

        mockResolver.resolve.mockReturnValue({
            found: true,
            locations: [{
                functionName: 'Publish',
                file: 'src/juno_broker.c',
                line: 51,
                assignmentFile: 'src/wiring.c',
                assignmentLine: 20,
            }],
        });

        const index = createMinimalIndex(
            new Map([['Publish', [{
                functionName: 'Publish',
                file: 'src/juno_broker.c',
                line: 51,
                isStatic: true,
                signature,
            }]]])
        );

        const provider = new VtableTraceProvider(
            mockResolver as unknown as VtableResolver,
            index,
            mockStatusBar as unknown as StatusBarHelper,
            mockCreateWebviewPanel,
            mockShowTextDocument
        );

        provider.showTrace(
            'any/file.c',
            100,
            20,
            '    ptBroker->ptApi->Publish(ptBroker, tTopic, tData);'
        );

        const html: string = fakePanel.webview.html;

        // Full signature text appears in the HTML
        expect(html).toContain('static JUNO_STATUS_T Publish');
        // Ensure 'JUNO_BROKER_ROOT_T' also appears (from signature, not hardcoded)
        expect(html).toContain('JUNO_BROKER_ROOT_T');
    });

    // -------------------------------------------------------------------------
    // TC-TRACE-007: HTML escaping prevents XSS in file paths
    // -------------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-027"]}
    it('TC-TRACE-007: HTML-special-char file paths are escaped — no raw <script> tag in HTML', () => {
        mockResolver.resolve.mockReturnValue({
            found: true,
            locations: [{
                functionName: 'Handler',
                file: 'src/<script>alert(1)</script>.c',
                line: 10,
                assignmentFile: 'src/<script>alert(1)</script>.c',
                assignmentLine: 5,
            }],
        });

        const provider = new VtableTraceProvider(
            mockResolver as unknown as VtableResolver,
            createMinimalIndex(),
            mockStatusBar as unknown as StatusBarHelper,
            mockCreateWebviewPanel,
            mockShowTextDocument
        );

        provider.showTrace('any/file.c', 1, 1, '    ptApi->Handler();');

        expect(mockCreateWebviewPanel).toHaveBeenCalledTimes(1);

        const html: string = fakePanel.webview.html;
        // The raw XSS payload must NOT appear verbatim (except inside the CSP meta nonce attr)
        // Specifically the <script> in the file path must be escaped
        // The HTML may contain a legitimate <script nonce="..."> tag — exclude that from check
        // We check the injected file path content is escaped
        expect(html).not.toContain('<script>alert(1)</script>');
        expect(html).toContain('&lt;script&gt;alert(1)&lt;/script&gt;');
    });

    // -------------------------------------------------------------------------
    // TC-TRACE-008: CSP nonce appears in both the meta tag and the inline script tag
    // -------------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-027"]}
    it('TC-TRACE-008: CSP nonce in meta tag matches nonce in inline script tag', () => {
        mockResolver.resolve.mockReturnValue({
            found: true,
            locations: [{
                functionName: 'DoThing',
                file: 'src/impl.c',
                line: 42,
                assignmentFile: 'src/root.c',
                assignmentLine: 10,
            }],
        });

        const provider = new VtableTraceProvider(
            mockResolver as unknown as VtableResolver,
            createMinimalIndex(),
            mockStatusBar as unknown as StatusBarHelper,
            mockCreateWebviewPanel,
            mockShowTextDocument
        );

        provider.showTrace('any/file.c', 1, 1, '    ptApi->DoThing();');

        const html: string = fakePanel.webview.html;

        // Extract nonce from the CSP meta tag
        const cspMatch = html.match(/script-src\s+'nonce-([a-zA-Z0-9+/=]+)'/);
        expect(cspMatch).not.toBeNull();
        const nonce = cspMatch![1];
        expect(nonce.length).toBeGreaterThan(0);

        // The same nonce must appear in the <script nonce="..."> attribute
        expect(html).toContain(`<script nonce="${nonce}">`);
    });

    // -------------------------------------------------------------------------
    // TC-TRACE-009: File link click triggers showTextDocument
    // -------------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-027"]}
    it('TC-TRACE-009: onDidReceiveMessage navigate event triggers showTextDocument', () => {
        mockResolver.resolve.mockReturnValue({
            found: true,
            locations: [{
                functionName: 'JunoDs_Heap_Insert',
                file: 'src/juno_heap.c',
                line: 259,
                assignmentFile: 'src/root.c',
                assignmentLine: 18,
            }],
        });

        const provider = new VtableTraceProvider(
            mockResolver as unknown as VtableResolver,
            createMinimalIndex(),
            mockStatusBar as unknown as StatusBarHelper,
            mockCreateWebviewPanel,
            mockShowTextDocument
        );

        provider.showTrace('any/file.c', 1, 1, '    ptHeap->ptApi->Insert(ptHeap, tValue);');

        // Retrieve the registered message handler
        expect(fakePanel.webview.onDidReceiveMessage).toHaveBeenCalledTimes(1);
        const handler = (fakePanel.webview.onDidReceiveMessage as jest.Mock).mock.calls[0][0];
        expect(typeof handler).toBe('function');

        // Manually invoke the handler with a navigate message
        handler({ type: 'navigate', file: 'src/juno_heap.c', line: 259 });

        // showTextDocument must have been called once
        expect(mockShowTextDocument).toHaveBeenCalledTimes(1);

        // Verify the URI argument
        const uriArg = (mockShowTextDocument as jest.Mock).mock.calls[0][0];
        expect(uriArg.fsPath).toBe('src/juno_heap.c');

        // Verify the line number in the options/selection argument
        const callArgs = (mockShowTextDocument as jest.Mock).mock.calls[0];
        const options = callArgs[1]; // second argument
        expect(options).toBeDefined();
        expect(options.selection).toBeDefined();
        // The selection range should start at line 258 (0-based from 1-based line 259)
        expect(options.selection.start.line).toBe(258);
    });

    // -------------------------------------------------------------------------
    // TC-TRACE-010: libjuno.showVtableTrace command defined in package.json
    // -------------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-029"]}
    it('TC-TRACE-010: package.json contributes libjuno.showVtableTrace command', () => {
        const pkg = JSON.parse(
            fs.readFileSync(path.join(__dirname, '../../../package.json'), 'utf8')
        );

        const commands: Array<{ command: string; title: string }> = pkg.contributes.commands;
        expect(Array.isArray(commands)).toBe(true);

        const entry = commands.find(c => c.command === 'libjuno.showVtableTrace');
        expect(entry).toBeDefined();
        expect(entry!.command).toBe('libjuno.showVtableTrace');
    });

    // -------------------------------------------------------------------------
    // TC-TRACE-011: Keybinding Ctrl+Shift+T defined in package.json
    // -------------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-028"]}
    it('TC-TRACE-011: package.json keybinding ctrl+shift+t → libjuno.showVtableTrace', () => {
        const pkg = JSON.parse(
            fs.readFileSync(path.join(__dirname, '../../../package.json'), 'utf8')
        );

        const keybindings: Array<{ command: string; key: string; when?: string }> =
            pkg.contributes.keybindings;
        expect(Array.isArray(keybindings)).toBe(true);

        const entry = keybindings.find(
            k => k.command === 'libjuno.showVtableTrace' &&
                 k.key.toLowerCase() === 'ctrl+shift+t'
        );
        expect(entry).toBeDefined();
        expect(entry!.when).toContain('editorTextFocus');
        expect(entry!.when).toContain('resourceLangId');
    });

    // -------------------------------------------------------------------------
    // TC-TRACE-012: Context menu entry defined in package.json
    // -------------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-029"]}
    it('TC-TRACE-012: package.json editor/context menu contains libjuno.showVtableTrace with group navigation', () => {
        const pkg = JSON.parse(
            fs.readFileSync(path.join(__dirname, '../../../package.json'), 'utf8')
        );

        expect(pkg.contributes.menus).toBeDefined();
        const contextEntries: Array<{ command: string; when?: string; group?: string }> =
            pkg.contributes.menus['editor/context'];
        expect(Array.isArray(contextEntries)).toBe(true);

        const entry = contextEntries.find(e => e.command === 'libjuno.showVtableTrace');
        expect(entry).toBeDefined();
        expect(entry!.when).toContain('resourceLangId');
        expect(entry!.group).toBe('navigation');
    });

    // -------------------------------------------------------------------------
    // TC-TRACE-013: WebviewPanel created with enableScripts:true and retainContextWhenHidden:true
    // -------------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-027"]}
    it('TC-TRACE-013: createWebviewPanel called with enableScripts:true and retainContextWhenHidden:true', () => {
        mockResolver.resolve.mockReturnValue({
            found: true,
            locations: [{
                functionName: 'Method',
                file: 'src/impl.c',
                line: 10,
                assignmentFile: 'src/root.c',
                assignmentLine: 5,
            }],
        });

        const provider = new VtableTraceProvider(
            mockResolver as unknown as VtableResolver,
            createMinimalIndex(),
            mockStatusBar as unknown as StatusBarHelper,
            mockCreateWebviewPanel,
            mockShowTextDocument
        );

        provider.showTrace('any/file.c', 1, 1, '    ptApi->Method();');

        expect(mockCreateWebviewPanel).toHaveBeenCalledTimes(1);

        const callArgs = (mockCreateWebviewPanel as jest.Mock).mock.calls[0];
        // callArgs[0] = viewType, callArgs[1] = title, callArgs[2] = column, callArgs[3] = options
        const viewType: string = callArgs[0];
        const options: { enableScripts: boolean; retainContextWhenHidden: boolean } = callArgs[3];

        expect(viewType.toLowerCase()).toContain('vtable');
        expect(options.enableScripts).toBe(true);
        expect(options.retainContextWhenHidden).toBe(true);
    });

    // -------------------------------------------------------------------------
    // TC-TRACE-014: Generated HTML contains no external http:// or https:// URLs
    // -------------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-027"]}
    it('TC-TRACE-014: generated HTML contains no external http:// or https:// URLs', () => {
        mockResolver.resolve.mockReturnValue({
            found: true,
            locations: [{
                functionName: 'Method',
                file: 'src/impl.c',
                line: 10,
                assignmentFile: 'src/root.c',
                assignmentLine: 5,
            }],
        });

        const provider = new VtableTraceProvider(
            mockResolver as unknown as VtableResolver,
            createMinimalIndex(),
            mockStatusBar as unknown as StatusBarHelper,
            mockCreateWebviewPanel,
            mockShowTextDocument
        );

        provider.showTrace('any/file.c', 1, 1, '    ptApi->Method();');

        const html: string = fakePanel.webview.html;
        expect(html).not.toContain('http://');
        expect(html).not.toContain('https://');
    });

    // -------------------------------------------------------------------------
    // TC-TRACE-015: Resolver returns found:false with empty locations — error, no panel
    // -------------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-027"]}
    it('TC-TRACE-015: resolver found:false with empty locations array shows error and does not create panel', () => {
        mockResolver.resolve.mockReturnValue({
            found: false,
            locations: [],
            errorMsg: 'No LibJuno API call pattern found at cursor position.',
        });

        const provider = new VtableTraceProvider(
            mockResolver as unknown as VtableResolver,
            createMinimalIndex(),
            mockStatusBar as unknown as StatusBarHelper,
            mockCreateWebviewPanel,
            mockShowTextDocument
        );

        provider.showTrace(
            'src/my_app.c',
            42,
            8,
            '    JUNO_MODULE_SUPER(ptSelf, MY_ROOT_T)->DoThing(ptSelf);'
        );

        expect(mockStatusBar.showError).toHaveBeenCalledTimes(1);
        expect(mockStatusBar.showError).toHaveBeenCalledWith(
            expect.stringContaining('No LibJuno API call pattern found')
        );
        expect(mockCreateWebviewPanel).not.toHaveBeenCalled();
    });

    // -------------------------------------------------------------------------
    // TC-TRACE-021: Composition root prefers initCallFile/initCallLine over assignmentFile/Line
    // -------------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-036"]}
    it('TC-TRACE-021: composition-root node uses initCallFile/initCallLine when present', () => {
        mockResolver.resolve.mockReturnValue({
            found: true,
            locations: [{
                functionName: 'JunoLog_DebugLogger_LogInfo',
                file: 'src/juno_log.c',
                line: 55,
                assignmentFile: 'src/log_api.c',
                assignmentLine: 10,
                initCallFile: 'examples/app/main.c',
                initCallLine: 42,
            }],
        });

        const provider = new VtableTraceProvider(
            mockResolver as unknown as VtableResolver,
            createMinimalIndex(),
            mockStatusBar as unknown as StatusBarHelper,
            mockCreateWebviewPanel,
            mockShowTextDocument
        );

        provider.showTrace(
            'examples/app/main.c',
            60,
            10,
            '    ptLogger->ptApi->LogInfo(ptLogger, "hello");'
        );

        expect(mockCreateWebviewPanel).toHaveBeenCalledTimes(1);
        const html: string = fakePanel.webview.html;

        // The composition-root link must use initCallFile/initCallLine: main.c:42
        expect(html).toContain('examples/app/main.c:42');
        // The composition-root link must NOT use assignmentLine 10 as the display line for main.c
        expect(html).not.toContain('examples/app/main.c:10');
    });

    // -------------------------------------------------------------------------
    // TC-TRACE-022: Composition root falls back to assignmentFile/Line when initCallFile absent
    // -------------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-036"]}
    it('TC-TRACE-022: composition-root node falls back to assignmentFile/assignmentLine when initCallFile absent', () => {
        mockResolver.resolve.mockReturnValue({
            found: true,
            locations: [{
                functionName: 'JunoDs_Heap_Insert',
                file: 'src/juno_heap.c',
                line: 259,
                assignmentFile: 'src/juno_heap.c',
                assignmentLine: 18,
                // no initCallFile or initCallLine
            }],
        });

        const provider = new VtableTraceProvider(
            mockResolver as unknown as VtableResolver,
            createMinimalIndex(),
            mockStatusBar as unknown as StatusBarHelper,
            mockCreateWebviewPanel,
            mockShowTextDocument
        );

        provider.showTrace(
            'src/juno_heap.c',
            300,
            20,
            '    ptHeap->ptApi->Insert(ptHeap, tVal);'
        );

        expect(mockCreateWebviewPanel).toHaveBeenCalledTimes(1);
        const html: string = fakePanel.webview.html;

        // The composition-root link must reference assignmentFile 'src/juno_heap.c' at line 18
        const compositionRootLinkRe = /data-file="([^"]+)"\s+data-line="(\d+)"/g;
        const links: Array<{ file: string; line: string }> = [];
        let m: RegExpExecArray | null;
        while ((m = compositionRootLinkRe.exec(html)) !== null) {
            links.push({ file: m[1], line: m[2] });
        }

        // At least one link must point to assignmentFile:assignmentLine
        const rootLink = links.find(l => l.file === 'src/juno_heap.c' && l.line === '18');
        expect(rootLink).toBeDefined();
    });
});
