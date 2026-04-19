/*
    MIT License

    Copyright (c) 2025 Robin A. Onsay
*/

/*
 * Minimal reproducer header for the Go-to-Definition bug in which ctrl-click
 * on a vtable member call navigates to the vtable initializer line instead
 * of the function definition line.
 *
 * Shape mirrors the JUNO_POINTER_API_T from libjuno — a plain C struct of
 * function pointers.
 */

#ifndef FIXTURE_API_H
#define FIXTURE_API_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int FIXTURE_STATUS_T;

typedef struct
{
    void *pvAddr;
    struct FIXTURE_API_TAG *ptApi;
} FIXTURE_POINTER_T;

struct FIXTURE_API_TAG
{
    FIXTURE_STATUS_T (*Copy)(FIXTURE_POINTER_T tDest, const FIXTURE_POINTER_T tSrc);
    FIXTURE_STATUS_T (*Reset)(FIXTURE_POINTER_T tPointer);
};

typedef struct FIXTURE_API_TAG FIXTURE_API_T;

typedef struct
{
    int iX;
} FIXTURE_MSG_T;

#ifdef __cplusplus
}
#endif

#endif /* FIXTURE_API_H */
