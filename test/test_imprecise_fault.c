#include <unity.h>
void bad_addr_double_word_write(void) {
  volatile uint64_t *buf = (volatile uint64_t *)0x30000000;
  *buf = 0x1122334455667788;
}

void test_imprecise_fault( void ) {
    bad_addr_double_word_write();
}