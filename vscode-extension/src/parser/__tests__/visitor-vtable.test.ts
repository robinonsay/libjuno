/**
 * @file visitor-vtable.test.ts
 *
 * Jest tests for the CST visitor's vtable-related extraction in the LibJuno
 * VSCode extension — specifically vtable assignment records from designated
 * initializers, direct assignments, and positional initializers.
 *
 * Test cases: TC-P6-001 through TC-P8-004 (Sections 6–8 of design/test-cases.md)
 *
 * VtableAssignmentRecord shape (from types.ts):
 *   { apiType, field, functionName, file, line }
 */

/// <reference types="jest" />

import { parseFile } from "../visitor";

// ---------------------------------------------------------------------------
// Section 6: visitVtableDeclaration — Designated Initializer (REQ-VSCODE-010)
// ---------------------------------------------------------------------------

describe("visitVtableDeclaration", () => {
    describe("Designated Initializer (Section 6)", () => {

        // -------------------------------------------------------------------
        // TC-P6-001: 3-field designated initializer — JUNO_APP_API_T
        // -------------------------------------------------------------------

        it("TC-P6-001: extracts 3 designated-initializer vtable records from JUNO_APP_API_T", () => {
            const src = `
static const JUNO_APP_API_T tEngineAppApi = {
    .OnStart = OnStart,
    .OnProcess = OnProcess,
    .OnExit = OnExit
};
`;
            const result = parseFile("/test/engine_app.c", src);

            expect(result.vtableAssignments).toHaveLength(3);
            expect(result.vtableAssignments[0]).toMatchObject({
                apiType: "JUNO_APP_API_T",
                field: "OnStart",
                functionName: "OnStart",
            });
            expect(result.vtableAssignments[1]).toMatchObject({
                apiType: "JUNO_APP_API_T",
                field: "OnProcess",
                functionName: "OnProcess",
            });
            expect(result.vtableAssignments[2]).toMatchObject({
                apiType: "JUNO_APP_API_T",
                field: "OnExit",
                functionName: "OnExit",
            });
        });

        // -------------------------------------------------------------------
        // TC-P6-002: 2-field designated initializer — JUNO_DS_HEAP_POINTER_API_T
        // -------------------------------------------------------------------

        it("TC-P6-002: extracts 2 designated-initializer vtable records from JUNO_DS_HEAP_POINTER_API_T", () => {
            const src = `
static const JUNO_DS_HEAP_POINTER_API_T tHeapPointerApi = {
    .Compare = HeapCompare,
    .Swap = HeapSwap,
};
`;
            const result = parseFile("/test/heap.c", src);

            expect(result.vtableAssignments).toHaveLength(2);
            expect(result.vtableAssignments[0]).toMatchObject({
                apiType: "JUNO_DS_HEAP_POINTER_API_T",
                field: "Compare",
                functionName: "HeapCompare",
            });
            expect(result.vtableAssignments[1]).toMatchObject({
                apiType: "JUNO_DS_HEAP_POINTER_API_T",
                field: "Swap",
                functionName: "HeapSwap",
            });
        });

        // -------------------------------------------------------------------
        // TC-P6-003: 2-field designated initializer — JUNO_POINTER_API_T
        // -------------------------------------------------------------------

        it("TC-P6-003: extracts 2 designated-initializer vtable records from JUNO_POINTER_API_T", () => {
            const src = `
static const JUNO_POINTER_API_T tPointerApi = {
    .Copy = PointerCopy,
    .Reset = PointerReset,
};
`;
            const result = parseFile("/test/pointer.c", src);

            expect(result.vtableAssignments).toHaveLength(2);
            expect(result.vtableAssignments[0]).toMatchObject({
                apiType: "JUNO_POINTER_API_T",
                field: "Copy",
                functionName: "PointerCopy",
            });
            expect(result.vtableAssignments[1]).toMatchObject({
                apiType: "JUNO_POINTER_API_T",
                field: "Reset",
                functionName: "PointerReset",
            });
        });

        // -------------------------------------------------------------------
        // TC-P6-NEG-001: non-API type does not produce vtableAssignments
        // -------------------------------------------------------------------

        it("TC-P6-NEG-001: non-API-type const variable does not produce vtableAssignments", () => {
            // JUNO_DS_HEAP_ROOT_T does not end in _API_T → walkVtableDeclaration is never entered
            const src = `
static const JUNO_DS_HEAP_ROOT_T tHeap = { 0 };
`;
            const result = parseFile("/test/file.c", src);
            expect(result.vtableAssignments).toHaveLength(0);
        });

        // -------------------------------------------------------------------
        // TC-P6-META: file and line metadata are recorded correctly
        // -------------------------------------------------------------------

        it("TC-P6-META: vtableAssignment record carries correct file path and 1-based line number", () => {
            const src =
`static const JUNO_APP_API_T tApi = {
    .OnStart = MyStart,
};`;
            const result = parseFile("/test/meta.c", src);

            expect(result.vtableAssignments).toHaveLength(1);
            expect(result.vtableAssignments[0].file).toBe("/test/meta.c");
            // The assignment is on line 2 of the source text
            expect(result.vtableAssignments[0].line).toBe(2);
        });
    });

    // ---------------------------------------------------------------------------
    // Section 7: visitVtableDeclaration — Direct Assignment (REQ-VSCODE-011)
    // ---------------------------------------------------------------------------

    describe("Direct Assignment (Section 7)", () => {

        // -------------------------------------------------------------------
        // TC-P7-001: 3 direct assignments via function-parameter API pointer
        // -------------------------------------------------------------------

        it("TC-P7-001: extracts 3 direct vtable assignments via API pointer function parameter", () => {
            // ptApi is a JUNO_DS_HEAP_API_T* parameter; visitor resolves its type
            // from functionParameters localTypeInfo and emits one record per field.
            const src = `
static void SomeInit(JUNO_DS_HEAP_API_T *ptApi) {
    ptApi->Insert = HeapInsert;
    ptApi->Heapify = HeapHeapify;
    ptApi->Pop = HeapPop;
}
`;
            const result = parseFile("/test/heap.c", src);

            expect(result.vtableAssignments).toHaveLength(3);
            expect(result.vtableAssignments[0]).toMatchObject({
                apiType: "JUNO_DS_HEAP_API_T",
                field: "Insert",
                functionName: "HeapInsert",
            });
            expect(result.vtableAssignments[1]).toMatchObject({
                apiType: "JUNO_DS_HEAP_API_T",
                field: "Heapify",
                functionName: "HeapHeapify",
            });
            expect(result.vtableAssignments[2]).toMatchObject({
                apiType: "JUNO_DS_HEAP_API_T",
                field: "Pop",
                functionName: "HeapPop",
            });
        });

        // -------------------------------------------------------------------
        // TC-P7-002: single direct assignment via function-parameter API pointer
        // -------------------------------------------------------------------

        it("TC-P7-002: extracts single direct vtable assignment via API pointer function parameter", () => {
            const src = `
static void InitBroker(JUNO_SB_BROKER_API_T *ptApi) {
    ptApi->Publish = BrokerPublish;
}
`;
            const result = parseFile("/test/broker.c", src);

            expect(result.vtableAssignments).toHaveLength(1);
            expect(result.vtableAssignments[0]).toMatchObject({
                apiType: "JUNO_SB_BROKER_API_T",
                field: "Publish",
                functionName: "BrokerPublish",
            });
        });

        // -------------------------------------------------------------------
        // TC-P7-003: negative — dereference assignment to non-API type pointer
        // -------------------------------------------------------------------

        it("TC-P7-003: dereference assignment (*ptr = value) does not produce vtableAssignments", () => {
            // *ptr = 42 is a unary dereference, not a member access;
            // tryExtractDirectVtableAssign finds no memberIdentifier nodes.
            const src = `
static void foo(int *ptr) {
    *ptr = 42;
}
`;
            const result = parseFile("/test/file.c", src);
            expect(result.vtableAssignments).toHaveLength(0);
        });

        // -------------------------------------------------------------------
        // TC-P7-NEG-001: direct assignment to non-API-type local variable field
        // -------------------------------------------------------------------

        it("TC-P7-NEG-001: member assignment on non-API-type variable does not produce vtableAssignments", () => {
            // tRoot is a JUNO_DS_HEAP_ROOT_T* (does not end in _API_T),
            // so the type check inside tryExtractDirectVtableAssign rejects it.
            const src = `
static void Setup(JUNO_DS_HEAP_ROOT_T *tRoot) {
    tRoot->zLength = 0;
}
`;
            const result = parseFile("/test/file.c", src);
            expect(result.vtableAssignments).toHaveLength(0);
        });
    });

    // ---------------------------------------------------------------------------
    // Section 8: visitVtableDeclaration — Positional Initializer (REQ-VSCODE-012)
    // ---------------------------------------------------------------------------

    describe("Positional Initializer (Section 8)", () => {

        // -------------------------------------------------------------------
        // TC-P8-001: positional without API struct definition — deferred, no records
        // -------------------------------------------------------------------

        it("TC-P8-001: positional initializer without prior API struct definition produces no records (deferred)", () => {
            // The visitor's extractPositionalVtable returns early when apiStructDefinitions
            // does not yet contain the target type. Cross-file resolution is handled by the
            // Workspace Indexer after all files are parsed.
            const src = `
static const JUNO_APP_API_T tAppApi = {
    EngineOnStart,
    EngineOnLoop,
    EngineOnStop,
};
`;
            const result = parseFile("/test/deferred.c", src);
            expect(result.vtableAssignments).toHaveLength(0);
        });

        // -------------------------------------------------------------------
        // TC-P8-002: positional with API struct definition in same source file
        // -------------------------------------------------------------------

        it("TC-P8-002: positional initializer with struct definition in same source resolves all slots", () => {
            // The struct definition precedes the variable declaration; the walker
            // populates apiStructDefinitions on the first pass, so the positional
            // zip succeeds for all three fields.
            const src = `
struct JUNO_DS_HEAP_API_TAG
{
    JUNO_STATUS_T (*Insert)(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tValue);
    JUNO_STATUS_T (*Heapify)(JUNO_DS_HEAP_ROOT_T *ptHeap);
    JUNO_STATUS_T (*Pop)(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tReturn);
};

static const JUNO_DS_HEAP_API_T tHeapApi = {
    JunoDs_Heap_Insert,
    JunoDs_Heap_Heapify,
    JunoDs_Heap_Pop,
};
`;
            const result = parseFile("/test/juno_heap.c", src);

            expect(result.vtableAssignments).toHaveLength(3);
            expect(result.vtableAssignments[0]).toMatchObject({
                apiType: "JUNO_DS_HEAP_API_T",
                field: "Insert",
                functionName: "JunoDs_Heap_Insert",
            });
            expect(result.vtableAssignments[1]).toMatchObject({
                apiType: "JUNO_DS_HEAP_API_T",
                field: "Heapify",
                functionName: "JunoDs_Heap_Heapify",
            });
            expect(result.vtableAssignments[2]).toMatchObject({
                apiType: "JUNO_DS_HEAP_API_T",
                field: "Pop",
                functionName: "JunoDs_Heap_Pop",
            });
        });

        // -------------------------------------------------------------------
        // TC-P8-002b: positional — 2 fields, broker API
        // -------------------------------------------------------------------

        it("TC-P8-002b: positional initializer resolves 2 broker API fields from adjacent struct definition", () => {
            const src = `
struct JUNO_SB_BROKER_API_TAG
{
    JUNO_STATUS_T (*Publish)(JUNO_SB_BROKER_ROOT_T *ptBroker, int iMid, const JUNO_POINTER_T *ptMsg);
    JUNO_STATUS_T (*RegisterSubscriber)(JUNO_SB_BROKER_ROOT_T *ptBroker, int iMid, JUNO_DS_ARRAY_ROOT_T *ptArray);
};

static const JUNO_SB_BROKER_API_T gtBrokerApi =
{
    Publish,
    RegisterSubscriber
};
`;
            const result = parseFile("/test/juno_broker.c", src);

            expect(result.vtableAssignments).toHaveLength(2);
            expect(result.vtableAssignments[0]).toMatchObject({
                apiType: "JUNO_SB_BROKER_API_T",
                field: "Publish",
                functionName: "Publish",
            });
            expect(result.vtableAssignments[1]).toMatchObject({
                apiType: "JUNO_SB_BROKER_API_T",
                field: "RegisterSubscriber",
                functionName: "RegisterSubscriber",
            });
        });

        // -------------------------------------------------------------------
        // TC-P8-003: mixed designated and positional — only designated emitted
        // -------------------------------------------------------------------

        it("TC-P8-003: mixed designated and positional initializer emits only designated entries", () => {
            // extractDesignatedVtable is called (designations.length > 0).
            // Un-designated (positional) slots are skipped — `if (!desig) { continue; }`.
            // The positional branch is NOT entered, so OnLoop is not emitted.
            const src = `
static const JUNO_APP_API_T tAppApi = {
    .OnStart = OnStart,
    OnLoop,
};
`;
            const result = parseFile("/test/mixed.c", src);

            // Designated entry is present
            const designatedEntries = result.vtableAssignments.filter(r => r.field === "OnStart");
            expect(designatedEntries).toHaveLength(1);
            expect(designatedEntries[0]).toMatchObject({
                apiType: "JUNO_APP_API_T",
                field: "OnStart",
                functionName: "OnStart",
            });

            // Positional entry is NOT double-counted
            const positionalEntries = result.vtableAssignments.filter(r => r.field === "OnLoop");
            expect(positionalEntries).toHaveLength(0);
        });

        // -------------------------------------------------------------------
        // TC-P8-004: empty initializer block — no records
        // -------------------------------------------------------------------

        it("TC-P8-004: empty initializer block produces no vtableAssignments", () => {
            // initializerList has no initializers; both designated and positional
            // paths produce zero iterations.
            const src = `
static const JUNO_APP_API_T tAppApi = { };
`;
            const result = parseFile("/test/empty.c", src);
            expect(result.vtableAssignments).toHaveLength(0);
        });

        // -------------------------------------------------------------------
        // TC-P8-NEG-001: designated initializer does NOT fall through to positional path
        // -------------------------------------------------------------------

        it("TC-P8-NEG-001: designated initializer block does not invoke positional path", () => {
            // Regression: when designations.length > 0, extractDesignatedVtable is called
            // and extractPositionalVtable is never invoked — so no double-counting.
            const src = `
struct JUNO_APP_API_TAG
{
    JUNO_STATUS_T (*OnStart)(JUNO_APP_ROOT_T *ptApp);
    JUNO_STATUS_T (*OnProcess)(JUNO_APP_ROOT_T *ptApp);
    JUNO_STATUS_T (*OnExit)(JUNO_APP_ROOT_T *ptApp);
};

static const JUNO_APP_API_T tEngineAppApi = {
    .OnStart = OnStart,
    .OnProcess = OnProcess,
    .OnExit = OnExit
};
`;
            const result = parseFile("/test/engine_app.c", src);

            // Exactly 3 records — one per designated field, no positional duplicates
            expect(result.vtableAssignments).toHaveLength(3);
            expect(result.vtableAssignments[0]).toMatchObject({ field: "OnStart",   functionName: "OnStart"   });
            expect(result.vtableAssignments[1]).toMatchObject({ field: "OnProcess", functionName: "OnProcess" });
            expect(result.vtableAssignments[2]).toMatchObject({ field: "OnExit",    functionName: "OnExit"    });
        });

        // -------------------------------------------------------------------
        // TC-P8-BND-001: single-field positional initializer produces one vtable record
        // -------------------------------------------------------------------

        it("TC-P8-BND-001: single-field positional initializer produces one vtable record", () => {
            // Boundary case: the field-zip algorithm must correctly produce exactly one
            // (field, functionName) pair when the API struct has only one field.
            const src = `
typedef struct FOO_API_TAG {
    JUNO_STATUS_T (*DoThing)(void *pvSelf);
} FOO_API_T;

static const FOO_API_T tFooApi = {
    FooDoThing
};
`;
            const result = parseFile("/test/foo.c", src);

            expect(result.vtableAssignments).toHaveLength(1);
            expect(result.vtableAssignments[0]).toMatchObject({
                apiType: "FOO_API_T",
                field: "DoThing",
                functionName: "FooDoThing",
            });
        });
    });
});
