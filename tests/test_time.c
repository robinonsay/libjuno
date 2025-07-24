#include "juno/module.h"
#include "juno/status.h"
#include "juno/time/time_api.h"
#include "unity.h"
#include "unity_internals.h"
#include <stdbool.h>
#include <time.h>
#include <stdint.h>  // For UINT64_MAX

// Bind the time API into a module for testing
union JUNO_TIME_TAG JUNO_MODULE(JUNO_TIME_API_T, JUNO_TIME_ROOT_T,
    JUNO_MODULE_EMPTY
);

// Now implementation uses real clock; verify it returns reasonable values
static JUNO_STATUS_T Now(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T *ptRetTime)
{
    struct timespec tTimeNow = {0};
    clock_gettime(CLOCK_REALTIME, &tTimeNow);
    JUNO_STATUS_T tStatus = JunoTime_NanosToTimestamp(ptTime, tTimeNow.tv_nsec, ptRetTime);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    ptRetTime->iSeconds = tTimeNow.tv_sec;
    return tStatus;
}

// Stub SleepTo always succeeds
static JUNO_STATUS_T SleepTo(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tTimeToWakeup)
{
    (void)ptTime; (void)tTimeToWakeup;
    return JUNO_STATUS_SUCCESS;
}

// Stub Sleep always succeeds
static JUNO_STATUS_T Sleep(JUNO_TIME_T *ptTime, JUNO_TIMESTAMP_T tDuration)
{
    (void)ptTime; (void)tDuration;
    return JUNO_STATUS_SUCCESS;
}

// Define the API vtable for tests
const JUNO_TIME_API_T tTimeApi =
{
    .Now               = Now,
    .AddTime           = JunoTime_AddTime,
    .SubtractTime      = JunoTime_SubtractTime,
    .SleepTo           = SleepTo,
    .Sleep             = Sleep,
    .TimestampToNanos  = JunoTime_TimestampToNanos,
    .TimestampToMicros = JunoTime_TimestampToMicros,
    .TimestampToMillis = JunoTime_TimestampToMillis,
    .NanosToTimestamp  = JunoTime_NanosToTimestamp,
    .MicrosToTimestamp = JunoTime_MicrosToTimestamp,
    .MillisToTimestamp = JunoTime_MillisToTimestamp,
};

// Global module instance
JUNO_TIME_T tTimeMod = {0};

void setUp(void)
{
    // Initialize module with our API
    tTimeMod = (JUNO_TIME_T){0};
    tTimeMod.ptApi = &tTimeApi;
}

void tearDown(void)
{
    // Reset module
    tTimeMod = (JUNO_TIME_T){0};
}

// Positive test: AddTime without subseconds overflow
static void test_AddTime_no_subseconds_overflow(void)
{
    JUNO_TIMESTAMP_T ret = { .iSeconds = 2, .iSubSeconds = 100 };
    JUNO_TIMESTAMP_T add = { .iSeconds = 3, .iSubSeconds = 50 };
    JUNO_STATUS_T status = tTimeMod.ptApi->AddTime(&tTimeMod, &ret, add);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT64(5, ret.iSeconds);     // 2 + 3
    TEST_ASSERT_EQUAL_UINT64(150, ret.iSubSeconds); // 100 + 50
}

// Positive test: AddTime with subseconds overflow
static void test_AddTime_with_subseconds_overflow(void)
{
    const uint64_t max_sub = UINT64_MAX;
    JUNO_TIMESTAMP_T ret = { .iSeconds = 1, .iSubSeconds = max_sub - 5 };
    JUNO_TIMESTAMP_T add = { .iSeconds = 0, .iSubSeconds = 10 };
    // Difference = max_sub - (max_sub -5) = 5; 10 >=5 triggers overflow branch
    JUNO_STATUS_T status = tTimeMod.ptApi->AddTime(&tTimeMod, &ret, add);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT64(2, ret.iSeconds);         // 1 + 0 + overflow(1)
    TEST_ASSERT_EQUAL_UINT64(5, ret.iSubSeconds);      // 10 - 5
}

// Positive test: SubtractTime without borrow
static void test_SubtractTime_no_borrow(void)
{
    JUNO_TIMESTAMP_T ret = { .iSeconds = 10, .iSubSeconds = 1000 };
    JUNO_TIMESTAMP_T sub = { .iSeconds = 3, .iSubSeconds = 100 };

    JUNO_STATUS_T status = tTimeMod.ptApi->SubtractTime(&tTimeMod, &ret, sub);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT64(7, ret.iSeconds);       // 10 - 3
    TEST_ASSERT_EQUAL_UINT64(900, ret.iSubSeconds);  // 1000 - 100
}

// Positive test: SubtractTime with subseconds borrow
static void test_SubtractTime_borrow_subseconds(void)
{
    const uint64_t max_sub = UINT64_MAX;
    JUNO_TIMESTAMP_T ret = { .iSeconds = 10, .iSubSeconds = 2 };
    JUNO_TIMESTAMP_T sub = { .iSeconds = 3, .iSubSeconds = 100 };
    // 2 < 100 triggers borrow: seconds--, subseconds = max_sub - (100 - 2)
    JUNO_STATUS_T status = tTimeMod.ptApi->SubtractTime(&tTimeMod, &ret, sub);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT64(6, ret.iSeconds);                   // 10 - borrow(1)
    TEST_ASSERT_EQUAL_UINT64(max_sub - 98, ret.iSubSeconds);     // max - (100 - 2)
}

// Negative test: SubtractTime when subtracting more seconds than available
static void test_SubtractTime_invalid_lower_seconds(void)
{
    JUNO_TIMESTAMP_T ret = { .iSeconds = 1, .iSubSeconds = 10 };
    JUNO_TIMESTAMP_T sub = { .iSeconds = 2, .iSubSeconds = 0 };
    JUNO_STATUS_T status = tTimeMod.ptApi->SubtractTime(&tTimeMod, &ret, sub);
    TEST_ASSERT_EQUAL(JUNO_STATUS_INVALID_DATA_ERROR, status);
    TEST_ASSERT_EQUAL_UINT64(0, ret.iSeconds);
    TEST_ASSERT_EQUAL_UINT64(0, ret.iSubSeconds);
}

// Negative test: SubtractTime when seconds = 0 but subseconds < subtract
static void test_SubtractTime_invalid_zero_seconds_insufficient_subseconds(void)
{
    JUNO_TIMESTAMP_T ret = { .iSeconds = 0, .iSubSeconds = 5 };
    JUNO_TIMESTAMP_T sub = { .iSeconds = 0, .iSubSeconds = 10 };
    JUNO_STATUS_T status = tTimeMod.ptApi->SubtractTime(&tTimeMod, &ret, sub);
    TEST_ASSERT_EQUAL(JUNO_STATUS_INVALID_DATA_ERROR, status);
    TEST_ASSERT_EQUAL_UINT64(0, ret.iSeconds);
    TEST_ASSERT_EQUAL_UINT64(0, ret.iSubSeconds);
}

// Positive test: TimestampToNanos integer seconds
static void test_TimestampToNanos_success_integer(void)
{
    JUNO_TIMESTAMP_T t = { .iSeconds = 3, .iSubSeconds = 0 };
    uint64_t nanos = 0xDEADBEEF;
    JUNO_STATUS_T status = tTimeMod.ptApi->TimestampToNanos(&tTimeMod, t, &nanos);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT64(3000000000ULL, nanos);
}

// Positive test: TimestampToNanos fractional subseconds
static void test_TimestampToNanos_success_fractional(void)
{
    const uint64_t max_sub = UINT64_MAX;
    JUNO_TIMESTAMP_T t = { .iSeconds = 0, .iSubSeconds = max_sub };
    uint64_t nanos = 0;
    JUNO_STATUS_T status = tTimeMod.ptApi->TimestampToNanos(&tTimeMod, t, &nanos);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT64(1000000000ULL, nanos);
}

// Negative test: TimestampToNanos overflow detection
static void test_TimestampToNanos_overflow(void)
{
    JUNO_TIMESTAMP_T t = { .iSeconds = UINT64_MAX, .iSubSeconds = 0 };
    uint64_t nanos = 123;
    JUNO_STATUS_T status = tTimeMod.ptApi->TimestampToNanos(&tTimeMod, t, &nanos);
    TEST_ASSERT_EQUAL(JUNO_STATUS_INVALID_DATA_ERROR, status);
    TEST_ASSERT_EQUAL_UINT64(123, nanos); // unchanged on error
}

// Positive test: TimestampToMicros integer seconds
static void test_TimestampToMicros_success_integer(void)
{
    JUNO_TIMESTAMP_T t = { .iSeconds = 2, .iSubSeconds = 0 };
    uint64_t micros = 0;
    JUNO_STATUS_T status = tTimeMod.ptApi->TimestampToMicros(&tTimeMod, t, &micros);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT64(2000000ULL, micros);
}

// Positive test: TimestampToMicros fractional subseconds
static void test_TimestampToMicros_success_fractional(void)
{
    const uint64_t max_sub = UINT64_MAX;
    JUNO_TIMESTAMP_T t = { .iSeconds = 0, .iSubSeconds = max_sub };
    uint64_t micros = 0;
    JUNO_STATUS_T status = tTimeMod.ptApi->TimestampToMicros(&tTimeMod, t, &micros);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT64(1000000ULL, micros);
}

// Negative test: TimestampToMicros overflow detection
static void test_TimestampToMicros_overflow(void)
{
    JUNO_TIMESTAMP_T t = { .iSeconds = UINT64_MAX, .iSubSeconds = 0 };
    uint64_t micros = 999;
    JUNO_STATUS_T status = tTimeMod.ptApi->TimestampToMicros(&tTimeMod, t, &micros);
    TEST_ASSERT_EQUAL(JUNO_STATUS_INVALID_DATA_ERROR, status);
    TEST_ASSERT_EQUAL_UINT64(999, micros);
}

// Positive test: TimestampToMillis integer seconds
static void test_TimestampToMillis_success_integer(void)
{
    JUNO_TIMESTAMP_T t = { .iSeconds = 5, .iSubSeconds = 0 };
    uint64_t millis = 0;
    JUNO_STATUS_T status = tTimeMod.ptApi->TimestampToMillis(&tTimeMod, t, &millis);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT64(5000ULL, millis);
}

// Positive test: TimestampToMillis fractional subseconds
static void test_TimestampToMillis_success_fractional(void)
{
    const uint64_t max_sub = UINT64_MAX;
    JUNO_TIMESTAMP_T t = { .iSeconds = 0, .iSubSeconds = max_sub };
    uint64_t millis = 0;
    JUNO_STATUS_T status = tTimeMod.ptApi->TimestampToMillis(&tTimeMod, t, &millis);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT64(1000ULL, millis);
}

// Negative test: TimestampToMillis overflow detection
static void test_TimestampToMillis_overflow(void)
{
    JUNO_TIMESTAMP_T t = { .iSeconds = UINT64_MAX, .iSubSeconds = 0 };
    uint64_t millis = 42;
    JUNO_STATUS_T status = tTimeMod.ptApi->TimestampToMillis(&tTimeMod, t, &millis);
    TEST_ASSERT_EQUAL(JUNO_STATUS_INVALID_DATA_ERROR, status);
    TEST_ASSERT_EQUAL_UINT64(42, millis);
}

// Positive test: NanosToTimestamp with zero input
static void test_NanosToTimestamp_zero(void)
{
    JUNO_TIMESTAMP_T ret = {0};
    uint64_t input = 0;
    JUNO_STATUS_T status = tTimeMod.ptApi->NanosToTimestamp(&tTimeMod, input, &ret);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT64(0, ret.iSeconds);
    TEST_ASSERT_EQUAL_UINT64(0, ret.iSubSeconds);
}

// Positive test: NanosToTimestamp integer and fractional parts
static void test_NanosToTimestamp_integer_and_fractional(void)
{
    const uint64_t NANO_PER_SEC = 1000000000ULL;
    const uint64_t max_sub = UINT64_MAX;
    const uint64_t subs_per_nano = max_sub / NANO_PER_SEC;
    uint64_t input = 1500000000ULL; // 1.5 seconds
    JUNO_TIMESTAMP_T ret = {0};
    JUNO_STATUS_T status = tTimeMod.ptApi->NanosToTimestamp(&tTimeMod, input, &ret);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT64(1, ret.iSeconds);
    TEST_ASSERT_EQUAL_UINT64((input % NANO_PER_SEC) * subs_per_nano, ret.iSubSeconds);
}

// Positive test: MicrosToTimestamp with zero input
static void test_MicrosToTimestamp_zero(void)
{
    JUNO_TIMESTAMP_T ret = {0};
    uint64_t input = 0;
    JUNO_STATUS_T status = tTimeMod.ptApi->MicrosToTimestamp(&tTimeMod, input, &ret);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT64(0, ret.iSeconds);
    TEST_ASSERT_EQUAL_UINT64(0, ret.iSubSeconds);
}

// Positive test: MicrosToTimestamp integer and fractional parts
static void test_MicrosToTimestamp_integer_and_fractional(void)
{
    const uint64_t MICRO_PER_SEC = 1000000ULL;
    const uint64_t max_sub = UINT64_MAX;
    const uint64_t subs_per_micro = max_sub / MICRO_PER_SEC;
    uint64_t input = 2500000ULL; // 2.5 seconds
    JUNO_TIMESTAMP_T ret = {0};
    JUNO_STATUS_T status = tTimeMod.ptApi->MicrosToTimestamp(&tTimeMod, input, &ret);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT64(2, ret.iSeconds);
    TEST_ASSERT_EQUAL_UINT64((input % MICRO_PER_SEC) * subs_per_micro, ret.iSubSeconds);
}

// Positive test: MillisToTimestamp with zero input
static void test_MillisToTimestamp_zero(void)
{
    JUNO_TIMESTAMP_T ret = {0};
    uint64_t input = 0;
    JUNO_STATUS_T status = tTimeMod.ptApi->MillisToTimestamp(&tTimeMod, input, &ret);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT64(0, ret.iSeconds);
    TEST_ASSERT_EQUAL_UINT64(0, ret.iSubSeconds);
}

// Positive test: MillisToTimestamp integer and fractional parts
static void test_MillisToTimestamp_integer_and_fractional(void)
{
    const uint64_t MILLI_PER_SEC = 1000ULL;
    const uint64_t max_sub = UINT64_MAX;
    const uint64_t subs_per_milli = max_sub / MILLI_PER_SEC;
    uint64_t input = 4500ULL; // 4.5 seconds
    JUNO_TIMESTAMP_T ret = {0};
    JUNO_STATUS_T status = tTimeMod.ptApi->MillisToTimestamp(&tTimeMod, input, &ret);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL_UINT64(4, ret.iSeconds);
    TEST_ASSERT_EQUAL_UINT64((input % MILLI_PER_SEC) * subs_per_milli, ret.iSubSeconds);
}

// Test Now function returns reasonable values
static void test_Now_returns_success(void)
{
    JUNO_TIMESTAMP_T ret = {0};
    JUNO_STATUS_T status = tTimeMod.ptApi->Now(&tTimeMod, &ret);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
    TEST_ASSERT_TRUE(ret.iSeconds > 0);      // Epoch time should be non-zero
}

// Test SleepTo stub returns success
static void test_SleepTo_returns_success(void)
{
    JUNO_TIMESTAMP_T when = { .iSeconds = 1, .iSubSeconds = 0 };
    JUNO_STATUS_T status = tTimeMod.ptApi->SleepTo(&tTimeMod, when);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
}

// Test Sleep stub returns success
static void test_Sleep_returns_success(void)
{
    JUNO_TIMESTAMP_T duration = { .iSeconds = 1, .iSubSeconds = 0 };
    JUNO_STATUS_T status = tTimeMod.ptApi->Sleep(&tTimeMod, duration);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, status);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_AddTime_no_subseconds_overflow);
    RUN_TEST(test_AddTime_with_subseconds_overflow);
    RUN_TEST(test_SubtractTime_no_borrow);
    RUN_TEST(test_SubtractTime_borrow_subseconds);
    RUN_TEST(test_SubtractTime_invalid_lower_seconds);
    RUN_TEST(test_SubtractTime_invalid_zero_seconds_insufficient_subseconds);
    RUN_TEST(test_TimestampToNanos_success_integer);
    RUN_TEST(test_TimestampToNanos_success_fractional);
    RUN_TEST(test_TimestampToNanos_overflow);
    RUN_TEST(test_TimestampToMicros_success_integer);
    RUN_TEST(test_TimestampToMicros_success_fractional);
    RUN_TEST(test_TimestampToMicros_overflow);
    RUN_TEST(test_TimestampToMillis_success_integer);
    RUN_TEST(test_TimestampToMillis_success_fractional);
    RUN_TEST(test_TimestampToMillis_overflow);
    RUN_TEST(test_NanosToTimestamp_zero);
    RUN_TEST(test_NanosToTimestamp_integer_and_fractional);
    RUN_TEST(test_MicrosToTimestamp_zero);
    RUN_TEST(test_MicrosToTimestamp_integer_and_fractional);
    RUN_TEST(test_MillisToTimestamp_zero);
    RUN_TEST(test_MillisToTimestamp_integer_and_fractional);
    RUN_TEST(test_Now_returns_success);
    RUN_TEST(test_SleepTo_returns_success);
    RUN_TEST(test_Sleep_returns_success);
    return UNITY_END();
}
