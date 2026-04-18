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
    // Keywords — previously imported
    Static,
    Const,
    Struct,
    // Keywords — additional
    Inline,
    Union,
    Enum,
    Typedef,
    Extern,
    Volatile,
    Void,
    Char,
    Short,
    Int,
    Long,
    Float,
    Double,
    Signed,
    Unsigned,
    SizeT,
    Bool,
    If,
    Else,
    For,
    While,
    Do,
    Switch,
    Case,
    Default,
    Break,
    Continue,
    Return,
    Goto,
    Sizeof,
    // Punctuators — compound (previously imported)
    ArrowOp,
    EqEq,
    LShiftAssign,
    Ellipsis,
    // Punctuators — compound (additional)
    PlusPlus,
    MinusMinus,
    PlusAssign,
    MinusAssign,
    StarAssign,
    SlashAssign,
    PercentAssign,
    AmpAssign,
    PipeAssign,
    CaretAssign,
    RShiftAssign,
    LShift,
    RShift,
    LtEq,
    GtEq,
    BangEq,
    AmpAmp,
    PipePipe,
    // Punctuators — simple (previously imported)
    LParen,
    RParen,
    LBrace,
    RBrace,
    Semicolon,
    Comma,
    Dot,
    Assign,
    // Punctuators — simple (additional)
    LBracket,
    RBracket,
    Star,
    Amp,
    Plus,
    Minus,
    Slash,
    Percent,
    Bang,
    Tilde,
    Lt,
    Gt,
    Colon,
    Question,
    Caret,
    Pipe,
    Hash,
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

// @{"verify": ["REQ-VSCODE-003", "REQ-VSCODE-008", "REQ-VSCODE-009", "REQ-VSCODE-014", "REQ-VSCODE-015"]}
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

// @{"verify": ["REQ-VSCODE-003"]}
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

// @{"verify": ["REQ-VSCODE-003"]}
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

// @{"verify": ["REQ-VSCODE-003"]}
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

// @{"verify": ["REQ-VSCODE-003"]}
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

// @{"verify": ["REQ-VSCODE-003", "REQ-VSCODE-008", "REQ-VSCODE-009"]}
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

// ---------------------------------------------------------------------------
// 7. Keyword Token Identification — one test per keyword
//
// Each test feeds exactly the keyword string to the lexer and verifies:
//   (a) exactly one token is produced (pattern match is correct)
//   (b) the token type reference is the expected exported token
//   (c) the token type name string is correct (kills StringLiteral mutants on name)
//   (d) the image is exactly the keyword string (kills Regex mutants on pattern)
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("Keyword token identification", () => {
    test("inline → Inline", () => {
        const tokens = tokenize("inline");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Inline);
        expect(tokens[0].tokenType.name).toBe("Inline");
        expect(tokens[0].image).toBe("inline");
    });

    test("union → Union", () => {
        const tokens = tokenize("union");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Union);
        expect(tokens[0].tokenType.name).toBe("Union");
        expect(tokens[0].image).toBe("union");
    });

    test("enum → Enum", () => {
        const tokens = tokenize("enum");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Enum);
        expect(tokens[0].tokenType.name).toBe("Enum");
        expect(tokens[0].image).toBe("enum");
    });

    test("typedef → Typedef", () => {
        const tokens = tokenize("typedef");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Typedef);
        expect(tokens[0].tokenType.name).toBe("Typedef");
        expect(tokens[0].image).toBe("typedef");
    });

    test("extern → Extern", () => {
        const tokens = tokenize("extern");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Extern);
        expect(tokens[0].tokenType.name).toBe("Extern");
        expect(tokens[0].image).toBe("extern");
    });

    test("volatile → Volatile", () => {
        const tokens = tokenize("volatile");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Volatile);
        expect(tokens[0].tokenType.name).toBe("Volatile");
        expect(tokens[0].image).toBe("volatile");
    });

    test("void → Void", () => {
        const tokens = tokenize("void");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Void);
        expect(tokens[0].tokenType.name).toBe("Void");
        expect(tokens[0].image).toBe("void");
    });

    test("char → Char", () => {
        const tokens = tokenize("char");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Char);
        expect(tokens[0].tokenType.name).toBe("Char");
        expect(tokens[0].image).toBe("char");
    });

    test("short → Short", () => {
        const tokens = tokenize("short");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Short);
        expect(tokens[0].tokenType.name).toBe("Short");
        expect(tokens[0].image).toBe("short");
    });

    test("int → Int", () => {
        const tokens = tokenize("int");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Int);
        expect(tokens[0].tokenType.name).toBe("Int");
        expect(tokens[0].image).toBe("int");
    });

    test("long → Long", () => {
        const tokens = tokenize("long");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Long);
        expect(tokens[0].tokenType.name).toBe("Long");
        expect(tokens[0].image).toBe("long");
    });

    test("float → Float", () => {
        const tokens = tokenize("float");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Float);
        expect(tokens[0].tokenType.name).toBe("Float");
        expect(tokens[0].image).toBe("float");
    });

    test("double → Double", () => {
        const tokens = tokenize("double");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Double);
        expect(tokens[0].tokenType.name).toBe("Double");
        expect(tokens[0].image).toBe("double");
    });

    test("signed → Signed", () => {
        const tokens = tokenize("signed");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Signed);
        expect(tokens[0].tokenType.name).toBe("Signed");
        expect(tokens[0].image).toBe("signed");
    });

    test("unsigned → Unsigned", () => {
        const tokens = tokenize("unsigned");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Unsigned);
        expect(tokens[0].tokenType.name).toBe("Unsigned");
        expect(tokens[0].image).toBe("unsigned");
    });

    test("size_t → SizeT", () => {
        const tokens = tokenize("size_t");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(SizeT);
        expect(tokens[0].tokenType.name).toBe("SizeT");
        expect(tokens[0].image).toBe("size_t");
    });

    test("_Bool → Bool", () => {
        const tokens = tokenize("_Bool");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Bool);
        expect(tokens[0].tokenType.name).toBe("Bool");
        expect(tokens[0].image).toBe("_Bool");
    });

    test("if → If", () => {
        const tokens = tokenize("if");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(If);
        expect(tokens[0].tokenType.name).toBe("If");
        expect(tokens[0].image).toBe("if");
    });

    test("else → Else", () => {
        const tokens = tokenize("else");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Else);
        expect(tokens[0].tokenType.name).toBe("Else");
        expect(tokens[0].image).toBe("else");
    });

    test("for → For", () => {
        const tokens = tokenize("for");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(For);
        expect(tokens[0].tokenType.name).toBe("For");
        expect(tokens[0].image).toBe("for");
    });

    test("while → While", () => {
        const tokens = tokenize("while");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(While);
        expect(tokens[0].tokenType.name).toBe("While");
        expect(tokens[0].image).toBe("while");
    });

    test("do → Do", () => {
        const tokens = tokenize("do");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Do);
        expect(tokens[0].tokenType.name).toBe("Do");
        expect(tokens[0].image).toBe("do");
    });

    test("switch → Switch", () => {
        const tokens = tokenize("switch");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Switch);
        expect(tokens[0].tokenType.name).toBe("Switch");
        expect(tokens[0].image).toBe("switch");
    });

    test("case → Case", () => {
        const tokens = tokenize("case");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Case);
        expect(tokens[0].tokenType.name).toBe("Case");
        expect(tokens[0].image).toBe("case");
    });

    test("default → Default", () => {
        const tokens = tokenize("default");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Default);
        expect(tokens[0].tokenType.name).toBe("Default");
        expect(tokens[0].image).toBe("default");
    });

    test("break → Break", () => {
        const tokens = tokenize("break");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Break);
        expect(tokens[0].tokenType.name).toBe("Break");
        expect(tokens[0].image).toBe("break");
    });

    test("continue → Continue", () => {
        const tokens = tokenize("continue");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Continue);
        expect(tokens[0].tokenType.name).toBe("Continue");
        expect(tokens[0].image).toBe("continue");
    });

    test("return → Return", () => {
        const tokens = tokenize("return");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Return);
        expect(tokens[0].tokenType.name).toBe("Return");
        expect(tokens[0].image).toBe("return");
    });

    test("goto → Goto", () => {
        const tokens = tokenize("goto");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Goto);
        expect(tokens[0].tokenType.name).toBe("Goto");
        expect(tokens[0].image).toBe("goto");
    });

    test("sizeof → Sizeof", () => {
        const tokens = tokenize("sizeof");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Sizeof);
        expect(tokens[0].tokenType.name).toBe("Sizeof");
        expect(tokens[0].image).toBe("sizeof");
    });

    // Also verify the three keywords tested in section 2 assert name and image too
    test("static → Static (name and image verified)", () => {
        const tokens = tokenize("static");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Static);
        expect(tokens[0].tokenType.name).toBe("Static");
        expect(tokens[0].image).toBe("static");
    });

    test("const → Const (name and image verified)", () => {
        const tokens = tokenize("const");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Const);
        expect(tokens[0].tokenType.name).toBe("Const");
        expect(tokens[0].image).toBe("const");
    });

    test("struct → Struct (name and image verified)", () => {
        const tokens = tokenize("struct");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Struct);
        expect(tokens[0].tokenType.name).toBe("Struct");
        expect(tokens[0].image).toBe("struct");
    });
});

// ---------------------------------------------------------------------------
// 8. Keyword longer_alt — identifiers that start with a keyword are NOT split
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("Keyword longer_alt — identifiers containing keywords are not split", () => {
    test("inlined is Identifier (not Inline + d)", () => {
        const tokens = tokenize("inlined");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Identifier);
        expect(tokens[0].image).toBe("inlined");
    });

    test("unions is Identifier (not Union + s)", () => {
        const tokens = tokenize("unions");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Identifier);
        expect(tokens[0].image).toBe("unions");
    });

    test("enumValue is Identifier (not Enum + Value)", () => {
        const tokens = tokenize("enumValue");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Identifier);
        expect(tokens[0].image).toBe("enumValue");
    });

    test("voidPtr is Identifier (not Void + Ptr)", () => {
        const tokens = tokenize("voidPtr");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Identifier);
        expect(tokens[0].image).toBe("voidPtr");
    });

    test("returned is Identifier (not Return + ed)", () => {
        const tokens = tokenize("returned");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Identifier);
        expect(tokens[0].image).toBe("returned");
    });

    test("sizeof_x is Identifier (not Sizeof + _x)", () => {
        const tokens = tokenize("sizeof_x");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Identifier);
        expect(tokens[0].image).toBe("sizeof_x");
    });

    test("integer is Identifier (not Int + eger)", () => {
        const tokens = tokenize("integer");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Identifier);
        expect(tokens[0].image).toBe("integer");
    });

    test("charter is Identifier (not Char + ter)", () => {
        const tokens = tokenize("charter");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Identifier);
        expect(tokens[0].image).toBe("charter");
    });
});

// ---------------------------------------------------------------------------
// 9. Compound operator token identification
//
// Each test verifies a compound operator that has no standalone test yet.
// Asserts: length 1, correct tokenType reference, correct name, correct image.
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("Compound operator token identification", () => {
    test("++ → PlusPlus", () => {
        const tokens = tokenize("++");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(PlusPlus);
        expect(tokens[0].tokenType.name).toBe("PlusPlus");
        expect(tokens[0].image).toBe("++");
    });

    test("-- → MinusMinus", () => {
        const tokens = tokenize("--");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(MinusMinus);
        expect(tokens[0].tokenType.name).toBe("MinusMinus");
        expect(tokens[0].image).toBe("--");
    });

    test("+= → PlusAssign", () => {
        const tokens = tokenize("+=");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(PlusAssign);
        expect(tokens[0].tokenType.name).toBe("PlusAssign");
        expect(tokens[0].image).toBe("+=");
    });

    test("-= → MinusAssign", () => {
        const tokens = tokenize("-=");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(MinusAssign);
        expect(tokens[0].tokenType.name).toBe("MinusAssign");
        expect(tokens[0].image).toBe("-=");
    });

    test("*= → StarAssign", () => {
        const tokens = tokenize("*=");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(StarAssign);
        expect(tokens[0].tokenType.name).toBe("StarAssign");
        expect(tokens[0].image).toBe("*=");
    });

    test("/= → SlashAssign", () => {
        const tokens = tokenize("/=");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(SlashAssign);
        expect(tokens[0].tokenType.name).toBe("SlashAssign");
        expect(tokens[0].image).toBe("/=");
    });

    test("%= → PercentAssign", () => {
        const tokens = tokenize("%=");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(PercentAssign);
        expect(tokens[0].tokenType.name).toBe("PercentAssign");
        expect(tokens[0].image).toBe("%=");
    });

    test("&= → AmpAssign", () => {
        const tokens = tokenize("&=");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(AmpAssign);
        expect(tokens[0].tokenType.name).toBe("AmpAssign");
        expect(tokens[0].image).toBe("&=");
    });

    test("|= → PipeAssign", () => {
        const tokens = tokenize("|=");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(PipeAssign);
        expect(tokens[0].tokenType.name).toBe("PipeAssign");
        expect(tokens[0].image).toBe("|=");
    });

    test("^= → CaretAssign", () => {
        const tokens = tokenize("^=");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(CaretAssign);
        expect(tokens[0].tokenType.name).toBe("CaretAssign");
        expect(tokens[0].image).toBe("^=");
    });

    test(">>= → RShiftAssign", () => {
        const tokens = tokenize(">>=");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(RShiftAssign);
        expect(tokens[0].tokenType.name).toBe("RShiftAssign");
        expect(tokens[0].image).toBe(">>=");
    });

    test("<< → LShift", () => {
        const tokens = tokenize("<<");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(LShift);
        expect(tokens[0].tokenType.name).toBe("LShift");
        expect(tokens[0].image).toBe("<<");
    });

    test(">> → RShift", () => {
        const tokens = tokenize(">>");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(RShift);
        expect(tokens[0].tokenType.name).toBe("RShift");
        expect(tokens[0].image).toBe(">>");
    });

    test("<= → LtEq", () => {
        const tokens = tokenize("<=");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(LtEq);
        expect(tokens[0].tokenType.name).toBe("LtEq");
        expect(tokens[0].image).toBe("<=");
    });

    test(">= → GtEq", () => {
        const tokens = tokenize(">=");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(GtEq);
        expect(tokens[0].tokenType.name).toBe("GtEq");
        expect(tokens[0].image).toBe(">=");
    });

    test("!= → BangEq", () => {
        const tokens = tokenize("!=");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(BangEq);
        expect(tokens[0].tokenType.name).toBe("BangEq");
        expect(tokens[0].image).toBe("!=");
    });

    test("&& → AmpAmp", () => {
        const tokens = tokenize("&&");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(AmpAmp);
        expect(tokens[0].tokenType.name).toBe("AmpAmp");
        expect(tokens[0].image).toBe("&&");
    });

    test("|| → PipePipe", () => {
        const tokens = tokenize("||");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(PipePipe);
        expect(tokens[0].tokenType.name).toBe("PipePipe");
        expect(tokens[0].image).toBe("||");
    });
});

// ---------------------------------------------------------------------------
// 10. Simple punctuator token identification
//
// Each test feeds a single punctuator character and asserts the exact token.
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("Simple punctuator token identification", () => {
    test("[ → LBracket", () => {
        const tokens = tokenize("[");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(LBracket);
        expect(tokens[0].tokenType.name).toBe("LBracket");
        expect(tokens[0].image).toBe("[");
    });

    test("] → RBracket", () => {
        const tokens = tokenize("]");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(RBracket);
        expect(tokens[0].tokenType.name).toBe("RBracket");
        expect(tokens[0].image).toBe("]");
    });

    test("* → Star", () => {
        const tokens = tokenize("*");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Star);
        expect(tokens[0].tokenType.name).toBe("Star");
        expect(tokens[0].image).toBe("*");
    });

    test("& → Amp", () => {
        const tokens = tokenize("&");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Amp);
        expect(tokens[0].tokenType.name).toBe("Amp");
        expect(tokens[0].image).toBe("&");
    });

    test("+ → Plus", () => {
        const tokens = tokenize("+");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Plus);
        expect(tokens[0].tokenType.name).toBe("Plus");
        expect(tokens[0].image).toBe("+");
    });

    test("- → Minus", () => {
        const tokens = tokenize("-");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Minus);
        expect(tokens[0].tokenType.name).toBe("Minus");
        expect(tokens[0].image).toBe("-");
    });

    test("/ → Slash", () => {
        const tokens = tokenize("/");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Slash);
        expect(tokens[0].tokenType.name).toBe("Slash");
        expect(tokens[0].image).toBe("/");
    });

    test("% → Percent", () => {
        const tokens = tokenize("%");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Percent);
        expect(tokens[0].tokenType.name).toBe("Percent");
        expect(tokens[0].image).toBe("%");
    });

    test("! → Bang", () => {
        const tokens = tokenize("!");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Bang);
        expect(tokens[0].tokenType.name).toBe("Bang");
        expect(tokens[0].image).toBe("!");
    });

    test("~ → Tilde", () => {
        const tokens = tokenize("~");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Tilde);
        expect(tokens[0].tokenType.name).toBe("Tilde");
        expect(tokens[0].image).toBe("~");
    });

    test("< → Lt", () => {
        const tokens = tokenize("<");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Lt);
        expect(tokens[0].tokenType.name).toBe("Lt");
        expect(tokens[0].image).toBe("<");
    });

    test("> → Gt", () => {
        const tokens = tokenize(">");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Gt);
        expect(tokens[0].tokenType.name).toBe("Gt");
        expect(tokens[0].image).toBe(">");
    });

    test(": → Colon", () => {
        const tokens = tokenize(":");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Colon);
        expect(tokens[0].tokenType.name).toBe("Colon");
        expect(tokens[0].image).toBe(":");
    });

    test("? → Question", () => {
        const tokens = tokenize("?");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Question);
        expect(tokens[0].tokenType.name).toBe("Question");
        expect(tokens[0].image).toBe("?");
    });

    test("^ → Caret", () => {
        const tokens = tokenize("^");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Caret);
        expect(tokens[0].tokenType.name).toBe("Caret");
        expect(tokens[0].image).toBe("^");
    });

    test("| → Pipe", () => {
        const tokens = tokenize("|");
        expect(tokens).toHaveLength(1);
        expect(tokens[0].tokenType).toBe(Pipe);
        expect(tokens[0].tokenType.name).toBe("Pipe");
        expect(tokens[0].image).toBe("|");
    });

    test("# → Hash (bare, not a full directive)", () => {
        // A bare # that is not at start-of-line (mid-expression) should be Hash, not HashDirective
        const result = CLexer.tokenize("a #");
        const tokens = result.tokens;
        expect(tokens[1].tokenType).toBe(Hash);
        expect(tokens[1].tokenType.name).toBe("Hash");
        expect(tokens[1].image).toBe("#");
    });
});

// ---------------------------------------------------------------------------
// 11. HashDirective conditional-logic boundary cases
//
// The HashDirective custom match function uses:
//   if (startOffset !== 0 && text[startOffset - 1] !== "\n") { return null; }
//   const match = _hashDirectiveRe.exec(text.slice(startOffset));
//   if (!match || match.index !== 0) { return null; }
//
// These tests probe the boundary conditions to kill ConditionalExpression,
// BlockStatement, EqualityOperator, ArithmeticOperator, and StringLiteral
// mutants on this function.
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-003"]}
describe("HashDirective conditional logic boundary cases", () => {
    test("directive at startOffset 0 is recognized as HashDirective", () => {
        // startOffset === 0 → condition (startOffset !== 0 && ...) is false → proceed
        const result = CLexer.tokenize("#define FOO 1");
        expect(result.errors).toHaveLength(0);
        expect(result.tokens).toHaveLength(1);
        expect(result.tokens[0].tokenType).toBe(HashDirective);
        expect(result.tokens[0].image).toBe("#define FOO 1");
    });

    test("directive on second line (prev char is newline) is recognized as HashDirective", () => {
        // startOffset > 0, text[startOffset - 1] === '\n' → second condition is false → proceed
        const result = CLexer.tokenize("a\n#define X 1");
        expect(result.errors).toHaveLength(0);
        const types = result.tokens.map((t) => t.tokenType);
        expect(types[0]).toBe(Identifier);           // 'a'
        expect(types[1]).toBe(HashDirective);         // '#define X 1'
        expect(result.tokens[1].image).toBe("#define X 1");
    });

    test("hash mid-line (prev char is not newline) produces bare Hash, not HashDirective", () => {
        // startOffset > 0, text[startOffset - 1] === ' ' (not '\n') → return null from guard
        // Remaining '#define ...' becomes Hash + Identifier + ...
        const result = CLexer.tokenize("x #define Y 2");
        expect(result.errors).toHaveLength(0);
        const types = result.tokens.map((t) => t.tokenType);
        expect(types[0]).toBe(Identifier);   // 'x'
        expect(types[1]).toBe(Hash);          // '#' — NOT HashDirective
    });

    test("directive preceded immediately by non-newline produces Hash, not HashDirective", () => {
        // No space between 'a' and '#' — prev char is 'a', not '\n'
        const result = CLexer.tokenize("a#ifndef GUARD");
        expect(result.errors).toHaveLength(0);
        const types = result.tokens.map((t) => t.tokenType);
        expect(types[0]).toBe(Identifier);  // 'a'
        expect(types[1]).toBe(Hash);         // '#' bare
    });

    test("HashDirective recognizes #ifndef", () => {
        const result = CLexer.tokenize("#ifndef GUARD_H");
        expect(result.errors).toHaveLength(0);
        expect(result.tokens).toHaveLength(1);
        expect(result.tokens[0].tokenType).toBe(HashDirective);
        expect(result.tokens[0].image).toContain("#ifndef");
    });

    test("HashDirective recognizes #endif", () => {
        const result = CLexer.tokenize("#endif");
        expect(result.errors).toHaveLength(0);
        expect(result.tokens).toHaveLength(1);
        expect(result.tokens[0].tokenType).toBe(HashDirective);
        expect(result.tokens[0].image).toBe("#endif");
    });

    test("HashDirective recognizes #pragma once", () => {
        const result = CLexer.tokenize("#pragma once");
        expect(result.errors).toHaveLength(0);
        expect(result.tokens).toHaveLength(1);
        expect(result.tokens[0].tokenType).toBe(HashDirective);
        expect(result.tokens[0].image).toContain("#pragma");
    });

    test("HashDirective recognizes #undef", () => {
        const result = CLexer.tokenize("#undef MY_MACRO");
        expect(result.errors).toHaveLength(0);
        expect(result.tokens).toHaveLength(1);
        expect(result.tokens[0].tokenType).toBe(HashDirective);
        expect(result.tokens[0].image).toContain("#undef");
    });
});
