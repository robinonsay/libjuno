"use strict";
/**
 * @file lexer.ts
 *
 * Chevrotain lexer (token definitions) for the LibJuno C parser.
 *
 * Token priority is determined by position in the `allTokens` array —
 * earlier entries win when multiple patterns match at the same position.
 *
 * Ordering rules enforced:
 *  1. `HashDirective` before `Hash` (consumes full directive lines first)
 *  2. All LibJuno macro tokens before `Identifier`
 *  3. All keyword tokens before `Identifier` (each uses `longer_alt: Identifier`)
 *  4. Compound operators before their simple operator prefixes
 *  5. `Identifier` is last among non-skipped tokens
 *  6. `WhiteSpace`, `LineComment`, `BlockComment` are SKIPPED
 */
Object.defineProperty(exports, "__esModule", { value: true });
exports.HashDirective = exports.JunoFailureUserData = exports.JunoFailureHandler = exports.JunoTraitDerive = exports.JunoTraitRoot = exports.JunoModule = exports.JunoModuleArg = exports.JunoModuleDeclare = exports.JunoModuleDerive = exports.JunoModuleRoot = exports.JunoModuleEmpty = exports.JunoModuleSuper = exports.JunoModuleResult = exports.JunoModuleGetApi = exports.JunoModuleDeriveDeclare = exports.JunoModuleRootDeclare = exports.Sizeof = exports.Goto = exports.Return = exports.Continue = exports.Break = exports.Default = exports.Case = exports.Switch = exports.Do = exports.While = exports.For = exports.Else = exports.If = exports.Bool = exports.SizeT = exports.Unsigned = exports.Signed = exports.Double = exports.Float = exports.Long = exports.Int = exports.Short = exports.Char = exports.Void = exports.Volatile = exports.Extern = exports.Typedef = exports.Enum = exports.Union = exports.Struct = exports.Inline = exports.Const = exports.Static = exports.Identifier = void 0;
exports.StringLiteral = exports.FloatingLiteral = exports.IntegerLiteral = exports.Hash = exports.Pipe = exports.Caret = exports.Question = exports.Colon = exports.Gt = exports.Lt = exports.Tilde = exports.Bang = exports.Percent = exports.Slash = exports.Minus = exports.Plus = exports.Amp = exports.Star = exports.Assign = exports.Dot = exports.Comma = exports.Semicolon = exports.RBracket = exports.LBracket = exports.RParen = exports.LParen = exports.RBrace = exports.LBrace = exports.PipePipe = exports.AmpAmp = exports.BangEq = exports.EqEq = exports.GtEq = exports.LtEq = exports.RShift = exports.LShift = exports.RShiftAssign = exports.LShiftAssign = exports.CaretAssign = exports.PipeAssign = exports.AmpAssign = exports.PercentAssign = exports.SlashAssign = exports.StarAssign = exports.MinusAssign = exports.PlusAssign = exports.MinusMinus = exports.PlusPlus = exports.ArrowOp = exports.Ellipsis = void 0;
exports.CLexer = exports.allTokens = exports.BlockComment = exports.LineComment = exports.WhiteSpace = exports.CharLiteral = void 0;
const chevrotain_1 = require("chevrotain");
// ---------------------------------------------------------------------------
// Identifier — defined first so keywords can reference it via `longer_alt`
// ---------------------------------------------------------------------------
exports.Identifier = (0, chevrotain_1.createToken)({
    name: "Identifier",
    pattern: /[a-zA-Z_][a-zA-Z0-9_]*/,
});
// ---------------------------------------------------------------------------
// Keywords — each uses `longer_alt: Identifier` so that identifiers that
// begin with a keyword string (e.g. "structure") are not split.
// ---------------------------------------------------------------------------
exports.Static = (0, chevrotain_1.createToken)({ name: "Static", pattern: /static/, longer_alt: exports.Identifier });
exports.Const = (0, chevrotain_1.createToken)({ name: "Const", pattern: /const/, longer_alt: exports.Identifier });
exports.Inline = (0, chevrotain_1.createToken)({ name: "Inline", pattern: /inline/, longer_alt: exports.Identifier });
exports.Struct = (0, chevrotain_1.createToken)({ name: "Struct", pattern: /struct/, longer_alt: exports.Identifier });
exports.Union = (0, chevrotain_1.createToken)({ name: "Union", pattern: /union/, longer_alt: exports.Identifier });
exports.Enum = (0, chevrotain_1.createToken)({ name: "Enum", pattern: /enum/, longer_alt: exports.Identifier });
exports.Typedef = (0, chevrotain_1.createToken)({ name: "Typedef", pattern: /typedef/, longer_alt: exports.Identifier });
exports.Extern = (0, chevrotain_1.createToken)({ name: "Extern", pattern: /extern/, longer_alt: exports.Identifier });
exports.Volatile = (0, chevrotain_1.createToken)({ name: "Volatile", pattern: /volatile/, longer_alt: exports.Identifier });
exports.Void = (0, chevrotain_1.createToken)({ name: "Void", pattern: /void/, longer_alt: exports.Identifier });
exports.Char = (0, chevrotain_1.createToken)({ name: "Char", pattern: /char/, longer_alt: exports.Identifier });
exports.Short = (0, chevrotain_1.createToken)({ name: "Short", pattern: /short/, longer_alt: exports.Identifier });
exports.Int = (0, chevrotain_1.createToken)({ name: "Int", pattern: /int/, longer_alt: exports.Identifier });
exports.Long = (0, chevrotain_1.createToken)({ name: "Long", pattern: /long/, longer_alt: exports.Identifier });
exports.Float = (0, chevrotain_1.createToken)({ name: "Float", pattern: /float/, longer_alt: exports.Identifier });
exports.Double = (0, chevrotain_1.createToken)({ name: "Double", pattern: /double/, longer_alt: exports.Identifier });
exports.Signed = (0, chevrotain_1.createToken)({ name: "Signed", pattern: /signed/, longer_alt: exports.Identifier });
exports.Unsigned = (0, chevrotain_1.createToken)({ name: "Unsigned", pattern: /unsigned/, longer_alt: exports.Identifier });
exports.SizeT = (0, chevrotain_1.createToken)({ name: "SizeT", pattern: /size_t/, longer_alt: exports.Identifier });
exports.Bool = (0, chevrotain_1.createToken)({ name: "Bool", pattern: /_Bool/, longer_alt: exports.Identifier });
exports.If = (0, chevrotain_1.createToken)({ name: "If", pattern: /if/, longer_alt: exports.Identifier });
exports.Else = (0, chevrotain_1.createToken)({ name: "Else", pattern: /else/, longer_alt: exports.Identifier });
exports.For = (0, chevrotain_1.createToken)({ name: "For", pattern: /for/, longer_alt: exports.Identifier });
exports.While = (0, chevrotain_1.createToken)({ name: "While", pattern: /while/, longer_alt: exports.Identifier });
exports.Do = (0, chevrotain_1.createToken)({ name: "Do", pattern: /do/, longer_alt: exports.Identifier });
exports.Switch = (0, chevrotain_1.createToken)({ name: "Switch", pattern: /switch/, longer_alt: exports.Identifier });
exports.Case = (0, chevrotain_1.createToken)({ name: "Case", pattern: /case/, longer_alt: exports.Identifier });
exports.Default = (0, chevrotain_1.createToken)({ name: "Default", pattern: /default/, longer_alt: exports.Identifier });
exports.Break = (0, chevrotain_1.createToken)({ name: "Break", pattern: /break/, longer_alt: exports.Identifier });
exports.Continue = (0, chevrotain_1.createToken)({ name: "Continue", pattern: /continue/, longer_alt: exports.Identifier });
exports.Return = (0, chevrotain_1.createToken)({ name: "Return", pattern: /return/, longer_alt: exports.Identifier });
exports.Goto = (0, chevrotain_1.createToken)({ name: "Goto", pattern: /goto/, longer_alt: exports.Identifier });
exports.Sizeof = (0, chevrotain_1.createToken)({ name: "Sizeof", pattern: /sizeof/, longer_alt: exports.Identifier });
// ---------------------------------------------------------------------------
// LibJuno Macro Tokens — defined before Identifier in allTokens.
// Longer/more specific prefixes listed first (defensive ordering).
// \b word-boundary prevents prefix matching (e.g. JUNO_MODULE_ROOT\b will
// NOT match JUNO_MODULE_ROOT_T because _ is a word char, so no boundary).
// ---------------------------------------------------------------------------
exports.JunoModuleRootDeclare = (0, chevrotain_1.createToken)({
    name: "JunoModuleRootDeclare",
    pattern: /JUNO_MODULE_ROOT_DECLARE\b/,
});
exports.JunoModuleDeriveDeclare = (0, chevrotain_1.createToken)({
    name: "JunoModuleDeriveDeclare",
    pattern: /JUNO_MODULE_DERIVE_DECLARE\b/,
});
exports.JunoModuleGetApi = (0, chevrotain_1.createToken)({
    name: "JunoModuleGetApi",
    pattern: /JUNO_MODULE_GET_API\b/,
});
exports.JunoModuleResult = (0, chevrotain_1.createToken)({
    name: "JunoModuleResult",
    pattern: /JUNO_MODULE_RESULT\b/,
});
exports.JunoModuleSuper = (0, chevrotain_1.createToken)({
    name: "JunoModuleSuper",
    pattern: /JUNO_MODULE_SUPER\b/,
});
exports.JunoModuleEmpty = (0, chevrotain_1.createToken)({
    name: "JunoModuleEmpty",
    pattern: /JUNO_MODULE_EMPTY\b/,
});
exports.JunoModuleRoot = (0, chevrotain_1.createToken)({
    name: "JunoModuleRoot",
    pattern: /JUNO_MODULE_ROOT\b/,
});
exports.JunoModuleDerive = (0, chevrotain_1.createToken)({
    name: "JunoModuleDerive",
    pattern: /JUNO_MODULE_DERIVE\b/,
});
exports.JunoModuleDeclare = (0, chevrotain_1.createToken)({
    name: "JunoModuleDeclare",
    pattern: /JUNO_MODULE_DECLARE\b/,
});
exports.JunoModuleArg = (0, chevrotain_1.createToken)({
    name: "JunoModuleArg",
    pattern: /JUNO_MODULE_ARG\b/,
});
exports.JunoModule = (0, chevrotain_1.createToken)({
    name: "JunoModule",
    pattern: /JUNO_MODULE\b/,
});
exports.JunoTraitRoot = (0, chevrotain_1.createToken)({
    name: "JunoTraitRoot",
    pattern: /JUNO_TRAIT_ROOT\b/,
});
exports.JunoTraitDerive = (0, chevrotain_1.createToken)({
    name: "JunoTraitDerive",
    pattern: /JUNO_TRAIT_DERIVE\b/,
});
exports.JunoFailureHandler = (0, chevrotain_1.createToken)({
    name: "JunoFailureHandler",
    pattern: /JUNO_FAILURE_HANDLER\b|_pfcnFailureHandler\b/,
});
exports.JunoFailureUserData = (0, chevrotain_1.createToken)({
    name: "JunoFailureUserData",
    pattern: /JUNO_FAILURE_USER_DATA\b|_pvFailureUserData\b/,
});
// ---------------------------------------------------------------------------
// Preprocessor — HashDirective must precede Hash
// Chevrotain does not allow the ^ anchor or the m/g flags in static patterns.
// Use a custom match function that preserves start-of-line semantics.
// ---------------------------------------------------------------------------
const _hashDirectiveRe = /[ \t]*#[ \t]*(?:define|include|ifdef|ifndef|if|elif|else|endif|pragma|undef|error|warning|line)[^\n]*/;
exports.HashDirective = (0, chevrotain_1.createToken)({
    name: "HashDirective",
    pattern: (text, startOffset) => {
        // Only match at the very beginning of the text or just after a newline.
        if (startOffset !== 0 && text[startOffset - 1] !== "\n") {
            return null;
        }
        const match = _hashDirectiveRe.exec(text.slice(startOffset));
        if (!match || match.index !== 0) {
            return null;
        }
        return match;
    },
    line_breaks: false,
});
// ---------------------------------------------------------------------------
// Punctuators — compound / longer forms listed before shorter prefix forms
// ---------------------------------------------------------------------------
// Multi-character compound operators — defined before their prefixes
exports.Ellipsis = (0, chevrotain_1.createToken)({ name: "Ellipsis", pattern: /\.\.\./ });
exports.ArrowOp = (0, chevrotain_1.createToken)({ name: "ArrowOp", pattern: /->/ });
exports.PlusPlus = (0, chevrotain_1.createToken)({ name: "PlusPlus", pattern: /\+\+/ });
exports.MinusMinus = (0, chevrotain_1.createToken)({ name: "MinusMinus", pattern: /--/ });
exports.PlusAssign = (0, chevrotain_1.createToken)({ name: "PlusAssign", pattern: /\+=/ });
exports.MinusAssign = (0, chevrotain_1.createToken)({ name: "MinusAssign", pattern: /-=/ });
exports.StarAssign = (0, chevrotain_1.createToken)({ name: "StarAssign", pattern: /\*=/ });
exports.SlashAssign = (0, chevrotain_1.createToken)({ name: "SlashAssign", pattern: /\/=/ });
exports.PercentAssign = (0, chevrotain_1.createToken)({ name: "PercentAssign", pattern: /%=/ });
exports.AmpAssign = (0, chevrotain_1.createToken)({ name: "AmpAssign", pattern: /&=/ });
exports.PipeAssign = (0, chevrotain_1.createToken)({ name: "PipeAssign", pattern: /\|=/ });
exports.CaretAssign = (0, chevrotain_1.createToken)({ name: "CaretAssign", pattern: /\^=/ });
exports.LShiftAssign = (0, chevrotain_1.createToken)({ name: "LShiftAssign", pattern: /<<=/ });
exports.RShiftAssign = (0, chevrotain_1.createToken)({ name: "RShiftAssign", pattern: />>=/ });
exports.LShift = (0, chevrotain_1.createToken)({ name: "LShift", pattern: /<</ });
exports.RShift = (0, chevrotain_1.createToken)({ name: "RShift", pattern: />>/ });
exports.LtEq = (0, chevrotain_1.createToken)({ name: "LtEq", pattern: /<=/ });
exports.GtEq = (0, chevrotain_1.createToken)({ name: "GtEq", pattern: />=/ });
exports.EqEq = (0, chevrotain_1.createToken)({ name: "EqEq", pattern: /==/ });
exports.BangEq = (0, chevrotain_1.createToken)({ name: "BangEq", pattern: /!=/ });
exports.AmpAmp = (0, chevrotain_1.createToken)({ name: "AmpAmp", pattern: /&&/ });
exports.PipePipe = (0, chevrotain_1.createToken)({ name: "PipePipe", pattern: /\|\|/ });
// Simple single-character operators — after their compound forms
exports.LBrace = (0, chevrotain_1.createToken)({ name: "LBrace", pattern: /\{/ });
exports.RBrace = (0, chevrotain_1.createToken)({ name: "RBrace", pattern: /\}/ });
exports.LParen = (0, chevrotain_1.createToken)({ name: "LParen", pattern: /\(/ });
exports.RParen = (0, chevrotain_1.createToken)({ name: "RParen", pattern: /\)/ });
exports.LBracket = (0, chevrotain_1.createToken)({ name: "LBracket", pattern: /\[/ });
exports.RBracket = (0, chevrotain_1.createToken)({ name: "RBracket", pattern: /\]/ });
exports.Semicolon = (0, chevrotain_1.createToken)({ name: "Semicolon", pattern: /;/ });
exports.Comma = (0, chevrotain_1.createToken)({ name: "Comma", pattern: /,/ });
exports.Dot = (0, chevrotain_1.createToken)({ name: "Dot", pattern: /\./ });
exports.Assign = (0, chevrotain_1.createToken)({ name: "Assign", pattern: /=/ });
exports.Star = (0, chevrotain_1.createToken)({ name: "Star", pattern: /\*/ });
exports.Amp = (0, chevrotain_1.createToken)({ name: "Amp", pattern: /&/ });
exports.Plus = (0, chevrotain_1.createToken)({ name: "Plus", pattern: /\+/ });
exports.Minus = (0, chevrotain_1.createToken)({ name: "Minus", pattern: /-/ });
exports.Slash = (0, chevrotain_1.createToken)({ name: "Slash", pattern: /\// });
exports.Percent = (0, chevrotain_1.createToken)({ name: "Percent", pattern: /%/ });
exports.Bang = (0, chevrotain_1.createToken)({ name: "Bang", pattern: /!/ });
exports.Tilde = (0, chevrotain_1.createToken)({ name: "Tilde", pattern: /~/ });
exports.Lt = (0, chevrotain_1.createToken)({ name: "Lt", pattern: /</ });
exports.Gt = (0, chevrotain_1.createToken)({ name: "Gt", pattern: />/ });
exports.Colon = (0, chevrotain_1.createToken)({ name: "Colon", pattern: /:/ });
exports.Question = (0, chevrotain_1.createToken)({ name: "Question", pattern: /\?/ });
exports.Caret = (0, chevrotain_1.createToken)({ name: "Caret", pattern: /\^/ });
exports.Pipe = (0, chevrotain_1.createToken)({ name: "Pipe", pattern: /\|/ });
exports.Hash = (0, chevrotain_1.createToken)({ name: "Hash", pattern: /#/ });
// ---------------------------------------------------------------------------
// Literals
// ---------------------------------------------------------------------------
exports.IntegerLiteral = (0, chevrotain_1.createToken)({
    name: "IntegerLiteral",
    pattern: /0[xX][0-9a-fA-F]+[uUlL]*|0[0-7]*[uUlL]*|[1-9][0-9]*[uUlL]*/,
});
exports.FloatingLiteral = (0, chevrotain_1.createToken)({
    name: "FloatingLiteral",
    pattern: /[0-9]*\.[0-9]+(?:[eE][+-]?[0-9]+)?[fFlL]?|[0-9]+\.[0-9]*(?:[eE][+-]?[0-9]+)?[fFlL]?/,
});
exports.StringLiteral = (0, chevrotain_1.createToken)({
    name: "StringLiteral",
    pattern: /L?"(?:[^"\\]|\\.)*"/,
});
exports.CharLiteral = (0, chevrotain_1.createToken)({
    name: "CharLiteral",
    pattern: /L?'(?:[^'\\]|\\.)+'/,
});
// ---------------------------------------------------------------------------
// Whitespace and Comments — SKIPPED (no CST nodes produced)
// ---------------------------------------------------------------------------
exports.WhiteSpace = (0, chevrotain_1.createToken)({
    name: "WhiteSpace",
    pattern: /[ \t\r\n]+/,
    group: chevrotain_1.Lexer.SKIPPED,
});
exports.LineComment = (0, chevrotain_1.createToken)({
    name: "LineComment",
    pattern: /\/\/[^\n]*/,
    group: chevrotain_1.Lexer.SKIPPED,
});
exports.BlockComment = (0, chevrotain_1.createToken)({
    name: "BlockComment",
    pattern: /\/\*[\s\S]*?\*\//,
    group: chevrotain_1.Lexer.SKIPPED,
});
// ---------------------------------------------------------------------------
// allTokens — ordering is the lexer's priority list
//
// 1. HashDirective  — before Hash (consumes full directive lines)
// 2. WhiteSpace, LineComment, BlockComment  — SKIPPED, checked early so they
//    don't interfere with other patterns (Chevrotain evaluates in order)
// 3. LibJuno macro tokens  — before Identifier; longer prefixes first
// 4. Keywords — before Identifier (each has longer_alt: Identifier)
// 5. Literals — before Identifier (string/char avoid being split by Identifier)
// 6. Compound punctuators — before their simple-prefix counterparts
// 7. Simple punctuators
// 8. Identifier — last among non-skipped tokens
// ---------------------------------------------------------------------------
exports.allTokens = [
    // Preprocessor (full-line directive — must precede bare Hash)
    exports.HashDirective,
    // Skipped (whitespace + comments)
    exports.WhiteSpace,
    exports.LineComment,
    exports.BlockComment,
    // LibJuno macro tokens (longer/more specific prefixes first)
    exports.JunoModuleRootDeclare,
    exports.JunoModuleDeriveDeclare,
    exports.JunoModuleGetApi,
    exports.JunoModuleResult,
    exports.JunoModuleSuper,
    exports.JunoModuleEmpty,
    exports.JunoModuleRoot,
    exports.JunoModuleDerive,
    exports.JunoModuleDeclare,
    exports.JunoModuleArg,
    exports.JunoModule,
    exports.JunoTraitRoot,
    exports.JunoTraitDerive,
    exports.JunoFailureHandler,
    exports.JunoFailureUserData,
    // Keywords (each has longer_alt: Identifier)
    exports.Static,
    exports.Const,
    exports.Inline,
    exports.Struct,
    exports.Union,
    exports.Enum,
    exports.Typedef,
    exports.Extern,
    exports.Volatile,
    exports.Void,
    exports.Char,
    exports.Short,
    exports.Int,
    exports.Long,
    exports.Float,
    exports.Double,
    exports.Signed,
    exports.Unsigned,
    exports.SizeT,
    exports.Bool,
    exports.If,
    exports.Else,
    exports.For,
    exports.While,
    exports.Do,
    exports.Switch,
    exports.Case,
    exports.Default,
    exports.Break,
    exports.Continue,
    exports.Return,
    exports.Goto,
    exports.Sizeof,
    // Literals
    exports.FloatingLiteral, // before IntegerLiteral (both can start with digits)
    exports.IntegerLiteral,
    exports.StringLiteral,
    exports.CharLiteral,
    // Compound punctuators (before their single-char prefixes)
    exports.Ellipsis, // ... before .
    exports.ArrowOp, // -> before - and >
    exports.PlusPlus, // ++ before +
    exports.MinusMinus, // -- before -
    exports.LShiftAssign, // <<= before << and <
    exports.RShiftAssign, // >>= before >> and >
    exports.LShift, // << before <
    exports.RShift, // >> before >
    exports.LtEq, // <= before <
    exports.GtEq, // >= before >
    exports.EqEq, // == before =
    exports.BangEq, // != before !
    exports.AmpAmp, // && before &
    exports.PipePipe, // || before |
    exports.PlusAssign, // += before +
    exports.MinusAssign, // -= before -
    exports.StarAssign, // *= before *
    exports.SlashAssign, // /= before /
    exports.PercentAssign, // %= before %
    exports.AmpAssign, // &= before &
    exports.PipeAssign, // |= before |
    exports.CaretAssign, // ^= before ^
    // Simple punctuators
    exports.LBrace,
    exports.RBrace,
    exports.LParen,
    exports.RParen,
    exports.LBracket,
    exports.RBracket,
    exports.Semicolon,
    exports.Comma,
    exports.Dot,
    exports.Assign,
    exports.Star,
    exports.Amp,
    exports.Plus,
    exports.Minus,
    exports.Slash,
    exports.Percent,
    exports.Bang,
    exports.Tilde,
    exports.Lt,
    exports.Gt,
    exports.Colon,
    exports.Question,
    exports.Caret,
    exports.Pipe,
    exports.Hash, // bare # — after HashDirective
    // Identifier — lowest priority, after all keywords and macro tokens
    exports.Identifier,
];
// ---------------------------------------------------------------------------
// Lexer instance
// ---------------------------------------------------------------------------
exports.CLexer = new chevrotain_1.Lexer(exports.allTokens);
//# sourceMappingURL=lexer.js.map