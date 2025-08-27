#include <cstddef>
#include <cstdint>

extern "C" {
#include "unity.h"
}

#include "juno/ds/juno_buff.hpp"

using namespace juno;
using namespace juno::buff;

void setUp(void) {}
void tearDown(void) {}

static void test_queue(void)
{
    constexpr size_t N = 10;
    // Underlying buffer storage

    // Initialize queue via API
    auto api_q     = JUNO_QUEUE_T<uint8_t, N>::NewApi();
    QUEUE_T<uint8_t, N> queueRoot{};
    auto tStatus = JUNO_QUEUE_T<uint8_t, N>::New(queueRoot, api_q, nullptr, nullptr);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    // — fill to capacity
    for (uint8_t i = 1; i <= N; ++i) {
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, api_q.Enqueue(queueRoot, i));
    }
    // — overflow yields INVALID_SIZE_ERROR
    TEST_ASSERT_EQUAL(JUNO_STATUS_INVALID_SIZE_ERROR, api_q.Enqueue(queueRoot, 0));

    // — drain completely
    for (uint8_t i = 1; i <= N; ++i) {
        auto r = api_q.Dequeue(queueRoot);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, r.tStatus);
        TEST_ASSERT_EQUAL_UINT8(i, r.tOk);
    }
    // — underflow yields ERR
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, api_q.Dequeue(queueRoot).tStatus);

    // — immediate enqueue/dequeue cycles
    for (uint8_t i = 1; i <= N; ++i) {
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, api_q.Enqueue(queueRoot, i));
        auto r = api_q.Dequeue(queueRoot);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, r.tStatus);
        TEST_ASSERT_EQUAL_UINT8(i, r.tOk);
    }
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, api_q.Dequeue(queueRoot).tStatus);

    // — wrap-around scenario
    // enqueue half…
    for (uint8_t i = 1; i <= N/2; ++i) {
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, api_q.Enqueue(queueRoot, i));
    }
    // dequeue half…
    for (uint8_t i = 1; i <= N/2; ++i) {
        auto r = api_q.Dequeue(queueRoot);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, r.tStatus);
        TEST_ASSERT_EQUAL_UINT8(i, r.tOk);
    }
    // now buffer is empty; enqueue N new items (values 6..15)
    for (uint8_t v = (N/2 + 1); v <= (N/2 + N); ++v) {
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, api_q.Enqueue(queueRoot, v));
    }
    // overflow again
    TEST_ASSERT_EQUAL(JUNO_STATUS_INVALID_SIZE_ERROR, api_q.Enqueue(queueRoot, 0));
    // drain all and check ordering
    for (uint8_t v = (N/2 + 1); v <= (N/2 + N); ++v) {
        auto r = api_q.Dequeue(queueRoot);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, r.tStatus);
        TEST_ASSERT_EQUAL_UINT8(v, r.tOk);
    }
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, api_q.Dequeue(queueRoot).tStatus);
}

static void test_stack(void)
{
    constexpr size_t N = 10;

    // Initialize stack via API
    auto api_s     = JUNO_STACK_T<uint8_t, N>::NewApi();
    STACK_T<uint8_t, N> stackRoot{};
    auto tStatus = JUNO_STACK_T<uint8_t, N>::New(stackRoot, api_s, nullptr, nullptr);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    // — fill to capacity
    for (uint8_t i = 1; i <= N; ++i) {
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, api_s.Push(stackRoot, i));
    }
    // — overflow yields INVALID_SIZE_ERROR
    TEST_ASSERT_EQUAL(JUNO_STATUS_INVALID_SIZE_ERROR, api_s.Push(stackRoot, 0));

    // — drain completely (LIFO)
    for (int i = static_cast<int>(N); i >= 1; --i) {
        auto r = api_s.Pop(stackRoot);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS,   r.tStatus);
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(i), r.tOk);
    }
    // — underflow yields ERR
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, api_s.Pop(stackRoot).tStatus);

    // — immediate push/pop cycles
    for (uint8_t i = 1; i <= N; ++i) {
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, api_s.Push(stackRoot, i));
        auto r = api_s.Pop(stackRoot);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, r.tStatus);
        TEST_ASSERT_EQUAL_UINT8(i, r.tOk);
    }
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, api_s.Pop(stackRoot).tStatus);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_queue);
    RUN_TEST(test_stack);
    return UNITY_END();
}

