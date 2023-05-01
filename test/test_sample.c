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
    TEST_PASS_MESSAGE("This test was successful!");
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
    int (*bad_instruction)(void) = (void *)0xE0000000;
    return bad_instruction();
}

#endif // TEST
