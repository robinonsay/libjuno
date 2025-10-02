// Updated to latest API: use JUNO_ARRAY_ROOT_T as backing storage and JUNO_POINTER_API
#include "juno/memory/memory_api.h"
#include "juno/module.h"
#include "juno/status.h"
#include "unity.h"
#include "unity_internals.h"
#include "juno/ds/array_api.h"
#include "juno/ds/buff_stack_api.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

// Backing array for the stack (10 bytes)
typedef struct TEST_ARRAY JUNO_MODULE_DERIVE(JUNO_ARRAY_ROOT_T,
	uint8_t iTestStack[10];
) TEST_ARRAY;

// Array API implementations over TEST_ARRAY
static inline JUNO_STATUS_T Stack_SetAt(JUNO_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex);
static inline JUNO_RESULT_POINTER_T Stack_GetAt(JUNO_ARRAY_ROOT_T *ptArray, size_t iIndex);
static inline JUNO_STATUS_T Stack_RemoveAt(JUNO_ARRAY_ROOT_T *ptArray, size_t iIndex);
// Pointer API implementations for uint8_t
static inline JUNO_STATUS_T Stack_Copy(JUNO_POINTER_T tDest, JUNO_POINTER_T tSrc);
static inline JUNO_STATUS_T Stack_Reset(JUNO_POINTER_T tDest);

const JUNO_ARRAY_API_T gtArrayApi = {
	Stack_SetAt,
	Stack_GetAt,
	Stack_RemoveAt,
};

const JUNO_POINTER_API_T gtPointerApi = {
	.Copy = Stack_Copy,
	.Reset = Stack_Reset,
};

union JUNO_ARRAY_ROOT_T JUNO_MODULE(void, JUNO_ARRAY_ROOT_T,
	TEST_ARRAY tTestArray;
);

static void test_stack(void)
{
	// Prepare the backing array (buffer)
	TEST_ARRAY tBuffer = (TEST_ARRAY){0};
	JUNO_ARRAY_ROOT_T *ptArray = (JUNO_ARRAY_ROOT_T *)&tBuffer;

	// Initialize the array root fields
	ptArray->ptApi        = &gtArrayApi;
	ptArray->ptPointerApi = &gtPointerApi;
	ptArray->zCapacity    = sizeof(tBuffer.iTestStack);
	ptArray->zLength      = 0;

	// Initialize the stack with the backing array
	JUNO_BUFF_STACK_ROOT_T tStack = (JUNO_BUFF_STACK_ROOT_T){0};
	JUNO_STATUS_T tStatus = JunoDs_Buff_StackInit(&tStack, ptArray, NULL, NULL);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);

	uint8_t iValue = 0;
	JUNO_POINTER_T tValPtr = Juno_PointerInit(&gtPointerApi, uint8_t, &iValue);

	// Fill the stack
	for (size_t i = 0; i < sizeof(tBuffer.iTestStack); i++)
	{
		iValue = (uint8_t)(i + 1);
		tStatus = tStack.ptApi->Push(&tStack, tValPtr);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	// One too many
	tStatus = tStack.ptApi->Push(&tStack, tValPtr);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);

	// Drain the stack (LIFO)
	for (size_t i = 0; i < sizeof(tBuffer.iTestStack); i++)
	{
		iValue = 0;
		tStatus = tStack.ptApi->Pop(&tStack, tValPtr);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_EQUAL(sizeof(tBuffer.iTestStack) - i, iValue);
	}
	// One too many
	tStatus = tStack.ptApi->Pop(&tStack, tValPtr);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);

	// Fill and drain again
	for (size_t i = 0; i < sizeof(tBuffer.iTestStack); i++)
	{
		iValue = (uint8_t)(i + 1);
		tStatus = tStack.ptApi->Push(&tStack, tValPtr);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	}
	tStatus = tStack.ptApi->Push(&tStack, tValPtr);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	for (size_t i = 0; i < sizeof(tBuffer.iTestStack); i++)
	{
		iValue = 0;
		tStatus = tStack.ptApi->Pop(&tStack, tValPtr);
		TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
		TEST_ASSERT_EQUAL(sizeof(tBuffer.iTestStack) - i, iValue);
	}
	tStatus = tStack.ptApi->Pop(&tStack, tValPtr);
	TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_stack);
	return UNITY_END();
}

static inline JUNO_STATUS_T Stack_SetAt(JUNO_ARRAY_ROOT_T *ptArray, JUNO_POINTER_T tItem, size_t iIndex)
{
	uint8_t *iItem = (uint8_t *)tItem.pvAddr;
	TEST_ARRAY *ptTestArray = (TEST_ARRAY *)ptArray;
	ptTestArray->iTestStack[iIndex] = *iItem;
	return JUNO_STATUS_SUCCESS;
}

static inline JUNO_RESULT_POINTER_T Stack_GetAt(JUNO_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
	TEST_ARRAY *ptTestArray = (TEST_ARRAY *)ptArray;
	JUNO_RESULT_POINTER_T tResult = JUNO_OK_RESULT(Juno_PointerInit(&gtPointerApi, uint8_t, &ptTestArray->iTestStack[iIndex]));
	return tResult;
}

static inline JUNO_STATUS_T Stack_RemoveAt(JUNO_ARRAY_ROOT_T *ptArray, size_t iIndex)
{
	TEST_ARRAY *ptTestArray = (TEST_ARRAY *)ptArray;
	ptTestArray->iTestStack[iIndex] = 0;
	return JUNO_STATUS_SUCCESS;
}

static inline JUNO_STATUS_T Stack_Copy(JUNO_POINTER_T tDest, JUNO_POINTER_T tSrc)
{
	uint8_t *iDest = (uint8_t *)tDest.pvAddr;
	uint8_t *iSrc  = (uint8_t *)tSrc.pvAddr;
	*iDest = *iSrc;
	return JUNO_STATUS_SUCCESS;
}

static inline JUNO_STATUS_T Stack_Reset(JUNO_POINTER_T tDest)
{
	uint8_t *iDest = (uint8_t *)tDest.pvAddr;
	*iDest = 0;
	return JUNO_STATUS_SUCCESS;
}

