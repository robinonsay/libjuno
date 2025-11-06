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

/**
 * @file test_map.c
 * @brief Comprehensive unit tests for the LibJuno Map API
 * @author Robin Onsay
 * 
 * This test suite aims for 100% code and branch coverage of the map
 * implementation, testing all edge cases and error paths including:
 * - Hash collision handling (linear probing)
 * - Set/Get/Remove operations
 * - Boundary conditions and table full scenarios
 */

#include "juno/ds/map_api.h"
#include "juno/ds/array_api.h"
#include "juno/macros.h"
#include "juno/memory/pointer_api.h"
#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

/* ============================================================================
 * Test Data Type Definition
 * ============================================================================ */

typedef struct TEST_MAP_ENTRY_TAG
{
    uint32_t iKey;
    uint32_t iValue;
    bool bIsNull; // Indicates if this entry is empty
} TEST_MAP_ENTRY_T;

/* ============================================================================
 * Test Map Implementation (Custom Array-backed Hash Map)
 * ============================================================================ */

typedef struct TEST_ARRAY_TAG
{
    JUNO_DS_ARRAY_ROOT_T tRoot;
    TEST_MAP_ENTRY_T *ptBuffer;
} TEST_ARRAY_T;

typedef struct TEST_MAP_TAG
{
    JUNO_MAP_ROOT_T tRoot;
    TEST_ARRAY_T tArray;
    TEST_MAP_ENTRY_T *ptBuffer;
} TEST_MAP_T;

/* Forward declarations for array API functions */
static JUNO_STATUS_T TestMap_SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
static JUNO_RESULT_POINTER_T TestMap_GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);
static JUNO_STATUS_T TestMap_RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex);

/* Forward declarations for pointer API functions */
static JUNO_STATUS_T TestMapEntry_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc);
static JUNO_STATUS_T TestMapEntry_Reset(JUNO_POINTER_T tPointer);
static JUNO_RESULT_BOOL_T TestMapEntry_Equals(const JUNO_POINTER_T tLeft, const JUNO_POINTER_T tRight);

/* Forward declarations for hashable pointer API functions */
static JUNO_RESULT_SIZE_T TestMapEntry_Hash(JUNO_POINTER_T tItem);
static JUNO_RESULT_BOOL_T TestMapEntry_IsNull(JUNO_POINTER_T tItem);

/* Pointer API for TEST_MAP_ENTRY_T */
const JUNO_POINTER_API_T gtTestMapEntryPointerApi = {
    TestMapEntry_Copy,
    TestMapEntry_Reset
};

/* Value Pointer API for TEST_MAP_ENTRY_T */
const JUNO_VALUE_POINTER_API_T gtTestMapEntryValuePointerApi = {
    TestMapEntry_Equals
};

/* Hashable Pointer API for TEST_MAP_ENTRY_T */
const JUNO_MAP_HASHABLE_POINTER_API_T gtTestMapEntryHashablePointerApi = {
    TestMapEntry_Hash,
    TestMapEntry_IsNull
};

/* Array API for TEST_MAP_T */
static const JUNO_DS_ARRAY_API_T gtTestMapArrayApi = {
    TestMap_SetAt,
    TestMap_GetAt,
    TestMap_RemoveAt
};

/* Macros to initialize and verify pointer types */
#define TestMapEntry_PointerInit(addr) (JUNO_POINTER_T){&gtTestMapEntryPointerApi, addr, sizeof(TEST_MAP_ENTRY_T), alignof(TEST_MAP_ENTRY_T)}
#define TEST_ARRAY_ASSERT_API(ptArray, ...)  if(ptArray->ptApi != &gtTestMapArrayApi) { __VA_ARGS__; }

/* ============================================================================
 * Pointer API Implementation
 * ============================================================================ */

static JUNO_STATUS_T TestMapEntry_Copy(JUNO_POINTER_T tDest, const JUNO_POINTER_T tSrc)
{
    if (!tDest.pvAddr || !tSrc.pvAddr) {
        return JUNO_STATUS_ERR;
    }
    if (tDest.zSize != sizeof(TEST_MAP_ENTRY_T) || tSrc.zSize != sizeof(TEST_MAP_ENTRY_T)) {
        return JUNO_STATUS_INVALID_SIZE_ERROR;
    }
    *(TEST_MAP_ENTRY_T *)tDest.pvAddr = *(TEST_MAP_ENTRY_T *)tSrc.pvAddr;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_STATUS_T TestMapEntry_Reset(JUNO_POINTER_T tPointer)
{
    if (!tPointer.pvAddr) {
        return JUNO_STATUS_ERR;
    }
    if (tPointer.zSize != sizeof(TEST_MAP_ENTRY_T)) {
        return JUNO_STATUS_INVALID_SIZE_ERROR;
    }
    TEST_MAP_ENTRY_T *ptEntry = (TEST_MAP_ENTRY_T *)tPointer.pvAddr;
    ptEntry->iKey = 0;
    ptEntry->iValue = 0;
    ptEntry->bIsNull = true;
    return JUNO_STATUS_SUCCESS;
}

static JUNO_RESULT_BOOL_T TestMapEntry_Equals(const JUNO_POINTER_T tLeft, const JUNO_POINTER_T tRight)
{
    JUNO_RESULT_BOOL_T tResult = {0};
    if (!tLeft.pvAddr || !tRight.pvAddr) {
        tResult.tStatus = JUNO_STATUS_ERR;
        return tResult;
    }
    if (tLeft.zSize != sizeof(TEST_MAP_ENTRY_T) || tRight.zSize != sizeof(TEST_MAP_ENTRY_T)) {
        tResult.tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        return tResult;
    }
    
    TEST_MAP_ENTRY_T *ptLeft = (TEST_MAP_ENTRY_T *)tLeft.pvAddr;
    TEST_MAP_ENTRY_T *ptRight = (TEST_MAP_ENTRY_T *)tRight.pvAddr;
    
    tResult.tOk = (ptLeft->iKey == ptRight->iKey);
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    return tResult;
}

/* ============================================================================
 * Hashable Pointer API Implementation
 * ============================================================================ */

static JUNO_RESULT_SIZE_T TestMapEntry_Hash(JUNO_POINTER_T tItem)
{
    JUNO_RESULT_SIZE_T tResult = {0};
    if (!tItem.pvAddr) {
        tResult.tStatus = JUNO_STATUS_ERR;
        return tResult;
    }
    if (tItem.zSize != sizeof(TEST_MAP_ENTRY_T)) {
        tResult.tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        return tResult;
    }
    
    TEST_MAP_ENTRY_T *ptEntry = (TEST_MAP_ENTRY_T *)tItem.pvAddr;
    // Simple hash function for testing
    tResult.tOk = (size_t)ptEntry->iKey;
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    return tResult;
}

static JUNO_RESULT_BOOL_T TestMapEntry_IsNull(JUNO_POINTER_T tItem)
{
    JUNO_RESULT_BOOL_T tResult = {0};
    if (!tItem.pvAddr) {
        tResult.tStatus = JUNO_STATUS_ERR;
        return tResult;
    }
    if (tItem.zSize != sizeof(TEST_MAP_ENTRY_T)) {
        tResult.tStatus = JUNO_STATUS_INVALID_SIZE_ERROR;
        return tResult;
    }
    
    TEST_MAP_ENTRY_T *ptEntry = (TEST_MAP_ENTRY_T *)tItem.pvAddr;
    tResult.tOk = ptEntry->bIsNull;
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    return tResult;
}

/* ============================================================================
 * Array API Implementation
 * ============================================================================ */

static JUNO_STATUS_T TestMap_SetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    TEST_ARRAY_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    
    if (tItem.zSize != sizeof(TEST_MAP_ENTRY_T)) {
        return JUNO_STATUS_INVALID_SIZE_ERROR;
    }
    
    tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    
    // Cast to get the containing TEST_ARRAY_T structure
    TEST_ARRAY_T *ptArrayImpl = (TEST_ARRAY_T *)ptArray;
    JUNO_POINTER_T tIndexPointer = {tItem.ptApi, &ptArrayImpl->ptBuffer[iIndex], sizeof(TEST_MAP_ENTRY_T), alignof(TEST_MAP_ENTRY_T)};
    tStatus = tItem.ptApi->Copy(tIndexPointer, tItem);
    return tStatus;
}

static JUNO_RESULT_POINTER_T TestMap_GetAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_RESULT_POINTER_T tResult = {0};
    tResult.tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    TEST_ARRAY_ASSERT_API(ptArray,
        tResult.tStatus = JUNO_STATUS_INVALID_TYPE_ERROR;
        return tResult
    );
    tResult.tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tResult.tStatus, return tResult);
    
    // Cast to get the containing TEST_ARRAY_T structure
    TEST_ARRAY_T *ptArrayImpl = (TEST_ARRAY_T *)ptArray;
    tResult.tOk.ptApi = &gtTestMapEntryPointerApi;
    tResult.tOk.pvAddr = &ptArrayImpl->ptBuffer[iIndex];
    tResult.tOk.zSize = sizeof(TEST_MAP_ENTRY_T);
    tResult.tOk.zAlignment = alignof(TEST_MAP_ENTRY_T);
    tResult.tStatus = JUNO_STATUS_SUCCESS;
    return tResult;
}

static JUNO_STATUS_T TestMap_RemoveAt(JUNO_DS_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
    JUNO_STATUS_T tStatus = JunoDs_ArrayVerify(ptArray);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    TEST_ARRAY_ASSERT_API(ptArray, return JUNO_STATUS_INVALID_TYPE_ERROR);
    tStatus = JunoDs_ArrayVerifyIndex(ptArray, iIndex);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    
    // Cast to get the containing TEST_ARRAY_T structure
    TEST_ARRAY_T *ptArrayImpl = (TEST_ARRAY_T *)ptArray;
    JUNO_POINTER_T tIndexPointer = {&gtTestMapEntryPointerApi, &ptArrayImpl->ptBuffer[iIndex], sizeof(TEST_MAP_ENTRY_T), alignof(TEST_MAP_ENTRY_T)};
    tStatus = tIndexPointer.ptApi->Reset(tIndexPointer);
    return tStatus;
}

/* ============================================================================
 * Test Fixture Setup/Teardown
 * ============================================================================ */

#define TEST_MAP_CAPACITY 10
static TEST_MAP_T gtTestMap;
static TEST_MAP_ENTRY_T gtTestMapBuffer[TEST_MAP_CAPACITY];

void setUp(void)
{
    memset(&gtTestMap, 0, sizeof(gtTestMap));
    memset(gtTestMapBuffer, 0, sizeof(gtTestMapBuffer));
    // Initialize all entries as null
    for (size_t i = 0; i < TEST_MAP_CAPACITY; i++) {
        gtTestMapBuffer[i].bIsNull = true;
    }
}

void tearDown(void)
{
    /* No dynamic allocations to clean up */
}

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static JUNO_STATUS_T InitTestMap(TEST_MAP_T *ptMap, size_t zCapacity)
{
    ptMap->ptBuffer = gtTestMapBuffer;
    ptMap->tArray.ptBuffer = gtTestMapBuffer;
    
    // Initialize the array root that backs the map
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(&ptMap->tArray.tRoot, &gtTestMapArrayApi, zCapacity, NULL, NULL);
    if (tStatus != JUNO_STATUS_SUCCESS) {
        return tStatus;
    }
    
    // Initialize the map root
    return JunoDs_MapInit(&ptMap->tRoot, &gtTestMapEntryHashablePointerApi, &gtTestMapEntryValuePointerApi, &ptMap->tArray.tRoot, NULL, NULL);
}

static TEST_MAP_ENTRY_T CreateTestEntry(uint32_t iKey, uint32_t iValue)
{
    TEST_MAP_ENTRY_T tEntry = {
        .iKey = iKey,
        .iValue = iValue,
        .bIsNull = false
    };
    return tEntry;
}

/* ============================================================================
 * Test Cases: Initialization
 * ============================================================================ */

static void test_map_init_nominal(void)
{
    JUNO_STATUS_T tStatus = InitTestMap(&gtTestMap, TEST_MAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(TEST_MAP_CAPACITY, gtTestMap.tArray.tRoot.zCapacity);
    TEST_ASSERT_NOT_NULL(gtTestMap.tRoot.ptApi);
    
    // Verify all entries are initialized as null
    for (size_t i = 0; i < TEST_MAP_CAPACITY; i++) {
        TEST_ASSERT_TRUE(gtTestMapBuffer[i].bIsNull);
    }
}

static void test_map_init_null_map(void)
{
    JUNO_STATUS_T tStatus = JunoDs_MapInit(NULL, &gtTestMapEntryHashablePointerApi, &gtTestMapEntryValuePointerApi, NULL, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_map_init_null_api(void)
{
    gtTestMap.ptBuffer = gtTestMapBuffer;
    JUNO_STATUS_T tStatus = JunoDs_MapInit(&gtTestMap.tRoot, NULL, &gtTestMapEntryValuePointerApi, &gtTestMap.tArray.tRoot, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_map_init_zero_capacity(void)
{
    gtTestMap.ptBuffer = gtTestMapBuffer;
    gtTestMap.tArray.ptBuffer = gtTestMapBuffer;
    // Initialize array with zero capacity - should fail
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(&gtTestMap.tArray.tRoot, &gtTestMapArrayApi, 0, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Set Operations
 * ============================================================================ */

static void test_map_set_single_item(void)
{
    JUNO_STATUS_T tStatus = InitTestMap(&gtTestMap, TEST_MAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    TEST_MAP_ENTRY_T tEntry = CreateTestEntry(42, 100);
    JUNO_POINTER_T tPointer = TestMapEntry_PointerInit(&tEntry);
    
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Verify the entry was stored at the correct index (hash % capacity)
    size_t iExpectedIndex = 42 % TEST_MAP_CAPACITY;
    TEST_ASSERT_FALSE(gtTestMapBuffer[iExpectedIndex].bIsNull);
    TEST_ASSERT_EQUAL(42, gtTestMapBuffer[iExpectedIndex].iKey);
    TEST_ASSERT_EQUAL(100, gtTestMapBuffer[iExpectedIndex].iValue);
}

static void test_map_set_multiple_items(void)
{
    JUNO_STATUS_T tStatus = InitTestMap(&gtTestMap, TEST_MAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Set multiple non-colliding entries
    for (uint32_t i = 0; i < 5; i++) {
        TEST_MAP_ENTRY_T tEntry = CreateTestEntry(i, i * 10);
        JUNO_POINTER_T tPointer = TestMapEntry_PointerInit(&tEntry);
        tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    // Verify all entries were stored correctly
    for (uint32_t i = 0; i < 5; i++) {
        TEST_ASSERT_FALSE(gtTestMapBuffer[i].bIsNull);
        TEST_ASSERT_EQUAL(i, gtTestMapBuffer[i].iKey);
        TEST_ASSERT_EQUAL(i * 10, gtTestMapBuffer[i].iValue);
    }
}

static void test_map_set_update_existing(void)
{
    JUNO_STATUS_T tStatus = InitTestMap(&gtTestMap, TEST_MAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Set initial value
    TEST_MAP_ENTRY_T tEntry1 = CreateTestEntry(42, 100);
    JUNO_POINTER_T tPointer1 = TestMapEntry_PointerInit(&tEntry1);
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Update with new value
    TEST_MAP_ENTRY_T tEntry2 = CreateTestEntry(42, 200);
    JUNO_POINTER_T tPointer2 = TestMapEntry_PointerInit(&tEntry2);
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer2);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Verify the value was updated
    size_t iExpectedIndex = 42 % TEST_MAP_CAPACITY;
    TEST_ASSERT_EQUAL(42, gtTestMapBuffer[iExpectedIndex].iKey);
    TEST_ASSERT_EQUAL(200, gtTestMapBuffer[iExpectedIndex].iValue);
}

static void test_map_set_with_collision(void)
{
    JUNO_STATUS_T tStatus = InitTestMap(&gtTestMap, TEST_MAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Set first entry at key 5 (hash = 5, index = 5)
    TEST_MAP_ENTRY_T tEntry1 = CreateTestEntry(5, 100);
    JUNO_POINTER_T tPointer1 = TestMapEntry_PointerInit(&tEntry1);
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Set second entry at key 15 (hash = 15, index = 5, collision!)
    TEST_MAP_ENTRY_T tEntry2 = CreateTestEntry(15, 200);
    JUNO_POINTER_T tPointer2 = TestMapEntry_PointerInit(&tEntry2);
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer2);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Verify linear probing: first entry at index 5, second at index 6
    TEST_ASSERT_EQUAL(5, gtTestMapBuffer[5].iKey);
    TEST_ASSERT_EQUAL(100, gtTestMapBuffer[5].iValue);
    TEST_ASSERT_EQUAL(15, gtTestMapBuffer[6].iKey);
    TEST_ASSERT_EQUAL(200, gtTestMapBuffer[6].iValue);
}

static void test_map_set_null_map(void)
{
    TEST_MAP_ENTRY_T tEntry = CreateTestEntry(42, 100);
    JUNO_POINTER_T tPointer = TestMapEntry_PointerInit(&tEntry);
    
    JUNO_STATUS_T tStatus = JunoDs_MapSet(NULL, tPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_map_set_table_full(void)
{
    JUNO_STATUS_T tStatus = InitTestMap(&gtTestMap, TEST_MAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Fill the entire map
    for (uint32_t i = 0; i < TEST_MAP_CAPACITY; i++) {
        TEST_MAP_ENTRY_T tEntry = CreateTestEntry(i, i * 10);
        JUNO_POINTER_T tPointer = TestMapEntry_PointerInit(&tEntry);
        tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    }
    
    // Attempt to add one more entry (should fail)
    TEST_MAP_ENTRY_T tEntry = CreateTestEntry(TEST_MAP_CAPACITY + 10, 999);
    JUNO_POINTER_T tPointer = TestMapEntry_PointerInit(&tEntry);
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_TABLE_FULL_ERROR, tStatus);
}

/* ============================================================================
 * Test Cases: Get Operations
 * ============================================================================ */

static void test_map_get_existing_item(void)
{
    JUNO_STATUS_T tStatus = InitTestMap(&gtTestMap, TEST_MAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Set an entry
    TEST_MAP_ENTRY_T tSetEntry = CreateTestEntry(42, 100);
    JUNO_POINTER_T tSetPointer = TestMapEntry_PointerInit(&tSetEntry);
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tSetPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Get the entry
    TEST_MAP_ENTRY_T tGetEntry = CreateTestEntry(42, 0);
    JUNO_POINTER_T tGetPointer = TestMapEntry_PointerInit(&tGetEntry);
    JUNO_RESULT_POINTER_T tResult = JunoDs_MapGet(&gtTestMap.tRoot, tGetPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    
    // Verify the retrieved entry
    TEST_MAP_ENTRY_T *ptRetrieved = (TEST_MAP_ENTRY_T *)tResult.tOk.pvAddr;
    TEST_ASSERT_EQUAL(42, ptRetrieved->iKey);
    TEST_ASSERT_EQUAL(100, ptRetrieved->iValue);
    TEST_ASSERT_FALSE(ptRetrieved->bIsNull);
}

static void test_map_get_nonexistent_item(void)
{
    JUNO_STATUS_T tStatus = InitTestMap(&gtTestMap, TEST_MAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Try to get an entry that doesn't exist
    TEST_MAP_ENTRY_T tGetEntry = CreateTestEntry(99, 0);
    JUNO_POINTER_T tGetPointer = TestMapEntry_PointerInit(&tGetEntry);
    JUNO_RESULT_POINTER_T tResult = JunoDs_MapGet(&gtTestMap.tRoot, tGetPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_DNE_ERROR, tResult.tStatus);
}

static void test_map_get_after_collision(void)
{
    JUNO_STATUS_T tStatus = InitTestMap(&gtTestMap, TEST_MAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Set entries that will collide
    TEST_MAP_ENTRY_T tEntry1 = CreateTestEntry(5, 100);
    JUNO_POINTER_T tPointer1 = TestMapEntry_PointerInit(&tEntry1);
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    TEST_MAP_ENTRY_T tEntry2 = CreateTestEntry(15, 200);
    JUNO_POINTER_T tPointer2 = TestMapEntry_PointerInit(&tEntry2);
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer2);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Get the second entry (which had collision)
    TEST_MAP_ENTRY_T tGetEntry = CreateTestEntry(15, 0);
    JUNO_POINTER_T tGetPointer = TestMapEntry_PointerInit(&tGetEntry);
    JUNO_RESULT_POINTER_T tResult = JunoDs_MapGet(&gtTestMap.tRoot, tGetPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    
    TEST_MAP_ENTRY_T *ptRetrieved = (TEST_MAP_ENTRY_T *)tResult.tOk.pvAddr;
    TEST_ASSERT_EQUAL(15, ptRetrieved->iKey);
    TEST_ASSERT_EQUAL(200, ptRetrieved->iValue);
}

static void test_map_get_null_map(void)
{
    TEST_MAP_ENTRY_T tEntry = CreateTestEntry(42, 0);
    JUNO_POINTER_T tPointer = TestMapEntry_PointerInit(&tEntry);
    
    JUNO_RESULT_POINTER_T tResult = JunoDs_MapGet(NULL, tPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
}

/* ============================================================================
 * Test Cases: Remove Operations
 * ============================================================================ */

static void test_map_remove_existing_item(void)
{
    JUNO_STATUS_T tStatus = InitTestMap(&gtTestMap, TEST_MAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Set an entry
    TEST_MAP_ENTRY_T tSetEntry = CreateTestEntry(42, 100);
    JUNO_POINTER_T tSetPointer = TestMapEntry_PointerInit(&tSetEntry);
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tSetPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Remove the entry
    TEST_MAP_ENTRY_T tRemoveEntry = CreateTestEntry(42, 0);
    JUNO_POINTER_T tRemovePointer = TestMapEntry_PointerInit(&tRemoveEntry);
    tStatus = JunoDs_MapRemove(&gtTestMap.tRoot, tRemovePointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Verify the entry was removed (marked as null)
    size_t iExpectedIndex = 42 % TEST_MAP_CAPACITY;
    TEST_ASSERT_TRUE(gtTestMapBuffer[iExpectedIndex].bIsNull);
}

static void test_map_remove_nonexistent_item(void)
{
    JUNO_STATUS_T tStatus = InitTestMap(&gtTestMap, TEST_MAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Add some entries to force linear probing search
    // Fill positions 9, 0 (wrapping) with colliding keys
    TEST_MAP_ENTRY_T tEntry1 = CreateTestEntry(9, 100);
    JUNO_POINTER_T tPointer1 = TestMapEntry_PointerInit(&tEntry1);
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    TEST_MAP_ENTRY_T tEntry2 = CreateTestEntry(19, 200); // Collides with 9, goes to index 0
    JUNO_POINTER_T tPointer2 = TestMapEntry_PointerInit(&tEntry2);
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer2);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Try to remove key 99 (hashes to 9, but occupied by key 9)
    // It will probe and find an empty slot eventually, returning success
    TEST_MAP_ENTRY_T tRemoveEntry = CreateTestEntry(99, 0);
    JUNO_POINTER_T tRemovePointer = TestMapEntry_PointerInit(&tRemoveEntry);
    tStatus = JunoDs_MapRemove(&gtTestMap.tRoot, tRemovePointer);
    // When it encounters an empty slot during probing (item not found), it breaks with SUCCESS
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_map_remove_after_collision(void)
{
    JUNO_STATUS_T tStatus = InitTestMap(&gtTestMap, TEST_MAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Set entries that will collide
    TEST_MAP_ENTRY_T tEntry1 = CreateTestEntry(5, 100);
    JUNO_POINTER_T tPointer1 = TestMapEntry_PointerInit(&tEntry1);
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    TEST_MAP_ENTRY_T tEntry2 = CreateTestEntry(15, 200);
    JUNO_POINTER_T tPointer2 = TestMapEntry_PointerInit(&tEntry2);
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer2);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Remove the second entry (which had collision)
    TEST_MAP_ENTRY_T tRemoveEntry = CreateTestEntry(15, 0);
    JUNO_POINTER_T tRemovePointer = TestMapEntry_PointerInit(&tRemoveEntry);
    tStatus = JunoDs_MapRemove(&gtTestMap.tRoot, tRemovePointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Verify the second entry was removed but first remains
    TEST_ASSERT_FALSE(gtTestMapBuffer[5].bIsNull);
    TEST_ASSERT_EQUAL(5, gtTestMapBuffer[5].iKey);
    TEST_ASSERT_TRUE(gtTestMapBuffer[6].bIsNull);
}

static void test_map_remove_null_map(void)
{
    TEST_MAP_ENTRY_T tEntry = CreateTestEntry(42, 0);
    JUNO_POINTER_T tPointer = TestMapEntry_PointerInit(&tEntry);
    
    JUNO_STATUS_T tStatus = JunoDs_MapRemove(NULL, tPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

/* ============================================================================
 * Test Cases: Collision Handling & Linear Probing
 * ============================================================================ */

static void test_map_multiple_collisions_linear_probing(void)
{
    JUNO_STATUS_T tStatus = InitTestMap(&gtTestMap, TEST_MAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Set entries that all hash to the same initial index
    // Keys: 3, 13, 23 all hash to index 3, 3, 3
    TEST_MAP_ENTRY_T tEntry1 = CreateTestEntry(3, 100);
    JUNO_POINTER_T tPointer1 = TestMapEntry_PointerInit(&tEntry1);
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    TEST_MAP_ENTRY_T tEntry2 = CreateTestEntry(13, 200);
    JUNO_POINTER_T tPointer2 = TestMapEntry_PointerInit(&tEntry2);
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer2);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    TEST_MAP_ENTRY_T tEntry3 = CreateTestEntry(23, 300);
    JUNO_POINTER_T tPointer3 = TestMapEntry_PointerInit(&tEntry3);
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer3);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Verify linear probing: should be at indices 3, 4, 5
    TEST_ASSERT_EQUAL(3, gtTestMapBuffer[3].iKey);
    TEST_ASSERT_EQUAL(100, gtTestMapBuffer[3].iValue);
    TEST_ASSERT_EQUAL(13, gtTestMapBuffer[4].iKey);
    TEST_ASSERT_EQUAL(200, gtTestMapBuffer[4].iValue);
    TEST_ASSERT_EQUAL(23, gtTestMapBuffer[5].iKey);
    TEST_ASSERT_EQUAL(300, gtTestMapBuffer[5].iValue);
    
    // Verify all can be retrieved correctly
    TEST_MAP_ENTRY_T tGet1 = CreateTestEntry(3, 0);
    JUNO_POINTER_T tGetPtr1 = TestMapEntry_PointerInit(&tGet1);
    JUNO_RESULT_POINTER_T tResult1 = JunoDs_MapGet(&gtTestMap.tRoot, tGetPtr1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult1.tStatus);
    TEST_ASSERT_EQUAL(3, ((TEST_MAP_ENTRY_T *)tResult1.tOk.pvAddr)->iKey);
    
    TEST_MAP_ENTRY_T tGet2 = CreateTestEntry(13, 0);
    JUNO_POINTER_T tGetPtr2 = TestMapEntry_PointerInit(&tGet2);
    JUNO_RESULT_POINTER_T tResult2 = JunoDs_MapGet(&gtTestMap.tRoot, tGetPtr2);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult2.tStatus);
    TEST_ASSERT_EQUAL(13, ((TEST_MAP_ENTRY_T *)tResult2.tOk.pvAddr)->iKey);
    
    TEST_MAP_ENTRY_T tGet3 = CreateTestEntry(23, 0);
    JUNO_POINTER_T tGetPtr3 = TestMapEntry_PointerInit(&tGet3);
    JUNO_RESULT_POINTER_T tResult3 = JunoDs_MapGet(&gtTestMap.tRoot, tGetPtr3);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult3.tStatus);
    TEST_ASSERT_EQUAL(23, ((TEST_MAP_ENTRY_T *)tResult3.tOk.pvAddr)->iKey);
}

static void test_map_wraparound_probing(void)
{
    JUNO_STATUS_T tStatus = InitTestMap(&gtTestMap, TEST_MAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Fill slots 9, then try to add items that would probe past end
    TEST_MAP_ENTRY_T tEntry1 = CreateTestEntry(9, 100);
    JUNO_POINTER_T tPointer1 = TestMapEntry_PointerInit(&tEntry1);
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer1);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Key 19 hashes to 19 % 10 = 9, should wrap to index 0
    TEST_MAP_ENTRY_T tEntry2 = CreateTestEntry(19, 200);
    JUNO_POINTER_T tPointer2 = TestMapEntry_PointerInit(&tEntry2);
    tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tPointer2);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Verify wraparound: first at 9, second at 0 (wrapped)
    TEST_ASSERT_EQUAL(9, gtTestMapBuffer[9].iKey);
    TEST_ASSERT_EQUAL(19, gtTestMapBuffer[0].iKey);
}

/* ============================================================================
 * Test Cases: Edge Cases
 * ============================================================================ */

static void test_map_single_element_capacity(void)
{
    TEST_MAP_T tSmallMap;
    TEST_MAP_ENTRY_T tSmallBuffer[1];
    tSmallBuffer[0].bIsNull = true;
    
    tSmallMap.ptBuffer = tSmallBuffer;
    tSmallMap.tArray.ptBuffer = tSmallBuffer;
    
    // Initialize array with capacity 1
    JUNO_STATUS_T tStatus = JunoDs_ArrayInit(&tSmallMap.tArray.tRoot, &gtTestMapArrayApi, 1, NULL, NULL);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Initialize map
    tStatus = JunoDs_MapInit(&tSmallMap.tRoot, &gtTestMapEntryHashablePointerApi, &gtTestMapEntryValuePointerApi, &tSmallMap.tArray.tRoot, NULL, NULL);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Set one entry
    TEST_MAP_ENTRY_T tEntry = CreateTestEntry(42, 100);
    JUNO_POINTER_T tPointer = TestMapEntry_PointerInit(&tEntry);
    tStatus = JunoDs_MapSet(&tSmallMap.tRoot, tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    // Verify it's stored
    TEST_ASSERT_EQUAL(42, tSmallBuffer[0].iKey);
    TEST_ASSERT_EQUAL(100, tSmallBuffer[0].iValue);
    
    // Attempt to add another (should fail - table full)
    TEST_MAP_ENTRY_T tEntry2 = CreateTestEntry(99, 200);
    JUNO_POINTER_T tPointer2 = TestMapEntry_PointerInit(&tEntry2);
    tStatus = JunoDs_MapSet(&tSmallMap.tRoot, tPointer2);
    TEST_ASSERT_EQUAL(JUNO_STATUS_TABLE_FULL_ERROR, tStatus);
}

static void test_map_set_get_remove_cycle(void)
{
    JUNO_STATUS_T tStatus = InitTestMap(&gtTestMap, TEST_MAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    for (uint32_t i = 0; i < 5; i++) {
        // Set
        TEST_MAP_ENTRY_T tSetEntry = CreateTestEntry(i, i * 100);
        JUNO_POINTER_T tSetPointer = TestMapEntry_PointerInit(&tSetEntry);
        tStatus = JunoDs_MapSet(&gtTestMap.tRoot, tSetPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        
        // Get
        TEST_MAP_ENTRY_T tGetEntry = CreateTestEntry(i, 0);
        JUNO_POINTER_T tGetPointer = TestMapEntry_PointerInit(&tGetEntry);
        JUNO_RESULT_POINTER_T tResult = JunoDs_MapGet(&gtTestMap.tRoot, tGetPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
        TEST_ASSERT_EQUAL(i * 100, ((TEST_MAP_ENTRY_T *)tResult.tOk.pvAddr)->iValue);
        
        // Remove
        tStatus = JunoDs_MapRemove(&gtTestMap.tRoot, tGetPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        
        // Verify removed (Get should fail)
        tResult = JunoDs_MapGet(&gtTestMap.tRoot, tGetPointer);
        TEST_ASSERT_EQUAL(JUNO_STATUS_DNE_ERROR, tResult.tStatus);
    }
}

/* ============================================================================
 * Test Cases: API Verification
 * ============================================================================ */

static void test_map_verify_valid_map(void)
{
    JUNO_STATUS_T tStatus = InitTestMap(&gtTestMap, TEST_MAP_CAPACITY);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    
    tStatus = JunoDs_MapVerify(&gtTestMap.tRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_map_verify_null_map(void)
{
    JUNO_STATUS_T tStatus = JunoDs_MapVerify(NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

// API verification tests removed - map API is now internal to implementation

/* ============================================================================
 * Test Cases: Pointer API Functions
 * ============================================================================ */

static void test_map_entry_copy_nominal(void)
{
    TEST_MAP_ENTRY_T tSrc = CreateTestEntry(42, 100);
    TEST_MAP_ENTRY_T tDest = {0};
    
    JUNO_POINTER_T tSrcPointer = {&gtTestMapEntryPointerApi, &tSrc, sizeof(TEST_MAP_ENTRY_T), alignof(TEST_MAP_ENTRY_T)};
    JUNO_POINTER_T tDestPointer = {&gtTestMapEntryPointerApi, &tDest, sizeof(TEST_MAP_ENTRY_T), alignof(TEST_MAP_ENTRY_T)};
    
    JUNO_STATUS_T tStatus = TestMapEntry_Copy(tDestPointer, tSrcPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(42, tDest.iKey);
    TEST_ASSERT_EQUAL(100, tDest.iValue);
    TEST_ASSERT_FALSE(tDest.bIsNull);
}

static void test_map_entry_copy_null_dest(void)
{
    TEST_MAP_ENTRY_T tSrc = CreateTestEntry(42, 100);
    JUNO_POINTER_T tSrcPointer = {&gtTestMapEntryPointerApi, &tSrc, sizeof(TEST_MAP_ENTRY_T), alignof(TEST_MAP_ENTRY_T)};
    JUNO_POINTER_T tDestPointer = {&gtTestMapEntryPointerApi, NULL, sizeof(TEST_MAP_ENTRY_T), alignof(TEST_MAP_ENTRY_T)};
    
    JUNO_STATUS_T tStatus = TestMapEntry_Copy(tDestPointer, tSrcPointer);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_map_entry_reset_nominal(void)
{
    TEST_MAP_ENTRY_T tEntry = CreateTestEntry(42, 100);
    JUNO_POINTER_T tPointer = {&gtTestMapEntryPointerApi, &tEntry, sizeof(TEST_MAP_ENTRY_T), alignof(TEST_MAP_ENTRY_T)};
    
    JUNO_STATUS_T tStatus = TestMapEntry_Reset(tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(0, tEntry.iKey);
    TEST_ASSERT_EQUAL(0, tEntry.iValue);
    TEST_ASSERT_TRUE(tEntry.bIsNull);
}

static void test_map_entry_equals_same_key(void)
{
    TEST_MAP_ENTRY_T tEntry1 = CreateTestEntry(42, 100);
    TEST_MAP_ENTRY_T tEntry2 = CreateTestEntry(42, 200); // Same key, different value
    
    JUNO_POINTER_T tPtr1 = {&gtTestMapEntryPointerApi, &tEntry1, sizeof(TEST_MAP_ENTRY_T), alignof(TEST_MAP_ENTRY_T)};
    JUNO_POINTER_T tPtr2 = {&gtTestMapEntryPointerApi, &tEntry2, sizeof(TEST_MAP_ENTRY_T), alignof(TEST_MAP_ENTRY_T)};
    
    JUNO_RESULT_BOOL_T tResult = TestMapEntry_Equals(tPtr1, tPtr2);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    TEST_ASSERT_TRUE(tResult.tOk);
}

static void test_map_entry_equals_different_key(void)
{
    TEST_MAP_ENTRY_T tEntry1 = CreateTestEntry(42, 100);
    TEST_MAP_ENTRY_T tEntry2 = CreateTestEntry(43, 100);
    
    JUNO_POINTER_T tPtr1 = {&gtTestMapEntryPointerApi, &tEntry1, sizeof(TEST_MAP_ENTRY_T), alignof(TEST_MAP_ENTRY_T)};
    JUNO_POINTER_T tPtr2 = {&gtTestMapEntryPointerApi, &tEntry2, sizeof(TEST_MAP_ENTRY_T), alignof(TEST_MAP_ENTRY_T)};
    
    JUNO_RESULT_BOOL_T tResult = TestMapEntry_Equals(tPtr1, tPtr2);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    TEST_ASSERT_FALSE(tResult.tOk);
}

static void test_map_entry_hash_nominal(void)
{
    TEST_MAP_ENTRY_T tEntry = CreateTestEntry(42, 100);
    JUNO_POINTER_T tPointer = TestMapEntry_PointerInit(&tEntry);
    
    JUNO_RESULT_SIZE_T tResult = TestMapEntry_Hash(tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    TEST_ASSERT_EQUAL(42, tResult.tOk);
}

static void test_map_entry_is_null_true(void)
{
    TEST_MAP_ENTRY_T tEntry = {0, 0, true};
    JUNO_POINTER_T tPointer = TestMapEntry_PointerInit(&tEntry);
    
    JUNO_RESULT_BOOL_T tResult = TestMapEntry_IsNull(tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    TEST_ASSERT_TRUE(tResult.tOk);
}

static void test_map_entry_is_null_false(void)
{
    TEST_MAP_ENTRY_T tEntry = CreateTestEntry(42, 100);
    JUNO_POINTER_T tPointer = TestMapEntry_PointerInit(&tEntry);
    
    JUNO_RESULT_BOOL_T tResult = TestMapEntry_IsNull(tPointer);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tResult.tStatus);
    TEST_ASSERT_FALSE(tResult.tOk);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

int main(void)
{
    UNITY_BEGIN();
    
    // Initialization tests
    RUN_TEST(test_map_init_nominal);
    RUN_TEST(test_map_init_null_map);
    RUN_TEST(test_map_init_null_api);
    RUN_TEST(test_map_init_zero_capacity);
    
    // Set operation tests
    RUN_TEST(test_map_set_single_item);
    RUN_TEST(test_map_set_multiple_items);
    RUN_TEST(test_map_set_update_existing);
    RUN_TEST(test_map_set_with_collision);
    RUN_TEST(test_map_set_null_map);
    RUN_TEST(test_map_set_table_full);
    
    // Get operation tests
    RUN_TEST(test_map_get_existing_item);
    RUN_TEST(test_map_get_nonexistent_item);
    RUN_TEST(test_map_get_after_collision);
    RUN_TEST(test_map_get_null_map);
    
    // Remove operation tests
    RUN_TEST(test_map_remove_existing_item);
    RUN_TEST(test_map_remove_nonexistent_item);
    RUN_TEST(test_map_remove_after_collision);
    RUN_TEST(test_map_remove_null_map);
    
    // Collision handling tests
    RUN_TEST(test_map_multiple_collisions_linear_probing);
    RUN_TEST(test_map_wraparound_probing);
    
    // Edge case tests
    RUN_TEST(test_map_single_element_capacity);
    RUN_TEST(test_map_set_get_remove_cycle);
    
    // API verification tests
    RUN_TEST(test_map_verify_valid_map);
    RUN_TEST(test_map_verify_null_map);
    
    // Pointer API tests
    RUN_TEST(test_map_entry_copy_nominal);
    RUN_TEST(test_map_entry_copy_null_dest);
    RUN_TEST(test_map_entry_reset_nominal);
    RUN_TEST(test_map_entry_equals_same_key);
    RUN_TEST(test_map_entry_equals_different_key);
    RUN_TEST(test_map_entry_hash_nominal);
    RUN_TEST(test_map_entry_is_null_true);
    RUN_TEST(test_map_entry_is_null_false);
    
    return UNITY_END();
}
