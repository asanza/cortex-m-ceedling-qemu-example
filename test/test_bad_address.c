#include <unity.h>

static uint32_t read_from_bad_address(void) {
  return *(volatile uint32_t *)0xbadcafe;
}

void test_bad_address_read( void ) {
    TEST_ASSERT_EQUAL(0, read_from_bad_address());
}