/*
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 * 
 * Copyright(c) 2023 Diego Asanza <f.asanza@gmail.com>
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "backtrace.h"

#define WEAK __attribute__((weak))
#define ALIAS(f) __attribute__((weak, alias(#f)))

#define CPACR 0xE000ED88
#define ICSR 0xE000ED04
#define VTOR 0xE000ED08

#define REGADDR(x) ((unsigned int *)x)

extern void __StackTop(void);

extern unsigned int __bss_start__;
extern unsigned int __bss_end__;
extern unsigned int __data_start__;
extern unsigned int __data_end__;
extern unsigned int __etext;

void
fault_handler_c(unsigned int *stack);
extern void initialise_monitor_handles(void);

void Default_Handler(void)
{
	/*
     * If we are here, chances are that we triggered an unhandled exception
     * handler. Read the active interrupt number bellow.
     */
	volatile __unused uint32_t vector = *REGADDR(ICSR) & 0xFFU;
	printf("Default handler called: Vector %d\n", vector);
	exit(-1);
}

__attribute__((naked)) void HardFault_Handler(void)
{
#if defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_6M__)
	asm volatile(
        "movs   r0, #4            \n"
        "mov    r1, lr            \n"
        "tst    r0, r1            \n"
        "bne    is_psp            \n"
        "mrs    r0, msp           \n"
        "b      fault_handler     \n"
        "is_psp:                  \n"
        "mrs r0, psp              \n"
        "fault_handler:           \n"
        "ldr r2,=fault_handler_c  \n"
        "bx r2                    \n"
    );
#elif defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_8M__)
	asm volatile(
        "tst lr, #4 \n"
		     "ite eq \n"
		     "mrseq r0, msp \n"
		     "mrsne r0, psp \n"
		     "b fault_handler_c \n"
		     "nop\n"
    );
#else
#error "Unsupported Architecture"
#endif
}

void NMI_Handler(void) ALIAS(Default_Handler);
void MemManage_Handler(void) ALIAS(Default_Handler);
void BusFault_Handler(void) ALIAS(Default_Handler);
void UsageFault_Handler(void) ALIAS(Default_Handler);
void SecureFault_Handler(void) ALIAS(Default_Handler);
void SVC_Handler(void) ALIAS(Default_Handler);
void DebugMon_Handler(void) ALIAS(Default_Handler);
void PendSV_Handler(void) ALIAS(Default_Handler);
void SysTick_Handler(void) ALIAS(Default_Handler);

void reset_handler(void);
void main(void);

__attribute__((used, section(".isr_vector"))) void (
		*const g_interrupt_vector[])(void) = {
	&__StackTop,
	reset_handler,
	NMI_Handler,
	HardFault_Handler,
	MemManage_Handler,
	BusFault_Handler,
	UsageFault_Handler,
	SecureFault_Handler,
	0,
	0,
	0,
	SVC_Handler,
	DebugMon_Handler,
	0,
	PendSV_Handler,
	SysTick_Handler,
};

void reset_handler(void)
{
	asm volatile("cpsid i");

	/* Remap the interrupt vector */
	*REGADDR(VTOR) = (unsigned int)g_interrupt_vector;

	/* enable the FPU */
	*REGADDR(CPACR) = (0x0FU) << 20;

	/* now we initialize data in ram from values saved in flash */
	volatile unsigned int *it = &__data_start__;
	volatile unsigned int *dr = &__etext;

	while (it < &__data_end__) {
		*it++ = *dr++;
	}

	/* clear the bss section */
	it = &__bss_start__;
	while (it < &__bss_end__) {
		*it++ = 0;
	}
    initialise_monitor_handles();
	main();
	exit(0);
}

void 
decode_bfsr_ufsr(uint32_t bfsr, uint32_t ufsr)
{
    volatile uint8_t *bfar  = (volatile uint8_t *)0xE000ED38;

    if (bfsr & (1 << 7)) {
        printf("BFAR : 0x%08X\n", *bfar);
    }
    printf("UFSR DIVBYZERO    : %d\n", (ufsr & (1 << 9)) != 0);
    printf("UFSR UNALIGNED    : %d\n", (ufsr & (1 << 8)) != 0);
    printf("UFSR NOCP         : %d\n", (ufsr & (1 << 3)) != 0);
    printf("UFSR INVPC        : %d\n", (ufsr & (1 << 2)) != 0);
    printf("UFSR INVSTATE     : %d\n", (ufsr & (1 << 1)) != 0);
    printf("UFSR UNDEFINSTR   : %d\n", (ufsr & (1 << 0)) != 0);
    printf("BFSR BFARVALID    : %d\n", (bfsr & (1 << 7)) != 0);
    printf("BFSR LSPERR       : %d\n", (bfsr & (1 << 5)) != 0);
    printf("BFSR STKERR       : %d\n", (bfsr & (1 << 4)) != 0);
    printf("BFSR UNSTKERR     : %d\n", (bfsr & (1 << 3)) != 0);
    printf("BFSR IMPRECISERR  : %d\n", (bfsr & (1 << 2)) != 0);
    printf("BFSR PRECISERR    : %d\n", (bfsr & (1 << 1)) != 0);
    printf("BFSR IBUSERR      : %d\n", (bfsr & (1 << 0)) != 0);
}

void
fault_handler_c(unsigned int *stack)
{
    char buf[100];
    volatile uint32_t *cfsr  = (volatile uint32_t *)0xE000ED28;
    volatile uint16_t *ufsr  = (volatile uint16_t *)0xE000ED2A;
    volatile uint8_t *bfsr  = (volatile uint8_t *)0xE000ED29;
    volatile uint8_t *mmfsr = (volatile uint8_t *)0xE000ED28;

    printf("\n======== PANIC! Fault Handler Called ===========\n\n");
    printf("PC: 0x%08X - %s\n", stack[6], backtrace_function_name(stack[6]));
    printf("LR: 0x%08X - %s\n", stack[5], backtrace_function_name(stack[5]));
    printf("\nRegisters: \n");
    printf("R0   : 0x%08X\n", stack[0]);
    printf("R1   : 0x%08X\n", stack[1]);
    printf("R2   : 0x%08X\n", stack[2]);
    printf("R3   : 0x%08X\n", stack[3]);
    printf("R12  : 0x%08X\n", stack[4]);
    printf("XPSR : 0x%08X\n", stack[7]);
    printf("CFSR : 0x%08X\n", *cfsr);
    printf("\n");
    decode_bfsr_ufsr(*bfsr, *ufsr);
    printf("================================================\n");
    exit(-1);
}
