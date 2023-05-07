#include <unity.h>

void test_pendsv( void )
{
    *((volatile unsigned int*)0xE000ED04) = (0x1 << 28);
}