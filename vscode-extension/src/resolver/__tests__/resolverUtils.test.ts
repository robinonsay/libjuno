/// <reference types="jest" />

/**
 * @file resolverUtils.test.ts
 *
 * Unit tests for the four resolverUtils functions: findEnclosingFunction,
 * lookupVariableType, walkToRootType, parseIntermediates.
 *
 * Test cases: TC-UTIL-001 through TC-UTIL-006, TC-UTIL-NEG-001,
 * TC-UTIL-NEG-002, TC-UTIL-BND-001, TC-UTIL-PRI-001
 */

import {
    findEnclosingFunction,
    lookupVariableType,
    walkToRootType,
    parseIntermediates,
} from '../resolverUtils';
import { createEmptyIndex } from '../../indexer/navigationIndex';
import { LocalTypeInfo, TypeInfo } from '../../parser/types';

// @{"verify": ["REQ-VSCODE-002", "REQ-VSCODE-009", "REQ-VSCODE-015"]}
describe('resolverUtils', () => {

    // =========================================================================
    // findEnclosingFunction
    // =========================================================================

    describe('findEnclosingFunction', () => {

        // TC-UTIL-001: cursor inside function body
        it('TC-UTIL-001: returns the enclosing function when cursor is inside its body', () => {
            const index = createEmptyIndex();
            index.functionDefinitions.set('MyInit', [
                { functionName: 'MyInit', file: '/src/a.c', line: 10, isStatic: true },
            ]);
            index.functionDefinitions.set('MyRun', [
                { functionName: 'MyRun', file: '/src/a.c', line: 30, isStatic: true },
            ]);

            const result = findEnclosingFunction(index, '/src/a.c', 20);

            // line 20 is after MyInit (starts at 10) and before MyRun (starts at 30)
            expect(result).toBe('MyInit');
        });

        // TC-UTIL-002: cursor before any function definition → undefined
        it('TC-UTIL-002: returns undefined when cursor is before all function definitions', () => {
            const index = createEmptyIndex();
            index.functionDefinitions.set('MyInit', [
                { functionName: 'MyInit', file: '/src/a.c', line: 10, isStatic: true },
            ]);

            const result = findEnclosingFunction(index, '/src/a.c', 5);

            expect(result).toBeUndefined();
        });

        // TC-UTIL-BND-001: cursor at exact function definition line
        it('TC-UTIL-BND-001: returns the function when cursor is at the exact definition line', () => {
            const index = createEmptyIndex();
            index.functionDefinitions.set('MyInit', [
                { functionName: 'MyInit', file: '/src/a.c', line: 10, isStatic: true },
            ]);

            const result = findEnclosingFunction(index, '/src/a.c', 10);

            // def.line <= line: 10 <= 10 is true, so MyInit must match
            expect(result).toBe('MyInit');
        });
    });

    // =========================================================================
    // lookupVariableType
    // =========================================================================

    describe('lookupVariableType', () => {

        // TC-UTIL-003: variable found in localVariables (primary path)
        it('TC-UTIL-003: returns TypeInfo from localVariables when variable is declared locally', () => {
            const index = createEmptyIndex();
            index.localTypeInfo.set('/src/a.c', {
                localVariables: new Map([
                    ['MyInit', new Map([
                        ['ptTime', { name: 'ptTime', typeName: 'JUNO_TIME_T', isPointer: true, isConst: false, isArray: false }],
                    ])],
                ]),
                functionParameters: new Map(),
            });

            const result = lookupVariableType(index, '/src/a.c', 'MyInit', 'ptTime');

            expect(result).toBeDefined();
            expect(result!.typeName).toBe('JUNO_TIME_T');
            expect(result!.isPointer).toBe(true);
            expect(result!.name).toBe('ptTime');
        });

        // TC-UTIL-004: variable found in functionParameters (fallback path)
        it('TC-UTIL-004: falls back to functionParameters when localVariables has no match', () => {
            const index = createEmptyIndex();
            index.localTypeInfo.set('/src/a.c', {
                localVariables: new Map([
                    ['MyInit', new Map()], // empty locals — no match here
                ]),
                functionParameters: new Map([
                    ['MyInit', [
                        { name: 'ptApp', typeName: 'JUNO_APP_ROOT_T', isPointer: true, isConst: false, isArray: false },
                    ]],
                ]),
            });

            const result = lookupVariableType(index, '/src/a.c', 'MyInit', 'ptApp');

            expect(result).toBeDefined();
            expect(result!.typeName).toBe('JUNO_APP_ROOT_T');
            expect(result!.name).toBe('ptApp');
            expect(result!.isPointer).toBe(true);
        });

        // TC-UTIL-NEG-001: variable not found in either map → undefined
        it('TC-UTIL-NEG-001: returns undefined when the variable is not in localVariables or functionParameters', () => {
            const index = createEmptyIndex();
            index.localTypeInfo.set('/src/a.c', {
                localVariables: new Map([
                    ['MyInit', new Map()],
                ]),
                functionParameters: new Map([
                    ['MyInit', [
                        { name: 'ptApp', typeName: 'JUNO_APP_ROOT_T', isPointer: true, isConst: false, isArray: false },
                    ]],
                ]),
            });

            const result = lookupVariableType(index, '/src/a.c', 'MyInit', 'ptUnknown');

            expect(result).toBeUndefined();
        });

        it('TC-UTIL-PRI-001: lookupVariableType — localVariables takes priority over functionParameters', () => {
            const index = createEmptyIndex();
            const localTypeInfo: LocalTypeInfo = {
                localVariables: new Map([
                    ['MyFunc', new Map<string, TypeInfo>([
                        ['ptThing', { name: 'ptThing', typeName: 'LOCAL_TYPE_T', isPointer: true, isConst: false, isArray: false }],
                    ])],
                ]),
                functionParameters: new Map([
                    ['MyFunc', [
                        { name: 'ptThing', typeName: 'PARAM_TYPE_T', isPointer: true, isConst: false, isArray: false },
                    ]],
                ]),
            };
            index.localTypeInfo.set('/src/a.c', localTypeInfo);

            const result = lookupVariableType(index, '/src/a.c', 'MyFunc', 'ptThing');
            expect(result).toBeDefined();
            expect(result!.typeName).toBe('LOCAL_TYPE_T'); // localVariables wins over functionParameters
        });
    });

    // =========================================================================
    // walkToRootType
    // =========================================================================

    describe('walkToRootType', () => {

        // TC-UTIL-005a: 3-hop derivation chain
        it('TC-UTIL-005a: walkToRootType — multi-hop derivation chain resolves to root', () => {
            const index = createEmptyIndex();
            index.derivationChain.set('JUNO_DS_HEAP_IMPL_T', 'JUNO_DS_HEAP_DERIVE_T');
            index.derivationChain.set('JUNO_DS_HEAP_DERIVE_T', 'JUNO_DS_HEAP_ROOT_T');
            // JUNO_DS_HEAP_ROOT_T is NOT in the chain → walk stops there

            const result = walkToRootType(index, 'JUNO_DS_HEAP_IMPL_T');

            // IMPL → DERIVE → ROOT → stops (ROOT not a key)
            expect(result).toBe('JUNO_DS_HEAP_ROOT_T');
        });

        // TC-UTIL-005b: cycle detection
        it('TC-UTIL-005b: walkToRootType — cycle in derivation chain terminates', () => {
            const index = createEmptyIndex();
            // Cycle detection: A → B → A → ... must not infinite-loop
            index.derivationChain.set('CYCLE_A_T', 'CYCLE_B_T');
            index.derivationChain.set('CYCLE_B_T', 'CYCLE_A_T');

            const cycleResult = walkToRootType(index, 'CYCLE_A_T');

            // Implementation deterministically returns CYCLE_A_T:
            // visited={} → current=CYCLE_A_T → add → advance to CYCLE_B_T
            // visited={A} → current=CYCLE_B_T → add → advance to CYCLE_A_T
            // visited={A,B} → current=CYCLE_A_T → visited.has(CYCLE_A_T) → exit
            expect(cycleResult).toBe('CYCLE_A_T');
        });

        // TC-UTIL-NEG-002: type not in derivation chain → returns input unchanged
        it('TC-UTIL-NEG-002: returns the input type unchanged when it is not in the derivation chain', () => {
            const index = createEmptyIndex();

            const result = walkToRootType(index, 'UNKNOWN_TYPE_T');

            expect(result).toBe('UNKNOWN_TYPE_T');
        });
    });

    // =========================================================================
    // parseIntermediates
    // =========================================================================

    describe('parseIntermediates', () => {

        // TC-UTIL-006: single, multi, dot, empty inputs
        it('TC-UTIL-006: parses arrow and dot member chains; returns empty array for blank input', () => {
            expect(parseIntermediates('->ptApi')).toEqual(['ptApi']);
            expect(parseIntermediates('->ptBroker->ptApi')).toEqual(['ptBroker', 'ptApi']);
            expect(parseIntermediates('.tOk->ptApi')).toEqual(['tOk', 'ptApi']);
            expect(parseIntermediates('')).toEqual([]);
            expect(parseIntermediates('   ')).toEqual([]);
        });
    });
});
