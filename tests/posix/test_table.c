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

static void test_init_invalid_params(void)
{
    JUNO_STATUS_T tStatus;
    JUNO_TABLE_MANAGER_T tMngr = {};
    JUNO_TEST_TABLE_T tTable = {};
    // Null manager
    tStatus = gptTableApi->Init(NULL, &tTable.tHdr, sizeof(tTable), pcTableName, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    // Null buffer
    tStatus = gptTableApi->Init(&tMngr, NULL, sizeof(tTable), pcTableName, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    // Zero size
    tStatus = gptTableApi->Init(&tMngr, &tTable.tHdr, 0, pcTableName, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    // Null path
    tStatus = gptTableApi->Init(&tMngr, &tTable.tHdr, sizeof(tTable), NULL, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_load_save_setbuffer_without_init(void)
{
    JUNO_STATUS_T tStatus;
    JUNO_TABLE_MANAGER_T tMngr = {};
    JUNO_TEST_TABLE_T tTable = {}, tTable2 = {};
    // Load without init
    tStatus = gptTableApi->Load(&tMngr);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    // Save without init
    tStatus = gptTableApi->Save(&tMngr);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    // SetBuffer without init
    tStatus = gptTableApi->SetBuffer(&tMngr, &tTable.tHdr);
    TEST_ASSERT_NOT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
}

static void test_overwrite_data(void)
{
    JUNO_STATUS_T tStatus;
    JUNO_TABLE_MANAGER_T tMngr = {};
    JUNO_TEST_TABLE_T tTable = {}, tTable2 = {};
    // Initialize manager
    tStatus = gptTableApi->Init(&tMngr, &tTable.tHdr, sizeof(tTable), pcTableName, NULL, NULL);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    // First write
    tTable.iTestData = 111;
    tStatus = gptTableApi->Save(&tMngr);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    // Load into tTable2
    tTable2.iTestData = 0;
    tStatus = gptTableApi->SetBuffer(&tMngr, &tTable2.tHdr);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    tStatus = gptTableApi->Load(&tMngr);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(111, tTable2.iTestData);
    tStatus = gptTableApi->SetBuffer(&tMngr, &tTable.tHdr);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    // Overwrite with new value
    tTable.iTestData = 222;
    tStatus = gptTableApi->Save(&tMngr);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    // Reload into tTable2
    tTable2.iTestData = 0;
    tStatus = gptTableApi->SetBuffer(&tMngr, &tTable2.tHdr);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    tStatus = gptTableApi->Load(&tMngr);
    TEST_ASSERT_EQUAL(JUNO_STATUS_SUCCESS, tStatus);
    TEST_ASSERT_EQUAL(222, tTable2.iTestData);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_nominal_table);
    RUN_TEST(test_init_invalid_params);
    RUN_TEST(test_load_save_setbuffer_without_init);
    RUN_TEST(test_overwrite_data);
    return UNITY_END();
}
