/// <reference types="jest" />

import { VtableResolver } from '../../resolver/vtableResolver';
import { createEmptyIndex } from '../../indexer/navigationIndex';
import { ConcreteLocation } from '../../parser/types';

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
    it('should return found=false with errorMsg when no API call pattern matches at cursor', () => {
        const index = createEmptyIndex();
        const resolver = new VtableResolver(index);

        // Plain assignment line — no JUNO_MODULE_GET_API or -> call pattern
        const result = resolver.resolve('/src/myModule.c', 10, 5, '    ptSelf->someField = 0;');

        expect(result.found).toBe(false);
        expect(result.locations).toHaveLength(0);
        expect(result.errorMsg).toBeTruthy();
        expect(result.errorMsg).toContain('No LibJuno API call pattern found');
    });

    it('should return found=false with errorMsg when the macro pattern matches but the root type has no registered API type', () => {
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

    it('should return found=false with errorMsg when the API type exists but has no vtable assignments at all', () => {
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

    it('should return found=false with errorMsg when the API type is registered and has assignments but the specific field is absent', () => {
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

    it('should return found=true with exactly one location when a single vtable assignment exists', () => {
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

    it('should return the ConcreteLocation with the exact file, line, and functionName from the index', () => {
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
