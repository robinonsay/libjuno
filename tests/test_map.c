/*
    MIT License

    Copyright (c) 2025 Robin A. Onsay

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.
*/

#include "unity.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "juno/ds/map_api.h"
#include "juno/memory/memory_api.h"

#ifndef TEST_MAP_CAPACITY
#define TEST_MAP_CAPACITY 16
#endif

// Storage for test map (buffer implementation for JUNO_MAP_BUFFER_API_T)
static JUNO_POINTER_T gKeys[TEST_MAP_CAPACITY];
static JUNO_POINTER_T gValues[TEST_MAP_CAPACITY];
static bool gOccupied[TEST_MAP_CAPACITY];

// Forward-declared buffer API and map root
static JUNO_MAP_BUFFER_API_T gBufferApi;
static JUNO_MAP_ROOT_T gRoot;

// Simple digit parser
static size_t parse_digits(const char *s)
{
    size_t v = 0;
    while (*s >= '0' && *s <= '9')
    {
        v = (v * 10) + (size_t)(*s - '0');
        s++;
    }
    return v;
}

// Pointer API used by tests (needed to build JUNO_POINTER_T values)
static JUNO_STATUS_T TestPointer_Copy(JUNO_POINTER_T tDest, JUNO_POINTER_T tSrc)
{
    // Basic copy: sizes must match
    if (!tDest.pvAddr || !tSrc.pvAddr || tDest.zSize != tSrc.zSize)
        return JUNO_STATUS_ERR;
    memcpy(tDest.pvAddr, tSrc.pvAddr, tDest.zSize);
    return JUNO_STATUS_SUCCESS;
}
static JUNO_STATUS_T TestPointer_Reset(JUNO_POINTER_T tPointer)
{
    if (!tPointer.pvAddr || !tPointer.zSize) return JUNO_STATUS_ERR;
    memset(tPointer.pvAddr, 0, tPointer.zSize);
    return JUNO_STATUS_SUCCESS;
}
static const JUNO_POINTER_API_T gPointerApi = {
    .Copy = TestPointer_Copy,
    .Reset = TestPointer_Reset,
};

// Helpers
static void TestMap_Reset(void)
{
    memset(gKeys, 0, sizeof(gKeys));
    memset(gValues, 0, sizeof(gValues));
    memset(gOccupied, 0, sizeof(gOccupied));
}

// Hash function:
// - If key starts with '#' or 'C', parse the following digits as the hash seed.
// - Else, use a simple sum to avoid dependence on external functions.
static JUNO_RESULT_SIZE_T TestHash(JUNO_POINTER_T tKey)
{
    JUNO_RESULT_SIZE_T r = (JUNO_RESULT_SIZE_T){0};
    if (!tKey.pvAddr)
    {
        r.tStatus = JUNO_STATUS_TABLE_FULL_ERROR; // any non-success code works for tests
        return r;
    }
    const char *s = (const char *)tKey.pvAddr;
    size_t v = 0;
    if (s[0] == '#' || s[0] == 'C')
    {
        v = parse_digits(s + 1);
    }
    else
    {
        for (int i = 0; s[i] && i < 8; i++) v += (unsigned char)s[i];
    }
    r.tOk = v;
    r.tStatus = JUNO_STATUS_SUCCESS;
    return r;
}

static JUNO_RESULT_BOOL_T TestKeyIsEqual(JUNO_POINTER_T tLeft, JUNO_POINTER_T tRight)
{
    JUNO_RESULT_BOOL_T r = (JUNO_RESULT_BOOL_T){0};
    const char *l = (const char *)tLeft.pvAddr;
    const char *p = (const char *)tRight.pvAddr;
    if (!l || !p)
    {
        r.tOk = (l == p);
        r.tStatus = JUNO_STATUS_SUCCESS;
        return r;
    }
    r.tOk = (strcmp(l, p) == 0);
    r.tStatus = JUNO_STATUS_SUCCESS;
    return r;
}

static JUNO_RESULT_POINTER_T TestGetValue(size_t iIndex)
{
    JUNO_RESULT_POINTER_T r = (JUNO_RESULT_POINTER_T){0};
    if (iIndex >= TEST_MAP_CAPACITY)
    {
        r.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
        return r;
    }
    r.tOk = gValues[iIndex];
    r.tStatus = JUNO_STATUS_SUCCESS;
    return r;
}

static JUNO_RESULT_POINTER_T TestGetKey(size_t iIndex)
{
    JUNO_RESULT_POINTER_T r = (JUNO_RESULT_POINTER_T){0};
    if (iIndex >= TEST_MAP_CAPACITY)
    {
        r.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
        return r;
    }
    r.tOk = gKeys[iIndex];
    r.tStatus = JUNO_STATUS_SUCCESS;
    return r;
}

static JUNO_STATUS_T TestSetValue(size_t iIndex, JUNO_POINTER_T tValue)
{
    if (iIndex >= TEST_MAP_CAPACITY) return JUNO_STATUS_TABLE_FULL_ERROR;
    gValues[iIndex] = tValue;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestSetKey(size_t iIndex, JUNO_POINTER_T tKey)
{
    if (iIndex >= TEST_MAP_CAPACITY) return JUNO_STATUS_TABLE_FULL_ERROR;
    gKeys[iIndex] = tKey;
    gOccupied[iIndex] = (tKey.pvAddr != NULL);
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestRemove(size_t iIndex)
{
    if (iIndex >= TEST_MAP_CAPACITY) return JUNO_STATUS_TABLE_FULL_ERROR;
    if (!gOccupied[iIndex]) return JUNO_STATUS_TABLE_FULL_ERROR;
    memset(&gKeys[iIndex], 0, sizeof(JUNO_POINTER_T));
    memset(&gValues[iIndex], 0, sizeof(JUNO_POINTER_T));
    gOccupied[iIndex] = false;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_RESULT_BOOL_T TestIsEmpty(size_t zIndex)
{
    JUNO_RESULT_BOOL_T r = (JUNO_RESULT_BOOL_T){0};
    if (zIndex >= TEST_MAP_CAPACITY)
    {
        r.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
        return r;
    }
    r.tOk = !gOccupied[zIndex];
    r.tStatus = JUNO_STATUS_SUCCESS;
    return r;
}

// Count occupied slots
static size_t TestMap_CountOccupied(void)
{
    size_t c = 0;
    for (size_t i = 0; i < TEST_MAP_CAPACITY; i++)
        if (gOccupied[i]) c++;
    return c;
}

// Injection helpers for error-path testing
static bool gInjectGetKeyError = false;
static size_t gInjectGetKeyErrorIndex = 0;

static JUNO_RESULT_POINTER_T TestGetKey_WithInjectedError(size_t iIndex)
{
    JUNO_RESULT_POINTER_T r = (JUNO_RESULT_POINTER_T){0};
    if (gInjectGetKeyError && iIndex == gInjectGetKeyErrorIndex)
    {
        r.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
        return r;
    }
    return TestGetKey(iIndex);
}

static bool gInjectIsEmptyError = false;
static size_t gInjectIsEmptyErrorIndex = 0;

static JUNO_RESULT_BOOL_T TestIsEmpty_WithInjectedError(size_t zIndex)
{
    JUNO_RESULT_BOOL_T r = (JUNO_RESULT_BOOL_T){0};
    if (gInjectIsEmptyError && zIndex == gInjectIsEmptyErrorIndex)
    {
        r.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
        return r;
    }
    return TestIsEmpty(zIndex);
}

static const char *gInjectHashErrorKey = NULL;

static JUNO_RESULT_SIZE_T TestHash_WithInjectedError(JUNO_POINTER_T tKey)
{
    JUNO_RESULT_SIZE_T r = (JUNO_RESULT_SIZE_T){0};
    const char *s = (const char *)tKey.pvAddr;
    if (gInjectHashErrorKey && s && strcmp(s, gInjectHashErrorKey) == 0)
    {
        r.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
        return r;
    }
    return TestHash(tKey);
}

// Unity fixtures
void setUp(void)
{
    TestMap_Reset();

    gBufferApi.Hash = TestHash;
    gBufferApi.KeyIsEqual = TestKeyIsEqual;
    gBufferApi.GetValue = TestGetValue;
    gBufferApi.GetKey = TestGetKey;
    gBufferApi.SetValue = TestSetValue;
    gBufferApi.SetKey = TestSetKey;
    gBufferApi.Remove = TestRemove;
    gBufferApi.IsEmpty = TestIsEmpty;

    // Initialize map root via API init (sets ptApi to JunoMap implementation)
    (void)JunoMap_Init(&gRoot, &gBufferApi, TEST_MAP_CAPACITY, NULL, NULL);

    gInjectGetKeyError = false;
    gInjectIsEmptyError = false;
    gInjectHashErrorKey = NULL;
}

void tearDown(void)
{
}

// Convenience builders for test pointers
#define K(strlit) (JunoMemory_PointerInit(&gPointerApi, char, (void*)(strlit)))
#define V_int(pint) (JunoMemory_PointerInit(&gPointerApi, int, (void*)(pint)))

// Tests
static void test_nominal_map(void)
{
    const char *k1_c = "#1";
    int v1 = 42;
    JUNO_POINTER_T k1 = K(k1_c);
    JUNO_POINTER_T v1p = V_int(&v1);

    JUNO_STATUS_T s = gRoot.ptApi->Set(&gRoot, k1, v1p);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, s);

    JUNO_RESULT_POINTER_T r = gRoot.ptApi->Get(&gRoot, k1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, r.tStatus);
    TEST_ASSERT_EQUAL_PTR(&v1, r.tOk.pvAddr);

    s = gRoot.ptApi->Remove(&gRoot, k1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, s);

    r = gRoot.ptApi->Get(&gRoot, k1);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, r.tStatus);
    TEST_ASSERT_NULL(r.tOk.pvAddr);

    const char *ka_c = "C3a";
    const char *kb_c = "C4b";
    int va = 11, vb = 22;

    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, gRoot.ptApi->Set(&gRoot, K(ka_c), V_int(&va)));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, gRoot.ptApi->Set(&gRoot, K(kb_c), V_int(&vb)));

    JUNO_RESULT_POINTER_T ra = gRoot.ptApi->Get(&gRoot, K(ka_c));
    JUNO_RESULT_POINTER_T rb = gRoot.ptApi->Get(&gRoot, K(kb_c));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, ra.tStatus);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, rb.tStatus);
    TEST_ASSERT_EQUAL_PTR(&va, ra.tOk.pvAddr);
    TEST_ASSERT_EQUAL_PTR(&vb, rb.tOk.pvAddr);

    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, gRoot.ptApi->Remove(&gRoot, K(ka_c)));
    ra = gRoot.ptApi->Get(&gRoot, K(ka_c));
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, ra.tStatus);
    TEST_ASSERT_NULL(ra.tOk.pvAddr);
    rb = gRoot.ptApi->Get(&gRoot, K(kb_c));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, rb.tStatus);
    TEST_ASSERT_EQUAL_PTR(&vb, rb.tOk.pvAddr);
}

static void test_null_init(void)
{
    // Buffer API verify should fail on NULL
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_BufferApiVerify(NULL));

    JUNO_MAP_ROOT_T badRoot1 = {0};
    badRoot1.ptApi = NULL;
    badRoot1.zCapacity = TEST_MAP_CAPACITY;
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Verify(&badRoot1));

    JUNO_MAP_ROOT_T badRoot2 = {0};
    badRoot2.ptApi = gRoot.ptApi; // but capacity 0
    badRoot2.zCapacity = 0;
    badRoot2.ptBufferApi = &gBufferApi;
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Verify(&badRoot2));
}

static void test_null_set(void)
{
    int v = 1;
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, gRoot.ptApi->Set(NULL, K("#1"), V_int(&v)));
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, gRoot.ptApi->Set(&gRoot, (JUNO_POINTER_T){0}, V_int(&v)));
}

static void test_null_get(void)
{
    JUNO_RESULT_POINTER_T r = gRoot.ptApi->Get(NULL, K("#2"));
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, r.tStatus);

    r = gRoot.ptApi->Get(&gRoot, (JUNO_POINTER_T){0});
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, r.tStatus);
}

static void test_null_remove(void)
{
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, gRoot.ptApi->Remove(NULL, K("#3")));
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, gRoot.ptApi->Remove(&gRoot, (JUNO_POINTER_T){0}));
}

static void test_remove_nonexistent_key(void)
{
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, gRoot.ptApi->Remove(&gRoot, K("#7")));

    int vals[4] = {10, 20, 30, 40};
    const char *k0 = "C7a";
    const char *k1 = "C7b";
    const char *k2 = "C7c";
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, gRoot.ptApi->Set(&gRoot, K(k0), V_int(&vals[0])));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, gRoot.ptApi->Set(&gRoot, K(k1), V_int(&vals[1])));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, gRoot.ptApi->Set(&gRoot, K(k2), V_int(&vals[2])));

    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, gRoot.ptApi->Remove(&gRoot, K("C7x")));
}

static void test_duplicate_key_insertion(void)
{
    const char *k = "#5";
    int a = 100, b = 200;

    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, gRoot.ptApi->Set(&gRoot, K(k), V_int(&a)));
    size_t before = TestMap_CountOccupied();
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, gRoot.ptApi->Set(&gRoot, K(k), V_int(&b)));
    size_t after = TestMap_CountOccupied();

    TEST_ASSERT_EQUAL(before, after);

    JUNO_RESULT_POINTER_T r = gRoot.ptApi->Get(&gRoot, K(k));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, r.tStatus);
    TEST_ASSERT_EQUAL_PTR(&b, r.tOk.pvAddr);
}

static void test_overflow_map(void)
{
    int vals[TEST_MAP_CAPACITY];
    char keys[TEST_MAP_CAPACITY][8];
    for (size_t i = 0; i < TEST_MAP_CAPACITY; i++)
    {
        vals[i] = (int)i + 1;
        snprintf(keys[i], sizeof(keys[i]), "#%u", (unsigned)i);
    }
    for (size_t i = 0; i < TEST_MAP_CAPACITY; i++)
    {
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, gRoot.ptApi->Set(&gRoot, K(keys[i]), V_int(&vals[i])));
    }
    TEST_ASSERT_EQUAL(TEST_MAP_CAPACITY, TestMap_CountOccupied());

    JUNO_STATUS_T s = gRoot.ptApi->Set(&gRoot, K("C0_overflow"), V_int(&vals[0]));
    TEST_ASSERT_EQUAL(JUNO_STATUS_TABLE_FULL_ERROR, s);
}

#include <stdlib.h>

static void test_fill_map(void)
{
    int baseVals[TEST_MAP_CAPACITY * 2];
    char tmp[32];
    // Store stable pointers for all keys we touch in this test
    const char *keys[TEST_MAP_CAPACITY] = {0};

    // Insert: allocate and store unique key strings "C<i>"
    for (size_t i = 0; i < TEST_MAP_CAPACITY; i++)
    {
        baseVals[i] = 1000 + (int)i;
        snprintf(tmp, sizeof(tmp), "C%u", (unsigned)i);

        // Duplicate key into heap so the map stores a stable pointer
        size_t len = strlen(tmp) + 1;
        char *k = (char *)malloc(len);
        TEST_ASSERT_NOT_NULL(k);
        memcpy(k, tmp, len);
        keys[i] = k;
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, gRoot.ptApi->Set(&gRoot, K(keys[i]), V_int(&baseVals[i])));
    }

    // Remove every other entry using the exact same pointer used for insert
    for (size_t i = 0; i < TEST_MAP_CAPACITY / 2; i += 2)
    {
        TEST_ASSERT_NOT_NULL(keys[i]);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, gRoot.ptApi->Remove(&gRoot, K(keys[i])));
    }

    // Reinsert the same keys (same pointers) for those removed, with new values
    for (size_t i = 0; i < TEST_MAP_CAPACITY / 2; i++)
    {
        if ((i % 2) == 0)
        {
            baseVals[TEST_MAP_CAPACITY + i] = 2000 + (int)i;
            TEST_ASSERT_NOT_NULL(keys[i]);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, gRoot.ptApi->Set(&gRoot, K(keys[i]), V_int(&baseVals[TEST_MAP_CAPACITY + i])));
        }
    }

    // 1) GetKey error path
    gBufferApi.GetKey = TestGetKey_WithInjectedError;
    gInjectGetKeyErrorIndex = 2;
    gInjectGetKeyError = true;
    JUNO_RESULT_SIZE_T gi = JunoMap_GetIndex(&gRoot, K("#2"));
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, gi.tStatus);
    gInjectGetKeyError = false;
    gBufferApi.GetKey = TestGetKey;

    // 2) IsEmpty error path
    gBufferApi.IsEmpty = TestIsEmpty_WithInjectedError;
    gInjectIsEmptyErrorIndex = 3;
    gInjectIsEmptyError = true;
    gi = JunoMap_GetIndex(&gRoot, K("#3"));
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, gi.tStatus);
    gInjectIsEmptyError = false;
    gBufferApi.IsEmpty = TestIsEmpty;

    // 3) Hash error path
    gBufferApi.Hash = TestHash_WithInjectedError;
    gInjectHashErrorKey = "BADHASH";
    gi = JunoMap_GetIndex(&gRoot, K("BADHASH"));
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, gi.tStatus);
    gInjectHashErrorKey = NULL;
    gBufferApi.Hash = TestHash;

    // Note: keys[] are intentionally not freed to avoid dangling pointers
    // inside the map; tests exit shortly after.
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_nominal_map);
    RUN_TEST(test_null_init);
    RUN_TEST(test_null_set);
    RUN_TEST(test_null_get);
    RUN_TEST(test_null_remove);
    RUN_TEST(test_remove_nonexistent_key);
    RUN_TEST(test_duplicate_key_insertion);
    RUN_TEST(test_overflow_map);
    RUN_TEST(test_fill_map);
    return UNITY_END();
}

