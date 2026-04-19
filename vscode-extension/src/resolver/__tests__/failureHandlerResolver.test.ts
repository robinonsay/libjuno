/// <reference types="jest" />

/**
 * @file failureHandlerResolver.test.ts
 *
 * Unit tests for FailureHandlerResolver — assignment-form resolution (TC-FH-001),
 * macro-form resolution with derivation chain setup (TC-FH-002), multi-match
 * scenarios via both Step 1 and Step 2 (TC-FH-003a/b), no-handler error path
 * (TC-FH-004), column guard documentation (TC-FH-005a/b), silent fallthrough
 * (TC-FH-006), comment-line behavior (TC-FH-NEG-001), non-assignment
 * PRIMARY_VAR_RE branch (TC-FH-007), multi-hop derivation chain (TC-FH-BND-001),
 * early-exit on non-handler line (TC-FH-NEG-002), and unknown variable
 * type in Step 2 (TC-FH-NEG-003).
 *
 * Requirements covered: REQ-VSCODE-016
 */

import { FailureHandlerResolver } from '../../resolver/failureHandlerResolver';
import { createEmptyIndex } from '../../indexer/navigationIndex';
import { LocalTypeInfo, TypeInfo, FunctionDefinitionRecord, NavigationIndex } from '../../parser/types';

// @{"verify": ["REQ-VSCODE-016", "REQ-VSCODE-022", "REQ-VSCODE-023", "REQ-VSCODE-024", "REQ-VSCODE-025", "REQ-VSCODE-026"]}
describe('FailureHandlerResolver', () => {

    // =========================================================================
    // TC-FH-001: Assignment form — _pfcnFailureHandler = Handler
    // =========================================================================

    it('TC-FH-001: assignment form (_pfcnFailureHandler = Handler) resolves to handler function definition', () => {
        const index = createEmptyIndex();

        // Function definition for the explicitly named RHS handler
        const handlerDef: FunctionDefinitionRecord = {
            functionName: 'MyHeapFailureHandler',
            file: '/src/init.c',
            line: 42,
            isStatic: false,
        };
        index.functionDefinitions.set('MyHeapFailureHandler', [handlerDef]);

        // failureHandlerAssignments satisfied for completeness
        index.failureHandlerAssignments.set('JUNO_DS_HEAP_ROOT_T', [
            { functionName: 'MyHeapFailureHandler', file: '/src/init.c', line: 42 },
        ]);

        // localTypeInfo: ptHeap → JUNO_DS_HEAP_ROOT_T in HeapInit
        const localTypeInfo: LocalTypeInfo = {
            localVariables: new Map([
                ['HeapInit', new Map<string, TypeInfo>([
                    ['ptHeap', { name: 'ptHeap', typeName: 'JUNO_DS_HEAP_ROOT_T', isPointer: true, isConst: false, isArray: false }],
                ])],
            ]),
            functionParameters: new Map(),
        };
        index.localTypeInfo.set('/src/init.c', localTypeInfo);

        const resolver = new FailureHandlerResolver(index);
        const lineText = '    ptHeap->_pfcnFailureHandler = MyHeapFailureHandler;';

        const result = resolver.resolve('/src/init.c', 50, 15, lineText, 'HeapInit');

        // Step 1 must fire: returns exactly the function definition location
        expect(result.found).toBe(true);
        expect(result.locations).toHaveLength(1);
        expect(result.locations[0].functionName).toBe('MyHeapFailureHandler');
        expect(result.locations[0].file).toBe('/src/init.c');
        expect(result.locations[0].line).toBe(42);
    });

    // =========================================================================
    // TC-FH-002: Macro form — JUNO_FAILURE_HANDLER with derivation chain setup
    // =========================================================================

    it('TC-FH-002: macro form (JUNO_FAILURE_HANDLER) with derivation chain resolves via assignment step', () => {
        const index = createEmptyIndex();

        // Function definition for the RHS handler
        const handlerDef: FunctionDefinitionRecord = {
            functionName: 'pfcnFailureHandler',
            file: '/src/engine_app.c',
            line: 111,
            isStatic: false,
        };
        index.functionDefinitions.set('pfcnFailureHandler', [handlerDef]);

        // failureHandlerAssignments for root type
        index.failureHandlerAssignments.set('JUNO_APP_ROOT_T', [
            { functionName: 'pfcnFailureHandler', file: '/src/engine_app.c', line: 111 },
        ]);

        // derivationChain: ENGINE_APP_T → JUNO_APP_ROOT_T
        index.derivationChain.set('ENGINE_APP_T', 'JUNO_APP_ROOT_T');

        // localTypeInfo: ptEngineApp → ENGINE_APP_T in EngineApp_Init
        const localTypeInfo: LocalTypeInfo = {
            localVariables: new Map([
                ['EngineApp_Init', new Map<string, TypeInfo>([
                    ['ptEngineApp', { name: 'ptEngineApp', typeName: 'ENGINE_APP_T', isPointer: true, isConst: false, isArray: false }],
                ])],
            ]),
            functionParameters: new Map(),
        };
        index.localTypeInfo.set('/src/engine_app.c', localTypeInfo);

        const resolver = new FailureHandlerResolver(index);
        const lineText = '    ptEngineApp->tRoot.JUNO_FAILURE_HANDLER = pfcnFailureHandler;';

        const result = resolver.resolve('/src/engine_app.c', 115, 20, lineText, 'EngineApp_Init');

        // Step 1 fires (ASSIGNMENT_RE matches, RHS is in functionDefinitions)
        expect(result.found).toBe(true);
        expect(result.locations).toHaveLength(1);
        expect(result.locations[0].functionName).toBe('pfcnFailureHandler');
        expect(result.locations[0].file).toBe('/src/engine_app.c');
        expect(result.locations[0].line).toBe(111);
    });

    // =========================================================================
    // TC-FH-003a: Multi-match — Step 1 returns the single explicitly named handler
    // =========================================================================

    it('TC-FH-003a: Step 1 (assignment form) returns the single explicitly-named handler when RHS is in functionDefinitions', () => {
        const index = createEmptyIndex();

        // functionDefinitions has EngineFailureHandler only
        const handlerDef: FunctionDefinitionRecord = {
            functionName: 'EngineFailureHandler',
            file: '/src/engine_app.c',
            line: 111,
            isStatic: false,
        };
        index.functionDefinitions.set('EngineFailureHandler', [handlerDef]);

        // failureHandlerAssignments has two handlers for JUNO_APP_ROOT_T
        index.failureHandlerAssignments.set('JUNO_APP_ROOT_T', [
            { functionName: 'EngineFailureHandler', file: '/src/engine_app.c', line: 111 },
            { functionName: 'SysManFailureHandler', file: '/src/sys_manager_app.c', line: 88 },
        ]);

        // localTypeInfo: ptApp → JUNO_APP_ROOT_T in App_Start
        const localTypeInfo: LocalTypeInfo = {
            localVariables: new Map([
                ['App_Start', new Map<string, TypeInfo>([
                    ['ptApp', { name: 'ptApp', typeName: 'JUNO_APP_ROOT_T', isPointer: true, isConst: false, isArray: false }],
                ])],
            ]),
            functionParameters: new Map(),
        };
        index.localTypeInfo.set('/src/app.c', localTypeInfo);

        const resolver = new FailureHandlerResolver(index);
        const lineText = '    ptApp->_pfcnFailureHandler = EngineFailureHandler;';

        const result = resolver.resolve('/src/app.c', 30, 10, lineText, 'App_Start');

        // Step 1 fires: returns exactly the one function definition, not both handlers
        expect(result.found).toBe(true);
        expect(result.locations).toHaveLength(1);
        expect(result.locations[0].functionName).toBe('EngineFailureHandler');
        expect(result.locations[0].file).toBe('/src/engine_app.c');
        expect(result.locations[0].line).toBe(111);
    });

    // =========================================================================
    // TC-FH-003b: Multi-match — Step 2 (type walk) returns all handlers
    //            when the RHS function name is not in functionDefinitions
    // =========================================================================

    it('TC-FH-003b: Step 2 (type walk) returns all registered handlers for the root type when RHS is not in functionDefinitions', () => {
        const index = createEmptyIndex();

        // functionDefinitions does NOT contain "UnknownHandler" → Step 1 falls through
        index.functionDefinitions.set('OtherFunc', [
            { functionName: 'OtherFunc', file: '/src/other.c', line: 5, isStatic: false },
        ]);

        // failureHandlerAssignments has two handlers for JUNO_APP_ROOT_T
        index.failureHandlerAssignments.set('JUNO_APP_ROOT_T', [
            { functionName: 'EngineFailureHandler', file: '/src/engine_app.c', line: 111 },
            { functionName: 'SysManFailureHandler', file: '/src/sys_manager_app.c', line: 88 },
        ]);

        // localTypeInfo: ptApp → JUNO_APP_ROOT_T in App_Start
        const localTypeInfo: LocalTypeInfo = {
            localVariables: new Map([
                ['App_Start', new Map<string, TypeInfo>([
                    ['ptApp', { name: 'ptApp', typeName: 'JUNO_APP_ROOT_T', isPointer: true, isConst: false, isArray: false }],
                ])],
            ]),
            functionParameters: new Map(),
        };
        index.localTypeInfo.set('/src/app.c', localTypeInfo);

        const resolver = new FailureHandlerResolver(index);
        // RHS is "UnknownHandler" — not in functionDefinitions
        const lineText = '    ptApp->_pfcnFailureHandler = UnknownHandler;';

        const result = resolver.resolve('/src/app.c', 30, 10, lineText, 'App_Start');

        // Step 2 fires: all handlers for JUNO_APP_ROOT_T are returned
        expect(result.found).toBe(true);
        expect(result.locations).toHaveLength(2);
        expect(result.locations[0].functionName).toBe('EngineFailureHandler');
        expect(result.locations[0].file).toBe('/src/engine_app.c');
        expect(result.locations[0].line).toBe(111);
        expect(result.locations[1].functionName).toBe('SysManFailureHandler');
        expect(result.locations[1].file).toBe('/src/sys_manager_app.c');
        expect(result.locations[1].line).toBe(88);
    });

    // =========================================================================
    // TC-FH-004: No handler found → found: false with error naming the root type
    // =========================================================================

    it('TC-FH-004: returns found=false with error message naming the root type when no handler is registered', () => {
        const index = createEmptyIndex();

        // Both functionDefinitions and failureHandlerAssignments are empty
        // (createEmptyIndex initialises them as empty Maps)

        // localTypeInfo: ptLogger → JUNO_LOG_ROOT_T in Log_Init
        const localTypeInfo: LocalTypeInfo = {
            localVariables: new Map([
                ['Log_Init', new Map<string, TypeInfo>([
                    ['ptLogger', { name: 'ptLogger', typeName: 'JUNO_LOG_ROOT_T', isPointer: true, isConst: false, isArray: false }],
                ])],
            ]),
            functionParameters: new Map(),
        };
        index.localTypeInfo.set('/src/log.c', localTypeInfo);

        const resolver = new FailureHandlerResolver(index);
        const lineText = '    ptLogger->_pfcnFailureHandler = SomeHandler;';

        const result = resolver.resolve('/src/log.c', 20, 10, lineText, 'Log_Init');

        // Step 1 falls through (SomeHandler not in functionDefinitions)
        // Step 2 walks to root JUNO_LOG_ROOT_T but finds no registered handler
        expect(result.found).toBe(false);
        expect(result.locations).toHaveLength(0);
        expect(result.errorMsg).toBeDefined();
        expect(result.errorMsg).toContain('JUNO_LOG_ROOT_T');
    });

    // =========================================================================
    // TC-FH-005a / TC-FH-005b: No column guard — any column triggers resolution
    // =========================================================================

    describe('column guard behavior', () => {
        let index!: NavigationIndex;

        beforeEach(() => {
            index = createEmptyIndex();

            index.functionDefinitions.set('OnFailure', [
                { functionName: 'OnFailure', file: '/src/mod.c', line: 25, isStatic: false },
            ]);

            index.failureHandlerAssignments.set('JUNO_MOD_ROOT_T', [
                { functionName: 'OnFailure', file: '/src/mod.c', line: 25 },
            ]);

            const localTypeInfo: LocalTypeInfo = {
                localVariables: new Map([
                    ['Mod_Init', new Map<string, TypeInfo>([
                        ['ptMod', { name: 'ptMod', typeName: 'JUNO_MOD_ROOT_T', isPointer: true, isConst: false, isArray: false }],
                    ])],
                ]),
                functionParameters: new Map(),
            };
            index.localTypeInfo.set('/src/mod.c', localTypeInfo);
        });

        // @{"verify": ["REQ-VSCODE-016"]}
        it('TC-FH-005a: column 0 on a _pfcnFailureHandler line still triggers resolution (no column guard)', () => {
            // Documents current behavior: no column guard — any cursor on this line triggers resolution.
            const resolver = new FailureHandlerResolver(index);
            const lineText = '    ptMod->_pfcnFailureHandler = OnFailure;';

            const result = resolver.resolve('/src/mod.c', 50, 0, lineText, 'Mod_Init');

            expect(result.found).toBe(true);
            expect(result.locations).toHaveLength(1);
            expect(result.locations[0].functionName).toBe('OnFailure');
        });

        // @{"verify": ["REQ-VSCODE-016"]}
        it('TC-FH-005b: column within RHS function name triggers resolution and returns that handler', () => {
            const resolver = new FailureHandlerResolver(index);
            const lineText = '    ptMod->_pfcnFailureHandler = OnFailure;';

            // Column 36 is inside "OnFailure" (starts at column 34)
            const result = resolver.resolve('/src/mod.c', 50, 36, lineText, 'Mod_Init');

            expect(result.found).toBe(true);
            expect(result.locations).toHaveLength(1);
            expect(result.locations[0].functionName).toBe('OnFailure');
            expect(result.locations[0].file).toBe('/src/mod.c');
            expect(result.locations[0].line).toBe(25);
        });
    });

    // =========================================================================
    // TC-FH-006: Silent fallthrough — ASSIGNMENT_RE matches but RHS not in index
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-016"]}
    it('TC-FH-006: Step 1 ASSIGNMENT_RE matches but RHS is not in functionDefinitions — falls through to Step 2, returns all handlers for root type', () => {
        // Step 1 ASSIGNMENT_RE matches but RHS 'MissingFunc' is not in functionDefinitions.
        // Step 2 walks ptTime → JUNO_TIME_ROOT_T and returns all registered handlers — silent fallthrough behavior.
        const index = createEmptyIndex();

        // functionDefinitions is empty — RHS will never be found in Step 1
        index.failureHandlerAssignments.set('JUNO_TIME_ROOT_T', [
            { functionName: 'TimeFailure', file: '/src/time.c', line: 80 },
        ]);

        const localTypeInfo: LocalTypeInfo = {
            localVariables: new Map([
                ['Time_Init', new Map<string, TypeInfo>([
                    ['ptTime', { name: 'ptTime', typeName: 'JUNO_TIME_ROOT_T', isPointer: true, isConst: false, isArray: false }],
                ])],
            ]),
            functionParameters: new Map(),
        };
        index.localTypeInfo.set('/src/time.c', localTypeInfo);

        const resolver = new FailureHandlerResolver(index);
        const lineText = '    ptTime->_pfcnFailureHandler = MissingFunc;';

        const result = resolver.resolve('/src/time.c', 60, 15, lineText, 'Time_Init');

        expect(result.found).toBe(true);
        expect(result.locations).toHaveLength(1);
        expect(result.locations[0].functionName).toBe('TimeFailure');
        expect(result.locations[0].file).toBe('/src/time.c');
        expect(result.locations[0].line).toBe(80);
    });

    // =========================================================================
    // TC-FH-NEG-001: Handler keyword inside a C comment — documents behavior
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-016"]}
    it('TC-FH-NEG-001: _pfcnFailureHandler inside a C comment passes the presence check but fails both resolution steps', () => {
        // The presence regex does not distinguish comments from code.
        // This test documents current behavior: the line passes the presence check
        // but fails both resolution steps.
        const index = createEmptyIndex();
        const resolver = new FailureHandlerResolver(index);

        // Comment line — no -> or . access chain, no assignment
        const lineText = '    // Set _pfcnFailureHandler for the module root';

        // No functionName supplied; empty index → enclosingFunc is undefined
        const result = resolver.resolve('/src/a.c', 10, 5, lineText);

        expect(result.found).toBe(false);
        expect(result.locations).toHaveLength(0);
        expect(result.errorMsg).toBe('Could not resolve failure handler: enclosing function or variable type unknown.');
    });

    // =========================================================================
    // Additional code path coverage
    // =========================================================================

    // =========================================================================
    // TC-FH-NEG-002: Step 0 early-exit — line does not contain handler keyword
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-016"]}
    it('TC-FH-NEG-002: line without _pfcnFailureHandler or JUNO_FAILURE_HANDLER returns found=false immediately', () => {
        const index = createEmptyIndex();
        const resolver = new FailureHandlerResolver(index);

        const lineText = '    ptModule->ptApi->method();';

        const result = resolver.resolve('/src/mod.c', 10, 5, lineText, 'SomeFunc');

        expect(result.found).toBe(false);
        expect(result.locations).toHaveLength(0);
        expect(result.errorMsg).toBe('Line does not contain a failure handler reference.');
    });

    // =========================================================================
    // TC-FH-007: Non-assignment line — ASSIGNMENT_RE returns null,
    //            PRIMARY_VAR_RE extracts 'ptMod', Step 2 type-walks to root
    //            and returns all handlers.
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-016"]}
    it('TC-FH-007: non-assignment line — ASSIGNMENT_RE returns null, PRIMARY_VAR_RE extracts primary ident, Step 2 returns handlers', () => {
        // Non-assignment line: ASSIGNMENT_RE returns null, PRIMARY_VAR_RE fires.
        // Step 2 type-walks ptMod → JUNO_MOD_ROOT_T and returns all handlers.
        const index = createEmptyIndex();

        index.failureHandlerAssignments.set('JUNO_MOD_ROOT_T', [
            { functionName: 'OnFail', file: '/src/mod.c', line: 33 },
        ]);

        const localTypeInfo: LocalTypeInfo = {
            localVariables: new Map([
                ['Check_Handler', new Map<string, TypeInfo>([
                    ['ptMod', { name: 'ptMod', typeName: 'JUNO_MOD_ROOT_T', isPointer: true, isConst: false, isArray: false }],
                ])],
            ]),
            functionParameters: new Map(),
        };
        index.localTypeInfo.set('/src/check.c', localTypeInfo);

        const resolver = new FailureHandlerResolver(index);
        const lineText = '    if (ptMod->_pfcnFailureHandler != NULL) {';

        const result = resolver.resolve('/src/check.c', 40, 10, lineText, 'Check_Handler');

        expect(result.found).toBe(true);
        expect(result.locations).toHaveLength(1);
        expect(result.locations[0].functionName).toBe('OnFail');
        expect(result.locations[0].file).toBe('/src/mod.c');
        expect(result.locations[0].line).toBe(33);
    });

    // =========================================================================
    // TC-FH-BND-001: Multi-hop derivation chain — ENGINE_IMPL_T →
    //                ENGINE_DERIVE_T → JUNO_ENGINE_ROOT_T; verifies the
    //                walkToRootType while-loop body is exercised in Step 2.
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-016"]}
    it('TC-FH-BND-001: multi-hop derivation ENGINE_IMPL_T → ENGINE_DERIVE_T → JUNO_ENGINE_ROOT_T resolves to handler via Step 2', () => {
        // Multi-hop derivation from ENGINE_IMPL_T → ENGINE_DERIVE_T → JUNO_ENGINE_ROOT_T;
        // verifies walkToRootType while-loop is exercised in Step 2.
        const index = createEmptyIndex();

        index.derivationChain.set('ENGINE_IMPL_T', 'ENGINE_DERIVE_T');
        index.derivationChain.set('ENGINE_DERIVE_T', 'JUNO_ENGINE_ROOT_T');

        index.failureHandlerAssignments.set('JUNO_ENGINE_ROOT_T', [
            { functionName: 'EngFail', file: '/src/eng.c', line: 55 },
        ]);

        const localTypeInfo: LocalTypeInfo = {
            localVariables: new Map([
                ['Eng_Run', new Map<string, TypeInfo>([
                    ['ptEng', { name: 'ptEng', typeName: 'ENGINE_IMPL_T', isPointer: true, isConst: false, isArray: false }],
                ])],
            ]),
            functionParameters: new Map(),
        };
        index.localTypeInfo.set('/src/eng.c', localTypeInfo);

        const resolver = new FailureHandlerResolver(index);
        // No assignment form — forces Step 2 via PRIMARY_VAR_RE
        const lineText = '    ptEng->_pfcnFailureHandler;';

        const result = resolver.resolve('/src/eng.c', 70, 10, lineText, 'Eng_Run');

        expect(result.found).toBe(true);
        expect(result.locations).toHaveLength(1);
        expect(result.locations[0].functionName).toBe('EngFail');
        expect(result.locations[0].file).toBe('/src/eng.c');
        expect(result.locations[0].line).toBe(55);
    });

    // =========================================================================
    // TC-FH-NEG-003: typeInfo undefined in Step 2 — variable not in
    //                localTypeInfo falls to final catch-all error.
    // =========================================================================

    // @{"verify": ["REQ-VSCODE-016"]}
    it('TC-FH-NEG-003: PRIMARY_VAR_RE finds ident but variable absent from localTypeInfo — returns catch-all error', () => {
        const index = createEmptyIndex();

        // localTypeInfo for this file/function has no entry for 'ptUnknown'
        const localTypeInfo: LocalTypeInfo = {
            localVariables: new Map([
                ['SomeFunc', new Map<string, TypeInfo>()],
            ]),
            functionParameters: new Map(),
        };
        index.localTypeInfo.set('/src/x.c', localTypeInfo);

        const resolver = new FailureHandlerResolver(index);
        const lineText = '    ptUnknown->_pfcnFailureHandler;';

        const result = resolver.resolve('/src/x.c', 10, 5, lineText, 'SomeFunc');

        expect(result.found).toBe(false);
        expect(result.locations).toHaveLength(0);
        expect(result.errorMsg).toBe('Could not resolve failure handler: enclosing function or variable type unknown.');
    });

    // =========================================================================
    // FAIL Macro Call Site Resolution (§5.3.1) — TC-FAIL-001 through TC-FAIL-012
    // =========================================================================

    describe('FAIL macro call site resolution (§5.3.1)', () => {

        // =====================================================================
        // TC-FAIL-001: JUNO_FAIL — handler name found in functionDefinitions
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-023"]}
        it('TC-FAIL-001: JUNO_FAIL with known handler resolves directly to functionDefinitions entry', () => {
            const index = createEmptyIndex();

            index.functionDefinitions.set('MyFailureHandler', [
                { functionName: 'MyFailureHandler', file: 'src/module.c', line: 55, isStatic: false },
            ]);
            // failureHandlerAssignments empty — not consulted for JUNO_FAIL
            // derivationChain empty — not consulted for JUNO_FAIL
            // localTypeInfo empty — not consulted for JUNO_FAIL

            const resolver = new FailureHandlerResolver(index);
            const lineText = '    JUNO_FAIL(eStatus, MyFailureHandler, NULL, "operation failed");';

            const result = resolver.resolve('src/caller.c', 87, 4, lineText, 'testFn');

            expect(result.found).toBe(true);
            expect(result.locations).toHaveLength(1);
            expect(result.locations[0].functionName).toBe('MyFailureHandler');
            expect(result.locations[0].file).toBe('src/module.c');
            expect(result.locations[0].line).toBe(55);
        });

        // =====================================================================
        // TC-FAIL-002: JUNO_FAIL — unknown handler not in functionDefinitions
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-023"]}
        it('TC-FAIL-002: JUNO_FAIL with unknown handler returns found=false with identifier in errorMsg', () => {
            const index = createEmptyIndex();
            // functionDefinitions empty — no entry for UnknownHandler

            const resolver = new FailureHandlerResolver(index);
            const lineText = '    JUNO_FAIL(eStatus, UnknownHandler, pvUserData, "msg");';

            const result = resolver.resolve('src/caller.c', 102, 4, lineText, 'testFn');

            expect(result.found).toBe(false);
            expect(result.locations).toHaveLength(0);
            expect(result.errorMsg).toContain('UnknownHandler');
        });

        // =====================================================================
        // TC-FAIL-003: JUNO_FAIL_MODULE — derived type walks chain to handler
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-024"]}
        it('TC-FAIL-003: JUNO_FAIL_MODULE walks derivation chain ENGINE_APP_T → JUNO_APP_ROOT_T and finds handler', () => {
            const index = createEmptyIndex();

            index.derivationChain.set('ENGINE_APP_T', 'JUNO_APP_ROOT_T');
            index.failureHandlerAssignments.set('JUNO_APP_ROOT_T', [
                { functionName: 'EngineFailureHandler', file: 'engine/src/engine_app.c', line: 111 },
            ]);
            // functionDefinitions non-empty — verify resolver does NOT use it for JUNO_FAIL_MODULE
            index.functionDefinitions.set('OtherFunc', [
                { functionName: 'OtherFunc', file: 'other.c', line: 5, isStatic: false },
            ]);

            const localTypeInfo: LocalTypeInfo = {
                localVariables: new Map([
                    ['testFn', new Map<string, TypeInfo>([
                        ['ptEngineApp', { name: 'ptEngineApp', typeName: 'ENGINE_APP_T', isPointer: true, isConst: false, isArray: false }],
                    ])],
                ]),
                functionParameters: new Map(),
            };
            index.localTypeInfo.set('engine/src/engine_app.c', localTypeInfo);

            const resolver = new FailureHandlerResolver(index);
            const lineText = '    JUNO_FAIL_MODULE(eStatus, ptEngineApp, "init failed");';

            const result = resolver.resolve('engine/src/engine_app.c', 210, 4, lineText, 'testFn');

            expect(result.found).toBe(true);
            expect(result.locations).toHaveLength(1);
            expect(result.locations[0].functionName).toBe('EngineFailureHandler');
            expect(result.locations[0].file).toBe('engine/src/engine_app.c');
            expect(result.locations[0].line).toBe(111);
        });

        // =====================================================================
        // TC-FAIL-004: JUNO_FAIL_MODULE — no handler for resolved root type
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-024"]}
        it('TC-FAIL-004: JUNO_FAIL_MODULE with no handler for root type returns found=false naming root type', () => {
            const index = createEmptyIndex();

            index.derivationChain.set('MY_SENSOR_T', 'JUNO_APP_ROOT_T');
            // failureHandlerAssignments empty — no entry for JUNO_APP_ROOT_T

            const localTypeInfo: LocalTypeInfo = {
                localVariables: new Map([
                    ['testFn', new Map<string, TypeInfo>([
                        ['ptSensor', { name: 'ptSensor', typeName: 'MY_SENSOR_T', isPointer: true, isConst: false, isArray: false }],
                    ])],
                ]),
                functionParameters: new Map(),
            };
            index.localTypeInfo.set('sensors/src/sensor.c', localTypeInfo);

            const resolver = new FailureHandlerResolver(index);
            const lineText = '    JUNO_FAIL_MODULE(eStatus, ptSensor, "sensor error");';

            const result = resolver.resolve('sensors/src/sensor.c', 78, 4, lineText, 'testFn');

            expect(result.found).toBe(false);
            expect(result.locations).toHaveLength(0);
            expect(result.errorMsg).toContain('JUNO_APP_ROOT_T');
        });

        // =====================================================================
        // TC-FAIL-005: JUNO_FAIL_ROOT — root type pointer, handler found
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-025"]}
        it('TC-FAIL-005: JUNO_FAIL_ROOT with root type pointer resolves directly via failureHandlerAssignments', () => {
            const index = createEmptyIndex();

            // derivationChain empty — not consulted for JUNO_FAIL_ROOT
            index.failureHandlerAssignments.set('JUNO_LOG_ROOT_T', [
                { functionName: 'LogFailureHandler', file: 'src/logger.c', line: 33 },
            ]);

            const localTypeInfo: LocalTypeInfo = {
                localVariables: new Map([
                    ['testFn', new Map<string, TypeInfo>([
                        ['ptLogRoot', { name: 'ptLogRoot', typeName: 'JUNO_LOG_ROOT_T', isPointer: true, isConst: false, isArray: false }],
                    ])],
                ]),
                functionParameters: new Map(),
            };
            index.localTypeInfo.set('src/logger.c', localTypeInfo);

            const resolver = new FailureHandlerResolver(index);
            const lineText = '    JUNO_FAIL_ROOT(eStatus, ptLogRoot, "logger failure");';

            const result = resolver.resolve('src/logger.c', 120, 4, lineText, 'testFn');

            expect(result.found).toBe(true);
            expect(result.locations).toHaveLength(1);
            expect(result.locations[0].functionName).toBe('LogFailureHandler');
            expect(result.locations[0].file).toBe('src/logger.c');
            expect(result.locations[0].line).toBe(33);
        });

        // =====================================================================
        // TC-FAIL-006: JUNO_FAIL_ROOT — no handler registered for root type
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-025"]}
        it('TC-FAIL-006: JUNO_FAIL_ROOT with no handler registered returns found=false naming root type', () => {
            const index = createEmptyIndex();

            // derivationChain empty, failureHandlerAssignments empty

            const localTypeInfo: LocalTypeInfo = {
                localVariables: new Map([
                    ['testFn', new Map<string, TypeInfo>([
                        ['ptHeapRoot', { name: 'ptHeapRoot', typeName: 'JUNO_DS_HEAP_ROOT_T', isPointer: true, isConst: false, isArray: false }],
                    ])],
                ]),
                functionParameters: new Map(),
            };
            index.localTypeInfo.set('src/heap_usage.c', localTypeInfo);

            const resolver = new FailureHandlerResolver(index);
            const lineText = '    JUNO_FAIL_ROOT(eStatus, ptHeapRoot, "heap overflow");';

            const result = resolver.resolve('src/heap_usage.c', 45, 4, lineText, 'testFn');

            expect(result.found).toBe(false);
            expect(result.locations).toHaveLength(0);
            expect(result.errorMsg).toContain('JUNO_DS_HEAP_ROOT_T');
        });

        // =====================================================================
        // TC-FAIL-007: JUNO_ASSERT_EXISTS_MODULE — derived type, handler found
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-026"]}
        it('TC-FAIL-007: JUNO_ASSERT_EXISTS_MODULE walks derivation chain JUNO_SB_PIPE_T → JUNO_DS_QUEUE_ROOT_T and finds handler', () => {
            const index = createEmptyIndex();

            index.derivationChain.set('JUNO_SB_PIPE_T', 'JUNO_DS_QUEUE_ROOT_T');
            index.failureHandlerAssignments.set('JUNO_DS_QUEUE_ROOT_T', [
                { functionName: 'QueueFailureHandler', file: 'src/juno_queue.c', line: 28 },
            ]);

            const localTypeInfo: LocalTypeInfo = {
                localVariables: new Map([
                    ['testFn', new Map<string, TypeInfo>([
                        ['ptPipe', { name: 'ptPipe', typeName: 'JUNO_SB_PIPE_T', isPointer: true, isConst: false, isArray: false }],
                    ])],
                ]),
                functionParameters: new Map(),
            };
            index.localTypeInfo.set('src/broker.c', localTypeInfo);

            const resolver = new FailureHandlerResolver(index);
            // arg[0] = "ptPipe != NULL", arg[1] = "ptPipe", arg[2] = "\"pipe must exist\""
            const lineText = '    JUNO_ASSERT_EXISTS_MODULE(ptPipe != NULL, ptPipe, "pipe must exist");';

            const result = resolver.resolve('src/broker.c', 66, 4, lineText, 'testFn');

            expect(result.found).toBe(true);
            expect(result.locations).toHaveLength(1);
            expect(result.locations[0].functionName).toBe('QueueFailureHandler');
            expect(result.locations[0].file).toBe('src/juno_queue.c');
            expect(result.locations[0].line).toBe(28);
        });

        // =====================================================================
        // TC-FAIL-008: JUNO_ASSERT_EXISTS_MODULE — no handler registered
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-026"]}
        it('TC-FAIL-008: JUNO_ASSERT_EXISTS_MODULE with no handler registered returns found=false naming root type', () => {
            const index = createEmptyIndex();

            index.derivationChain.set('MY_IMPL_T', 'JUNO_POINTER_ROOT_T');
            // failureHandlerAssignments empty — no entry for JUNO_POINTER_ROOT_T

            const localTypeInfo: LocalTypeInfo = {
                localVariables: new Map([
                    ['testFn', new Map<string, TypeInfo>([
                        ['ptImpl', { name: 'ptImpl', typeName: 'MY_IMPL_T', isPointer: true, isConst: false, isArray: false }],
                    ])],
                ]),
                functionParameters: new Map(),
            };
            index.localTypeInfo.set('src/init.c', localTypeInfo);

            const resolver = new FailureHandlerResolver(index);
            const lineText = '    JUNO_ASSERT_EXISTS_MODULE(ptImpl != NULL, ptImpl, "impl required");';

            const result = resolver.resolve('src/init.c', 91, 4, lineText, 'testFn');

            expect(result.found).toBe(false);
            expect(result.locations).toHaveLength(0);
            expect(result.errorMsg).toContain('JUNO_POINTER_ROOT_T');
        });

        // =====================================================================
        // TC-FAIL-009: Non-macro line — falls through to §5.3 algorithm
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-022"]}
        it('TC-FAIL-009: non-macro _pfcnFailureHandler assignment line falls through to §5.3 and resolves via failureHandlerAssignments', () => {
            // Line contains _pfcnFailureHandler but NOT a JUNO_FAIL* macro —
            // FAIL_MACRO_RE finds no match, so §5.3.1 Step 0 does not fire.
            // The §5.3 algorithm takes over: ASSIGNMENT_RE fires (RHS not in
            // functionDefinitions), falls through to Step 2 type-walk.
            const index = createEmptyIndex();

            index.failureHandlerAssignments.set('JUNO_DS_HEAP_ROOT_T', [
                { functionName: 'HeapFailureHandler', file: 'src/heap.c', line: 77 },
            ]);
            // functionDefinitions empty — §5.3 Step 1 falls through to Step 2

            const localTypeInfo: LocalTypeInfo = {
                localVariables: new Map([
                    ['testFn', new Map<string, TypeInfo>([
                        ['ptHeap', { name: 'ptHeap', typeName: 'JUNO_DS_HEAP_ROOT_T', isPointer: true, isConst: false, isArray: false }],
                    ])],
                ]),
                functionParameters: new Map(),
            };
            index.localTypeInfo.set('src/init.c', localTypeInfo);

            const resolver = new FailureHandlerResolver(index);
            const lineText = '    ptHeap->_pfcnFailureHandler = HeapFailureHandler;';

            const result = resolver.resolve('src/init.c', 40, 12, lineText, 'testFn');

            // Verify §5.3 result: found via failureHandlerAssignments type-walk
            expect(result.found).toBe(true);
            expect(result.locations).toHaveLength(1);
            expect(result.locations[0].functionName).toBe('HeapFailureHandler');
            expect(result.locations[0].file).toBe('src/heap.c');
            expect(result.locations[0].line).toBe(77);
        });

        // =====================================================================
        // TC-FAIL-010: JUNO_FAIL — compound expression in arg[1] not a bare identifier
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-023"]}
        it('TC-FAIL-010: JUNO_FAIL with compound arg[1] (nested call) returns found=false with expression in errorMsg', () => {
            const index = createEmptyIndex();

            // functionDefinitions has entries but none matching the compound expression
            index.functionDefinitions.set('someOtherFunc', [
                { functionName: 'someOtherFunc', file: 'other.c', line: 10, isStatic: false },
            ]);
            // failureHandlerAssignments, derivationChain, localTypeInfo all empty

            const resolver = new FailureHandlerResolver(index);
            const lineText = '    JUNO_FAIL(eStatus, getHandler(ptMod), NULL, "msg");';

            const result = resolver.resolve('src/caller.c', 130, 4, lineText, 'testFn');

            expect(result.found).toBe(false);
            expect(result.locations).toHaveLength(0);
            expect(result.errorMsg).toContain('getHandler');
        });

        // =====================================================================
        // TC-FAIL-011: JUNO_FAIL_MODULE — cast expression in arg[1] stripped
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-024"]}
        it('TC-FAIL-011: JUNO_FAIL_MODULE strips cast (MY_MOD_T *)ptMod to bare identifier and resolves via type chain', () => {
            const index = createEmptyIndex();

            index.derivationChain.set('MY_MOD_T', 'JUNO_APP_ROOT_T');
            index.failureHandlerAssignments.set('JUNO_APP_ROOT_T', [
                { functionName: 'AppFailureHandler', file: 'src/app.c', line: 19 },
            ]);

            const localTypeInfo: LocalTypeInfo = {
                localVariables: new Map([
                    ['testFn', new Map<string, TypeInfo>([
                        ['ptMod', { name: 'ptMod', typeName: 'MY_MOD_T', isPointer: true, isConst: false, isArray: false }],
                    ])],
                ]),
                functionParameters: new Map(),
            };
            index.localTypeInfo.set('src/app.c', localTypeInfo);

            const resolver = new FailureHandlerResolver(index);
            const lineText = '    JUNO_FAIL_MODULE(eStatus, (MY_MOD_T *)ptMod, "cast call");';

            const result = resolver.resolve('src/app.c', 155, 4, lineText, 'testFn');

            expect(result.found).toBe(true);
            expect(result.locations).toHaveLength(1);
            expect(result.locations[0].functionName).toBe('AppFailureHandler');
            expect(result.locations[0].file).toBe('src/app.c');
            expect(result.locations[0].line).toBe(19);
        });

        // =====================================================================
        // TC-FAIL-012: JUNO_FAIL_MODULE — two-level derivation chain
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-024"]}
        it('TC-FAIL-012: JUNO_FAIL_MODULE walks two-hop chain TYPE_A_T → TYPE_B_T → ROOT_T and finds handler', () => {
            const index = createEmptyIndex();

            index.derivationChain.set('TYPE_A_T', 'TYPE_B_T');
            index.derivationChain.set('TYPE_B_T', 'ROOT_T');
            index.failureHandlerAssignments.set('ROOT_T', [
                { functionName: 'RootFailureHandler', file: 'src/root_module.c', line: 9 },
            ]);

            const localTypeInfo: LocalTypeInfo = {
                localVariables: new Map([
                    ['testFn', new Map<string, TypeInfo>([
                        ['ptTypeA', { name: 'ptTypeA', typeName: 'TYPE_A_T', isPointer: true, isConst: false, isArray: false }],
                    ])],
                ]),
                functionParameters: new Map(),
            };
            index.localTypeInfo.set('src/deep_caller.c', localTypeInfo);

            const resolver = new FailureHandlerResolver(index);
            const lineText = '    JUNO_FAIL_MODULE(eStatus, ptTypeA, "deep error");';

            const result = resolver.resolve('src/deep_caller.c', 200, 4, lineText, 'testFn');

            expect(result.found).toBe(true);
            expect(result.locations).toHaveLength(1);
            expect(result.locations[0].functionName).toBe('RootFailureHandler');
            expect(result.locations[0].file).toBe('src/root_module.c');
            expect(result.locations[0].line).toBe(9);
        });

    }); // end describe('FAIL macro call site resolution (§5.3.1)')

    // =========================================================================
    // REQ-VSCODE-038: Full derivation chain walk in lookupHandlersByType
    // REQ-VSCODE-041: ConcreteLocation.kind discrimination
    // =========================================================================

    describe('derivation chain walk (REQ-VSCODE-038) and kind field (REQ-VSCODE-041)', () => {

        // =====================================================================
        // TC-FH-APP-001: Handler found at intermediate type in derivation chain
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-038"]}
        it('TC-FH-APP-001: Step 2 finds handler registered at intermediate type APP_ROOT_T in ENGINE_APP_T → APP_ROOT_T → MODULE_ROOT_T chain', () => {
            const index = createEmptyIndex();

            // Two-hop chain: ENGINE_APP_T → APP_ROOT_T → MODULE_ROOT_T
            index.derivationChain.set('ENGINE_APP_T', 'APP_ROOT_T');
            index.derivationChain.set('APP_ROOT_T', 'MODULE_ROOT_T');

            // Handler registered at the intermediate type APP_ROOT_T (not terminal root)
            index.failureHandlerAssignments.set('APP_ROOT_T', [
                { functionName: 'AppFailureHandler', file: '/src/engine_app.c', line: 111 },
            ]);

            // localTypeInfo: ptEngineApp → ENGINE_APP_T in EngineApp_Init
            const localTypeInfo: LocalTypeInfo = {
                localVariables: new Map([
                    ['EngineApp_Init', new Map<string, TypeInfo>([
                        ['ptEngineApp', { name: 'ptEngineApp', typeName: 'ENGINE_APP_T', isPointer: true, isConst: false, isArray: false }],
                    ])],
                ]),
                functionParameters: new Map(),
            };
            index.localTypeInfo.set('/src/engine_app.c', localTypeInfo);

            const resolver = new FailureHandlerResolver(index);
            // Non-assignment reference — Step 1 is skipped, Step 2 runs
            const lineText = 'ptEngineApp->tRoot.JUNO_FAILURE_HANDLER;';

            const result = resolver.resolve('/src/engine_app.c', 60, 0, lineText, 'EngineApp_Init');

            expect(result.found).toBe(true);
            expect(result.locations).toHaveLength(1);
            expect(result.locations[0].functionName).toBe('AppFailureHandler');
        });

        // =====================================================================
        // TC-FH-APP-002: Handler NOT found when chain exhausted
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-038"]}
        it('TC-FH-APP-002: Step 2 returns found=false with type name in errorMsg when no handler at any level of ENGINE_APP_T → APP_ROOT_T → MODULE_ROOT_T', () => {
            const index = createEmptyIndex();

            // Two-hop chain: ENGINE_APP_T → APP_ROOT_T → MODULE_ROOT_T
            index.derivationChain.set('ENGINE_APP_T', 'APP_ROOT_T');
            index.derivationChain.set('APP_ROOT_T', 'MODULE_ROOT_T');

            // failureHandlerAssignments is empty — no handler at any level

            const localTypeInfo: LocalTypeInfo = {
                localVariables: new Map([
                    ['EngineApp_Init', new Map<string, TypeInfo>([
                        ['ptEngineApp', { name: 'ptEngineApp', typeName: 'ENGINE_APP_T', isPointer: true, isConst: false, isArray: false }],
                    ])],
                ]),
                functionParameters: new Map(),
            };
            index.localTypeInfo.set('/src/engine_app.c', localTypeInfo);

            const resolver = new FailureHandlerResolver(index);
            const lineText = 'ptEngineApp->tRoot.JUNO_FAILURE_HANDLER;';

            const result = resolver.resolve('/src/engine_app.c', 60, 0, lineText, 'EngineApp_Init');

            expect(result.found).toBe(false);
            expect(result.locations).toHaveLength(0);
            expect(result.errorMsg).toBeDefined();
            // errorMsg must contain some type name — the terminal root or an intermediate type
            expect(result.errorMsg!.length).toBeGreaterThan(0);
            expect(result.errorMsg).toMatch(/ENGINE_APP_T|APP_ROOT_T|MODULE_ROOT_T/);
        });

        // =====================================================================
        // TC-FH-APP-003: JUNO_FAIL_MODULE resolves via intermediate type
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-038"]}
        it('TC-FH-APP-003: JUNO_FAIL_MODULE resolves handler registered at intermediate type APP_ROOT_T via ENGINE_APP_T → APP_ROOT_T → MODULE_ROOT_T chain', () => {
            const index = createEmptyIndex();

            // Two-hop chain: ENGINE_APP_T → APP_ROOT_T → MODULE_ROOT_T
            index.derivationChain.set('ENGINE_APP_T', 'APP_ROOT_T');
            index.derivationChain.set('APP_ROOT_T', 'MODULE_ROOT_T');

            // Handler registered at the intermediate type APP_ROOT_T
            index.failureHandlerAssignments.set('APP_ROOT_T', [
                { functionName: 'AppFailureHandler', file: '/src/engine_app.c', line: 111 },
            ]);

            const localTypeInfo: LocalTypeInfo = {
                localVariables: new Map([
                    ['EngineApp_Init', new Map<string, TypeInfo>([
                        ['ptEngineApp', { name: 'ptEngineApp', typeName: 'ENGINE_APP_T', isPointer: true, isConst: false, isArray: false }],
                    ])],
                ]),
                functionParameters: new Map(),
            };
            index.localTypeInfo.set('/src/engine_app.c', localTypeInfo);

            const resolver = new FailureHandlerResolver(index);
            const lineText = 'JUNO_FAIL_MODULE(tStatus, ptEngineApp, "error");';

            const result = resolver.resolve('/src/engine_app.c', 60, 0, lineText, 'EngineApp_Init');

            expect(result.found).toBe(true);
            expect(result.locations).toHaveLength(1);
            expect(result.locations[0].functionName).toBe('AppFailureHandler');
        });

        // =====================================================================
        // TC-FH-KIND-001: Step 2 result has kind: 'assignment'
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-041"]}
        it('TC-FH-KIND-001: Step 2 type-walk result has kind === "assignment"', () => {
            const index = createEmptyIndex();

            // Single-hop chain: DERIVED_T → ROOT_T
            index.derivationChain.set('DERIVED_T', 'ROOT_T');
            index.failureHandlerAssignments.set('ROOT_T', [
                { functionName: 'RootHandler', file: '/src/mod.c', line: 5 },
            ]);

            const localTypeInfo: LocalTypeInfo = {
                localVariables: new Map([
                    ['Mod_Init', new Map<string, TypeInfo>([
                        ['ptMod', { name: 'ptMod', typeName: 'DERIVED_T', isPointer: true, isConst: false, isArray: false }],
                    ])],
                ]),
                functionParameters: new Map(),
            };
            index.localTypeInfo.set('/src/mod.c', localTypeInfo);

            const resolver = new FailureHandlerResolver(index);
            // Non-assignment form — Step 1 is skipped, Step 2 runs
            const lineText = 'ptMod->JUNO_FAILURE_HANDLER;';

            const result = resolver.resolve('/src/mod.c', 20, 0, lineText, 'Mod_Init');

            expect(result.found).toBe(true);
            expect(result.locations).toHaveLength(1);
            expect(result.locations[0].kind).toBe('assignment');
        });

        // =====================================================================
        // TC-FH-KIND-002: JUNO_FAIL macro result has kind: 'invocation'
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-041"]}
        it('TC-FH-KIND-002: JUNO_FAIL macro result has kind === "invocation"', () => {
            const index = createEmptyIndex();

            index.functionDefinitions.set('MyHandler', [
                { functionName: 'MyHandler', file: '/src/handler.c', line: 10, isStatic: false },
            ]);

            const resolver = new FailureHandlerResolver(index);
            const lineText = 'JUNO_FAIL(tStatus, MyHandler, pvData, "msg");';

            const result = resolver.resolve('/src/caller.c', 30, 0, lineText, 'SomeFn');

            expect(result.found).toBe(true);
            expect(result.locations).toHaveLength(1);
            expect(result.locations[0].functionName).toBe('MyHandler');
            expect(result.locations[0].file).toBe('/src/handler.c');
            expect(result.locations[0].line).toBe(10);
            expect(result.locations[0].kind).toBe('invocation');
        });

        // =====================================================================
        // TC-FH-KIND-003: Step 1 assignment form result has kind: 'assignment'
        // =====================================================================

        // @{"verify": ["REQ-VSCODE-041"]}
        it('TC-FH-KIND-003: Step 1 assignment form result has kind === "assignment"', () => {
            const index = createEmptyIndex();

            index.functionDefinitions.set('MyHandler', [
                { functionName: 'MyHandler', file: '/src/handler.c', line: 20, isStatic: false },
            ]);

            const resolver = new FailureHandlerResolver(index);
            const lineText = 'ptMod->JUNO_FAILURE_HANDLER = MyHandler;';

            const result = resolver.resolve('/src/mod.c', 50, 0, lineText, 'Mod_Init');

            expect(result.found).toBe(true);
            expect(result.locations).toHaveLength(1);
            expect(result.locations[0].functionName).toBe('MyHandler');
            expect(result.locations[0].file).toBe('/src/handler.c');
            expect(result.locations[0].line).toBe(20);
            expect(result.locations[0].kind).toBe('assignment');
        });

    }); // end describe('derivation chain walk and kind field')

});
