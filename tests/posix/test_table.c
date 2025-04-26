#include "juno/status.h"
#include "juno/table/table_api.h"
#include "juno/table/table_types.h"
#include "unity.h"
#include "unity_internals.h"
#include <stdbool.h>
#include <stdio.h>
#include "juno/table/table_posix.h"

static const JUNO_TABLE_API_T *gptTableApi = NULL;

typedef struct
{
	JUNO_TABLE_HDR_T tHdr;
	int iTestData;
} JUNO_TEST_TABLE_T;

static const char *pcTableName = "test_table.tbl";

void setUp(void)
{
	gptTableApi = Juno_TablePosixApi();
}
void tearDown(void)
{
	remove(pcTableName);
}

static void test_nominal_table(void)
{
	JUNO_TABLE_MANAGER_T tMngr = {};
	JUNO_TEST_TABLE_T tTable = {};
	JUNO_STATUS_T tStatus = gptTableApi->Init(
		&tMngr,
		&tTable.tHdr,
		sizeof(tTable),
		pcTableName,
		NULL,
		NULL
	);
	tStatus = gptTableApi->Load(&tMngr);
	TEST_ASSERT_EQUAL(JUNO_STATUS_DNE_ERROR, tStatus);
	tTable.iTestData = 1234;
	tStatus = gptTableApi->Save(&tMngr);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	tStatus = gptTableApi->Load(&tMngr);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	JUNO_TEST_TABLE_T tTable2 = {};
	tStatus = gptTableApi->SetBuffer(&tMngr, &tTable2.tHdr);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	tStatus = gptTableApi->Load(&tMngr);
	TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
	TEST_ASSERT_EQUAL(tTable.iTestData, tTable2.iTestData);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_nominal_table);
	return UNITY_END();
}
