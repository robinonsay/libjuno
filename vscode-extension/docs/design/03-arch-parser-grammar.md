> Part of: [Software Design Document](index.md) — Section 3.1.2 Grammar

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
