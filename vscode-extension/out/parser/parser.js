"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.CParser = void 0;
const chevrotain_1 = require("chevrotain");
const lexer_1 = require("./lexer");
class CParser extends chevrotain_1.CstParser {
    constructor() {
        super(lexer_1.allTokens, { recoveryEnabled: true, nodeLocationTracking: "full", skipValidations: true });
        // -------------------------------------------------------------------------
        // Top-level
        // -------------------------------------------------------------------------
        this.translationUnit = this.RULE("translationUnit", () => {
            this.MANY(() => {
                this.OR([
                    { ALT: () => this.SUBRULE(this.preprocessorDirective) },
                    { ALT: () => this.SUBRULE(this.externalDeclaration) },
                ]);
            });
        });
        this.externalDeclaration = this.RULE("externalDeclaration", () => {
            this.OR([
                { GATE: this.BACKTRACK(this.functionDefinition), ALT: () => this.SUBRULE(this.functionDefinition) },
                { ALT: () => this.SUBRULE(this.junoStandaloneDeclaration) },
                { ALT: () => this.SUBRULE(this.declaration) },
            ]);
        });
        this.preprocessorDirective = this.RULE("preprocessorDirective", () => {
            this.CONSUME(lexer_1.HashDirective);
        });
        // -------------------------------------------------------------------------
        // Declarations
        // -------------------------------------------------------------------------
        this.declaration = this.RULE("declaration", () => {
            this.SUBRULE(this.declarationSpecifiers);
            this.OPTION(() => this.SUBRULE(this.initDeclaratorList));
            this.CONSUME(lexer_1.Semicolon);
        });
        this.declarationSpecifiers = this.RULE("declarationSpecifiers", () => {
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
                    if (!(0, chevrotain_1.tokenMatcher)(la1, lexer_1.Identifier)) {
                        // Only enter loop for tokens that can actually start a specifier.
                        return ((0, chevrotain_1.tokenMatcher)(la1, lexer_1.Static) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Typedef) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Extern) ||
                            (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Const) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Volatile) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Inline) ||
                            (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Void) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Char) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Short) ||
                            (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Int) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Long) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Float) ||
                            (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Double) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Signed) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Unsigned) ||
                            (0, chevrotain_1.tokenMatcher)(la1, lexer_1.SizeT) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Bool) ||
                            (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Struct) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Union) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Enum));
                    }
                    // Identifier: only a type specifier if followed by another Identifier, *,
                    // a qualifier, or a function-pointer open-paren+star.
                    const la2 = this.LA(2);
                    return ((0, chevrotain_1.tokenMatcher)(la2, lexer_1.Identifier) ||
                        (0, chevrotain_1.tokenMatcher)(la2, lexer_1.Star) ||
                        (0, chevrotain_1.tokenMatcher)(la2, lexer_1.Const) ||
                        (0, chevrotain_1.tokenMatcher)(la2, lexer_1.Volatile) ||
                        ((0, chevrotain_1.tokenMatcher)(la2, lexer_1.LParen) && (0, chevrotain_1.tokenMatcher)(this.LA(3), lexer_1.Star)));
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
        this.storageClassSpecifier = this.RULE("storageClassSpecifier", () => {
            this.OR([
                { ALT: () => this.CONSUME(lexer_1.Static) },
                { ALT: () => this.CONSUME(lexer_1.Typedef) },
                { ALT: () => this.CONSUME(lexer_1.Extern) },
            ]);
        });
        this.typeQualifier = this.RULE("typeQualifier", () => {
            this.OR([
                { ALT: () => this.CONSUME(lexer_1.Const) },
                { ALT: () => this.CONSUME(lexer_1.Volatile) },
                { ALT: () => this.CONSUME(lexer_1.Inline) },
            ]);
        });
        this.typeSpecifier = this.RULE("typeSpecifier", () => {
            this.OR([
                { ALT: () => this.SUBRULE(this.primitiveType) },
                { ALT: () => this.SUBRULE(this.structOrUnionSpecifier) },
                { ALT: () => this.SUBRULE(this.enumSpecifier) },
                { ALT: () => this.CONSUME(lexer_1.Identifier) },
            ]);
        });
        this.primitiveType = this.RULE("primitiveType", () => {
            this.OR([
                { ALT: () => this.CONSUME(lexer_1.Void) },
                { ALT: () => this.CONSUME(lexer_1.Char) },
                { ALT: () => this.CONSUME(lexer_1.Short) },
                { ALT: () => this.CONSUME(lexer_1.Int) },
                { ALT: () => this.CONSUME(lexer_1.Long) },
                { ALT: () => this.CONSUME(lexer_1.Float) },
                { ALT: () => this.CONSUME(lexer_1.Double) },
                { ALT: () => this.CONSUME(lexer_1.Signed) },
                { ALT: () => this.CONSUME(lexer_1.Unsigned) },
                { ALT: () => this.CONSUME(lexer_1.SizeT) },
                { ALT: () => this.CONSUME(lexer_1.Bool) },
            ]);
        });
        // -------------------------------------------------------------------------
        // Struct / Union
        // -------------------------------------------------------------------------
        this.structOrUnionSpecifier = this.RULE("structOrUnionSpecifier", () => {
            this.OR([
                { ALT: () => this.CONSUME(lexer_1.Struct) },
                { ALT: () => this.CONSUME(lexer_1.Union) },
            ]);
            // Discriminate: has-body (Identifier? '{') vs. forward-ref/macro (Identifier macroInvocation?)
            this.OR2([
                {
                    GATE: () => (0, chevrotain_1.tokenMatcher)(this.LA(1), lexer_1.LBrace) ||
                        ((0, chevrotain_1.tokenMatcher)(this.LA(1), lexer_1.Identifier) && (0, chevrotain_1.tokenMatcher)(this.LA(2), lexer_1.LBrace)),
                    ALT: () => {
                        this.OPTION(() => this.CONSUME(lexer_1.Identifier));
                        this.CONSUME(lexer_1.LBrace);
                        this.SUBRULE(this.structDeclarationList);
                        this.CONSUME(lexer_1.RBrace);
                    },
                },
                {
                    ALT: () => {
                        this.CONSUME2(lexer_1.Identifier);
                        this.OPTION2(() => this.SUBRULE(this.junoMacroInvocation));
                    },
                },
            ]);
        });
        this.structDeclarationList = this.RULE("structDeclarationList", () => {
            this.AT_LEAST_ONE(() => this.SUBRULE(this.structDeclaration));
        });
        this.structDeclaration = this.RULE("structDeclaration", () => {
            // junoMacroInvocation starts with distinct Juno tokens; no GATE needed.
            this.OR([
                {
                    ALT: () => {
                        this.SUBRULE(this.junoMacroInvocation);
                        this.OPTION(() => this.CONSUME(lexer_1.Semicolon));
                    },
                },
                {
                    ALT: () => {
                        this.SUBRULE(this.specifierQualifierList);
                        this.SUBRULE(this.structDeclaratorList);
                        this.CONSUME2(lexer_1.Semicolon);
                    },
                },
            ]);
        });
        this.specifierQualifierList = this.RULE("specifierQualifierList", () => {
            // Require at least one specifier or qualifier
            this.OR([
                { ALT: () => this.SUBRULE(this.typeSpecifier) },
                { ALT: () => this.SUBRULE(this.typeQualifier) },
            ]);
            // Continue consuming. Same GATE logic as declarationSpecifiers (no storage-class here).
            this.MANY({
                GATE: () => {
                    const la1 = this.LA(1);
                    if (!(0, chevrotain_1.tokenMatcher)(la1, lexer_1.Identifier)) {
                        return ((0, chevrotain_1.tokenMatcher)(la1, lexer_1.Const) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Volatile) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Inline) ||
                            (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Void) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Char) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Short) ||
                            (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Int) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Long) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Float) ||
                            (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Double) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Signed) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Unsigned) ||
                            (0, chevrotain_1.tokenMatcher)(la1, lexer_1.SizeT) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Bool) ||
                            (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Struct) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Union) || (0, chevrotain_1.tokenMatcher)(la1, lexer_1.Enum));
                    }
                    const la2 = this.LA(2);
                    return ((0, chevrotain_1.tokenMatcher)(la2, lexer_1.Identifier) ||
                        (0, chevrotain_1.tokenMatcher)(la2, lexer_1.Star) ||
                        (0, chevrotain_1.tokenMatcher)(la2, lexer_1.Const) ||
                        (0, chevrotain_1.tokenMatcher)(la2, lexer_1.Volatile) ||
                        ((0, chevrotain_1.tokenMatcher)(la2, lexer_1.LParen) && (0, chevrotain_1.tokenMatcher)(this.LA(3), lexer_1.Star)));
                },
                DEF: () => {
                    this.OR2([
                        { ALT: () => this.SUBRULE2(this.typeSpecifier) },
                        { ALT: () => this.SUBRULE2(this.typeQualifier) },
                    ]);
                },
            });
        });
        this.structDeclaratorList = this.RULE("structDeclaratorList", () => {
            this.SUBRULE(this.structDeclarator);
            this.MANY(() => {
                this.CONSUME(lexer_1.Comma);
                this.SUBRULE2(this.structDeclarator);
            });
        });
        this.structDeclarator = this.RULE("structDeclarator", () => {
            this.SUBRULE(this.declarator);
            this.OPTION(() => {
                this.CONSUME(lexer_1.Colon);
                this.SUBRULE(this.constantExpression);
            });
        });
        // -------------------------------------------------------------------------
        // Enum
        // -------------------------------------------------------------------------
        this.enumSpecifier = this.RULE("enumSpecifier", () => {
            this.CONSUME(lexer_1.Enum);
            this.OR([
                {
                    GATE: () => (0, chevrotain_1.tokenMatcher)(this.LA(1), lexer_1.LBrace) ||
                        ((0, chevrotain_1.tokenMatcher)(this.LA(1), lexer_1.Identifier) && (0, chevrotain_1.tokenMatcher)(this.LA(2), lexer_1.LBrace)),
                    ALT: () => {
                        this.OPTION(() => this.CONSUME(lexer_1.Identifier));
                        this.CONSUME(lexer_1.LBrace);
                        this.SUBRULE(this.enumeratorList);
                        this.OPTION2(() => this.CONSUME(lexer_1.Comma)); // trailing comma
                        this.CONSUME(lexer_1.RBrace);
                    },
                },
                {
                    ALT: () => this.CONSUME2(lexer_1.Identifier),
                },
            ]);
        });
        this.enumeratorList = this.RULE("enumeratorList", () => {
            this.SUBRULE(this.enumerator);
            this.MANY({
                // Stop before trailing comma followed by '}'
                GATE: () => (0, chevrotain_1.tokenMatcher)(this.LA(1), lexer_1.Comma) && !(0, chevrotain_1.tokenMatcher)(this.LA(2), lexer_1.RBrace),
                DEF: () => {
                    this.CONSUME(lexer_1.Comma);
                    this.SUBRULE2(this.enumerator);
                },
            });
        });
        this.enumerator = this.RULE("enumerator", () => {
            this.CONSUME(lexer_1.Identifier);
            this.OPTION(() => {
                this.CONSUME(lexer_1.Assign);
                this.SUBRULE(this.constantExpression);
            });
        });
        // -------------------------------------------------------------------------
        // LibJuno Macro Productions
        // -------------------------------------------------------------------------
        this.junoMacroInvocation = this.RULE("junoMacroInvocation", () => {
            this.OR([
                { ALT: () => this.SUBRULE(this.junoModuleRootMacro) },
                { ALT: () => this.SUBRULE(this.junoModuleDeriveMacro) },
                { ALT: () => this.SUBRULE(this.junoTraitRootMacro) },
                { ALT: () => this.SUBRULE(this.junoTraitDeriveMacro) },
                { ALT: () => this.SUBRULE(this.junoModuleMacro) },
            ]);
        });
        this.junoModuleRootMacro = this.RULE("junoModuleRootMacro", () => {
            this.CONSUME(lexer_1.JunoModuleRoot);
            this.CONSUME(lexer_1.LParen);
            this.CONSUME(lexer_1.Identifier);
            this.CONSUME(lexer_1.Comma);
            this.SUBRULE(this.macroBodyTokens);
            this.CONSUME(lexer_1.RParen);
        });
        this.junoModuleDeriveMacro = this.RULE("junoModuleDeriveMacro", () => {
            this.CONSUME(lexer_1.JunoModuleDerive);
            this.CONSUME(lexer_1.LParen);
            this.CONSUME(lexer_1.Identifier);
            this.CONSUME(lexer_1.Comma);
            this.SUBRULE(this.macroBodyTokens);
            this.CONSUME(lexer_1.RParen);
        });
        this.junoTraitRootMacro = this.RULE("junoTraitRootMacro", () => {
            this.CONSUME(lexer_1.JunoTraitRoot);
            this.CONSUME(lexer_1.LParen);
            this.CONSUME(lexer_1.Identifier);
            this.CONSUME(lexer_1.Comma);
            this.SUBRULE(this.macroBodyTokens);
            this.CONSUME(lexer_1.RParen);
        });
        this.junoTraitDeriveMacro = this.RULE("junoTraitDeriveMacro", () => {
            this.CONSUME(lexer_1.JunoTraitDerive);
            this.CONSUME(lexer_1.LParen);
            this.CONSUME(lexer_1.Identifier);
            this.CONSUME(lexer_1.Comma);
            this.SUBRULE(this.macroBodyTokens);
            this.CONSUME(lexer_1.RParen);
        });
        this.junoModuleMacro = this.RULE("junoModuleMacro", () => {
            this.CONSUME(lexer_1.JunoModule);
            this.CONSUME(lexer_1.LParen);
            this.CONSUME(lexer_1.Identifier);
            this.CONSUME(lexer_1.Comma);
            this.CONSUME2(lexer_1.Identifier);
            this.CONSUME2(lexer_1.Comma);
            this.SUBRULE(this.macroBodyTokens);
            this.CONSUME(lexer_1.RParen);
        });
        this.macroBodyTokens = this.RULE("macroBodyTokens", () => {
            let depth = 0;
            this.MANY({
                GATE: () => !((0, chevrotain_1.tokenMatcher)(this.LA(1), lexer_1.RParen) && depth === 0),
                DEF: () => {
                    if ((0, chevrotain_1.tokenMatcher)(this.LA(1), lexer_1.LParen)) {
                        depth++;
                    }
                    else if ((0, chevrotain_1.tokenMatcher)(this.LA(1), lexer_1.RParen)) {
                        depth--;
                    }
                    this.consumeToken();
                },
            });
        });
        this.junoStandaloneDeclaration = this.RULE("junoStandaloneDeclaration", () => {
            this.OR([
                {
                    ALT: () => {
                        this.OR2([
                            { ALT: () => this.CONSUME(lexer_1.JunoModuleDeclare) },
                            { ALT: () => this.CONSUME(lexer_1.JunoModuleRootDeclare) },
                            { ALT: () => this.CONSUME(lexer_1.JunoModuleDeriveDeclare) },
                        ]);
                        this.CONSUME(lexer_1.LParen);
                        this.CONSUME(lexer_1.Identifier);
                        this.CONSUME(lexer_1.RParen);
                        this.OPTION(() => this.CONSUME(lexer_1.Semicolon));
                    },
                },
                {
                    ALT: () => {
                        this.CONSUME(lexer_1.JunoModuleResult);
                        this.CONSUME2(lexer_1.LParen);
                        this.CONSUME2(lexer_1.Identifier);
                        this.CONSUME(lexer_1.Comma);
                        this.CONSUME3(lexer_1.Identifier);
                        this.CONSUME2(lexer_1.RParen);
                        this.OPTION2(() => this.CONSUME2(lexer_1.Semicolon));
                    },
                },
            ]);
        });
        // -------------------------------------------------------------------------
        // Function Definition
        // -------------------------------------------------------------------------
        this.functionDefinition = this.RULE("functionDefinition", () => {
            this.SUBRULE(this.declarationSpecifiers);
            this.SUBRULE(this.declarator);
            this.SUBRULE(this.compoundStatement);
        });
        // -------------------------------------------------------------------------
        // Stubs — bodies filled in later
        // -------------------------------------------------------------------------
        this.initDeclaratorList = this.RULE("initDeclaratorList", () => {
            this.SUBRULE(this.initDeclarator);
            this.MANY(() => {
                this.CONSUME(lexer_1.Comma);
                this.SUBRULE2(this.initDeclarator);
            });
        });
        this.initDeclarator = this.RULE("initDeclarator", () => {
            this.SUBRULE(this.declarator);
            this.OPTION(() => {
                this.CONSUME(lexer_1.Assign);
                this.SUBRULE(this.initializer);
            });
        });
        this.declarator = this.RULE("declarator", () => {
            this.OPTION(() => this.SUBRULE(this.pointer));
            this.SUBRULE(this.directDeclarator);
        });
        this.pointer = this.RULE("pointer", () => {
            this.CONSUME(lexer_1.Star);
            this.MANY(() => this.SUBRULE(this.typeQualifier));
            this.OPTION(() => this.SUBRULE(this.pointer));
        });
        this.directDeclarator = this.RULE("directDeclarator", () => {
            // Base: Identifier | '(' declarator ')'
            this.OR([
                { ALT: () => this.CONSUME(lexer_1.Identifier) },
                {
                    ALT: () => {
                        this.CONSUME(lexer_1.LParen);
                        this.SUBRULE(this.declarator);
                        this.CONSUME(lexer_1.RParen);
                    },
                },
            ]);
            // Suffixes: ( '[' expression? ']' | '(' ... ')' )*
            this.MANY(() => {
                this.OR2([
                    {
                        // '[' expression? ']'
                        GATE: () => (0, chevrotain_1.tokenMatcher)(this.LA(1), lexer_1.LBracket),
                        ALT: () => {
                            this.CONSUME(lexer_1.LBracket);
                            this.OPTION(() => this.SUBRULE(this.expression));
                            this.CONSUME(lexer_1.RBracket);
                        },
                    },
                    {
                        // '(' ( parameterTypeList | identifierList? ) ')'
                        GATE: () => (0, chevrotain_1.tokenMatcher)(this.LA(1), lexer_1.LParen),
                        ALT: () => {
                            this.CONSUME2(lexer_1.LParen);
                            this.OR3([
                                {
                                    GATE: this.BACKTRACK(this.parameterTypeList),
                                    ALT: () => this.SUBRULE(this.parameterTypeList),
                                },
                                {
                                    ALT: () => this.OPTION2(() => this.SUBRULE(this.identifierList)),
                                },
                            ]);
                            this.CONSUME2(lexer_1.RParen);
                        },
                    },
                ]);
            });
        });
        this.parameterTypeList = this.RULE("parameterTypeList", () => {
            this.SUBRULE(this.parameterList);
            this.OPTION(() => {
                this.CONSUME(lexer_1.Comma);
                this.CONSUME(lexer_1.Ellipsis);
            });
        });
        this.parameterList = this.RULE("parameterList", () => {
            this.SUBRULE(this.parameterDeclaration);
            this.MANY({
                // Stop before ', ...' — the Ellipsis belongs to parameterTypeList
                GATE: () => (0, chevrotain_1.tokenMatcher)(this.LA(1), lexer_1.Comma) && !(0, chevrotain_1.tokenMatcher)(this.LA(2), lexer_1.Ellipsis),
                DEF: () => {
                    this.CONSUME(lexer_1.Comma);
                    this.SUBRULE2(this.parameterDeclaration);
                },
            });
        });
        this.parameterDeclaration = this.RULE("parameterDeclaration", () => {
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
        this.abstractDeclarator = this.RULE("abstractDeclarator", () => {
            this.OPTION(() => this.SUBRULE(this.pointer));
            this.OPTION2(() => this.SUBRULE(this.abstractDirectDeclarator));
        });
        this.abstractDirectDeclarator = this.RULE("abstractDirectDeclarator", () => {
            // '(' abstractDeclarator ')' only when '(' is followed by '*' or '('
            // to distinguish from a parameterTypeList opening paren.
            this.OPTION({
                GATE: () => (0, chevrotain_1.tokenMatcher)(this.LA(1), lexer_1.LParen) &&
                    ((0, chevrotain_1.tokenMatcher)(this.LA(2), lexer_1.Star) || (0, chevrotain_1.tokenMatcher)(this.LA(2), lexer_1.LParen)),
                DEF: () => {
                    this.CONSUME(lexer_1.LParen);
                    this.SUBRULE(this.abstractDeclarator);
                    this.CONSUME(lexer_1.RParen);
                },
            });
            // ( '[' expression? ']' | '(' parameterTypeList? ')' )*
            this.MANY(() => {
                this.OR([
                    {
                        ALT: () => {
                            this.CONSUME(lexer_1.LBracket);
                            this.OPTION2(() => this.SUBRULE(this.expression));
                            this.CONSUME(lexer_1.RBracket);
                        },
                    },
                    {
                        ALT: () => {
                            this.CONSUME2(lexer_1.LParen);
                            this.OPTION3(() => this.SUBRULE(this.parameterTypeList));
                            this.CONSUME2(lexer_1.RParen);
                        },
                    },
                ]);
            });
        });
        this.identifierList = this.RULE("identifierList", () => {
            this.CONSUME(lexer_1.Identifier);
            this.MANY(() => {
                this.CONSUME(lexer_1.Comma);
                this.CONSUME2(lexer_1.Identifier);
            });
        });
        this.typeName = this.RULE("typeName", () => {
            this.SUBRULE(this.specifierQualifierList);
            this.OPTION(() => this.SUBRULE(this.abstractDeclarator));
        });
        this.compoundStatement = this.RULE("compoundStatement", () => {
            this.CONSUME(lexer_1.LBrace);
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
            this.CONSUME(lexer_1.RBrace);
        });
        this.statement = this.RULE("statement", () => {
            this.OR([
                { ALT: () => this.SUBRULE(this.compoundStatement) },
                { ALT: () => this.SUBRULE(this.selectionStatement) },
                { ALT: () => this.SUBRULE(this.iterationStatement) },
                { ALT: () => this.SUBRULE(this.jumpStatement) },
                {
                    // labeledStatement: Identifier ':' | 'case' | 'default'
                    GATE: () => ((0, chevrotain_1.tokenMatcher)(this.LA(1), lexer_1.Identifier) && (0, chevrotain_1.tokenMatcher)(this.LA(2), lexer_1.Colon)) ||
                        (0, chevrotain_1.tokenMatcher)(this.LA(1), lexer_1.Case) ||
                        (0, chevrotain_1.tokenMatcher)(this.LA(1), lexer_1.Default),
                    ALT: () => this.SUBRULE(this.labeledStatement),
                },
                { ALT: () => this.SUBRULE(this.expressionStatement) },
            ]);
        });
        this.expressionStatement = this.RULE("expressionStatement", () => {
            this.OPTION(() => this.SUBRULE(this.expression));
            this.CONSUME(lexer_1.Semicolon);
        });
        this.selectionStatement = this.RULE("selectionStatement", () => {
            this.OR([
                {
                    // 'if' '(' expression ')' statement ( 'else' statement )?
                    ALT: () => {
                        this.CONSUME(lexer_1.If);
                        this.CONSUME(lexer_1.LParen);
                        this.SUBRULE(this.expression);
                        this.CONSUME(lexer_1.RParen);
                        this.SUBRULE(this.statement);
                        this.OPTION(() => {
                            this.CONSUME(lexer_1.Else);
                            this.SUBRULE2(this.statement);
                        });
                    },
                },
                {
                    // 'switch' '(' expression ')' statement
                    ALT: () => {
                        this.CONSUME(lexer_1.Switch);
                        this.CONSUME2(lexer_1.LParen);
                        this.SUBRULE2(this.expression);
                        this.CONSUME2(lexer_1.RParen);
                        this.SUBRULE3(this.statement);
                    },
                },
            ]);
        });
        this.iterationStatement = this.RULE("iterationStatement", () => {
            this.OR([
                {
                    // 'while' '(' expression ')' statement
                    ALT: () => {
                        this.CONSUME(lexer_1.While);
                        this.CONSUME(lexer_1.LParen);
                        this.SUBRULE(this.expression);
                        this.CONSUME(lexer_1.RParen);
                        this.SUBRULE(this.statement);
                    },
                },
                {
                    // 'do' statement 'while' '(' expression ')' ';'
                    ALT: () => {
                        this.CONSUME(lexer_1.Do);
                        this.SUBRULE2(this.statement);
                        this.CONSUME2(lexer_1.While);
                        this.CONSUME2(lexer_1.LParen);
                        this.SUBRULE2(this.expression);
                        this.CONSUME2(lexer_1.RParen);
                        this.CONSUME(lexer_1.Semicolon);
                    },
                },
                {
                    // 'for' '(' ( declaration | expressionStatement ) expressionStatement expression? ')' statement
                    ALT: () => {
                        this.CONSUME(lexer_1.For);
                        this.CONSUME3(lexer_1.LParen);
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
                        this.CONSUME3(lexer_1.RParen);
                        this.SUBRULE3(this.statement);
                    },
                },
            ]);
        });
        this.jumpStatement = this.RULE("jumpStatement", () => {
            this.OR([
                {
                    // 'return' expression? ';'
                    ALT: () => {
                        this.CONSUME(lexer_1.Return);
                        this.OPTION(() => this.SUBRULE(this.expression));
                        this.CONSUME(lexer_1.Semicolon);
                    },
                },
                {
                    // 'break' ';'
                    ALT: () => {
                        this.CONSUME(lexer_1.Break);
                        this.CONSUME2(lexer_1.Semicolon);
                    },
                },
                {
                    // 'continue' ';'
                    ALT: () => {
                        this.CONSUME(lexer_1.Continue);
                        this.CONSUME3(lexer_1.Semicolon);
                    },
                },
                {
                    // 'goto' Identifier ';'
                    ALT: () => {
                        this.CONSUME(lexer_1.Goto);
                        this.CONSUME(lexer_1.Identifier);
                        this.CONSUME4(lexer_1.Semicolon);
                    },
                },
            ]);
        });
        this.labeledStatement = this.RULE("labeledStatement", () => {
            this.OR([
                {
                    // Identifier ':' statement
                    ALT: () => {
                        this.CONSUME(lexer_1.Identifier);
                        this.CONSUME(lexer_1.Colon);
                        this.SUBRULE(this.statement);
                    },
                },
                {
                    // 'case' constantExpression ':' statement
                    ALT: () => {
                        this.CONSUME(lexer_1.Case);
                        this.SUBRULE(this.constantExpression);
                        this.CONSUME2(lexer_1.Colon);
                        this.SUBRULE2(this.statement);
                    },
                },
                {
                    // 'default' ':' statement
                    ALT: () => {
                        this.CONSUME(lexer_1.Default);
                        this.CONSUME3(lexer_1.Colon);
                        this.SUBRULE3(this.statement);
                    },
                },
            ]);
        });
        this.expression = this.RULE("expression", () => {
            this.SUBRULE(this.assignmentExpression);
            this.MANY(() => {
                this.CONSUME(lexer_1.Comma);
                this.SUBRULE2(this.assignmentExpression);
            });
        });
        this.assignmentExpression = this.RULE("assignmentExpression", () => {
            this.SUBRULE(this.conditionalExpression);
            this.OPTION(() => {
                this.SUBRULE(this.assignmentOperator);
                this.SUBRULE(this.assignmentExpression);
            });
        });
        this.assignmentOperator = this.RULE("assignmentOperator", () => {
            this.OR([
                { ALT: () => this.CONSUME(lexer_1.Assign) },
                { ALT: () => this.CONSUME(lexer_1.PlusAssign) },
                { ALT: () => this.CONSUME(lexer_1.MinusAssign) },
                { ALT: () => this.CONSUME(lexer_1.StarAssign) },
                { ALT: () => this.CONSUME(lexer_1.SlashAssign) },
                { ALT: () => this.CONSUME(lexer_1.PercentAssign) },
                { ALT: () => this.CONSUME(lexer_1.AmpAssign) },
                { ALT: () => this.CONSUME(lexer_1.PipeAssign) },
                { ALT: () => this.CONSUME(lexer_1.CaretAssign) },
                { ALT: () => this.CONSUME(lexer_1.LShiftAssign) },
                { ALT: () => this.CONSUME(lexer_1.RShiftAssign) },
            ]);
        });
        this.conditionalExpression = this.RULE("conditionalExpression", () => {
            this.SUBRULE(this.logicalOrExpression);
            this.OPTION(() => {
                this.CONSUME(lexer_1.Question);
                this.SUBRULE(this.expression);
                this.CONSUME(lexer_1.Colon);
                this.SUBRULE(this.conditionalExpression);
            });
        });
        this.logicalOrExpression = this.RULE("logicalOrExpression", () => {
            this.SUBRULE(this.logicalAndExpression);
            this.MANY(() => {
                this.CONSUME(lexer_1.PipePipe);
                this.SUBRULE2(this.logicalAndExpression);
            });
        });
        this.logicalAndExpression = this.RULE("logicalAndExpression", () => {
            this.SUBRULE(this.bitwiseOrExpression);
            this.MANY(() => {
                this.CONSUME(lexer_1.AmpAmp);
                this.SUBRULE2(this.bitwiseOrExpression);
            });
        });
        this.bitwiseOrExpression = this.RULE("bitwiseOrExpression", () => {
            this.SUBRULE(this.bitwiseXorExpression);
            this.MANY(() => {
                this.CONSUME(lexer_1.Pipe);
                this.SUBRULE2(this.bitwiseXorExpression);
            });
        });
        this.bitwiseXorExpression = this.RULE("bitwiseXorExpression", () => {
            this.SUBRULE(this.bitwiseAndExpression);
            this.MANY(() => {
                this.CONSUME(lexer_1.Caret);
                this.SUBRULE2(this.bitwiseAndExpression);
            });
        });
        this.bitwiseAndExpression = this.RULE("bitwiseAndExpression", () => {
            this.SUBRULE(this.equalityExpression);
            this.MANY(() => {
                this.CONSUME(lexer_1.Amp);
                this.SUBRULE2(this.equalityExpression);
            });
        });
        this.equalityExpression = this.RULE("equalityExpression", () => {
            this.SUBRULE(this.relationalExpression);
            this.MANY(() => {
                this.OR([
                    { ALT: () => this.CONSUME(lexer_1.EqEq) },
                    { ALT: () => this.CONSUME(lexer_1.BangEq) },
                ]);
                this.SUBRULE2(this.relationalExpression);
            });
        });
        this.relationalExpression = this.RULE("relationalExpression", () => {
            this.SUBRULE(this.shiftExpression);
            this.MANY(() => {
                this.OR([
                    { ALT: () => this.CONSUME(lexer_1.Lt) },
                    { ALT: () => this.CONSUME(lexer_1.Gt) },
                    { ALT: () => this.CONSUME(lexer_1.LtEq) },
                    { ALT: () => this.CONSUME(lexer_1.GtEq) },
                ]);
                this.SUBRULE2(this.shiftExpression);
            });
        });
        this.shiftExpression = this.RULE("shiftExpression", () => {
            this.SUBRULE(this.additiveExpression);
            this.MANY(() => {
                this.OR([
                    { ALT: () => this.CONSUME(lexer_1.LShift) },
                    { ALT: () => this.CONSUME(lexer_1.RShift) },
                ]);
                this.SUBRULE2(this.additiveExpression);
            });
        });
        this.additiveExpression = this.RULE("additiveExpression", () => {
            this.SUBRULE(this.multiplicativeExpression);
            this.MANY(() => {
                this.OR([
                    { ALT: () => this.CONSUME(lexer_1.Plus) },
                    { ALT: () => this.CONSUME(lexer_1.Minus) },
                ]);
                this.SUBRULE2(this.multiplicativeExpression);
            });
        });
        this.multiplicativeExpression = this.RULE("multiplicativeExpression", () => {
            this.SUBRULE(this.castExpression);
            this.MANY(() => {
                this.OR([
                    { ALT: () => this.CONSUME(lexer_1.Star) },
                    { ALT: () => this.CONSUME(lexer_1.Slash) },
                    { ALT: () => this.CONSUME(lexer_1.Percent) },
                ]);
                this.SUBRULE2(this.castExpression);
            });
        });
        this.castTypePrefix = this.RULE("castTypePrefix", () => {
            this.CONSUME(lexer_1.LParen);
            this.SUBRULE(this.typeName);
            this.CONSUME(lexer_1.RParen);
        });
        this.castExpression = this.RULE("castExpression", () => {
            this.OR([
                {
                    GATE: this.BACKTRACK(this.castTypePrefix),
                    ALT: () => {
                        this.CONSUME(lexer_1.LParen);
                        this.SUBRULE(this.typeName);
                        this.CONSUME(lexer_1.RParen);
                        this.SUBRULE(this.castExpression);
                    },
                },
                {
                    ALT: () => this.SUBRULE(this.unaryExpression),
                },
            ]);
        });
        this.unaryExpression = this.RULE("unaryExpression", () => {
            this.OR([
                {
                    ALT: () => {
                        this.OR2([
                            { ALT: () => this.CONSUME(lexer_1.PlusPlus) },
                            { ALT: () => this.CONSUME(lexer_1.MinusMinus) },
                            { ALT: () => this.CONSUME(lexer_1.Amp) },
                            { ALT: () => this.CONSUME(lexer_1.Star) },
                            { ALT: () => this.CONSUME(lexer_1.Plus) },
                            { ALT: () => this.CONSUME(lexer_1.Minus) },
                            { ALT: () => this.CONSUME(lexer_1.Tilde) },
                            { ALT: () => this.CONSUME(lexer_1.Bang) },
                        ]);
                        this.SUBRULE(this.unaryExpression);
                    },
                },
                {
                    ALT: () => {
                        this.CONSUME(lexer_1.Sizeof);
                        this.OR3([
                            {
                                GATE: this.BACKTRACK(this.castTypePrefix),
                                ALT: () => {
                                    this.CONSUME(lexer_1.LParen);
                                    this.SUBRULE(this.typeName);
                                    this.CONSUME(lexer_1.RParen);
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
        this.postfixExpression = this.RULE("postfixExpression", () => {
            this.SUBRULE(this.primaryExpression);
            this.MANY(() => {
                this.OR([
                    {
                        ALT: () => {
                            this.CONSUME(lexer_1.LBracket);
                            this.SUBRULE(this.expression);
                            this.CONSUME(lexer_1.RBracket);
                        },
                    },
                    {
                        ALT: () => {
                            this.CONSUME(lexer_1.LParen);
                            this.OPTION(() => this.SUBRULE(this.argumentExpressionList));
                            this.CONSUME(lexer_1.RParen);
                        },
                    },
                    {
                        ALT: () => {
                            this.CONSUME(lexer_1.Dot);
                            this.SUBRULE(this.memberIdentifier);
                        },
                    },
                    {
                        ALT: () => {
                            this.CONSUME(lexer_1.ArrowOp);
                            this.SUBRULE2(this.memberIdentifier);
                        },
                    },
                    { ALT: () => this.CONSUME(lexer_1.PlusPlus) },
                    { ALT: () => this.CONSUME(lexer_1.MinusMinus) },
                ]);
            });
        });
        this.primaryExpression = this.RULE("primaryExpression", () => {
            this.OR([
                { ALT: () => this.CONSUME(lexer_1.Identifier) },
                { ALT: () => this.CONSUME(lexer_1.IntegerLiteral) },
                { ALT: () => this.CONSUME(lexer_1.FloatingLiteral) },
                { ALT: () => this.CONSUME(lexer_1.StringLiteral) },
                { ALT: () => this.CONSUME(lexer_1.CharLiteral) },
                {
                    ALT: () => {
                        this.CONSUME(lexer_1.LParen);
                        this.SUBRULE(this.expression);
                        this.CONSUME(lexer_1.RParen);
                    },
                },
                { ALT: () => this.SUBRULE(this.junoModuleGetApiMacro) },
            ]);
        });
        this.memberIdentifier = this.RULE("memberIdentifier", () => {
            this.OR([
                { ALT: () => this.CONSUME(lexer_1.Identifier) },
                { ALT: () => this.CONSUME(lexer_1.JunoModuleSuper) },
                { ALT: () => this.CONSUME(lexer_1.JunoFailureHandler) },
                { ALT: () => this.CONSUME(lexer_1.JunoFailureUserData) },
            ]);
        });
        this.argumentExpressionList = this.RULE("argumentExpressionList", () => {
            this.SUBRULE(this.assignmentExpression);
            this.MANY(() => {
                this.CONSUME(lexer_1.Comma);
                this.SUBRULE2(this.assignmentExpression);
            });
        });
        this.junoModuleGetApiMacro = this.RULE("junoModuleGetApiMacro", () => {
            this.CONSUME(lexer_1.JunoModuleGetApi);
            this.CONSUME(lexer_1.LParen);
            this.SUBRULE(this.expression);
            this.CONSUME(lexer_1.Comma);
            this.SUBRULE(this.typeSpecifier);
            this.CONSUME(lexer_1.RParen);
        });
        this.initializer = this.RULE("initializer", () => {
            this.OR([
                { ALT: () => this.SUBRULE(this.assignmentExpression) },
                {
                    ALT: () => {
                        this.CONSUME(lexer_1.LBrace);
                        this.SUBRULE(this.initializerList);
                        this.OPTION(() => this.CONSUME(lexer_1.Comma));
                        this.CONSUME(lexer_1.RBrace);
                    },
                },
            ]);
        });
        this.initializerList = this.RULE("initializerList", () => {
            this.OPTION(() => this.SUBRULE(this.designation));
            this.SUBRULE(this.initializer);
            this.MANY({
                GATE: () => (0, chevrotain_1.tokenMatcher)(this.LA(1), lexer_1.Comma) && !(0, chevrotain_1.tokenMatcher)(this.LA(2), lexer_1.RBrace),
                DEF: () => {
                    this.CONSUME(lexer_1.Comma);
                    this.OPTION2(() => this.SUBRULE2(this.designation));
                    this.SUBRULE2(this.initializer);
                },
            });
        });
        this.designation = this.RULE("designation", () => {
            this.AT_LEAST_ONE(() => this.SUBRULE(this.designator));
            this.CONSUME(lexer_1.Assign);
        });
        this.designator = this.RULE("designator", () => {
            this.OR([
                {
                    ALT: () => {
                        this.CONSUME(lexer_1.LBracket);
                        this.SUBRULE(this.constantExpression);
                        this.CONSUME(lexer_1.RBracket);
                    },
                },
                {
                    ALT: () => {
                        this.CONSUME(lexer_1.Dot);
                        this.CONSUME(lexer_1.Identifier);
                    },
                },
            ]);
        });
        this.constantExpression = this.RULE("constantExpression", () => {
            this.SUBRULE(this.conditionalExpression);
        });
        this.performSelfAnalysis();
    }
}
exports.CParser = CParser;
//# sourceMappingURL=parser.js.map