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
   char pcTruthZipData[32] = {0};
   FILE *ptTruthFile = fopen("test_zip_crc.bin", "r");
   TEST_ASSERT_NOT_NULL(ptTruthFile);
   ssize_t zFileSize = fread(pcTruthZipData, zTestDataLen + 4, 1, ptTruthFile);
   fclose(ptTruthFile);
   TEST_ASSERT_GREATER_THAN_size_t(0, zFileSize);
   size_t zCrc = Juno_CrcZipUpdate(JUNO_ZIP_CRC_INIT, (uint8_t *)pcTestZipData, zTestDataLen);
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
   size_t zCrc = Juno_CrcCcittUpdate(JUNO_CCITT_CRC_INIT, (uint8_t *)pcTestZipData, zTestDataLen);
   memcpy(&pcTestZipData[zTestDataLen], &zCrc, sizeof(zCrc));
   for(size_t i = 0; i < sizeof(pcTestZipData); i++)
   {
	   TEST_ASSERT_EQUAL(pcTruthZipData[i], pcTestZipData[i]);
   }
}

static void test_empty_crc_all(void)
{
    // Empty data should return initial CRC values
    TEST_ASSERT_EQUAL_UINT16(JUNO_ARC_CRC_INIT, Juno_CrcArcUpdate(JUNO_ARC_CRC_INIT, NULL, 0));
    TEST_ASSERT_EQUAL_UINT16(JUNO_BINHEX_CRC_INIT, Juno_CrcBinhexUpdate(JUNO_BINHEX_CRC_INIT, NULL, 0));
    TEST_ASSERT_EQUAL_UINT16(JUNO_CCITT_CRC_INIT, Juno_CrcCcittUpdate(JUNO_CCITT_CRC_INIT, NULL, 0));
    TEST_ASSERT_EQUAL_UINT32(JUNO_CCITT32_CRC_INIT, Juno_CrcCcitt32Update(JUNO_CCITT32_CRC_INIT, NULL, 0));
    TEST_ASSERT_EQUAL_UINT32(0u, Juno_CrcZipUpdate(JUNO_ZIP_CRC_INIT, NULL, 0));
}

static void test_various_crc_behaviour(void)
{
    const char data1[] = "test";
    const char data2[] = "Test";
    size_t len1 = strlen(data1);
    size_t len2 = strlen(data2);
    // ARC
    uint16_t crc_arc1 = Juno_CrcArcUpdate(JUNO_ARC_CRC_INIT, data1, len1);
    uint16_t crc_arc2 = Juno_CrcArcUpdate(JUNO_ARC_CRC_INIT, data1, len1);
    TEST_ASSERT_EQUAL_UINT16(crc_arc1, crc_arc2);
    TEST_ASSERT_NOT_EQUAL(crc_arc1, Juno_CrcArcUpdate(JUNO_ARC_CRC_INIT, data2, len2));
    // BINHEX
    uint16_t crc_bx1 = Juno_CrcBinhexUpdate(JUNO_BINHEX_CRC_INIT, data1, len1);
    TEST_ASSERT_EQUAL_UINT16(crc_bx1, Juno_CrcBinhexUpdate(JUNO_BINHEX_CRC_INIT, data1, len1));
    TEST_ASSERT_NOT_EQUAL(crc_bx1, Juno_CrcBinhexUpdate(JUNO_BINHEX_CRC_INIT, data2, len2));
    // CCITT32
    uint32_t crc32_1 = Juno_CrcCcitt32Update(JUNO_CCITT32_CRC_INIT, data1, len1);
    TEST_ASSERT_EQUAL_UINT32(crc32_1, Juno_CrcCcitt32Update(JUNO_CCITT32_CRC_INIT, data1, len1));
    TEST_ASSERT_NOT_EQUAL(crc32_1, Juno_CrcCcitt32Update(JUNO_CCITT32_CRC_INIT, data2, len2));
    // KERMIT
    uint32_t crc_k1 = Juno_CrcKermitUpdate(JUNO_KERMIT_CRC_INIT, data1, len1);
    TEST_ASSERT_EQUAL_UINT32(crc_k1, Juno_CrcKermitUpdate(JUNO_KERMIT_CRC_INIT, data1, len1));
    TEST_ASSERT_NOT_EQUAL(crc_k1, Juno_CrcKermitUpdate(JUNO_KERMIT_CRC_INIT, data2, len2));
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_zip_crc);
	RUN_TEST(test_ccitt_crc);
	RUN_TEST(test_empty_crc_all);
	RUN_TEST(test_various_crc_behaviour);
	return UNITY_END();
}
