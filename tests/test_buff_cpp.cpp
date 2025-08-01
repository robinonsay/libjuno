#include <cstddef>
#include <cstdint>

extern "C" {
#include "unity.h"
}

#include "juno/buff/buff_api.hpp"

using namespace juno;
using namespace juno::buff;

void setUp(void) {}
void tearDown(void) {}

static void test_queue(void)
{
    constexpr size_t N = 10;
    // Underlying buffer storage
    ARRAY_T<uint8_t, N> buffer{};

    // Initialize queue via API
    auto api_q     = NewQueueApi<uint8_t, N>();
    auto result_q  = QUEUE_ROOT_T<uint8_t, N>::New(&api_q, buffer, nullptr, nullptr);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, result_q.tStatus);
    auto queueRoot = result_q.tSuccess;

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
        TEST_ASSERT_EQUAL_UINT8(i, r.tSuccess);
    }
    // — underflow yields ERR
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, api_q.Dequeue(queueRoot).tStatus);

    // — immediate enqueue/dequeue cycles
    for (uint8_t i = 1; i <= N; ++i) {
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, api_q.Enqueue(queueRoot, i));
        auto r = api_q.Dequeue(queueRoot);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, r.tStatus);
        TEST_ASSERT_EQUAL_UINT8(i, r.tSuccess);
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
        TEST_ASSERT_EQUAL_UINT8(i, r.tSuccess);
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
        TEST_ASSERT_EQUAL_UINT8(v, r.tSuccess);
    }
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, api_q.Dequeue(queueRoot).tStatus);
}

static void test_stack(void)
{
    constexpr size_t N = 10;
    // Underlying buffer storage
    ARRAY_T<uint8_t, N> buffer{};

    // Initialize stack via API
    auto api_s     = NewStackApi<uint8_t, N>();
    auto result_s  = STACK_ROOT_T<uint8_t, N>::New(&api_s, buffer, nullptr, nullptr);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, result_s.tStatus);
    auto stackRoot = result_s.tSuccess;

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
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(i), r.tSuccess);
    }
    // — underflow yields ERR
    TEST_ASSERT_EQUAL(JUNO_STATUS_ERR, api_s.Pop(stackRoot).tStatus);

    // — immediate push/pop cycles
    for (uint8_t i = 1; i <= N; ++i) {
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, api_s.Push(stackRoot, i));
        auto r = api_s.Pop(stackRoot);
        TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, r.tStatus);
        TEST_ASSERT_EQUAL_UINT8(i, r.tSuccess);
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

