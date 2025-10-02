#include "unity.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "juno/ds/map_api.h"

#ifndef TEST_MAP_CAPACITY
#define TEST_MAP_CAPACITY 16
#endif

// Storage for test map
static void *gKeys[TEST_MAP_CAPACITY];
static void *gValues[TEST_MAP_CAPACITY];
static bool gOccupied[TEST_MAP_CAPACITY];

// Forward-declared API and root
static JUNO_MAP_API_T gApi;
static JUNO_MAP_ROOT_T gRoot;

// Helpers
static void TestMap_Reset(void)
{
    memset(gKeys, 0, sizeof(gKeys));
    memset(gValues, 0, sizeof(gValues));
    memset(gOccupied, 0, sizeof(gOccupied));
}

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

// Hash function:
// - If key starts with '#' or 'C', parse the following digits as the hash seed.
// - Else, use a simple sum to avoid dependence on external functions.
static JUNO_RESULT_SIZE_T TestHash(void *ptKey)
{
    JUNO_RESULT_SIZE_T r = {0};
    if (!ptKey)
    {
        r.tStatus = JUNO_STATUS_TABLE_FULL_ERROR; // any non-success code works for tests
        return r;
    }
    const char *s = (const char *)ptKey;
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

static JUNO_RESULT_BOOL_T TestKeyIsEqual(void *ptLeft, void *ptRight)
{
    JUNO_RESULT_BOOL_T r = {0};
    const char *l = (const char *)ptLeft;
    const char *p = (const char *)ptRight;
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

static JUNO_RESULT_VOID_PTR_T TestGetValue(size_t iIndex)
{
    JUNO_RESULT_VOID_PTR_T r = {0};
    if (iIndex >= TEST_MAP_CAPACITY)
    {
        r.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
        return r;
    }
    r.tOk = gValues[iIndex];
    r.tStatus = JUNO_STATUS_SUCCESS;
    return r;
}

static JUNO_RESULT_VOID_PTR_T TestGetKey(size_t iIndex)
{
    JUNO_RESULT_VOID_PTR_T r = {0};
    if (iIndex >= TEST_MAP_CAPACITY)
    {
        r.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
        return r;
    }
    r.tOk = gKeys[iIndex];
    r.tStatus = JUNO_STATUS_SUCCESS;
    return r;
}

static JUNO_STATUS_T TestSetValue(size_t iIndex, void *ptValue)
{
    if (iIndex >= TEST_MAP_CAPACITY) return JUNO_STATUS_TABLE_FULL_ERROR;
    gValues[iIndex] = ptValue;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestSetKey(size_t iIndex, void *ptKey)
{
    if (iIndex >= TEST_MAP_CAPACITY) return JUNO_STATUS_TABLE_FULL_ERROR;
    gKeys[iIndex] = ptKey;
    gOccupied[iIndex] = (ptKey != NULL);
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestRemove(size_t iIndex)
{
    if (iIndex >= TEST_MAP_CAPACITY) return JUNO_STATUS_TABLE_FULL_ERROR;
    if (!gOccupied[iIndex]) return JUNO_STATUS_TABLE_FULL_ERROR;
    gKeys[iIndex] = NULL;
    gValues[iIndex] = NULL;
    gOccupied[iIndex] = false;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_RESULT_BOOL_T TestIsEmpty(size_t zIndex)
{
    JUNO_RESULT_BOOL_T r = {0};
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

static JUNO_RESULT_VOID_PTR_T TestGetKey_WithInjectedError(size_t iIndex)
{
    JUNO_RESULT_VOID_PTR_T r = {0};
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
    JUNO_RESULT_BOOL_T r = {0};
    if (gInjectIsEmptyError && zIndex == gInjectIsEmptyErrorIndex)
    {
        r.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
        return r;
    }
    return TestIsEmpty(zIndex);
}

static int gInjectKeyEqErrorCount = 0;

static const char *gInjectHashErrorKey = NULL;

static JUNO_RESULT_SIZE_T TestHash_WithInjectedError(void *ptKey)
{
    JUNO_RESULT_SIZE_T r = {0};
    if (gInjectHashErrorKey && ptKey && strcmp((const char *)ptKey, gInjectHashErrorKey) == 0)
    {
        r.tStatus = JUNO_STATUS_TABLE_FULL_ERROR;
        return r;
    }
    return TestHash(ptKey);
}

// Unity fixtures
void setUp(void)
{
    TestMap_Reset();
    gApi.Hash = TestHash;
    gApi.KeyIsEqual = TestKeyIsEqual;
    gApi.GetValue = TestGetValue;
    gApi.GetKey = TestGetKey;
    gApi.SetValue = TestSetValue;
    gApi.SetKey = TestSetKey;
    gApi.Remove = TestRemove;
    gApi.IsEmpty = TestIsEmpty;

    gRoot.ptApi = &gApi;
    gRoot.zCapacity = TEST_MAP_CAPACITY;

    gInjectGetKeyError = false;
    gInjectIsEmptyError = false;
    gInjectKeyEqErrorCount = 0;
    gInjectHashErrorKey = NULL;
}

void tearDown(void)
{
}

// Tests
static void test_nominal_map(void)
{
    const char *k1 = "#1";
    int v1 = 42;

    JUNO_STATUS_T s = JunoMap_Set(&gRoot, (void *)k1, &v1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, s);

    JUNO_RESULT_VOID_PTR_T r = JunoMap_Get(&gRoot, (void *)k1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, r.tStatus);
    TEST_ASSERT_EQUAL_PTR(&v1, r.tOk);

    s = JunoMap_Remove(&gRoot, (void *)k1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, s);

    r = JunoMap_Get(&gRoot, (void *)k1);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, r.tStatus);
    TEST_ASSERT_NULL(r.tOk);

    const char *ka = "C3a";
    const char *kb = "C4b";
    int va = 11, vb = 22;

    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Set(&gRoot, (void *)ka, &va));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Set(&gRoot, (void *)kb, &vb));

    JUNO_RESULT_VOID_PTR_T ra = JunoMap_Get(&gRoot, (void *)ka);
    JUNO_RESULT_VOID_PTR_T rb = JunoMap_Get(&gRoot, (void *)kb);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, ra.tStatus);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, rb.tStatus);
    TEST_ASSERT_EQUAL_PTR(&va, ra.tOk);
    TEST_ASSERT_EQUAL_PTR(&vb, rb.tOk);

    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Remove(&gRoot, (void *)ka));
    ra = JunoMap_Get(&gRoot, (void *)ka);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, ra.tStatus);
    TEST_ASSERT_NULL(ra.tOk);
    rb = JunoMap_Get(&gRoot, (void *)kb);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, rb.tStatus);
    TEST_ASSERT_EQUAL_PTR(&vb, rb.tOk);
}

static void test_null_init(void)
{
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_VerifyApi(NULL));

    JUNO_MAP_ROOT_T badRoot1 = {0};
    badRoot1.ptApi = NULL;
    badRoot1.zCapacity = TEST_MAP_CAPACITY;
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Verify(&badRoot1));

    JUNO_MAP_ROOT_T badRoot2 = {0};
    badRoot2.ptApi = &gApi;
    badRoot2.zCapacity = 0;
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Verify(&badRoot2));
}

static void test_null_set(void)
{
    int v = 1;
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Set(NULL, (void *)"#1", &v));
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Set(&gRoot, NULL, &v));
}

static void test_null_get(void)
{
    JUNO_RESULT_VOID_PTR_T r = JunoMap_Get(NULL, (void *)"#2");
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, r.tStatus);

    r = JunoMap_Get(&gRoot, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, r.tStatus);
}

static void test_null_remove(void)
{
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Remove(NULL, (void *)"#3"));
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Remove(&gRoot, NULL));
}

static void test_remove_nonexistent_key(void)
{
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Remove(&gRoot, (void *)"#7"));

    int vals[4] = {10, 20, 30, 40};
    const char *k0 = "C7a";
    const char *k1 = "C7b";
    const char *k2 = "C7c";
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Set(&gRoot, (void *)k0, &vals[0]));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Set(&gRoot, (void *)k1, &vals[1]));
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Set(&gRoot, (void *)k2, &vals[2]));

    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Remove(&gRoot, (void *)"C7x"));
}

static void test_duplicate_key_insertion(void)
{
    const char *k = "#5";
    int a = 100, b = 200;

    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Set(&gRoot, (void *)k, &a));
    size_t before = TestMap_CountOccupied();
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Set(&gRoot, (void *)k, &b));
    size_t after = TestMap_CountOccupied();

    TEST_ASSERT_EQUAL(before, after);

    JUNO_RESULT_VOID_PTR_T r = JunoMap_Get(&gRoot, (void *)k);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, r.tStatus);
    TEST_ASSERT_EQUAL_PTR(&b, r.tOk);
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
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Set(&gRoot, keys[i], &vals[i]));
    }
    TEST_ASSERT_EQUAL(TEST_MAP_CAPACITY, TestMap_CountOccupied());

    JUNO_STATUS_T s = JunoMap_Set(&gRoot, (void *)"C0_overflow", &vals[0]);
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
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Set(&gRoot, (void *)keys[i], &baseVals[i]));
    }

    // Remove every other entry using the exact same pointer used for insert
    for (size_t i = 0; i < TEST_MAP_CAPACITY / 2; i += 2)
    {
        TEST_ASSERT_NOT_NULL(keys[i]);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Remove(&gRoot, (void *)keys[i]));
    }

    // Reinsert the same keys (same pointers) for those removed, with new values
    for (size_t i = 0; i < TEST_MAP_CAPACITY / 2; i++)
    {
        if ((i % 2) == 0)
        {
            baseVals[TEST_MAP_CAPACITY + i] = 2000 + (int)i;
            TEST_ASSERT_NOT_NULL(keys[i]);
            TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoMap_Set(&gRoot, (void *)keys[i], &baseVals[TEST_MAP_CAPACITY + i]));
        }
    }

    // 1) GetKey error path
    gApi.GetKey = TestGetKey_WithInjectedError;
    gInjectGetKeyErrorIndex = 2;
    gInjectGetKeyError = true;
    JUNO_RESULT_SIZE_T gi = JunoMap_GetIndex(&gRoot, (void *)"#2");
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, gi.tStatus);
    gInjectGetKeyError = false;
    gApi.GetKey = TestGetKey;

    // 2) IsEmpty error path
    gApi.IsEmpty = TestIsEmpty_WithInjectedError;
    gInjectIsEmptyErrorIndex = 3;
    gInjectIsEmptyError = true;
    gi = JunoMap_GetIndex(&gRoot, (void *)"#3");
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, gi.tStatus);
    gInjectIsEmptyError = false;
    gApi.IsEmpty = TestIsEmpty;
    // 4) Hash error path
    gApi.Hash = TestHash_WithInjectedError;
    gInjectHashErrorKey = "BADHASH";
    gi = JunoMap_GetIndex(&gRoot, (void *)"BADHASH");
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, gi.tStatus);
    gInjectHashErrorKey = NULL;
    gApi.Hash = TestHash;

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

