// @{"req": ["REQ-VSCODE-003", "REQ-VSCODE-008", "REQ-VSCODE-009", "REQ-VSCODE-014", "REQ-VSCODE-015"]}
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

import { createToken, Lexer } from "chevrotain";

// ---------------------------------------------------------------------------
// Identifier — defined first so keywords can reference it via `longer_alt`
// ---------------------------------------------------------------------------

export const Identifier = createToken({
    name: "Identifier",
    pattern: /[a-zA-Z_][a-zA-Z0-9_]*/,
});

// ---------------------------------------------------------------------------
// Keywords — each uses `longer_alt: Identifier` so that identifiers that
// begin with a keyword string (e.g. "structure") are not split.
// ---------------------------------------------------------------------------

export const Static = createToken({ name: "Static", pattern: /static/, longer_alt: Identifier });
export const Const = createToken({ name: "Const", pattern: /const/, longer_alt: Identifier });
export const Inline = createToken({ name: "Inline", pattern: /inline/, longer_alt: Identifier });
export const Struct = createToken({ name: "Struct", pattern: /struct/, longer_alt: Identifier });
export const Union = createToken({ name: "Union", pattern: /union/, longer_alt: Identifier });
export const Enum = createToken({ name: "Enum", pattern: /enum/, longer_alt: Identifier });
export const Typedef = createToken({ name: "Typedef", pattern: /typedef/, longer_alt: Identifier });
export const Extern = createToken({ name: "Extern", pattern: /extern/, longer_alt: Identifier });
export const Volatile = createToken({ name: "Volatile", pattern: /volatile/, longer_alt: Identifier });
export const Void = createToken({ name: "Void", pattern: /void/, longer_alt: Identifier });
export const Char = createToken({ name: "Char", pattern: /char/, longer_alt: Identifier });
export const Short = createToken({ name: "Short", pattern: /short/, longer_alt: Identifier });
export const Int = createToken({ name: "Int", pattern: /int/, longer_alt: Identifier });
export const Long = createToken({ name: "Long", pattern: /long/, longer_alt: Identifier });
export const Float = createToken({ name: "Float", pattern: /float/, longer_alt: Identifier });
export const Double = createToken({ name: "Double", pattern: /double/, longer_alt: Identifier });
export const Signed = createToken({ name: "Signed", pattern: /signed/, longer_alt: Identifier });
export const Unsigned = createToken({ name: "Unsigned", pattern: /unsigned/, longer_alt: Identifier });
export const SizeT = createToken({ name: "SizeT", pattern: /size_t/, longer_alt: Identifier });
export const Bool = createToken({ name: "Bool", pattern: /_Bool/, longer_alt: Identifier });
export const If = createToken({ name: "If", pattern: /if/, longer_alt: Identifier });
export const Else = createToken({ name: "Else", pattern: /else/, longer_alt: Identifier });
export const For = createToken({ name: "For", pattern: /for/, longer_alt: Identifier });
export const While = createToken({ name: "While", pattern: /while/, longer_alt: Identifier });
export const Do = createToken({ name: "Do", pattern: /do/, longer_alt: Identifier });
export const Switch = createToken({ name: "Switch", pattern: /switch/, longer_alt: Identifier });
export const Case = createToken({ name: "Case", pattern: /case/, longer_alt: Identifier });
export const Default = createToken({ name: "Default", pattern: /default/, longer_alt: Identifier });
export const Break = createToken({ name: "Break", pattern: /break/, longer_alt: Identifier });
export const Continue = createToken({ name: "Continue", pattern: /continue/, longer_alt: Identifier });
export const Return = createToken({ name: "Return", pattern: /return/, longer_alt: Identifier });
export const Goto = createToken({ name: "Goto", pattern: /goto/, longer_alt: Identifier });
export const Sizeof = createToken({ name: "Sizeof", pattern: /sizeof/, longer_alt: Identifier });

// ---------------------------------------------------------------------------
// LibJuno Macro Tokens — defined before Identifier in allTokens.
// Longer/more specific prefixes listed first (defensive ordering).
// \b word-boundary prevents prefix matching (e.g. JUNO_MODULE_ROOT\b will
// NOT match JUNO_MODULE_ROOT_T because _ is a word char, so no boundary).
// ---------------------------------------------------------------------------

export const JunoModuleRootDeclare = createToken({
    name: "JunoModuleRootDeclare",
    pattern: /JUNO_MODULE_ROOT_DECLARE\b/,
});

export const JunoModuleDeriveDeclare = createToken({
    name: "JunoModuleDeriveDeclare",
    pattern: /JUNO_MODULE_DERIVE_DECLARE\b/,
});

export const JunoModuleGetApi = createToken({
    name: "JunoModuleGetApi",
    pattern: /JUNO_MODULE_GET_API\b/,
});

export const JunoModuleResult = createToken({
    name: "JunoModuleResult",
    pattern: /JUNO_MODULE_RESULT\b/,
});

export const JunoModuleOption = createToken({
    name: "JunoModuleOption",
    pattern: /JUNO_MODULE_OPTION\b/,
});

export const JunoModuleSuper = createToken({
    name: "JunoModuleSuper",
    pattern: /JUNO_MODULE_SUPER\b/,
});

export const JunoModuleEmpty = createToken({
    name: "JunoModuleEmpty",
    pattern: /JUNO_MODULE_EMPTY\b/,
});

export const JunoModuleRoot = createToken({
    name: "JunoModuleRoot",
    pattern: /JUNO_MODULE_ROOT\b/,
});

export const JunoModuleDerive = createToken({
    name: "JunoModuleDerive",
    pattern: /JUNO_MODULE_DERIVE\b/,
});

export const JunoModuleDeclare = createToken({
    name: "JunoModuleDeclare",
    pattern: /JUNO_MODULE_DECLARE\b/,
});

export const JunoModuleArg = createToken({
    name: "JunoModuleArg",
    pattern: /JUNO_MODULE_ARG\b/,
});

export const JunoModule = createToken({
    name: "JunoModule",
    pattern: /JUNO_MODULE\b/,
});

export const JunoTraitRoot = createToken({
    name: "JunoTraitRoot",
    pattern: /JUNO_TRAIT_ROOT\b/,
});

export const JunoTraitDerive = createToken({
    name: "JunoTraitDerive",
    pattern: /JUNO_TRAIT_DERIVE\b/,
});

export const JunoFailureHandler = createToken({
    name: "JunoFailureHandler",
    pattern: /JUNO_FAILURE_HANDLER\b|_pfcnFailureHandler\b/,
});

export const JunoFailureUserData = createToken({
    name: "JunoFailureUserData",
    pattern: /JUNO_FAILURE_USER_DATA\b|_pvFailureUserData\b/,
});

// ---------------------------------------------------------------------------
// Preprocessor — HashDirective must precede Hash
// Chevrotain does not allow the ^ anchor or the m/g flags in static patterns.
// Use a custom match function that preserves start-of-line semantics.
// ---------------------------------------------------------------------------

const _hashDirectiveRe = /[ \t]*#[ \t]*(?:define|include|ifdef|ifndef|if|elif|else|endif|pragma|undef|error|warning|line)/;

export const HashDirective = createToken({
    name: "HashDirective",
    pattern: (text: string, startOffset: number): RegExpExecArray | null => {
        // Only match at the very beginning of the text or just after a newline.
        if (startOffset !== 0 && text[startOffset - 1] !== "\n") { return null; }
        const remaining = text.slice(startOffset);
        const match = _hashDirectiveRe.exec(remaining);
        if (!match || match.index !== 0) { return null; }

        // Consume the rest of the directive, including continuation lines (\ + \n).
        let pos = match[0].length;
        while (pos < remaining.length) {
            const ch = remaining[pos];
            if (ch === "\n") {
                // Look back past any trailing whitespace (excluding \n) for a backslash.
                let lookBack = pos - 1;
                while (lookBack >= 0 && (remaining[lookBack] === " " || remaining[lookBack] === "\t" || remaining[lookBack] === "\r")) {
                    lookBack--;
                }
                if (lookBack >= 0 && remaining[lookBack] === "\\") {
                    // Line continuation — consume the newline and keep going.
                    pos++;
                    continue;
                }
                // No continuation — stop here (do not consume the newline).
                break;
            }
            pos++;
        }

        const result = [remaining.slice(0, pos)] as unknown as RegExpExecArray;
        result.index = 0;
        result.input = remaining;
        return result;
    },
    line_breaks: true,
});

// ---------------------------------------------------------------------------
// Punctuators — compound / longer forms listed before shorter prefix forms
// ---------------------------------------------------------------------------

// Multi-character compound operators — defined before their prefixes
export const Ellipsis = createToken({ name: "Ellipsis", pattern: /\.\.\./ });
export const ArrowOp = createToken({ name: "ArrowOp", pattern: /->/ });
export const PlusPlus = createToken({ name: "PlusPlus", pattern: /\+\+/ });
export const MinusMinus = createToken({ name: "MinusMinus", pattern: /--/ });
export const PlusAssign = createToken({ name: "PlusAssign", pattern: /\+=/ });
export const MinusAssign = createToken({ name: "MinusAssign", pattern: /-=/ });
export const StarAssign = createToken({ name: "StarAssign", pattern: /\*=/ });
export const SlashAssign = createToken({ name: "SlashAssign", pattern: /\/=/ });
export const PercentAssign = createToken({ name: "PercentAssign", pattern: /%=/ });
export const AmpAssign = createToken({ name: "AmpAssign", pattern: /&=/ });
export const PipeAssign = createToken({ name: "PipeAssign", pattern: /\|=/ });
export const CaretAssign = createToken({ name: "CaretAssign", pattern: /\^=/ });
export const LShiftAssign = createToken({ name: "LShiftAssign", pattern: /<<=/  });
export const RShiftAssign = createToken({ name: "RShiftAssign", pattern: />>=/  });
export const LShift = createToken({ name: "LShift", pattern: /<</ });
export const RShift = createToken({ name: "RShift", pattern: />>/ });
export const LtEq = createToken({ name: "LtEq", pattern: /<=/ });
export const GtEq = createToken({ name: "GtEq", pattern: />=/ });
export const EqEq = createToken({ name: "EqEq", pattern: /==/ });
export const BangEq = createToken({ name: "BangEq", pattern: /!=/ });
export const AmpAmp = createToken({ name: "AmpAmp", pattern: /&&/ });
export const PipePipe = createToken({ name: "PipePipe", pattern: /\|\|/ });

// Simple single-character operators — after their compound forms
export const LBrace = createToken({ name: "LBrace", pattern: /\{/ });
export const RBrace = createToken({ name: "RBrace", pattern: /\}/ });
export const LParen = createToken({ name: "LParen", pattern: /\(/ });
export const RParen = createToken({ name: "RParen", pattern: /\)/ });
export const LBracket = createToken({ name: "LBracket", pattern: /\[/ });
export const RBracket = createToken({ name: "RBracket", pattern: /\]/ });
export const Semicolon = createToken({ name: "Semicolon", pattern: /;/ });
export const Comma = createToken({ name: "Comma", pattern: /,/ });
export const Dot = createToken({ name: "Dot", pattern: /\./ });
export const Assign = createToken({ name: "Assign", pattern: /=/ });
export const Star = createToken({ name: "Star", pattern: /\*/ });
export const Amp = createToken({ name: "Amp", pattern: /&/ });
export const Plus = createToken({ name: "Plus", pattern: /\+/ });
export const Minus = createToken({ name: "Minus", pattern: /-/ });
export const Slash = createToken({ name: "Slash", pattern: /\// });
export const Percent = createToken({ name: "Percent", pattern: /%/ });
export const Bang = createToken({ name: "Bang", pattern: /!/ });
export const Tilde = createToken({ name: "Tilde", pattern: /~/ });
export const Lt = createToken({ name: "Lt", pattern: /</ });
export const Gt = createToken({ name: "Gt", pattern: />/ });
export const Colon = createToken({ name: "Colon", pattern: /:/ });
export const Question = createToken({ name: "Question", pattern: /\?/ });
export const Caret = createToken({ name: "Caret", pattern: /\^/ });
export const Pipe = createToken({ name: "Pipe", pattern: /\|/ });
export const Hash = createToken({ name: "Hash", pattern: /#/ });

// ---------------------------------------------------------------------------
// Literals
// ---------------------------------------------------------------------------

export const IntegerLiteral = createToken({
    name: "IntegerLiteral",
    pattern: /0[xX][0-9a-fA-F]+[uUlL]*|0[0-7]*[uUlL]*|[1-9][0-9]*[uUlL]*/,
});

export const FloatingLiteral = createToken({
    name: "FloatingLiteral",
    pattern: /[0-9]*\.[0-9]+(?:[eE][+-]?[0-9]+)?[fFlL]?|[0-9]+\.[0-9]*(?:[eE][+-]?[0-9]+)?[fFlL]?/,
});

export const StringLiteral = createToken({
    name: "StringLiteral",
    pattern: /L?"(?:[^"\\]|\\.)*"/,
});

export const CharLiteral = createToken({
    name: "CharLiteral",
    pattern: /L?'(?:[^'\\]|\\.)+'/,
});

// ---------------------------------------------------------------------------
// Whitespace and Comments — SKIPPED (no CST nodes produced)
// ---------------------------------------------------------------------------

export const WhiteSpace = createToken({
    name: "WhiteSpace",
    pattern: /[ \t\r\n]+/,
    group: Lexer.SKIPPED,
});

export const LineComment = createToken({
    name: "LineComment",
    pattern: /\/\/[^\n]*/,
    group: Lexer.SKIPPED,
});

export const BlockComment = createToken({
    name: "BlockComment",
    pattern: /\/\*[\s\S]*?\*\//,
    group: Lexer.SKIPPED,
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

export const allTokens = [
    // Preprocessor (full-line directive — must precede bare Hash)
    HashDirective,

    // Skipped (whitespace + comments)
    WhiteSpace,
    LineComment,
    BlockComment,

    // LibJuno macro tokens (longer/more specific prefixes first)
    JunoModuleRootDeclare,
    JunoModuleDeriveDeclare,
    JunoModuleGetApi,
    JunoModuleResult,
    JunoModuleOption,
    JunoModuleSuper,
    JunoModuleEmpty,
    JunoModuleRoot,
    JunoModuleDerive,
    JunoModuleDeclare,
    JunoModuleArg,
    JunoModule,
    JunoTraitRoot,
    JunoTraitDerive,
    JunoFailureHandler,
    JunoFailureUserData,

    // Keywords (each has longer_alt: Identifier)
    Static,
    Const,
    Inline,
    Struct,
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

    // Literals
    FloatingLiteral,    // before IntegerLiteral (both can start with digits)
    IntegerLiteral,
    StringLiteral,
    CharLiteral,

    // Compound punctuators (before their single-char prefixes)
    Ellipsis,           // ... before .
    ArrowOp,            // -> before - and >
    PlusPlus,           // ++ before +
    MinusMinus,         // -- before -
    LShiftAssign,       // <<= before << and <
    RShiftAssign,       // >>= before >> and >
    LShift,             // << before <
    RShift,             // >> before >
    LtEq,               // <= before <
    GtEq,               // >= before >
    EqEq,               // == before =
    BangEq,             // != before !
    AmpAmp,             // && before &
    PipePipe,           // || before |
    PlusAssign,         // += before +
    MinusAssign,        // -= before -
    StarAssign,         // *= before *
    SlashAssign,        // /= before /
    PercentAssign,      // %= before %
    AmpAssign,          // &= before &
    PipeAssign,         // |= before |
    CaretAssign,        // ^= before ^

    // Simple punctuators
    LBrace,
    RBrace,
    LParen,
    RParen,
    LBracket,
    RBracket,
    Semicolon,
    Comma,
    Dot,
    Assign,
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
    Hash,               // bare # — after HashDirective

    // Identifier — lowest priority, after all keywords and macro tokens
    Identifier,
];

// ---------------------------------------------------------------------------
// Lexer instance
// ---------------------------------------------------------------------------

export const CLexer = new Lexer(allTokens);
