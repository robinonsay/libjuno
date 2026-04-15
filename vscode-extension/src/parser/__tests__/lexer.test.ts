/**
 * @file lexer.test.ts
 *
 * Jest tests for the LibJuno Chevrotain lexer (lexer.ts).
 *
 * Tests verify token recognition, priority ordering, compound operators,
 * literals, whitespace/comment skipping, and full-snippet tokenization.
 */

/// <reference types="jest" />

import { CLexer } from "../lexer";
import {
    // LibJuno macro tokens
    JunoModuleRoot,
    JunoModuleDerive,
    JunoModule,
    JunoTraitRoot,
    JunoTraitDerive,
    JunoModuleDeclare,
    JunoModuleRootDeclare,
    JunoModuleDeriveDeclare,
    JunoModuleResult,
    JunoModuleGetApi,
    JunoModuleSuper,
    JunoModuleEmpty,
    JunoModuleArg,
    JunoFailureHandler,
    JunoFailureUserData,
    // Keywords
    Static,
    Const,
    Struct,
    // Punctuators — compound
    ArrowOp,
    EqEq,
    LShiftAssign,
    Ellipsis,
    // Punctuators — simple
    LParen,
    RParen,
    LBrace,
    RBrace,
    Semicolon,
    Comma,
    Dot,
    Assign,
    // Literals
    IntegerLiteral,
    FloatingLiteral,
    StringLiteral,
    CharLiteral,
    // Catch-all
    Identifier,
    // Preprocessor
    HashDirective,
} from "../lexer";

// ---------------------------------------------------------------------------
// Helper — tokenize text and assert no lexer errors
// ---------------------------------------------------------------------------

function tokenize(text: string) {
    const result = CLexer.tokenize(text);
    expect(result.errors).toHaveLength(0);
    return result.tokens;
}

// ---------------------------------------------------------------------------
// 1. LibJuno Macro Token Recognition
// ---------------------------------------------------------------------------

describe("LibJuno macro token recognition", () => {
    const cases: [string, string, object][] = [
        ["JUNO_MODULE_ROOT", "JUNO_MODULE_ROOT(A, B)", JunoModuleRoot],
        ["JUNO_MODULE_DERIVE", "JUNO_MODULE_DERIVE(A, B)", JunoModuleDerive],
        ["JUNO_MODULE", "JUNO_MODULE(A, B)", JunoModule],
        ["JUNO_TRAIT_ROOT", "JUNO_TRAIT_ROOT(A, B)", JunoTraitRoot],
        ["JUNO_TRAIT_DERIVE", "JUNO_TRAIT_DERIVE(A, B)", JunoTraitDerive],
        ["JUNO_MODULE_DECLARE", "JUNO_MODULE_DECLARE(X)", JunoModuleDeclare],
        ["JUNO_MODULE_ROOT_DECLARE", "JUNO_MODULE_ROOT_DECLARE(X)", JunoModuleRootDeclare],
        ["JUNO_MODULE_DERIVE_DECLARE", "JUNO_MODULE_DERIVE_DECLARE(X)", JunoModuleDeriveDeclare],
        ["JUNO_MODULE_RESULT", "JUNO_MODULE_RESULT", JunoModuleResult],
        ["JUNO_MODULE_GET_API", "JUNO_MODULE_GET_API(X, Y)", JunoModuleGetApi],
        ["JUNO_MODULE_SUPER", "JUNO_MODULE_SUPER", JunoModuleSuper],
        ["JUNO_MODULE_EMPTY", "JUNO_MODULE_EMPTY", JunoModuleEmpty],
        ["JUNO_MODULE_ARG", "JUNO_MODULE_ARG(x)", JunoModuleArg],
    ];

    test.each(cases)("%s is recognized as its macro token", (_name, text, expectedType) => {
        const tokens = tokenize(text);
        expect(tokens[0].tokenType).toBe(expectedType);
        expect(tokens[0].image).toMatch(new RegExp(`^${_name}`));
    });

    test("JUNO_FAILURE_HANDLER macro form produces JunoFailureHandler", () => {
        const tokens = tokenize("JUNO_FAILURE_HANDLER");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(JunoFailureHandler);
        expect(tokens[0].image).toBe("JUNO_FAILURE_HANDLER");
    });

    test("_pfcnFailureHandler member-name form produces JunoFailureHandler", () => {
        const tokens = tokenize("_pfcnFailureHandler");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(JunoFailureHandler);
        expect(tokens[0].image).toBe("_pfcnFailureHandler");
    });

    test("JUNO_FAILURE_USER_DATA macro form produces JunoFailureUserData", () => {
        const tokens = tokenize("JUNO_FAILURE_USER_DATA");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(JunoFailureUserData);
        expect(tokens[0].image).toBe("JUNO_FAILURE_USER_DATA");
    });

    test("_pvFailureUserData member-name form produces JunoFailureUserData", () => {
        const tokens = tokenize("_pvFailureUserData");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(JunoFailureUserData);
        expect(tokens[0].image).toBe("_pvFailureUserData");
    });
});

// ---------------------------------------------------------------------------
// 2. Token Priority / Ordering
// ---------------------------------------------------------------------------

describe("Token priority and ordering", () => {
    test("JUNO_MODULE_ROOT wins over Identifier for the exact string", () => {
        const tokens = tokenize("JUNO_MODULE_ROOT");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(JunoModuleRoot);
    });

    test("JUNO_MODULE_ROOT_T is tokenized as a single Identifier (word boundary prevents macro match)", () => {
        const tokens = tokenize("JUNO_MODULE_ROOT_T");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Identifier);
        expect(tokens[0].image).toBe("JUNO_MODULE_ROOT_T");
    });

    test("JUNO_MODULE_ROOT_DECLARE is NOT consumed as JUNO_MODULE_ROOT + remaining chars", () => {
        const tokens = tokenize("JUNO_MODULE_ROOT_DECLARE(X)");
        expect(tokens[0].tokenType).toBe(JunoModuleRootDeclare);
        expect(tokens[0].image).toBe("JUNO_MODULE_ROOT_DECLARE");
    });

    test("struct keyword is recognized as Struct, not Identifier", () => {
        const tokens = tokenize("struct");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Struct);
    });

    test("static keyword is recognized as Static, not Identifier", () => {
        const tokens = tokenize("static");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Static);
    });

    test("const keyword is recognized as Const, not Identifier", () => {
        const tokens = tokenize("const");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Const);
    });

    test("structure is tokenized as Identifier (not Struct + ure) due to longer_alt", () => {
        const tokens = tokenize("structure");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Identifier);
        expect(tokens[0].image).toBe("structure");
    });

    test("constFoo is tokenized as Identifier (not Const + Identifier) due to longer_alt", () => {
        const tokens = tokenize("constFoo");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Identifier);
        expect(tokens[0].image).toBe("constFoo");
    });

    test("HashDirective consumes a full #include line as a single token", () => {
        const tokens = CLexer.tokenize("#include <stdio.h>").tokens;
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(HashDirective);
        expect(tokens[0].image).toContain("#include");
    });

    test("HashDirective consumes a full #define line as a single token", () => {
        const tokens = CLexer.tokenize("#define FOO 42").tokens;
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(HashDirective);
        expect(tokens[0].image).toContain("#define");
    });

    test("HashDirective consumes a full #ifdef line as a single token", () => {
        const tokens = CLexer.tokenize("#ifdef SOME_FLAG").tokens;
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(HashDirective);
        expect(tokens[0].image).toContain("#ifdef");
    });
});

// ---------------------------------------------------------------------------
// 3. Compound Operator Priority
// ---------------------------------------------------------------------------

describe("Compound operator priority", () => {
    test("-> is tokenized as ArrowOp, not Minus + Gt", () => {
        const tokens = tokenize("->");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(ArrowOp);
    });

    test("== is tokenized as EqEq, not Assign + Assign", () => {
        const tokens = tokenize("==");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(EqEq);
    });

    test("<<= is tokenized as LShiftAssign, not LShift + Assign", () => {
        const tokens = tokenize("<<=");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(LShiftAssign);
    });

    test("... is tokenized as Ellipsis, not three Dot tokens", () => {
        const tokens = tokenize("...");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Ellipsis);
    });

    test("a->b produces Identifier, ArrowOp, Identifier sequence", () => {
        const tokens = tokenize("a->b");
        expect(tokens).toHaveLength(3);
        expect(tokens[0].tokenType).toBe(Identifier);
        expect(tokens[1].tokenType).toBe(ArrowOp);
        expect(tokens[2].tokenType).toBe(Identifier);
    });

    test("a == b produces Identifier, EqEq, Identifier sequence", () => {
        const tokens = tokenize("a == b");
        expect(tokens).toHaveLength(3);
        expect(tokens[0].tokenType).toBe(Identifier);
        expect(tokens[1].tokenType).toBe(EqEq);
        expect(tokens[2].tokenType).toBe(Identifier);
    });
});

// ---------------------------------------------------------------------------
// 4. Literals
// ---------------------------------------------------------------------------

describe("Literal token recognition", () => {
    test("decimal integer literal 42", () => {
        const tokens = tokenize("42");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(IntegerLiteral);
        expect(tokens[0].image).toBe("42");
    });

    test("hex integer literal 0xFF", () => {
        const tokens = tokenize("0xFF");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(IntegerLiteral);
        expect(tokens[0].image).toBe("0xFF");
    });

    test("octal integer literal 077", () => {
        const tokens = tokenize("077");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(IntegerLiteral);
        expect(tokens[0].image).toBe("077");
    });

    test("floating literal 3.14", () => {
        const tokens = tokenize("3.14");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(FloatingLiteral);
        expect(tokens[0].image).toBe("3.14");
    });

    test("floating literal with exponent 1.0e5", () => {
        const tokens = tokenize("1.0e5");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(FloatingLiteral);
        expect(tokens[0].image).toBe("1.0e5");
    });

    test("string literal \"hello\"", () => {
        const tokens = tokenize('"hello"');
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(StringLiteral);
        expect(tokens[0].image).toBe('"hello"');
    });

    test('string literal with escaped quote "escaped\\"quote"', () => {
        const tokens = tokenize('"escaped\\"quote"');
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(StringLiteral);
    });

    test("char literal 'a'", () => {
        const tokens = tokenize("'a'");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(CharLiteral);
        expect(tokens[0].image).toBe("'a'");
    });

    test("char literal '\\n'", () => {
        const tokens = tokenize("'\\n'");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(CharLiteral);
        expect(tokens[0].image).toBe("'\\n'");
    });
});

// ---------------------------------------------------------------------------
// 5. Whitespace and Comment Skipping
// ---------------------------------------------------------------------------

describe("Whitespace and comment skipping", () => {
    test("whitespace between tokens is not in the token stream", () => {
        const tokens = tokenize("a   b");
        expect(tokens).toHaveLength(2);
        expect(tokens[0].tokenType).toBe(Identifier);
        expect(tokens[1].tokenType).toBe(Identifier);
    });

    test("tab and newline whitespace is not in the token stream", () => {
        const tokens = tokenize("a\t\nb");
        expect(tokens).toHaveLength(2);
        expect(tokens[0].tokenType).toBe(Identifier);
        expect(tokens[1].tokenType).toBe(Identifier);
    });

    test("line comment is skipped", () => {
        const tokens = tokenize("a // this is a comment\nb");
        expect(tokens).toHaveLength(2);
        expect(tokens[0].image).toBe("a");
        expect(tokens[1].image).toBe("b");
    });

    test("block comment is skipped", () => {
        const tokens = tokenize("a /* block comment */ b");
        expect(tokens).toHaveLength(2);
        expect(tokens[0].image).toBe("a");
        expect(tokens[1].image).toBe("b");
    });

    test("multi-line block comment is skipped entirely", () => {
        const tokens = tokenize("a\n/*\n * multi\n * line\n */\nb");
        expect(tokens).toHaveLength(2);
        expect(tokens[0].image).toBe("a");
        expect(tokens[1].image).toBe("b");
    });

    test("only meaningful tokens remain after stripping whitespace and comments", () => {
        const tokens = tokenize("  /* header */\nstruct  // inline\nFoo");
        expect(tokens).toHaveLength(2);
        expect(tokens[0].tokenType).toBe(Struct);
        expect(tokens[1].tokenType).toBe(Identifier);
    });
});

// ---------------------------------------------------------------------------
// 6. Full Snippet Tokenization
// ---------------------------------------------------------------------------

describe("Full snippet tokenization", () => {
    test("LibJuno struct definition with JUNO_MODULE_ROOT macro", () => {
        const text =
            "struct JUNO_DS_HEAP_ROOT_TAG JUNO_MODULE_ROOT(JUNO_DS_HEAP_API_T, JUNO_MODULE_EMPTY);";
        const tokens = tokenize(text);

        expect(tokens).toHaveLength(9);
        expect(tokens[0].tokenType).toBe(Struct);
        expect(tokens[1].tokenType).toBe(Identifier);
        expect(tokens[1].image).toBe("JUNO_DS_HEAP_ROOT_TAG");
        expect(tokens[2].tokenType).toBe(JunoModuleRoot);
        expect(tokens[3].tokenType).toBe(LParen);
        expect(tokens[4].tokenType).toBe(Identifier);
        expect(tokens[4].image).toBe("JUNO_DS_HEAP_API_T");
        expect(tokens[5].tokenType).toBe(Comma);
        expect(tokens[6].tokenType).toBe(JunoModuleEmpty);
        expect(tokens[7].tokenType).toBe(RParen);
        expect(tokens[8].tokenType).toBe(Semicolon);
    });

    test("vtable assignment snippet with static const declaration and dot-field initializer", () => {
        const text = [
            "static const JUNO_APP_API_T tEngineAppApi = {",
            "    .OnStart = OnStart,",
            "};",
        ].join("\n");

        const tokens = tokenize(text);

        // Collect token types for readability
        const types = tokens.map((t) => t.tokenType);

        expect(types[0]).toBe(Static);
        expect(types[1]).toBe(Const);
        expect(types[2]).toBe(Identifier);          // JUNO_APP_API_T
        expect(tokens[2].image).toBe("JUNO_APP_API_T");
        expect(types[3]).toBe(Identifier);          // tEngineAppApi
        expect(tokens[3].image).toBe("tEngineAppApi");
        expect(types[4]).toBe(Assign);
        expect(types[5]).toBe(LBrace);
        expect(types[6]).toBe(Dot);
        expect(types[7]).toBe(Identifier);          // OnStart (lhs)
        expect(tokens[7].image).toBe("OnStart");
        expect(types[8]).toBe(Assign);
        expect(types[9]).toBe(Identifier);          // OnStart (rhs)
        expect(tokens[9].image).toBe("OnStart");
        expect(types[10]).toBe(Comma);
        expect(types[11]).toBe(RBrace);
        expect(types[12]).toBe(Semicolon);
    });

    test("JUNO_MODULE_RESULT and JUNO_MODULE_GET_API in a function return expression", () => {
        const text = "JUNO_MODULE_RESULT JUNO_MODULE_GET_API(ptHeap, JUNO_DS_HEAP_API_T);";
        const tokens = tokenize(text);

        expect(tokens[0].tokenType).toBe(JunoModuleResult);
        expect(tokens[0].image).toBe("JUNO_MODULE_RESULT");
        expect(tokens[1].tokenType).toBe(JunoModuleGetApi);
        expect(tokens[1].image).toBe("JUNO_MODULE_GET_API");
        expect(tokens[2].tokenType).toBe(LParen);
        expect(tokens[3].tokenType).toBe(Identifier);
        expect(tokens[4].tokenType).toBe(Comma);
        expect(tokens[5].tokenType).toBe(Identifier);
        expect(tokens[6].tokenType).toBe(RParen);
        expect(tokens[7].tokenType).toBe(Semicolon);
    });

    test("failure handler assignment using macro form produces JunoFailureHandler token", () => {
        const text = "ptRoot->JUNO_FAILURE_HANDLER = MyHandler;";
        const tokens = tokenize(text);

        expect(tokens[0].tokenType).toBe(Identifier);           // ptRoot
        expect(tokens[1].tokenType).toBe(ArrowOp);
        expect(tokens[2].tokenType).toBe(JunoFailureHandler);
        expect(tokens[2].image).toBe("JUNO_FAILURE_HANDLER");
        expect(tokens[3].tokenType).toBe(Assign);
        expect(tokens[4].tokenType).toBe(Identifier);           // MyHandler
        expect(tokens[5].tokenType).toBe(Semicolon);
    });

    test("failure handler assignment using member-name form produces JunoFailureHandler token", () => {
        const text = "ptRoot->_pfcnFailureHandler = MyHandler;";
        const tokens = tokenize(text);

        expect(tokens[0].tokenType).toBe(Identifier);           // ptRoot
        expect(tokens[1].tokenType).toBe(ArrowOp);
        expect(tokens[2].tokenType).toBe(JunoFailureHandler);
        expect(tokens[2].image).toBe("_pfcnFailureHandler");
        expect(tokens[3].tokenType).toBe(Assign);
        expect(tokens[4].tokenType).toBe(Identifier);           // MyHandler
        expect(tokens[5].tokenType).toBe(Semicolon);
    });
});
