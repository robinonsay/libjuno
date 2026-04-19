/**
 * @file navigationIndex.test.ts
 *
 * Tests for NavigationIndex CRUD operations: createEmptyIndex, clearIndex,
 * removeFileRecords.
 *
 * Test cases: TC-IDX-001 through TC-IDX-005, TC-IDX-NEG-001, TC-IDX-BND-001
 */

/// <reference types="jest" />

import { createEmptyIndex, clearIndex, removeFileRecords } from "../navigationIndex";
import { ConcreteLocation, FunctionDefinitionRecord, LocalTypeInfo, TypeInfo } from "../../parser/types";

// @{"verify": ["REQ-VSCODE-001"]}
describe("NavigationIndex CRUD", () => {

    // === TC-IDX-001: createEmptyIndex() — all Maps initialized ===
    it("TC-IDX-001: createEmptyIndex() initialises all 9 Maps to empty", () => {
        const index = createEmptyIndex();

        expect(index.moduleRoots).toBeInstanceOf(Map);
        expect(index.moduleRoots.size).toBe(0);

        expect(index.traitRoots).toBeInstanceOf(Map);
        expect(index.traitRoots.size).toBe(0);

        expect(index.derivationChain).toBeInstanceOf(Map);
        expect(index.derivationChain.size).toBe(0);

        expect(index.apiStructFields).toBeInstanceOf(Map);
        expect(index.apiStructFields.size).toBe(0);

        expect(index.vtableAssignments).toBeInstanceOf(Map);
        expect(index.vtableAssignments.size).toBe(0);

        expect(index.failureHandlerAssignments).toBeInstanceOf(Map);
        expect(index.failureHandlerAssignments.size).toBe(0);

        expect(index.apiMemberRegistry).toBeInstanceOf(Map);
        expect(index.apiMemberRegistry.size).toBe(0);

        expect(index.functionDefinitions).toBeInstanceOf(Map);
        expect(index.functionDefinitions.size).toBe(0);

        expect(index.localTypeInfo).toBeInstanceOf(Map);
        expect(index.localTypeInfo.size).toBe(0);
    });

    // === TC-IDX-002: Direct population — add records to all 9 Maps, verify retrieval ===
    it("TC-IDX-002: all 9 Map types accept entries and return them by key", () => {
        const index = createEmptyIndex();

        // moduleRoots: rootType → apiType (string → string)
        index.moduleRoots.set("MY_ROOT_T", "MY_API_T");
        expect(index.moduleRoots.get("MY_ROOT_T")).toBe("MY_API_T");

        // traitRoots: traitRootType → apiType (string → string)
        index.traitRoots.set("MY_TRAIT_T", "MY_TRAIT_API_T");
        expect(index.traitRoots.get("MY_TRAIT_T")).toBe("MY_TRAIT_API_T");

        // derivationChain: derivedType → rootType (string → string)
        index.derivationChain.set("MY_IMPL_T", "MY_ROOT_T");
        expect(index.derivationChain.get("MY_IMPL_T")).toBe("MY_ROOT_T");

        // apiStructFields: apiType → string[] (string → string[])
        index.apiStructFields.set("MY_API_T", ["Init", "Run", "Stop"]);
        const fields = index.apiStructFields.get("MY_API_T");
        expect(fields).toEqual(["Init", "Run", "Stop"]);

        // vtableAssignments: apiType → Map<fieldName, ConcreteLocation[]>
        const loc: ConcreteLocation = { functionName: "MyImpl_Run", file: "/src/my.c", line: 42 };
        const fieldMap = new Map<string, ConcreteLocation[]>();
        fieldMap.set("Run", [loc]);
        index.vtableAssignments.set("MY_API_T", fieldMap);
        const retrieved = index.vtableAssignments.get("MY_API_T");
        expect(retrieved).toBeInstanceOf(Map);
        expect(retrieved!.get("Run")).toHaveLength(1);
        expect(retrieved!.get("Run")![0]).toEqual({ functionName: "MyImpl_Run", file: "/src/my.c", line: 42 });

        // failureHandlerAssignments: rootType → ConcreteLocation[]
        const handlerLoc: ConcreteLocation = { functionName: "MyFailureHandler", file: "/src/my.c", line: 10 };
        index.failureHandlerAssignments.set("MY_ROOT_T", [handlerLoc]);
        const handlers = index.failureHandlerAssignments.get("MY_ROOT_T");
        expect(handlers).toHaveLength(1);
        expect(handlers![0]).toEqual({ functionName: "MyFailureHandler", file: "/src/my.c", line: 10 });

        // apiMemberRegistry: memberName → apiType (string → string)
        index.apiMemberRegistry.set("ptMyApi", "MY_API_T");
        expect(index.apiMemberRegistry.get("ptMyApi")).toBe("MY_API_T");

        // functionDefinitions: functionName → FunctionDefinitionRecord[]
        const fnDef: FunctionDefinitionRecord = { functionName: "MyImpl_Run", file: "/src/my.c", line: 38, isStatic: false };
        index.functionDefinitions.set("MyImpl_Run", [fnDef]);
        const defs = index.functionDefinitions.get("MyImpl_Run");
        expect(defs).toHaveLength(1);
        expect(defs![0]).toEqual({ functionName: "MyImpl_Run", file: "/src/my.c", line: 38, isStatic: false });

        // localTypeInfo: filePath → LocalTypeInfo
        const typeInfo: TypeInfo = { name: "ptRoot", typeName: "MY_ROOT_T", isPointer: true, isConst: false, isArray: false };
        const localInfo: LocalTypeInfo = {
            localVariables: new Map([["MyFunc", new Map([["ptRoot", typeInfo]])]]),
            functionParameters: new Map([["MyFunc", [typeInfo]]]),
        };
        index.localTypeInfo.set("/src/my.c", localInfo);
        const retrieved_local = index.localTypeInfo.get("/src/my.c");
        expect(retrieved_local).toBeDefined();
        expect(retrieved_local!.localVariables.get("MyFunc")!.get("ptRoot")).toEqual(typeInfo);
        expect(retrieved_local!.functionParameters.get("MyFunc")![0]).toEqual(typeInfo);
    });

    // === TC-IDX-003: clearIndex() — all Maps empty after clear ===
    it("TC-IDX-003: clearIndex() empties all 9 Maps", () => {
        const index = createEmptyIndex();

        // Populate a subset of Maps to ensure clear actually removes something
        index.moduleRoots.set("SOME_ROOT_T", "SOME_API_T");
        index.traitRoots.set("SOME_TRAIT_T", "SOME_TRAIT_API_T");
        index.derivationChain.set("SOME_IMPL_T", "SOME_ROOT_T");
        index.apiStructFields.set("SOME_API_T", ["Field1"]);
        const handlerLoc: ConcreteLocation = { functionName: "Handler", file: "/f.c", line: 1 };
        index.vtableAssignments.set("SOME_API_T", new Map([["Field1", [handlerLoc]]]));
        index.failureHandlerAssignments.set("SOME_ROOT_T", [handlerLoc]);
        index.apiMemberRegistry.set("ptSomeApi", "SOME_API_T");
        const fnDef: FunctionDefinitionRecord = { functionName: "Handler", file: "/f.c", line: 1, isStatic: false };
        index.functionDefinitions.set("Handler", [fnDef]);
        const localInfo: LocalTypeInfo = {
            localVariables: new Map(),
            functionParameters: new Map(),
        };
        index.localTypeInfo.set("/f.c", localInfo);

        clearIndex(index);

        expect(index.moduleRoots.size).toBe(0);
        expect(index.traitRoots.size).toBe(0);
        expect(index.derivationChain.size).toBe(0);
        expect(index.apiStructFields.size).toBe(0);
        expect(index.vtableAssignments.size).toBe(0);
        expect(index.failureHandlerAssignments.size).toBe(0);
        expect(index.apiMemberRegistry.size).toBe(0);
        expect(index.functionDefinitions.size).toBe(0);
        expect(index.localTypeInfo.size).toBe(0);
    });

    // === TC-IDX-004: removeFileRecords() — removes records for "/src/a.c", keeps "/src/b.c" ===
    it("TC-IDX-004: removeFileRecords() removes file-keyed entries and filters arrays, preserving other files", () => {
        const index = createEmptyIndex();

        const locA: ConcreteLocation = { functionName: "A_Impl_Run", file: "/src/a.c", line: 5 };
        const locB: ConcreteLocation = { functionName: "B_Impl_Run", file: "/src/b.c", line: 7 };

        // vtableAssignments — both files contribute to the same apiType+field
        const fieldMap = new Map<string, ConcreteLocation[]>();
        fieldMap.set("Run", [locA, locB]);
        index.vtableAssignments.set("SHARED_API_T", fieldMap);

        // failureHandlerAssignments — one entry per rootType
        index.failureHandlerAssignments.set("A_ROOT_T", [locA]);
        index.failureHandlerAssignments.set("B_ROOT_T", [locB]);

        // functionDefinitions — distinct function names per file
        const fnA: FunctionDefinitionRecord = { functionName: "A_Impl_Run", file: "/src/a.c", line: 3, isStatic: false };
        const fnB: FunctionDefinitionRecord = { functionName: "B_Impl_Run", file: "/src/b.c", line: 5, isStatic: false };
        index.functionDefinitions.set("A_Impl_Run", [fnA]);
        index.functionDefinitions.set("B_Impl_Run", [fnB]);

        // localTypeInfo — keyed by file path
        const emptyLocal: LocalTypeInfo = { localVariables: new Map(), functionParameters: new Map() };
        index.localTypeInfo.set("/src/a.c", emptyLocal);
        index.localTypeInfo.set("/src/b.c", emptyLocal);

        removeFileRecords(index, "/src/a.c");

        // vtableAssignments: locA filtered out, locB still present
        const remaining = index.vtableAssignments.get("SHARED_API_T");
        expect(remaining).toBeDefined();
        const runLocs = remaining!.get("Run");
        expect(runLocs).toHaveLength(1);
        expect(runLocs![0].file).toBe("/src/b.c");

        // failureHandlerAssignments: A_ROOT_T entry gone, B_ROOT_T still present
        expect(index.failureHandlerAssignments.has("A_ROOT_T")).toBe(false);
        expect(index.failureHandlerAssignments.has("B_ROOT_T")).toBe(true);

        // functionDefinitions: A_Impl_Run gone, B_Impl_Run still present
        expect(index.functionDefinitions.has("A_Impl_Run")).toBe(false);
        expect(index.functionDefinitions.has("B_Impl_Run")).toBe(true);
        expect(index.functionDefinitions.get("B_Impl_Run")![0].file).toBe("/src/b.c");

        // localTypeInfo: /src/a.c deleted, /src/b.c still present
        expect(index.localTypeInfo.has("/src/a.c")).toBe(false);
        expect(index.localTypeInfo.has("/src/b.c")).toBe(true);
    });

    // === TC-IDX-005: removeFileRecords() — stale entries in flat maps are NOT pruned ===
    //
    // Per design milestone M4: the flat string→string Maps (moduleRoots, traitRoots,
    // derivationChain, apiStructFields, apiMemberRegistry) do not carry a file pointer.
    // removeFileRecords() cannot identify which entries originated from a given file, so
    // these entries are left in place (stale) and will be overwritten on the next full
    // re-index of the file. This is known, documented behavior — NOT a bug.
    it("TC-IDX-005: removeFileRecords() does NOT prune flat-map entries (stale entries remain)", () => {
        const index = createEmptyIndex();

        // Populate all 5 flat maps with entries conceptually sourced from /src/a.c
        index.moduleRoots.set("A_ROOT_T", "A_API_T");
        index.traitRoots.set("A_TRAIT_T", "A_TRAIT_API_T");
        index.derivationChain.set("A_DERIVED_T", "A_ROOT_T");
        index.apiStructFields.set("A_API_T", ["Field1", "Field2"]);
        index.apiMemberRegistry.set("Field1", "A_API_T");

        removeFileRecords(index, "/src/a.c");

        // All 5 flat-map entries must still be present after removal
        expect(index.moduleRoots.get("A_ROOT_T")).toBe("A_API_T");
        expect(index.traitRoots.get("A_TRAIT_T")).toBe("A_TRAIT_API_T");
        expect(index.derivationChain.get("A_DERIVED_T")).toBe("A_ROOT_T");
        expect(index.apiStructFields.get("A_API_T")).toEqual(["Field1", "Field2"]);
        expect(index.apiMemberRegistry.get("Field1")).toBe("A_API_T");
    });

    // === TC-IDX-NEG-001: removeFileRecords() with unknown file path — no-op, no error ===
    it("TC-IDX-NEG-001: removeFileRecords() is a no-op when the file path is not in the index", () => {
        const index = createEmptyIndex();

        // Populate with entries from "/src/known.c"
        index.vtableAssignments.set("SOME_API_T", new Map([
            ["DoThing", [{ functionName: "Impl", file: "/src/known.c", line: 10 }]]
        ]));
        index.functionDefinitions.set("Impl", [
            { functionName: "Impl", file: "/src/known.c", line: 10, isStatic: false }
        ]);

        // Remove a file that was NEVER indexed — must not throw
        expect(() => removeFileRecords(index, "/src/unknown.c")).not.toThrow();

        // All existing entries remain untouched
        expect(index.vtableAssignments.get("SOME_API_T")?.get("DoThing")).toHaveLength(1);
        expect(index.functionDefinitions.get("Impl")).toHaveLength(1);
    });

    // === TC-IDX-BND-001: Index boundary — pre-existing entry survives empty add ===
    it("TC-IDX-BND-001: pre-existing moduleRoots entry is intact when no new data is written", () => {
        const index = createEmptyIndex();

        // Pre-populate with one entry
        index.moduleRoots.set("EXISTING_ROOT_T", "EXISTING_API_T");

        // Verify the pre-existing entry survives when no new data is added
        expect(index.moduleRoots.size).toBe(1);
        expect(index.moduleRoots.get("EXISTING_ROOT_T")).toBe("EXISTING_API_T");

        // All other maps remain at size 0
        expect(index.vtableAssignments.size).toBe(0);
        expect(index.failureHandlerAssignments.size).toBe(0);
        expect(index.functionDefinitions.size).toBe(0);
    });
});
