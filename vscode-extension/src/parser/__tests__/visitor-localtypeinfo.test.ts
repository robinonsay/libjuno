/**
 * @file visitor-localtypeinfo.test.ts
 *
 * Tests for the CST visitor's local type info extraction — localVariables
 * and functionParameters population in ParsedFile.localTypeInfo.
 *
 * Test cases: TC-LTI-001 through TC-LTI-005, TC-LTI-NEG-001, TC-LTI-NEG-002, TC-LTI-BND-001
 */

/// <reference types="jest" />

import { parseFile } from "../visitor";

// @{"verify": ["REQ-VSCODE-003", "REQ-VSCODE-009", "REQ-VSCODE-015"]}
describe("Visitor — Local Type Info Extraction", () => {

    // -----------------------------------------------------------------------
    // TC-LTI-001: Simple typed local variable
    // -----------------------------------------------------------------------

    it("TC-LTI-001: extracts simple typed local variable into localVariables", () => {
        const src =
            "static JUNO_STATUS_T MyInit(JUNO_APP_ROOT_T *ptApp)\n" +
            "{\n" +
            "    JUNO_TIME_T *ptTime;\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}";

        const parsed = parseFile("/test/file.c", src);

        expect(parsed.localTypeInfo.localVariables.has("MyInit")).toBe(true);

        const fnMap = parsed.localTypeInfo.localVariables.get("MyInit")!;
        expect(fnMap.has("ptTime")).toBe(true);

        const ti = fnMap.get("ptTime")!;
        expect(ti.name).toBe("ptTime");
        expect(ti.typeName).toBe("JUNO_TIME_T");
        expect(ti.isPointer).toBe(true);
        expect(ti.isConst).toBe(false);
        expect(ti.isArray).toBe(false);
    });

    // -----------------------------------------------------------------------
    // TC-LTI-002: Pointer-to-struct and const-qualified local variables
    // -----------------------------------------------------------------------

    it("TC-LTI-002: extracts pointer and const-qualified local variables", () => {
        const src =
            "static JUNO_STATUS_T Process(void)\n" +
            "{\n" +
            "    JUNO_DS_HEAP_ROOT_T *ptHeap;\n" +
            "    const JUNO_LOG_ROOT_T *ptLog;\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}";

        const parsed = parseFile("/test/file.c", src);

        expect(parsed.localTypeInfo.localVariables.has("Process")).toBe(true);

        const fnMap = parsed.localTypeInfo.localVariables.get("Process")!;
        expect(fnMap.size).toBe(2);

        const ptHeap = fnMap.get("ptHeap")!;
        expect(ptHeap.name).toBe("ptHeap");
        expect(ptHeap.typeName).toBe("JUNO_DS_HEAP_ROOT_T");
        expect(ptHeap.isPointer).toBe(true);
        expect(ptHeap.isConst).toBe(false);

        const ptLog = fnMap.get("ptLog")!;
        expect(ptLog.name).toBe("ptLog");
        expect(ptLog.typeName).toBe("JUNO_LOG_ROOT_T");
        expect(ptLog.isPointer).toBe(true);
        expect(ptLog.isConst).toBe(true);
    });

    // -----------------------------------------------------------------------
    // TC-LTI-003: Single typed function parameter
    // -----------------------------------------------------------------------

    it("TC-LTI-003: extracts single typed function parameter into functionParameters", () => {
        const src =
            "static JUNO_STATUS_T OnStart(JUNO_APP_ROOT_T *ptApp)\n" +
            "{\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}";

        const parsed = parseFile("/test/file.c", src);

        expect(parsed.localTypeInfo.functionParameters.has("OnStart")).toBe(true);

        const params = parsed.localTypeInfo.functionParameters.get("OnStart")!;
        expect(params).toHaveLength(1);
        expect(params[0].name).toBe("ptApp");
        expect(params[0].typeName).toBe("JUNO_APP_ROOT_T");
        expect(params[0].isPointer).toBe(true);
        expect(params[0].isConst).toBe(false);
        expect(params[0].isArray).toBe(false);
    });

    // -----------------------------------------------------------------------
    // TC-LTI-004: Multiple function parameters — order preserved
    // -----------------------------------------------------------------------

    it("TC-LTI-004: extracts multiple function parameters in declaration order", () => {
        const src =
            "static JUNO_STATUS_T HeapCompare(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tParent, JUNO_POINTER_T tChild)\n" +
            "{\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}";

        const parsed = parseFile("/test/file.c", src);

        expect(parsed.localTypeInfo.functionParameters.has("HeapCompare")).toBe(true);

        const params = parsed.localTypeInfo.functionParameters.get("HeapCompare")!;
        expect(params).toHaveLength(3);

        expect(params[0].name).toBe("ptHeap");
        expect(params[0].typeName).toBe("JUNO_DS_HEAP_ROOT_T");
        expect(params[0].isPointer).toBe(true);

        expect(params[1].name).toBe("tParent");
        expect(params[1].typeName).toBe("JUNO_POINTER_T");
        expect(params[1].isPointer).toBe(false);

        expect(params[2].name).toBe("tChild");
        expect(params[2].typeName).toBe("JUNO_POINTER_T");
        expect(params[2].isPointer).toBe(false);
    });

    // -----------------------------------------------------------------------
    // TC-LTI-005: Function scope isolation — variables from different functions
    //             do not collide
    // -----------------------------------------------------------------------

    it("TC-LTI-005: local variables and parameters are scoped per function", () => {
        const src =
            "static JUNO_STATUS_T FuncA(JUNO_APP_ROOT_T *ptApp)\n" +
            "{\n" +
            "    JUNO_TIME_T *ptTime;\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}\n" +
            "\n" +
            "static JUNO_STATUS_T FuncB(JUNO_DS_HEAP_ROOT_T *ptHeap)\n" +
            "{\n" +
            "    JUNO_LOG_ROOT_T *ptLog;\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}";

        const parsed = parseFile("/test/file.c", src);

        // Both functions have entries in localVariables
        expect(parsed.localTypeInfo.localVariables.has("FuncA")).toBe(true);
        expect(parsed.localTypeInfo.localVariables.has("FuncB")).toBe(true);

        const funcAVars = parsed.localTypeInfo.localVariables.get("FuncA")!;
        const funcBVars = parsed.localTypeInfo.localVariables.get("FuncB")!;

        // FuncA has ptTime but NOT ptLog
        expect(funcAVars.has("ptTime")).toBe(true);
        expect(funcAVars.has("ptLog")).toBe(false);
        expect(funcAVars.get("ptTime")!.typeName).toBe("JUNO_TIME_T");

        // FuncB has ptLog but NOT ptTime
        expect(funcBVars.has("ptLog")).toBe(true);
        expect(funcBVars.has("ptTime")).toBe(false);
        expect(funcBVars.get("ptLog")!.typeName).toBe("JUNO_LOG_ROOT_T");

        // functionParameters scoped correctly
        const paramsA = parsed.localTypeInfo.functionParameters.get("FuncA")!;
        expect(paramsA).toHaveLength(1);
        expect(paramsA[0].name).toBe("ptApp");
        expect(paramsA[0].typeName).toBe("JUNO_APP_ROOT_T");

        const paramsB = parsed.localTypeInfo.functionParameters.get("FuncB")!;
        expect(paramsB).toHaveLength(1);
        expect(paramsB[0].name).toBe("ptHeap");
        expect(paramsB[0].typeName).toBe("JUNO_DS_HEAP_ROOT_T");
    });

    // -----------------------------------------------------------------------
    // TC-LTI-NEG-001: Function with no local variables → no entry in localVariables
    // -----------------------------------------------------------------------

    it("TC-LTI-NEG-001: function with no local variables has no entry in localVariables", () => {
        const src =
            "static JUNO_STATUS_T EmptyFunc(JUNO_APP_ROOT_T *ptApp)\n" +
            "{\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}";

        const parsed = parseFile("/test/file.c", src);

        // Visitor does not create an entry for a function that declares no locals
        expect(parsed.localTypeInfo.localVariables.has("EmptyFunc")).toBe(false);
    });

    // -----------------------------------------------------------------------
    // TC-LTI-NEG-002: Function with void parameter list → empty functionParameters array
    // -----------------------------------------------------------------------

    it("TC-LTI-NEG-002: function declared with void parameters has an empty functionParameters array", () => {
        const src =
            "static void NoParams(void)\n" +
            "{\n" +
            "    return;\n" +
            "}";

        const parsed = parseFile("/test/file.c", src);

        // Visitor creates an entry for every function, but the array is empty for void
        expect(parsed.localTypeInfo.functionParameters.has("NoParams")).toBe(true);
        const params = parsed.localTypeInfo.functionParameters.get("NoParams")!;
        expect(params).toHaveLength(0);
    });

    // -----------------------------------------------------------------------
    // TC-LTI-BND-001: Double-pointer variable → base type extracted, isPointer true
    // -----------------------------------------------------------------------

    it("TC-LTI-BND-001: double-pointer variable extracts base typeName and sets isPointer true", () => {
        const src =
            "static JUNO_STATUS_T DeepPtr(void)\n" +
            "{\n" +
            "    JUNO_FOO_T **ppFoo;\n" +
            "    return JUNO_STATUS_SUCCESS;\n" +
            "}";

        const parsed = parseFile("/test/file.c", src);

        expect(parsed.localTypeInfo.localVariables.has("DeepPtr")).toBe(true);

        const fnMap = parsed.localTypeInfo.localVariables.get("DeepPtr")!;
        expect(fnMap.has("ppFoo")).toBe(true);

        const ti = fnMap.get("ppFoo")!;
        expect(ti.name).toBe("ppFoo");
        expect(ti.typeName).toBe("JUNO_FOO_T");
        expect(ti.isPointer).toBe(true);
    });
});
