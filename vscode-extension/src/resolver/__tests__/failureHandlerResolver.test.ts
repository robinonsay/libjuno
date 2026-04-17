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

// @{"verify": ["REQ-VSCODE-016"]}
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

});
