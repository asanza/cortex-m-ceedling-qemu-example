/*
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 * 
 * Copyright(c) 2023 Diego Asanza <f.asanza@gmail.com>
 */

#include "unity.h"
#include "printf.h"
#include <stdint.h>

/* ARM semihosting allows an ARM device connected to a development machine
 * to make service requests.
 * See:
 * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0471k/pge1358787045051.html
 */
static inline int
__semihost(int nr_syscall, const void *arg);

/* See
 * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0471k/pge1358787045051.html
 */
enum {
    SYS_WRITEC = 0x03,
    SYS_WRITE0 = 0x04,
    SYS_EXIT   = 0x18,
};

/* See
 * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0471k/pge1358787050566.html
 */
enum {
    ADP_Stopped_ApplicationExit = 0x20026,
    ADP_Stopped_InternalError = 0x20024,
};

static inline int
__semihost(int nr_syscall, const void *arg)
{
    register int         n __asm__("r0") = nr_syscall;
    register const void *a __asm__("r1") = arg;
    __asm__ volatile("bkpt 0xab"
                     : "+r"(n) /* Outputs */
                     : "r"(a)  /* Inputs */
                     : "r2", "r3", "ip", "lr", "memory", "cc"
                     /* Clobbers r0 and r1, and lr if in supervisor mode */);
    /* We're being conservative with clobbered registers here and
     * doing the same thing as newlib, which has the following comment:
     *
     * Accordingly to page 13-77 of ARM DUI 0040D other registers
     * can also be clobbered.  Some memory positions may also be
     * changed by a system call, so they should not be kept in registers. */
    return n;
}

// define the size of the communications buffer (adjust to suit)

int
sh_putstr(char *str)
{
    return __semihost(SYS_WRITE0, str);
}

int
sh_putchar(char c)
{
    return __semihost(SYS_WRITEC, &c);
}

void
_putchar(char character)
{
    sh_putchar(character);
}

int
sh_flush(void)
{
    return 0;
}

int
sh_end(void)
{
    /* flush the buffer */
    sh_flush();
    __semihost(SYS_EXIT, (void *)ADP_Stopped_ApplicationExit);
    return 0;
}

int
sh_error( void )
{
    sh_flush();
    __semihost(SYS_EXIT, (void *)ADP_Stopped_InternalError);
    return -1;
}

void
fault_handler_c(unsigned int *stack)
{
    char buf[100];
    volatile uint32_t *cfsr  = (volatile uint32_t *)0xE000ED28;
    volatile uint16_t *ufsr  = (volatile uint16_t *)0xE000ED2A;
    volatile uint8_t *bfsr  = (volatile uint8_t *)0xE000ED29;
    volatile uint8_t *mmfsr = (volatile uint8_t *)0xE000ED28;
    volatile uint8_t *bfar  = (volatile uint8_t *)0xE000ED38;

    printf("================================================\n\n");
    printf("PANIC!!! Fault Handler Called...\n");
    printf("\n");
    printf("Stacktrace: \n");
    printf("R0   : 0x%08X\n", stack[0]);
    printf("R1   : 0x%08X\n", stack[1]);
    printf("R2   : 0x%08X\n", stack[2]);
    printf("R3   : 0x%08X\n", stack[3]);
    printf("R12  : 0x%08X\n", stack[4]);
    printf("LR   : 0x%08X\n", stack[5]);
    printf("PC   : 0x%08X\n", stack[6]);
    printf("XPSR : 0x%08X\n", stack[7]);
    printf("CFSR : 0x%08X\n", *cfsr);
    if (*bfsr & (1 << 7)) {
        printf("BFAR : 0x%08X\n", *bfar);
    }
    printf("\n");
    printf("UFSR DIVBYZERO    : %d\n", (*ufsr & (1 << 9)) != 0);
    printf("UFSR UNALIGNED    : %d\n", (*ufsr & (1 << 8)) != 0);
    printf("UFSR NOCP         : %d\n", (*ufsr & (1 << 3)) != 0);
    printf("UFSR INVPC        : %d\n", (*ufsr & (1 << 2)) != 0);
    printf("UFSR INVSTATE     : %d\n", (*ufsr & (1 << 1)) != 0);
    printf("UFSR UNDEFINSTR   : %d\n", (*ufsr & (1 << 0)) != 0);
    printf("BFSR BFARVALID    : %d\n", (*bfsr & (1 << 7)) != 0);
    printf("BFSR LSPERR       : %d\n", (*bfsr & (1 << 5)) != 0);
    printf("BFSR STKERR       : %d\n", (*bfsr & (1 << 4)) != 0);
    printf("BFSR UNSTKERR     : %d\n", (*bfsr & (1 << 3)) != 0);
    printf("BFSR IMPRECISERR  : %d\n", (*bfsr & (1 << 2)) != 0);
    printf("BFSR PRECISERR    : %d\n", (*bfsr & (1 << 1)) != 0);
    printf("BFSR IBUSERR      : %d\n", (*bfsr & (1 << 0)) != 0);
    printf("================================================\n");
    sprintf(buf, "Hard Fault Called at PC Address 0x%08x!", stack[6]);
    TEST_FAIL_MESSAGE(buf);
    sh_end();
}
