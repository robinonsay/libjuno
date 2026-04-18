# LibJuno VSCode Extension — Software Design Document

**Date:** 2026-04-14  
**Module:** VSCODE  
**Requirements file:** `requirements/vscode-extension/requirements.json`

---

// @{"design": ["REQ-VSCODE-001"]}
## 1. Overview

The LibJuno VSCode Extension assists developers navigating LibJuno-based embedded C projects. LibJuno uses vtable-based dependency injection (DI) — function calls dispatched through `ptApi->Foo(...)` — that standard IDE tooling cannot resolve to their concrete implementations. This extension bridges that gap by building a workspace-wide navigation index and wiring it into VSCode's native Go to Definition system.

The extension also exposes its resolution capabilities to AI agent platforms via an embedded MCP (Model Context Protocol) server, enabling AI-assisted workflows to benefit from the same vtable resolution as manual developer workflows.

### Requirements in Scope

| ID | Title |
|----|-------|
| REQ-VSCODE-001 | VSCode Extension |
| REQ-VSCODE-002 | Vtable-Aware Go to Definition |
| REQ-VSCODE-003 | LibJuno API Pattern Recognition |
| REQ-VSCODE-004 | Graceful Error on Missing Implementation |
| REQ-VSCODE-005 | Single Implementation Navigation |
| REQ-VSCODE-006 | Multiple Implementation Selection |
| REQ-VSCODE-007 | Native Go to Definition Integration |
| REQ-VSCODE-008 | Module Root API Discovery |
| REQ-VSCODE-009 | Module Derivation Chain Resolution |
| REQ-VSCODE-010 | Designated Initializer Recognition |
| REQ-VSCODE-011 | Direct Assignment Recognition |
| REQ-VSCODE-012 | Positional Initializer Recognition |
| REQ-VSCODE-013 | Informative Non-Intrusive Error |
| REQ-VSCODE-014 | Trait Root API Discovery |
| REQ-VSCODE-015 | Trait Derivation Chain Resolution |
| REQ-VSCODE-016 | Failure Handler Navigation |
| REQ-VSCODE-017 | AI Agent Accessibility |
| REQ-VSCODE-018 | AI Vtable Resolution Access |
| REQ-VSCODE-019 | AI Failure Handler Resolution Access |
| REQ-VSCODE-020 | Platform-Agnostic AI Interface |
| REQ-VSCODE-021 | C and C++ File Type Support |
| REQ-VSCODE-022 | FAIL Macro Failure Handler Navigation |
| REQ-VSCODE-023 | JUNO_FAIL Direct Handler Resolution |
| REQ-VSCODE-024 | JUNO_FAIL_MODULE Handler Resolution |
| REQ-VSCODE-025 | JUNO_FAIL_ROOT Handler Resolution |
| REQ-VSCODE-026 | JUNO_ASSERT_EXISTS_MODULE Handler Resolution |
| REQ-VSCODE-027 | Vtable Resolution Trace View |
| REQ-VSCODE-028 | Trace View Activation via Keyboard |
| REQ-VSCODE-029 | Trace View Activation via Command Palette |
| REQ-VSCODE-030 | Trace View Call Site Node |
| REQ-VSCODE-031 | Trace View Composition Root Node |
| REQ-VSCODE-032 | Trace View Implementation Node |

---

// @{"design": ["REQ-VSCODE-001"]}
## 2. Design Approach

### 2.1 Technology Stack

- **Language:** TypeScript, targeting the VSCode Extension API
- **Runtime:** Node.js (bundled with VSCode)
- **C parsing:** Chevrotain-based context-free grammar parser. Chevrotain is a zero-runtime-dependency parser generator for TypeScript that runs natively in Node.js. It produces a concrete syntax tree (CST) that visitor methods can walk to extract all index data in a single pass. Chevrotain includes built-in error recovery support, and its pure TypeScript implementation allows LibJuno macros to be treated as first-class grammar constructs without requiring a build system or native binaries.
- **Navigation index:** In-memory data structures populated at activation and maintained incrementally via file watchers.
- **Persistence:** JSON file cache at `.libjuno/navigation-cache.json` in the workspace root. Prevents full re-scan on every activation.
- **AI interface:** Embedded MCP (Model Context Protocol) server. MCP is platform-agnostic and supported by GitHub Copilot, Claude, and other AI platforms, satisfying REQ-VSCODE-020.

### 2.2 Alternatives Considered

| Alternative | Rejected Because |
|-------------|-----------------|
| Regex-based text scanning | Fragile on edge cases: Allman-style braces, multiline parameter lists, and the multiple macro forms of LibJuno constructs. Required 11 separate patterns (P1–P11) plus a 5-strategy call-site system to approximate what a grammar handles uniformly. Could not track local variable types without backward regex scans spanning up to 200 lines. |
| Full C AST parser (libclang via Node.js FFI) | Requires native binaries and FFI bindings — not portable across macOS, Linux, and Windows without a build step. Cannot expand `JUNO_MODULE_ROOT` and related macros without the full build system and include paths. Chevrotain avoids both issues: it is pure TypeScript and handles LibJuno macros as first-class grammar constructs. |
| LSP extension (C language server extension) | LSP hooks cannot override symbol resolution for macro-generated struct fields. |
| Per-request file scan (no index) | Unacceptably slow for large workspaces; each Go to Definition would freeze the editor. |

---

// @{"design": ["REQ-VSCODE-001", "REQ-VSCODE-002", "REQ-VSCODE-003", "REQ-VSCODE-004", "REQ-VSCODE-016", "REQ-VSCODE-017", "REQ-VSCODE-027"]}
## 3. Architecture and Component Design

The extension is composed of eight components. Each component has a single responsibility and communicates with adjacent components through defined interfaces.

```
┌─────────────────────────────────────────────────────────────────┐
│                      VSCode Extension Host                       │
│                                                                  │
│  ┌─────────────────────────┐   ┌──────────────────────────────┐ │
│  │  C Parser (Chevrotain)  │◄──│     Workspace Indexer        │ │
│  │  ┌───────┐ ┌────────┐   │   └──────────────┬───────────────┘ │
│  │  │ Lexer │ │ Parser │   │                  │                  │
│  │  └───────┘ └────────┘   │   ┌──────────────▼───────────────┐ │
│  │  ┌─────────────────┐    │   │       Cache Manager          │ │
│  │  │  CST Visitor    │    │   └─────────────────────────────┘  │
│  │  └─────────────────┘    │                  │                  │
│  └─────────────────────────┘   ┌──────────────▼───────────────┐ │
│                                │     Navigation Index         │ │
│  ┌───────────────────┐         └──────────────▲───────────────┘ │
│  │  Vtable Resolver  │◄────────────────────────┘                │
│  └────────┬──────────┘                                          │
│           │                                                      │
│  ┌────────▼──────────┐      ┌──────────────────────────────┐    │
│  │  Failure Handler  │      │  Failure Handler Resolver    │    │
│  │    Resolver       │      └──────────────────────────────┘    │
│  └────────┬──────────┘                                          │
│           │                                                      │
│  ┌────────▼──────────────────────────────────────────────────┐  │
│  │               VSCode Integration Layer                     │  │
│  │  (DefinitionProvider, QuickPick, Commands, StatusBar,     │  │
│  │   VtableTraceProvider)                                     │  │
│  └────────────────────────────────────────┬───────────────────┘  │
│                                           │                      │
│  ┌────────────────────────────────────────▼───────────────────┐  │
│  │                    MCP Server                               │  │
│  │         (resolve_vtable_call, resolve_failure_handler)     │  │
│  └────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

### 3.1 C Parser (Chevrotain)

Responsible for extracting structured data from C source text using a Chevrotain-based context-free grammar. Takes a file path and its text content as input; emits parsed records consumed by the Workspace Indexer.

The parser operates in a single pass per file: the lexer tokenizes the input, the parser builds a CST, and the CST visitor walks the tree to collect all records. It does not maintain state between files.

**Interface:**

```typescript
interface ParsedFile {
  filePath: string;
  moduleRoots:           ModuleRootRecord[];
  traitRoots:            TraitRootRecord[];
  derivations:           DerivationRecord[];
  apiStructDefinitions:  ApiStructRecord[];
  vtableAssignments:     VtableAssignmentRecord[];
  failureHandlerAssigns: FailureHandlerRecord[];
  apiCallSites:          ApiCallSiteRecord[];
  localTypeInfo:         LocalTypeInfo;
}

interface ModuleRootRecord {
  rootType: string;   // e.g. "JUNO_DS_HEAP_ROOT_T"
  apiType:  string;   // e.g. "JUNO_DS_HEAP_API_T"
  file:     string;
  line:     number;
}

interface TraitRootRecord {
  rootType: string;
  apiType:  string;
  file:     string;
  line:     number;
}

interface DerivationRecord {
  derivedType: string;  // e.g. "JUNO_DS_HEAP_IMPL_T"
  rootType:    string;  // e.g. "JUNO_DS_HEAP_ROOT_T"
  file:        string;
  line:        number;
}

interface ApiStructRecord {
  apiType:    string;          // e.g. "JUNO_DS_HEAP_API_T"
  fields:     string[];        // ordered list: ["Insert", "Heapify", "Pop"]
  file:       string;
  line:       number;
}

interface VtableAssignmentRecord {
  apiType:      string;   // e.g. "JUNO_DS_HEAP_API_T"
  field:        string;   // e.g. "Insert"
  functionName: string;   // e.g. "JunoDs_Heap_Insert"
  file:         string;
  line:         number;
}

interface FailureHandlerRecord {
  rootType:     string;   // variable's type resolved via index (may be empty at parse time)
  functionName: string;
  file:         string;
  line:         number;
}

interface ApiCallSiteRecord {
  variableName: string;   // e.g. "ptHeap"
  fieldName:    string;   // e.g. "Insert"
  file:         string;
  line:         number;
  column:       number;
}
```

#### 3.1.1 Lexer (Token Definitions)

The Chevrotain lexer is defined as an ordered list of token types. Token priority is determined by list position: tokens defined earlier take priority over later tokens when the same input position can match multiple patterns. The two critical ordering rules are:

1. **LibJuno macro tokens must appear BEFORE the generic `Identifier` token.** Without this, `JUNO_MODULE_ROOT` would be consumed as an ordinary identifier.
2. **Keyword tokens must appear BEFORE `Identifier`**, using Chevrotain's `longer_alt: Identifier` option so that `structure` is not tokenized as `struct` + `ure`.

Tokens are grouped into the following categories:

---

**Keywords** (use `longer_alt: Identifier` on each)

| Token Name | Pattern |
|------------|---------|
| `Static` | `/static/` |
| `Const` | `/const/` |
| `Inline` | `/inline/` |
| `Struct` | `/struct/` |
| `Union` | `/union/` |
| `Enum` | `/enum/` |
| `Typedef` | `/typedef/` |
| `Extern` | `/extern/` |
| `Volatile` | `/volatile/` |
| `Void` | `/void/` |
| `Char` | `/char/` |
| `Short` | `/short/` |
| `Int` | `/int/` |
| `Long` | `/long/` |
| `Float` | `/float/` |
| `Double` | `/double/` |
| `Signed` | `/signed/` |
| `Unsigned` | `/unsigned/` |
| `SizeT` | `/size_t/` |
| `Bool` | `/_Bool/` |
| `If` | `/if/` |
| `Else` | `/else/` |
| `For` | `/for/` |
| `While` | `/while/` |
| `Do` | `/do/` |
| `Switch` | `/switch/` |
| `Case` | `/case/` |
| `Default` | `/default/` |
| `Break` | `/break/` |
| `Continue` | `/continue/` |
| `Return` | `/return/` |
| `Goto` | `/goto/` |
| `Sizeof` | `/sizeof/` |

Example Chevrotain definition:
```typescript
const Struct = createToken({ name: "Struct", pattern: /struct/, longer_alt: Identifier });
```

---

**LibJuno Macro Tokens** (higher priority than `Identifier` — defined before it in the token list)

All patterns use `\b` (word boundary) to prevent prefix matching. For example, `/JUNO_MODULE_ROOT\b/` matches `JUNO_MODULE_ROOT` but NOT `JUNO_MODULE_ROOT_T` (because `_` is a word character, so no boundary exists between `T` and `_`). This makes each token pattern self-disambiguating without relying on `longer_alt`.

Tokens with the same prefix are safe: `/JUNO_MODULE\b/` does NOT match `JUNO_MODULE_ROOT` because `_` follows `E` (both word characters, no boundary). Only standalone `JUNO_MODULE` followed by a non-word character (whitespace, parenthesis, etc.) matches.

For macros that are member-name aliases (`JUNO_FAILURE_HANDLER` → `_pfcnFailureHandler`, `JUNO_FAILURE_USER_DATA` → `_pvFailureUserData`), the pattern uses alternation to match BOTH the macro name and the underlying member name. Since the parser operates on raw (unparsed) source text, both forms appear in real code and must produce the same token type.

| Token Name | Pattern | Semantic Note |
|------------|---------|---------------|
| `JunoModuleRootDeclare` | `/JUNO_MODULE_ROOT_DECLARE\b/` | Forward-declare module root struct |
| `JunoModuleDeriveDeclare` | `/JUNO_MODULE_DERIVE_DECLARE\b/` | Forward-declare derived module struct |
| `JunoModuleGetApi` | `/JUNO_MODULE_GET_API\b/` | Cast macro to retrieve API pointer |
| `JunoModuleResult` | `/JUNO_MODULE_RESULT\b/` | Result type typedef macro |
| `JunoModuleSuper` | `/JUNO_MODULE_SUPER\b/` | Embedded root member alias (resolves to `tRoot`) |
| `JunoModuleEmpty` | `/JUNO_MODULE_EMPTY\b/` | Empty member list placeholder |
| `JunoModuleRoot` | `/JUNO_MODULE_ROOT\b/` | Module root struct definition macro |
| `JunoModuleDerive` | `/JUNO_MODULE_DERIVE\b/` | Module derivation macro |
| `JunoModuleDeclare` | `/JUNO_MODULE_DECLARE\b/` | Forward-declare module union |
| `JunoModuleArg` | `/JUNO_MODULE_ARG\b/` | Variadic pass-through helper |
| `JunoModule` | `/JUNO_MODULE\b/` | Module union definition macro |
| `JunoTraitRoot` | `/JUNO_TRAIT_ROOT\b/` | Trait root struct definition macro |
| `JunoTraitDerive` | `/JUNO_TRAIT_DERIVE\b/` | Trait derivation macro |
| `JunoFailureHandler` | `/JUNO_FAILURE_HANDLER\b\|_pfcnFailureHandler\b/` | Failure handler member — matches both macro and underlying name |
| `JunoFailureUserData` | `/JUNO_FAILURE_USER_DATA\b\|_pvFailureUserData\b/` | Failure user data member — matches both macro and underlying name |

> **Ordering note:** Tokens with longer literal prefixes (e.g., `JUNO_MODULE_ROOT_DECLARE`) are listed before shorter ones (e.g., `JUNO_MODULE_ROOT`, then `JUNO_MODULE`). With `\b` patterns this ordering is not strictly required for correctness (each pattern is self-disambiguating), but it follows the defensive convention of listing more specific tokens first. All LibJuno macro tokens must appear before `Identifier` in the token array so that same-length matches resolve in favor of the macro token.

Example Chevrotain definition:
```typescript
const JunoModuleRoot = createToken({ name: "JunoModuleRoot", pattern: /JUNO_MODULE_ROOT\b/ });
const JunoFailureHandler = createToken({ name: "JunoFailureHandler", pattern: /JUNO_FAILURE_HANDLER\b|_pfcnFailureHandler\b/ });
```

---

**Punctuators**

| Token Name | Pattern | Notes |
|------------|---------|-------|
| `Ellipsis` | `/\.\.\./` | Must precede `Dot` |
| `ArrowOp` | `/\->/` | `->` |
| `LBrace` | `/\{/` | |
| `RBrace` | `/\}/` | |
| `LParen` | `/\(/` | |
| `RParen` | `/\)/` | |
| `LBracket` | `/\[/` | |
| `RBracket` | `/\]/` | |
| `Semicolon` | `/;/` | |
| `Comma` | `/,/` | |
| `Dot` | `/\./` | |
| `Assign` | `/=/` | After compound assigns |
| `Star` | `/\*/` | |
| `Amp` | `/&/` | |
| `Plus` | `/\+/` | After `++`, `+=` |
| `Minus` | `/-/` | After `--`, `-=` |
| `Slash` | `/\//` | |
| `Percent` | `/%/` | |
| `Bang` | `/!/` | After `!=` |
| `Tilde` | `/~/` | |
| `Lt` | `/</` | After `<=`, `<<`, `<<=` |
| `Gt` | `/>/` | After `>=`, `>>`, `>>=` |
| `Colon` | `/:/` | |
| `Question` | `/\?/` | |
| `Caret` | `/\^/` | |
| `Pipe` | `/\|/` | After `||`, `|=` |
| `PlusPlus` | `/\+\+/` | Before `Plus` |
| `MinusMinus` | `/--/` | Before `Minus` |
| `PlusAssign` | `/\+=/` | Before `Plus` |
| `MinusAssign` | `/-=/` | |
| `StarAssign` | `/\*=/` | |
| `SlashAssign` | `/\/=/` | |
| `PercentAssign` | `/%=/` | |
| `AmpAssign` | `/&=/` | |
| `PipeAssign` | `/\|=/` | |
| `CaretAssign` | `/\^=/` | |
| `LShiftAssign` | `/<<=/` | Before `LShift` |
| `RShiftAssign` | `/>>=/` | Before `RShift` |
| `LShift` | `/<</` | Before `Lt` |
| `RShift` | `/>>/` | Before `Gt` |
| `LtEq` | `/<=/` | Before `Lt` |
| `GtEq` | `/>=/` | Before `Gt` |
| `EqEq` | `/==/` | Before `Assign` |
| `BangEq` | `/!=/` | Before `Bang` |
| `AmpAmp` | `/&&/` | Before `Amp` |
| `PipePipe` | `/\|\|/` | Before `Pipe` |
| `Hash` | `/#/` | For preprocessor — see below |

---

**Literals**

| Token Name | Pattern | Notes |
|------------|---------|-------|
| `IntegerLiteral` | `/0[xX][0-9a-fA-F]+[uUlL]*\|0[0-7]*[uUlL]*\|[1-9][0-9]*[uUlL]*/` | Hex, octal, decimal |
| `FloatingLiteral` | `/[0-9]*\.[0-9]+(?:[eE][+-]?[0-9]+)?[fFlL]?\|[0-9]+\.[0-9]*(?:[eE][+-]?[0-9]+)?[fFlL]?/` | |
| `StringLiteral` | `/L?"(?:[^"\\]|\\.)*"/` | Wide and narrow strings |
| `CharLiteral` | `/L?'(?:[^'\\]|\\.)+'/` | Wide and narrow chars |

---

**Identifier**

| Token Name | Pattern | Notes |
|------------|---------|-------|
| `Identifier` | `/[a-zA-Z_][a-zA-Z0-9_]*/` | Lowest priority — after all keywords and LibJuno macro tokens |

---

**Preprocessor**

| Token Name | Pattern | Notes |
|------------|---------|-------|
| `HashDirective` | `/^[ \t]*#[ \t]*(?:define\|include\|ifdef\|ifndef\|if\|elif\|else\|endif\|pragma\|undef\|error\|warning\|line)[^\n]*/m` | Captures the entire directive line as a single token. The content after the directive keyword is the token payload, available to the CST visitor for further processing. Must use multiline mode so `^` anchors to the start of any line. |

> **Note:** `HashDirective` must be placed near the top of the token list (before `Hash`) so that lines beginning with `#` followed by a known directive keyword are consumed as a single token rather than as a bare `#` punctuator.

---

**Whitespace and Comments** (skipped — do not produce CST nodes)

| Token Name | Pattern | Notes |
|------------|---------|-------|
| `WhiteSpace` | `/[ \t\r\n]+/` | `{ group: Lexer.SKIPPED }` |
| `LineComment` | `/\/\/[^\n]*/` | `{ group: Lexer.SKIPPED }` |
| `BlockComment` | `/\/\*[\s\S]*?\*\//` | `{ group: Lexer.SKIPPED }` |

> **Line number tracking:** Although whitespace and comments are skipped (no CST nodes produced), Chevrotain uses the token stream's character offsets to compute line and column numbers for every non-skipped token. This provides accurate `line`/`column` values throughout the visitor without additional bookkeeping.

---

#### 3.1.2 Grammar (Parser Productions)

The grammar is implemented as a Chevrotain `CstParser` subclass (`CParser`). Each production rule is a method decorated with Chevrotain's DSL (`this.RULE`, `this.CONSUME`, `this.SUBRULE`, `this.OR`, `this.MANY`, `this.OPTION`). The notation below is conceptual EBNF; the `?` suffix means optional (`this.OPTION`), `*` means zero or more (`this.MANY`), `+` means one or more (`this.AT_LEAST_ONE`), and `|` means alternatives (`this.OR`).

---

**Top-Level**

```
translationUnit
  → ( externalDeclaration | preprocessorDirective )*

externalDeclaration
  → functionDefinition
  | declaration
  | junoStandaloneDeclaration
```

`externalDeclaration` uses `{ recoveryEnabled: true }` (see Error Recovery below).

---

**Declarations**

```
declaration
  → declarationSpecifiers initDeclaratorList? ';'

declarationSpecifiers
  → ( storageClassSpecifier | typeQualifier | typeSpecifier )+

storageClassSpecifier
  → 'static' | 'typedef' | 'extern'

typeQualifier
  → 'const' | 'volatile' | 'inline'

typeSpecifier
  → primitiveType
  | structOrUnionSpecifier
  | enumSpecifier
  | Identifier

primitiveType
  → 'void' | 'char' | 'short' | 'int' | 'long' | 'float' | 'double'
  | 'signed' | 'unsigned' | 'size_t' | '_Bool'

initDeclaratorList
  → initDeclarator ( ',' initDeclarator )*

initDeclarator
  → declarator ( '=' initializer )?

declarator
  → pointer? directDeclarator

pointer
  → '*' typeQualifier* pointer?

directDeclarator
  → ( Identifier | '(' declarator ')' )
    ( '[' expression? ']'
    | '(' parameterTypeList ')'
    | '(' identifierList? ')'
    )*

parameterTypeList
  → parameterList ( ',' '...' )?

parameterList
  → parameterDeclaration ( ',' parameterDeclaration )*

parameterDeclaration
  → declarationSpecifiers ( declarator | abstractDeclarator )?

abstractDeclarator
  → pointer? abstractDirectDeclarator?

abstractDirectDeclarator
  → ( '(' abstractDeclarator ')' )?
    ( '[' expression? ']' | '(' parameterTypeList? ')' )*

identifierList
  → Identifier ( ',' Identifier )*
```

---

**Struct and Union**

```
structOrUnionSpecifier
  → ( 'struct' | 'union' )
    ( Identifier? '{' structDeclarationList '}'
    | Identifier junoMacroInvocation?
    )

structDeclarationList
  → structDeclaration+

structDeclaration
  → specifierQualifierList structDeclaratorList ';'
  | junoMacroInvocation ';'?

specifierQualifierList
  → ( typeSpecifier | typeQualifier )+

structDeclaratorList
  → structDeclarator ( ',' structDeclarator )*

structDeclarator
  → declarator ( ':' constantExpression )?
```

**Function pointer fields in struct bodies:**

The standard `structDeclaration` production naturally handles function pointer fields. For example:
```c
JUNO_STATUS_T (*Insert)(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tValue);
```
This parses as `specifierQualifierList` (`JUNO_STATUS_T`) followed by a `structDeclarator` whose `declarator` has a `directDeclarator` of the form `'(' '*' Identifier ')' '(' parameterTypeList ')'`. The field name `Insert` is the `Identifier` inside the parenthesized group.

---

**Enum**

```
enumSpecifier
  → 'enum' ( Identifier? '{' enumeratorList ','? '}' | Identifier )

enumeratorList
  → enumerator ( ',' enumerator )*

enumerator
  → Identifier ( '=' constantExpression )?
```

---

**LibJuno Macro Productions**

These are critical first-class grammar constructs. They appear inside struct definitions and disambiguation requires the macro tokens defined in §3.1.1.

```
junoMacroInvocation
  → junoModuleRootMacro
  | junoModuleDeriveMacro
  | junoTraitRootMacro
  | junoTraitDeriveMacro
  | junoModuleMacro

junoModuleRootMacro
  → JUNO_MODULE_ROOT '(' Identifier ',' macroBodyTokens ')'

junoModuleDeriveMacro
  → JUNO_MODULE_DERIVE '(' Identifier ',' macroBodyTokens ')'

junoTraitRootMacro
  → JUNO_TRAIT_ROOT '(' Identifier ',' macroBodyTokens ')'

junoTraitDeriveMacro
  → JUNO_TRAIT_DERIVE '(' Identifier ',' macroBodyTokens ')'

junoModuleMacro
  → JUNO_MODULE '(' Identifier ',' Identifier ',' macroBodyTokens ')'

macroBodyTokens
  → token*   (balanced — consumes tokens until the unmatched closing ')')
```

**Critical pattern:** In LibJuno C headers, the struct is defined as:
```c
struct JUNO_DS_HEAP_ROOT_TAG JUNO_MODULE_ROOT(JUNO_DS_HEAP_API_T,
    const JUNO_DS_HEAP_POINTER_API_T *ptHeapPointerApi;
    ...
);
```
The grammar handles this as `'struct' Identifier junoMacroInvocation` within `structOrUnionSpecifier`. The struct tag (`JUNO_DS_HEAP_ROOT_TAG`) becomes the type name (with `_TAG` → `_T` conversion performed by the visitor), and the macro's first `Identifier` argument is the API type.

`macroBodyTokens` consumes tokens greedily, tracking nested parenthesis depth to stop at the correct unmatched `)`. This allows the macro body to contain any valid C tokens (including nested parentheses appearing in function pointer types declared inside the macro body).

```
junoStandaloneDeclaration
  → ( JunoModuleDeclare | JunoModuleRootDeclare | JunoModuleDeriveDeclare ) '(' Identifier ')' ';'?
  | JunoModuleResult '(' Identifier ',' Identifier ')' ';'?
```

These are file-scope macro invocations that expand to typedef declarations. The parser recognizes them as first-class constructs rather than relying on error recovery.

---

**Function Definitions**

```
functionDefinition
  → declarationSpecifiers declarator compoundStatement
```

This production naturally handles both K&R-style (`{` on the same line as `)`) and Allman-style (`{` on the next line) brace placement, because the grammar is whitespace-insensitive: `declarator` ends at `)` and `compoundStatement` begins at the next `{`, regardless of intervening newlines. This eliminates the two-pass workaround required by the previous regex approach.

`static` in `declarationSpecifiers` indicates file-scoped linkage and is captured by the visitor.

**Disambiguation from declarations:** A `declaration` ends with `;`. A `functionDefinition` ends with `compoundStatement` (which ends with `}`). The Chevrotain parser uses LL(k) lookahead to distinguish them: if, after parsing `declarationSpecifiers declarator`, the next token is `{`, it is a function definition; if the next token is `;` or `=` or `,`, it is a declaration. Chevrotain's `BACKTRACK` or a gated alternative can be used if lookahead is insufficient.

---

**Statements** (minimal — sufficient for local variable tracking within function bodies)

```
compoundStatement
  → '{' ( declaration | statement )* '}'

statement
  → expressionStatement
  | compoundStatement
  | selectionStatement
  | iterationStatement
  | jumpStatement
  | labeledStatement

expressionStatement
  → expression? ';'

selectionStatement
  → 'if' '(' expression ')' statement ( 'else' statement )?
  | 'switch' '(' expression ')' statement

iterationStatement
  → 'while' '(' expression ')' statement
  | 'do' statement 'while' '(' expression ')' ';'
  | 'for' '(' ( declaration | expressionStatement ) expressionStatement expression? ')' statement

jumpStatement
  → ( 'return' expression? | 'break' | 'continue' | 'goto' Identifier ) ';'

labeledStatement
  → ( Identifier | 'case' constantExpression | 'default' ) ':' statement
```

---

**Expressions**

The expression grammar follows standard C11 operator precedence. The key production for vtable call resolution is `postfixExpression`, which captures the full `->` and `.` access chain naturally.

```
expression
  → assignmentExpression ( ',' assignmentExpression )*

assignmentExpression
  → conditionalExpression ( assignmentOperator assignmentExpression )?

assignmentOperator
  → '=' | '+=' | '-=' | '*=' | '/=' | '%=' | '&=' | '|=' | '^=' | '<<=' | '>>='

conditionalExpression
  → logicalOrExpression ( '?' expression ':' conditionalExpression )?

logicalOrExpression  → logicalAndExpression  ( '||' logicalAndExpression  )*
logicalAndExpression → bitwiseOrExpression   ( '&&' bitwiseOrExpression   )*
bitwiseOrExpression  → bitwiseXorExpression  ( '|'  bitwiseXorExpression  )*
bitwiseXorExpression → bitwiseAndExpression  ( '^'  bitwiseAndExpression  )*
bitwiseAndExpression → equalityExpression    ( '&'  equalityExpression    )*
equalityExpression   → relationalExpression  ( ( '==' | '!=' ) relationalExpression )*
relationalExpression → shiftExpression       ( ( '<' | '>' | '<=' | '>=' ) shiftExpression )*
shiftExpression      → additiveExpression    ( ( '<<' | '>>' ) additiveExpression )*
additiveExpression   → multiplicativeExpression ( ( '+' | '-' ) multiplicativeExpression )*
multiplicativeExpression → castExpression    ( ( '*' | '/' | '%' ) castExpression )*

castExpression
  → '(' typeName ')' castExpression
  | unaryExpression

unaryExpression
  → ( '++' | '--' | '&' | '*' | '+' | '-' | '~' | '!' ) unaryExpression
  | 'sizeof' ( '(' typeName ')' | unaryExpression )
  | postfixExpression

memberIdentifier
  → Identifier
  | JunoModuleSuper
  | JunoFailureHandler
  | JunoFailureUserData

postfixExpression
  → primaryExpression
    ( '[' expression ']'
    | '(' argumentExpressionList? ')'
    | '.' memberIdentifier
    | '->' memberIdentifier
    | '++'
    | '--'
    )*

argumentExpressionList
  → assignmentExpression ( ',' assignmentExpression )*

primaryExpression
  → Identifier
  | IntegerLiteral
  | FloatingLiteral
  | StringLiteral
  | CharLiteral
  | '(' expression ')'
  | junoModuleGetApiMacro

junoModuleGetApiMacro
  → JUNO_MODULE_GET_API '(' expression ',' typeSpecifier ')'
```

**Key observation for vtable call resolution:** The chain of `->` and `.` accesses in `postfixExpression` is naturally captured in the CST as a flat list of suffixes. The chain-walk algorithm in §5.1 iterates this suffix list left-to-right, resolving the type at each step. Patterns such as `ptTime->ptApi->Now(ptTime)`, `tReturn.ptApi->Copy(tReturn, tResult.tOk)`, and `JUNO_MODULE_GET_API(ptModule, ROOT_T)->Field(...)` are all instances of `postfixExpression` with different suffix sequences — the algorithm handles them uniformly without strategy enumeration.

---

**Initializers**

```
initializer
  → assignmentExpression
  | '{' initializerList ','? '}'

initializerList
  → ( designation? initializer ) ( ',' designation? initializer )*

designation
  → designator+ '='

designator
  → '[' constantExpression ']'
  | '.' Identifier

constantExpression
  → conditionalExpression
```

---

**Type Names** (used in casts, `sizeof`, and abstract declarators)

```
typeName
  → specifierQualifierList abstractDeclarator?
```

---

**Preprocessor**

```
preprocessorDirective
  → HashDirective
```

The lexer captures the entire directive line as a single `HashDirective` token. The visitor inspects the token payload to handle `#define`, `#ifdef`/`#ifndef`/`#endif`, and `#include` directives (see `visitPreprocessorDirective` in §3.1.3).

---

**Error Recovery**

Each `externalDeclaration` rule is registered with `{ recoveryEnabled: true }`:

```typescript
this.RULE("externalDeclaration", () => {
  // ... grammar body ...
}, { recoveryEnabled: true });
```

On a parse error within a `declaration` or `functionDefinition`:
- The parser skips tokens until it finds a `;` or a `}` at the appropriate brace nesting level (nesting depth 0 relative to the current `externalDeclaration`).
- Parsing then resumes at the next `externalDeclaration`.

Inside `compoundStatement` bodies, error recovery skips to the next `;` or `}`.

This ensures that malformed constructs — inline assembly blocks, unrecognized compiler extensions, complex macro forms that fall outside the grammar — cause only a local failure. The parser continues and correctly indexes the remainder of the file.

**Out-of-scope C features:** The following are intentionally not handled and will trigger error recovery if encountered: `_Generic`, K&R-style function definitions (identifier-list parameter declarations), complex nested designated initializers beyond the grammar above, and bit-fields beyond `declarator ':' constantExpression`.

---

#### 3.1.3 CST Visitor

The `IndexBuildingVisitor` extends Chevrotain's generated CST visitor base class. It walks the CST produced by the parser and populates the `ParsedFile` output record. Seven visitor methods replace the previous regex-based pattern system.

---

**1. `visitStructDefinition(ctx)`** — Replaces P1, P2, P3, P4, P5

Invoked for each `structOrUnionSpecifier` CST node. Dispatches based on which child node is present:

- **`struct TAG JUNO_MODULE_ROOT(API_T, ...)`** (ctx contains `junoModuleRootMacro`):
  - rootType = TAG with `_TAG` → `_T` suffix substitution
  - apiType = first `Identifier` argument of the macro
  - Emit `ModuleRootRecord` → add to `moduleRoots`

- **`struct TAG JUNO_MODULE_DERIVE(ROOT_T, ...)`** (ctx contains `junoModuleDeriveMacro`):
  - derivedType = TAG with `_TAG` → `_T`
  - rootType = first `Identifier` argument
  - Emit `DerivationRecord` → add to `derivationChain`

- **`struct TAG JUNO_TRAIT_ROOT(API_T, ...)`** (ctx contains `junoTraitRootMacro`):
  - Same semantics as `JUNO_MODULE_ROOT`
  - Emit `TraitRootRecord` → add to `traitRoots`

- **`struct TAG JUNO_TRAIT_DERIVE(ROOT_T, ...)`** (ctx contains `junoTraitDeriveMacro`):
  - Same semantics as `JUNO_MODULE_DERIVE`
  - Emit `DerivationRecord` → add to `derivationChain`

- **`struct TAG { ... }` where TAG ends in `_API_TAG`**:
  - apiType = TAG with `_API_TAG` → `_API_T`
  - Walk `structDeclarationList` to extract function pointer fields in document order:
    - For each `structDeclarator` whose `directDeclarator` has the form `'(' '*' Identifier ')' '(' parameterTypeList ')'`:
      - field name = the `Identifier` inside the parenthesized group
  - Emit `ApiStructRecord` → add to `apiStructFields`

- **For ALL struct bodies** (any TAG): walk member declarations and record members whose declared type ends in `_API_T`:
  - memberName → API type → add to `apiMemberRegistry`
  - This covers `ROOT_TAG`, `IMPL_TAG`, and all other struct kinds in a single pass.

- **`JUNO_MODULE_SUPER` resolution:** When the visitor encounters a `JunoModuleSuper` token as a member name in any struct body, it treats it as equivalent to the literal member `tRoot`. This is consistent with the macro definition `#define JUNO_MODULE_SUPER tRoot`. The chain-walk algorithm (§5.1, Step 4b) applies the same equivalence at query time.

---

**2. `visitVtableDeclaration(ctx)`** — Replaces P6, P7, P8

Handles three vtable assignment forms:

- **Designated initializer (P6 equivalent):** Matches a `declaration` of the form `(static)? const _API_T varName = { designation? initializer ... };`.
  - The `initializerList` contains `designation` nodes (`.Field = FuncName`).
  - For each designator: field = designator identifier, functionName = initializer identifier.
  - Emit `VtableAssignmentRecord` for each pair → add to `vtableAssignments`.

- **Positional initializer (P8 equivalent):** Same declaration shape, but `initializerList` contains no `designation` nodes (all initializers are bare expressions).
  - Look up the field order from `apiStructFields` for the declared API type.
  - If the field order is available (API struct was parsed earlier in the same file): zip field names with initializer expressions in document order → emit `VtableAssignmentRecord` for each pair.
  - If not yet available (API struct defined in another file): defer — record the positional initializer and retry after all files in the workspace have been indexed (cross-file deferred resolution, same as before but handled inline for same-file cases).

- **Direct assignment (P7 equivalent):** Matches `expressionStatement` nodes (within function bodies) of the form `identifier '.' identifier '=' identifier ';'`.
  - When the first identifier's declared type (from `LocalTypeInfo`) is a known API type → emit `VtableAssignmentRecord`.
  - This visitor method is also invoked from `visitCompoundStatement` rather than from the top level, since direct assignments appear inside function bodies.

---

**3. `visitFunctionDefinition(ctx)`** — Replaces P11

Invoked for each `functionDefinition` CST node.
- functionName = `Identifier` from the innermost `directDeclarator` in the `declarator`.
- isStatic = `true` if `declarationSpecifiers` contains a `Static` token.
- file and line = from the CST node's token position.
- Emit `FunctionDefinitionRecord` → add to `functionDefinitions`.

Because the grammar handles both K&R and Allman brace styles natively, no two-pass workaround is needed. Because `functionDefinition` is a distinct production from `declaration` (which ends with `;`), forward declarations are automatically excluded — they parse as `declaration`, not `functionDefinition`.

---

**4. `visitFailureHandlerAssignment(ctx)`** — Replaces P10a, P10b

Invoked for `expressionStatement` nodes where the `assignmentExpression` LHS is a `postfixExpression` ending with a `JunoFailureHandler` or `JunoFailureUserData` token in a member access position.

Because the lexer's alternation patterns match both the macro name (`JUNO_FAILURE_HANDLER`) and the underlying member name (`_pfcnFailureHandler`) as the same `JunoFailureHandler` token type, the visitor handles both code styles uniformly — no dual-pattern workaround is needed. Whether the source contains:
```c
ptEngineApp->tRoot.JUNO_FAILURE_HANDLER = pfcnFailureHandler;
```
or:
```c
ptEngineApp->tRoot._pfcnFailureHandler = pfcnFailureHandler;
```
the grammar sees the same `JunoFailureHandler` token in the `memberIdentifier` position.

- variableName = first token of the LHS `postfixExpression`.
- functionName = identifier on the RHS of the assignment.
- Emit `FailureHandlerRecord`; rootType is resolved at index-merge time via `LocalTypeInfo`.

---

**5. `visitLocalDeclaration(ctx)`** — NEW (no regex equivalent)

Invoked for `declaration` nodes appearing inside `compoundStatement` bodies (i.e., within function bodies).
- Extracts: variable name, declared type, pointer depth, `isConst`, `isArray` flags.
- Builds a `Map<string, TypeInfo>` for the enclosing function scope.
- Stored in `LocalTypeInfo.localVariables[functionName]`.
- Replaces the 200-line backward regex scan: type information for all local variables is available deterministically from the CST.

---

**6. `visitFunctionParameters(ctx)`** — NEW (no regex equivalent)

Invoked for each `functionDefinition` node's `parameterList`.
- Extracts: parameter name, declared type, pointer depth, `isConst` flags for each parameter.
- Stored in `LocalTypeInfo.functionParameters[functionName]`.
- Used by the query-time chain-walk resolver (§5.1) to determine the types of parameters such as `ptHeap`, `tReturn`, and `ptLoggerApi`.

---

**7. `visitPreprocessorDirective(ctx)`** — NEW

Invoked for each `preprocessorDirective` CST node.
- **`#define NAME value`:** Records the macro definition for known LibJuno macros. Opaque macros (not in the LibJuno macro token list) are ignored.
- **`#ifdef`/`#ifndef`/`#endif`:** Tracks nesting depth for future conditional compilation awareness. For the current design, branches are not selectively parsed — all branches are parsed and merged, with `#if`/`#endif` nesting tracked for informational purposes.
- **`#include "path"` / `#include <path>`:** Records the included path for future cross-file navigation support.

---

**8. `visitJunoStandaloneDeclaration(ctx)`** — NEW

Invoked for `junoStandaloneDeclaration` CST nodes.
- **`JUNO_MODULE_DECLARE(NAME_T)`:** Records a forward-declared module union type. Not directly used for navigation but prevents parse errors from macro invocations at file scope.
- **`JUNO_MODULE_ROOT_DECLARE(NAME_T)` / `JUNO_MODULE_DERIVE_DECLARE(NAME_T)`:** Same — forward-declaration bookkeeping.
- **`JUNO_MODULE_RESULT(NAME_T, OK_T)`:** Records a result type definition. The visitor notes the result type name and its payload type. This data is available for future type resolution but is not currently used by the chain-walk algorithm.



---

### 3.2 Workspace Indexer

Responsible for scanning all C and H files in the workspace, invoking the Chevrotain Parser on each, and populating the Navigation Index. Also coordinates with the Cache Manager to load, validate, and save the JSON cache.

**Responsibilities:**
- On activation: load cache; re-index files whose hash has changed; index new files.
- On `FileSystemWatcher` events: re-index the changed file, update cache.
- Provide the populated `NavigationIndex` to the Vtable Resolver, Failure Handler Resolver, and MCP Server.

**File scan scope:** All `*.c`, `*.h`, `*.cpp`, `*.hpp`, `*.hh`, and `*.cc` files in the workspace, excluding `build/`, `deps/`, and `.libjuno/` directories (configurable via extension settings).

**Indexing algorithm:**

```
FOR EACH file in workspace (*.c, *.h, *.cpp), excluding excluded dirs:
  hash = sha256(fileContent)
  IF hash == cache.fileHashes[filePath]:
    SKIP (use cached data)
  ELSE:
    parsedFile = ChevrotainParser.parse(filePath, fileContent)
    MERGE parsedFile.moduleRoots        INTO index.moduleRoots
    MERGE parsedFile.traitRoots         INTO index.traitRoots
    MERGE parsedFile.derivations        INTO index.derivationChain
    MERGE parsedFile.apiStructDefs      INTO index.apiStructFields
    MERGE parsedFile.vtableAssignments  INTO index.vtableAssignments
    MERGE parsedFile.failureHandlers    INTO index.failureHandlerAssignments
    MERGE parsedFile.functionDefs       INTO index.functionDefinitions
    MERGE parsedFile.apiMemberRegistry  INTO index.apiMemberRegistry
    STORE parsedFile.localTypeInfo      INTO index.localTypeInfo[filePath]
    cache.fileHashes[filePath] = hash
END FOR

RESOLVE positional initializers with missing field orders (cross-file deferred records):
  For any positional initializer whose API struct was defined in another file,
  now that all files have been parsed, the field order in apiStructFields is available.
  Retry zipping deferred records.
  If field order still unavailable after full workspace scan: log warning, skip.

SAVE cache to disk
```

> **Single-pass note:** Function definitions are extracted by `visitFunctionDefinition` in the same parse pass as vtable assignments. The previous "second pass for Pattern P11" is eliminated. The `apiMemberRegistry` is also populated inline by `visitStructDefinition` for all struct bodies encountered during the same parse pass.

For each file, the parser also extracts local declarations and function parameters into a per-file type map (`LocalTypeInfo`). This data is stored in the cache under `localTypeInfo` and used at query time for expression type resolution in the chain-walk algorithm (§5.1).

### 3.3 Vtable Resolver

Given a cursor position (file, line, column) in a C source file, resolves the API call under the cursor to one or more concrete function locations.

**Input:** `{ file: string, line: number, column: number }`  
**Output:** `VtableResolutionResult`

```typescript
interface VtableResolutionResult {
  found:     boolean;
  locations: ConcreteLocation[];
  errorMsg?: string;
}

interface ConcreteLocation {
  functionName:    string;
  file:            string;
  line:            number;      // Line of the function **definition**.
                               // Resolved in the same parse pass as vtable assignments (see Section 3.2).
  assignmentFile?: string;     // File where the vtable assignment occurs (composition root)
  assignmentLine?: number;     // Line of the vtable assignment (composition root)
}
```

Resolution algorithm is detailed in Section 5.1.

### 3.4 Failure Handler Resolver

Given a cursor position on a line containing `->_pfcnFailureHandler` or a failure handler call site, resolves the concrete handler function.

**Input:** `{ file: string, line: number, column: number }`  
**Output:** `VtableResolutionResult` (same type, reused)

Resolution algorithm is detailed in Section 5.3.

### 3.5 VSCode Integration Layer

Provides the user-facing features: DefinitionProvider registration, QuickPick for multiple results, commands, status bar messages, and the vtable resolution trace WebviewPanel (VtableTraceProvider).

**Registered providers and commands:**

| Item | Type | Trigger |
|------|------|---------|
| `JunoDefinitionProvider` | `vscode.DefinitionProvider` | F12, Ctrl+Click on C/C++ files |
| `VtableTraceProvider` | WebviewPanel | `libjuno.showVtableTrace` command |
| `libjuno.goToImplementation` | Command | Command Palette |
| `libjuno.reindexWorkspace` | Command | Command Palette |
| `libjuno.showVtableTrace` | Command | Command Palette; Ctrl+Shift+T; Right-click context menu |

### 3.6 MCP Server

An HTTP server embedded in the extension process, implementing the Model Context Protocol. Exposes two tools to AI agent platforms. Described in detail in Section 7.

### 3.7 Cache Manager

Handles reading and writing `.libjuno/navigation-cache.json`. Detects staleness by comparing file hashes. Described in detail in Section 9.

---

// @{"design": ["REQ-VSCODE-003", "REQ-VSCODE-008", "REQ-VSCODE-009", "REQ-VSCODE-010", "REQ-VSCODE-011", "REQ-VSCODE-012", "REQ-VSCODE-014", "REQ-VSCODE-015"]}
## 4. Data Model

### 4.1 In-Memory Navigation Index

```typescript
interface NavigationIndex {
  // REQ-VSCODE-008: rootType → apiType (from JUNO_MODULE_ROOT)
  moduleRoots: Map<string, string>;

  // REQ-VSCODE-014: traitRootType → apiType (from JUNO_TRAIT_ROOT)
  traitRoots: Map<string, string>;

  // REQ-VSCODE-009, REQ-VSCODE-015: derivedType → rootType
  // (from JUNO_MODULE_DERIVE / JUNO_TRAIT_DERIVE)
  derivationChain: Map<string, string>;

  // REQ-VSCODE-012: apiType → ordered list of function pointer field names
  // Required for positional initializer resolution
  apiStructFields: Map<string, string[]>;

  // REQ-VSCODE-010, REQ-VSCODE-011, REQ-VSCODE-012:
  // apiType → fieldName → list of concrete implementations
  // ConcreteLocation.line is the function definition line (resolved in the same parse pass)
  vtableAssignments: Map<string, Map<string, ConcreteLocation[]>>;

  // REQ-VSCODE-016: rootType → list of concrete failure handlers
  failureHandlerAssignments: Map<string, ConcreteLocation[]>;

  // Chain-walk algorithm Step 5a: struct member name → API pointer type.
  // Populated during indexing: for any struct body (ROOT, IMPL, or any other),
  // if a member's declared type ends in _API_T it is recorded here.
  // Example: ptHeapPointerApi → JUNO_DS_HEAP_POINTER_API_T
  // Used by the chain-walk fallback to resolve non-ptApi API member names at call sites.
  apiMemberRegistry: Map<string, string>;

  // visitFunctionDefinition: functionName → list of definition records.
  // Multiple entries arise when identically named static functions exist in different files.
  // Used during vtable assignment resolution and as a fallback for the Vtable Resolver.
  functionDefinitions: Map<string, FunctionDefinitionRecord[]>;

  // Per-file local variable and parameter type information.
  // Populated by visitLocalDeclaration and visitFunctionParameters.
  // Keyed by workspace-relative file path.
  localTypeInfo: Map<string, LocalTypeInfo>;
}

interface ConcreteLocation {
  functionName:    string;
  file:            string;
  line:            number;      // Line of the function definition
  assignmentFile?: string;      // File where the vtable assignment occurs (composition root)
  assignmentLine?: number;      // Line of the vtable assignment (composition root)
}

interface FunctionDefinitionRecord {
  functionName: string;
  file:         string;
  line:         number;
  isStatic:     boolean;  // true if the function was declared with the `static` keyword
  signature?:   string;  // Full function signature text (return type + name + params)
}
```

The `NavigationIndex` is held in memory by the Workspace Indexer and shared by reference with the Vtable Resolver, Failure Handler Resolver, and MCP Server.

### 4.2 JSON Cache Schema

File: `.libjuno/navigation-cache.json`

```jsonc
{
  "version": "1",
  "generatedAt": "2026-04-14T12:00:00.000Z",
  "fileHashes": {
    "include/juno/ds/heap_api.h": "a3f...bc1",
    "src/juno_heap.c": "9d2...f44"
  },
  "moduleRoots": {
    "JUNO_DS_HEAP_ROOT_T": "JUNO_DS_HEAP_API_T"
  },
  "traitRoots": {
    "JUNO_POINTER_T": "JUNO_POINTER_API_T"
  },
  "derivationChain": {
    "JUNO_DS_HEAP_IMPL_T": "JUNO_DS_HEAP_ROOT_T"
  },
  "apiStructFields": {
    "JUNO_DS_HEAP_API_T": ["Insert", "Heapify", "Pop"]
  },
  "vtableAssignments": {
    "JUNO_DS_HEAP_API_T": {
      "Insert":  [{ "functionName": "JunoDs_Heap_Insert",  "file": "src/juno_heap.c", "line": 259, "assignmentFile": "src/juno_heap.c", "assignmentLine": 18 }],
      "Heapify": [{ "functionName": "JunoDs_Heap_Heapify", "file": "src/juno_heap.c", "line": 270, "assignmentFile": "src/juno_heap.c", "assignmentLine": 19 }],
      "Pop":     [{ "functionName": "JunoDs_Heap_Pop",     "file": "src/juno_heap.c", "line": 285, "assignmentFile": "src/juno_heap.c", "assignmentLine": 20 }]
    }
  },
  "failureHandlerAssignments": {
    "JUNO_DS_HEAP_ROOT_T": [
      { "functionName": "MyFailureHandler", "file": "examples/example.c", "line": 42 }
    ]
  },
  "apiMemberRegistry": {
    "ptHeapPointerApi": "JUNO_DS_HEAP_POINTER_API_T"
  },
  "functionDefinitions": {
    "Publish": [
      { "file": "src/juno_broker.c", "line": 51, "isStatic": true, "signature": "static JUNO_STATUS_T Publish(JUNO_BROKER_ROOT_T *ptBroker, JUNO_BROKER_TOPIC_T tTopic, JUNO_POINTER_T tData)" }
    ],
    "JunoDs_Heap_Insert": [
      { "file": "src/juno_heap.c", "line": 259, "isStatic": false, "signature": "JUNO_STATUS_T JunoDs_Heap_Insert(JUNO_DS_HEAP_ROOT_T *ptHeap, JUNO_POINTER_T tValue)" }
    ]
  },
  "localTypeInfo": {
    "src/juno_heap.c": {
      "localVariables": {
        "JunoDs_Heap_Init": {
          "iCounter": { "name": "iCounter", "typeName": "int", "isPointer": false, "isConst": false, "isArray": false }
        }
      },
      "functionParameters": {
        "JunoDs_Heap_Init": [
          { "name": "ptHeap", "typeName": "JUNO_DS_HEAP_ROOT_T", "isPointer": true, "isConst": false, "isArray": false }
        ]
      }
    }
  }
}
```

**Schema fields:**

| Field | Type | Description |
|-------|------|-------------|
| `version` | `string` | Cache format version. Cache is discarded on version mismatch. |
| `generatedAt` | `string` (ISO 8601) | Timestamp of last full index. |
| `fileHashes` | `object` | workspace-relative path → SHA-256 hex of file content. |
| `moduleRoots` | `object` | rootType → apiType (from `JUNO_MODULE_ROOT`). |
| `traitRoots` | `object` | traitRootType → apiType (from `JUNO_TRAIT_ROOT`). |
| `derivationChain` | `object` | derivedType → immediate rootType. Chain is walked at resolution time. |
| `apiStructFields` | `object` | apiType → ordered field names (from struct definition). |
| `vtableAssignments` | `object` | apiType → fieldName → array of `ConcreteLocation`. Each `line` is the function **definition** line; `assignmentFile`/`assignmentLine` record the vtable assignment site (composition root). |
| `failureHandlerAssignments` | `object` | rootType → array of `ConcreteLocation`. |
| `apiMemberRegistry` | `object` | struct member name → API pointer type (`_API_T`). Built during indexing from all struct definitions. Used by the chain-walk fallback to resolve non-`ptApi` API member names. |
| `functionDefinitions` | `object` | functionName → array of `FunctionDefinitionRecord` (`file`, `line`, `isStatic`, `signature?`). Multiple entries arise when identically named `static` functions exist in different files. |
| `localTypeInfo` | `object` | filePath → `LocalTypeInfo` (local variable and parameter type maps, keyed per function). Used at query time for the chain-walk type resolver. |

---

// @{"design": ["REQ-VSCODE-003"]}
## 4.3 Per-File Type Information

```typescript
interface LocalTypeInfo {
  /** functionName → Map of variableName → declared type */
  localVariables: Map<string, Map<string, TypeInfo>>;
  /** functionName → list of parameter type entries */
  functionParameters: Map<string, TypeInfo[]>;
}

interface TypeInfo {
  name:      string;   // variable/parameter name
  typeName:  string;   // declared type, e.g. "JUNO_DS_HEAP_ROOT_T"
  isPointer: boolean;  // true if declared as pointer (*)
  isConst:   boolean;  // true if declared with const
  isArray:   boolean;  // true if declared as array
}
```

`LocalTypeInfo` is populated by `visitLocalDeclaration` and `visitFunctionParameters` during the parse pass. It is stored in `NavigationIndex.localTypeInfo` (keyed by file path) and serialized to the JSON cache under `localTypeInfo`.

---

// @{"design": ["REQ-VSCODE-002", "REQ-VSCODE-004", "REQ-VSCODE-005", "REQ-VSCODE-006", "REQ-VSCODE-009", "REQ-VSCODE-015", "REQ-VSCODE-016", "REQ-VSCODE-022", "REQ-VSCODE-023", "REQ-VSCODE-024", "REQ-VSCODE-025", "REQ-VSCODE-026"]}
## 5. Resolution Algorithms

### 5.1 Vtable Call Resolution

Triggered when the user places the cursor on a vtable call site and activates Go to Definition.

**Input:** cursor position `(file, line, column)`

**Algorithm:**

```
STEP 1 — Re-parse the current file (or retrieve cached CST)
  Parse the file using the Chevrotain parser to obtain the CST.
  In practice, the CST may be cached from the most recent index run. If the file
  has been modified since the last index, re-parse it.

STEP 2 — Locate the CST node at the cursor position
  Walk the CST to find the deepest node spanning (line, column).
  The target node should be an Identifier within a postfixExpression.
  If the node is not part of a postfixExpression with a function call suffix:
    RETURN { found: false, errorMsg: "Cursor is not on a LibJuno API call site." }

STEP 3 — Extract the postfixExpression chain
  Walk up from the cursor node to the enclosing postfixExpression.
  The postfixExpression has the form:
    primaryExpr (suffix)*
  where each suffix is: [expr], (args), .id, ->id, ++, or --

  Identify which suffix contains the cursor. It should be a '->identifier' suffix
  where 'identifier' is the field name being called, followed by a '(args)' suffix.

  fieldName = the identifier in the '->identifier' suffix under the cursor.

STEP 4 — Resolve the receiver type by walking the chain
  Starting from the primaryExpression (leftmost element), resolve the type at each step:

  4a. primaryExpression:
    - If Identifier: look up in localTypeInfo for the containing file and function
      (local variables first, then function parameters). The type comes from
      the TypeInfo entry produced by visitLocalDeclaration / visitFunctionParameters.
    - If junoModuleGetApiMacro (JUNO_MODULE_GET_API(expr, TYPE)):
        rootType = TYPE (explicit in the macro); skip to Step 5 (derivation chain).
    - If '(' expression ')': recursively resolve the inner expression type.

  4b. For each suffix in order (left to right up to the cursor suffix):
    - '->member' or '.member':
        The member token may be an Identifier, a JunoModuleSuper token, a
        JunoFailureHandler token, or a JunoFailureUserData token (these
        are all valid in the memberIdentifier grammar position).

        If the member is JunoModuleSuper:
          currentType = the root type embedded as the first member of
          currentType's struct. This is semantically equivalent to
          accessing the literal member 'tRoot', consistent with:
            #define JUNO_MODULE_SUPER tRoot

        If the member is JunoFailureHandler / JunoFailureUserData:
          This is a failure handler member access (handled by §5.3).

        Otherwise (Identifier):
          '->member': currentType must be a pointer-to-struct.
          Look up 'member' in the struct's definition from the index
          (apiMemberRegistry or struct member data from visitStructDefinition).
          currentType = declared type of 'member'.
          '.member': same as '->' but for non-pointer struct access.

    - '[expr]': array subscript — strip one pointer/array level from currentType.
    - '(args)': function call — result type is the return type of the function.
        (For vtable dispatch this is typically the final suffix and return type
        resolution is not needed.)

  4c. When we reach the '->fieldName' suffix under the cursor:
    - currentType at this point should be an '_API_T *' (API struct pointer).
    - apiType = currentType (stripped of pointer).
    - If apiType IS a registered API type: GOTO STEP 6 (look up implementations).
    - If currentType is NOT a registered API type:
        The suffix immediately before '->fieldName' is likely '->ptApi' or '.ptApi'.
        In that case the type before 'ptApi' was a root/derived type.
        rootType = currentType (the type resolved just before the ptApi access).
        GOTO STEP 5 (derivation chain resolution).

STEP 5 — Fallback: apiMemberRegistry and field-name search
  If Step 4 could not determine apiType (type resolution failed at some intermediate step):

  5a. Check apiMemberRegistry for the member immediately before '->fieldName':
    memberName = the identifier in the suffix immediately before '->fieldName'
    IF memberName is in index.apiMemberRegistry:
      apiType = index.apiMemberRegistry[memberName]
      GOTO STEP 6.

  5b. Field-name search across all API types (uniform fallback):
    candidates = all apiTypes in index.apiStructFields that contain fieldName.
    IF candidates.length == 0:
      RETURN { found: false, errorMsg: "No API type contains field '${fieldName}'." }
    IF candidates.length == 1:
      apiType = candidates[0]; GOTO STEP 6.
    IF candidates.length > 1:
      allLocations = []
      FOR EACH candidate IN candidates:
        locs = index.vtableAssignments.get(candidate)?.get(fieldName) ?? []
        allLocations.push(...locs)
      RETURN { found: allLocations.length > 0, locations: allLocations }

STEP 5 (from 4c) — Resolve derivation chain to the root type
  current = rootType
  WHILE index.derivationChain.has(current):
    current = index.derivationChain.get(current)
  rootType = current
  // rootType is now the topmost root type with an API mapping

  apiType = index.moduleRoots.get(rootType) ?? index.traitRoots.get(rootType)
  IF apiType is undefined:
    RETURN { found: false, errorMsg: "No API type registered for root '${rootType}'." }

STEP 6 — Look up concrete implementations
  fieldMap = index.vtableAssignments.get(apiType)
  IF fieldMap is undefined:
    RETURN { found: false, errorMsg: "No vtable assignments found for '${apiType}'." }
  locations = fieldMap.get(fieldName)
  IF locations is undefined or empty:
    RETURN { found: false, errorMsg: "No implementation found for '${apiType}::${fieldName}'." }

STEP 7 — Return results
  RETURN { found: true, locations: locations }
```

**Key improvements over the regex-based approach:**

- **No backward regex scans:** Local variable types come from `LocalTypeInfo`, populated deterministically by the parser visitor. The 200-line backward scan heuristic is eliminated.
- **No strategy ordering:** The chain-walk algorithm handles ALL call patterns uniformly. `ptTime->ptApi->Now(...)`, `ptAppList[i]->ptApi->OnStart(...)`, `ptEngineApp->ptBroker->ptApi->RegisterSubscriber(...)`, `tReturn.ptApi->Copy(...)`, and `ptLoggerApi->LogInfo(...)` are all different shapes of `postfixExpression` chains resolved by the same algorithm.
- **No Allman brace workarounds:** The grammar handles both K&R and Allman brace styles natively.
- **No macro form gaps:** `JUNO_FAILURE_HANDLER` is a first-class lexer token, so `var->member.JUNO_FAILURE_HANDLER = func;` is parsed correctly without dual patterns.

**Call pattern coverage — mapping the 20 patterns from prior catalog to the chain-walk algorithm:**

| Old Category | Example | Chain-Walk Path |
|-------------|---------|----------------|
| Category 1 — Simple `->ptApi->Field` | `ptTime->ptApi->Now(ptTime)` | Identifier → `->ptApi` (root type) → `->Now` (field): rootType from localTypeInfo[ptTime]; walk derivation chain to apiType |
| Category 1 — Array subscript | `ptAppList[i]->ptApi->OnStart(...)` | Identifier → `[i]` (strip array) → `->ptApi` → `->OnStart` |
| Category 1 — Array subscript counter | `ptAppList[iCounter]->ptApi->OnProcess(...)` | Same as above |
| Category 1 — Chained member | `ptEngineApp->ptBroker->ptApi->RegisterSubscriber(...)` | Identifier → `->ptBroker` (member type) → `->ptApi` → `->RegisterSubscriber` |
| Category 1 — Chained member | `ptSystemManagerApp->ptTime->ptApi->Now(...)` | Identifier → `->ptTime` → `->ptApi` → `->Now` |
| Category 1 — Chained member | `ptStack->ptStackArray->ptApi->GetAt(...)` | Identifier → `->ptStackArray` → `->ptApi` → `->GetAt` |
| Category 1 — Simple pointer | `ptRecvQueue->ptApi->Enqueue(...)` | Identifier → `->ptApi` → `->Enqueue` |
| Category 1 — Simple pointer | `ptBuffer->ptApi->SetAt(...)` | Identifier → `->ptApi` → `->SetAt` |
| Category 2 — Nested dot | `tPtrResult.tOk.ptApi->Copy(...)` | Identifier → `.tOk` → `.ptApi` → `->Copy` |
| Category 2 — Stack value | `tReturn.ptApi->Copy(tReturn, tResult.tOk)` | Identifier → `.ptApi` → `->Copy` |
| Category 2 — Stack value | `tItem.ptApi->Copy(...)` | Identifier → `.ptApi` → `->Copy` |
| Category 2 — Nested dot | `tResult.tOk.ptApi->Reset(...)` | Identifier → `.tOk` → `.ptApi` → `->Reset` |
| Category 2 — Stack value | `tIndexPointer.ptApi->Copy(...)` | Identifier → `.ptApi` → `->Copy` |
| Category 2 — Stack value | `tIndexPointer.ptApi->Reset(...)` | Identifier → `.ptApi` → `->Reset` |
| Category 2 — Stack value | `tMemory.ptApi->Reset(...)` | Identifier → `.ptApi` → `->Reset` |
| Category 3 — Direct API ptr | `ptLoggerApi->LogInfo(...)` | Identifier → `->LogInfo`: localTypeInfo[ptLoggerApi] = `JUNO_LOG_API_T *`; apiType direct from type |
| Category 3 — Direct API ptr | `ptCmdPipeApi->Dequeue(...)` | Identifier → `->Dequeue`: localTypeInfo resolves to `JUNO_DS_QUEUE_API_T *` |
| Category 3 — Direct API ptr | `ptArrayApi->GetAt(...)` | Identifier → `->GetAt`: localTypeInfo resolves to `JUNO_DS_ARRAY_API_T *` |
| Category 4 — Named API member | `ptHeap->ptHeapPointerApi->Compare(...)` | Identifier → `->ptHeapPointerApi` (apiMemberRegistry lookup) → `->Compare` |
| Category 5 — Macro-based | `JUNO_MODULE_GET_API(ptModule, ROOT_T)->Field(...)` | `junoModuleGetApiMacro` primary expression → rootType = ROOT_T explicit |

All 20 patterns are handled by the single chain-walk algorithm without strategy enumeration.

### 5.2 Multiple Result Dispatch (VSCode Integration)

After the Vtable Resolver returns:

```
IF result.found == false:
  Show status bar message (see Section 8)
  RETURN (no navigation)

IF result.locations.length == 1:
  // REQ-VSCODE-005: direct navigation
  Navigate to result.locations[0] via vscode.window.showTextDocument

IF result.locations.length > 1:
  // REQ-VSCODE-006: selection list
  Show vscode.window.showQuickPick with items:
    label: functionName
    description: file:line
    detail: relative file path
  ON selection: Navigate to selected location
```

### 5.3 Failure Handler Resolution

Triggered when the cursor is on a line containing `_pfcnFailureHandler` or `JUNO_FAILURE_HANDLER`.

**Algorithm:**

```
STEP 1 — Re-parse the current file (or retrieve cached CST)
  Same as Vtable Resolution Step 1.

STEP 2 — Locate the CST node at the cursor position
  Walk the CST to find the deepest node spanning (line, column).
  The target node should be a JunoFailureHandler token appearing as the
  final member access in an assignment expression. Because the lexer's
  alternation pattern (/JUNO_FAILURE_HANDLER\b|_pfcnFailureHandler\b/)
  matches both the macro name and the underlying member name as the same
  token type, the algorithm handles both code styles uniformly.

STEP 3 — Extract the assignment
  Walk up from the cursor node to the enclosing assignmentExpression.
  The LHS is a postfixExpression ending with a JunoFailureHandler token
  in a memberIdentifier position (via '->' or '.' access). The chained
  form '->member.JUNO_FAILURE_HANDLER' and the direct member form
  '->_pfcnFailureHandler' both produce the same token type.
  The RHS is the function name identifier.

  variableName = first identifier of the LHS postfixExpression.
  functionName = RHS identifier.

STEP 4 — Resolve the root type of variableName
  Use the chain-walk algorithm (same as Vtable Resolution Step 4):
  look up variableName in LocalTypeInfo (local variables, then function
  parameters). Walk the derivation chain if the resolved type is a
  derived type.
  rootType = resolved root type.

STEP 5 — Look up concrete handler(s)
  locations = index.failureHandlerAssignments.get(rootType)
  IF empty:
    RETURN { found: false, errorMsg: "No failure handler registered for '${rootType}'." }

STEP 6 — Dispatch (same single/multiple logic as Section 5.2)
```

// @{"design": ["REQ-VSCODE-022", "REQ-VSCODE-023", "REQ-VSCODE-024", "REQ-VSCODE-025", "REQ-VSCODE-026"]}
### 5.3.1 FAIL Macro Call Site Resolution

This subsection extends the `FailureHandlerResolver` to handle the four FAIL/ASSERT macro call sites. Recognition and resolution happen **at query time** inside the resolver — no new lexer tokens or index record types are required, because these macros parse as ordinary `Identifier '(' argumentExpressionList ')'` expressions whose arguments are already present as tokens in the line text.

```
STEP 0 — Check for FAIL macro call site
  Inspect the line text at the cursor position for one of the following patterns:
    /\bJUNO_FAIL\s*\(/
    /\bJUNO_FAIL_MODULE\s*\(/
    /\bJUNO_FAIL_ROOT\s*\(/
    /\bJUNO_ASSERT_EXISTS_MODULE\s*\(/
  IF a pattern matches:
    Record the macro name and proceed with FAIL macro resolution (Steps 1–2 below).
  ELSE:
    Fall through to the existing §5.3 algorithm (cursor on _pfcnFailureHandler member).

STEP 1 — Extract macro arguments from line text
  Scan the line text starting from the opening '(' of the macro call.
  Use balanced-parenthesis tracking to handle nested expressions within arguments:
    depth = 0
    FOR each character from the opening '(':
      IF '(': depth++
      IF ')': depth--; IF depth == 0: end of argument list
      IF ',' AND depth == 1: argument boundary
  Collect each argument as a trimmed substring.

  Extract the 2nd argument (index 1, 0-indexed) for all four macro forms:
    JUNO_FAIL(tStatus, pfcnHandler, pvUserData, msg)         → arg[1] = pfcnHandler
    JUNO_FAIL_MODULE(tStatus, ptMod, msg)                   → arg[1] = ptMod
    JUNO_FAIL_ROOT(tStatus, ptRootMod, msg)                 → arg[1] = ptRootMod
    JUNO_ASSERT_EXISTS_MODULE(ptr, ptMod, str)              → arg[1] = ptMod

  Strip surrounding whitespace and cast expressions (e.g. '(TYPE *)ptr' → 'ptr').
  extractedArg = the bare identifier name.

STEP 2 — Resolve based on macro type

  IF macro is JUNO_FAIL:
    handlerName = extractedArg  // a function pointer variable or function name
    locations = index.functionDefinitions.get(handlerName)
    IF locations is defined and non-empty:
      RETURN { found: true, locations: locations.map(fd => ({
        functionName: fd.functionName, file: fd.file, line: fd.line
      }))}
    ELSE:
      RETURN { found: false, errorMsg: "No definition found for failure handler '${handlerName}'." }

  IF macro is JUNO_FAIL_MODULE or JUNO_ASSERT_EXISTS_MODULE:
    // The second argument is a pointer to a derived or root module struct.
    modulePtrName = extractedArg
    Resolve modulePtrName's declared type using localTypeInfo
    (local variables first, then function parameters — same lookup as §5.3 Step 4).
    Walk the derivation chain to the root type
    (same WHILE loop as §5.1 Step 5):
      current = resolvedType
      WHILE index.derivationChain.has(current):
        current = index.derivationChain.get(current)
      rootType = current
    locations = index.failureHandlerAssignments.get(rootType)
    IF empty:
      RETURN { found: false, errorMsg: "No failure handler registered for '${rootType}'." }
    ELSE:
      RETURN { found: true, locations: locations }

  IF macro is JUNO_FAIL_ROOT:
    // The second argument is already a root (not derived) module pointer.
    rootPtrName = extractedArg
    Resolve rootPtrName's declared type using localTypeInfo (no derivation chain walk).
    rootType = resolvedType
    locations = index.failureHandlerAssignments.get(rootType)
    IF empty:
      RETURN { found: false, errorMsg: "No failure handler registered for '${rootType}'." }
    ELSE:
      RETURN { found: true, locations: locations }
```

**Design rationale:** Handling FAIL macro recognition at query time (in the resolver) rather than at parse/index time (in the CST visitor) avoids adding new record types and index structures. The macro arguments are available directly in the line text; the type information needed for module pointer resolution is already populated in `localTypeInfo` by `visitLocalDeclaration` and `visitFunctionParameters`. This approach keeps the indexer and data model unchanged.

---

// @{"design": ["REQ-VSCODE-005", "REQ-VSCODE-006", "REQ-VSCODE-007", "REQ-VSCODE-013"]}
## 6. VSCode Integration Details

### 6.1 DefinitionProvider Registration

The extension registers a `vscode.DefinitionProvider` for C and C++ files in `extension.ts`'s `activate` function:

```typescript
vscode.languages.registerDefinitionProvider(
  [{ language: 'c' }, { language: 'cpp' }],
  new JunoDefinitionProvider(index, vtableResolver, failureHandlerResolver)
);
```

### 6.2 JunoDefinitionProvider Logic

```typescript
class JunoDefinitionProvider implements vscode.DefinitionProvider {
  provideDefinition(
    document: vscode.TextDocument,
    position: vscode.Position,
    token: vscode.CancellationToken
  ): vscode.ProviderResult<vscode.Definition | vscode.LocationLink[]> {
    // 1. Check if line matches a FAIL macro call site pattern (§5.3.1 Step 0):
    //      /\bJUNO_FAIL\s*\(/, /\bJUNO_FAIL_MODULE\s*\(/, etc.
    //    If yes: invoke FailureHandlerResolver in FAIL macro mode → goto 4.
    // 2. Check if line matches a LibJuno vtable call site pattern (ptApi->Field)
    //    If vtable call: invoke VtableResolver → goto 4.
    // 3. Check if line matches a failure handler assignment (JUNO_FAILURE_HANDLER / _pfcnFailureHandler)
    //    If yes: invoke FailureHandlerResolver in standard §5.3 mode → goto 4.
    // 4. If result.found and single: return LocationLink[]
    // 5. If result.found and multiple: show QuickPick, return undefined
    //    (QuickPick handles navigation imperatively)
    // 6. If not found: show status bar error (Section 8), return undefined
  }
}
```

The provider returns `undefined` (not an error) in the multiple-results and not-found cases; navigation is handled imperatively to allow QuickPick use.

### 6.3 QuickPick Display (REQ-VSCODE-006)

When multiple implementations are found, a QuickPick is displayed with:
- **label:** function name (e.g., `JunoDs_Heap_Insert`)
- **description:** `file.c:27`
- **detail:** workspace-relative file path

Selecting an item opens the file at the specified line.

### 6.4 VSCode Commands

| Command ID | Title | Behavior |
|------------|-------|----------|
| `libjuno.goToImplementation` | LibJuno: Go to Implementation | Runs resolver on current cursor position |
| `libjuno.reindexWorkspace` | LibJuno: Re-index Workspace | Clears cache, runs full workspace scan |

---

// @{"design": ["REQ-VSCODE-017", "REQ-VSCODE-018", "REQ-VSCODE-019", "REQ-VSCODE-020"]}
## 7. MCP Server Design

### 7.1 Overview

The extension starts an HTTP MCP server on a local port (default 6543, configurable) bound to `127.0.0.1`. The port is advertised in a `.libjuno/mcp.json` file so AI platforms can discover it.

MCP is chosen as the AI interface mechanism (REQ-VSCODE-020) because it is platform-agnostic: GitHub Copilot, Claude (via Claude Desktop), and other platforms all support MCP tool servers. The extension does not implement any platform-specific API beyond MCP.

### 7.2 MCP Tool: `resolve_vtable_call`

**Satisfies:** REQ-VSCODE-018

**Description:** Given a C source file path and cursor position, resolves the LibJuno vtable API call to its concrete implementation function(s).

**Input Schema:**
```json
{
  "type": "object",
  "required": ["file", "line", "column"],
  "properties": {
    "file": {
      "type": "string",
      "description": "Absolute or workspace-relative path to the C source file."
    },
    "line": {
      "type": "integer",
      "description": "1-based line number of the API call site."
    },
    "column": {
      "type": "integer",
      "description": "1-based column number of the cursor within the line."
    }
  }
}
```

**Output Schema:**
```json
{
  "type": "object",
  "properties": {
    "found": { "type": "boolean" },
    "locations": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "functionName": { "type": "string" },
          "file":         { "type": "string" },
          "line":         { "type": "integer" }
        }
      }
    },
    "error": { "type": "string" }
  }
}
```

**Example request:**
```json
{ "file": "src/main.c", "line": 42, "column": 15 }
```

**Example response (found):**
```json
{
  "found": true,
  "locations": [
    { "functionName": "JunoDs_Heap_Insert", "file": "src/juno_heap.c", "line": 27 }
  ]
}
```

**Example response (not found):**
```json
{
  "found": false,
  "locations": [],
  "error": "No implementation found for 'JUNO_DS_HEAP_API_T::Insert'."
}
```

### 7.3 MCP Tool: `resolve_failure_handler`

**Satisfies:** REQ-VSCODE-019

**Description:** Given a C source file path and cursor position on a failure handler assignment or call, resolves the concrete handler function(s).

**Input Schema:** Same as `resolve_vtable_call`.

**Output Schema:** Same as `resolve_vtable_call`.

### 7.4 MCP Discovery File

The extension writes `.libjuno/mcp.json` to the workspace root on activation:

```json
{
  "mcpServers": {
    "libjuno": {
      "url": "http://127.0.0.1:6543/mcp"
    }
  }
}
```

This is the format recognized by Claude Desktop and similar platforms. For platforms using the VSCode MCP host API (e.g., GitHub Copilot), the extension also registers the server via `vscode.lm.registerMcpServer` (if available in the host version).

---

// @{"design": ["REQ-VSCODE-004", "REQ-VSCODE-013"]}
## 8. Error Handling Design

### 8.1 Resolution Failure (REQ-VSCODE-004, REQ-VSCODE-013)

When a resolution fails:

1. **Status bar message (primary, non-intrusive):**  
   Display a temporary status bar item with the message:  
   `$(warning) LibJuno: Could not resolve implementation — ${errorMsg}`  
   The item auto-clears after 5 seconds.

2. **Information message (optional, on repeated failure):**  
   If the user triggers resolution and it fails again within 10 seconds, show:  
   `vscode.window.showInformationMessage(errorMsg, "Show Details")`  
   Selecting "Show Details" opens the Output Channel with full diagnostic information (matched patterns, index state for the relevant type).

3. **No modal dialogs.** `showErrorMessage` (which produces a modal in some contexts) is never used for resolution failures.

### 8.2 Indexing Errors

If a file cannot be read during indexing, it is skipped with a warning written to the extension's Output Channel (`LibJuno`). The error does not surface to the user unless all indexing fails.

### 8.3 MCP Server Errors

MCP tool errors are returned as standard MCP error responses (HTTP 200 with `isError: true` in the result), not HTTP error codes. The `error` field in the output schema carries the human-readable explanation.

---

// @{"design": ["REQ-VSCODE-001"]}
## 9. Cache Design

### 9.1 Cache File Location

`.libjuno/navigation-cache.json` in the workspace root folder. If the workspace has multiple root folders, one cache file is created per root folder.

### 9.2 Cache Validity

The cache is considered valid if:
1. `version` matches the extension's current cache format version string.
2. At least one file was indexed (non-empty `fileHashes`).

On load, files whose hash in `fileHashes` does not match the current on-disk hash are re-indexed. Files present on disk but absent from `fileHashes` are indexed as new. Files present in `fileHashes` but absent from disk are removed from the index.

### 9.3 File Watcher Invalidation

The extension registers a `vscode.FileSystemWatcher` for `**/*.{c,h,cpp,hpp,hh,cc}` (excluding the excluded directories). On `onDidChange`, `onDidCreate`, and `onDidDelete` events:

- `onDidChange` or `onDidCreate`: Re-parse the file, remove the old records for that file from the index, merge in the new records, update `fileHashes[filePath]`, schedule a debounced cache write (500 ms delay to batch rapid saves).
- `onDidDelete`: Remove all index records sourced from that file, remove `fileHashes[filePath]`, schedule debounced cache write.

### 9.4 Full Re-index

Triggered by:
- Extension activation when cache is missing, version-mismatched, or explicitly invalidated.
- The `libjuno.reindexWorkspace` command.

Full re-index clears the in-memory index and rebuilds it from scratch, then writes the cache.

### 9.5 Cache Write Strategy

Cache writes are debounced (500 ms) to avoid excessive disk I/O during bulk file events (e.g., `git checkout`). The extension uses `fs.writeFile` with a temporary file and rename to ensure atomic writes, preventing a corrupt cache if the extension process is killed during a write.

---

## 10. Requirements Traceability Matrix

| Requirement ID | Title | Design Element(s) |
|----------------|-------|-------------------|
| REQ-VSCODE-001 | VSCode Extension | Entire extension; `activate()` entry point; DefinitionProvider registration |
| REQ-VSCODE-002 | Vtable-Aware Go to Definition | `JunoDefinitionProvider`; Vtable Resolver (Section 3.3, 5.1) |
| REQ-VSCODE-003 | LibJuno API Pattern Recognition | C Parser (Section 3.1); Lexer token definitions (Section 3.1.1); Grammar productions including `postfixExpression` and `structOrUnionSpecifier` (Section 3.1.2); CST Visitor methods `visitStructDefinition`, `visitVtableDeclaration`, `visitFunctionDefinition`, `visitFailureHandlerAssignment` (Section 3.1.3); chain-walk resolution algorithm (Section 5.1); `apiMemberRegistry` in index and cache |
| REQ-VSCODE-004 | Graceful Error on Missing Implementation | Error Handling Design (Section 8); `VtableResolutionResult.found == false` path |
| REQ-VSCODE-005 | Single Implementation Navigation | Multiple Result Dispatch — single branch (Section 5.2) |
| REQ-VSCODE-006 | Multiple Implementation Selection | Multiple Result Dispatch — QuickPick branch (Section 5.2, 6.3) |
| REQ-VSCODE-007 | Native Go to Definition Integration | `vscode.languages.registerDefinitionProvider` (Section 6.1) |
| REQ-VSCODE-008 | Module Root API Discovery | `visitStructDefinition` — `JUNO_MODULE_ROOT` branch (Section 3.1.3); `junoModuleRootMacro` grammar production (Section 3.1.2); `moduleRoots` in index and cache |
| REQ-VSCODE-009 | Module Derivation Chain Resolution | `visitStructDefinition` — `JUNO_MODULE_DERIVE` branch (Section 3.1.3); `junoModuleDeriveMacro` grammar production (Section 3.1.2); `derivationChain` in index and cache; Step 5 (derivation chain) of resolution algorithm |
| REQ-VSCODE-010 | Designated Initializer Recognition | `visitVtableDeclaration` — designated initializer branch (Section 3.1.3); `designation` grammar production (Section 3.1.2); `vtableAssignments` population |
| REQ-VSCODE-011 | Direct Assignment Recognition | `visitVtableDeclaration` — direct assignment branch (Section 3.1.3); `expressionStatement` grammar production (Section 3.1.2); `vtableAssignments` population |
| REQ-VSCODE-012 | Positional Initializer Recognition | `visitStructDefinition` — API struct field extraction (Section 3.1.3); `visitVtableDeclaration` — positional initializer branch (Section 3.1.3); `apiStructFields` in index and cache; positional zip algorithm |
| REQ-VSCODE-013 | Informative Non-Intrusive Error | Status bar message; optional `showInformationMessage` (Section 8.1) |
| REQ-VSCODE-014 | Trait Root API Discovery | `visitStructDefinition` — `JUNO_TRAIT_ROOT` branch (Section 3.1.3); `junoTraitRootMacro` grammar production (Section 3.1.2); `traitRoots` in index and cache |
| REQ-VSCODE-015 | Trait Derivation Chain Resolution | `visitStructDefinition` — `JUNO_TRAIT_DERIVE` branch (Section 3.1.3); `junoTraitDeriveMacro` grammar production (Section 3.1.2); `derivationChain` shared with module derivations; Step 5 (derivation chain) of resolution algorithm |
| REQ-VSCODE-016 | Failure Handler Navigation | Failure Handler Resolver (Section 3.4, 5.3); `visitFailureHandlerAssignment` (Section 3.1.3); `JunoFailureHandler` lexer token (Section 3.1.1); `failureHandlerAssignments` in index and cache |
| REQ-VSCODE-017 | AI Agent Accessibility | MCP Server (Section 3.6, 7); `.libjuno/mcp.json` discovery file |
| REQ-VSCODE-018 | AI Vtable Resolution Access | MCP tool `resolve_vtable_call` (Section 7.2) |
| REQ-VSCODE-019 | AI Failure Handler Resolution Access | MCP tool `resolve_failure_handler` (Section 7.3) |
| REQ-VSCODE-020 | Platform-Agnostic AI Interface | MCP protocol selection rationale (Section 7.1); no platform-specific AI API used |
| REQ-VSCODE-021 | C and C++ File Type Support | File scan scope (Section 3.2); FileSystemWatcher glob (Section 9.3); configurable extension settings |
| REQ-VSCODE-022 | FAIL Macro Failure Handler Navigation | `FailureHandlerResolver` §5.3.1; `JunoDefinitionProvider` §6.2 |
| REQ-VSCODE-023 | JUNO_FAIL Direct Handler Resolution | §5.3.1 Step 2 — JUNO_FAIL branch; `functionDefinitions` index lookup |
| REQ-VSCODE-024 | JUNO_FAIL_MODULE Handler Resolution | §5.3.1 Step 2 — JUNO_FAIL_MODULE branch; derivation chain walk + `failureHandlerAssignments` lookup |
| REQ-VSCODE-025 | JUNO_FAIL_ROOT Handler Resolution | §5.3.1 Step 2 — JUNO_FAIL_ROOT branch; `failureHandlerAssignments` direct lookup (no derivation chain walk) |
| REQ-VSCODE-026 | JUNO_ASSERT_EXISTS_MODULE Handler Resolution | §5.3.1 Step 2 — same as JUNO_FAIL_MODULE branch; derivation chain walk + `failureHandlerAssignments` lookup |
| REQ-VSCODE-027 | Vtable Resolution Trace View | VtableTraceProvider (§11); WebviewPanel (§11.3); TraceNode/VtableTrace interfaces (§11.2) |
| REQ-VSCODE-028 | Trace View Activation via Keyboard | Keybinding: Ctrl+Shift+T (§11.5); `when` clause guard for C/C++ files |
| REQ-VSCODE-029 | Trace View Activation via Command Palette | Command: `libjuno.showVtableTrace` (§11.5) |
| REQ-VSCODE-030 | Trace View Call Site Node | TraceNode type='call-site' (§11.2 Step 2); WebviewPanel call-site div (§11.3) |
| REQ-VSCODE-031 | Trace View Composition Root Node | ConcreteLocation.assignmentFile/.assignmentLine (§4.1); TraceNode type='composition-root' (§11.2 Step 3) |
| REQ-VSCODE-032 | Trace View Implementation Node | FunctionDefinitionRecord.signature (§4.1); TraceNode type='implementation' (§11.2 Step 4) |

---

// @{"design": ["REQ-VSCODE-027", "REQ-VSCODE-028", "REQ-VSCODE-029", "REQ-VSCODE-030", "REQ-VSCODE-031", "REQ-VSCODE-032"]}
## 11. Vtable Resolution Trace View Design

### 11.1 Overview

The vtable resolution trace view provides a visual tree showing the full resolution chain from an API call site, through the composition root where the vtable was initialized, to the concrete implementation function. This satisfies REQ-VSCODE-027.

The trace view complements the existing Go to Definition feature (§5.1): Go to Definition navigates directly to the implementation, while the trace view surfaces the intermediate wiring step — the composition root vtable assignment — making the full dependency injection chain visible. This is especially useful for debugging DI configuration issues and understanding the wiring of large LibJuno-based systems.

### 11.2 Component: VtableTraceProvider

A new component added to the VSCode Integration Layer (§3.5). Responsibility: collect the full 3-node resolution trace and render it in a `vscode.WebviewPanel`.

**TypeScript interfaces:**

```typescript
interface TraceNode {
  type:   'call-site' | 'composition-root' | 'implementation';
  label:  string;   // e.g., "ptCmdPipeApi->Dequeue(...)"
  file:   string;   // workspace-relative path
  line:   number;
  detail: string;   // additional context line
}

interface VtableTrace {
  callSite:        TraceNode;
  compositionRoot: TraceNode;
  implementation:  TraceNode;
}
```

**Data Collection Algorithm:**

```
STEP 1 — Resolve the vtable call using VtableResolver
  result = vtableResolver.resolve(file, line, column)
  IF result.found == false:
    Show error via StatusBarHelper (same as §8.1)
    RETURN

STEP 2 — Build the call site node (REQ-VSCODE-030)
  callSite = {
    type:   'call-site',
    label:  extractCallExpression(lineText),
    file:   currentFile,
    line:   cursorLine,
    detail: lineText.trim()
  }

STEP 3 — Build the composition root node (REQ-VSCODE-031)
  // Use the extended ConcreteLocation (with assignmentFile/assignmentLine from §4.1)
  location = result.locations[selectedIndex or 0]
  compositionRoot = {
    type:   'composition-root',
    label:  `.${fieldName} = ${location.functionName}`,
    file:   location.assignmentFile,
    line:   location.assignmentLine,
    detail: // read line from assignment file at assignmentLine
  }

STEP 4 — Build the implementation node (REQ-VSCODE-032)
  // Use FunctionDefinitionRecord.signature (from §4.1) if available
  implementation = {
    type:   'implementation',
    label:  location.functionName,
    file:   location.file,
    line:   location.line,
    detail: location.signature ?? location.functionName
  }

STEP 5 — Display the WebviewPanel
  Show a WebviewPanel with HTML rendering of the 3 nodes in tree layout (see §11.3)
```

### 11.3 WebviewPanel Layout

The panel is opened via `vscode.window.createWebviewPanel` with `enableScripts: true`. It uses a self-contained HTML template with inline CSS and a nonce-based inline script — no external resources.

The tree layout uses CSS border and padding to create a visual connection between nodes:

```html
<div class="trace-tree">
  <div class="trace-node call-site">
    <span class="node-icon">📍</span>
    <span class="node-label">Call Site</span>
    <div class="node-detail">
      <a href="#" data-file="..." data-line="...">engine_app.c:223</a>
      <code>ptCmdPipeApi-&gt;Dequeue(...)</code>
    </div>
  </div>
  <div class="trace-connector">│</div>
  <div class="trace-node composition-root">
    <span class="node-icon">🔗</span>
    <span class="node-label">Composition Root</span>
    <div class="node-detail">
      <a href="#" data-file="..." data-line="...">engine_app.c:45</a>
      <code>.Dequeue = JunoDs_BuffQueue_Dequeue</code>
    </div>
  </div>
  <div class="trace-connector">│</div>
  <div class="trace-node implementation">
    <span class="node-icon">⚡</span>
    <span class="node-label">Implementation</span>
    <div class="node-detail">
      <a href="#" data-file="..." data-line="...">juno_buff_queue.c:112</a>
      <code>JUNO_STATUS_T JunoDs_BuffQueue_Dequeue(...)</code>
    </div>
  </div>
</div>
```

File link clicks are communicated back to the extension host via `postMessage`. The extension host handles each click by calling `vscode.window.showTextDocument` to navigate to the referenced file and line.

### 11.4 Multiple Results Handling

When `result.locations.length > 1`, the WebviewPanel shows all results. Each location becomes a collapsible section with its own composition root → implementation subtree. The call site node is shared at the top.

### 11.5 Command Registration (REQ-VSCODE-028, REQ-VSCODE-029)

The following entries are added to `package.json` under `contributes`:

```json
{
  "commands": [
    {
      "command": "libjuno.showVtableTrace",
      "title": "LibJuno: Show Vtable Resolution Trace"
    }
  ],
  "keybindings": [
    {
      "command": "libjuno.showVtableTrace",
      "key": "ctrl+shift+t",
      "when": "editorTextFocus && resourceLangId =~ /^c/"
    }
  ],
  "menus": {
    "editor/context": [
      {
        "command": "libjuno.showVtableTrace",
        "when": "resourceLangId =~ /^c/",
        "group": "navigation"
      }
    ]
  }
}
```

**Activation gesture summary:**

| Gesture | Details |
|---------|---------|
| Command Palette | `libjuno.showVtableTrace` — "LibJuno: Show Vtable Resolution Trace" |
| Keyboard shortcut | `Ctrl+Shift+T` with `when: editorTextFocus && resourceLangId =~ /^c/` |
| Right-click context menu | "LibJuno: Show Vtable Resolution Trace" — group: navigation |

> **Note:** `Ctrl+Shift+Click` is a built-in VSCode gesture (multi-cursor) and is NOT used for trace view activation.

### 11.6 Security

The WebviewPanel uses `enableScripts: true` to handle file link clicks via `postMessage`. Security measures:

- All file paths and code text are **HTML-escaped** before insertion into the panel HTML to prevent XSS.
- The Content Security Policy restricts script sources to `nonce`-based inline scripts only:
  ```
  Content-Security-Policy: default-src 'none'; script-src 'nonce-${nonce}'; style-src 'unsafe-inline';
  ```
- No external resources (fonts, images, CDN scripts) are loaded.

---

// @{"design": ["REQ-VSCODE-001", "REQ-VSCODE-021"]}
## Appendix A: Extension File Structure

```
vscode-extension/
  package.json              — extension manifest, commands, activationEvents
  tsconfig.json
  src/
    extension.ts            — activate(), register providers and commands
    parser/
      lexer.ts              — Chevrotain token definitions
      parser.ts             — Chevrotain grammar rules (CParser class)
      visitor.ts            — CST visitor (IndexBuildingVisitor)
      types.ts              — ParsedFile interface, record types, LocalTypeInfo
    indexer/
      workspaceIndexer.ts   — WorkspaceIndexer, file scan, FileSystemWatcher
      navigationIndex.ts    — NavigationIndex type and in-memory store
    resolver/
      vtableResolver.ts     — VtableResolver (chain-walk algorithm)
      failureHandlerResolver.ts
    vscode/
      junoDefinitionProvider.ts
      quickPickHelper.ts    — multiple result QuickPick
      statusBarHelper.ts    — non-intrusive error display
    providers/
      vtableTraceProvider.ts   — WebviewPanel trace view
    mcp/
      mcpServer.ts          — HTTP MCP server, tool registration
    cache/
      cacheManager.ts       — JSON read/write, hash comparison, atomic write
  design/
    design.md               — this document
    test-cases.md           — test case specification
```

## Appendix B: Key Type Summary

| TypeScript Type | Purpose |
|-----------------|---------|
| `ParsedFile` | Output of the Chevrotain parser + visitor for one file |
| `NavigationIndex` | In-memory index (Maps) populated by the CST visitor |
| `VtableResolutionResult` | Output of VtableResolver and FailureHandlerResolver |
| `ConcreteLocation` | `{ functionName, file, line, assignmentFile?, assignmentLine? }` — `line` is the function definition line; `assignmentFile`/`assignmentLine` are the composition root vtable assignment site |
| `FunctionDefinitionRecord` | `{ functionName, file, line, isStatic, signature? }` — one entry per definition site |
| `LocalTypeInfo` | Per-file local variable and parameter type maps, keyed per function |
| `TypeInfo` | `{ name, typeName, isPointer, isConst, isArray }` — one entry per variable/parameter |
| `CacheFile` | Serialized JSON cache schema (matches Section 4.2) |
| `TraceNode` | One node in the resolution trace: call site, composition root, or implementation |
| `VtableTrace` | The complete 3-node resolution trace (callSite, compositionRoot, implementation) |
