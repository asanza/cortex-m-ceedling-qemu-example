/*
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 * 
 * Copyright(c) 2023 Diego Asanza <f.asanza@gmail.com>
 */

#include <stdint.h>
#include "semihost.h"
#include "printf.h"
#include "unity.h"

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

void Default_Handler(void)
{
	/*
     * If we are here, chances are that we triggered an unhandled exception
     * handler. Read the active interrupt number bellow.
     */
	volatile __unused uint32_t vector = *REGADDR(ICSR) & 0xFFU;
	printf("Default handler called: Vector %d\n", vector);
	sh_error();
}

void HardFault_Handler(void)
{
    #if defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_6M__)
    asm volatile("movs   R0, #4\n"
            "mov    R1, LR\n"
            "tst    R0, R1\n"
            "bne    PSP_ACTIVE\n"
            "mrs    R0, MSP\n"
            "b      CALL_HANDLER\n"
            "PSP_ACTIVE:\n"
            "mrs R0, PSP\n"
            "CALL_HANDLER:\n"
            "ldr R2,=fault_handler_c\n"
            "bx R2\n");
    #elif defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_8M__)
	asm volatile("tst lr, #4 \n"
		     "ite eq \n"
		     "mrseq r0, msp \n"
		     "mrsne r0, psp \n"
		     "b fault_handler_c \n"
             "nop\n");
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

	main();
	sh_end();
}
