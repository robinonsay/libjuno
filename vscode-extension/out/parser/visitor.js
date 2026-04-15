"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.parseFile = parseFile;
exports.parseFileWithDefs = parseFileWithDefs;
const lexer_1 = require("./lexer");
const parser_1 = require("./parser");
// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
function isToken(node) {
    return "image" in node;
}
function tok(children, key, idx = 0) {
    const arr = children[key];
    if (!arr || idx >= arr.length) {
        return undefined;
    }
    const entry = arr[idx];
    return isToken(entry) ? entry : undefined;
}
function child(children, key, idx = 0) {
    const arr = children[key];
    if (!arr || idx >= arr.length) {
        return undefined;
    }
    const entry = arr[idx];
    return !isToken(entry) ? entry : undefined;
}
function hasToken(children, key) {
    return !!(children[key]?.length);
}
/** Replace `_TAG` suffix with `_T`. For `_API_TAG` suffix, replaces with `_API_T`. */
function tagToType(tag) {
    return tag.replace(/_TAG$/, "_T");
}
// ---------------------------------------------------------------------------
// Extract the type name from declarationSpecifiers
// ---------------------------------------------------------------------------
function extractTypeName(declSpecNode) {
    let typeName = "";
    let isConst = false;
    let isStatic = false;
    const c = declSpecNode.children;
    // Static
    for (const node of c["storageClassSpecifier"] ?? []) {
        const n = node;
        if (hasToken(n.children, "Static")) {
            isStatic = true;
        }
    }
    // Walk typeQualifier nodes for Const
    for (const node of c["typeQualifier"] ?? []) {
        const n = node;
        if (hasToken(n.children, "Const")) {
            isConst = true;
        }
    }
    // Walk typeSpecifier nodes for the type name
    for (const node of c["typeSpecifier"] ?? []) {
        const n = node;
        // Could be an Identifier (custom type like JUNO_DS_HEAP_API_T)
        const ident = tok(n.children, "Identifier");
        if (ident) {
            typeName = ident.image;
            break;
        }
        // Could be a primitiveType — not interesting for our purposes
    }
    return { typeName, isConst, isStatic };
}
// ---------------------------------------------------------------------------
// Extract variable name and pointer/array flags from a declarator
// ---------------------------------------------------------------------------
function extractDeclaratorInfo(declaratorNode) {
    const c = declaratorNode.children;
    const isPointer = hasToken(c, "pointer") || !!(c["pointer"]?.length);
    const directDecl = child(c, "directDeclarator");
    if (!directDecl) {
        return { name: "", isPointer: false, isArray: false };
    }
    const dc = directDecl.children;
    // The first token in directDeclarator is the identifier (function name or variable name)
    const ident = tok(dc, "Identifier");
    // Check if it has array suffix
    const isArray = hasToken(dc, "LBracket");
    // Pointer: present in declarator
    const ptrNode = child(c, "pointer");
    return { name: ident?.image ?? "", isPointer: !!ptrNode, isArray };
}
// ---------------------------------------------------------------------------
// Drill through expression layers to reach assignmentExpression
// ---------------------------------------------------------------------------
function drillToAssignmentExpr(exprNode) {
    // expression → assignmentExpression*
    const ae = child(exprNode.children, "assignmentExpression");
    return ae ?? undefined;
}
// Drill from assignmentExpression down to postfixExpression (through the
// conditional → logicalOr → … → unary → postfix chain)
function drillToPostfix(node) {
    const names = [
        "conditionalExpression",
        "logicalOrExpression",
        "logicalAndExpression",
        "bitwiseOrExpression",
        "bitwiseXorExpression",
        "bitwiseAndExpression",
        "equalityExpression",
        "relationalExpression",
        "shiftExpression",
        "additiveExpression",
        "multiplicativeExpression",
        "castExpression",
        "unaryExpression",
        "postfixExpression",
    ];
    let cur = node;
    for (const name of names) {
        const next = child(cur.children, name);
        if (!next) {
            return cur.name === "postfixExpression" ? cur : undefined;
        }
        cur = next;
    }
    return cur;
}
function getPostfixPrimary(postNode) {
    const primary = child(postNode.children, "primaryExpression");
    if (!primary) {
        return undefined;
    }
    const ident = tok(primary.children, "Identifier");
    return ident?.image;
}
function getPostfixSuffixes(postNode) {
    const suffixes = [];
    const c = postNode.children;
    // Chevrotain stores repeated nodes as arrays — we need to interleave
    // by their source order (startOffset).
    const allChildren = Object.values(c).flat();
    // Sort all non-primary children by start offset
    const sorted = allChildren
        .filter((n) => {
        if (isToken(n)) {
            return true;
        }
        return n.name !== "primaryExpression";
    })
        .sort((a, b) => {
        const aOff = isToken(a) ? a.startOffset : (a.location?.startOffset ?? 0);
        const bOff = isToken(b) ? b.startOffset : (b.location?.startOffset ?? 0);
        return aOff - bOff;
    });
    for (const entry of sorted) {
        if (isToken(entry)) {
            const t = entry;
            const tName = t.tokenType?.name ?? "";
            if (tName === "JunoFailureHandler" || tName === "JunoFailureUserData") {
                // These appear after a . or -> which was already processed; mark last suffix
                if (suffixes.length > 0 && (suffixes[suffixes.length - 1].kind === "arrow" || suffixes[suffixes.length - 1].kind === "dot")) {
                    suffixes[suffixes.length - 1].member = t.image;
                    suffixes[suffixes.length - 1].isFhMember = true;
                }
            }
        }
        else {
            const n = entry;
            if (n.name === "memberIdentifier") {
                // Determine kind from sibling token (ArrowOp or Dot)
                // The member identifier appears after ArrowOp or Dot
                const ident = tok(n.children, "Identifier");
                const fhTok = (n.children["JunoFailureHandler"] ?? n.children["JunoFailureUserData"]);
                const isFh = !!(fhTok?.length);
                const memberName = ident?.image ?? (fhTok ? fhTok[0].image : "");
                // We'll fix up the kind after we track ArrowOp/Dot in prior token
                suffixes.push({ kind: "dot", member: memberName, isFhMember: isFh });
            }
        }
    }
    return suffixes;
}
// ---------------------------------------------------------------------------
// Main walker class
// ---------------------------------------------------------------------------
const parserInstance = new parser_1.CParser();
class CSTWalker {
    constructor(filePath) {
        this.currentFunction = null;
        this.insideFunctionBody = false;
        /** Gathered during walk — exposed for Workspace Indexer to merge. */
        this._functionDefs = [];
        /** Side-channel for apiMemberRegistry (merged by Workspace Indexer). */
        this._apiMemberRegistry = new Map();
        this.filePath = filePath;
        this.result = {
            filePath,
            moduleRoots: [],
            traitRoots: [],
            derivations: [],
            apiStructDefinitions: [],
            vtableAssignments: [],
            failureHandlerAssigns: [],
            apiCallSites: [],
            localTypeInfo: {
                localVariables: new Map(),
                functionParameters: new Map(),
            },
        };
    }
    // -----------------------------------------------------------------------
    // Public entry
    // -----------------------------------------------------------------------
    walk(rootNode) {
        this.walkTranslationUnit(rootNode);
        return this.result;
    }
    // -----------------------------------------------------------------------
    // walkTranslationUnit
    // -----------------------------------------------------------------------
    walkTranslationUnit(node) {
        const c = node.children;
        // Both externalDeclaration and preprocessorDirective are arrays of CstNodes
        for (const n of (c["externalDeclaration"] ?? [])) {
            this.walkExternalDeclaration(n);
        }
        for (const n of (c["preprocessorDirective"] ?? [])) {
            this.walkPreprocessorDirective(n);
        }
    }
    walkExternalDeclaration(node) {
        const c = node.children;
        if (c["functionDefinition"]?.length) {
            this.walkFunctionDefinition(c["functionDefinition"][0]);
        }
        else if (c["declaration"]?.length) {
            this.walkDeclaration(c["declaration"][0]);
        }
        // junoStandaloneDeclaration: no data to extract for the index
    }
    // -----------------------------------------------------------------------
    // walkPreprocessorDirective — no data extracted (satisfies design 3.1.3 §7)
    // -----------------------------------------------------------------------
    walkPreprocessorDirective(_node) {
        // Opaque directive lines: no actionable data for the navigation index.
    }
    // -----------------------------------------------------------------------
    // walkFunctionDefinition
    // -----------------------------------------------------------------------
    walkFunctionDefinition(node) {
        const c = node.children;
        const declSpec = child(c, "declarationSpecifiers");
        const declarator = child(c, "declarator");
        const body = child(c, "compoundStatement");
        if (!declSpec || !declarator) {
            return;
        }
        const { isStatic } = extractTypeName(declSpec);
        // Function name is the Identifier in the innermost directDeclarator
        const directDecl = child(declarator.children, "directDeclarator");
        const fnName = directDecl ? (tok(directDecl.children, "Identifier")?.image ?? "") : "";
        if (!fnName) {
            return;
        }
        const line = tok(directDecl?.children ?? {}, "Identifier")?.startLine ?? 0;
        const rec = {
            functionName: fnName,
            file: this.filePath,
            line,
            isStatic,
        };
        // Emit function definition — stored outside ParsedFile; callers merge into index.
        // But ParsedFile itself does not have a functionDefinitions field;
        // the NavigationIndex has it. The Workspace Indexer merges it.
        // We still need to populate localTypeInfo.functionParameters.
        // Emit into a temporary holder; the Workspace Indexer will pick it up via
        // this._functionDefs — however ParsedFile has no such field.
        // Per the design, ParsedFile.localTypeInfo holds per-function data.
        // FunctionDefinitionRecord is part of NavigationIndex, not ParsedFile.
        // We append them via the indexer. For now, store them so walkFunctionDefinition
        // can be called and the indexer can gather them via a separate getter.
        this._functionDefs.push(rec);
        // Extract parameters → functionParameters
        const params = this.extractParameters(node, fnName);
        this.result.localTypeInfo.functionParameters.set(fnName, params);
        // Walk body for local declarations and expression statements
        this.currentFunction = fnName;
        this.insideFunctionBody = true;
        if (body) {
            this.walkCompoundStatement(body);
        }
        this.insideFunctionBody = false;
        this.currentFunction = null;
    }
    // -----------------------------------------------------------------------
    // extractParameters
    // -----------------------------------------------------------------------
    extractParameters(fnDefNode, fnName) {
        const params = [];
        const declarator = child(fnDefNode.children, "declarator");
        if (!declarator) {
            return params;
        }
        // directDeclarator → parameterTypeList → parameterList → parameterDeclaration[]
        const directDecl = child(declarator.children, "directDeclarator");
        if (!directDecl) {
            return params;
        }
        const ptl = child(directDecl.children, "parameterTypeList");
        if (!ptl) {
            return params;
        }
        const pl = child(ptl.children, "parameterList");
        if (!pl) {
            return params;
        }
        for (const pdEntry of (pl.children["parameterDeclaration"] ?? [])) {
            const ti = this.extractTypeInfoFromParamDecl(pdEntry, fnName);
            if (ti) {
                params.push(ti);
            }
        }
        return params;
    }
    extractTypeInfoFromParamDecl(pdNode, _fnName) {
        const c = pdNode.children;
        const declSpec = child(c, "declarationSpecifiers");
        if (!declSpec) {
            return null;
        }
        const { typeName, isConst } = extractTypeName(declSpec);
        if (!typeName) {
            return null;
        }
        // Declarator (could be absent for abstract declarators)
        const declarator = child(c, "declarator");
        let name = "";
        let isPointer = false;
        let isArray = false;
        if (declarator) {
            const info = extractDeclaratorInfo(declarator);
            name = info.name;
            isPointer = info.isPointer;
            isArray = info.isArray;
        }
        else {
            // Check abstractDeclarator for pointer
            const absDecl = child(c, "abstractDeclarator");
            if (absDecl && absDecl.children["pointer"]?.length) {
                isPointer = true;
            }
        }
        return { name, typeName, isPointer, isConst, isArray };
    }
    // -----------------------------------------------------------------------
    // walkCompoundStatement
    // -----------------------------------------------------------------------
    walkCompoundStatement(node) {
        const c = node.children;
        // Local declarations
        for (const n of (c["declaration"] ?? [])) {
            this.walkDeclaration(n);
        }
        // Statements (may contain nested compound statements, expression statements, etc.)
        for (const n of (c["statement"] ?? [])) {
            this.walkStatement(n);
        }
    }
    walkStatement(node) {
        const c = node.children;
        if (c["compoundStatement"]?.length) {
            this.walkCompoundStatement(c["compoundStatement"][0]);
        }
        else if (c["expressionStatement"]?.length) {
            this.walkExpressionStatement(c["expressionStatement"][0]);
        }
        else if (c["selectionStatement"]?.length) {
            this.walkSelectionStatement(c["selectionStatement"][0]);
        }
        else if (c["iterationStatement"]?.length) {
            this.walkIterationStatement(c["iterationStatement"][0]);
        }
        // jumpStatement / labeledStatement: no index data
    }
    walkSelectionStatement(node) {
        for (const n of (node.children["statement"] ?? [])) {
            this.walkStatement(n);
        }
    }
    walkIterationStatement(node) {
        for (const n of (node.children["statement"] ?? [])) {
            this.walkStatement(n);
        }
    }
    // -----------------------------------------------------------------------
    // walkDeclaration
    // -----------------------------------------------------------------------
    walkDeclaration(node) {
        const c = node.children;
        const declSpec = child(c, "declarationSpecifiers");
        if (!declSpec) {
            return;
        }
        const { typeName, isConst, isStatic } = extractTypeName(declSpec);
        if (this.insideFunctionBody) {
            // Local variable declaration → populate localVariables
            this.walkLocalDeclaration(node, typeName, isConst, isStatic);
            return;
        }
        // Top-level: detect vtable assignments (struct with _API_T type)
        if (typeName.endsWith("_API_T")) {
            this.walkVtableDeclaration(node, typeName);
        }
        // Walk any structOrUnionSpecifier in the declarationSpecifiers
        for (const n of (declSpec.children["typeSpecifier"] ?? [])) {
            const sus = child(n.children, "structOrUnionSpecifier");
            if (sus) {
                this.walkStructOrUnionSpecifier(sus);
            }
        }
    }
    // -----------------------------------------------------------------------
    // walkLocalDeclaration — inside function bodies
    // -----------------------------------------------------------------------
    walkLocalDeclaration(node, typeName, isConst, _isStatic) {
        const fn = this.currentFunction;
        if (!fn || !typeName) {
            return;
        }
        const c = node.children;
        const idl = child(c, "initDeclaratorList");
        if (!idl) {
            return;
        }
        if (!this.result.localTypeInfo.localVariables.has(fn)) {
            this.result.localTypeInfo.localVariables.set(fn, new Map());
        }
        const fnMap = this.result.localTypeInfo.localVariables.get(fn);
        for (const entry of (idl.children["initDeclarator"] ?? [])) {
            const declarator = child(entry.children, "declarator");
            if (!declarator) {
                continue;
            }
            const info = extractDeclaratorInfo(declarator);
            if (!info.name) {
                continue;
            }
            const ti = {
                name: info.name,
                typeName,
                isPointer: info.isPointer,
                isConst,
                isArray: info.isArray,
            };
            fnMap.set(info.name, ti);
        }
    }
    // -----------------------------------------------------------------------
    // walkVtableDeclaration — at top-level, for const API_T var = { ... }
    // -----------------------------------------------------------------------
    walkVtableDeclaration(node, apiType) {
        const c = node.children;
        const idl = child(c, "initDeclaratorList");
        if (!idl) {
            return;
        }
        for (const entry of (idl.children["initDeclarator"] ?? [])) {
            const initNode = child(entry.children, "initializer");
            if (!initNode) {
                continue;
            }
            // Brace-enclosed initializer list?
            const ilitNode = child(initNode.children, "initializerList");
            if (!ilitNode) {
                continue;
            }
            // Collect all designation+initializer pairs
            const ilic = ilitNode.children;
            const designations = (ilic["designation"] ?? []);
            const initializers = (ilic["initializer"] ?? []);
            if (designations.length > 0) {
                // Designated initializer
                this.extractDesignatedVtable(apiType, designations, initializers, ilitNode);
            }
            else {
                // Positional initializer
                this.extractPositionalVtable(apiType, initializers, ilitNode);
            }
        }
    }
    extractDesignatedVtable(apiType, designations, initializers, ilitNode) {
        // Interleave designations with their initializers by source order.
        // initializerList: (designation? initializer)* so they are co-indexed.
        // Chevrotain stores all designation nodes in one array and all initializer
        // nodes in another — but they appear in document order within each array.
        // We zip them positionally (N-th designation matches N-th initializer).
        const entries = [];
        // We need to reconstruct the pairs. The initializerList grammar is:
        //   (designation? initializer) (',' designation? initializer)*
        // Chevrotain stores all designation subrule results in one array and
        // all initializer subrule results in another.
        // Their relative order is preserved within each array.
        // We zip directly: designations[i] pairs with initializers[i] when i < designations.length.
        let dIdx = 0;
        for (let i = 0; i < initializers.length; i++) {
            const initN = initializers[i];
            // Find the line of this assignment
            const line = (initN.location?.startLine ?? 0);
            // Check whether this initializer is preceded by a designation.
            // Since designation and initializer are co-indexed, designations[dIdx]
            // exists only when the current slot has one.
            // We detect this by checking if there's a designation whose end offset
            // is before the start of initN.
            let desig = null;
            if (dIdx < designations.length) {
                const d = designations[dIdx];
                const dEnd = d.location?.endOffset ?? 0;
                const iStart = initN.location?.startOffset ?? Infinity;
                if (dEnd < iStart) {
                    desig = d;
                    dIdx++;
                }
            }
            entries.push({ desig, init: initN, line });
        }
        for (const { desig, init, line } of entries) {
            if (!desig) {
                continue;
            } // skip un-designated entries in a mixed list
            const field = this.extractDesignationField(desig);
            const fnName = this.extractInitializerIdent(init);
            if (field && fnName) {
                this.result.vtableAssignments.push({
                    apiType,
                    field,
                    functionName: fnName,
                    file: this.filePath,
                    line,
                });
            }
        }
    }
    extractDesignationField(desigNode) {
        // designation → designator+ '='
        for (const desigEntry of (desigNode.children["designator"] ?? [])) {
            // designator → '.' Identifier (for field designation)
            const ident = tok(desigEntry.children, "Identifier");
            if (ident) {
                return ident.image;
            }
        }
        return undefined;
    }
    extractInitializerIdent(initNode) {
        // initializer → assignmentExpression | '{' initializerList '}'
        const ae = child(initNode.children, "assignmentExpression");
        if (!ae) {
            return undefined;
        }
        const pf = drillToPostfix(ae);
        if (!pf) {
            return undefined;
        }
        return getPostfixPrimary(pf);
    }
    extractPositionalVtable(apiType, initializers, _ilitNode) {
        // Look up the field order from already-parsed API struct definitions
        const apiRec = this.result.apiStructDefinitions.find((r) => r.apiType === apiType);
        if (!apiRec) {
            // Can't resolve positional — defer (indexer handles cross-file case)
            return;
        }
        const fields = apiRec.fields;
        for (let i = 0; i < initializers.length && i < fields.length; i++) {
            const fnName = this.extractInitializerIdent(initializers[i]);
            if (!fnName) {
                continue;
            }
            const line = initializers[i].location?.startLine ?? 0;
            this.result.vtableAssignments.push({
                apiType,
                field: fields[i],
                functionName: fnName,
                file: this.filePath,
                line,
            });
        }
    }
    // -----------------------------------------------------------------------
    // walkStructOrUnionSpecifier
    // -----------------------------------------------------------------------
    walkStructOrUnionSpecifier(node) {
        const c = node.children;
        const line = node.location?.startLine ?? 0;
        // Has body → body form uses Identifier (first CONSUME) as optional tag
        const hasBody = hasToken(c, "LBrace");
        if (!hasBody) {
            // Macro form: struct TAG JUNO_XXX(...)
            // Tag is under Identifier[0] — CONSUME2(Identifier) in this OR2 branch
            // is the only Identifier consumed, so Chevrotain places it at index 0.
            const tagTok = tok(c, "Identifier");
            if (!tagTok) {
                return;
            }
            const tag = tagTok.image;
            const macroCst = child(c, "junoMacroInvocation");
            if (macroCst) {
                this.dispatchMacro(macroCst, tag, line);
            }
            return;
        }
        // Body form
        const tagTok = tok(c, "Identifier"); // optional in grammar
        const tag = tagTok?.image ?? "";
        const sdl = child(c, "structDeclarationList");
        this.walkApiStructBody(tag, sdl, line);
    }
    dispatchMacro(macroCst, tag, line) {
        const mc = macroCst.children;
        if (mc["junoModuleRootMacro"]?.length) {
            const m = mc["junoModuleRootMacro"][0];
            const apiType = tok(m.children, "Identifier")?.image ?? "";
            if (!apiType) {
                return;
            }
            const rec = {
                rootType: tagToType(tag),
                apiType,
                file: this.filePath,
                line,
            };
            this.result.moduleRoots.push(rec);
            // Also walk macro body tokens for nested struct members
            this.walkMacroBodyForApiMembers(m, tag);
        }
        else if (mc["junoModuleDeriveMacro"]?.length) {
            const m = mc["junoModuleDeriveMacro"][0];
            const rootType = tok(m.children, "Identifier")?.image ?? "";
            if (!rootType) {
                return;
            }
            const rec = {
                derivedType: tagToType(tag),
                rootType,
                file: this.filePath,
                line,
            };
            this.result.derivations.push(rec);
            this.walkMacroBodyForApiMembers(m, tag);
        }
        else if (mc["junoTraitRootMacro"]?.length) {
            const m = mc["junoTraitRootMacro"][0];
            const apiType = tok(m.children, "Identifier")?.image ?? "";
            if (!apiType) {
                return;
            }
            const rec = {
                rootType: tagToType(tag),
                apiType,
                file: this.filePath,
                line,
            };
            this.result.traitRoots.push(rec);
            this.walkMacroBodyForApiMembers(m, tag);
        }
        else if (mc["junoTraitDeriveMacro"]?.length) {
            const m = mc["junoTraitDeriveMacro"][0];
            const rootType = tok(m.children, "Identifier")?.image ?? "";
            if (!rootType) {
                return;
            }
            const rec = {
                derivedType: tagToType(tag),
                rootType,
                file: this.filePath,
                line,
            };
            this.result.derivations.push(rec);
            this.walkMacroBodyForApiMembers(m, tag);
        }
        // junoModuleMacro: no root/derive data to extract for the index
    }
    /**
     * Walk the macroBodyTokens of a JUNO_MODULE_ROOT / JUNO_MODULE_DERIVE etc.
     * macro to find any member declarations whose type ends in `_API_T`.
     * These populate apiMemberRegistry.
     *
     * macroBodyTokens is an opaque token sequence, but we look for the pattern:
     *   Identifier(_API_T) '*'? Identifier(memberName) ';'
     * using a simple linear scan of the flat token stream.
     */
    walkMacroBodyForApiMembers(macroRuleNode, _tag) {
        const bodyNode = child(macroRuleNode.children, "macroBodyTokens");
        if (!bodyNode) {
            return;
        }
        // macroBodyTokens children is a flat bag of tokens (all consumed via consumeToken())
        // They are stored under their respective token type keys.
        // We need them in source order.
        const allToks = Object.values(bodyNode.children).flat()
            .filter(isToken)
            .sort((a, b) => a.startOffset - b.startOffset);
        // Scan for pattern: IDENT(_API_T) [*] IDENT ;
        for (let i = 0; i < allToks.length - 1; i++) {
            const t = allToks[i];
            if (t.image.endsWith("_API_T")) {
                // Look ahead for member name (skip optional '*')
                let j = i + 1;
                if (j < allToks.length && allToks[j].image === "*") {
                    j++;
                }
                if (j < allToks.length && /^[a-zA-Z_][a-zA-Z0-9_]*$/.test(allToks[j].image)) {
                    const memberName = allToks[j].image;
                    this.result.apiCallSites; // touch to avoid unused warning
                    // Populate apiMemberRegistry — stored in a side channel exposed via getter
                    this._apiMemberRegistry.set(memberName, t.image);
                }
            }
        }
    }
    // -----------------------------------------------------------------------
    // walkApiStructBody — `struct SOME_API_TAG { ... }`
    // -----------------------------------------------------------------------
    walkApiStructBody(tag, sdlNode, line) {
        if (!tag) {
            return;
        }
        const isApiTag = tag.endsWith("_API_TAG");
        const fields = [];
        if (sdlNode) {
            for (const sdEntry of (sdlNode.children["structDeclaration"] ?? [])) {
                // Function pointer field: structDeclarator → declarator
                // directDeclarator form: '(' '*' Identifier ')' '(' parameterTypeList ')'
                const sdlc = sdEntry.children;
                const sdList = child(sdlc, "structDeclaratorList");
                if (!sdList) {
                    continue;
                }
                for (const sd of (sdList.children["structDeclarator"] ?? [])) {
                    const declarator = child(sd.children, "declarator");
                    if (!declarator) {
                        continue;
                    }
                    const directDecl = child(declarator.children, "directDeclarator");
                    if (!directDecl) {
                        continue;
                    }
                    // Function pointer has nested declarator inside '(' ... ')' suffix
                    // directDeclarator children: LParen, declarator (inner), RParen,
                    // then another LParen, parameterTypeList, RParen
                    const innerDecl = child(directDecl.children, "declarator");
                    if (innerDecl) {
                        // This is a function pointer: (inner) -> look at inner's direct declarator
                        const innerDirect = child(innerDecl.children, "directDeclarator");
                        if (innerDirect) {
                            const ptrNode = child(innerDecl.children, "pointer");
                            const fieldName = tok(innerDirect.children, "Identifier")?.image;
                            if (fieldName && ptrNode) {
                                if (isApiTag) {
                                    fields.push(fieldName);
                                }
                                // fall through to also register as member
                            }
                        }
                    }
                    else {
                        // Regular member
                        const fieldName = tok(directDecl.children, "Identifier")?.image;
                        if (fieldName) {
                            // Check if the specifier type ends in _API_T → apiMemberRegistry
                            const specList = child(sdlc, "specifierQualifierList");
                            if (specList) {
                                const memberTypeName = this.extractTypeFromSpecifierQualifierList(specList);
                                if (memberTypeName.endsWith("_API_T")) {
                                    this._apiMemberRegistry.set(fieldName, memberTypeName);
                                }
                            }
                        }
                    }
                    // Also check specifierQualifierList for _API_T members (non-fnptr case)
                    const specList = child(sdlc, "specifierQualifierList");
                    if (specList) {
                        const memberTypeName = this.extractTypeFromSpecifierQualifierList(specList);
                        if (memberTypeName.endsWith("_API_T")) {
                            // Get member name from the declarator
                            const name = tok(directDecl.children, "Identifier")?.image ?? "";
                            if (name) {
                                this._apiMemberRegistry.set(name, memberTypeName);
                            }
                        }
                    }
                }
            }
        }
        if (isApiTag && fields.length > 0) {
            const apiType = tag.replace(/_API_TAG$/, "_API_T");
            const rec = {
                apiType,
                fields,
                file: this.filePath,
                line,
            };
            this.result.apiStructDefinitions.push(rec);
        }
    }
    extractTypeFromSpecifierQualifierList(sqlNode) {
        for (const tsEntry of (sqlNode.children["typeSpecifier"] ?? [])) {
            const ident = tok(tsEntry.children, "Identifier");
            if (ident) {
                return ident.image;
            }
        }
        return "";
    }
    // -----------------------------------------------------------------------
    // walkExpressionStatement — direct vtable assignment and failure handler
    // -----------------------------------------------------------------------
    walkExpressionStatement(node) {
        if (!this.insideFunctionBody) {
            return;
        }
        const c = node.children;
        const exprNode = child(c, "expression");
        if (!exprNode) {
            return;
        }
        const ae = drillToAssignmentExpr(exprNode);
        if (!ae) {
            return;
        }
        const aeC = ae.children;
        // Check if this is an assignment: conditionalExpression assignmentOperator assignmentExpression
        if (!aeC["assignmentOperator"]?.length) {
            // Not an assignment — check for call sites
            this.tryExtractCallSite(ae);
            return;
        }
        const lhsCond = child(aeC, "conditionalExpression");
        const rhsAE = child(aeC, "assignmentExpression");
        if (!lhsCond || !rhsAE) {
            return;
        }
        const lhsPostfix = drillToPostfix(lhsCond);
        if (!lhsPostfix) {
            return;
        }
        // Get RHS identifier (the function being assigned)
        const rhsPostfix = drillToPostfix(rhsAE);
        const functionName = rhsPostfix ? getPostfixPrimary(rhsPostfix) : undefined;
        if (!functionName) {
            return;
        }
        const assignLine = aeC["assignmentOperator"][0].location?.startLine
            ?? lhsPostfix.location?.startLine ?? 0;
        // Check for failure handler: LHS ends with JunoFailureHandler member access
        if (this.tryExtractFailureHandler(lhsPostfix, functionName, assignLine)) {
            return;
        }
        // Otherwise: direct vtable assignment (varName.Field = func or varName->Field = func)
        this.tryExtractDirectVtableAssign(lhsPostfix, functionName, assignLine);
    }
    tryExtractFailureHandler(postfix, functionName, line) {
        // Pattern: any prefix '.' JunoFailureHandler or '->' JunoFailureHandler
        // We scan the postfix expression's children for a JunoFailureHandler token
        // at the memberIdentifier level.
        const memberIdents = postfix.children["memberIdentifier"] ?? [];
        for (const mi of memberIdents) {
            if (hasToken(mi.children, "JunoFailureHandler") || hasToken(mi.children, "JunoFailureUserData")) {
                // rootType resolved at index-merge time
                this.result.failureHandlerAssigns.push({
                    rootType: "", // resolved at index-merge time from LocalTypeInfo
                    functionName,
                    file: this.filePath,
                    line,
                });
                return true;
            }
        }
        return false;
    }
    tryExtractDirectVtableAssign(lhsPostfix, functionName, line) {
        // Pattern: identifier '.' field = func  →  tApi.Insert = JunoDs_Heap_Insert
        // The LHS postfix primary is the variable name; the last memberIdentifier is the field.
        const varName = getPostfixPrimary(lhsPostfix);
        if (!varName) {
            return;
        }
        const memberIdents = (lhsPostfix.children["memberIdentifier"] ?? []);
        if (memberIdents.length === 0) {
            return;
        }
        const lastMember = memberIdents[memberIdents.length - 1];
        const field = tok(lastMember.children, "Identifier")?.image;
        if (!field) {
            return;
        }
        // Look up the variable type from localTypeInfo to get apiType
        const fn = this.currentFunction;
        if (!fn) {
            return;
        }
        const fnVars = this.result.localTypeInfo.localVariables.get(fn);
        const fnParams = this.result.localTypeInfo.functionParameters.get(fn) ?? [];
        let apiType;
        if (fnVars) {
            const vi = fnVars.get(varName);
            if (vi?.typeName.endsWith("_API_T")) {
                apiType = vi.typeName;
            }
        }
        if (!apiType) {
            const pi = fnParams.find((p) => p.name === varName);
            if (pi?.typeName.endsWith("_API_T")) {
                apiType = pi.typeName;
            }
        }
        if (!apiType) {
            return;
        }
        this.result.vtableAssignments.push({
            apiType,
            field,
            functionName,
            file: this.filePath,
            line,
        });
    }
    // -----------------------------------------------------------------------
    // tryExtractCallSite — `ptApi->Method(args)` patterns
    // -----------------------------------------------------------------------
    tryExtractCallSite(ae) {
        const postfix = drillToPostfix(ae);
        if (!postfix) {
            return;
        }
        const pc = postfix.children;
        // Requires at least: primaryExpression → varName, then '->' memberIdentifier, then '(' args ')'
        const varName = getPostfixPrimary(postfix);
        if (!varName) {
            return;
        }
        const memberIdents = (pc["memberIdentifier"] ?? []);
        if (memberIdents.length === 0) {
            return;
        }
        // Detect if there's a call (LParen following the last member)
        // In the postfix expression, the call '(' is represented by LParen tokens.
        // We look for the pattern: memberIdentifier followed by LParen.
        // Since all such children are in the postfix node, we check for ArrowOp or Dot
        // followed by memberIdentifier followed by LParen.
        const hasCall = hasToken(pc, "LParen");
        if (!hasCall) {
            return;
        }
        // The field being called is the last memberIdentifier before the call
        const lastMember = memberIdents[memberIdents.length - 1];
        const field = tok(lastMember.children, "Identifier")?.image;
        if (!field) {
            return;
        }
        const line = postfix.location?.startLine ?? 0;
        const col = tok(lastMember.children, "Identifier")?.startColumn ?? 0;
        this.result.apiCallSites.push({
            variableName: varName,
            fieldName: field,
            file: this.filePath,
            line,
            column: col,
        });
    }
}
// ---------------------------------------------------------------------------
// Shared parser instance (CParser is state-free between parses)
// ---------------------------------------------------------------------------
let sharedParser = null;
function getParser() {
    if (!sharedParser) {
        sharedParser = new parser_1.CParser();
    }
    return sharedParser;
}
// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
/**
 * Parse a single C/C++ source file and extract all navigation index data.
 *
 * @param filePath  Absolute path of the file (stored verbatim in records).
 * @param text      Raw file content.
 * @returns         Populated ParsedFile ready for Workspace Indexer merge.
 */
function parseFile(filePath, text) {
    const lexResult = lexer_1.CLexer.tokenize(text);
    const parser = getParser();
    parser.input = lexResult.tokens;
    const cst = parser.translationUnit();
    const walker = new CSTWalker(filePath);
    const parsed = walker.walk(cst);
    return parsed;
}
/**
 * Parse a file and also return function definition records for indexer merging.
 * The NavigationIndex.functionDefinitions field is not part of ParsedFile, so
 * the Workspace Indexer calls this variant to obtain both.
 */
function parseFileWithDefs(filePath, text) {
    const lexResult = lexer_1.CLexer.tokenize(text);
    const parser = getParser();
    parser.input = lexResult.tokens;
    const cst = parser.translationUnit();
    const walker = new CSTWalker(filePath);
    const parsed = walker.walk(cst);
    return {
        parsed,
        functionDefs: walker._functionDefs,
        apiMemberRegistry: walker._apiMemberRegistry,
    };
}
//# sourceMappingURL=visitor.js.map