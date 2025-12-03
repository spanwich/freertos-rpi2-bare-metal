/* Linker script for Raspberry Pi 2B v1.2 (BCM2837) */

MEMORY
{
    /* RPi2 boots at 0x8000, leave space for bootloader and GPU firmware */
    RAM (rwx) : ORIGIN = 0x00008000, LENGTH = 128M - 32K
}

SECTIONS
{
    /* Entry point at 0x8000 */
    . = 0x00008000;

    /* Code section */
    .init : {
        KEEP(*(.init))
    } > RAM

    .text : {
        *(.text*)
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
    } > RAM

    /* ARM exception index */
    .ARM.extab : {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > RAM

    .ARM.exidx : {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > RAM

    /* Data section */
    .data : {
        *(.data*)
    } > RAM

    /* BSS section for uninitialized data */
    .bss : {
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(8);
    } > RAM

    /* End of BSS before heap - THIS is what startup code should clear to */
    __bss_end__ = .;

    /* FreeRTOS heap - 1MB for tasks and buffers (reduced for faster boot) */
    /* Separate from BSS so startup doesn't clear it */
    .heap (NOLOAD) : {
        . = ALIGN(8);
        __heap_start__ = .;
        . = . + 1M;
        __heap_end__ = .;
    } > RAM

    /* End marker */
    _end = .;

    /* Discard unwanted sections */
    /DISCARD/ : {
        *(.note*)
        *(.comment*)
        *(.ARM.attributes*)
        *(.eh_frame*)
    }
}
