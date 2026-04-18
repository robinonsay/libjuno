/// <reference types="jest" />

/**
 * @file integration.test.ts
 *
 * End-to-end integration tests for the full LibJuno navigation pipeline:
 *   WorkspaceIndexer (real disk I/O → Chevrotain parse → mergeInto) →
 *   VtableResolver / FailureHandlerResolver (resolve()).
 *
 * No mocks. No manual NavigationIndex population. Synthetic C source files are
 * written to a temporary directory, indexed through the real WorkspaceIndexer
 * public API, and then resolved through the real resolver classes.
 *
 * TC-INT-001  Single-file vtable resolution (REQ-VSCODE-002)
 * TC-INT-002  Two-file cross-source vtable resolution (REQ-VSCODE-002)
 * TC-INT-003  Failure handler assignment resolution (REQ-VSCODE-016)
 * TC-INT-004  Multi-hop derivation chain resolution (REQ-VSCODE-002, REQ-VSCODE-009)
 * TC-INT-005  Two independent modules in one file (REQ-VSCODE-002)
 * TC-INT-006  No vtable patterns — graceful found:false (REQ-VSCODE-002, REQ-VSCODE-004)
 */

import * as fs from 'fs';
import * as path from 'path';
import * as os from 'os';
import { WorkspaceIndexer } from '../indexer/workspaceIndexer';
import { VtableResolver } from '../resolver/vtableResolver';
import { FailureHandlerResolver } from '../resolver/failureHandlerResolver';

// ---------------------------------------------------------------------------
// REQ-VSCODE-002 + REQ-VSCODE-016 — End-to-End Integration Tests
// ---------------------------------------------------------------------------

// @{"verify": ["REQ-VSCODE-002", "REQ-VSCODE-016"]}
describe('Integration — End-to-End Navigation Pipeline', () => {

    let tempDir: string;

    // TC-INT-001 state
    let file001: string;
    let vtRes001: VtableResolver;

    // TC-INT-002 state
    let file002B: string;
    let vtRes002: VtableResolver;

    // TC-INT-003 state
    let file003B: string;
    let fhRes003: FailureHandlerResolver;

    // TC-INT-004 state
    let file004: string;
    let vtRes004: VtableResolver;

    // TC-INT-005 state
    let file005: string;
    let vtRes005: VtableResolver;

    // TC-INT-006 state
    let file006: string;
    let vtRes006: VtableResolver;

    // -----------------------------------------------------------------------
    // Fixture setup
    // -----------------------------------------------------------------------

    beforeAll(async () => {
        tempDir = fs.mkdtempSync(path.join(os.tmpdir(), 'libjuno-int-'));

        // -------------------------------------------------------------------
        // TC-INT-001 — single C file
        //
        // Line layout (1-based):
        //   1  typedef struct FOO_API_TAG FOO_API_T;
        //   2  typedef struct FOO_ROOT_TAG FOO_ROOT_T;
        //   3  (empty)
        //   4  struct FOO_ROOT_TAG JUNO_MODULE_ROOT(FOO_API_T, );
        //   5  (empty)
        //   6  struct FOO_API_TAG
        //   7  {
        //   8      JUNO_STATUS_T (*DoThing)(FOO_ROOT_T *ptFoo);
        //   9  };
        //   10 (empty)
        //   11 static JUNO_STATUS_T DoThing_Impl(FOO_ROOT_T *ptFoo)   <- fn def
        //   12 {
        //   13     return JUNO_STATUS_SUCCESS;
        //   14 }
        //   15 (empty)
        //   16 static const FOO_API_T tFooApi = {
        //   17     .DoThing = DoThing_Impl,
        //   18 };
        //   19 (empty)
        //   20 void UserFunc(FOO_ROOT_T *ptFoo)                        <- fn def
        //   21 {
        //   22     ptFoo->ptApi->DoThing(ptFoo);                       <- call site
        //   23 }
        // -------------------------------------------------------------------

        const src001 = [
            'typedef struct FOO_API_TAG FOO_API_T;',
            'typedef struct FOO_ROOT_TAG FOO_ROOT_T;',
            '',
            'struct FOO_ROOT_TAG JUNO_MODULE_ROOT(FOO_API_T, );',
            '',
            'struct FOO_API_TAG',
            '{',
            '    JUNO_STATUS_T (*DoThing)(FOO_ROOT_T *ptFoo);',
            '};',
            '',
            'static JUNO_STATUS_T DoThing_Impl(FOO_ROOT_T *ptFoo)',
            '{',
            '    return JUNO_STATUS_SUCCESS;',
            '}',
            '',
            'static const FOO_API_T tFooApi = {',
            '    .DoThing = DoThing_Impl,',
            '};',
            '',
            'void UserFunc(FOO_ROOT_T *ptFoo)',
            '{',
            '    ptFoo->ptApi->DoThing(ptFoo);',
            '}',
            '',
        ].join('\n');

        const dir001 = path.join(tempDir, 'tc001');
        fs.mkdirSync(dir001);
        file001 = path.join(dir001, 'int001.c');
        fs.writeFileSync(file001, src001);

        const indexer001 = new WorkspaceIndexer(dir001, []);
        await indexer001.reindexFile(file001);
        vtRes001 = new VtableResolver(indexer001.index);

        // -------------------------------------------------------------------
        // TC-INT-002 — two C files (header-like A, impl B)
        //
        // File A line layout:
        //   1  typedef struct BAZ_API_TAG BAZ_API_T;
        //   2  typedef struct BAZ_ROOT_TAG BAZ_ROOT_T;
        //   3  (empty)
        //   4  struct BAZ_ROOT_TAG JUNO_MODULE_ROOT(BAZ_API_T, );
        //   5  (empty)
        //   6  struct BAZ_API_TAG
        //   7  {
        //   8      JUNO_STATUS_T (*Process)(BAZ_ROOT_T *ptBaz);
        //   9  };
        //
        // File B line layout:
        //   1  static JUNO_STATUS_T Process_Impl(BAZ_ROOT_T *ptBaz)   <- fn def
        //   2  {
        //   3      return JUNO_STATUS_SUCCESS;
        //   4  }
        //   5  (empty)
        //   6  static const BAZ_API_T tBazApi = {
        //   7      .Process = Process_Impl,
        //   8  };
        //   9  (empty)
        //   10 void RunBaz(BAZ_ROOT_T *ptBaz)                          <- fn def
        //   11 {
        //   12     ptBaz->ptApi->Process(ptBaz);                       <- call site
        //   13 }
        // -------------------------------------------------------------------

        const srcA002 = [
            'typedef struct BAZ_API_TAG BAZ_API_T;',
            'typedef struct BAZ_ROOT_TAG BAZ_ROOT_T;',
            '',
            'struct BAZ_ROOT_TAG JUNO_MODULE_ROOT(BAZ_API_T, );',
            '',
            'struct BAZ_API_TAG',
            '{',
            '    JUNO_STATUS_T (*Process)(BAZ_ROOT_T *ptBaz);',
            '};',
            '',
        ].join('\n');

        const srcB002 = [
            'static JUNO_STATUS_T Process_Impl(BAZ_ROOT_T *ptBaz)',
            '{',
            '    return JUNO_STATUS_SUCCESS;',
            '}',
            '',
            'static const BAZ_API_T tBazApi = {',
            '    .Process = Process_Impl,',
            '};',
            '',
            'void RunBaz(BAZ_ROOT_T *ptBaz)',
            '{',
            '    ptBaz->ptApi->Process(ptBaz);',
            '}',
            '',
        ].join('\n');

        const dir002 = path.join(tempDir, 'tc002');
        fs.mkdirSync(dir002);
        const file002A = path.join(dir002, 'int002_a.c');
        file002B = path.join(dir002, 'int002_b.c');
        fs.writeFileSync(file002A, srcA002);
        fs.writeFileSync(file002B, srcB002);

        // Index file A first so moduleRoots is populated before file B processes
        // its vtable assignments.
        const indexer002 = new WorkspaceIndexer(dir002, []);
        await indexer002.reindexFile(file002A);
        await indexer002.reindexFile(file002B);
        vtRes002 = new VtableResolver(indexer002.index);

        // -------------------------------------------------------------------
        // TC-INT-003 — two C files (module root A, handler impl B)
        //
        // File A line layout:
        //   1  typedef struct QUX_API_TAG QUX_API_T;
        //   2  typedef struct QUX_ROOT_TAG QUX_ROOT_T;
        //   3  (empty)
        //   4  struct QUX_ROOT_TAG JUNO_MODULE_ROOT(QUX_API_T, );
        //   5  (empty)
        //   6  struct QUX_API_TAG
        //   7  {
        //   8      JUNO_STATUS_T (*Init)(QUX_ROOT_T *ptQux);
        //   9  };
        //
        // File B line layout:
        //   1  static void MyHandler(QUX_ROOT_T *ptQux, JUNO_STATUS_T eStatus)  <- fn def
        //   2  {
        //   3  }
        //   4  (empty)
        //   5  void QuxSetup(QUX_ROOT_T *ptQux)
        //   6  {
        //   7      ptQux->JUNO_FAILURE_HANDLER = MyHandler;                      <- handler assign
        //   8  }
        // -------------------------------------------------------------------

        const srcA003 = [
            'typedef struct QUX_API_TAG QUX_API_T;',
            'typedef struct QUX_ROOT_TAG QUX_ROOT_T;',
            '',
            'struct QUX_ROOT_TAG JUNO_MODULE_ROOT(QUX_API_T, );',
            '',
            'struct QUX_API_TAG',
            '{',
            '    JUNO_STATUS_T (*Init)(QUX_ROOT_T *ptQux);',
            '};',
            '',
        ].join('\n');

        const srcB003 = [
            'static void MyHandler(QUX_ROOT_T *ptQux, JUNO_STATUS_T eStatus)',
            '{',
            '}',
            '',
            'void QuxSetup(QUX_ROOT_T *ptQux)',
            '{',
            '    ptQux->JUNO_FAILURE_HANDLER = MyHandler;',
            '}',
            '',
        ].join('\n');

        const dir003 = path.join(tempDir, 'tc003');
        fs.mkdirSync(dir003);
        const file003A = path.join(dir003, 'int003_a.c');
        file003B = path.join(dir003, 'int003_b.c');
        fs.writeFileSync(file003A, srcA003);
        fs.writeFileSync(file003B, srcB003);

        // Index file A first so resolveFailureHandlerRootType finds QUX_ROOT_T
        // in moduleRoots when merging file B's failureHandlerAssigns.
        const indexer003 = new WorkspaceIndexer(dir003, []);
        await indexer003.reindexFile(file003A);
        await indexer003.reindexFile(file003B);
        fhRes003 = new FailureHandlerResolver(indexer003.index);

        // -------------------------------------------------------------------
        // TC-INT-004 — multi-hop derivation chain (LEAF → DERIVED → ROOT)
        //
        // Line layout (1-based):
        //   1  typedef struct MOTOR_API_TAG MOTOR_API_T;
        //   2  typedef struct MOTOR_ROOT_TAG MOTOR_ROOT_T;
        //   3  typedef struct MOTOR_DERIVED_TAG MOTOR_DERIVED_T;
        //   4  typedef struct MOTOR_LEAF_TAG MOTOR_LEAF_T;
        //   5  (empty)
        //   6  struct MOTOR_ROOT_TAG JUNO_MODULE_ROOT(MOTOR_API_T, );
        //   7  (empty)
        //   8  struct MOTOR_DERIVED_TAG JUNO_MODULE_DERIVE(MOTOR_ROOT_T, );
        //   9  (empty)
        //   10 struct MOTOR_LEAF_TAG JUNO_MODULE_DERIVE(MOTOR_DERIVED_T, );
        //   11 (empty)
        //   12 struct MOTOR_API_TAG
        //   13 {
        //   14     JUNO_STATUS_T (*Spin)(MOTOR_ROOT_T *ptMotor);
        //   15 };
        //   16 (empty)
        //   17 static JUNO_STATUS_T Motor_Spin(MOTOR_ROOT_T *ptMotor)  <- fn def
        //   18 {
        //   19     return JUNO_STATUS_SUCCESS;
        //   20 }
        //   21 (empty)
        //   22 static const MOTOR_API_T tMotorLeafApi = {
        //   23     .Spin = Motor_Spin,
        //   24 };
        //   25 (empty)
        //   26 void MotorUser(MOTOR_LEAF_T *ptLeafMotor)               <- fn def
        //   27 {
        //   28     ptLeafMotor->ptApi->Spin(ptLeafMotor);              <- call site
        //   29 }
        // -------------------------------------------------------------------

        const src004 = [
            'typedef struct MOTOR_API_TAG MOTOR_API_T;',
            'typedef struct MOTOR_ROOT_TAG MOTOR_ROOT_T;',
            'typedef struct MOTOR_DERIVED_TAG MOTOR_DERIVED_T;',
            'typedef struct MOTOR_LEAF_TAG MOTOR_LEAF_T;',
            '',
            'struct MOTOR_ROOT_TAG JUNO_MODULE_ROOT(MOTOR_API_T, );',
            '',
            'struct MOTOR_DERIVED_TAG JUNO_MODULE_DERIVE(MOTOR_ROOT_T, );',
            '',
            'struct MOTOR_LEAF_TAG JUNO_MODULE_DERIVE(MOTOR_DERIVED_T, );',
            '',
            'struct MOTOR_API_TAG',
            '{',
            '    JUNO_STATUS_T (*Spin)(MOTOR_ROOT_T *ptMotor);',
            '};',
            '',
            'static JUNO_STATUS_T Motor_Spin(MOTOR_ROOT_T *ptMotor)',
            '{',
            '    return JUNO_STATUS_SUCCESS;',
            '}',
            '',
            'static const MOTOR_API_T tMotorLeafApi = {',
            '    .Spin = Motor_Spin,',
            '};',
            '',
            'void MotorUser(MOTOR_LEAF_T *ptLeafMotor)',
            '{',
            '    ptLeafMotor->ptApi->Spin(ptLeafMotor);',
            '}',
            '',
        ].join('\n');

        const dir004 = path.join(tempDir, 'tc004');
        fs.mkdirSync(dir004);
        file004 = path.join(dir004, 'int004.c');
        fs.writeFileSync(file004, src004);

        const indexer004 = new WorkspaceIndexer(dir004, []);
        await indexer004.reindexFile(file004);
        vtRes004 = new VtableResolver(indexer004.index);

        // -------------------------------------------------------------------
        // TC-INT-005 — two independent modules in a single file
        //
        // Line layout (1-based):
        //   1  typedef struct LED_API_TAG LED_API_T;
        //   2  typedef struct LED_ROOT_TAG LED_ROOT_T;
        //   3  (empty)
        //   4  struct LED_ROOT_TAG JUNO_MODULE_ROOT(LED_API_T, );
        //   5  (empty)
        //   6  struct LED_API_TAG
        //   7  {
        //   8      JUNO_STATUS_T (*TurnOn)(LED_ROOT_T *ptLed);
        //   9  };
        //   10 (empty)
        //   11 static JUNO_STATUS_T Led_TurnOn(LED_ROOT_T *ptLed)     <- fn def
        //   12 {
        //   13     return JUNO_STATUS_SUCCESS;
        //   14 }
        //   15 (empty)
        //   16 static const LED_API_T tLedApi = {
        //   17     .TurnOn = Led_TurnOn,
        //   18 };
        //   19 (empty)
        //   20 typedef struct BUZZER_API_TAG BUZZER_API_T;
        //   21 typedef struct BUZZER_ROOT_TAG BUZZER_ROOT_T;
        //   22 (empty)
        //   23 struct BUZZER_ROOT_TAG JUNO_MODULE_ROOT(BUZZER_API_T, );
        //   24 (empty)
        //   25 struct BUZZER_API_TAG
        //   26 {
        //   27     JUNO_STATUS_T (*Beep)(BUZZER_ROOT_T *ptBuzzer);
        //   28 };
        //   29 (empty)
        //   30 static JUNO_STATUS_T Buzzer_Beep(BUZZER_ROOT_T *ptBuzzer) <- fn def
        //   31 {
        //   32     return JUNO_STATUS_SUCCESS;
        //   33 }
        //   34 (empty)
        //   35 static const BUZZER_API_T tBuzzerApi = {
        //   36     .Beep = Buzzer_Beep,
        //   37 };
        //   38 (empty)
        //   39 void DeviceUser(LED_ROOT_T *ptLed, BUZZER_ROOT_T *ptBuzzer) <- fn def
        //   40 {
        //   41     ptLed->ptApi->TurnOn(ptLed);                        <- LED call site
        //   42     ptBuzzer->ptApi->Beep(ptBuzzer);                    <- BUZZER call site
        //   43 }
        // -------------------------------------------------------------------

        const src005 = [
            'typedef struct LED_API_TAG LED_API_T;',
            'typedef struct LED_ROOT_TAG LED_ROOT_T;',
            '',
            'struct LED_ROOT_TAG JUNO_MODULE_ROOT(LED_API_T, );',
            '',
            'struct LED_API_TAG',
            '{',
            '    JUNO_STATUS_T (*TurnOn)(LED_ROOT_T *ptLed);',
            '};',
            '',
            'static JUNO_STATUS_T Led_TurnOn(LED_ROOT_T *ptLed)',
            '{',
            '    return JUNO_STATUS_SUCCESS;',
            '}',
            '',
            'static const LED_API_T tLedApi = {',
            '    .TurnOn = Led_TurnOn,',
            '};',
            '',
            'typedef struct BUZZER_API_TAG BUZZER_API_T;',
            'typedef struct BUZZER_ROOT_TAG BUZZER_ROOT_T;',
            '',
            'struct BUZZER_ROOT_TAG JUNO_MODULE_ROOT(BUZZER_API_T, );',
            '',
            'struct BUZZER_API_TAG',
            '{',
            '    JUNO_STATUS_T (*Beep)(BUZZER_ROOT_T *ptBuzzer);',
            '};',
            '',
            'static JUNO_STATUS_T Buzzer_Beep(BUZZER_ROOT_T *ptBuzzer)',
            '{',
            '    return JUNO_STATUS_SUCCESS;',
            '}',
            '',
            'static const BUZZER_API_T tBuzzerApi = {',
            '    .Beep = Buzzer_Beep,',
            '};',
            '',
            'void DeviceUser(LED_ROOT_T *ptLed, BUZZER_ROOT_T *ptBuzzer)',
            '{',
            '    ptLed->ptApi->TurnOn(ptLed);',
            '    ptBuzzer->ptApi->Beep(ptBuzzer);',
            '}',
            '',
        ].join('\n');

        const dir005 = path.join(tempDir, 'tc005');
        fs.mkdirSync(dir005);
        file005 = path.join(dir005, 'int005.c');
        fs.writeFileSync(file005, src005);

        const indexer005 = new WorkspaceIndexer(dir005, []);
        await indexer005.reindexFile(file005);
        vtRes005 = new VtableResolver(indexer005.index);

        // -------------------------------------------------------------------
        // TC-INT-006 — plain C file with no LibJuno vtable patterns
        //
        // Line layout (1-based):
        //   1  static int add(int a, int b) { return a + b; }
        //   2  void compute(void)
        //   3  {
        //   4      int result = add(1, 2);                             <- call site
        //   5  }
        // -------------------------------------------------------------------

        const src006 = [
            'static int add(int a, int b) { return a + b; }',
            'void compute(void)',
            '{',
            '    int result = add(1, 2);',
            '}',
            '',
        ].join('\n');

        const dir006 = path.join(tempDir, 'tc006');
        fs.mkdirSync(dir006);
        file006 = path.join(dir006, 'int006.c');
        fs.writeFileSync(file006, src006);

        const indexer006 = new WorkspaceIndexer(dir006, []);
        await indexer006.reindexFile(file006);
        vtRes006 = new VtableResolver(indexer006.index);
    });

    afterAll(() => {
        fs.rmSync(tempDir, { recursive: true, force: true });
    });

    // -----------------------------------------------------------------------
    // TC-INT-001 — Single-file vtable resolution
    // -----------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-002"]}
    it('TC-INT-001: resolves vtable call to concrete implementation within a single indexed file', () => {
        // Line 22 of int001.c: "    ptFoo->ptApi->DoThing(ptFoo);"
        // Pass the trimmed line so col=0 is within the Strategy-3 match span.
        const result = vtRes001.resolve(
            file001,
            22,
            0,
            'ptFoo->ptApi->DoThing(ptFoo);',
        );

        expect(result.found).toBe(true);
        expect(result.locations).toHaveLength(1);
        expect(result.locations[0].functionName).toBe('DoThing_Impl');
        expect(result.locations[0].file).toBe(file001);
        // DoThing_Impl identifier is on line 11 of int001.c (empirically verified).
        expect(result.locations[0].line).toBe(11);
    });

    // -----------------------------------------------------------------------
    // TC-INT-002 — Two-file cross-source vtable resolution
    // -----------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-002"]}
    it('TC-INT-002: resolves cross-file vtable call — moduleRoots from file A, implementation from file B', () => {
        // Line 12 of int002_b.c: "    ptBaz->ptApi->Process(ptBaz);"
        const result = vtRes002.resolve(
            file002B,
            12,
            0,
            'ptBaz->ptApi->Process(ptBaz);',
        );

        expect(result.found).toBe(true);
        expect(result.locations).toHaveLength(1);
        expect(result.locations[0].functionName).toBe('Process_Impl');
        // Implementation is sourced from file B, not file A.
        expect(result.locations[0].file).toBe(file002B);
        // Process_Impl identifier is on line 1 of int002_b.c (empirically verified).
        expect(result.locations[0].line).toBe(1);
    });

    // -----------------------------------------------------------------------
    // TC-INT-003 — Failure handler assignment navigation
    // -----------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-016"]}
    it('TC-INT-003: resolves JUNO_FAILURE_HANDLER assignment to the concrete handler function definition', () => {
        // Line 7 of int003_b.c: "    ptQux->JUNO_FAILURE_HANDLER = MyHandler;"
        // The assignment form is matched by ASSIGNMENT_RE; the resolver navigates
        // directly to the RHS function definition via functionDefinitions.
        const result = fhRes003.resolve(
            file003B,
            7,
            0,
            'ptQux->JUNO_FAILURE_HANDLER = MyHandler;',
        );

        expect(result.found).toBe(true);
        expect(result.locations).toHaveLength(1);
        expect(result.locations[0].functionName).toBe('MyHandler');
        // Handler definition is sourced from file B.
        expect(result.locations[0].file).toBe(file003B);
        // MyHandler identifier is on line 1 of int003_b.c (empirically verified).
        expect(result.locations[0].line).toBe(1);
    });

    // -----------------------------------------------------------------------
    // TC-INT-004 — Multi-hop derivation chain resolution
    // -----------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-002", "REQ-VSCODE-009"]}
    it('TC-INT-004: resolves vtable call through a 3-level derivation chain (LEAF → DERIVED → ROOT)', () => {
        // Line 28 of int004.c: "    ptLeafMotor->ptApi->Spin(ptLeafMotor);"
        // walkToRootType: MOTOR_LEAF_T → MOTOR_DERIVED_T → MOTOR_ROOT_T (no further entry)
        // moduleRoots.get("MOTOR_ROOT_T") → MOTOR_API_T
        // vtableAssignments["MOTOR_API_T"]["Spin"] → Motor_Spin (line 17)
        const result = vtRes004.resolve(
            file004,
            28,
            0,
            'ptLeafMotor->ptApi->Spin(ptLeafMotor);',
        );

        expect(result.found).toBe(true);
        expect(result.locations).toHaveLength(1);
        expect(result.locations[0].functionName).toBe('Motor_Spin');
        expect(result.locations[0].file).toBe(file004);
        // Motor_Spin identifier is on line 17 of int004.c (empirically verified).
        expect(result.locations[0].line).toBe(17);
    });

    // -----------------------------------------------------------------------
    // TC-INT-005 — Two independent modules in a single file
    // -----------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-002"]}
    it('TC-INT-005: resolves two independent modules in one file without cross-contamination', () => {
        // Line 41 of int005.c: "    ptLed->ptApi->TurnOn(ptLed);"
        const ledResult = vtRes005.resolve(
            file005,
            41,
            0,
            'ptLed->ptApi->TurnOn(ptLed);',
        );

        expect(ledResult.found).toBe(true);
        expect(ledResult.locations).toHaveLength(1);
        expect(ledResult.locations[0].functionName).toBe('Led_TurnOn');
        expect(ledResult.locations[0].file).toBe(file005);
        // Led_TurnOn identifier is on line 11 of int005.c (empirically verified).
        expect(ledResult.locations[0].line).toBe(11);

        // Line 42 of int005.c: "    ptBuzzer->ptApi->Beep(ptBuzzer);"
        const buzzerResult = vtRes005.resolve(
            file005,
            42,
            0,
            'ptBuzzer->ptApi->Beep(ptBuzzer);',
        );

        expect(buzzerResult.found).toBe(true);
        expect(buzzerResult.locations).toHaveLength(1);
        expect(buzzerResult.locations[0].functionName).toBe('Buzzer_Beep');
        expect(buzzerResult.locations[0].file).toBe(file005);
        // Buzzer_Beep identifier is on line 30 of int005.c (empirically verified).
        expect(buzzerResult.locations[0].line).toBe(30);

        // Verify no cross-contamination: LED resolution must not return BUZZER functions.
        expect(ledResult.locations[0].functionName).not.toBe('Buzzer_Beep');
        expect(buzzerResult.locations[0].functionName).not.toBe('Led_TurnOn');
    });

    // -----------------------------------------------------------------------
    // TC-INT-006 — No vtable patterns → graceful found:false
    // -----------------------------------------------------------------------

    // @{"verify": ["REQ-VSCODE-002", "REQ-VSCODE-004"]}
    it('TC-INT-006: returns found:false without throwing when file has no LibJuno vtable patterns', () => {
        // Line 4 of int006.c: "    int result = add(1, 2);"
        // No JUNO_MODULE_ROOT, no vtable assignment, no API struct — the
        // resolve() call must return gracefully with found=false.
        const result = vtRes006.resolve(
            file006,
            4,
            0,
            'int result = add(1, 2);',
        );

        expect(result.found).toBe(false);
        expect(result.locations).toHaveLength(0);
        expect(result.errorMsg).toBeTruthy();
        expect(result.errorMsg).toContain('No LibJuno API call pattern found');
    });
});
