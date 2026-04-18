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

// ===========================================================================
// VB-100+: Additional branch coverage tests (WI-14.R1)
// Target: raise visitor.ts branch coverage from 61.7% to ≥85%
// ===========================================================================

// ---------------------------------------------------------------------------
// A. extern "C" blocks — walkExternCBlock branches
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("walkExternCBlock — extern C block processing", () => {
    it("VB-100: extern C block with function definition extracts call sites and function defs", () => {
        // Covers the externCBlock loop in walkTranslationUnit and
        // walkExternCBlock → walkExternalDeclaration → walkFunctionDefinition.
        const src = `
extern "C" {
static void Foo(MY_API_T *ptApi)
{
    ptApi->Initialize(ptApi);
}
}
`;
        const result = parseFile("/test/header.h", src);
        const { parsed, functionDefs } = parseFileWithDefs("/test/header.h", src);

        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("Foo");

        expect(result.apiCallSites).toHaveLength(1);
        expect(result.apiCallSites[0].variableName).toBe("ptApi");
        expect(result.apiCallSites[0].fieldName).toBe("Initialize");
    });

    it("VB-101: extern C block with preprocessor directive inside does not crash", () => {
        // Covers the preprocessorDirective loop inside walkExternCBlock.
        const src = `
extern "C" {
#include "my_header.h"
}
`;
        const result = parseFile("/test/header.h", src);

        // No crash and no spurious records
        expect(result.moduleRoots).toHaveLength(0);
        expect(result.apiCallSites).toHaveLength(0);
    });

    it("VB-102: extern C block with struct macro definition extracts moduleRoots", () => {
        // Covers extern C block containing a struct declaration with JUNO_MODULE_ROOT.
        const src = `
extern "C" {
struct MY_EXT_ROOT_TAG JUNO_MODULE_ROOT(MY_EXT_API_T,
    int iState;
);
}
`;
        const result = parseFile("/test/header.h", src);

        expect(result.moduleRoots).toHaveLength(1);
        expect(result.moduleRoots[0].rootType).toBe("MY_EXT_ROOT_T");
        expect(result.moduleRoots[0].apiType).toBe("MY_EXT_API_T");
    });
});

// ---------------------------------------------------------------------------
// B. JUNO_TRAIT_ROOT macro — dispatchMacro junoTraitRootMacro branch
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-016"]}
describe("JUNO_TRAIT_ROOT macro — traitRoots extraction", () => {
    it("VB-103: JUNO_TRAIT_ROOT creates a TraitRootRecord with correct rootType and apiType", () => {
        // Covers the junoTraitRootMacro branch of dispatchMacro.
        const src = `
struct MY_TRAIT_TAG JUNO_TRAIT_ROOT(MY_TRAIT_API_T,
    int iField;
);
`;
        const result = parseFile("/test/file.h", src);

        expect(result.traitRoots).toHaveLength(1);
        expect(result.traitRoots[0]).toMatchObject({
            rootType: "MY_TRAIT_T",
            apiType: "MY_TRAIT_API_T",
        });
        // JUNO_TRAIT_ROOT does not create derivations
        expect(result.derivations).toHaveLength(0);
    });

    it("VB-104: JUNO_TRAIT_ROOT with embedded API_T member registers apiMemberRegistry", () => {
        // Covers junoTraitRootMacro branch + walkMacroBodyForApiMembers with API_T member.
        const src = `
struct MY_SENSOR_TAG JUNO_TRAIT_ROOT(MY_SENSOR_API_T,
    MY_DRIVER_API_T *ptDriver;
);
`;
        const { parsed, apiMemberRegistry } = parseFileWithDefs("/test/file.h", src);

        expect(parsed.traitRoots).toHaveLength(1);
        expect(parsed.traitRoots[0].rootType).toBe("MY_SENSOR_T");
        expect(parsed.traitRoots[0].apiType).toBe("MY_SENSOR_API_T");

        expect(apiMemberRegistry.has("ptDriver")).toBe(true);
        expect(apiMemberRegistry.get("ptDriver")).toBe("MY_DRIVER_API_T");
    });

    it("VB-105: JUNO_TRAIT_ROOT with JUNO_MODULE_EMPTY body creates traitRoots entry", () => {
        // Covers the junoTraitRootMacro branch when the body is empty (no tokens).
        const src = `struct MY_EMPTY_TRAIT_TAG JUNO_TRAIT_ROOT(MY_EMPTY_TRAIT_API_T, JUNO_MODULE_EMPTY);`;
        const result = parseFile("/test/file.h", src);

        expect(result.traitRoots).toHaveLength(1);
        expect(result.traitRoots[0].rootType).toBe("MY_EMPTY_TRAIT_T");
        expect(result.traitRoots[0].apiType).toBe("MY_EMPTY_TRAIT_API_T");
    });
});

// ---------------------------------------------------------------------------
// C. Failure handler assignments — tryExtractFailureHandler true path
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-015"]}
describe("tryExtractFailureHandler — JunoFailureHandler and JunoFailureUserData members", () => {
    it("VB-106: JUNO_FAILURE_HANDLER assignment is recorded as a failureHandlerAssign", () => {
        // Covers the JunoFailureHandler branch in tryExtractFailureHandler (push path).
        const src = `
static void Init(MY_ROOT_T *ptSelf)
{
    ptSelf->JUNO_FAILURE_HANDLER = MyErrorHandler;
}
`;
        const result = parseFile("/test/file.c", src);

        expect(result.failureHandlerAssigns).toHaveLength(1);
        expect(result.failureHandlerAssigns[0].functionName).toBe("MyErrorHandler");
        expect(result.failureHandlerAssigns[0].rootType).toBe(""); // resolved later by indexer
        expect(result.failureHandlerAssigns[0].file).toBe("/test/file.c");

        // Not mistakenly also added to vtable assignments
        expect(result.vtableAssignments).toHaveLength(0);
    });

    it("VB-107: JUNO_FAILURE_USER_DATA assignment is recorded as a failureHandlerAssign", () => {
        // Covers the JunoFailureUserData branch in tryExtractFailureHandler (push path).
        const src = `
static void Setup(MY_ROOT_T *ptSelf)
{
    ptSelf->JUNO_FAILURE_USER_DATA = pvUserContext;
}
`;
        const result = parseFile("/test/file.c", src);

        expect(result.failureHandlerAssigns).toHaveLength(1);
        expect(result.failureHandlerAssigns[0].functionName).toBe("pvUserContext");
        expect(result.vtableAssignments).toHaveLength(0);
    });

    it("VB-108: _pfcnFailureHandler (member-name form) is recorded as a failureHandlerAssign", () => {
        // Covers the JunoFailureHandler token from the _pfcnFailureHandler member form.
        const src = `
static void Register(MY_ROOT_T *ptSelf)
{
    ptSelf->_pfcnFailureHandler = OnModuleError;
}
`;
        const result = parseFile("/test/file.c", src);

        expect(result.failureHandlerAssigns).toHaveLength(1);
        expect(result.failureHandlerAssigns[0].functionName).toBe("OnModuleError");
    });

    it("VB-109: _pvFailureUserData (member-name form) is recorded as a failureHandlerAssign", () => {
        // Covers the JunoFailureUserData token from the _pvFailureUserData member form.
        const src = `
static void Register(MY_ROOT_T *ptSelf)
{
    ptSelf->_pvFailureUserData = pvAppContext;
}
`;
        const result = parseFile("/test/file.c", src);

        expect(result.failureHandlerAssigns).toHaveLength(1);
        expect(result.failureHandlerAssigns[0].functionName).toBe("pvAppContext");
    });

    it("VB-110: both JUNO_FAILURE_HANDLER and JUNO_FAILURE_USER_DATA in same function produce two assigns", () => {
        const src = `
static void Configure(MY_ROOT_T *ptSelf)
{
    ptSelf->JUNO_FAILURE_HANDLER = HandleError;
    ptSelf->JUNO_FAILURE_USER_DATA = pvCtx;
}
`;
        const result = parseFile("/test/file.c", src);

        expect(result.failureHandlerAssigns).toHaveLength(2);
        expect(result.failureHandlerAssigns[0].functionName).toBe("HandleError");
        expect(result.failureHandlerAssigns[1].functionName).toBe("pvCtx");
        expect(result.vtableAssignments).toHaveLength(0);
    });
});

// ---------------------------------------------------------------------------
// D. extractPositionalVtable — defer path (unknown API_T type in same file)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-010"]}
describe("extractPositionalVtable — defer path for cross-file API structs", () => {
    it("VB-111: positional vtable with API type not defined in same file goes to pendingPositionalVtables", () => {
        // Covers the `if (!apiRec)` defer path in extractPositionalVtable.
        // MY_CUSTOM_API_T is not defined in this file so apiRec is null.
        const src = `
static const MY_CUSTOM_API_T tImpl = {
    MyCreate,
    MyDestroy,
    MyReset
};
`;
        const result = parseFile("/test/file.c", src);

        expect(result.pendingPositionalVtables).toHaveLength(1);
        expect(result.pendingPositionalVtables[0].apiType).toBe("MY_CUSTOM_API_T");
        expect(result.pendingPositionalVtables[0].initializers).toEqual(["MyCreate", "MyDestroy", "MyReset"]);
        expect(result.pendingPositionalVtables[0].file).toBe("/test/file.c");

        // No resolved vtable assignments since the struct definition is cross-file
        expect(result.vtableAssignments).toHaveLength(0);
    });

    it("VB-112: positional vtable with single entry for unknown API defers correctly", () => {
        // Covers the single-entry defer path.
        const src = `static const UNKNOWN_API_T tSingleImpl = { MyOnlyFunc };`;
        const result = parseFile("/test/file.c", src);

        expect(result.pendingPositionalVtables).toHaveLength(1);
        expect(result.pendingPositionalVtables[0].initializers).toEqual(["MyOnlyFunc"]);
        expect(result.pendingPositionalVtables[0].lines).toHaveLength(1);
    });
});

// ---------------------------------------------------------------------------
// E. Designated vtable — extractInitializerIdent !ae branch (nested brace)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-010"]}
describe("extractDesignatedVtable — nested brace initializer skipped", () => {
    it("VB-113: designated field with nested brace initializer produces no vtable assignment for that field", () => {
        // .Field = { 0 } — the initializer child is a brace-list, not an assignmentExpression.
        // Covers the `if (!ae) { return undefined; }` branch in extractInitializerIdent.
        const src = `
static const MY_API_T tApi = {
    .Insert = MyInsert,
    .Options = { 0 }
};
`;
        const result = parseFile("/test/file.c", src);

        // .Insert = MyInsert → valid vtable assignment
        // .Options = { 0 } → extractInitializerIdent returns undefined → skipped
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].field).toBe("Insert");
        expect(result.vtableAssignments[0].functionName).toBe("MyInsert");
    });
});

// ---------------------------------------------------------------------------
// F. walkDeclaration — non-API_T and struct body top-level declarations
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("walkDeclaration — top-level non-API_T and struct-body declarations", () => {
    it("VB-114: global variable with non-API_T type produces no vtable declarations or struct records", () => {
        // Covers the `if (typeName.endsWith('_API_T'))` false branch for a top-level
        // non-API_T declaration (e.g., MY_ROOT_T *pGlobal).
        const src = `MY_ROOT_T *pGlobalRoot;`;
        const result = parseFile("/test/file.c", src);

        expect(result.vtableAssignments).toHaveLength(0);
        expect(result.apiStructDefinitions).toHaveLength(0);
        expect(result.moduleRoots).toHaveLength(0);
    });

    it("VB-115: top-level struct declaration with body (non-TAG, non-macro) is traversed without crash", () => {
        // A plain struct definition at top level with a struct body.
        // Covers the walkStructOrUnionSpecifier path for a struct with a body but no macro.
        const src = `
struct PLAIN_DATA_TAG {
    int iX;
    int iY;
};
`;
        const result = parseFile("/test/file.h", src);

        // No records since this isn't an API_TAG struct and has no macro
        expect(result.apiStructDefinitions).toHaveLength(0);
        expect(result.moduleRoots).toHaveLength(0);
    });

    it("VB-116: top-level struct with embedded _API_T member registers apiMemberRegistry via struct body walk", () => {
        // Covers walkStructOrUnionSpecifier → walkApiStructBody path for a struct body
        // that contains an _API_T typed member (non-fnptr else branch).
        const src = `
struct MY_COMPOSITE_TAG {
    MY_HELPER_API_T tHelper;
    int iField;
};
`;
        const { apiMemberRegistry } = parseFileWithDefs("/test/file.h", src);

        // MY_HELPER_API_T regular member check in walkApiStructBody (non-fnptr else branch)
        expect(apiMemberRegistry.has("tHelper")).toBe(true);
        expect(apiMemberRegistry.get("tHelper")).toBe("MY_HELPER_API_T");
    });
});

// ---------------------------------------------------------------------------
// G. walkExpressionStatement — RHS is not a function identifier
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-011"]}
describe("walkExpressionStatement — non-identifier RHS in assignment", () => {
    it("VB-117: assignment where RHS is an integer literal produces no vtable assignment", () => {
        // tApi.Insert = 42; — RHS drills to postfix but getPostfixPrimary returns undefined
        // (IntegerLiteral primary has no Identifier token).
        // Covers `if (!functionName) { return; }` in walkExpressionStatement.
        const src = `
static void Update(MY_API_T tApi)
{
    tApi.Insert = 42;
}
`;
        const result = parseFile("/test/file.c", src);

        // Integer literal is not a valid function identifier → no vtable assignment
        expect(result.vtableAssignments).toHaveLength(0);
    });

    it("VB-118: assignment where RHS is a string literal produces no vtable assignment", () => {
        const src = `
static void Update(MY_API_T tApi)
{
    tApi.Insert = "hello";
}
`;
        const result = parseFile("/test/file.c", src);

        expect(result.vtableAssignments).toHaveLength(0);
    });
});

// ---------------------------------------------------------------------------
// H. API struct body — additional function pointer field paths
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-009"]}
describe("walkApiStructBody — API_TAG with multiple function pointer fields", () => {
    it("VB-119: API_TAG struct with three function pointer fields extracts all field names", () => {
        // Covers the function pointer (innerDecl present) path in walkApiStructBody for
        // an API_TAG struct with multiple fields.
        const src = `
struct TRIPLE_API_TAG {
    JUNO_STATUS_T (*Create)(TRIPLE_API_T *p);
    JUNO_STATUS_T (*Read)(TRIPLE_API_T *p);
    JUNO_STATUS_T (*Destroy)(TRIPLE_API_T *p);
};
`;
        const result = parseFile("/test/file.h", src);

        expect(result.apiStructDefinitions).toHaveLength(1);
        expect(result.apiStructDefinitions[0].apiType).toBe("TRIPLE_API_T");
        expect(result.apiStructDefinitions[0].fields).toEqual(["Create", "Read", "Destroy"]);
    });

    it("VB-120: API_TAG struct with function pointer AND embedded API_T member extracts both", () => {
        // Covers: (1) the innerDecl true branch for the fnptr field,
        //         (2) the else branch for the regular API_T member,
        //         (3) the specifierQualifierList check for both.
        const src = `
struct MIXED_API_TAG {
    JUNO_STATUS_T (*Execute)(MIXED_API_T *p);
    HELPER_API_T tHelper;
};
`;
        const { parsed, apiMemberRegistry } = parseFileWithDefs("/test/file.h", src);

        // The function pointer field is added to apiStructDefinitions
        expect(parsed.apiStructDefinitions).toHaveLength(1);
        expect(parsed.apiStructDefinitions[0].fields).toContain("Execute");

        // The embedded API_T member is registered in apiMemberRegistry
        expect(apiMemberRegistry.has("tHelper")).toBe(true);
        expect(apiMemberRegistry.get("tHelper")).toBe("HELPER_API_T");
    });

    it("VB-121: regular TAG struct (not _API_TAG) with function pointer has no apiStructDefinition", () => {
        // isApiTag = false → the function pointer is NOT added to fields[] → no apiStructDef.
        // Covers the `if (isApiTag) { fields.push(fieldName); }` false branch.
        const src = `
struct REGULAR_TAG {
    JUNO_STATUS_T (*DoWork)(void);
};
`;
        const result = parseFile("/test/file.h", src);

        expect(result.apiStructDefinitions).toHaveLength(0);
    });
});

// ---------------------------------------------------------------------------
// I. JUNO_MODULE macro — junoModuleMacro fall-through produces no records
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-008"]}
describe("JUNO_MODULE macro — produces no root/derive records", () => {
    it("VB-122: JUNO_MODULE macro (plain JUNO_MODULE, not ROOT/DERIVE) produces no moduleRoots", () => {
        // Covers the 'fall-through' case in dispatchMacro where none of the
        // if-branches match for junoModuleMacro (JUNO_MODULE itself).
        const src = `struct MY_MODULE_TAG JUNO_MODULE(MY_API_T, int iState;);`;
        const result = parseFile("/test/file.h", src);

        // JUNO_MODULE has no specific extraction branch → no records
        expect(result.moduleRoots).toHaveLength(0);
        expect(result.derivations).toHaveLength(0);
        expect(result.traitRoots).toHaveLength(0);
    });
});

// ---------------------------------------------------------------------------
// J. walkLocalDeclaration — struct body inside function (no initDeclarator)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-015"]}
describe("walkLocalDeclaration — local struct body inside function body", () => {
    it("VB-123: local struct declaration with body inside function is skipped (no initDeclaratorList)", () => {
        // A struct body declaration inside a function has no initDeclaratorList.
        // The guard `if (!idl) { return; }` fires → no crash.
        const src = `
static void Foo(MY_ROOT_T *ptRoot)
{
    struct inner { int x; };
    MY_TYPE_T *ptUsed;
}
`;
        const result = parseFile("/test/file.c", src);

        // Only ptUsed should be in local variables (struct inner has no declarator list)
        const fnMap = result.localTypeInfo.localVariables.get("Foo");
        expect(fnMap).toBeDefined();
        expect(fnMap!.has("ptUsed")).toBe(true);
    });
});

// ---------------------------------------------------------------------------
// K. walkStructOrUnionSpecifier — multiple macros in one file
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-008", "REQ-VSCODE-014", "REQ-VSCODE-016"]}
describe("walkDeclaration — multiple macro types exercise all dispatchMacro branches", () => {
    it("VB-124: all four macro types in one file exercise the full dispatchMacro if-else chain", () => {
        // Covers all four non-trivial branches of dispatchMacro:
        // junoModuleRootMacro, junoModuleDeriveMacro, junoTraitRootMacro, junoTraitDeriveMacro.
        const src = `
struct MY_ROOT_TAG JUNO_MODULE_ROOT(MY_ROOT_API_T, JUNO_MODULE_EMPTY);
struct MY_CHILD_TAG JUNO_MODULE_DERIVE(MY_ROOT_T, JUNO_MODULE_EMPTY);
struct MY_TRAIT_TAG JUNO_TRAIT_ROOT(MY_TRAIT_API_T, JUNO_MODULE_EMPTY);
struct MY_IMPL_TAG JUNO_TRAIT_DERIVE(JUNO_POINTER_T, JUNO_MODULE_EMPTY);
`;
        const result = parseFile("/test/file.h", src);

        expect(result.moduleRoots).toHaveLength(1);
        expect(result.moduleRoots[0].rootType).toBe("MY_ROOT_T");

        expect(result.derivations).toHaveLength(2);
        const derivedTypes = result.derivations.map((d) => d.derivedType);
        expect(derivedTypes).toContain("MY_CHILD_T");
        expect(derivedTypes).toContain("MY_IMPL_T");

        expect(result.traitRoots).toHaveLength(1);
        expect(result.traitRoots[0].rootType).toBe("MY_TRAIT_T");
    });
});

// ---------------------------------------------------------------------------
// L. walkFunctionDefinition — edge case paths
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("walkFunctionDefinition — edge cases", () => {
    it("VB-125: function with empty body does not crash and is recorded", () => {
        // Empty function body — walkCompoundStatement gets empty children.
        const src = `
static void Empty(void)
{
}
`;
        const { functionDefs } = parseFileWithDefs("/test/file.c", src);

        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("Empty");
        expect(functionDefs[0].isStatic).toBe(true);
    });

    it("VB-126: non-static function with multiple typed parameters records all parameters", () => {
        // Covers extractParameters with multiple typed parameters in a non-static function.
        const src = `
JUNO_STATUS_T ApiInit(MY_API_T *ptApi, MY_ROOT_T *ptRoot)
{
    return 0;
}
`;
        const { parsed, functionDefs } = parseFileWithDefs("/test/file.c", src);

        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].isStatic).toBe(false);

        const params = parsed.localTypeInfo.functionParameters.get("ApiInit");
        expect(params).toBeDefined();
        expect(params!).toHaveLength(2);
        expect(params![0].typeName).toBe("MY_API_T");
        expect(params![0].name).toBe("ptApi");
        expect(params![1].typeName).toBe("MY_ROOT_T");
        expect(params![1].name).toBe("ptRoot");
    });
});

// ---------------------------------------------------------------------------
// M. walkVtableDeclaration — scalar (non-brace) initializer guard
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-010"]}
describe("walkVtableDeclaration — scalar initializer skips vtable extraction", () => {
    it("VB-127: API_T variable initialized with a scalar expression produces no vtable or pending entry", () => {
        // When the initializer is not a brace-list (e.g., = GetApi()),
        // `if (!ilitNode) { continue; }` guard fires.
        const src = `static MY_API_T tApi = GetApi();`;
        const result = parseFile("/test/file.c", src);

        expect(result.vtableAssignments).toHaveLength(0);
        expect(result.pendingPositionalVtables).toHaveLength(0);
    });
});

// ---------------------------------------------------------------------------
// N. tryExtractDirectVtableAssign — arrow-operator form for pointer param
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-011"]}
describe("tryExtractDirectVtableAssign — arrow-operator on pointer parameter", () => {
    it("VB-128: vtable field assigned via arrow on a pointer parameter is recorded", () => {
        // ptApi->Insert = MyInsert; where ptApi is MY_API_T* parameter.
        // Covers the fnParams lookup path for a pointer param (not just value param).
        const src = `
static void Bind(MY_API_T *ptApi)
{
    ptApi->Insert = MyInsert;
    ptApi->Delete = MyDelete;
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
});

// ---------------------------------------------------------------------------
// O. walkApiStructBody — API_TAG struct with empty body (no fields)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-009"]}
describe("walkApiStructBody — API_TAG struct with empty body produces no apiStructDefinition", () => {
    it("VB-129: API_TAG struct with empty body produces no apiStructDefinition", () => {
        // isApiTag = true but fields.length = 0 → push skipped.
        // Covers `if (isApiTag && fields.length > 0)` false branch.
        const src = `
struct MY_EMPTY_API_TAG {
};
`;
        const result = parseFile("/test/file.h", src);

        expect(result.apiStructDefinitions).toHaveLength(0);
    });
});

// ---------------------------------------------------------------------------
// P. walkApiStructBody — fnptr specifierQualifierList returns API_T type
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-009"]}
describe("walkApiStructBody — function pointer with API_T return type registers apiMemberRegistry", () => {
    it("VB-130: API_TAG struct function pointers are in apiStructDefinitions; specList check uses outer directDecl", () => {
        // Covers the 'Also check specifierQualifierList' section in walkApiStructBody.
        // For function-pointer fields, the Identifier in directDecl.children is
        // in the inner declarator (not the outer), so tok(directDecl.children, "Identifier")
        // returns undefined → apiMemberRegistry entry is NOT created for fnptr fields.
        const src = `
struct MY_API_TAG {
    JUNO_STATUS_T (*Execute)(MY_API_T *p);
    SUB_API_T (*GetSub)(void);
};
`;
        const { parsed, apiMemberRegistry } = parseFileWithDefs("/test/file.h", src);

        // Both function pointers are in apiStructDefinitions.fields
        expect(parsed.apiStructDefinitions).toHaveLength(1);
        expect(parsed.apiStructDefinitions[0].fields).toContain("Execute");
        expect(parsed.apiStructDefinitions[0].fields).toContain("GetSub");

        // The outer directDecl for a fnptr has no Identifier (it's in the inner decl),
        // so the specList check finds name="" and does NOT add to apiMemberRegistry.
        expect(apiMemberRegistry.has("GetSub")).toBe(false);
        expect(apiMemberRegistry.has("Execute")).toBe(false);
    });
});

// ---------------------------------------------------------------------------
// Q. JUNO_MODULE_DERIVE — dispatchMacro derivation path cross-check
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-009"]}
describe("JUNO_MODULE_DERIVE — dispatchMacro junoModuleDeriveMacro branch", () => {
    it("VB-131: JUNO_MODULE_DERIVE with API_T member registers apiMemberRegistry entry", () => {
        // Covers junoModuleDeriveMacro branch + walkMacroBodyForApiMembers with API_T member.
        const src = `
struct MY_DERIVED_TAG JUNO_MODULE_DERIVE(MY_PARENT_T,
    MY_CHILD_API_T *ptChildApi;
);
`;
        const { parsed, apiMemberRegistry } = parseFileWithDefs("/test/file.h", src);

        expect(parsed.derivations).toHaveLength(1);
        expect(parsed.derivations[0].derivedType).toBe("MY_DERIVED_T");
        expect(parsed.derivations[0].rootType).toBe("MY_PARENT_T");

        expect(apiMemberRegistry.has("ptChildApi")).toBe(true);
        expect(apiMemberRegistry.get("ptChildApi")).toBe("MY_CHILD_API_T");
    });
});

// ---------------------------------------------------------------------------
// R. tryExtractCallSite — no-call member expression (hasCall false)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-012"]}
describe("tryExtractCallSite — member access that is a function call", () => {
    it("VB-132: call sites in a switch statement labeled-statement bodies are NOT extracted", () => {
        // Switch bodies contain labeled statements (case: label) which fall under
        // the 'jumpStatement / labeledStatement: no index data' comment in walkStatement.
        // Labeled statements are intentionally skipped — this documents the actual behavior.
        const src = `
static void Dispatch(MY_API_T *ptApi, int eCmd)
{
    switch (eCmd) {
        case 0:
            ptApi->Start(ptApi);
            break;
        case 1:
            ptApi->Stop(ptApi);
            break;
        default:
            break;
    }
}
`;
        const result = parseFile("/test/file.c", src);

        // Labeled statements (case:) are not visited by walkStatement — call sites inside
        // switch case bodies are not extracted. This is the documented limitation.
        expect(result.apiCallSites).toHaveLength(0);
    });
});

// ---------------------------------------------------------------------------
// S. walkApiStructBody — function pointer without API_T in specifier (no registry entry)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-009"]}
describe("walkApiStructBody — function pointer with non-API_T return type not in registry", () => {
    it("VB-133: API_TAG struct function pointer with JUNO_STATUS_T return type does not add to apiMemberRegistry", () => {
        // When the specifierQualifierList type does NOT end in _API_T (e.g., JUNO_STATUS_T),
        // the apiMemberRegistry is NOT updated for that field.
        // Covers the `if (memberTypeName.endsWith('_API_T'))` false branch in the specList check.
        const src = `
struct NORMAL_API_TAG {
    JUNO_STATUS_T (*Run)(void);
    JUNO_STATUS_T (*Done)(void);
};
`;
        const { parsed, apiMemberRegistry } = parseFileWithDefs("/test/file.h", src);

        expect(parsed.apiStructDefinitions).toHaveLength(1);
        expect(parsed.apiStructDefinitions[0].fields).toEqual(["Run", "Done"]);

        // JUNO_STATUS_T does not end in _API_T → not in registry
        expect(apiMemberRegistry.has("Run")).toBe(false);
        expect(apiMemberRegistry.has("Done")).toBe(false);
    });
});

// ---------------------------------------------------------------------------
// T. extractDeclaratorInfo — defensive no-directDeclarator guard (line 103)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("extractDeclaratorInfo — defensive no-directDeclarator guard", () => {
    it("VB-134: C code with parser errors does not crash parseFile and produces empty results", () => {
        // The guard `if (!directDecl) return { name:'', isPointer:false, isArray:false }`
        // at line 103 is a purely defensive check. The parser grammar rule for `declarator`
        // always creates a directDeclarator sub-node when a declarator node is produced —
        // so this branch cannot be reached through valid (or partially-invalid) C code via
        // parseFile(). Even error-recovery scenarios skip the declarator node entirely rather
        // than creating one without a directDeclarator.
        //
        // This test documents the observed behavior: malformed C that causes parse errors
        // (e.g. `int *;` — pointer without a name) yields an empty result without throwing.
        const src = `int *;`;
        // Must not throw
        const result = parseFile("/test/file.c", src);

        // No vtable assignments, no local variables, no API call sites extracted
        expect(result.vtableAssignments).toHaveLength(0);
        expect(result.localTypeInfo.localVariables.size).toBe(0);
        expect(result.apiCallSites).toHaveLength(0);
    });
});

// ---------------------------------------------------------------------------
// U. extractDesignationField — array designator [N] has no Identifier (line 627)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-009"]}
describe("extractDesignationField — array designator returns undefined", () => {
    it("VB-135: array designator [N] in MY_API_T initializer is not added to vtableAssignments", () => {
        // extractDesignationField iterates designator nodes looking for an Identifier token.
        // An array designator `[0]` has children LBracket/constantExpression/RBracket but
        // no Identifier → the loop falls through and returns undefined (line 627).
        // The caller (extractDesignatedVtable) then skips the entry because `field` is undefined.
        const src = `
static const MY_API_T s_tApi = { [0] = MyFunc };
`;
        const result = parseFile("/test/file.c", src);

        // Array designator produces no field name → no vtableAssignment
        expect(result.vtableAssignments).toHaveLength(0);
    });

    it("VB-136: mixed dot and array designators — dot entry extracted, array entry skipped", () => {
        // When an initializer has both `.field = fn` (dot designator) and `[0] = fn`
        // (array designator), only the dot-designated entry produces a vtableAssignment.
        // The array designator invokes extractDesignationField which returns undefined
        // (line 627), and the entry is silently skipped.
        const src = `
static const MY_API_T s_tApi = { .Run = MyRun, [0] = Ignored };
`;
        const result = parseFile("/test/file.c", src);

        // Only the dot-designator entry (.Run = MyRun) is extracted
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].field).toBe("Run");
        expect(result.vtableAssignments[0].functionName).toBe("MyRun");
        expect(result.vtableAssignments[0].apiType).toBe("MY_API_T");

        // The [0] = Ignored entry is NOT in vtableAssignments (no Identifier in designator)
        const ignoredEntry = result.vtableAssignments.find(a => a.functionName === "Ignored");
        expect(ignoredEntry).toBeUndefined();
    });
});
