/**
 * @file visitor-structs.test.ts
 *
 * Jest tests for the CST visitor's struct-related extraction in the LibJuno
 * VSCode extension — specifically the visitor methods that extract
 * ModuleRootRecord, TraitRootRecord, DerivationRecord, and ApiStructRecord
 * from parsed C source.
 *
 * Test cases: TC-P1-001 through TC-P5-005 (Sections 1–5 of design/test-cases.md)
 */

/// <reference types="jest" />

import { parseFile } from "../visitor";

// ---------------------------------------------------------------------------
// Section 1: JUNO_MODULE_ROOT
// ---------------------------------------------------------------------------

describe("visitStructDefinition", () => {
    describe("JUNO_MODULE_ROOT", () => {
        it("TC-P1-001: extracts rootType and apiType from minimal JUNO_MODULE_ROOT with JUNO_MODULE_EMPTY", () => {
            const src = `struct JUNO_LOG_ROOT_TAG JUNO_MODULE_ROOT(JUNO_LOG_API_T, JUNO_MODULE_EMPTY);`;
            const result = parseFile("/test/file.h", src);
            expect(result.moduleRoots).toHaveLength(1);
            expect(result.moduleRoots[0]).toMatchObject({
                rootType: "JUNO_LOG_ROOT_T",
                apiType: "JUNO_LOG_API_T",
            });
        });

        it("TC-P1-002: extracts rootType and apiType from multi-line JUNO_MODULE_ROOT with extra members", () => {
            const src = `struct JUNO_DS_HEAP_ROOT_TAG JUNO_MODULE_ROOT(JUNO_DS_HEAP_API_T,
    const JUNO_DS_HEAP_POINTER_API_T *ptHeapPointerApi;
    JUNO_DS_ARRAY_ROOT_T *ptHeapArray;
    size_t zLength;
);`;
            const result = parseFile("/test/file.h", src);
            expect(result.moduleRoots).toHaveLength(1);
            expect(result.moduleRoots[0]).toMatchObject({
                rootType: "JUNO_DS_HEAP_ROOT_T",
                apiType: "JUNO_DS_HEAP_API_T",
            });
        });

        it("TC-P1-003: extracts rootType and apiType from broker variant JUNO_MODULE_ROOT", () => {
            const src = `struct JUNO_SB_BROKER_ROOT_TAG JUNO_MODULE_ROOT(JUNO_SB_BROKER_API_T,
    JUNO_DS_ARRAY_ROOT_T *ptTopicArray;
);`;
            const result = parseFile("/test/file.h", src);
            expect(result.moduleRoots).toHaveLength(1);
            expect(result.moduleRoots[0]).toMatchObject({
                rootType: "JUNO_SB_BROKER_ROOT_T",
                apiType: "JUNO_SB_BROKER_API_T",
            });
        });

        it("TC-P1-004: JUNO_MODULE_DERIVE must NOT produce a moduleRoots entry", () => {
            const src = `struct ENGINE_APP_TAG JUNO_MODULE_DERIVE(JUNO_APP_ROOT_T,
    int iState;
);`;
            const result = parseFile("/test/file.h", src);
            expect(result.moduleRoots).toHaveLength(0);
        });
    });

    // -----------------------------------------------------------------------
    // Section 2: JUNO_MODULE_DERIVE
    // -----------------------------------------------------------------------

    describe("JUNO_MODULE_DERIVE", () => {
        it("TC-P2-001: extracts derivedType and rootType from application derivation", () => {
            const src = `struct ENGINE_APP_TAG JUNO_MODULE_DERIVE(JUNO_APP_ROOT_T,
    int iState;
);`;
            const result = parseFile("/test/file.h", src);
            expect(result.derivations).toHaveLength(1);
            expect(result.derivations[0]).toMatchObject({
                derivedType: "ENGINE_APP_T",
                rootType: "JUNO_APP_ROOT_T",
            });
        });

        it("TC-P2-002: extracts derivedType and rootType from pipe derivation from queue root", () => {
            const src = `struct JUNO_SB_PIPE_TAG JUNO_MODULE_DERIVE(JUNO_DS_QUEUE_ROOT_T,
    int iMid;
);`;
            const result = parseFile("/test/file.h", src);
            expect(result.derivations).toHaveLength(1);
            expect(result.derivations[0]).toMatchObject({
                derivedType: "JUNO_SB_PIPE_T",
                rootType: "JUNO_DS_QUEUE_ROOT_T",
            });
        });

        it("TC-P2-003: JUNO_MODULE_ROOT must NOT produce a derivations entry", () => {
            const src = `struct JUNO_LOG_ROOT_TAG JUNO_MODULE_ROOT(JUNO_LOG_API_T, JUNO_MODULE_EMPTY);`;
            const result = parseFile("/test/file.h", src);
            expect(result.derivations).toHaveLength(0);
        });
    });

    // -----------------------------------------------------------------------
    // Section 3: JUNO_TRAIT_ROOT
    // -----------------------------------------------------------------------

    describe("JUNO_TRAIT_ROOT", () => {
        it("TC-P3-001: extracts rootType and apiType from JUNO_TRAIT_ROOT", () => {
            const src = `struct JUNO_POINTER_TAG JUNO_TRAIT_ROOT(JUNO_POINTER_API_T,
    void *pvAddr;
    size_t zSize;
);`;
            const result = parseFile("/test/file.h", src);
            expect(result.traitRoots).toHaveLength(1);
            expect(result.traitRoots[0]).toMatchObject({
                rootType: "JUNO_POINTER_T",
                apiType: "JUNO_POINTER_API_T",
            });
        });

        it("TC-P3-002: JUNO_MODULE_ROOT must NOT produce a traitRoots entry", () => {
            const src = `struct JUNO_LOG_ROOT_TAG JUNO_MODULE_ROOT(JUNO_LOG_API_T, JUNO_MODULE_EMPTY);`;
            const result = parseFile("/test/file.h", src);
            expect(result.traitRoots).toHaveLength(0);
        });

        it("TC-P3-003: JUNO_MODULE_DERIVE must NOT produce a traitRoots entry", () => {
            const src = `struct ENGINE_APP_TAG JUNO_MODULE_DERIVE(JUNO_APP_ROOT_T,
    int iState;
);`;
            const result = parseFile("/test/file.h", src);
            expect(result.traitRoots).toHaveLength(0);
        });
    });

    // -----------------------------------------------------------------------
    // Section 4: JUNO_TRAIT_DERIVE
    // -----------------------------------------------------------------------

    describe("JUNO_TRAIT_DERIVE", () => {
        it("TC-P4-001: extracts derivedType and rootType from synthetic JUNO_TRAIT_DERIVE", () => {
            const src = `struct MY_POINTER_IMPL_TAG JUNO_TRAIT_DERIVE(JUNO_POINTER_T,
    void *pvExtraState;
);`;
            const result = parseFile("/test/file.h", src);
            expect(result.derivations).toHaveLength(1);
            expect(result.derivations[0]).toMatchObject({
                derivedType: "MY_POINTER_IMPL_T",
                rootType: "JUNO_POINTER_T",
            });
        });

        it("TC-P4-002: JUNO_TRAIT_ROOT must NOT produce a derivations entry", () => {
            const src = `struct JUNO_POINTER_TAG JUNO_TRAIT_ROOT(JUNO_POINTER_API_T,
    void *pvAddr;
    size_t zSize;
);`;
            const result = parseFile("/test/file.h", src);
            expect(result.derivations).toHaveLength(0);
        });
    });

    // -----------------------------------------------------------------------
    // Section 5: API Struct Field Extraction
    // -----------------------------------------------------------------------

    describe("API struct field extraction", () => {
        it("TC-P5-001: extracts three fields in order from JUNO_DS_HEAP_API_TAG", () => {
            const src = `struct JUNO_DS_HEAP_API_TAG
{
    JUNO_STATUS_T (*Insert)(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tValue);
    JUNO_STATUS_T (*Heapify)(JUNO_DS_HEAP_ROOT_T *ptHeap);
    JUNO_STATUS_T (*Pop)(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tReturn);
};`;
            const result = parseFile("/test/file.h", src);
            expect(result.apiStructDefinitions).toHaveLength(1);
            expect(result.apiStructDefinitions[0]).toMatchObject({
                apiType: "JUNO_DS_HEAP_API_T",
            });
            expect(result.apiStructDefinitions[0].fields).toEqual(["Insert", "Heapify", "Pop"]);
        });

        it("TC-P5-002: extracts four fields in document order from JUNO_LOG_API_TAG", () => {
            const src = `struct JUNO_LOG_API_TAG
{
    JUNO_STATUS_T (*LogDebug)(const JUNO_LOG_ROOT_T *ptJunoLog, const char *pcMsg, ...);
    JUNO_STATUS_T (*LogInfo)(const JUNO_LOG_ROOT_T *ptJunoLog, const char *pcMsg, ...);
    JUNO_STATUS_T (*LogWarning)(const JUNO_LOG_ROOT_T *ptJunoLog, const char *pcMsg, ...);
    JUNO_STATUS_T (*LogError)(const JUNO_LOG_ROOT_T *ptJunoLog, const char *pcMsg, ...);
};`;
            const result = parseFile("/test/file.h", src);
            expect(result.apiStructDefinitions).toHaveLength(1);
            expect(result.apiStructDefinitions[0]).toMatchObject({
                apiType: "JUNO_LOG_API_T",
            });
            expect(result.apiStructDefinitions[0].fields).toEqual([
                "LogDebug",
                "LogInfo",
                "LogWarning",
                "LogError",
            ]);
        });

        it("TC-P5-003: extracts two fields in order from JUNO_DS_HEAP_POINTER_API_TAG", () => {
            const src = `struct JUNO_DS_HEAP_POINTER_API_TAG
{
    JUNO_DS_HEAP_COMPARE_RESULT_T (*Compare)(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tParent, JUNO_POINTER_T tChild);
    JUNO_STATUS_T (*Swap)(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tLeft, JUNO_POINTER_T tRight);
};`;
            const result = parseFile("/test/file.h", src);
            expect(result.apiStructDefinitions).toHaveLength(1);
            expect(result.apiStructDefinitions[0]).toMatchObject({
                apiType: "JUNO_DS_HEAP_POINTER_API_T",
            });
            expect(result.apiStructDefinitions[0].fields).toEqual(["Compare", "Swap"]);
        });

        it("TC-P5-004: extracts two fields in order from JUNO_POINTER_API_TAG", () => {
            const src = `struct JUNO_POINTER_API_TAG
{
    JUNO_STATUS_T (*Copy)(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
    JUNO_STATUS_T (*Reset)(JUNO_POINTER_T tPointer);
};`;
            const result = parseFile("/test/file.h", src);
            expect(result.apiStructDefinitions).toHaveLength(1);
            expect(result.apiStructDefinitions[0]).toMatchObject({
                apiType: "JUNO_POINTER_API_T",
            });
            expect(result.apiStructDefinitions[0].fields).toEqual(["Copy", "Reset"]);
        });

        it("TC-P5-005: extracts two fields in order from JUNO_SB_BROKER_API_TAG", () => {
            const src = `struct JUNO_SB_BROKER_API_TAG
{
    JUNO_STATUS_T (*Publish)(JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_SB_MID_T tMid, JUNO_POINTER_T tMsg);
    JUNO_STATUS_T (*RegisterSubscriber)(JUNO_SB_BROKER_ROOT_T *ptBroker, JUNO_SB_PIPE_T *ptPipe);
};`;
            const result = parseFile("/test/file.h", src);
            expect(result.apiStructDefinitions).toHaveLength(1);
            expect(result.apiStructDefinitions[0]).toMatchObject({
                apiType: "JUNO_SB_BROKER_API_T",
            });
            expect(result.apiStructDefinitions[0].fields).toEqual(["Publish", "RegisterSubscriber"]);
        });
    });
});
