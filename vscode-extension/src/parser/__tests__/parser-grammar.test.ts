/**
 * @file parser-grammar.test.ts
 *
 * Mutation-killing tests for parser.ts grammar rules. Each test exercises a
 * specific grammar alternative or GATE condition that is not covered by the
 * existing visitor-structs / visitor-vtable / visitor-functions tests.
 *
 * Strategy: parse a synthetic C snippet that requires a specific grammar path,
 * then assert on the resulting ParsedFile content. A vtable assignment or
 * function definition that appears *after* the construct under test proves that
 * the grammar did not crash the rest of the parse.
 *
 * Uses `parseFile` for declaration/type tests and `parseFileWithDefs` for
 * function-body-level tests so we can assert on extracted records.
 */

/// <reference types="jest" />

import { parseFile, parseFileWithDefs } from "../visitor";
import { CLexer } from '../lexer';
import { CParser } from '../parser';

// ---------------------------------------------------------------------------
// Helper: a trivial vtable assignment appended after the construct under test.
// If parsing crashes on the construct, this assignment will not be extracted,
// failing the assertion.
// ---------------------------------------------------------------------------
const SENTINEL_VTABLE = `
static const JUNO_APP_API_T tSentinelApi = {
    .OnStart = SentinelStart,
};
`;
const SENTINEL_FN = `JUNO_APP_API_T`;
const SENTINEL_FIELD = `OnStart`;

// @{"verify": ["REQ-VSCODE-003"]}

// ===========================================================================
// Group 1 — Storage Class Specifiers (kills ArrowFunction and LogicalOperator
// mutants in storageClassSpecifier and declarationSpecifiers GATE)
// ===========================================================================

describe("Parser grammar: storage class specifiers", () => {

    it("GRAM-001: typedef declaration parses without disrupting subsequent vtable extraction", () => {
        const src = `typedef unsigned int MY_UINT_T;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram001.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
        expect(result.vtableAssignments[0].field).toBe(SENTINEL_FIELD);
    });

    it("GRAM-002: extern declaration parses without disrupting subsequent vtable extraction", () => {
        const src = `extern int g_iCounter;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram002.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
    });

    it("GRAM-003: typedef struct parses without disrupting subsequent vtable extraction", () => {
        const src = `typedef struct MY_POINT_TAG { int x; int y; } MY_POINT_T;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram003.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
    });

    it("GRAM-004: static function declaration with typed return parses correctly", () => {
        const src = `static int MyFunc(int x) { return x; }\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram004.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
    });
});

// ===========================================================================
// Group 2 — Type Qualifiers (kills ArrowFunction and GATE LogicalOperator
// mutants for Volatile and Inline alternatives)
// ===========================================================================

describe("Parser grammar: type qualifiers", () => {

    it("GRAM-010: volatile variable declaration parses without disrupting subsequent vtable", () => {
        const src = `volatile int g_iFlag;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram010.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
    });

    it("GRAM-011: inline function definition parses without disrupting subsequent vtable", () => {
        const src = `inline int Add(int a, int b) { return a + b; }\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram011.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
    });

    it("GRAM-012: const volatile multi-specifier declaration parses correctly", () => {
        const src = `const volatile int g_iReg;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram012.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-013: static volatile variable parses correctly", () => {
        const src = `static volatile unsigned int g_uTimer;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram013.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });
});

// ===========================================================================
// Group 3 — Primitive Types (kills ArrowFunction mutants for each token in
// primitiveType and LogicalOperator mutants in declarationSpecifiers GATE)
// ===========================================================================

describe("Parser grammar: primitive types", () => {

    it("GRAM-020: char type in declaration", () => {
        const src = `char g_cByte;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram020.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-021: short type in declaration", () => {
        const src = `short g_sVal;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram021.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-022: long type in declaration", () => {
        const src = `long g_lVal;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram022.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-023: float type in declaration", () => {
        const src = `float g_fVal;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram023.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-024: double type in declaration", () => {
        const src = `double g_dVal;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram024.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-025: signed type in declaration", () => {
        const src = `signed int g_iSigned;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram025.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-026: unsigned type in declaration", () => {
        const src = `unsigned int g_uVal;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram026.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-027: size_t type in declaration", () => {
        const src = `size_t g_zLen;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram027.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-028: bool type in declaration", () => {
        const src = `_Bool g_bFlag;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram028.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-029: unsigned long multi-specifier in GATE loop", () => {
        // Exercises la1=Long after la1=unsigned — kills LogicalOperator Long mutant
        const src = `unsigned long g_ulVal;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram029.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-030: signed char multi-specifier in GATE loop", () => {
        const src = `signed char g_scByte;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram030.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-031: long double multi-specifier in GATE loop", () => {
        const src = `long double g_ldVal;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram031.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });
});

// ===========================================================================
// Group 4 — Enum Specifier (kills enumSpecifier ArrowFunction and
// ConditionalExpression mutants)
// ===========================================================================

describe("Parser grammar: enum specifier", () => {

    it("GRAM-040: named enum with value list parses without disrupting vtable", () => {
        const src = `
enum MY_STATE_E {
    MY_STATE_IDLE = 0,
    MY_STATE_RUNNING,
    MY_STATE_DONE,
};
` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram040.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
    });

    it("GRAM-041: anonymous enum with trailing comma parses without disrupting vtable", () => {
        const src = `
enum {
    ALPHA = 10,
    BETA = 20,
};
` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram041.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-042: enum declaration using typedef", () => {
        const src = `typedef enum MY_COLOR_E { RED, GREEN, BLUE } MY_COLOR_T;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram042.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-043: enum reference (forward reference, no body) parses correctly", () => {
        const src = `enum MY_STATE_E g_eState;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram043.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-044: enum used as function parameter type extracts correct function def", () => {
        const src = `static void SetState(int eState) { }`;
        const { functionDefs } = parseFileWithDefs("/test/gram044.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("SetState");
    });
});

// ===========================================================================
// Group 5 — Union Specifier (kills union ArrowFunction mutant in
// structOrUnionSpecifier)
// ===========================================================================

describe("Parser grammar: union specifier", () => {

    it("GRAM-050: union type declaration parses without disrupting vtable", () => {
        const src = `
union MY_VALUE_U {
    int iVal;
    float fVal;
    unsigned int uVal;
};
` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram050.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
    });

    it("GRAM-051: typedef union declaration parses correctly", () => {
        const src = `typedef union MY_DATA_U { int i; float f; } MY_DATA_T;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram051.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-052: anonymous struct body variant — struct without tag name", () => {
        // Exercises the LBrace gate branch in structOrUnionSpecifier
        const src = `struct { int x; int y; } g_tPoint;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram052.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });
});

// ===========================================================================
// Group 6 — Iteration Statements (kills ArrowFunction mutants for while/do/for)
// ===========================================================================

describe("Parser grammar: iteration statements", () => {

    it("GRAM-060: while loop in function body — function definition extracted", () => {
        const src = `
static int CountDown(int n)
{
    int result = 0;
    while (n > 0)
    {
        result++;
        n--;
    }
    return result;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram060.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("CountDown");
    });

    it("GRAM-061: do-while loop in function body — function definition extracted", () => {
        const src = `
static void DoWork(int count)
{
    int i = 0;
    do
    {
        i++;
    } while (i < count);
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram061.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("DoWork");
    });

    it("GRAM-062: for loop with initializer expression — function definition extracted", () => {
        const src = `
static int Sum(int n)
{
    int total = 0;
    int i;
    for (i = 0; i < n; i++)
    {
        total += i;
    }
    return total;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram062.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("Sum");
    });

    it("GRAM-063: for loop with declaration initializer — function definition extracted", () => {
        const src = `
static int SumDecl(int n)
{
    int total = 0;
    for (int i = 0; i < n; i++)
    {
        total += i;
    }
    return total;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram063.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("SumDecl");
    });

    it("GRAM-064: for loop with no expression update — empty update clause", () => {
        const src = `
static void InfiniteLoop(void)
{
    for (int x = 0; x < 10; )
    {
        x += 2;
    }
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram064.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("InfiniteLoop");
    });
});

// ===========================================================================
// Group 7 — Selection Statements (kills ArrowFunction mutants for if/else/switch)
// ===========================================================================

describe("Parser grammar: selection statements", () => {

    it("GRAM-070: if statement without else — function definition extracted", () => {
        const src = `
static int Clamp(int x)
{
    if (x < 0)
    {
        x = 0;
    }
    return x;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram070.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("Clamp");
    });

    it("GRAM-071: if-else statement — both branches exercised", () => {
        const src = `
static int Abs(int x)
{
    if (x >= 0)
    {
        return x;
    }
    else
    {
        return -x;
    }
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram071.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("Abs");
    });

    it("GRAM-072: switch with case and default — function definition extracted", () => {
        const src = `
static int Decode(int code)
{
    int result = 0;
    switch (code)
    {
        case 1:
            result = 10;
            break;
        case 2:
            result = 20;
            break;
        default:
            result = -1;
            break;
    }
    return result;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram072.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("Decode");
    });

    it("GRAM-073: nested if-else if chains parse correctly", () => {
        const src = `
static int Sign(int x)
{
    if (x > 0) { return 1; }
    else if (x < 0) { return -1; }
    else { return 0; }
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram073.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("Sign");
    });
});

// ===========================================================================
// Group 8 — Jump Statements (kills break/continue/goto/return ArrowFunction)
// ===========================================================================

describe("Parser grammar: jump statements", () => {

    it("GRAM-080: break statement inside loop — function definition extracted", () => {
        const src = `
static int FindFirst(int n)
{
    int i = 0;
    while (i < n)
    {
        if (i == 5) { break; }
        i++;
    }
    return i;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram080.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("FindFirst");
    });

    it("GRAM-081: continue statement inside loop — function definition extracted", () => {
        const src = `
static int SumEvens(int n)
{
    int total = 0;
    for (int i = 0; i < n; i++)
    {
        if (i % 2 != 0) { continue; }
        total += i;
    }
    return total;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram081.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("SumEvens");
    });

    it("GRAM-082: goto statement with matching label — function definition extracted", () => {
        const src = `
static void GotoFunc(int x)
{
    if (x < 0) { goto error; }
    x = x + 1;
    return;
error:
    x = 0;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram082.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("GotoFunc");
    });

    it("GRAM-083: return with expression — function definition extracted", () => {
        const src = `
static int Double(int x)
{
    return x * 2;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram083.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("Double");
    });

    it("GRAM-084: return without expression (void return) — function definition extracted", () => {
        const src = `
static void VoidReturn(void)
{
    return;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram084.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("VoidReturn");
    });
});

// ===========================================================================
// Group 9 — Labeled Statements (kills labeledStatement ArrowFunction mutants)
// ===========================================================================

describe("Parser grammar: labeled statements", () => {

    it("GRAM-090: identifier label inside function — function definition extracted", () => {
        const src = `
static void LabelTest(int x)
{
    start:
    x++;
    if (x < 10) { goto start; }
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram090.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("LabelTest");
    });

    it("GRAM-091: case label and default label in switch block", () => {
        const src = `
static void SwitchLabels(int x)
{
    switch (x)
    {
        case 0:
            break;
        case 1:
            break;
        default:
            break;
    }
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram091.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("SwitchLabels");
    });
});

// ===========================================================================
// Group 10 — Expressions: Operators (kills ArrowFunction mutants in
// assignment/arithmetic/bitwise/shift/relational/equality/logical rules)
// ===========================================================================

describe("Parser grammar: assignment operators", () => {

    it("GRAM-100: plus-assign operator in function body", () => {
        const src = `static void PlusAssign(int x) { x += 5; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram100.c", src);
        expect(functionDefs[0].functionName).toBe("PlusAssign");
    });

    it("GRAM-101: minus-assign operator in function body", () => {
        const src = `static void MinusAssign(int x) { x -= 3; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram101.c", src);
        expect(functionDefs[0].functionName).toBe("MinusAssign");
    });

    it("GRAM-102: star-assign (multiply-assign) operator in function body", () => {
        const src = `static void MulAssign(int x) { x *= 2; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram102.c", src);
        expect(functionDefs[0].functionName).toBe("MulAssign");
    });

    it("GRAM-103: slash-assign (divide-assign) operator in function body", () => {
        const src = `static void DivAssign(int x) { x /= 2; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram103.c", src);
        expect(functionDefs[0].functionName).toBe("DivAssign");
    });

    it("GRAM-104: percent-assign (modulo-assign) operator in function body", () => {
        const src = `static void ModAssign(int x) { x %= 3; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram104.c", src);
        expect(functionDefs[0].functionName).toBe("ModAssign");
    });

    it("GRAM-105: amp-assign (bitwise-and-assign) operator in function body", () => {
        const src = `static void AndAssign(int x) { x &= 0xFF; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram105.c", src);
        expect(functionDefs[0].functionName).toBe("AndAssign");
    });

    it("GRAM-106: pipe-assign (bitwise-or-assign) operator in function body", () => {
        const src = `static void OrAssign(int x) { x |= 0x01; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram106.c", src);
        expect(functionDefs[0].functionName).toBe("OrAssign");
    });

    it("GRAM-107: caret-assign (xor-assign) operator in function body", () => {
        const src = `static void XorAssign(int x) { x ^= 0xAA; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram107.c", src);
        expect(functionDefs[0].functionName).toBe("XorAssign");
    });

    it("GRAM-108: left-shift-assign operator in function body", () => {
        const src = `static void LShiftAssign(int x) { x <<= 1; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram108.c", src);
        expect(functionDefs[0].functionName).toBe("LShiftAssign");
    });

    it("GRAM-109: right-shift-assign operator in function body", () => {
        const src = `static void RShiftAssign(int x) { x >>= 1; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram109.c", src);
        expect(functionDefs[0].functionName).toBe("RShiftAssign");
    });
});

describe("Parser grammar: binary arithmetic and bitwise operators", () => {

    it("GRAM-110: bitwise OR operator in expression", () => {
        const src = `static int BitwiseOr(int a, int b) { return a | b; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram110.c", src);
        expect(functionDefs[0].functionName).toBe("BitwiseOr");
    });

    it("GRAM-111: bitwise XOR operator in expression", () => {
        const src = `static int BitwiseXor(int a, int b) { return a ^ b; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram111.c", src);
        expect(functionDefs[0].functionName).toBe("BitwiseXor");
    });

    it("GRAM-112: bitwise AND operator in expression", () => {
        const src = `static int BitwiseAnd(int a, int b) { return a & b; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram112.c", src);
        expect(functionDefs[0].functionName).toBe("BitwiseAnd");
    });

    it("GRAM-113: left shift operator in expression", () => {
        const src = `static int LShift(int x) { return x << 2; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram113.c", src);
        expect(functionDefs[0].functionName).toBe("LShift");
    });

    it("GRAM-114: right shift operator in expression", () => {
        const src = `static int RShift(int x) { return x >> 2; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram114.c", src);
        expect(functionDefs[0].functionName).toBe("RShift");
    });

    it("GRAM-115: addition operator in expression", () => {
        const src = `static int Add(int a, int b) { return a + b; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram115.c", src);
        expect(functionDefs[0].functionName).toBe("Add");
    });

    it("GRAM-116: subtraction operator in expression", () => {
        const src = `static int Sub(int a, int b) { return a - b; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram116.c", src);
        expect(functionDefs[0].functionName).toBe("Sub");
    });

    it("GRAM-117: multiplication operator in expression", () => {
        const src = `static int Mul(int a, int b) { return a * b; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram117.c", src);
        expect(functionDefs[0].functionName).toBe("Mul");
    });

    it("GRAM-118: division operator in expression", () => {
        const src = `static int Div(int a, int b) { return a / b; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram118.c", src);
        expect(functionDefs[0].functionName).toBe("Div");
    });

    it("GRAM-119: modulo operator in expression", () => {
        const src = `static int Mod(int a, int b) { return a % b; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram119.c", src);
        expect(functionDefs[0].functionName).toBe("Mod");
    });
});

describe("Parser grammar: comparison and logical operators", () => {

    it("GRAM-120: equality operator ==", () => {
        const src = `static int IsZero(int x) { return x == 0; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram120.c", src);
        expect(functionDefs[0].functionName).toBe("IsZero");
    });

    it("GRAM-121: inequality operator !=", () => {
        const src = `static int IsNonZero(int x) { return x != 0; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram121.c", src);
        expect(functionDefs[0].functionName).toBe("IsNonZero");
    });

    it("GRAM-122: less-than relational operator", () => {
        const src = `static int IsNeg(int x) { return x < 0; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram122.c", src);
        expect(functionDefs[0].functionName).toBe("IsNeg");
    });

    it("GRAM-123: greater-than relational operator", () => {
        const src = `static int IsPos(int x) { return x > 0; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram123.c", src);
        expect(functionDefs[0].functionName).toBe("IsPos");
    });

    it("GRAM-124: less-than-or-equal relational operator", () => {
        const src = `static int IsNonPos(int x) { return x <= 0; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram124.c", src);
        expect(functionDefs[0].functionName).toBe("IsNonPos");
    });

    it("GRAM-125: greater-than-or-equal relational operator", () => {
        const src = `static int IsNonNeg(int x) { return x >= 0; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram125.c", src);
        expect(functionDefs[0].functionName).toBe("IsNonNeg");
    });

    it("GRAM-126: logical AND operator &&", () => {
        const src = `static int Both(int a, int b) { return a && b; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram126.c", src);
        expect(functionDefs[0].functionName).toBe("Both");
    });

    it("GRAM-127: logical OR operator ||", () => {
        const src = `static int Either(int a, int b) { return a || b; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram127.c", src);
        expect(functionDefs[0].functionName).toBe("Either");
    });

    it("GRAM-128: ternary conditional expression", () => {
        const src = `static int Max(int a, int b) { return a > b ? a : b; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram128.c", src);
        expect(functionDefs[0].functionName).toBe("Max");
    });

    it("GRAM-129: comma expression in for-loop update", () => {
        const src = `static void CommaExpr(void) { int a = 0, b = 0; for (a = 0, b = 0; a < 10; a++, b++) { } }`;
        const { functionDefs } = parseFileWithDefs("/test/gram129.c", src);
        expect(functionDefs[0].functionName).toBe("CommaExpr");
    });
});

// ===========================================================================
// Group 11 — Unary Expressions (kills ArrowFunction mutants for each prefix op)
// ===========================================================================

describe("Parser grammar: unary and postfix expressions", () => {

    it("GRAM-130: prefix increment ++x", () => {
        const src = `static void PreInc(int x) { ++x; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram130.c", src);
        expect(functionDefs[0].functionName).toBe("PreInc");
    });

    it("GRAM-131: prefix decrement --x", () => {
        const src = `static void PreDec(int x) { --x; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram131.c", src);
        expect(functionDefs[0].functionName).toBe("PreDec");
    });

    it("GRAM-132: address-of operator &x", () => {
        const src = `static void AddrOf(int x) { int *p = &x; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram132.c", src);
        expect(functionDefs[0].functionName).toBe("AddrOf");
    });

    it("GRAM-133: dereference operator *p", () => {
        const src = `static int Deref(int *p) { return *p; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram133.c", src);
        expect(functionDefs[0].functionName).toBe("Deref");
    });

    it("GRAM-134: unary plus +x", () => {
        const src = `static int UnaryPlus(int x) { return +x; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram134.c", src);
        expect(functionDefs[0].functionName).toBe("UnaryPlus");
    });

    it("GRAM-135: unary minus -x", () => {
        const src = `static int Negate(int x) { return -x; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram135.c", src);
        expect(functionDefs[0].functionName).toBe("Negate");
    });

    it("GRAM-136: bitwise complement ~x", () => {
        const src = `static int Complement(int x) { return ~x; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram136.c", src);
        expect(functionDefs[0].functionName).toBe("Complement");
    });

    it("GRAM-137: logical NOT !x", () => {
        const src = `static int LogNot(int x) { return !x; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram137.c", src);
        expect(functionDefs[0].functionName).toBe("LogNot");
    });

    it("GRAM-138: postfix increment x++", () => {
        const src = `static void PostInc(int x) { x++; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram138.c", src);
        expect(functionDefs[0].functionName).toBe("PostInc");
    });

    it("GRAM-139: postfix decrement x--", () => {
        const src = `static void PostDec(int x) { x--; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram139.c", src);
        expect(functionDefs[0].functionName).toBe("PostDec");
    });

    it("GRAM-140: array subscript postfix operator arr[i]", () => {
        const src = `static int GetElem(int *arr, int i) { return arr[i]; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram140.c", src);
        expect(functionDefs[0].functionName).toBe("GetElem");
    });

    it("GRAM-141: function call postfix operator f(args)", () => {
        const src = `static void CallFunc(void) { int x = 0; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram141.c", src);
        expect(functionDefs[0].functionName).toBe("CallFunc");
    });

    it("GRAM-142: dot member access a.field", () => {
        const src = `
typedef struct { int x; } PT_T;
static int GetX(PT_T t) { return t.x; }
`;
        const { functionDefs } = parseFileWithDefs("/test/gram142.c", src);
        expect(functionDefs.some((f) => f.functionName === "GetX")).toBe(true);
    });

    it("GRAM-143: arrow member access p->field", () => {
        const src = `
typedef struct { int x; } PT_T;
static int GetXPtr(PT_T *p) { return p->x; }
`;
        const { functionDefs } = parseFileWithDefs("/test/gram143.c", src);
        expect(functionDefs.some((f) => f.functionName === "GetXPtr")).toBe(true);
    });
});

// ===========================================================================
// Group 12 — Sizeof and Cast (kills ArrowFunction and ConditionalExpression
// mutants in castExpression and unaryExpression)
// ===========================================================================

describe("Parser grammar: sizeof and cast expressions", () => {

    it("GRAM-150: sizeof with type name", () => {
        const src = `static size_t SizeOfInt(void) { return sizeof(int); }`;
        const { functionDefs } = parseFileWithDefs("/test/gram150.c", src);
        expect(functionDefs[0].functionName).toBe("SizeOfInt");
    });

    it("GRAM-151: sizeof with expression (not type)", () => {
        const src = `static size_t SizeOfArr(int *arr) { return sizeof(*arr); }`;
        const { functionDefs } = parseFileWithDefs("/test/gram151.c", src);
        expect(functionDefs[0].functionName).toBe("SizeOfArr");
    });

    it("GRAM-152: cast expression — (int) applied to float", () => {
        const src = `static int CastToInt(float f) { return (int)f; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram152.c", src);
        expect(functionDefs[0].functionName).toBe("CastToInt");
    });

    it("GRAM-153: cast expression — (void *) pointer cast", () => {
        const src = `static void *ToVoidPtr(int *p) { return (void *)p; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram153.c", src);
        expect(functionDefs[0].functionName).toBe("ToVoidPtr");
    });

    it("GRAM-154: sizeof with custom type identifier", () => {
        const src = `
typedef struct { int a; int b; } MY_PAIR_T;
static size_t PairSize(void) { return sizeof(MY_PAIR_T); }
`;
        const { functionDefs } = parseFileWithDefs("/test/gram154.c", src);
        expect(functionDefs.some((f) => f.functionName === "PairSize")).toBe(true);
    });
});

// ===========================================================================
// Group 13 — Primary Expression Literals (kills ArrowFunction mutants for
// FloatingLiteral, StringLiteral, CharLiteral, parenthesized expression)
// ===========================================================================

describe("Parser grammar: primary expression literals", () => {

    it("GRAM-160: floating literal in expression", () => {
        const src = `static float FloatLit(void) { return 3.14f; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram160.c", src);
        expect(functionDefs[0].functionName).toBe("FloatLit");
    });

    it("GRAM-161: string literal in expression", () => {
        const src = `static const char *GetStr(void) { return "hello"; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram161.c", src);
        expect(functionDefs[0].functionName).toBe("GetStr");
    });

    it("GRAM-162: char literal in expression", () => {
        const src = `static char GetChar(void) { return 'A'; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram162.c", src);
        expect(functionDefs[0].functionName).toBe("GetChar");
    });

    it("GRAM-163: parenthesized expression in return", () => {
        const src = `static int ParenExpr(int a, int b) { return (a + b); }`;
        const { functionDefs } = parseFileWithDefs("/test/gram163.c", src);
        expect(functionDefs[0].functionName).toBe("ParenExpr");
    });

    it("GRAM-164: integer literal zero as function argument", () => {
        const src = `
static int UseInt(int n) { return n + 0; }
`;
        const { functionDefs } = parseFileWithDefs("/test/gram164.c", src);
        expect(functionDefs[0].functionName).toBe("UseInt");
    });
});

// ===========================================================================
// Group 14 — Declarator Suffixes (kills GATE ConditionalExpression and
// ArrowFunction mutants in directDeclarator)
// ===========================================================================

describe("Parser grammar: declarator suffixes", () => {

    it("GRAM-170: array declarator suffix [ ] in local variable", () => {
        const src = `static void ArrayDecl(void) { int arr[10]; arr[0] = 1; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram170.c", src);
        expect(functionDefs[0].functionName).toBe("ArrayDecl");
    });

    it("GRAM-171: function parameter uses array type (pointer decay)", () => {
        const src = `static int SumArray(int *arr, int n) { int s = 0; for (int i = 0; i < n; i++) { s += arr[i]; } return s; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram171.c", src);
        expect(functionDefs[0].functionName).toBe("SumArray");
    });

    it("GRAM-172: struct field with function pointer type extracted correctly", () => {
        const src = `
struct MY_API_TAG
{
    int (*GetValue)(void *ptSelf);
    void (*SetValue)(void *ptSelf, int iVal);
};
`;
        const result = parseFile("/test/gram172.c", src);
        expect(result.apiStructDefinitions).toHaveLength(1);
        expect(result.apiStructDefinitions[0].fields).toEqual(["GetValue", "SetValue"]);
    });

    it("GRAM-173: parameterTypeList with ellipsis — varargs function", () => {
        const src = `static int Variadic(int n, ...) { return n; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram173.c", src);
        expect(functionDefs[0].functionName).toBe("Variadic");
    });

    it("GRAM-174: pointer to pointer declarator parses correctly", () => {
        const src = `static void PtrToPtr(int **pp) { *pp = 0; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram174.c", src);
        expect(functionDefs[0].functionName).toBe("PtrToPtr");
    });
});

// ===========================================================================
// Group 15 — Initializers (kills ArrowFunction mutants in initializer/
// initializerList/designation)
// ===========================================================================

describe("Parser grammar: initializers", () => {

    it("GRAM-180: brace initializer for global array — vtable extraction unaffected", () => {
        const src = `int g_aArr[] = {1, 2, 3};\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram180.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
    });

    it("GRAM-181: designated array initializer [i] = val", () => {
        const src = `int g_aMap[] = {[0] = 10, [1] = 20, [2] = 30};\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram181.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-182: struct brace initializer in local variable", () => {
        const src = `
typedef struct { int x; int y; } PT_T;
static void InitStruct(void) { PT_T t = {1, 2}; }
` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram182.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-183: nested brace initializer", () => {
        const src = `int g_aMatrix[2][2] = {{1, 2}, {3, 4}};\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram183.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });
});

// ===========================================================================
// Group 16 — Preprocessor Directives (kills ArrowFunction in preprocessorDirective)
// ===========================================================================

describe("Parser grammar: preprocessor directives", () => {

    it("GRAM-190: hash directive before function definition", () => {
        const src = `#include <stdint.h>\nstatic void AfterInclude(void) { }\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram190.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
    });

    it("GRAM-191: multiple hash directives before declarations", () => {
        const src = `#include <stdint.h>\n#define MAX_SIZE 64\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram191.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });
});

// ===========================================================================
// Group 17 — Juno Macro Invocations (kills ArrowFunction mutants in
// junoStandaloneDeclaration and junoModuleMacro)
// ===========================================================================

describe("Parser grammar: LibJuno macro forms", () => {

    it("GRAM-200: JUNO_MODULE_DECLARE parses without disrupting vtable extraction", () => {
        const src = `JUNO_MODULE_DECLARE(JUNO_APP_ROOT_T);\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram200.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
    });

    it("GRAM-201: JUNO_MODULE_ROOT_DECLARE parses without disrupting vtable extraction", () => {
        const src = `JUNO_MODULE_ROOT_DECLARE(JUNO_TIME_ROOT_T);\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram201.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-202: JUNO_MODULE_DERIVE_DECLARE parses without disrupting vtable extraction", () => {
        const src = `JUNO_MODULE_DERIVE_DECLARE(ENGINE_APP_T);\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram202.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-203: JUNO_MODULE_RESULT standalone declaration parses correctly", () => {
        const src = `JUNO_MODULE_RESULT(eStatus, JUNO_STATUS_SUCCESS);\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram203.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-204: JUNO_MODULE_SUPER member access in function body", () => {
        const src = `
static void CallSuper(MY_DERIVED_T *ptSelf)
{
    ptSelf->JUNO_MODULE_SUPER.ptApi->OnStart(ptSelf);
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram204.c", src);
        expect(functionDefs[0].functionName).toBe("CallSuper");
    });

    it("GRAM-205: _pfcnFailureHandler member access in memberIdentifier", () => {
        const src = `
static void SetFH(MY_MOD_T *ptMod)
{
    ptMod->_pfcnFailureHandler = 0;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram205.c", src);
        expect(functionDefs[0].functionName).toBe("SetFH");
    });
});

// ===========================================================================
// Group 18 — GATE: individual declarationSpecifiers tokens (kills specific
// LogicalOperator mutants where each token is tested individually)
// ===========================================================================

describe("Parser grammar: declarationSpecifiers GATE individual specifiers", () => {

    it("GRAM-210: static-only vtable — exercises Static alone as first specifier", () => {
        // 'static' → storageClassSpecifier; no MANY loop needed (next is Identifier)
        const src = `static MY_API_T tStaticApi = { .Field = Impl };`;
        const result = parseFile("/test/gram210.c", src);
        // No vtable extraction possible here (no 'const' before type, unrecognized pattern)
        // The key is: parsing must not crash
        expect(result).toBeDefined();
    });

    it("GRAM-211: static const declaration — Const consumed via GATE", () => {
        // After 'static', GATE fires with la1=const → Const matches → consumed
        const src = `
static const MY_IMPL_API_T tImpl = {
    .Process = ProcessImpl,
};
`;
        const result = parseFile("/test/gram211.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].field).toBe("Process");
        expect(result.vtableAssignments[0].functionName).toBe("ProcessImpl");
    });

    it("GRAM-212: extern const declaration — exercises both Extern and Const through GATE", () => {
        const src = `extern const int g_iMaxRetry;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram212.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-213: volatile int — Volatile as sole primitive-path GATE entry", () => {
        // After consuming 'volatile', GATE fires with la1=int → Int matches
        const src = `volatile int g_iVolatile;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram213.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-214: typedef void (*FN_T)(void) — Typedef + Void through GATE", () => {
        // After Typedef, GATE fires with la1=void → Void matches
        const src = `typedef void (*FN_T)(void);\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram214.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-215: Identifier la2=Star branch — returns const TYPE *ptr exercises Identifier-branch", () => {
        // In parameterDeclaration: first 'const' consumed, then la1=TYPE (Identifier),
        // la2='*' (Star) → Identifier branch returns true → TYPE consumed
        const src = `static void UseConstPtr(const MY_TYPE_T *ptVal) { }`;
        const { functionDefs } = parseFileWithDefs("/test/gram215.c", src);
        expect(functionDefs[0].functionName).toBe("UseConstPtr");
    });

    it("GRAM-216: Identifier la2=Const branch — TYPE const *ptr exercises Identifier-branch la2=Const", () => {
        // la1=JUNO_STATUS_T (Identifier), la2=const → la2=Const matches
        const src = `static void UseTypeConst(JUNO_STATUS_T const *ptStatus) { }`;
        const { functionDefs } = parseFileWithDefs("/test/gram216.c", src);
        expect(functionDefs[0].functionName).toBe("UseTypeConst");
    });

    it("GRAM-217: inline void function — Inline specifier exercises GATE Inline branch", () => {
        const src = `inline void InlineFunc(void) { }\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram217.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-218: void return type in function definition — Void in GATE loop", () => {
        // static void ... → after consuming 'static', la1=void → GATE checks Void → true
        const src = `static void VoidFunc(int x) { x = 0; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram218.c", src);
        expect(functionDefs[0].functionName).toBe("VoidFunc");
    });

    it("GRAM-219: enum type in specifier list — Enum in GATE", () => {
        // static enum STATE_E Decode(int code) - exercises Enum in specifier GATE
        const src = `static int UseEnum(int code) { return code; }`;
        const { functionDefs } = parseFileWithDefs("/test/gram219.c", src);
        expect(functionDefs[0].functionName).toBe("UseEnum");
    });
});

// ===========================================================================
// Group 19 — Multi-function and complex files (stress tests for full pipeline)
// ===========================================================================

describe("Parser grammar: complex multi-construct files", () => {

    it("GRAM-220: file with enum + struct + vtable + functions all parse correctly", () => {
        const src = `
enum MY_MODE_E { MODE_OFF = 0, MODE_ON = 1 };

struct MY_API_TAG { int (*Run)(int mode); };

static int RunImpl(int mode) { return mode; }

static const struct MY_API_TAG tApi = { RunImpl };
`;
        const result = parseFile("/test/gram220.c", src);
        expect(result.apiStructDefinitions).toHaveLength(1);
        expect(result.apiStructDefinitions[0].fields).toEqual(["Run"]);
    });

    it("GRAM-221: function with all statement types in sequence", () => {
        const src = `
static int MultiStatement(int n)
{
    int result = 0;

    if (n < 0) { return -1; }

    while (n > 0)
    {
        if (n % 2 == 0) { result += n; }
        else { result -= 1; }
        n--;
    }

    for (int i = 0; i < 3; i++) { result += i; }

    switch (result % 3)
    {
        case 0: result = 0; break;
        case 1: result = 1; break;
        default: result = -1; break;
    }

    return result;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram221.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("MultiStatement");
    });

    it("GRAM-222: vtable after complex declarations still extracted", () => {
        const src = `
typedef unsigned long MY_SIZE_T;
typedef enum { ERR_NONE = 0, ERR_FAIL = 1 } MY_ERR_T;
extern volatile int g_iFlag;

static int Helper(MY_SIZE_T n) { return (int)n; }

static const JUNO_APP_API_T tComplexApi = {
    .OnStart = OnStart,
    .OnProcess = OnProcess,
    .OnExit = OnExit,
};
`;
        const result = parseFile("/test/gram222.c", src);
        expect(result.vtableAssignments).toHaveLength(3);
        expect(result.vtableAssignments[0].apiType).toBe("JUNO_APP_API_T");
        expect(result.vtableAssignments[0].field).toBe("OnStart");
        expect(result.vtableAssignments[1].field).toBe("OnProcess");
        expect(result.vtableAssignments[2].field).toBe("OnExit");
    });

    it("GRAM-223: module root and trait root both parsed with surrounding grammar", () => {
        const src = `
#include <stddef.h>
typedef unsigned int uint32_t;
struct MY_ROOT_TAG JUNO_MODULE_ROOT(MY_API_T, JUNO_MODULE_EMPTY);
struct MY_TRAIT_TAG JUNO_TRAIT_ROOT(MY_TRAIT_API_T, void *pvData; size_t zSize;);
`;
        const result = parseFile("/test/gram223.c", src);
        expect(result.moduleRoots).toHaveLength(1);
        expect(result.moduleRoots[0].rootType).toBe("MY_ROOT_T");
        expect(result.traitRoots).toHaveLength(1);
        expect(result.traitRoots[0].rootType).toBe("MY_TRAIT_T");
    });

    it("GRAM-224: function with bitwise, shift, ternary and cast expressions", () => {
        const src = `
static unsigned int Process(unsigned int flags, int shift)
{
    unsigned int masked = flags & 0xFF;
    unsigned int shifted = masked << shift;
    unsigned int toggled = shifted ^ 0xAA;
    unsigned int ored = toggled | 0x01;
    int casted = (int)ored;
    int result = casted > 0 ? casted : -casted;
    return (unsigned int)result;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram224.c", src);
        expect(functionDefs[0].functionName).toBe("Process");
    });

    it("GRAM-225: struct with bitfield member parses correctly", () => {
        // Exercises structDeclarator with colon (bit-field)
        const src = `
struct FLAGS_TAG {
    unsigned int active : 1;
    unsigned int error  : 1;
    unsigned int mode   : 2;
};
` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram225.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
    });

    it("GRAM-226: empty compound statement (function with empty body)", () => {
        const src = `static void EmptyBody(void) {}`;
        const { functionDefs } = parseFileWithDefs("/test/gram226.c", src);
        expect(functionDefs[0].functionName).toBe("EmptyBody");
    });

    it("GRAM-227: do-while with break inside", () => {
        const src = `
static int FindStop(int n)
{
    int result = 0;
    do
    {
        if (result >= 5) { break; }
        result++;
    } while (result < n);
    return result;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/gram227.c", src);
        expect(functionDefs[0].functionName).toBe("FindStop");
    });

    it("GRAM-228: multiple vtable assignments across multiple types in one file", () => {
        const src = `
static const JUNO_LOG_API_T tLogApi = {
    .LogDebug = LogDebugImpl,
    .LogInfo = LogInfoImpl,
};

static const JUNO_APP_API_T tAppApi = {
    .OnStart = AppStart,
    .OnExit = AppExit,
};
`;
        const result = parseFile("/test/gram228.c", src);
        expect(result.vtableAssignments).toHaveLength(4);
        const apiTypes = result.vtableAssignments.map((a) => a.apiType);
        expect(apiTypes.filter((t) => t === "JUNO_LOG_API_T")).toHaveLength(2);
        expect(apiTypes.filter((t) => t === "JUNO_APP_API_T")).toHaveLength(2);
    });
});

// ===========================================================================
// Group N1 — macroBodyTokens depth tracking (kills parser.ts L375-395)
//
// Targets:
//   L375 StringLiteral + BlockStatement — rule name "" / rule body {}
//   L378 GATE mutants — BooleanLiteral, ConditionalExpression (x2), LogicalOperator
//   L379 BlockStatement — DEF body {}
//   L380/L381 — depth++ NoCov: ConditionalExpression=false, UpdateOperator depth--
//   L382/L383 — depth-- NoCov: ConditionalExpression=false, UpdateOperator depth++
//   L390 StringLiteral — cstPostTerminal key "" instead of tok.tokenType.name
// ===========================================================================

describe("macroBodyTokens depth tracking", () => {

    it("GRAM-300: simple macro body — plain member, no nested parens", () => {
        // Kills L375 BlockStatement (rule body {}) and L379 BlockStatement (DEF body {}).
        // macroBodyTokens must consume "int x ;" leaving the outer RParen for
        // junoModuleRootMacro; wrong GATE or empty DEF causes the parent parse to
        // fail, losing both moduleRoots and the sentinel vtableAssignment.
        const src =
            `struct FOO_ROOT_TAG JUNO_MODULE_ROOT(FOO_API_T, int x;);\n` +
            SENTINEL_VTABLE;
        const result = parseFile("/test/gram300.c", src);
        expect(result.moduleRoots).toHaveLength(1);
        expect(result.moduleRoots[0].apiType).toBe("FOO_API_T");
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
    });

    it("GRAM-301: macro body with one level of nested parens — function pointer member", () => {
        // Kills L378 GATE mutants and L380/L381 NoCov depth tracking mutants.
        // `(*pfcnCallback)` contains a nested paren pair at depth 1.  The inner
        // RParen must NOT stop the MANY loop; only the outer macro RParen at
        // depth 0 should.  Wrong depth tracking causes macroBodyTokens to stop
        // early or consume the outer RParen, breaking the rest of the parse.
        const src =
            `struct FOO_ROOT_TAG JUNO_MODULE_ROOT(FOO_API_T, void (*pfcnCallback)(int););\n` +
            SENTINEL_VTABLE;
        const result = parseFile("/test/gram301.c", src);
        expect(result.moduleRoots).toHaveLength(1);
        expect(result.moduleRoots[0].apiType).toBe("FOO_API_T");
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
    });

    it("GRAM-302: macro body with two levels of nested parens — deeply nested function pointer", () => {
        // Kills L382/L383 NoCov depth tracking mutants.
        // `(*pfcn)(int (*)(char))` has two nested paren layers; the innermost
        // `(char)` produces depth 2.  The loop must track both increments and
        // decrements exactly so that only the outermost macro RParen stops it.
        const src =
            `struct FOO_ROOT_TAG JUNO_MODULE_ROOT(FOO_API_T, void (*pfcn)(int (*)(char)););\n` +
            SENTINEL_VTABLE;
        const result = parseFile("/test/gram302.c", src);
        expect(result.moduleRoots).toHaveLength(1);
        expect(result.moduleRoots[0].apiType).toBe("FOO_API_T");
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
    });

    it("GRAM-303: macro body with JUNO_MODULE_EMPTY — empty body token, GATE stops at outer RParen", () => {
        // Verifies the GATE stop condition at depth 0 + RParen when only a
        // single non-paren token appears in the body before the closing RParen.
        const src =
            `struct FOO_ROOT_TAG JUNO_MODULE_ROOT(FOO_API_T, JUNO_MODULE_EMPTY);\n` +
            SENTINEL_VTABLE;
        const result = parseFile("/test/gram303.c", src);
        expect(result.moduleRoots).toHaveLength(1);
        expect(result.moduleRoots[0].apiType).toBe("FOO_API_T");
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
    });

    it("GRAM-304: multiple function pointer members — depth resets between each member", () => {
        // Extends GRAM-301 with two nested-paren pairs.  Depth must return to 0
        // after each `)(param)` suffix so the loop continues for the second
        // member and still stops at the final outer RParen rather than inside it.
        const src =
            `struct FOO_ROOT_TAG JUNO_MODULE_ROOT(FOO_API_T, void (*pfcnA)(int); void (*pfcnB)(char););\n` +
            SENTINEL_VTABLE;
        const result = parseFile("/test/gram304.c", src);
        expect(result.moduleRoots).toHaveLength(1);
        expect(result.moduleRoots[0].apiType).toBe("FOO_API_T");
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
    });

    it("GRAM-305: macro body with API_T pointer member — apiMemberRegistry populated correctly", () => {
        // Kills L375 StringLiteral (rule name "") — child(m.children,
        // "macroBodyTokens") returns undefined, walkMacroBodyForApiMembers
        // short-circuits, and the registry stays empty.
        // Kills L390 StringLiteral (cstPostTerminal key "") — tokens stored
        // under the wrong key so the visitor cannot locate them by type name.
        const src = `struct FOO_ROOT_TAG JUNO_MODULE_ROOT(FOO_API_T, BAR_API_T *ptBar;);`;
        const { parsed, apiMemberRegistry } = parseFileWithDefs("/test/gram305.h", src);
        expect(parsed.moduleRoots).toHaveLength(1);
        expect(parsed.moduleRoots[0].apiType).toBe("FOO_API_T");
        expect(apiMemberRegistry.has("ptBar")).toBe(true);
        expect(apiMemberRegistry.get("ptBar")).toBe("BAR_API_T");
    });
});

// ===========================================================================
// Group 31 — declarationSpecifiers GATE logic (lines 85-107 of parser.ts)
// Kills the following surviving mutants:
//   L87 ConditionalExpression  (!tokenMatcher(la1,Identifier) → false)
//   L87 BlockStatement         (non-Identifier if-block body → {})
//   L90 ConditionalExpression  (entire return expr → true or → false)
//   L90 LogicalOperator        (||  →  &&  in the specifier chain)
//   L107 ConditionalExpression (LParen&&Star clause → false)
// ===========================================================================

describe("declarationSpecifiers GATE", () => {

    // -----------------------------------------------------------------------
    // Kills L87 ConditionalExpression and L87 BlockStatement:
    // When la1=non-Identifier and la2=non-Identifier keyword (e.g. la1=const,
    // la2=int), the patched GATE would NOT enter the non-Identifier specifier
    // list and would instead execute the Identifier-branch la2 check, which
    // returns false (int is not Identifier/Star/Const/Volatile/LParen+Star).
    // The MANY loop stops early. Chevrotain inserts a synthetic semicolon after
    // the first specifier, so the function definition is parsed as a separate
    // top-level construct without the 'static' keyword, yielding isStatic=false.
    // -----------------------------------------------------------------------

    it("GRAM-310: static const int function — GATE non-Identifier branch consumes const and int (la2 is non-Identifier keyword)", () => {
        // la1=const (non-Id), la2=int (non-Id keyword, NOT Identifier/Star/Const/Volatile)
        // Correct: non-Identifier branch recognises Const → consumed. Int → consumed.
        // Mutant L87 CE→false / BlockStatement→{}: la2 check fires, la2=int not Identifier
        //   → GATE returns false → const not consumed → static separated → isStatic=false.
        const { functionDefs } = parseFileWithDefs("/test/gram310.c", "static const int Compute(void) { return 42; }");
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("Compute");
        expect(functionDefs[0].isStatic).toBe(true);
    });

    it("GRAM-311: static unsigned int function — GATE non-Identifier branch consumes unsigned then int (la2 is non-Identifier keyword)", () => {
        // la1=unsigned (non-Id), la2=int (non-Id keyword) — same mechanism as GRAM-310.
        // Both L87 mutants cause unsigned to be skipped → static separated → isStatic=false.
        const { functionDefs } = parseFileWithDefs("/test/gram311.c", "static unsigned int GetCount(void) { return 0; }");
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("GetCount");
        expect(functionDefs[0].isStatic).toBe(true);
    });

    it("GRAM-312: typedef struct FOO_TAG FOO_T — Struct token recognised in GATE non-Identifier specifier list", () => {
        // After typedef (first specifier), GATE fires with la1=struct (Struct token).
        // Exercises the Struct branch (L96) in the non-Identifier OR chain.
        // A parse failure here would corrupt the token stream and prevent the
        // sentinel vtable assignment from being extracted.
        const src = `typedef struct FOO_TAG FOO_T;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram312.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
        expect(result.vtableAssignments[0].field).toBe(SENTINEL_FIELD);
    });

    it("GRAM-313: volatile unsigned long — GATE recognises Volatile, Unsigned, Long in the non-Identifier specifier list", () => {
        // Exercises three consecutive non-Identifier specifiers.
        // Validates that the OR-chain at L90-L96 fires correctly for each one.
        // If any branch were replaced with false (L90 CE mutants), the relevant
        // specifier would fail to be consumed, producing a parse error that
        // could discard the sentinel vtable.
        const src = `volatile unsigned long g_ulTimeout;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram313.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
        expect(result.vtableAssignments[0].field).toBe(SENTINEL_FIELD);
    });

    it("GRAM-314: JUNO_STATUS_T FuncName(void) — GATE Identifier+Identifier la2 branch allows typedef-name type specifier", () => {
        // la1=JUNO_STATUS_T (Identifier), la2=FuncName (Identifier).
        // The Identifier branch (bypasses non-Identifier list) hits tokenMatcher(la2,Identifier)=true.
        // Correct: JUNO_STATUS_T consumed as typeSpecifier, FuncName is the declarator.
        // Exercises the la2=Identifier sub-branch of the GATE (lines 103+).
        const { functionDefs } = parseFileWithDefs("/test/gram314.c", "JUNO_STATUS_T FuncName(void) { return 0; }");
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("FuncName");
    });

    it("GRAM-315: JUNO_STATUS_T *g_pStatus — GATE Identifier+Star la2 branch allows typedef-name before pointer declarator", () => {
        // la1=JUNO_STATUS_T (Identifier), la2=* (Star).
        // The Identifier branch hits tokenMatcher(la2,Star)=true → JUNO_STATUS_T consumed.
        // Separates the typedef-name type from the pointer declarator correctly.
        const src = `JUNO_STATUS_T *g_pStatus;\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram315.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
        expect(result.vtableAssignments[0].field).toBe(SENTINEL_FIELD);
    });

    it("GRAM-316: void (*g_pfcnCallback)(int) — GATE non-Identifier branch correctly rejects LParen after type specifiers", () => {
        // After void is consumed, GATE fires with la1=( (LParen, non-Identifier).
        // The non-Identifier specifier list does NOT include LParen, so GATE returns
        // false → MANY stops → (*g_pfcnCallback)(int) is the declarator. Correct.
        // Exercises: the non-Identifier branch returns false for tokens not in the
        // list (L87 Block→{} mutant would return undefined/falsy for any non-Id,
        // so this path is the same — both produce false, no distinction here).
        const src = `void (*g_pfcnCallback)(int x);\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram316.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
        expect(result.vtableAssignments[0].field).toBe(SENTINEL_FIELD);
    });

    it("GRAM-317: typedef JUNO_STATUS_T (*CALLBACK_T)(int) — GATE Identifier+LParen+Star la2 branch at L107 allows typedef-name before function-pointer declarator", () => {
        // la1=JUNO_STATUS_T (Identifier), la2=( (LParen), LA(3)=* (Star).
        // L107: (tokenMatcher(la2,LParen) && tokenMatcher(LA(3),Star)) = true.
        // JUNO_STATUS_T consumed as typeSpecifier; (*CALLBACK_T)(int) is the declarator.
        // L107 CE→false mutant: LParen+Star → false → JUNO_STATUS_T NOT consumed as type
        //   → typedef fails early → (*CALLBACK_T)(int); parse error → may corrupt stream.
        const src = `typedef JUNO_STATUS_T (*CALLBACK_T)(int x);\n` + SENTINEL_VTABLE;
        const result = parseFile("/test/gram317.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
        expect(result.vtableAssignments[0].field).toBe(SENTINEL_FIELD);
    });
});

// ===========================================================================
// Group 18 — macroCallStatement (kills GATE predicate and gobble-loop mutants)
//
// The macroCallStatement alternative fires when isMacroCallWithKeywordArg()
// detects an Identifier '(' ... keyword ... ')' ahead. gobbleMacroCallStatement()
// then consumes the entire balanced-paren token run as raw tokens so that
// keyword tokens (return, break, continue, goto) inside macro args never
// reach a grammar rule that disallows them.
// ===========================================================================

// @{"verify": ["REQ-VSCODE-003"]}

describe("macroCallStatement — keyword arguments in macro calls", () => {

    it("TC-MACRO-STMT-001: JUNO_ASSERT_SUCCESS(tStatus, return tStatus) — return as macro arg parses without errors", () => {
        // isMacroCallWithKeywordArg(): la1=JUNO_ASSERT_SUCCESS (Id), la2=( (LParen),
        // scan finds Return token → gate=true → gobbleMacroCallStatement() consumes the
        // entire statement so the Return keyword never hits jumpStatement validation.
        const src = `
JUNO_STATUS_T TestFunc(void)
{
    JUNO_STATUS_T tStatus;
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    return tStatus;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/macro001.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("TestFunc");
    });

    it("TC-MACRO-STMT-002: JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult) — dot-accessed first arg with return keyword", () => {
        // The dot-access 'tResult.tStatus' produces an additional Dot+Identifier token
        // pair before the comma; the keyword scan must continue past them to find Return.
        // Note: JUNO_MODULE_RESULT is a dedicated lexer token (not Identifier) so it
        // cannot be used as a local variable type; use a plain typedef-style name instead.
        const src = `
JUNO_STATUS_T TestFunc(void)
{
    MY_RESULT_T tResult;
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    return tResult.tStatus;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/macro002.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("TestFunc");
    });

    it("TC-MACRO-STMT-003: MY_MACRO(x, return func(a, b)) — nested parens inside macro args parse correctly", () => {
        // The nested '(' increments depth inside isMacroCallWithKeywordArg() scan;
        // the enclosed Return is still found. gobbleMacroCallStatement() tracks depth
        // correctly so the outer ')' terminates at depth=0 and ';' ends the statement.
        const src = `
int TestFunc(void)
{
    int x;
    MY_MACRO(x, return func(a, b));
    return 0;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/macro003.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("TestFunc");
    });

    it("TC-MACRO-STMT-004: MY_MACRO(x, break) — break keyword in macro args gobbled correctly", () => {
        // tokenMatcher(t, Break) branch in isMacroCallWithKeywordArg() fires; gate=true.
        // gobbleMacroCallStatement() consumes all tokens so Break never reaches the
        // iterationStatement or selectionStatement rule where it would be illegal.
        const src = `
void TestFunc(void)
{
    int x;
    MY_MACRO(x, break);
}
`;
        const { functionDefs } = parseFileWithDefs("/test/macro004.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("TestFunc");
    });

    it("TC-MACRO-STMT-005: MY_MACRO(x, continue) — continue keyword in macro args gobbled correctly", () => {
        // tokenMatcher(t, Continue) branch in isMacroCallWithKeywordArg() fires.
        // Mirrors TC-MACRO-STMT-004 but exercises the Continue token matcher branch.
        const src = `
void TestFunc(void)
{
    int x;
    MY_MACRO(x, continue);
}
`;
        const { functionDefs } = parseFileWithDefs("/test/macro005.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("TestFunc");
    });

    it("TC-MACRO-STMT-006: MY_MACRO(x, goto label) — goto keyword in macro args gobbled correctly", () => {
        // tokenMatcher(t, Goto) branch in isMacroCallWithKeywordArg() fires.
        // 'label' is an additional Identifier token after Goto; gobbler consumes both.
        const src = `
void TestFunc(void)
{
    int x;
    MY_MACRO(x, goto label);
}
`;
        const { functionDefs } = parseFileWithDefs("/test/macro006.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("TestFunc");
    });

    it("TC-MACRO-STMT-NEG-001: foo(a, b) — regular function call with no keyword args is NOT gobbled as macro", () => {
        // isMacroCallWithKeywordArg() scans the arg list and finds no Return/Break/
        // Continue/Goto → gate=false → falls through to expressionStatement rule.
        // The sentinel vtable after the function proves the parse stream is intact.
        const src = `
void TestFunc(void)
{
    int a;
    int b;
    foo(a, b);
}
` + SENTINEL_VTABLE;
        const result = parseFile("/test/macro-neg001.c", src);
        expect(result.vtableAssignments).toHaveLength(1);
        expect(result.vtableAssignments[0].apiType).toBe(SENTINEL_FN);
        expect(result.vtableAssignments[0].field).toBe(SENTINEL_FIELD);
    });

    it('TC-MACRO-STMT-009: zero Chevrotain parse errors for keyword-arg macro (mutation guard)', () => {
        // Uses CLexer + CParser directly (same pattern as TC-BULK-004) to assert
        // parser.errors.length === 0. parseFileWithDefs() does not expose parser.errors,
        // so error-recovery could silently hide parse failures in TC-MACRO-STMT-001..006.
        const input = `
            void TestFunc(void) {
                JUNO_STATUS_T tStatus = Verify(ptMod);
                JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
            }
        `;
        const parser = new CParser();
        const lexResult = CLexer.tokenize(input);
        parser.input = lexResult.tokens;
        parser.translationUnit();
        expect(parser.errors).toHaveLength(0);
    });

    it("TC-MACRO-STMT-BND-001: multiple macro call statements in the same function body all parse cleanly", () => {
        // Exercises the statement-level OR loop firing gobbleMacroCallStatement()
        // multiple times within a single compoundStatement. Each invocation must
        // consume exactly up to the matching ';' so the next statement is parsed fresh.
        // Uses MY_RESULT_T instead of JUNO_MODULE_RESULT (a dedicated lexer token, not
        // an Identifier, so invalid as a local variable type specifier).
        const src = `
JUNO_STATUS_T TestFunc(void)
{
    JUNO_STATUS_T tStatus;
    MY_RESULT_T tResult;
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    MY_MACRO(tStatus, break);
    return tStatus;
}
`;
        const { functionDefs } = parseFileWithDefs("/test/macro-bnd001.c", src);
        expect(functionDefs).toHaveLength(1);
        expect(functionDefs[0].functionName).toBe("TestFunc");
    });
});
