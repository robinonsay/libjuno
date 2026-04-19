> Part of: [Software Design Document](index.md) — Section 3 intro, 3.1 C Parser intro, 3.1.1 Lexer

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
