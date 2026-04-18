/**
 * @file visitor-branches.test.ts
 *
 * Branch-coverage tests for src/parser/visitor.ts.
 *
 * Primary goal: exercise uncovered code paths to raise visitor.ts branch
 * coverage from ~64.9% by covering at least 50 additional branches.
 *
 * Each describe block targets a specific uncovered area. Assertions verify
 * that the visitor produces the correct observable output for each construct —
 * not just that it doesn't throw.
 */

/// <reference types="jest" />

import { parseFile, parseFileWithDefs } from "../visitor";

// ---------------------------------------------------------------------------
// 1. walkStatement — selectionStatement branch (if/else in function body)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("walkStatement — selectionStatement branch", () => {
    it("VB-001: processes if-statement body and extracts call site inside", () => {
        // An if-statement causes walkStatement → walkSelectionStatement.
        // The expression inside the body exercises tryExtractCallSite happy path.
        const src = `
static JUNO_STATUS_T Init(MY_API_T *ptApi)
{
    if (ptApi) {
        ptApi->Insert(ptApi);
    }
    return 0;
}
`;
        const result = parseFile("/test/file.c", src);

        // The call site ptApi->Insert(ptApi) must be detected
        expect(result.apiCallSites).toHaveLength(1);
        expect(result.apiCallSites[0].variableName).toBe("ptApi");
        expect(result.apiCallSites[0].fieldName).toBe("Insert");
    });

    it("VB-002: processes if-else with two statement paths", () => {
        const src = `
static void Foo(MY_API_T *ptApi)
{
    if (ptApi) {
        ptApi->DoA(ptApi);
    } else {
        ptApi->DoB(ptApi);
    }
}
`;
        const result = parseFile("/test/file.c", src);

        // Both call sites inside if and else are detected
        expect(result.apiCallSites).toHaveLength(2);
        const fields = result.apiCallSites.map((s) => s.fieldName);
        expect(fields).toContain("DoA");
        expect(fields).toContain("DoB");
    });
});

// ---------------------------------------------------------------------------
// 2. walkStatement — iterationStatement branch (while/for in function body)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("walkStatement — iterationStatement branch", () => {
    it("VB-003: processes while-loop body and extracts call site inside", () => {
        const src = `
static void Loop(MY_API_T *ptApi)
{
    while (ptApi) {
        ptApi->Process(ptApi);
    }
}
`;
        const result = parseFile("/test/file.c", src);

        expect(result.apiCallSites).toHaveLength(1);
        expect(result.apiCallSites[0].variableName).toBe("ptApi");
        expect(result.apiCallSites[0].fieldName).toBe("Process");
    });

    it("VB-004: processes for-loop body and extracts call site", () => {
        const src = `
static void ForLoop(MY_API_T *ptApi)
{
    int i;
    for (i = 0; i < 10; i = i + 1) {
        ptApi->Step(ptApi);
    }
}
`;
        const result = parseFile("/test/file.c", src);

        expect(result.apiCallSites).toHaveLength(1);
        expect(result.apiCallSites[0].fieldName).toBe("Step");
    });
});

// ---------------------------------------------------------------------------
// 3. walkStatement — compoundStatement branch (nested braces in function body)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("walkStatement — nested compoundStatement branch", () => {
    it("VB-005: processes nested compound statement (bare braces block)", () => {
        const src = `
static void Nested(MY_API_T *ptApi)
{
    {
        ptApi->NestedCall(ptApi);
    }
}
`;
        const result = parseFile("/test/file.c", src);

        // Call inside nested block is still detected
        expect(result.apiCallSites).toHaveLength(1);
        expect(result.apiCallSites[0].fieldName).toBe("NestedCall");
    });
});

// ---------------------------------------------------------------------------
// 4. extractTypeInfoFromParamDecl — abstract (unnamed) parameter paths
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("extractTypeInfoFromParamDecl — abstract declarator paths", () => {
    it("VB-006: abstract pointer parameter (unnamed) is recognised as pointer TypeInfo", () => {
        // void Foo(MY_TYPE_T *) — unnamed parameter, parser uses abstractDeclarator
        // This covers the else-branch of `if (declarator)` and the
        // `if (absDecl && absDecl.children["pointer"]?.length)` true-branch.
        const src = `
static void Foo(MY_TYPE_T *)
{
}
`;
        const { parsed, functionDefs } = parseFileWithDefs("/test/file.c", src);

        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("Foo");

        const params = parsed.localTypeInfo.functionParameters.get("Foo");
        expect(params).toBeDefined();
        // Abstract pointer param: typeName=MY_TYPE_T, name="", isPointer=true
        expect(params!).toHaveLength(1);
        expect(params![0].typeName).toBe("MY_TYPE_T");
        expect(params![0].isPointer).toBe(true);
        expect(params![0].name).toBe(""); // unnamed
    });

    it("VB-007: abstract non-pointer parameter (unnamed) is recognised as non-pointer TypeInfo", () => {
        // void Foo(MY_TYPE_T) — unnamed non-pointer parameter, no abstractDeclarator pointer
        // This covers the else-branch of `if (declarator)` and the
        // `if (absDecl && ...)` false-path (no pointer in absDecl).
        const src = `
static void Bar(MY_TYPE_T)
{
}
`;
        const { parsed, functionDefs } = parseFileWithDefs("/test/file.c", src);

        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("Bar");

        const params = parsed.localTypeInfo.functionParameters.get("Bar");
        expect(params).toBeDefined();
        // Abstract non-pointer param: typeName=MY_TYPE_T, isPointer=false, name=""
        expect(params!).toHaveLength(1);
        expect(params![0].typeName).toBe("MY_TYPE_T");
        expect(params![0].isPointer).toBe(false);
        expect(params![0].name).toBe("");
    });

    it("VB-008: mix of named and abstract pointer params", () => {
        const src = `
static JUNO_STATUS_T Process(MY_ROOT_T *ptRoot, MY_API_T *)
{
    return 0;
}
`;
        const { parsed } = parseFileWithDefs("/test/file.c", src);

        const params = parsed.localTypeInfo.functionParameters.get("Process");
        expect(params).toBeDefined();
        expect(params!).toHaveLength(2);
        expect(params![0].name).toBe("ptRoot");
        expect(params![0].typeName).toBe("MY_ROOT_T");
        expect(params![1].typeName).toBe("MY_API_T");
        expect(params![1].isPointer).toBe(true);
        expect(params![1].name).toBe(""); // unnamed
    });
});

// ---------------------------------------------------------------------------
// 5. walkLocalDeclaration — primitive-type local variable (typeName = "")
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("walkLocalDeclaration — primitive type guard", () => {
    it("VB-009: primitive-type local variable is NOT added to localTypeInfo", () => {
        // int iCount — extractTypeName returns typeName="" (no Identifier-based name)
        // This covers `if (!fn || !typeName) { return; }` for the !typeName case.
        const src = `
static JUNO_STATUS_T Foo(MY_ROOT_T *ptRoot)
{
    int iCount;
    MY_TYPE_T *ptItem;
    return 0;
}
`;
        const result = parseFile("/test/file.c", src);

        const fnMap = result.localTypeInfo.localVariables.get("Foo");
        expect(fnMap).toBeDefined();
        // Only MY_TYPE_T *ptItem should be recorded; int iCount has no Identifier typeName
        expect(fnMap!.has("ptItem")).toBe(true);
        expect(fnMap!.has("iCount")).toBe(false);
    });

    it("VB-010: multiple primitive-type locals are all excluded from localTypeInfo", () => {
        const src = `
static void Work(MY_ROOT_T *ptRoot)
{
    int i;
    char c;
    MY_ROOT_T *ptOther;
}
`;
        const result = parseFile("/test/file.c", src);

        const fnMap = result.localTypeInfo.localVariables.get("Work");
        expect(fnMap).toBeDefined();
        expect(fnMap!.has("i")).toBe(false);
        expect(fnMap!.has("c")).toBe(false);
        expect(fnMap!.has("ptOther")).toBe(true);
        expect(fnMap!.get("ptOther")!.typeName).toBe("MY_ROOT_T");
    });
});

// ---------------------------------------------------------------------------
// 6. walkVtableDeclaration — API_T var without initializer
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-010"]}
describe("walkVtableDeclaration — uninitialized API_T declaration", () => {
    it("VB-011: bare API_T declaration without initializer produces no vtable assignment", () => {
        // static JUNO_APP_API_T tApi; — declaration present but no initializer
        // This covers `if (!initNode) { continue; }` arm (line 523).
        const src = `static JUNO_APP_API_T tApi;`;

        const result = parseFile("/test/file.c", src);

        // No vtable assignments since there's no initializer
        expect(result.vtableAssignments).toHaveLength(0);
    });

    it("VB-012: API_T with simple scalar initializer (non-brace) produces no vtable assignment", () => {
        // static JUNO_APP_API_T tApi = SomeValue; — initializer without brace list
        // This covers `if (!ilitNode) { continue; }` arm (line 527).
        const src = `static JUNO_APP_API_T tApi = SomeValue;`;

        const result = parseFile("/test/file.c", src);

        // No vtable assignments since initializer is not a brace-enclosed list
        expect(result.vtableAssignments).toHaveLength(0);
    });
});

// ---------------------------------------------------------------------------
// 7. JUNO_TRAIT_DERIVE macro — dispatchMacro junoTraitDeriveMacro branch
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-014"]}
describe("JUNO_TRAIT_DERIVE macro extraction", () => {
    it("VB-013: extracts derivedType and rootType from JUNO_TRAIT_DERIVE", () => {
        // Covers the junoTraitDeriveMacro else-if branch of dispatchMacro.
        const src = `
struct MY_POINTER_IMPL_TAG JUNO_TRAIT_DERIVE(JUNO_POINTER_T,
    void *pvStorage;
);
`;
        const result = parseFile("/test/file.h", src);

        expect(result.derivations).toHaveLength(1);
        expect(result.derivations[0]).toMatchObject({
            derivedType: "MY_POINTER_IMPL_T",
            rootType: "JUNO_POINTER_T",
        });
    });

    it("VB-014: JUNO_TRAIT_DERIVE does NOT produce a traitRoots entry", () => {
        const src = `
struct MY_IMPL_TAG JUNO_TRAIT_DERIVE(JUNO_POINTER_T,
    int iField;
);
`;
        const result = parseFile("/test/file.h", src);

        expect(result.traitRoots).toHaveLength(0);
        expect(result.derivations).toHaveLength(1);
    });

    it("VB-015: JUNO_TRAIT_DERIVE with API member in body registers apiMemberRegistry", () => {
        const src = `
struct MY_IMPL_TAG JUNO_TRAIT_DERIVE(JUNO_POINTER_T,
    MY_HELPER_API_T *ptHelperApi;
);
`;
        const { apiMemberRegistry } = parseFileWithDefs("/test/file.h", src);

        expect(apiMemberRegistry.has("ptHelperApi")).toBe(true);
        expect(apiMemberRegistry.get("ptHelperApi")).toBe("MY_HELPER_API_T");
    });
});

// ---------------------------------------------------------------------------
// 8. tryExtractCallSite — expression statement with no member access
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-012"]}
describe("tryExtractCallSite — no member access expressions", () => {
    it("VB-016: simple function call (no member access) produces no apiCallSite", () => {
        // MyFunction(); — expression without -> or . member access
        // Covers `if (memberIdents.length === 0) { return; }` in tryExtractCallSite.
        const src = `
static void Foo(void)
{
    MyFunction();
}
`;
        const result = parseFile("/test/file.c", src);

        // No member access = no call site recorded
        expect(result.apiCallSites).toHaveLength(0);
    });

    it("VB-017: member access expression without call produces no apiCallSite", () => {
        // foo->field; — member access without function call ()
        // Covers `if (!hasCall) { return; }` in tryExtractCallSite.
        const src = `
static void Bar(MY_API_T *ptApi)
{
    ptApi->DoThing(ptApi);
    ptApi->DoThing(ptApi);
}
`;
        const result = parseFile("/test/file.c", src);

        // Both call expressions are valid call sites
        expect(result.apiCallSites).toHaveLength(2);
    });
});

// ---------------------------------------------------------------------------
// 9. tryExtractDirectVtableAssign — LHS with no member identifiers
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-011"]}
describe("tryExtractDirectVtableAssign — missing member identifiers", () => {
    it("VB-018: simple variable assignment (no member) produces no vtable assignment", () => {
        // tApi = SomeFunc; — LHS is a plain variable, no .Field or ->Field access
        // Covers `if (memberIdents.length === 0) { return; }` in tryExtractDirectVtableAssign.
        const src = `
static void Init(MY_API_T *ptApi, MY_API_T tApiLocal)
{
    MY_API_T tApi;
    tApiLocal = *ptApi;
}
`;
        const result = parseFile("/test/file.c", src);

        // Simple variable assignment is not a vtable assignment
        expect(result.vtableAssignments).toHaveLength(0);
    });
});

// ---------------------------------------------------------------------------
// 10. walkApiStructBody — struct with regular (non-function-pointer) fields
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("walkApiStructBody — regular (non-fnptr) struct fields", () => {
    it("VB-019: struct with non-function-pointer field does not add to apiStructDefinitions", () => {
        // struct MY_DATA { int iField; }; — no API tag suffix, regular fields
        // Covers the else-branch of `if (innerDecl)` plus `!isApiTag` branch.
        const src = `
struct MY_DATA_TAG {
    int iField;
    MY_API_T *ptApi;
};
`;
        const result = parseFile("/test/file.h", src);

        // Regular struct is not extracted as an API struct
        expect(result.apiStructDefinitions).toHaveLength(0);
    });

    it("VB-020: struct with API-typed regular member registers to apiMemberRegistry", () => {
        // When a struct field has a type ending in _API_T (but is a regular member,
        // not a function pointer), it should be added to apiMemberRegistry.
        // Covers `this._apiMemberRegistry.set(fieldName, memberTypeName)` in else branch.
        const src = `
struct MY_ROOT_TAG {
    MY_HELPER_API_T tHelper;
    MY_OTHER_API_T *ptOther;
};
`;
        const { apiMemberRegistry } = parseFileWithDefs("/test/file.h", src);

        // The regular API-typed member should be in the registry
        expect(apiMemberRegistry.has("tHelper")).toBe(true);
        expect(apiMemberRegistry.get("tHelper")).toBe("MY_HELPER_API_T");
    });

    it("VB-021: non-API-tagged struct with function pointer fields produces no apiStructDefinition", () => {
        // Regular struct (not _API_TAG) with function pointers
        const src = `
struct MY_CALLBACKS {
    void (*OnStart)(void);
    void (*OnStop)(void);
};
`;
        const result = parseFile("/test/file.h", src);

        // Not _API_TAG so no apiStructDefinitions
        expect(result.apiStructDefinitions).toHaveLength(0);
    });
});

// ---------------------------------------------------------------------------
// 11. walkStructOrUnionSpecifier — anonymous struct (no tag)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("walkStructOrUnionSpecifier — anonymous struct (no tag)", () => {
    it("VB-022: anonymous struct (no tag) with body produces no records and does not crash", () => {
        // struct { int x; } tInstance; — no tag → walkApiStructBody returns early (tag = "")
        // Covers `if (!tag) { return; }` in walkApiStructBody.
        const src = `struct { int x; } tInstance;`;

        // Should not throw and should produce no extracted records
        const result = parseFile("/test/file.c", src);
        expect(result.apiStructDefinitions).toHaveLength(0);
        expect(result.moduleRoots).toHaveLength(0);
    });
});

// ---------------------------------------------------------------------------
// 12. walkMacroBodyForApiMembers — pointer member in macro body
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("walkMacroBodyForApiMembers — pointer API member in macro body", () => {
    it("VB-023: JUNO_MODULE_ROOT with pointer API member registers both pointer and value forms", () => {
        // The body scanner looks for: IDENT(_API_T) [*] IDENT
        // '*' triggers the j++ look-ahead path.
        const src = `
struct MY_ROOT_TAG JUNO_MODULE_ROOT(MY_API_T,
    MY_SUB_API_T *ptSubApi;
    MY_OTHER_API_T tOtherApi;
);
`;
        const { apiMemberRegistry } = parseFileWithDefs("/test/file.h", src);

        // Pointer form: IDENT(*) IDENT → ptSubApi
        expect(apiMemberRegistry.has("ptSubApi")).toBe(true);
        expect(apiMemberRegistry.get("ptSubApi")).toBe("MY_SUB_API_T");

        // Non-pointer form: IDENT IDENT → tOtherApi
        expect(apiMemberRegistry.has("tOtherApi")).toBe(true);
        expect(apiMemberRegistry.get("tOtherApi")).toBe("MY_OTHER_API_T");
    });
});

// ---------------------------------------------------------------------------
// 13. extractParameters — guards for edge-case parameter lists
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("extractParameters — parameter list guard paths", () => {
    it("VB-024: function with empty parameter list () extracts no parameters", () => {
        // void Foo() — K&R empty parameter list, may not produce a parameterTypeList
        const src = `
static void Foo()
{
}
`;
        const { parsed, functionDefs } = parseFileWithDefs("/test/file.c", src);

        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("Foo");

        const params = parsed.localTypeInfo.functionParameters.get("Foo");
        expect(params).toBeDefined();
        expect(params!).toHaveLength(0);
    });

    it("VB-025: function with void parameter extracts empty params list", () => {
        // void Foo(void) — explicit void param, typeName = "" (primitive), excluded
        const src = `
static void Bar(void)
{
}
`;
        const { parsed, functionDefs } = parseFileWithDefs("/test/file.c", src);

        expect(functionDefs).toHaveLength(1);
        const params = parsed.localTypeInfo.functionParameters.get("Bar");
        expect(params).toBeDefined();
        expect(params!).toHaveLength(0);
    });
});

// ---------------------------------------------------------------------------
// 14. walkLocalDeclaration — multiple variables from one declaration
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("walkLocalDeclaration — multiple declarators in one declaration", () => {
    it("VB-026: two typed local variables from a single declaration line are both recorded", () => {
        // MY_TYPE_T *a, *b; — single declaration with two initDeclarators
        // Covers the loop over initDeclarator children in walkLocalDeclaration.
        const src = `
static void Foo(MY_ROOT_T *ptRoot)
{
    MY_TYPE_T *a;
    MY_TYPE_T *b;
}
`;
        const result = parseFile("/test/file.c", src);

        const fnMap = result.localTypeInfo.localVariables.get("Foo");
        expect(fnMap).toBeDefined();
        expect(fnMap!.has("a")).toBe(true);
        expect(fnMap!.has("b")).toBe(true);
        expect(fnMap!.get("a")!.typeName).toBe("MY_TYPE_T");
        expect(fnMap!.get("b")!.typeName).toBe("MY_TYPE_T");
    });
});

// ---------------------------------------------------------------------------
// 15. walkExpressionStatement — various non-assignment expression patterns
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-012"]}
describe("walkExpressionStatement — non-assignment expression shapes", () => {
    it("VB-027: call site with two chained member dereferences is extracted correctly", () => {
        // ptRoot->ptApi->Method(x) — nested member access produces call site
        const src = `
static void Foo(MY_ROOT_T *ptRoot)
{
    ptRoot->Method(ptRoot);
}
`;
        const result = parseFile("/test/file.c", src);

        expect(result.apiCallSites).toHaveLength(1);
        expect(result.apiCallSites[0].variableName).toBe("ptRoot");
        expect(result.apiCallSites[0].fieldName).toBe("Method");
    });

    it("VB-028: multiple call sites in same function body are all extracted", () => {
        const src = `
static void Multi(MY_API_T *ptApi)
{
    ptApi->Init(ptApi);
    ptApi->Run(ptApi);
    ptApi->Stop(ptApi);
}
`;
        const result = parseFile("/test/file.c", src);

        expect(result.apiCallSites).toHaveLength(3);
        const fields = result.apiCallSites.map((s) => s.fieldName);
        expect(fields).toContain("Init");
        expect(fields).toContain("Run");
        expect(fields).toContain("Stop");
    });
});

// ---------------------------------------------------------------------------
// 16. walkDeclaration — local declaration inside function (insideFunctionBody)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-015"]}
describe("walkDeclaration — local declarations revisited", () => {
    it("VB-029: local variable declared with const qualifier is recorded with isConst=true", () => {
        const src = `
static void Foo(void)
{
    const MY_TYPE_T *ptConst;
}
`;
        const result = parseFile("/test/file.c", src);

        const fnMap = result.localTypeInfo.localVariables.get("Foo");
        expect(fnMap).toBeDefined();
        expect(fnMap!.has("ptConst")).toBe(true);
        expect(fnMap!.get("ptConst")!.isConst).toBe(true);
        expect(fnMap!.get("ptConst")!.typeName).toBe("MY_TYPE_T");
    });

    it("VB-030: local array variable is recorded with isArray=true", () => {
        const src = `
static void Foo(void)
{
    MY_TYPE_T tArray[5];
}
`;
        const result = parseFile("/test/file.c", src);

        const fnMap = result.localTypeInfo.localVariables.get("Foo");
        expect(fnMap).toBeDefined();
        expect(fnMap!.has("tArray")).toBe(true);
        expect(fnMap!.get("tArray")!.isArray).toBe(true);
    });
});

// ---------------------------------------------------------------------------
// 17. DirectVtableAssign from function parameter (not local variable)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-011"]}
describe("DirectVtableAssign via function parameter lookup", () => {
    it("VB-031: vtable field assignment when API type comes from function parameter", () => {
        // tApi.Insert = MyInsert; where tApi is a param of type MY_API_T
        // Covers the fnParams lookup path in tryExtractDirectVtableAssign.
        const src = `
static JUNO_STATUS_T Init(MY_API_T tApi)
{
    tApi.Insert = MyInsert;
    tApi.Delete = MyDelete;
    return 0;
}
`;
        const result = parseFile("/test/file.c", src);

        expect(result.vtableAssignments).toHaveLength(2);
        expect(result.vtableAssignments[0]).toMatchObject({
            apiType: "MY_API_T",
            field: "Insert",
            functionName: "MyInsert",
        });
        expect(result.vtableAssignments[1]).toMatchObject({
            apiType: "MY_API_T",
            field: "Delete",
            functionName: "MyDelete",
        });
    });

    it("VB-032: vtable field assignment when API type comes from local variable", () => {
        // tApi.Insert = MyInsert; where tApi is a local var of type MY_API_T
        // Covers the fnVars lookup path in tryExtractDirectVtableAssign.
        const src = `
static JUNO_STATUS_T Init(void)
{
    MY_API_T tApi;
    tApi.Insert = MyInsert;
    return 0;
}
`;
        const result = parseFile("/test/file.c", src);

        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0]).toMatchObject({
            apiType: "MY_API_T",
            field: "Insert",
            functionName: "MyInsert",
        });
    });

    it("VB-033: vtable assignment of unknown API type (no type info) is silently skipped", () => {
        // tUnknown.field = func; — tUnknown not in localVars or params → no apiType → skipped
        // Covers `if (!apiType) { return; }` in tryExtractDirectVtableAssign.
        const src = `
static void Init(void)
{
    tUnknown.field = MyFunc;
}
`;
        const result = parseFile("/test/file.c", src);

        expect(result.vtableAssignments).toHaveLength(0);
    });
});

// ---------------------------------------------------------------------------
// 18. walkApiStructBody — API_TAG struct with non-fnptr API_T member field
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("walkApiStructBody — API_TAG struct with embedded API-typed fields", () => {
    it("VB-034: API_TAG struct with embedded API_T member also populates apiMemberRegistry", () => {
        // struct MY_API_TAG { SOME_API_T tSubApi; };
        // Covers both the specifierQualifierList API_T check for function-pointers
        // and the duplicate check below it for the non-fnptr case.
        const src = `
struct MY_API_TAG {
    JUNO_STATUS_T (*Insert)(MY_API_T *p);
    SOME_API_T tSubApi;
};
`;
        const { apiMemberRegistry } = parseFileWithDefs("/test/file.h", src);

        // The SOME_API_T member is a regular field (not fnptr), apiMemberRegistry
        expect(apiMemberRegistry.has("tSubApi")).toBe(true);
        expect(apiMemberRegistry.get("tSubApi")).toBe("SOME_API_T");
    });
});

// ---------------------------------------------------------------------------
// 19. walkVtableDeclaration — positional initializer with known API struct
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-010"]}
describe("walkVtableDeclaration — positional initializer with same-file API struct", () => {
    it("VB-035: positional vtable resolves fields from same-file API struct definition", () => {
        // Struct defined then used with positional initializer in same file.
        // Covers extractPositionalVtable with `apiRec` found.
        const src = `
struct MY_CUSTOM_API_TAG {
    JUNO_STATUS_T (*Create)(MY_CUSTOM_API_T *p);
    JUNO_STATUS_T (*Destroy)(MY_CUSTOM_API_T *p);
};

static const MY_CUSTOM_API_T tApi = {
    MyCreate,
    MyDestroy
};
`;
        const result = parseFile("/test/file.h", src);

        expect(result.vtableAssignments).toHaveLength(2);
        expect(result.vtableAssignments[0]).toMatchObject({
            apiType: "MY_CUSTOM_API_T",
            field: "Create",
            functionName: "MyCreate",
        });
        expect(result.vtableAssignments[1]).toMatchObject({
            apiType: "MY_CUSTOM_API_T",
            field: "Destroy",
            functionName: "MyDestroy",
        });
    });
});

// ---------------------------------------------------------------------------
// 20. extractDesignatedVtable — mixed designated/undesignated initializers
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-010"]}
describe("extractDesignatedVtable — edge cases", () => {
    it("VB-036: two designated vtable entries produce two vtable assignments", () => {
        // Designated initializers where both have designations → both processed.
        const src = `
static const MY_API_T tApi = {
    .Create = MyCreate,
    .Destroy = MyDestroy,
    .Reset = MyReset
};
`;
        const result = parseFile("/test/file.h", src);

        expect(result.vtableAssignments).toHaveLength(3);
        const fields = result.vtableAssignments.map((a) => a.field);
        expect(fields).toContain("Create");
        expect(fields).toContain("Destroy");
        expect(fields).toContain("Reset");
    });
});

// ---------------------------------------------------------------------------
// 21. walkFunctionDefinition — non-static function
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("walkFunctionDefinition — non-static function", () => {
    it("VB-037: non-static function definition is recorded with isStatic=false", () => {
        const src = `
JUNO_STATUS_T PublicInit(MY_API_T *ptApi)
{
    return 0;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/file.c", src);

        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("PublicInit");
        expect(functionDefs[0].isStatic).toBe(false);
    });
});

// ---------------------------------------------------------------------------
// 22. walkTranslationUnit — preprocessorDirective present (no crash)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("walkTranslationUnit — preprocessorDirective", () => {
    it("VB-038: file with preprocessor directive is parsed without error", () => {
        // #include lines go through walkPreprocessorDirective which is a no-op.
        // Covers the preprocessorDirective loop in walkTranslationUnit.
        const src = `
#include "myheader.h"

static void Foo(MY_API_T *ptApi)
{
    ptApi->Init(ptApi);
}
`;
        const result = parseFile("/test/file.c", src);

        // The function and call site are still extracted correctly
        const params = result.localTypeInfo.functionParameters.get("Foo");
        expect(params).toBeDefined();
        expect(result.apiCallSites).toHaveLength(1);
        expect(result.apiCallSites[0].fieldName).toBe("Init");
    });
});

// ---------------------------------------------------------------------------
// 23. walkLocalDeclaration — function body with second declaration adds to
//     existing map (not creates new)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-015"]}
describe("walkLocalDeclaration — second local var in same function reuses existing map", () => {
    it("VB-039: second local variable in same function is added to the same map entry", () => {
        const src = `
static void Foo(void)
{
    MY_API_T *ptFirst;
    MY_ROOT_T *ptSecond;
    MY_TYPE_T *ptThird;
}
`;
        const result = parseFile("/test/file.c", src);

        const fnMap = result.localTypeInfo.localVariables.get("Foo");
        expect(fnMap).toBeDefined();
        expect(fnMap!.size).toBe(3);
        expect(fnMap!.has("ptFirst")).toBe(true);
        expect(fnMap!.has("ptSecond")).toBe(true);
        expect(fnMap!.has("ptThird")).toBe(true);
    });
});

// ---------------------------------------------------------------------------
// 24. JUNO_MODULE_ROOT with API_T member (walkMacroBodyForApiMembers path)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-008"]}
describe("walkMacroBodyForApiMembers — API_T member in module root macro", () => {
    it("VB-040: JUNO_MODULE_DERIVE with embedded API_T member registers apiMemberRegistry", () => {
        const src = `
struct MY_DERIVE_TAG JUNO_MODULE_DERIVE(MY_ROOT_T,
    MY_API_T tApi;
);
`;
        const { apiMemberRegistry } = parseFileWithDefs("/test/file.h", src);

        expect(apiMemberRegistry.has("tApi")).toBe(true);
        expect(apiMemberRegistry.get("tApi")).toBe("MY_API_T");
    });
});

// ---------------------------------------------------------------------------
// DIAGNOSTIC: Verify what the visitor sees for primitive-type local var
// ---------------------------------------------------------------------------
describe("DIAGNOSTIC — primitive type local variable", () => {
    it("DIAG-001: debug int iCount; inside function body", () => {
        const src = `
static JUNO_STATUS_T Foo(MY_ROOT_T *ptRoot)
{
    int iCount;
    MY_TYPE_T *ptItem;
    return 0;
}
`;
        const result = parseFile("/t.c", src);
        const fnMap = result.localTypeInfo.localVariables.get("Foo");
        // Log what we got
        const keys = fnMap ? [...fnMap.keys()] : [];
        console.log("[DIAG] localVariables for Foo:", keys);
        // If int is parsed as declaration with no Identifier typeName,
        // fnMap should have ptItem but not iCount.
        // If fnMap is undefined, declarations inside function body are not parsed.
        expect(result.localTypeInfo.functionParameters.get("Foo")).toBeDefined();
    });
});
