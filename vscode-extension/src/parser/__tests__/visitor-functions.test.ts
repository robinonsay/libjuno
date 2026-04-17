/**
 * @file visitor-functions.test.ts
 *
 * Jest tests for the CST visitor's function definition extraction and failure
 * handler assignment extraction in the LibJuno VSCode extension.
 *
 * Test cases: TC-P11-001 through TC-P11-005 (function definitions)
 *             TC-P10-001 through TC-P10-007 (failure handler assignments)
 *
 * FunctionDefinitionRecord shape (from types.ts):
 *   { functionName, file, line, isStatic }
 * Parameters live in parsed.localTypeInfo.functionParameters (Map<string, TypeInfo[]>).
 * TypeInfo shape: { name, typeName, isPointer, isConst, isArray }
 *
 * FailureHandlerRecord shape (from types.ts):
 *   { rootType, functionName, file, line }
 * Lives in ParsedFile.failureHandlerAssigns.
 */

/// <reference types="jest" />

import { parseFile, parseFileWithDefs } from "../visitor";

// ---------------------------------------------------------------------------
// Section 11: Function Definition Extraction (TC-P11-001 through TC-P11-005)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("Function Definition Extraction (Section 11)", () => {

    // -----------------------------------------------------------------------
    // TC-P11-001: Simple function — no parameters (void param → empty list)
    // -----------------------------------------------------------------------

    it("TC-P11-001: extracts simple no-param function definition", () => {
        const src =
            "static void MyInit(void)\n" +
            "{\n" +
            "    return;\n" +
            "}";

        const { parsed, functionDefs } = parseFileWithDefs("/test/file.c", src);

        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("MyInit");
        expect(functionDefs[0].file).toBe("/test/file.c");
        expect(functionDefs[0].isStatic).toBe(true);
        expect(functionDefs[0].line).toBeGreaterThan(0);

        // void as sole parameter: typeName is empty (primitive), extractTypeInfoFromParamDecl
        // returns null → parameters list is empty.
        const params = parsed.localTypeInfo.functionParameters.get("MyInit");
        expect(params).toBeDefined();
        expect(params!).toHaveLength(0);
    });

    // -----------------------------------------------------------------------
    // TC-P11-002: Function with one typed pointer parameter
    // -----------------------------------------------------------------------

    it("TC-P11-002: extracts function definition with one parameter", () => {
        const src =
            "static JUNO_STATUS_T OnStart(JUNO_APP_ROOT_T *ptApp)\n" +
            "{\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}";

        const { parsed, functionDefs } = parseFileWithDefs("/test/file.c", src);

        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("OnStart");
        expect(functionDefs[0].isStatic).toBe(true);

        const params = parsed.localTypeInfo.functionParameters.get("OnStart");
        expect(params).toBeDefined();
        expect(params!).toHaveLength(1);
        expect(params![0].name).toBe("ptApp");
        expect(params![0].typeName).toBe("JUNO_APP_ROOT_T");
        expect(params![0].isPointer).toBe(true);
    });

    // -----------------------------------------------------------------------
    // TC-P11-003: Function with multiple parameters — 3 params extracted
    // -----------------------------------------------------------------------

    it("TC-P11-003: extracts function definition with multiple parameters", () => {
        const src =
            "static JUNO_STATUS_T HeapCompare(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tParent, JUNO_POINTER_T tChild)\n" +
            "{\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}";

        const { parsed, functionDefs } = parseFileWithDefs("/test/file.c", src);

        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("HeapCompare");

        const params = parsed.localTypeInfo.functionParameters.get("HeapCompare");
        expect(params).toBeDefined();
        expect(params!).toHaveLength(3);

        expect(params![0].name).toBe("ptHeap");
        expect(params![0].typeName).toBe("JUNO_DS_HEAP_ROOT_T");
        expect(params![0].isPointer).toBe(true);

        expect(params![1].name).toBe("tParent");
        expect(params![1].typeName).toBe("JUNO_POINTER_T");

        expect(params![2].name).toBe("tChild");
        expect(params![2].typeName).toBe("JUNO_POINTER_T");
    });

    // -----------------------------------------------------------------------
    // TC-P11-004: Multiple functions in one file — all 3 definitions captured
    // -----------------------------------------------------------------------

    it("TC-P11-004: extracts all function definitions from a multi-function file", () => {
        const src =
            "static JUNO_STATUS_T OnStart(JUNO_APP_ROOT_T *ptApp)\n" +
            "{\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}\n" +
            "\n" +
            "static JUNO_STATUS_T OnLoop(JUNO_APP_ROOT_T *ptApp)\n" +
            "{\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}\n" +
            "\n" +
            "static JUNO_STATUS_T OnStop(JUNO_APP_ROOT_T *ptApp)\n" +
            "{\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}";

        const { functionDefs } = parseFileWithDefs("/test/file.c", src);

        expect(functionDefs).toHaveLength(3);
        const names = functionDefs.map((f) => f.functionName);
        expect(names).toContain("OnStart");
        expect(names).toContain("OnLoop");
        expect(names).toContain("OnStop");

        // Each record must carry the correct file path
        for (const def of functionDefs) {
            expect(def.file).toBe("/test/file.c");
            expect(def.line).toBeGreaterThan(0);
            expect(def.isStatic).toBe(true);
        }
    });

    // -----------------------------------------------------------------------
    // TC-P11-005: Negative — forward declaration must NOT produce a functionDef
    // -----------------------------------------------------------------------

    it("TC-P11-005: forward declaration with semicolon produces no function definition", () => {
        // A declaration ending with ';' is parsed as 'declaration', not
        // 'functionDefinition', so _functionDefs must remain empty.
        const src = "static JUNO_STATUS_T OnStart(JUNO_APP_ROOT_T *ptApp);";

        const { functionDefs } = parseFileWithDefs("/test/file.c", src);

        expect(functionDefs).toHaveLength(0);
    });
});

// ---------------------------------------------------------------------------
// Section 10: Failure Handler Assignment Extraction (TC-P10-001 through TC-P10-007)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-016"]}
describe("Failure Handler Assignment Extraction (Section 10)", () => {

    // -----------------------------------------------------------------------
    // TC-P10-001: JUNO_FAILURE_HANDLER member assignment (macro token form)
    // -----------------------------------------------------------------------

    it("TC-P10-001: extracts JUNO_FAILURE_HANDLER assignment record", () => {
        const src =
            "static JUNO_STATUS_T SomeInit(JUNO_APP_ROOT_T *ptApp)\n" +
            "{\n" +
            "    ptApp->tApi.JUNO_FAILURE_HANDLER = AppFailureHandler;\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}";

        const result = parseFile("/test/file.c", src);

        expect(result.failureHandlerAssigns).toHaveLength(1);
        expect(result.failureHandlerAssigns[0].functionName).toBe("AppFailureHandler");
        expect(result.failureHandlerAssigns[0].file).toBe("/test/file.c");
        expect(result.failureHandlerAssigns[0].line).toBeGreaterThan(0);
    });

    // -----------------------------------------------------------------------
    // TC-P10-002: _pfcnFailureHandler member name (expanded form)
    // -----------------------------------------------------------------------

    it("TC-P10-002: extracts _pfcnFailureHandler assignment record", () => {
        const src =
            "static JUNO_STATUS_T SomeInit(JUNO_APP_ROOT_T *ptApp)\n" +
            "{\n" +
            "    ptApp->tApi._pfcnFailureHandler = AppFailureHandler;\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}";

        const result = parseFile("/test/file.c", src);

        expect(result.failureHandlerAssigns).toHaveLength(1);
        expect(result.failureHandlerAssigns[0].functionName).toBe("AppFailureHandler");
        expect(result.failureHandlerAssigns[0].file).toBe("/test/file.c");
    });

    // -----------------------------------------------------------------------
    // TC-P10-003: Pointer chain assignment — deep member access
    // -----------------------------------------------------------------------

    it("TC-P10-003: extracts failure handler from deep pointer chain", () => {
        const src =
            "static JUNO_STATUS_T HeapInit(JUNO_DS_HEAP_ROOT_T *ptHeap)\n" +
            "{\n" +
            "    ptHeap->tBase.tApi.JUNO_FAILURE_HANDLER = HeapFailure;\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}";

        const result = parseFile("/test/file.c", src);

        expect(result.failureHandlerAssigns).toHaveLength(1);
        expect(result.failureHandlerAssigns[0].functionName).toBe("HeapFailure");
        expect(result.failureHandlerAssigns[0].file).toBe("/test/file.c");
    });

    // -----------------------------------------------------------------------
    // TC-P10-004: Multiple failure handlers in same function — 2 records
    // -----------------------------------------------------------------------

    it("TC-P10-004: extracts two failure handler assignments from the same function", () => {
        const src =
            "static JUNO_STATUS_T InitAll(JUNO_APP_ROOT_T *ptApp, JUNO_DS_HEAP_ROOT_T *ptHeap)\n" +
            "{\n" +
            "    ptApp->tApi.JUNO_FAILURE_HANDLER = AppFailureHandler;\n" +
            "    ptHeap->tBase.tApi.JUNO_FAILURE_HANDLER = HeapFailure;\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}";

        const result = parseFile("/test/file.c", src);

        expect(result.failureHandlerAssigns).toHaveLength(2);

        const handlers = result.failureHandlerAssigns.map((r) => r.functionName);
        expect(handlers).toContain("AppFailureHandler");
        expect(handlers).toContain("HeapFailure");

        // Both records must carry the correct file path
        for (const rec of result.failureHandlerAssigns) {
            expect(rec.file).toBe("/test/file.c");
            expect(rec.line).toBeGreaterThan(0);
        }
    });

    // -----------------------------------------------------------------------
    // TC-P10-005: Negative — function with no failure handler assignment
    // -----------------------------------------------------------------------

    it("TC-P10-005: produces no failure handler record when none is assigned", () => {
        const src =
            "static JUNO_STATUS_T OnStart(JUNO_APP_ROOT_T *ptApp)\n" +
            "{\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}";

        const result = parseFile("/test/file.c", src);

        expect(result.failureHandlerAssigns).toHaveLength(0);
    });

    // -----------------------------------------------------------------------
    // TC-P10-006: Negative — regular vtable member assignment is not a failure handler
    // -----------------------------------------------------------------------

    it("TC-P10-006: regular member assignment does not produce a failure handler record", () => {
        const src =
            "static JUNO_STATUS_T SomeInit(JUNO_APP_ROOT_T *ptApp)\n" +
            "{\n" +
            "    ptApp->tApi.OnStart = SomeFunction;\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}";

        const result = parseFile("/test/file.c", src);

        // OnStart is a vtable field, not a failure handler member
        expect(result.failureHandlerAssigns).toHaveLength(0);
    });

    // -----------------------------------------------------------------------
    // TC-P10-007: Failure handler assignment is not mistakenly captured as a
    //             call site — apiCallSites must not include the FH assignment
    // -----------------------------------------------------------------------

    it("TC-P10-007: JUNO_FAILURE_HANDLER assignment produces a failureHandlerAssign record and no apiCallSite", () => {
        const src =
            "static JUNO_STATUS_T SomeInit(JUNO_APP_ROOT_T *ptApp)\n" +
            "{\n" +
            "    ptApp->tApi.JUNO_FAILURE_HANDLER = AppFailureHandler;\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}";

        const result = parseFile("/test/file.c", src);

        // Failure handler record must be present
        expect(result.failureHandlerAssigns).toHaveLength(1);
        expect(result.failureHandlerAssigns[0].functionName).toBe("AppFailureHandler");

        // An assignment expression is not a call site — apiCallSites must be empty
        expect(result.apiCallSites).toHaveLength(0);
    });
});
