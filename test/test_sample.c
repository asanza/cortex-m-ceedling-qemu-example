#ifdef TEST

#include "unity.h"

#include "sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_success_message( void ) {
    TEST_ASSERT_EQUAL(sum(1, 2), 3);
}

void test_ignore_message( void ) {
    TEST_IGNORE_MESSAGE("This test is ignored!");
}

void test_sample_FailOnHardFault(void)
{
    /*
     * Check that HardFault is handled correctly on qemu-crt
     * THIS TEST ALWAYS FAILS!.
     */
    TEST_IGNORE_MESSAGE("Comment out this line to check failure hard faults");
    int (*bad_instruction)(void) = (void *)0xE0000000;
    bad_instruction();
}

#endif // TEST
