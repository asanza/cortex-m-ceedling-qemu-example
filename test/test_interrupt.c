#include <unity.h>

void test_pendsv( void )
{
    TEST_IGNORE_MESSAGE("Comment out this line to check pendsv interrupt response");
    *((volatile unsigned int*)0xE000ED04) = (0x1 << 28);
}