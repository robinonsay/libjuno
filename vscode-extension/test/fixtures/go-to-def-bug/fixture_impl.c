/*
    MIT License

    Copyright (c) 2025 Robin A. Onsay
*/

/*
 * Minimal reproducer for Go-to-Definition bug.
 *
 * This file mirrors engine_cmd_msg.c lines 49-96: a positional vtable
 * initializer referencing two static functions, followed by their actual
 * definitions AFTER the initializer. Each function body contains the exact
 * shapes that broke the Chevrotain grammar:
 *   - `*(TYPE *) ptr = *(TYPE *) src;`           (dereference of cast)
 *   - `*(TYPE *) ptr = (TYPE){0};`               (compound literal assign)
 *   - JUNO_ASSERT_SUCCESS(expr, return tStatus); (macro-with-keyword arg)
 *
 * Expected behaviour after the parser fix:
 *   - ctrl-click `Reset` at its use-site (line ~52 in the initializer)
 *     jumps to the `FixtureImpl_Reset` function definition (line ~40 below).
 *   - ctrl-click `Copy` at the initializer jumps to `FixtureImpl_Copy`.
 */

#include "fixture_api.h"

#define JUNO_ASSERT_SUCCESS(expr, stmt)  do { if ((expr) != 0) { stmt; } } while (0)

static FIXTURE_STATUS_T FixtureImpl_Copy(FIXTURE_POINTER_T tDest, const FIXTURE_POINTER_T tSrc);
static FIXTURE_STATUS_T FixtureImpl_Reset(FIXTURE_POINTER_T tPointer);

/* Positional vtable initializer — references Copy (line 34) and Reset (line 35). */
const FIXTURE_API_T gtFixtureApi =
{
    FixtureImpl_Copy,
    FixtureImpl_Reset
};

/* Function definitions — these are the lines that ctrl-click should navigate to. */

static FIXTURE_STATUS_T FixtureImpl_Copy(FIXTURE_POINTER_T tDest, const FIXTURE_POINTER_T tSrc)
{
    FIXTURE_STATUS_T tStatus = 0;
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    *(FIXTURE_MSG_T *) tDest.pvAddr = *(FIXTURE_MSG_T *) tSrc.pvAddr;
    return tStatus;
}

static FIXTURE_STATUS_T FixtureImpl_Reset(FIXTURE_POINTER_T tPointer)
{
    FIXTURE_STATUS_T tStatus = 0;
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    *(FIXTURE_MSG_T *) tPointer.pvAddr = (FIXTURE_MSG_T){0};
    return tStatus;
}
