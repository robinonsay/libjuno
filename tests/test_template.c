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

#include "unity.h"
#include "unity_internals.h"
#include <stdbool.h>


void setUp(void)
{

}
void tearDown(void)
{

}

static void test_template(void)
{
    TEST_ASSERT_TRUE(true);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_template);
	return UNITY_END();
}
