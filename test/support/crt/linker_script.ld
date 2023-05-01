/*
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 * 
 * Copyright(c) 2023 Diego Asanza <f.asanza@gmail.com>
 */

/* 
 * Simple linker script to be used with QEMU netduino2 board
 */

MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 1M
  RAM (rwx) : ORIGIN =  0x20000000, LENGTH = 192K
}

HEAP_SIZE  = DEFINED(__heap_size__)  ? __heap_size__  : 0x2000;
STACK_SIZE = DEFINED(__stack_size__) ? __stack_size__ : 0x0400;

ENTRY(reset_handler)

SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector))

        . += 400;

        *(.text*)

        KEEP(*(.init))
        KEEP(*(.fini))

        /* .ctors */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)

        /* .dtors */
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)

        *(.rodata*)

        KEEP(*(.eh_frame*))
    } > FLASH

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH
    __exidx_end = .;

    __etext = .;

    PROVIDE(_sidata = __etext);
    .data : AT (__etext)
    {
        __data_start__ = .;
        PROVIDE(_sdata = __data_start__);
        *(vtable)
        *(.data*)

        . = ALIGN(4);
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(4);
        /* init data */
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);


        . = ALIGN(4);
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

        KEEP(*(.jcr*))
        . = ALIGN(4);
        /* All data end */
        __data_end__ = .;
        PROVIDE(_edata = __data_end__);

    } > RAM

    .bss :
    {
        . = ALIGN(4);
        __bss_start__ = .;
        PROVIDE(_sbss = __bss_start__);
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
        PROVIDE(_ebss = __bss_end__);
    } > RAM

    .heap (COPY):
    {
        __end__ = .;
        end = __end__;
        *(.heap*)
        . += HEAP_SIZE;
        __HeapLimit = .;
    } > RAM

    /* .stack_dummy section doesn't contains any symbols. It is only
     * used for linker to calculate size of stack sections, and assign
     * values to stack symbols later */
    .stack_dummy (COPY):
    {
        *(.stack*)
        . += STACK_SIZE;
    } > RAM

    /* Set stack top to end of RAM, and stack limit move down by
     * size of stack_dummy section */
    __StackTop = ORIGIN(RAM) + LENGTH(RAM);
    __StackLimit = __StackTop - SIZEOF(.stack_dummy);
    PROVIDE(__stack = __StackTop);
    PROVIDE(_estack = __stack);

    /* Check if data + heap + stack exceeds RAM limit */
    ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
}
