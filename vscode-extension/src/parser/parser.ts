// @{"req": ["REQ-VSCODE-003", "REQ-VSCODE-010", "REQ-VSCODE-011", "REQ-VSCODE-012"]}
import { CstParser, tokenMatcher, EOF } from "chevrotain";
import {
    allTokens,
    HashDirective,
    Static, Typedef, Extern,
    Const, Volatile, Inline,
    Struct, Union, Enum,
    Void, Char, Short, Int, Long, Float, Double, Signed, Unsigned, SizeT, Bool,
    If, Else, While, Do, For, Switch, Case, Default, Break, Continue, Return, Goto,
    Sizeof,
    Identifier,
    IntegerLiteral, FloatingLiteral, StringLiteral, CharLiteral,
    LBrace, RBrace, LParen, RParen, LBracket, RBracket,
    Semicolon, Comma, Colon, Assign, Star, Ellipsis,
    ArrowOp, Dot,
    PlusPlus, MinusMinus,
    Plus, Minus, Slash, Percent,
    Amp, Pipe, Caret, Bang, Tilde,
    Lt, Gt, LtEq, GtEq,
    LShift, RShift,
    EqEq, BangEq,
    AmpAmp, PipePipe,
    PlusAssign, MinusAssign, StarAssign, SlashAssign, PercentAssign,
    AmpAssign, PipeAssign, CaretAssign, LShiftAssign, RShiftAssign,
    Question,
    JunoModuleRoot, JunoModuleDerive, JunoModule,
    JunoTraitRoot, JunoTraitDerive,
    JunoModuleDeclare, JunoModuleRootDeclare, JunoModuleDeriveDeclare,
    JunoModuleResult, JunoModuleOption,
    JunoModuleGetApi, JunoModuleSuper, JunoFailureHandler, JunoFailureUserData,
} from "./lexer";

export class CParser extends CstParser {
    constructor() {
        super(allTokens, { recoveryEnabled: true, nodeLocationTracking: "full", skipValidations: true });
        this.performSelfAnalysis();
    }

    // -------------------------------------------------------------------------
    // Top-level
    // -------------------------------------------------------------------------

    translationUnit = this.RULE("translationUnit", () => {
        this.MANY(() => {
            this.OR([
                { ALT: () => this.SUBRULE(this.preprocessorDirective) },
                { ALT: () => this.SUBRULE(this.externCBlock) },
                { ALT: () => this.SUBRULE(this.externalDeclaration) },
            ]);
        });
    });

    externCBlock = this.RULE("externCBlock", () => {
        this.CONSUME(Extern);
        this.CONSUME(StringLiteral);
        this.CONSUME(LBrace);
        this.MANY(() => {
            this.OR2([
                { ALT: () => this.SUBRULE(this.preprocessorDirective) },
                { ALT: () => this.SUBRULE(this.externalDeclaration) },
            ]);
        });
        this.CONSUME(RBrace);
    });

    externalDeclaration = this.RULE("externalDeclaration", () => {
        this.OR([
            { GATE: this.BACKTRACK(this.functionDefinition), ALT: () => this.SUBRULE(this.functionDefinition) },
            { ALT: () => this.SUBRULE(this.junoStandaloneDeclaration) },
            { ALT: () => this.SUBRULE(this.declaration) },
        ]);
    });

    preprocessorDirective = this.RULE("preprocessorDirective", () => {
        this.CONSUME(HashDirective);
    });

    // -------------------------------------------------------------------------
    // Declarations
    // -------------------------------------------------------------------------

    declaration = this.RULE("declaration", () => {
        this.SUBRULE(this.declarationSpecifiers);
        this.OPTION(() => this.SUBRULE(this.initDeclaratorList));
        this.CONSUME(Semicolon);
    });

    declarationSpecifiers = this.RULE("declarationSpecifiers", () => {
        // Require at least one specifier/qualifier/type
        this.OR([
            { ALT: () => this.SUBRULE(this.storageClassSpecifier) },
            { ALT: () => this.SUBRULE(this.typeQualifier) },
            { ALT: () => this.SUBRULE(this.typeSpecifier) },
        ]);
        // Continue consuming specifiers. GATE replicates AT_LEAST_ONE's FIRST-set check
        // for non-Identifier tokens, AND prevents consuming a declarator Identifier as
        // a typedef-name type specifier.
        this.MANY({
            GATE: () => {
                const la1 = this.LA(1);
                if (!tokenMatcher(la1, Identifier)) {
                    // Only enter loop for tokens that can actually start a specifier.
                    return (
                        tokenMatcher(la1, Static) || tokenMatcher(la1, Typedef) || tokenMatcher(la1, Extern) ||
                        tokenMatcher(la1, Const)  || tokenMatcher(la1, Volatile) || tokenMatcher(la1, Inline) ||
                        tokenMatcher(la1, Void)   || tokenMatcher(la1, Char)   || tokenMatcher(la1, Short) ||
                        tokenMatcher(la1, Int)    || tokenMatcher(la1, Long)   || tokenMatcher(la1, Float) ||
                        tokenMatcher(la1, Double) || tokenMatcher(la1, Signed) || tokenMatcher(la1, Unsigned) ||
                        tokenMatcher(la1, SizeT)  || tokenMatcher(la1, Bool)   ||
                        tokenMatcher(la1, Struct) || tokenMatcher(la1, Union)  || tokenMatcher(la1, Enum)
                    );
                }
                // Identifier: only a type specifier if followed by another Identifier, *,
                // a qualifier, or a function-pointer open-paren+star.
                const la2 = this.LA(2);
                return (
                    tokenMatcher(la2, Identifier) ||
                    tokenMatcher(la2, Star)       ||
                    tokenMatcher(la2, Const)      ||
                    tokenMatcher(la2, Volatile)   ||
                    (tokenMatcher(la2, LParen) && tokenMatcher(this.LA(3), Star))
                );
            },
            DEF: () => {
                this.OR2([
                    { ALT: () => this.SUBRULE2(this.storageClassSpecifier) },
                    { ALT: () => this.SUBRULE2(this.typeQualifier) },
                    { ALT: () => this.SUBRULE2(this.typeSpecifier) },
                ]);
            },
        });
    });

    storageClassSpecifier = this.RULE("storageClassSpecifier", () => {
        this.OR([
            { ALT: () => this.CONSUME(Static) },
            { ALT: () => this.CONSUME(Typedef) },
            { ALT: () => this.CONSUME(Extern) },
        ]);
    });

    typeQualifier = this.RULE("typeQualifier", () => {
        this.OR([
            { ALT: () => this.CONSUME(Const) },
            { ALT: () => this.CONSUME(Volatile) },
            { ALT: () => this.CONSUME(Inline) },
        ]);
    });

    typeSpecifier = this.RULE("typeSpecifier", () => {
        this.OR([
            { ALT: () => this.SUBRULE(this.primitiveType) },
            { ALT: () => this.SUBRULE(this.structOrUnionSpecifier) },
            { ALT: () => this.SUBRULE(this.enumSpecifier) },
            { ALT: () => this.CONSUME(Identifier) },
        ]);
    });

    primitiveType = this.RULE("primitiveType", () => {
        this.OR([
            { ALT: () => this.CONSUME(Void) },
            { ALT: () => this.CONSUME(Char) },
            { ALT: () => this.CONSUME(Short) },
            { ALT: () => this.CONSUME(Int) },
            { ALT: () => this.CONSUME(Long) },
            { ALT: () => this.CONSUME(Float) },
            { ALT: () => this.CONSUME(Double) },
            { ALT: () => this.CONSUME(Signed) },
            { ALT: () => this.CONSUME(Unsigned) },
            { ALT: () => this.CONSUME(SizeT) },
            { ALT: () => this.CONSUME(Bool) },
        ]);
    });

    // -------------------------------------------------------------------------
    // Struct / Union
    // -------------------------------------------------------------------------

    structOrUnionSpecifier = this.RULE("structOrUnionSpecifier", () => {
        this.OR([
            { ALT: () => this.CONSUME(Struct) },
            { ALT: () => this.CONSUME(Union) },
        ]);
        // Discriminate: has-body (Identifier? '{') vs. forward-ref/macro (Identifier macroInvocation?)
        this.OR2([
            {
                GATE: () =>
                    tokenMatcher(this.LA(1), LBrace) ||
                    (tokenMatcher(this.LA(1), Identifier) && tokenMatcher(this.LA(2), LBrace)),
                ALT: () => {
                    this.OPTION(() => this.CONSUME(Identifier));
                    this.CONSUME(LBrace);
                    this.SUBRULE(this.structDeclarationList);
                    this.CONSUME(RBrace);
                },
            },
            {
                ALT: () => {
                    this.CONSUME2(Identifier);
                    this.OPTION2(() => this.SUBRULE(this.junoMacroInvocation));
                },
            },
        ]);
    });

    structDeclarationList = this.RULE("structDeclarationList", () => {
        this.AT_LEAST_ONE(() => this.SUBRULE(this.structDeclaration));
    });

    structDeclaration = this.RULE("structDeclaration", () => {
        // junoMacroInvocation starts with distinct Juno tokens; no GATE needed.
        this.OR([
            {
                ALT: () => {
                    this.SUBRULE(this.junoMacroInvocation);
                    this.OPTION(() => this.CONSUME(Semicolon));
                },
            },
            {
                ALT: () => {
                    this.SUBRULE(this.specifierQualifierList);
                    this.SUBRULE(this.structDeclaratorList);
                    this.CONSUME2(Semicolon);
                },
            },
        ]);
    });

    specifierQualifierList = this.RULE("specifierQualifierList", () => {
        // Require at least one specifier or qualifier
        this.OR([
            { ALT: () => this.SUBRULE(this.typeSpecifier) },
            { ALT: () => this.SUBRULE(this.typeQualifier) },
        ]);
        // Continue consuming. Same GATE logic as declarationSpecifiers (no storage-class here).
        this.MANY({
            GATE: () => {
                const la1 = this.LA(1);
                if (!tokenMatcher(la1, Identifier)) {
                    return (
                        tokenMatcher(la1, Const)   || tokenMatcher(la1, Volatile) || tokenMatcher(la1, Inline) ||
                        tokenMatcher(la1, Void)    || tokenMatcher(la1, Char)     || tokenMatcher(la1, Short) ||
                        tokenMatcher(la1, Int)     || tokenMatcher(la1, Long)     || tokenMatcher(la1, Float) ||
                        tokenMatcher(la1, Double)  || tokenMatcher(la1, Signed)   || tokenMatcher(la1, Unsigned) ||
                        tokenMatcher(la1, SizeT)   || tokenMatcher(la1, Bool)     ||
                        tokenMatcher(la1, Struct)  || tokenMatcher(la1, Union)    || tokenMatcher(la1, Enum)
                    );
                }
                const la2 = this.LA(2);
                return (
                    tokenMatcher(la2, Identifier) ||
                    tokenMatcher(la2, Star)       ||
                    tokenMatcher(la2, Const)      ||
                    tokenMatcher(la2, Volatile)   ||
                    (tokenMatcher(la2, LParen) && tokenMatcher(this.LA(3), Star))
                );
            },
            DEF: () => {
                this.OR2([
                    { ALT: () => this.SUBRULE2(this.typeSpecifier) },
                    { ALT: () => this.SUBRULE2(this.typeQualifier) },
                ]);
            },
        });
    });

    structDeclaratorList = this.RULE("structDeclaratorList", () => {
        this.SUBRULE(this.structDeclarator);
        this.MANY(() => {
            this.CONSUME(Comma);
            this.SUBRULE2(this.structDeclarator);
        });
    });

    structDeclarator = this.RULE("structDeclarator", () => {
        this.SUBRULE(this.declarator);
        this.OPTION(() => {
            this.CONSUME(Colon);
            this.SUBRULE(this.constantExpression);
        });
    });

    // -------------------------------------------------------------------------
    // Enum
    // -------------------------------------------------------------------------

    enumSpecifier = this.RULE("enumSpecifier", () => {
        this.CONSUME(Enum);
        this.OR([
            {
                GATE: () =>
                    tokenMatcher(this.LA(1), LBrace) ||
                    (tokenMatcher(this.LA(1), Identifier) && tokenMatcher(this.LA(2), LBrace)),
                ALT: () => {
                    this.OPTION(() => this.CONSUME(Identifier));
                    this.CONSUME(LBrace);
                    this.SUBRULE(this.enumeratorList);
                    this.OPTION2(() => this.CONSUME(Comma)); // trailing comma
                    this.CONSUME(RBrace);
                },
            },
            {
                ALT: () => this.CONSUME2(Identifier),
            },
        ]);
    });

    enumeratorList = this.RULE("enumeratorList", () => {
        this.SUBRULE(this.enumerator);
        this.MANY({
            // Stop before trailing comma followed by '}'
            GATE: () => tokenMatcher(this.LA(1), Comma) && !tokenMatcher(this.LA(2), RBrace),
            DEF: () => {
                this.CONSUME(Comma);
                this.SUBRULE2(this.enumerator);
            },
        });
    });

    enumerator = this.RULE("enumerator", () => {
        this.CONSUME(Identifier);
        this.OPTION(() => {
            this.CONSUME(Assign);
            this.SUBRULE(this.constantExpression);
        });
    });

    // -------------------------------------------------------------------------
    // LibJuno Macro Productions
    // -------------------------------------------------------------------------

    junoMacroInvocation = this.RULE("junoMacroInvocation", () => {
        this.OR([
            { ALT: () => this.SUBRULE(this.junoModuleRootMacro) },
            { ALT: () => this.SUBRULE(this.junoModuleDeriveMacro) },
            { ALT: () => this.SUBRULE(this.junoTraitRootMacro) },
            { ALT: () => this.SUBRULE(this.junoTraitDeriveMacro) },
            { ALT: () => this.SUBRULE(this.junoModuleMacro) },
        ]);
    });

    junoModuleRootMacro = this.RULE("junoModuleRootMacro", () => {
        this.CONSUME(JunoModuleRoot);
        this.CONSUME(LParen);
        this.OR([
            { ALT: () => this.CONSUME(Identifier) },
            { ALT: () => this.CONSUME(Void) },
        ]);
        this.CONSUME(Comma);
        this.SUBRULE(this.macroBodyTokens);
        this.CONSUME(RParen);
    });

    junoModuleDeriveMacro = this.RULE("junoModuleDeriveMacro", () => {
        this.CONSUME(JunoModuleDerive);
        this.CONSUME(LParen);
        this.CONSUME(Identifier);
        this.CONSUME(Comma);
        this.SUBRULE(this.macroBodyTokens);
        this.CONSUME(RParen);
    });

    junoTraitRootMacro = this.RULE("junoTraitRootMacro", () => {
        this.CONSUME(JunoTraitRoot);
        this.CONSUME(LParen);
        this.CONSUME(Identifier);
        this.CONSUME(Comma);
        this.SUBRULE(this.macroBodyTokens);
        this.CONSUME(RParen);
    });

    junoTraitDeriveMacro = this.RULE("junoTraitDeriveMacro", () => {
        this.CONSUME(JunoTraitDerive);
        this.CONSUME(LParen);
        this.CONSUME(Identifier);
        this.CONSUME(Comma);
        this.SUBRULE(this.macroBodyTokens);
        this.CONSUME(RParen);
    });

    junoModuleMacro = this.RULE("junoModuleMacro", () => {
        this.CONSUME(JunoModule);
        this.CONSUME(LParen);
        this.CONSUME(Identifier);
        this.CONSUME(Comma);
        this.CONSUME2(Identifier);
        this.CONSUME2(Comma);
        this.SUBRULE(this.macroBodyTokens);
        this.CONSUME(RParen);
    });

    macroBodyTokens = this.RULE("macroBodyTokens", () => {
        let depth = 0;
        this.MANY({
            GATE: () => !(tokenMatcher(this.LA(1), RParen) && depth === 0),
            DEF: () => {
                if (tokenMatcher(this.LA(1), LParen)) {
                    depth++;
                } else if (tokenMatcher(this.LA(1), RParen)) {
                    depth--;
                }
                const tok = this.LA(1);
                (this as any).consumeToken();
                // Record the consumed token in the CST so visitors can access it.
                // Guard against Chevrotain's grammar recording phase where CST_STACK is empty.
                if (!(this as any).RECORDING_PHASE) {
                    (this as any).cstPostTerminal(tok.tokenType.name, tok);
                }
            },
        });
    });

    junoStandaloneDeclaration = this.RULE("junoStandaloneDeclaration", () => {
        this.OR([
            {
                ALT: () => {
                    this.OR2([
                        { ALT: () => this.CONSUME(JunoModuleDeclare) },
                        { ALT: () => this.CONSUME(JunoModuleRootDeclare) },
                        { ALT: () => this.CONSUME(JunoModuleDeriveDeclare) },
                    ]);
                    this.CONSUME(LParen);
                    this.CONSUME(Identifier);
                    this.CONSUME(RParen);
                    this.OPTION(() => this.CONSUME(Semicolon));
                },
            },
            {
                ALT: () => {
                    this.OR4([
                        { ALT: () => this.CONSUME(JunoModuleResult) },
                        { ALT: () => this.CONSUME(JunoModuleOption) },
                    ]);
                    this.CONSUME2(LParen);
                    this.CONSUME2(Identifier);
                    this.CONSUME(Comma);
                    this.OR3([
                        { ALT: () => this.CONSUME3(Identifier) },
                        { ALT: () => this.CONSUME(SizeT) },
                        { ALT: () => this.CONSUME(Bool) },
                        { ALT: () => this.CONSUME(Void) },
                        { ALT: () => this.CONSUME(Double) },
                        { ALT: () => this.CONSUME(Float) },
                        { ALT: () => this.CONSUME(Int) },
                        { ALT: () => this.CONSUME(Char) },
                        { ALT: () => this.CONSUME(Long) },
                        { ALT: () => this.CONSUME(Short) },
                        { ALT: () => this.CONSUME(Unsigned) },
                        { ALT: () => this.CONSUME(Signed) },
                    ]);
                    this.OPTION3(() => this.CONSUME(Star));
                    this.CONSUME2(RParen);
                    this.OPTION2(() => this.CONSUME2(Semicolon));
                },
            },
        ]);
    });

    // -------------------------------------------------------------------------
    // Function Definition
    // -------------------------------------------------------------------------

    functionDefinition = this.RULE("functionDefinition", () => {
        this.SUBRULE(this.declarationSpecifiers);
        this.SUBRULE(this.declarator);
        this.SUBRULE(this.compoundStatement);
    });

    // -------------------------------------------------------------------------
    // Stubs — bodies filled in later
    // -------------------------------------------------------------------------

    initDeclaratorList = this.RULE("initDeclaratorList", () => {
        this.SUBRULE(this.initDeclarator);
        this.MANY(() => {
            this.CONSUME(Comma);
            this.SUBRULE2(this.initDeclarator);
        });
    });

    initDeclarator = this.RULE("initDeclarator", () => {
        this.SUBRULE(this.declarator);
        this.OPTION(() => {
            this.CONSUME(Assign);
            this.SUBRULE(this.initializer);
        });
    });

    declarator = this.RULE("declarator", () => {
        this.OPTION(() => this.SUBRULE(this.pointer));
        this.SUBRULE(this.directDeclarator);
    });

    pointer = this.RULE("pointer", () => {
        this.CONSUME(Star);
        this.MANY(() => this.SUBRULE(this.typeQualifier));
        this.OPTION(() => this.SUBRULE(this.pointer));
    });

    directDeclarator = this.RULE("directDeclarator", () => {
        // Base: Identifier | '(' declarator ')'
        this.OR([
            { ALT: () => this.CONSUME(Identifier) },
            {
                ALT: () => {
                    this.CONSUME(LParen);
                    this.SUBRULE(this.declarator);
                    this.CONSUME(RParen);
                },
            },
        ]);
        // Suffixes: ( '[' expression? ']' | '(' ... ')' )*
        this.MANY(() => {
            this.OR2([
                {
                    // '[' expression? ']'
                    GATE: () => tokenMatcher(this.LA(1), LBracket),
                    ALT: () => {
                        this.CONSUME(LBracket);
                        this.OPTION(() => this.SUBRULE(this.expression));
                        this.CONSUME(RBracket);
                    },
                },
                {
                    // '(' ( parameterTypeList | identifierList? ) ')'
                    GATE: () => tokenMatcher(this.LA(1), LParen),
                    ALT: () => {
                        this.CONSUME2(LParen);
                        this.OR3([
                            {
                                GATE: this.BACKTRACK(this.parameterTypeList),
                                ALT: () => this.SUBRULE(this.parameterTypeList),
                            },
                            {
                                ALT: () => this.OPTION2(() => this.SUBRULE(this.identifierList)),
                            },
                        ]);
                        this.CONSUME2(RParen);
                    },
                },
            ]);
        });
    });

    parameterTypeList = this.RULE("parameterTypeList", () => {
        this.SUBRULE(this.parameterList);
        this.OPTION(() => {
            this.CONSUME(Comma);
            this.CONSUME(Ellipsis);
        });
    });

    parameterList = this.RULE("parameterList", () => {
        this.SUBRULE(this.parameterDeclaration);
        this.MANY({
            // Stop before ', ...' — the Ellipsis belongs to parameterTypeList
            GATE: () => tokenMatcher(this.LA(1), Comma) && !tokenMatcher(this.LA(2), Ellipsis),
            DEF: () => {
                this.CONSUME(Comma);
                this.SUBRULE2(this.parameterDeclaration);
            },
        });
    });

    parameterDeclaration = this.RULE("parameterDeclaration", () => {
        this.SUBRULE(this.declarationSpecifiers);
        this.OPTION(() => {
            this.OR([
                {
                    GATE: this.BACKTRACK(this.declarator),
                    ALT: () => this.SUBRULE(this.declarator),
                },
                {
                    ALT: () => this.SUBRULE(this.abstractDeclarator),
                },
            ]);
        });
    });

    abstractDeclarator = this.RULE("abstractDeclarator", () => {
        this.OPTION(() => this.SUBRULE(this.pointer));
        this.OPTION2(() => this.SUBRULE(this.abstractDirectDeclarator));
    });

    abstractDirectDeclarator = this.RULE("abstractDirectDeclarator", () => {
        // '(' abstractDeclarator ')' only when '(' is followed by '*' or '('
        // to distinguish from a parameterTypeList opening paren.
        this.OPTION({
            GATE: () =>
                tokenMatcher(this.LA(1), LParen) &&
                (tokenMatcher(this.LA(2), Star) || tokenMatcher(this.LA(2), LParen)),
            DEF: () => {
                this.CONSUME(LParen);
                this.SUBRULE(this.abstractDeclarator);
                this.CONSUME(RParen);
            },
        });
        // ( '[' expression? ']' | '(' parameterTypeList? ')' )*
        this.MANY(() => {
            this.OR([
                {
                    ALT: () => {
                        this.CONSUME(LBracket);
                        this.OPTION2(() => this.SUBRULE(this.expression));
                        this.CONSUME(RBracket);
                    },
                },
                {
                    ALT: () => {
                        this.CONSUME2(LParen);
                        this.OPTION3(() => this.SUBRULE(this.parameterTypeList));
                        this.CONSUME2(RParen);
                    },
                },
            ]);
        });
    });

    identifierList = this.RULE("identifierList", () => {
        this.CONSUME(Identifier);
        this.MANY(() => {
            this.CONSUME(Comma);
            this.CONSUME2(Identifier);
        });
    });

    typeName = this.RULE("typeName", () => {
        this.SUBRULE(this.specifierQualifierList);
        this.OPTION(() => this.SUBRULE(this.abstractDeclarator));
    });

    compoundStatement = this.RULE("compoundStatement", () => {
        this.CONSUME(LBrace);
        this.MANY(() => {
            this.OR([
                {
                    GATE: this.BACKTRACK(this.declaration),
                    ALT: () => this.SUBRULE(this.declaration),
                },
                {
                    ALT: () => this.SUBRULE(this.statement),
                },
            ]);
        });
        this.CONSUME(RBrace);
    });

    statement = this.RULE("statement", () => {
        this.OR([
            { ALT: () => this.SUBRULE(this.compoundStatement) },
            { ALT: () => this.SUBRULE(this.selectionStatement) },
            { ALT: () => this.SUBRULE(this.iterationStatement) },
            { ALT: () => this.SUBRULE(this.jumpStatement) },
            {
                // labeledStatement: Identifier ':' | 'case' | 'default'
                GATE: () =>
                    (tokenMatcher(this.LA(1), Identifier) && tokenMatcher(this.LA(2), Colon)) ||
                    tokenMatcher(this.LA(1), Case) ||
                    tokenMatcher(this.LA(1), Default),
                ALT: () => this.SUBRULE(this.labeledStatement),
            },
            {
                GATE: () => this.isMacroCallWithKeywordArg(),
                ALT: () => this.gobbleMacroCallStatement(),
            },
            { ALT: () => this.SUBRULE(this.expressionStatement) },
        ]);
    });

    expressionStatement = this.RULE("expressionStatement", () => {
        this.OPTION(() => this.SUBRULE(this.expression));
        this.CONSUME(Semicolon);
    });

    selectionStatement = this.RULE("selectionStatement", () => {
        this.OR([
            {
                // 'if' '(' expression ')' statement ( 'else' statement )?
                ALT: () => {
                    this.CONSUME(If);
                    this.CONSUME(LParen);
                    this.SUBRULE(this.expression);
                    this.CONSUME(RParen);
                    this.SUBRULE(this.statement);
                    this.OPTION(() => {
                        this.CONSUME(Else);
                        this.SUBRULE2(this.statement);
                    });
                },
            },
            {
                // 'switch' '(' expression ')' statement
                ALT: () => {
                    this.CONSUME(Switch);
                    this.CONSUME2(LParen);
                    this.SUBRULE2(this.expression);
                    this.CONSUME2(RParen);
                    this.SUBRULE3(this.statement);
                },
            },
        ]);
    });

    iterationStatement = this.RULE("iterationStatement", () => {
        this.OR([
            {
                // 'while' '(' expression ')' statement
                ALT: () => {
                    this.CONSUME(While);
                    this.CONSUME(LParen);
                    this.SUBRULE(this.expression);
                    this.CONSUME(RParen);
                    this.SUBRULE(this.statement);
                },
            },
            {
                // 'do' statement 'while' '(' expression ')' ';'
                ALT: () => {
                    this.CONSUME(Do);
                    this.SUBRULE2(this.statement);
                    this.CONSUME2(While);
                    this.CONSUME2(LParen);
                    this.SUBRULE2(this.expression);
                    this.CONSUME2(RParen);
                    this.CONSUME(Semicolon);
                },
            },
            {
                // 'for' '(' ( declaration | expressionStatement ) expressionStatement expression? ')' statement
                ALT: () => {
                    this.CONSUME(For);
                    this.CONSUME3(LParen);
                    this.OR2([
                        {
                            GATE: this.BACKTRACK(this.declaration),
                            ALT: () => this.SUBRULE(this.declaration),
                        },
                        {
                            ALT: () => this.SUBRULE(this.expressionStatement),
                        },
                    ]);
                    this.SUBRULE2(this.expressionStatement);
                    this.OPTION(() => this.SUBRULE3(this.expression));
                    this.CONSUME3(RParen);
                    this.SUBRULE3(this.statement);
                },
            },
        ]);
    });

    jumpStatement = this.RULE("jumpStatement", () => {
        this.OR([
            {
                // 'return' expression? ';'
                ALT: () => {
                    this.CONSUME(Return);
                    this.OPTION(() => this.SUBRULE(this.expression));
                    this.CONSUME(Semicolon);
                },
            },
            {
                // 'break' ';'
                ALT: () => {
                    this.CONSUME(Break);
                    this.CONSUME2(Semicolon);
                },
            },
            {
                // 'continue' ';'
                ALT: () => {
                    this.CONSUME(Continue);
                    this.CONSUME3(Semicolon);
                },
            },
            {
                // 'goto' Identifier ';'
                ALT: () => {
                    this.CONSUME(Goto);
                    this.CONSUME(Identifier);
                    this.CONSUME4(Semicolon);
                },
            },
        ]);
    });

    labeledStatement = this.RULE("labeledStatement", () => {
        this.OR([
            {
                // Identifier ':' statement
                ALT: () => {
                    this.CONSUME(Identifier);
                    this.CONSUME(Colon);
                    this.SUBRULE(this.statement);
                },
            },
            {
                // 'case' constantExpression ':' statement
                ALT: () => {
                    this.CONSUME(Case);
                    this.SUBRULE(this.constantExpression);
                    this.CONSUME2(Colon);
                    this.SUBRULE2(this.statement);
                },
            },
            {
                // 'default' ':' statement
                ALT: () => {
                    this.CONSUME(Default);
                    this.CONSUME3(Colon);
                    this.SUBRULE3(this.statement);
                },
            },
        ]);
    });
    expression = this.RULE("expression", () => {
        this.SUBRULE(this.assignmentExpression);
        this.MANY(() => {
            this.CONSUME(Comma);
            this.SUBRULE2(this.assignmentExpression);
        });
    });

    assignmentExpression = this.RULE("assignmentExpression", () => {
        this.SUBRULE(this.conditionalExpression);
        this.OPTION(() => {
            this.SUBRULE(this.assignmentOperator);
            this.SUBRULE(this.assignmentExpression);
        });
    });

    assignmentOperator = this.RULE("assignmentOperator", () => {
        this.OR([
            { ALT: () => this.CONSUME(Assign) },
            { ALT: () => this.CONSUME(PlusAssign) },
            { ALT: () => this.CONSUME(MinusAssign) },
            { ALT: () => this.CONSUME(StarAssign) },
            { ALT: () => this.CONSUME(SlashAssign) },
            { ALT: () => this.CONSUME(PercentAssign) },
            { ALT: () => this.CONSUME(AmpAssign) },
            { ALT: () => this.CONSUME(PipeAssign) },
            { ALT: () => this.CONSUME(CaretAssign) },
            { ALT: () => this.CONSUME(LShiftAssign) },
            { ALT: () => this.CONSUME(RShiftAssign) },
        ]);
    });

    conditionalExpression = this.RULE("conditionalExpression", () => {
        this.SUBRULE(this.logicalOrExpression);
        this.OPTION(() => {
            this.CONSUME(Question);
            this.SUBRULE(this.expression);
            this.CONSUME(Colon);
            this.SUBRULE(this.conditionalExpression);
        });
    });

    logicalOrExpression = this.RULE("logicalOrExpression", () => {
        this.SUBRULE(this.logicalAndExpression);
        this.MANY(() => {
            this.CONSUME(PipePipe);
            this.SUBRULE2(this.logicalAndExpression);
        });
    });

    logicalAndExpression = this.RULE("logicalAndExpression", () => {
        this.SUBRULE(this.bitwiseOrExpression);
        this.MANY(() => {
            this.CONSUME(AmpAmp);
            this.SUBRULE2(this.bitwiseOrExpression);
        });
    });

    bitwiseOrExpression = this.RULE("bitwiseOrExpression", () => {
        this.SUBRULE(this.bitwiseXorExpression);
        this.MANY(() => {
            this.CONSUME(Pipe);
            this.SUBRULE2(this.bitwiseXorExpression);
        });
    });

    bitwiseXorExpression = this.RULE("bitwiseXorExpression", () => {
        this.SUBRULE(this.bitwiseAndExpression);
        this.MANY(() => {
            this.CONSUME(Caret);
            this.SUBRULE2(this.bitwiseAndExpression);
        });
    });

    bitwiseAndExpression = this.RULE("bitwiseAndExpression", () => {
        this.SUBRULE(this.equalityExpression);
        this.MANY(() => {
            this.CONSUME(Amp);
            this.SUBRULE2(this.equalityExpression);
        });
    });

    equalityExpression = this.RULE("equalityExpression", () => {
        this.SUBRULE(this.relationalExpression);
        this.MANY(() => {
            this.OR([
                { ALT: () => this.CONSUME(EqEq) },
                { ALT: () => this.CONSUME(BangEq) },
            ]);
            this.SUBRULE2(this.relationalExpression);
        });
    });

    relationalExpression = this.RULE("relationalExpression", () => {
        this.SUBRULE(this.shiftExpression);
        this.MANY(() => {
            this.OR([
                { ALT: () => this.CONSUME(Lt) },
                { ALT: () => this.CONSUME(Gt) },
                { ALT: () => this.CONSUME(LtEq) },
                { ALT: () => this.CONSUME(GtEq) },
            ]);
            this.SUBRULE2(this.shiftExpression);
        });
    });

    shiftExpression = this.RULE("shiftExpression", () => {
        this.SUBRULE(this.additiveExpression);
        this.MANY(() => {
            this.OR([
                { ALT: () => this.CONSUME(LShift) },
                { ALT: () => this.CONSUME(RShift) },
            ]);
            this.SUBRULE2(this.additiveExpression);
        });
    });

    additiveExpression = this.RULE("additiveExpression", () => {
        this.SUBRULE(this.multiplicativeExpression);
        this.MANY(() => {
            this.OR([
                { ALT: () => this.CONSUME(Plus) },
                { ALT: () => this.CONSUME(Minus) },
            ]);
            this.SUBRULE2(this.multiplicativeExpression);
        });
    });

    multiplicativeExpression = this.RULE("multiplicativeExpression", () => {
        this.SUBRULE(this.castExpression);
        this.MANY(() => {
            this.OR([
                { ALT: () => this.CONSUME(Star) },
                { ALT: () => this.CONSUME(Slash) },
                { ALT: () => this.CONSUME(Percent) },
            ]);
            this.SUBRULE2(this.castExpression);
        });
    });

    castTypePrefix = this.RULE("castTypePrefix", () => {
        this.CONSUME(LParen);
        this.SUBRULE(this.typeName);
        this.CONSUME(RParen);
    });

    castExpression = this.RULE("castExpression", () => {
        // Pre-capture the backtrack function BEFORE the OR call.
        // Chevrotain's BACKTRACK() must not be invoked from inside a GATE lambda —
        // it must be called at OR-construction time so Chevrotain's internal
        // backtracking stack is properly set up before the GATE runs.
        const castTypeCheck = this.BACKTRACK(this.castTypePrefix);
        this.OR([
            {
                // looksLikeCast() is a fast lookahead that rejects (identifier) as a
                // cast when the token after the closing ')' cannot start a cast operand
                // (e.g. ';', ',', ')'). This prevents (ptFoo) from being mistakenly
                // parsed as a cast type instead of a parenthesised expression.
                GATE: () => this.looksLikeCast() && castTypeCheck.call(this),
                ALT: () => {
                    this.CONSUME(LParen);
                    this.SUBRULE(this.typeName);
                    this.CONSUME(RParen);
                    this.SUBRULE(this.castExpression);
                },
            },
            {
                ALT: () => this.SUBRULE(this.unaryExpression),
            },
        ]);
    });

    unaryExpression = this.RULE("unaryExpression", () => {
        this.OR([
            {
                ALT: () => {
                    this.OR2([
                        { ALT: () => this.CONSUME(PlusPlus) },
                        { ALT: () => this.CONSUME(MinusMinus) },
                        { ALT: () => this.CONSUME(Amp) },
                        { ALT: () => this.CONSUME(Star) },
                        { ALT: () => this.CONSUME(Plus) },
                        { ALT: () => this.CONSUME(Minus) },
                        { ALT: () => this.CONSUME(Tilde) },
                        { ALT: () => this.CONSUME(Bang) },
                    ]);
                    this.SUBRULE(this.unaryExpression);
                },
            },
            {
                ALT: () => {
                    this.CONSUME(Sizeof);
                    this.OR3([
                        {
                            GATE: this.BACKTRACK(this.castTypePrefix),
                            ALT: () => {
                                this.CONSUME(LParen);
                                this.SUBRULE(this.typeName);
                                this.CONSUME(RParen);
                            },
                        },
                        {
                            ALT: () => this.SUBRULE2(this.unaryExpression),
                        },
                    ]);
                },
            },
            {
                ALT: () => this.SUBRULE(this.postfixExpression),
            },
        ]);
    });

    postfixExpression = this.RULE("postfixExpression", () => {
        this.SUBRULE(this.primaryExpression);
        this.MANY(() => {
            this.OR([
                {
                    ALT: () => {
                        this.CONSUME(LBracket);
                        this.SUBRULE(this.expression);
                        this.CONSUME(RBracket);
                    },
                },
                {
                    ALT: () => {
                        this.CONSUME(LParen);
                        this.OPTION(() => this.SUBRULE(this.argumentExpressionList));
                        this.CONSUME(RParen);
                    },
                },
                {
                    ALT: () => {
                        this.CONSUME(Dot);
                        this.SUBRULE(this.memberIdentifier);
                    },
                },
                {
                    ALT: () => {
                        this.CONSUME(ArrowOp);
                        this.SUBRULE2(this.memberIdentifier);
                    },
                },
                { ALT: () => this.CONSUME(PlusPlus) },
                { ALT: () => this.CONSUME(MinusMinus) },
            ]);
        });
    });

    primaryExpression = this.RULE("primaryExpression", () => {
        this.OR([
            { ALT: () => this.CONSUME(Identifier) },
            { ALT: () => this.CONSUME(IntegerLiteral) },
            { ALT: () => this.CONSUME(FloatingLiteral) },
            { ALT: () => this.CONSUME(StringLiteral) },
            { ALT: () => this.CONSUME(CharLiteral) },
            {
                ALT: () => {
                    this.CONSUME(LParen);
                    this.SUBRULE(this.expression);
                    this.CONSUME(RParen);
                },
            },
            { ALT: () => this.SUBRULE(this.junoModuleGetApiMacro) },
            {
                // Braced compound literal / raw initializer list used as a macro argument
                // (e.g. MACRO(ARG, {0})). Not standard C expression syntax, but common
                // in LibJuno macro calls where {…} is passed as a raw token argument.
                ALT: () => {
                    this.CONSUME(LBrace);
                    this.SUBRULE(this.initializerList);
                    this.OPTION(() => this.CONSUME(Comma));
                    this.CONSUME(RBrace);
                },
            },
        ]);
    });

    memberIdentifier = this.RULE("memberIdentifier", () => {
        this.OR([
            { ALT: () => this.CONSUME(Identifier) },
            { ALT: () => this.CONSUME(JunoModuleSuper) },
            { ALT: () => this.CONSUME(JunoFailureHandler) },
            { ALT: () => this.CONSUME(JunoFailureUserData) },
        ]);
    });

    argumentExpressionList = this.RULE("argumentExpressionList", () => {
        this.SUBRULE(this.assignmentExpression);
        this.MANY(() => {
            this.CONSUME(Comma);
            this.SUBRULE2(this.assignmentExpression);
        });
    });

    junoModuleGetApiMacro = this.RULE("junoModuleGetApiMacro", () => {
        this.CONSUME(JunoModuleGetApi);
        this.CONSUME(LParen);
        this.SUBRULE(this.expression);
        this.CONSUME(Comma);
        this.SUBRULE(this.typeSpecifier);
        this.CONSUME(RParen);
    });

    initializer = this.RULE("initializer", () => {
        this.OR([
            {
                // Skip this alternative when the next token is '{'.
                // A bare '{' always means a brace-initializer list (second alt),
                // NOT a compound-literal primaryExpression — even though that is
                // now syntactically possible via the primaryExpression rule.
                // Keeping the brace path in the second alt preserves the CST
                // structure that the vtable visitor relies on.
                GATE: () => !tokenMatcher(this.LA(1), LBrace),
                ALT: () => this.SUBRULE(this.assignmentExpression),
            },
            {
                ALT: () => {
                    this.CONSUME(LBrace);
                    this.SUBRULE(this.initializerList);
                    this.OPTION(() => this.CONSUME(Comma));
                    this.CONSUME(RBrace);
                },
            },
        ]);
    });

    initializerList = this.RULE("initializerList", () => {
        this.OPTION(() => this.SUBRULE(this.designation));
        this.SUBRULE(this.initializer);
        this.MANY({
            GATE: () => tokenMatcher(this.LA(1), Comma) && !tokenMatcher(this.LA(2), RBrace),
            DEF: () => {
                this.CONSUME(Comma);
                this.OPTION2(() => this.SUBRULE2(this.designation));
                this.SUBRULE2(this.initializer);
            },
        });
    });

    designation = this.RULE("designation", () => {
        this.AT_LEAST_ONE(() => this.SUBRULE(this.designator));
        this.CONSUME(Assign);
    });

    designator = this.RULE("designator", () => {
        this.OR([
            {
                ALT: () => {
                    this.CONSUME(LBracket);
                    this.SUBRULE(this.constantExpression);
                    this.CONSUME(RBracket);
                },
            },
            {
                ALT: () => {
                    this.CONSUME(Dot);
                    this.CONSUME(Identifier);
                },
            },
        ]);
    });

    constantExpression = this.RULE("constantExpression", () => {
        this.SUBRULE(this.conditionalExpression);
    });

    /**
     * Fast lookahead: returns true only when the token sequence starting at LA(1)
     * looks like a C cast expression `( type-name )`, meaning:
     *  1. LA(1) is '('
     *  2. The token immediately after the matching closing ')' can be the start
     *     of a unary/primary expression (identifier, literal, '(', unary op,
     *     sizeof, or '{' compound literal).
     *
     * This prevents `(identifier)` followed by ';', ',', ')', '&&', etc. from
     * being speculatively mis-parsed as a C cast.
     */
    private looksLikeCast(): boolean {
        if (!tokenMatcher(this.LA(1), LParen)) { return false; }
        let depth = 1;
        let i = 2;
        while (depth > 0) {
            const t = this.LA(i);
            if (tokenMatcher(t, EOF)) { return false; } // EOF
            if (tokenMatcher(t, LParen))       { depth++; }
            else if (tokenMatcher(t, RParen))  { depth--; }
            i++;
        }
        // LA(i) is the token immediately after the matching ')'
        const after = this.LA(i);
        if (tokenMatcher(after, EOF)) { return false; }
        return (
            tokenMatcher(after, LParen)          || // another cast or paren-expr
            tokenMatcher(after, PlusPlus)        || // prefix ++
            tokenMatcher(after, MinusMinus)      || // prefix --
            tokenMatcher(after, Amp)             || // address-of
            tokenMatcher(after, Star)            || // deref
            tokenMatcher(after, Plus)            || // unary +
            tokenMatcher(after, Minus)           || // unary -
            tokenMatcher(after, Tilde)           || // bitwise NOT
            tokenMatcher(after, Bang)            || // logical NOT
            tokenMatcher(after, Sizeof)          || // sizeof
            tokenMatcher(after, Identifier)      || // primary
            tokenMatcher(after, IntegerLiteral)  || // primary
            tokenMatcher(after, FloatingLiteral) || // primary
            tokenMatcher(after, StringLiteral)   || // primary
            tokenMatcher(after, CharLiteral)     || // primary
            tokenMatcher(after, LBrace)             // compound literal (GCC / macro arg)
        );
    }

    /**
     * Scan ahead to determine if the current Identifier '(' ... ')' contains a
     * C keyword that cannot appear in an expression (return, break, continue, goto).
     * This indicates a macro call, not a function call, and must be gobbled.
     */
    private isMacroCallWithKeywordArg(): boolean {
        // LA(1) = Identifier, LA(2) = LParen
        if (!tokenMatcher(this.LA(1), Identifier) || !tokenMatcher(this.LA(2), LParen)) {
            return false;
        }
        let depth = 1;
        let i = 3;
        while (depth > 0) {
            const t = this.LA(i);
            if (tokenMatcher(t, EOF)) break; // EOF
            if (tokenMatcher(t, LParen)) { depth++; }
            else if (tokenMatcher(t, RParen)) { depth--; if (depth === 0) break; }
            else if (
                tokenMatcher(t, Return) ||
                tokenMatcher(t, Break) ||
                tokenMatcher(t, Continue) ||
                tokenMatcher(t, Goto)
            ) {
                return true;
            }
            i++;
        }
        return false;
    }

    /**
     * Gobble an entire macro call statement: Identifier '(' ... ')' ';'
     * Uses raw consumeToken() to avoid creating a grammar rule that would
     * cause Chevrotain's path-analysis to recurse infinitely through the
     * already-recursive statement → compoundStatement chain.
     */
    private gobbleMacroCallStatement(): void {
        // During Chevrotain's recording phase the CST stack is empty;
        // return immediately so no grammar structure is captured (the GATE
        // provides the lookahead predicate, removing the need for FIRST-set
        // analysis of this alternative).
        if ((this as any).RECORDING_PHASE) return;
        let depth = 0;
        while (true) {
            const tok = this.LA(1);
            if (tokenMatcher(tok, EOF)) break; // EOF sentinel
            const isLParen = tokenMatcher(tok, LParen);
            const isRParen = tokenMatcher(tok, RParen);
            const isSemi  = tokenMatcher(tok, Semicolon);
            if (isLParen) { depth++; }
            else if (isRParen) { depth--; }
            (this as any).consumeToken();
            (this as any).cstPostTerminal(tok.tokenType.name, tok);
            if (isSemi && depth === 0) break;
        }
    }
}
