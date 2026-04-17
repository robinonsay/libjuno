/// <reference types="jest" />

/**
 * @file vtableResolver.test.ts
 *
 * Tests for VtableResolver — graceful error handling (TC-RES-006a–d),
 * single-implementation navigation (TC-RES-001a/b), chain resolution
 * strategies (TC-RES-002–005), multi-match (TC-RES-007), and regex
 * boundary conditions (TC-RES-008–011).
 */

import { VtableResolver } from '../../resolver/vtableResolver';
import { createEmptyIndex } from '../../indexer/navigationIndex';
import { ConcreteLocation, LocalTypeInfo, TypeInfo } from '../../parser/types';

// ---------------------------------------------------------------------------
// Shared test fixture
// ---------------------------------------------------------------------------

/**
 * A source line that matches Strategy 1 (JUNO_MODULE_GET_API macro) starting
 * at column 0. The regex extracts rootType="MY_ROOT_T", field="DoThing".
 */
const MACRO_LINE =
    'JUNO_MODULE_GET_API(ptSelf, MY_ROOT_T)->DoThing(ptSelf);';

/** Column 0 is inside the macro match. */
const CURSOR_IN_MACRO = 0;

// ---------------------------------------------------------------------------
// REQ-VSCODE-004 — Graceful Error on Missing Implementation
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-004"]}
describe('VtableResolver — Graceful Error on Missing Implementation', () => {
    it('TC-RES-006a: should return found=false with errorMsg when no API call pattern matches at cursor', () => {
        const index = createEmptyIndex();
        const resolver = new VtableResolver(index);

        // Plain assignment line — no JUNO_MODULE_GET_API or -> call pattern
        const result = resolver.resolve('/src/myModule.c', 10, 5, '    ptSelf->someField = 0;');

        expect(result.found).toBe(false);
        expect(result.locations).toHaveLength(0);
        expect(result.errorMsg).toBeTruthy();
        expect(result.errorMsg).toContain('No LibJuno API call pattern found');
    });

    it('TC-RES-006b: should return found=false with errorMsg when the macro pattern matches but the root type has no registered API type', () => {
        // Index is empty — MY_ROOT_T not in moduleRoots or traitRoots
        const index = createEmptyIndex();
        const resolver = new VtableResolver(index);

        const result = resolver.resolve('/src/myModule.c', 10, CURSOR_IN_MACRO, MACRO_LINE);

        expect(result.found).toBe(false);
        expect(result.locations).toHaveLength(0);
        expect(result.errorMsg).toBeTruthy();
        // The error message must mention the unresolved root type
        expect(result.errorMsg).toContain('MY_ROOT_T');
    });

    it('TC-RES-006c: should return found=false with errorMsg when the API type exists but has no vtable assignments at all', () => {
        const index = createEmptyIndex();
        // moduleRoots maps MY_ROOT_T → MY_API_T, but vtableAssignments is empty
        index.moduleRoots.set('MY_ROOT_T', 'MY_API_T');
        const resolver = new VtableResolver(index);

        const result = resolver.resolve('/src/myModule.c', 10, CURSOR_IN_MACRO, MACRO_LINE);

        expect(result.found).toBe(false);
        expect(result.locations).toHaveLength(0);
        expect(result.errorMsg).toBeTruthy();
        expect(result.errorMsg).toContain('MY_API_T');
    });

    it('TC-RES-006d: should return found=false with errorMsg when the API type is registered and has assignments but the specific field is absent', () => {
        const index = createEmptyIndex();
        index.moduleRoots.set('MY_ROOT_T', 'MY_API_T');

        // MY_API_T has 'OtherMethod' mapped, but NOT 'DoThing'
        const fieldMap = new Map<string, ConcreteLocation[]>();
        fieldMap.set('OtherMethod', [
            { functionName: 'Impl_OtherMethod', file: '/impl/impl.c', line: 42 },
        ]);
        index.vtableAssignments.set('MY_API_T', fieldMap);

        const resolver = new VtableResolver(index);

        const result = resolver.resolve('/src/myModule.c', 10, CURSOR_IN_MACRO, MACRO_LINE);

        expect(result.found).toBe(false);
        expect(result.locations).toHaveLength(0);
        expect(result.errorMsg).toBeTruthy();
        // Error must name the missing field
        expect(result.errorMsg).toContain('DoThing');
    });
});

// ---------------------------------------------------------------------------
// REQ-VSCODE-005 — Single Implementation Navigation
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-005"]}
describe('VtableResolver — Single Implementation Navigation', () => {
    /** Build an index where MY_ROOT_T→MY_API_T and MY_API_T::DoThing has one impl. */
    function buildSingleImplIndex(location: ConcreteLocation) {
        const index = createEmptyIndex();
        index.moduleRoots.set('MY_ROOT_T', 'MY_API_T');
        const fieldMap = new Map<string, ConcreteLocation[]>();
        fieldMap.set('DoThing', [location]);
        index.vtableAssignments.set('MY_API_T', fieldMap);
        return index;
    }

    it('TC-RES-001a: should return found=true with exactly one location when a single vtable assignment exists', () => {
        const impl: ConcreteLocation = {
            functionName: 'MyImpl_DoThing',
            file: '/impl/myImpl.c',
            line: 77,
        };
        const index = buildSingleImplIndex(impl);
        const resolver = new VtableResolver(index);

        const result = resolver.resolve('/src/myModule.c', 10, CURSOR_IN_MACRO, MACRO_LINE);

        expect(result.found).toBe(true);
        expect(result.locations).toHaveLength(1);
    });

    it('TC-RES-001b: should return the ConcreteLocation with the exact file, line, and functionName from the index', () => {
        const impl: ConcreteLocation = {
            functionName: 'MyImpl_DoThing',
            file: '/impl/myImpl.c',
            line: 77,
        };
        const index = buildSingleImplIndex(impl);
        const resolver = new VtableResolver(index);

        const result = resolver.resolve('/src/myModule.c', 10, CURSOR_IN_MACRO, MACRO_LINE);

        expect(result.found).toBe(true);
        expect(result.locations).toHaveLength(1);
        expect(result.locations[0].functionName).toBe('MyImpl_DoThing');
        expect(result.locations[0].file).toBe('/impl/myImpl.c');
        expect(result.locations[0].line).toBe(77);
    });
});

// ---------------------------------------------------------------------------
// Regex Boundary Conditions (TC-RES-008 through TC-RES-011)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-002", "REQ-VSCODE-005", "REQ-VSCODE-006"]}
describe('VtableResolver — Regex Boundary Conditions', () => {
    /** Build index resolving MY_ROOT_T→MY_API_T::DoThing→Impl_DoThing. */
    function buildMacroTestIndex() {
        const index = createEmptyIndex();
        index.moduleRoots.set('MY_ROOT_T', 'MY_API_T');
        const fieldMap = new Map<string, ConcreteLocation[]>([
            ['DoThing', [{ functionName: 'Impl_DoThing', file: '/impl.c', line: 10 }]],
        ]);
        index.vtableAssignments.set('MY_API_T', fieldMap);
        return index;
    }

    /** Build index resolving JUNO_APP_ROOT_T→JUNO_APP_API_T with OnStart and Run fields. */
    function buildChainTestIndex() {
        const index = createEmptyIndex();
        const localTypeInfo: LocalTypeInfo = {
            localVariables: new Map([
                ['TestFunc', new Map<string, TypeInfo>([
                    ['ptModules', { name: 'ptModules', typeName: 'JUNO_APP_ROOT_T', isPointer: true, isConst: false, isArray: true }],
                    ['ptApp', { name: 'ptApp', typeName: 'JUNO_APP_ROOT_T', isPointer: true, isConst: false, isArray: false }],
                ])],
            ]),
            functionParameters: new Map(),
        };
        index.localTypeInfo.set('/src/a.c', localTypeInfo);
        index.functionDefinitions.set('TestFunc', [
            { functionName: 'TestFunc', file: '/src/a.c', line: 1, isStatic: true },
        ]);
        index.moduleRoots.set('JUNO_APP_ROOT_T', 'JUNO_APP_API_T');
        const fieldMap = new Map<string, ConcreteLocation[]>([
            ['OnStart', [{ functionName: 'App_OnStart', file: '/src/app.c', line: 50 }]],
            ['Run', [{ functionName: 'App_Run', file: '/src/app.c', line: 75 }]],
        ]);
        index.vtableAssignments.set('JUNO_APP_API_T', fieldMap);
        return index;
    }

    it('TC-RES-008: macroRe boundary — cursor at column 0 (match start) should resolve', () => {
        // macroRe match: "JUNO_MODULE_GET_API(ptSelf, MY_ROOT_T)->DoThing(" at index 0, length 48 → range [0, 48)
        // column 0: 0 >= 0 && 0 < 48 → inside match
        const resolver = new VtableResolver(buildMacroTestIndex());
        const result = resolver.resolve('/src/a.c', 10, 0, MACRO_LINE);

        expect(result.found).toBe(true);
        expect(result.locations[0].functionName).toBe('Impl_DoThing');
    });

    it('TC-RES-009: macroRe boundary — cursor at last character of match (column 47) should resolve', () => {
        // macroRe match length is 48; last included column is 47
        // column 47: 47 >= 0 && 47 < 48 → inside match
        const macroMatchLength = 'JUNO_MODULE_GET_API(ptSelf, MY_ROOT_T)->DoThing('.length;
        expect(macroMatchLength).toBe(48); // Verify expected length
        const resolver = new VtableResolver(buildMacroTestIndex());
        const result = resolver.resolve('/src/a.c', 10, macroMatchLength - 1, MACRO_LINE);

        expect(result.found).toBe(true);
        expect(result.locations[0].functionName).toBe('Impl_DoThing');
    });

    it('TC-RES-010: arrayRe boundary — cursor one past match end (column 33) should return found=false', () => {
        // arrayRe matches "ptModules[0]->ptApi->OnStart(" at index 4, length 29 → range [4, 33)
        // generalRe also ends at 33 (matches "ptApi->OnStart(" at index 18, length 15 → range [18, 33))
        // column 33: outside both ranges → no strategy matches
        const arrayMatchEnd = 4 + 'ptModules[0]->ptApi->OnStart('.length;
        expect(arrayMatchEnd).toBe(33); // Verify expected boundary
        const resolver = new VtableResolver(buildChainTestIndex());
        const lineText = '    ptModules[0]->ptApi->OnStart(ptModules[0]);';
        const result = resolver.resolve('/src/a.c', 10, arrayMatchEnd, lineText);

        expect(result.found).toBe(false);
    });

    it('TC-RES-011a: generalRe boundary — cursor at match start (column 4) should resolve', () => {
        // generalRe matches "ptApp->ptApi->Run(" at index 4, length 18 → range [4, 22)
        // column 4: 4 >= 4 && 4 < 22 → inside match
        const resolver = new VtableResolver(buildChainTestIndex());
        const lineText = '    ptApp->ptApi->Run(ptApp);';
        const result = resolver.resolve('/src/a.c', 10, 4, lineText);

        expect(result.found).toBe(true);
        expect(result.locations[0].functionName).toBe('App_Run');
    });

    it('TC-RES-011b: generalRe boundary — cursor at last character of match (column 21) should resolve', () => {
        // match length 18 at index 4 → last included column = 4 + 18 - 1 = 21
        // column 21: 21 >= 4 && 21 < 22 → inside match
        const generalMatchEnd = 4 + 'ptApp->ptApi->Run('.length;
        expect(generalMatchEnd).toBe(22); // Verify expected boundary
        const resolver = new VtableResolver(buildChainTestIndex());
        const lineText = '    ptApp->ptApi->Run(ptApp);';
        const result = resolver.resolve('/src/a.c', 10, generalMatchEnd - 1, lineText);

        expect(result.found).toBe(true);
        expect(result.locations[0].functionName).toBe('App_Run');
    });

    it('TC-RES-011c: generalRe boundary — cursor one past match end (column 22) should return found=false', () => {
        // column 22: 22 >= 4 && 22 < 22 → false → not inside this match
        // No further generalRe match found at column 22 in this line
        const generalMatchEnd = 4 + 'ptApp->ptApi->Run('.length;
        expect(generalMatchEnd).toBe(22); // Verify expected boundary
        const resolver = new VtableResolver(buildChainTestIndex());
        const lineText = '    ptApp->ptApi->Run(ptApp);';
        const result = resolver.resolve('/src/a.c', 10, generalMatchEnd, lineText);

        expect(result.found).toBe(false);
    });
});

// ---------------------------------------------------------------------------
// REQ-VSCODE-002, REQ-VSCODE-006 — Chain Resolution Strategies
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-002", "REQ-VSCODE-006"]}
describe('VtableResolver — Chain Resolution Strategies', () => {
    /**
     * Populate localTypeInfo and functionDefinitions for a single variable
     * declared in "TestFunc" at line 1 in "/src/main.c".
     */
    function buildChainIndex(varName: string, typeName: string) {
        const index = createEmptyIndex();
        index.functionDefinitions.set('TestFunc', [
            { functionName: 'TestFunc', file: '/src/main.c', line: 1, isStatic: false },
        ]);
        const typeInfo: LocalTypeInfo = {
            localVariables: new Map([
                ['TestFunc', new Map([[varName, { name: varName, typeName, isPointer: true, isConst: false, isArray: false }]])],
            ]),
            functionParameters: new Map(),
        };
        index.localTypeInfo.set('/src/main.c', typeInfo);
        return index;
    }

    // @{"verify": ["REQ-VSCODE-002"]}
    it('TC-RES-002: should resolve an array-subscript chain (Strategy 2) to a concrete location', () => {
        const index = buildChainIndex('ptModules', 'JUNO_APP_ROOT_T');
        index.moduleRoots.set('JUNO_APP_ROOT_T', 'JUNO_APP_API_T');
        const fieldMap = new Map<string, ConcreteLocation[]>();
        fieldMap.set('OnStart', [{ functionName: 'App_OnStart', file: '/impl/appImpl.c', line: 50 }]);
        index.vtableAssignments.set('JUNO_APP_API_T', fieldMap);

        const resolver = new VtableResolver(index);
        // Strategy 2 (arrayRe): ptModules[0]->ptApi->OnStart( — cursor at col 4 inside match
        const lineText = '    ptModules[0]->ptApi->OnStart(ptModules[0]);';
        const result = resolver.resolve('/src/main.c', 10, 4, lineText);

        expect(result.found).toBe(true);
        expect(result.locations).toHaveLength(1);
        expect(result.locations[0].functionName).toBe('App_OnStart');
        expect(result.locations[0].file).toBe('/impl/appImpl.c');
        expect(result.locations[0].line).toBe(50);
    });

    // @{"verify": ["REQ-VSCODE-002"]}
    it('TC-RES-003: should resolve a general access chain (Strategy 3) to a concrete location', () => {
        const index = buildChainIndex('ptApp', 'JUNO_APP_ROOT_T');
        index.moduleRoots.set('JUNO_APP_ROOT_T', 'JUNO_APP_API_T');
        const fieldMap = new Map<string, ConcreteLocation[]>();
        fieldMap.set('Run', [{ functionName: 'App_Run', file: '/impl/appImpl.c', line: 60 }]);
        index.vtableAssignments.set('JUNO_APP_API_T', fieldMap);

        const resolver = new VtableResolver(index);
        // Strategy 3 (generalRe): ptApp->ptApi->Run( — cursor at col 4 inside match
        const lineText = '    ptApp->ptApi->Run(ptApp);';
        const result = resolver.resolve('/src/main.c', 10, 4, lineText);

        expect(result.found).toBe(true);
        expect(result.locations).toHaveLength(1);
        expect(result.locations[0].functionName).toBe('App_Run');
        expect(result.locations[0].file).toBe('/impl/appImpl.c');
        expect(result.locations[0].line).toBe(60);
    });

    // @{"verify": ["REQ-VSCODE-006"]}
    it('TC-RES-004: should resolve via apiMemberRegistry when primary API type has no matching field', () => {
        const index = buildChainIndex('ptHeap', 'JUNO_DS_HEAP_ROOT_T');
        index.moduleRoots.set('JUNO_DS_HEAP_ROOT_T', 'JUNO_DS_HEAP_API_T');

        // Primary API type exists but does NOT have "Compare"
        index.vtableAssignments.set('JUNO_DS_HEAP_API_T', new Map());

        // Named API member registry: "ptHeapPointerApi" → secondary API type
        index.apiMemberRegistry.set('ptHeapPointerApi', 'JUNO_DS_HEAP_POINTER_API_T');

        // Secondary API type has the "Compare" implementation
        const pointerApiMap = new Map<string, ConcreteLocation[]>();
        pointerApiMap.set('Compare', [{ functionName: 'Heap_Compare', file: '/impl/heapImpl.c', line: 80 }]);
        index.vtableAssignments.set('JUNO_DS_HEAP_POINTER_API_T', pointerApiMap);

        const resolver = new VtableResolver(index);
        // ptHeap->ptHeapPointerApi->Compare( — generalRe, cursor at col 4
        const lineText = '    ptHeap->ptHeapPointerApi->Compare(ptHeap, a, b);';
        const result = resolver.resolve('/src/main.c', 10, 4, lineText);

        expect(result.found).toBe(true);
        expect(result.locations).toHaveLength(1);
        expect(result.locations[0].functionName).toBe('Heap_Compare');
    });

    // @{"verify": ["REQ-VSCODE-006"]}
    it('TC-RES-005: should report no pattern found when JUNO_MODULE_SUPER macro is used (not a recognized pattern)', () => {
        // Even with apiStructFields + vtableAssignments populated, the resolver
        // must first match a pattern. JUNO_MODULE_SUPER is not matched by any
        // of the three strategies, so the no-pattern error is returned before
        // fieldNameFallback is reached.
        const index = createEmptyIndex();
        index.apiStructFields.set('MY_API_T', ['DoThing']);
        const fieldMap = new Map<string, ConcreteLocation[]>();
        fieldMap.set('DoThing', [{ functionName: 'MyImpl_DoThing', file: '/impl/myImpl.c', line: 30 }]);
        index.vtableAssignments.set('MY_API_T', fieldMap);

        const resolver = new VtableResolver(index);
        // JUNO_MODULE_SUPER is not macroRe, has no array subscript, and the
        // `)->DoThing(` pattern is not matched by generalRe (`)` is not `\w`).
        const lineText = '    JUNO_MODULE_SUPER(ptSelf, MY_ROOT_T)->DoThing(ptSelf);';
        const result = resolver.resolve('/src/main.c', 10, 4, lineText);

        expect(result.found).toBe(false);
        expect(result.errorMsg).toContain('No LibJuno API call pattern found');
    });
});

// ---------------------------------------------------------------------------
// REQ-VSCODE-005, REQ-VSCODE-006 — Multi-Match and Negative Cases
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-005", "REQ-VSCODE-006"]}
describe('VtableResolver — Multi-Match and Negative Cases', () => {
    // @{"verify": ["REQ-VSCODE-005"]}
    it('TC-RES-007: should return found=true with all locations when multiple implementations are registered', () => {
        const index = createEmptyIndex();
        index.moduleRoots.set('MY_ROOT_T', 'MY_API_T');
        const fieldMap = new Map<string, ConcreteLocation[]>();
        fieldMap.set('DoThing', [
            { functionName: 'ImplA_DoThing', file: '/impl/implA.c', line: 10 },
            { functionName: 'ImplB_DoThing', file: '/impl/implB.c', line: 20 },
        ]);
        index.vtableAssignments.set('MY_API_T', fieldMap);

        const resolver = new VtableResolver(index);
        // Reuse the shared MACRO_LINE / CURSOR_IN_MACRO fixture (Strategy 1)
        const result = resolver.resolve('/src/myModule.c', 10, CURSOR_IN_MACRO, MACRO_LINE);

        expect(result.found).toBe(true);
        expect(result.locations).toHaveLength(2);
        expect(result.locations[0].functionName).toBe('ImplA_DoThing');
        expect(result.locations[1].functionName).toBe('ImplB_DoThing');
    });

    // @{"verify": ["REQ-VSCODE-006"]}
    it('TC-RES-NEG-001: should return found=false when the variable type has no moduleRoots or derivationChain entry', () => {
        const index = createEmptyIndex();

        // Enclosing function so localTypeInfo lookup succeeds
        index.functionDefinitions.set('TestFunc', [
            { functionName: 'TestFunc', file: '/src/main.c', line: 1, isStatic: false },
        ]);

        // ptWidget declared as WIDGET_T — not registered in moduleRoots, traitRoots, or derivationChain
        const localTypeInfo: LocalTypeInfo = {
            localVariables: new Map([
                ['TestFunc', new Map([
                    ['ptWidget', { name: 'ptWidget', typeName: 'WIDGET_T', isPointer: true, isConst: false, isArray: false }],
                ])],
            ]),
            functionParameters: new Map(),
        };
        index.localTypeInfo.set('/src/main.c', localTypeInfo);

        const resolver = new VtableResolver(index);
        // generalRe: ptWidget->ptApi->Draw( — cursor at col 4
        const lineText = '    ptWidget->ptApi->Draw(ptWidget);';
        const result = resolver.resolve('/src/main.c', 10, 4, lineText);

        expect(result.found).toBe(false);
        expect(result.errorMsg).toBeTruthy();
    });
});
