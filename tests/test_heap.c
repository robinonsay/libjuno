#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include "juno/ds/heap_api.h"
#include <stdbool.h>
#include <stddef.h>

/* --------------------------------------------------------------------------------------
   A test heap type that derives from JUNO_DS_HEAP_ROOT_T (same pattern as heap_example.c)
   It also includes a few flags we can toggle to force error paths in the API.
---------------------------------------------------------------------------------------*/
typedef struct TEST_HEAP JUNO_MODULE_DERIVE(JUNO_DS_HEAP_ROOT_T,
    int   data[64];
    bool  fail_compare;
    bool  fail_swap;
    bool  fail_reset;
) TEST_HEAP;

/* --------------------------------------------------------------------------------------
   Test Heap API implementations
---------------------------------------------------------------------------------------*/

/// Max-heap comparator: returns true when parent >= child
static JUNO_DS_HEAP_COMPARE_RESULT_T CompareMax(JUNO_DS_HEAP_ROOT_T *ptHeap, size_t parent, size_t child)
{
    JUNO_DS_HEAP_COMPARE_RESULT_T tResult = { JunoDs_Heap_Verify(ptHeap), false };
    if (tResult.tStatus != JUNO_STATUS_SUCCESS) { return tResult; }

    TEST_HEAP *h = (TEST_HEAP*)ptHeap;
    if (h->fail_compare) {
        tResult.tStatus = JUNO_STATUS_ERR;
        return tResult;
    }
    tResult.tSuccess = (h->data[parent] >= h->data[child]);
    return tResult;
}

/// Min-heap comparator: returns true when parent <= child
static JUNO_DS_HEAP_COMPARE_RESULT_T CompareMin(JUNO_DS_HEAP_ROOT_T *ptHeap, size_t parent, size_t child)
{
    JUNO_DS_HEAP_COMPARE_RESULT_T tResult = { JunoDs_Heap_Verify(ptHeap), false };
    if (tResult.tStatus != JUNO_STATUS_SUCCESS) { return tResult; }

    TEST_HEAP *h = (TEST_HEAP*)ptHeap;
    if (h->fail_compare) {
        tResult.tStatus = JUNO_STATUS_ERR;
        return tResult;
    }
    tResult.tSuccess = (h->data[parent] <= h->data[child]);
    return tResult;
}

static JUNO_STATUS_T SwapMaybeFail(JUNO_DS_HEAP_ROOT_T *ptHeap, size_t iFrom, size_t iTo)
{
    JUNO_STATUS_T s = JunoDs_Heap_Verify(ptHeap);
    if (s != JUNO_STATUS_SUCCESS) { return s; }

    TEST_HEAP *h = (TEST_HEAP*)ptHeap;
    if (h->fail_swap) {
        return JUNO_STATUS_ERR;
    }
    int tmp = h->data[iTo];
    h->data[iTo] = h->data[iFrom];
    h->data[iFrom] = tmp;
    return s;
}

static JUNO_STATUS_T ResetMaybeFail(JUNO_DS_HEAP_ROOT_T *ptHeap, size_t iIndex)
{
    JUNO_STATUS_T s = JunoDs_Heap_Verify(ptHeap);
    if (s != JUNO_STATUS_SUCCESS) { return s; }

    TEST_HEAP *h = (TEST_HEAP*)ptHeap;
    if (h->fail_reset) {
        return JUNO_STATUS_ERR;
    }
    h->data[iIndex] = 0;
    return s;
}

static const JUNO_DS_HEAP_API_T kMaxApi = { CompareMax, SwapMaybeFail, ResetMaybeFail };
static const JUNO_DS_HEAP_API_T kMinApi = { CompareMin, SwapMaybeFail, ResetMaybeFail };

/* --------------------------------------------------------------------------------------
   Helpers
---------------------------------------------------------------------------------------*/
static void make_heap(TEST_HEAP *h, const JUNO_DS_HEAP_API_T *api, size_t capacity)
{
    for (size_t i = 0; i < 64; ++i) { h->data[i] = 0; }
    h->fail_compare = false;
    h->fail_swap    = false;
    h->fail_reset   = false;
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoDs_Heap_Init(&h->tRoot, api, capacity));
    h->tRoot.zLength = 0;
}

static void assert_max_heap_property(TEST_HEAP *h)
{
    size_t n = h->tRoot.zLength;
    for (size_t i = 0; i < n; ++i) {
        size_t l = 2*i + 1;
        size_t r = 2*i + 2;
        if (l < n) { TEST_ASSERT_TRUE_MESSAGE(h->data[i] >= h->data[l], "Max-heap property violated on left child"); }
        if (r < n) { TEST_ASSERT_TRUE_MESSAGE(h->data[i] >= h->data[r], "Max-heap property violated on right child"); }
    }
}

static void assert_min_heap_property(TEST_HEAP *h)
{
    size_t n = h->tRoot.zLength;
    for (size_t i = 0; i < n; ++i) {
        size_t l = 2*i + 1;
        size_t r = 2*i + 2;
        if (l < n) { TEST_ASSERT_TRUE_MESSAGE(h->data[i] <= h->data[l], "Min-heap property violated on left child"); }
        if (r < n) { TEST_ASSERT_TRUE_MESSAGE(h->data[i] <= h->data[r], "Min-heap property violated on right child"); }
    }
}

/* --------------------------------------------------------------------------------------
   Unity fixtures
---------------------------------------------------------------------------------------*/
void setUp(void) {}
void tearDown(void) {}

/* --------------------------------------------------------------------------------------
   Tests
---------------------------------------------------------------------------------------*/

static void test_verify_null_ptr_and_invalid_fields(void)
{
    // Null heap
    TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, JunoDs_Heap_Verify(NULL));

    // Heap exists, api is NULL -> NULLPTR_ERROR
    TEST_HEAP h = {0};
    h.tRoot.ptApi = NULL;
    h.tRoot.zCapacity = 10;
    TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, JunoDs_Heap_Verify(&h.tRoot));

    // Heap exists, api set, capacity == 0 -> NULLPTR_ERROR
    h.tRoot.ptApi = &kMaxApi;
    h.tRoot.zCapacity = 0;
    TEST_ASSERT_EQUAL(JUNO_STATUS_NULLPTR_ERROR, JunoDs_Heap_Verify(&h.tRoot));
}

static void test_child_index_helpers_success_and_failure(void)
{
    TEST_HEAP h;
    make_heap(&h, &kMaxApi, 10);
    h.tRoot.zLength = 10;
    // Success cases
    JUNO_DS_HEAP_INDEX_OPTION_RESULT_T res;
    res = JunoDs_Heap_ChildGetLeft(&h.tRoot, 2);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, res.tStatus);
    TEST_ASSERT_EQUAL_size_t(5, res.tSuccess.tSome);

    res = JunoDs_Heap_ChildGetRight(&h.tRoot, 3);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, res.tStatus);
    TEST_ASSERT_EQUAL_size_t(8, res.tSuccess.tSome);

    res = JunoDs_Heap_ChildGetParent(&h.tRoot, 5);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, res.tStatus);
    TEST_ASSERT_EQUAL_size_t(2, res.tSuccess.tSome);

    // Failure cases (index > capacity checks)
    make_heap(&h, &kMaxApi, 2);
    res = JunoDs_Heap_ChildGetLeft(&h.tRoot, 3);  // 2*3+1 = 7 > 2 -> error
    TEST_ASSERT_FALSE(res.tSuccess.bIsSome);

    res = JunoDs_Heap_ChildGetRight(&h.tRoot, 2); // 2*2+2 = 6 > 2 -> error
    TEST_ASSERT_FALSE(res.tSuccess.bIsSome);

    res = JunoDs_Heap_ChildGetParent(&h.tRoot, 7); // (7-1)/2 = 3 > 2 -> error
    TEST_ASSERT_FALSE(res.tSuccess.bIsSome);
}

static void test_init_insert_update_and_capacity_overflow_maxheap(void)
{
    TEST_HEAP h;
    make_heap(&h, &kMaxApi, 16);

    // Insert a fixed pattern, bubbling up each time
    const int vals[] = {5, 3, 8, 1, 6, 9, 2, 7, 4, 0};
    const size_t N = sizeof(vals)/sizeof(vals[0]);

    for (size_t i = 0; i < N; ++i) {
        JUNO_DS_HEAP_INDEX_RESULT_T ir = JunoDs_Heap_Insert(&h.tRoot);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, ir.tStatus);
        TEST_ASSERT_EQUAL_size_t(i, ir.tSuccess);
        h.data[ir.tSuccess] = vals[i];
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoDs_Heap_Update(&h.tRoot));
    }

    // Heap property should hold for max-heap
    assert_max_heap_property(&h);

    // SiftDown on an already-valid heap should break early and succeed
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoDs_Heap_SiftDown(&h.tRoot, 0));

    // Fill to capacity
    while (h.tRoot.zLength < h.tRoot.zCapacity) {
        JUNO_DS_HEAP_INDEX_RESULT_T ir = JunoDs_Heap_Insert(&h.tRoot);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, ir.tStatus);
        h.data[ir.tSuccess] = (int)ir.tSuccess;
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoDs_Heap_Update(&h.tRoot));
    }
    // One more insert must fail
    JUNO_DS_HEAP_INDEX_RESULT_T overflow = JunoDs_Heap_Insert(&h.tRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, overflow.tStatus);
}

static void test_min_heap_behaviour_and_delete_sequence(void)
{
    TEST_HEAP h;
    make_heap(&h, &kMinApi, 20);

    const int vals[] = {9, 1, 8, 2, 7, 3, 6, 4, 5, 0};
    const size_t N = sizeof(vals)/sizeof(vals[0]);

    for (size_t i = 0; i < N; ++i) {
        JUNO_DS_HEAP_INDEX_RESULT_T ir = JunoDs_Heap_Insert(&h.tRoot);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, ir.tStatus);
        h.data[ir.tSuccess] = vals[i];
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoDs_Heap_Update(&h.tRoot));
    }
    assert_min_heap_property(&h);

    // Delete down to one element: each delete should succeed and the popped min should be non-decreasing
    int last = -1000000; // effectively -INF
    while (h.tRoot.zLength > 1) {
        int root_val = h.data[0];
        TEST_ASSERT_TRUE(root_val >= last);
        last = root_val;
        JUNO_STATUS_T tStatus = JunoDs_Heap_Delete(&h.tRoot);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
        assert_min_heap_property(&h);
    }

    // Deleting the final element should return an error (SiftDown on length==0)
    TEST_ASSERT_EQUAL(1u, h.tRoot.zLength);
    JUNO_STATUS_T st = JunoDs_Heap_Delete(&h.tRoot);
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, st);
    TEST_ASSERT_EQUAL(0u, h.tRoot.zLength);
}

static void test_heapify_from_unsorted_array_maxheap(void)
{
    TEST_HEAP h;
    make_heap(&h, &kMaxApi, 32);

    // Put an arbitrary unsorted array directly and then heapify
    int vals[] = {7, 12, 3, 1, 25, 9, 8, 15, 2, 4, 11, 5, 6, 13, 10, 14};
    size_t N = sizeof(vals)/sizeof(vals[0]);

    for (size_t i = 0; i < N; ++i) { h.data[i] = vals[i]; }
    h.tRoot.zLength = N;

    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoDs_Heap_Heapify(&h.tRoot));
    assert_max_heap_property(&h);
}

static void test_heapify_triggers_continue_when_children_exceed_capacity(void)
{
    TEST_HEAP h;
    // capacity small, length large -> left/right (≈ length) exceed capacity for higher parents
    make_heap(&h, &kMaxApi, 10);
    h.tRoot.zLength = 21;

    // Fill some values just to avoid undefined comparisons if reached
    for (size_t i = 0; i < h.tRoot.zLength && i < 64; ++i) {
        h.data[i] = (int)(100 - i);
    }

    // This should succeed; internal ASSERT_SUCCESS(...) uses 'continue' when children are > capacity.
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, JunoDs_Heap_Heapify(&h.tRoot));
}

static void test_heapify_and_siftdown_guard_against_zero_length(void)
{
    TEST_HEAP h;
    make_heap(&h, &kMaxApi, 8);

    // Heapify with zero length -> ERR
    h.tRoot.zLength = 0;
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, JunoDs_Heap_Heapify(&h.tRoot));

    // SiftDown with zero length -> ERR
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, JunoDs_Heap_SiftDown(&h.tRoot, 0));
}

static void test_update_with_zero_length_and_with_small_capacity_parent_error(void)
{
    TEST_HEAP h;
    make_heap(&h, &kMaxApi, 8);

    // zLength = 0, Update should error (parent computation out of capacity)
    h.tRoot.zLength = 0;
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, JunoDs_Heap_Update(&h.tRoot));

    // Make capacity very small relative to length to force parent>capacity
    make_heap(&h, &kMaxApi, 1);
    h.tRoot.zLength = 6; // iCurrentIndex = 5, parent = 2 > capacity(1) -> error
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, JunoDs_Heap_Update(&h.tRoot));
}

static void test_update_and_siftdown_propagate_compare_errors(void)
{
    TEST_HEAP h;
    make_heap(&h, &kMaxApi, 8);

    // Build a tiny heap of 2 elements
    JUNO_DS_HEAP_INDEX_RESULT_T ir = JunoDs_Heap_Insert(&h.tRoot);
    h.data[ir.tSuccess] = 1;
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoDs_Heap_Update(&h.tRoot));

    ir = JunoDs_Heap_Insert(&h.tRoot);
    h.data[ir.tSuccess] = 2;
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoDs_Heap_Update(&h.tRoot));

    // Force comparator failure for Update
    h.fail_compare = true;
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, JunoDs_Heap_Update(&h.tRoot));
    h.fail_compare = false;

    // Build a 3-node heap and force comparator failure in SiftDown
    make_heap(&h, &kMaxApi, 8);
    h.tRoot.zLength = 3;
    h.data[0] = 1; h.data[1] = 2; h.data[2] = 3;
    h.fail_compare = true;
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, JunoDs_Heap_SiftDown(&h.tRoot, 0));
    h.fail_compare = false;
}

static void test_siftdown_swaps_and_propagates_swap_error(void)
{
    TEST_HEAP h;
    make_heap(&h, &kMaxApi, 8);

    // Configure a case where swap is required: root < left (max-heap)
    h.tRoot.zLength = 3;
    h.data[0] = 1;  // root
    h.data[1] = 10; // left
    h.data[2] = 5;  // right

    // Force swap error path
    h.fail_swap = true;
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, JunoDs_Heap_SiftDown(&h.tRoot, 0));
    h.fail_swap = false;

    // Now let it succeed and actually perform the swap
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoDs_Heap_SiftDown(&h.tRoot, 0));
    TEST_ASSERT_TRUE(h.data[0] >= h.data[1] && h.data[0] >= h.data[2]);
}

static void test_delete_on_empty_and_delete_reset_behaviour(void)
{
    TEST_HEAP h;
    make_heap(&h, &kMaxApi, 8);

    // Empty delete -> error
    h.tRoot.zLength = 0;
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, JunoDs_Heap_Delete(&h.tRoot));

    // Two elements; deleting once should succeed and Reset should zero the old-root slot
    make_heap(&h, &kMaxApi, 8);
    JUNO_DS_HEAP_INDEX_RESULT_T ir;

    ir = JunoDs_Heap_Insert(&h.tRoot); h.data[ir.tSuccess] = 3; TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoDs_Heap_Update(&h.tRoot));
    ir = JunoDs_Heap_Insert(&h.tRoot); h.data[ir.tSuccess] = 1; TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoDs_Heap_Update(&h.tRoot));

    size_t last_index_before = h.tRoot.zLength - 1;
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoDs_Heap_Delete(&h.tRoot));
    TEST_ASSERT_EQUAL(1u, h.tRoot.zLength);
    TEST_ASSERT_EQUAL(0, h.data[last_index_before]); // Reset should have zeroed this

    // Now demonstrate that Delete ignores Reset errors (it still returns success)
    make_heap(&h, &kMaxApi, 8);
    ir = JunoDs_Heap_Insert(&h.tRoot); h.data[ir.tSuccess] = 5; TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoDs_Heap_Update(&h.tRoot));
    ir = JunoDs_Heap_Insert(&h.tRoot); h.data[ir.tSuccess] = 4; TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoDs_Heap_Update(&h.tRoot));
    last_index_before = h.tRoot.zLength - 1;

    h.fail_reset = true; // Reset will return error, but Delete shouldn't propagate it
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, JunoDs_Heap_Delete(&h.tRoot));
    TEST_ASSERT_EQUAL(1u, h.tRoot.zLength);
    // Because Reset failed, the last slot should *not* be zeroed
    TEST_ASSERT_NOT_EQUAL(0, h.data[last_index_before]);
    h.fail_reset = false;
}

static void test_siftdown_error_when_right_child_index_exceeds_capacity(void)
{
    TEST_HEAP h;
    make_heap(&h, &kMaxApi, 1); // capacity=1 -> left(1) ok, right(2) -> error
    h.tRoot.zLength = 1;        // pass the "length <= 0" check

    // Even on a trivial heap, right child index should exceed capacity and cause error
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, JunoDs_Heap_SiftDown(&h.tRoot, 0));
}

/* --------------------------------------------------------------------------------------
   Runner
---------------------------------------------------------------------------------------*/
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_verify_null_ptr_and_invalid_fields);
    RUN_TEST(test_child_index_helpers_success_and_failure);

    RUN_TEST(test_init_insert_update_and_capacity_overflow_maxheap);
    RUN_TEST(test_min_heap_behaviour_and_delete_sequence);

    RUN_TEST(test_heapify_from_unsorted_array_maxheap);
    RUN_TEST(test_heapify_triggers_continue_when_children_exceed_capacity);
    RUN_TEST(test_heapify_and_siftdown_guard_against_zero_length);

    RUN_TEST(test_update_with_zero_length_and_with_small_capacity_parent_error);
    RUN_TEST(test_update_and_siftdown_propagate_compare_errors);
    RUN_TEST(test_siftdown_swaps_and_propagates_swap_error);

    RUN_TEST(test_delete_on_empty_and_delete_reset_behaviour);
    RUN_TEST(test_siftdown_error_when_right_child_index_exceeds_capacity);

    return UNITY_END();
}
