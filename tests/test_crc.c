#include "unity.h"
#include "unity_internals.h"
#include <stdbool.h>
#include "juno/crc/crc.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void setUp(void)
{

}

void tearDown(void)
{

}

static void test_zip_crc(void)
{
   char pcTestZipData[32] = "helloworld";
   size_t zTestDataLen = strlen(pcTestZipData);
   char pcTruthZipData[32] = {};
   FILE *ptTruthFile = fopen("test_zip_crc.bin", "r");
   TEST_ASSERT_NOT_NULL(ptTruthFile);
   ssize_t zFileSize = fread(pcTruthZipData, zTestDataLen + 4, 1, ptTruthFile);
   fclose(ptTruthFile);
   TEST_ASSERT_GREATER_THAN_size_t(0, zFileSize);
   size_t zCrc = Juno_ZipUpdateCrc(JUNO_ZIP_CRC_INIT, (uint8_t *)pcTestZipData, zTestDataLen);
   memcpy(&pcTestZipData[zTestDataLen], &zCrc, sizeof(zCrc));
   for(size_t i = 0; i < sizeof(pcTestZipData); i++)
   {
	   TEST_ASSERT_EQUAL(pcTruthZipData[i], pcTestZipData[i]);
   }
}
static void test_ccitt_crc(void)
{
   char pcTestZipData[32] = "helloworld";
   size_t zTestDataLen = strlen(pcTestZipData);
   char pcTruthZipData[32] = "helloworld\x8a\xab";
   size_t zCrc = Juno_CcittUpdateCrc(JUNO_CCITT_CRC_INIT, (uint8_t *)pcTestZipData, zTestDataLen);
   memcpy(&pcTestZipData[zTestDataLen], &zCrc, sizeof(zCrc));
   for(size_t i = 0; i < sizeof(pcTestZipData); i++)
   {
	   TEST_ASSERT_EQUAL(pcTruthZipData[i], pcTestZipData[i]);
   }
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_zip_crc);
	RUN_TEST(test_ccitt_crc);
	return UNITY_END();
}
