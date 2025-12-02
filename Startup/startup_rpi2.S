.section .init
.global _start

_start:
    @ Check CPU ID - only CPU0 continues, others park
    mrc p15, 0, r0, c0, c0, 5    @ Read MPIDR
    ands r0, r0, #3              @ Extract CPU ID (bits 0-1)
    bne cpu_park                 @ If not CPU0, park

    @ Disable interrupts
    cpsid if

    @ Set up vector table
    ldr r0, =vector_table
    mcr p15, 0, r0, c12, c0, 0   @ Write VBAR

    @ Initialize FPU (Cortex-A53 has VFPv4)
    mrc p15, 0, r0, c1, c0, 2    @ Read CPACR
    orr r0, r0, #0xF00000        @ Enable CP10 and CP11
    mcr p15, 0, r0, c1, c0, 2    @ Write CPACR
    isb
    mov r0, #0x40000000          @ Enable FPU
    vmsr fpexc, r0

    @ Set up stack pointer for IRQ mode
    cps #0x12                    @ Switch to IRQ mode
    ldr sp, =irq_stack_top

    @ Set up stack pointer for SVC mode (supervisor)
    cps #0x13                    @ Switch to SVC mode
    ldr sp, =stack_top

    @ Initialize BSS section
    ldr r0, =__bss_start__
    ldr r1, =__bss_end__
    mov r2, #0
bss_clear_loop:
    cmp r0, r1
    strlt r2, [r0], #4
    blt bss_clear_loop

    @ Jump to main
    bl main

    @ If main returns, hang
hang:
    wfi
    b hang

@ Park secondary CPUs
cpu_park:
    wfi
    b cpu_park

@ Exception vector table (ARM32 format)
.align 5
vector_table:
    ldr pc, reset_handler_addr
    ldr pc, undefined_handler_addr
    ldr pc, svc_handler_addr
    ldr pc, prefetch_handler_addr
    ldr pc, data_handler_addr
    ldr pc, unused_handler_addr
    ldr pc, irq_handler_addr
    ldr pc, fiq_handler_addr

reset_handler_addr:     .word _start
undefined_handler_addr: .word undefined_handler
svc_handler_addr:       .word FreeRTOS_SWI_Handler
prefetch_handler_addr:  .word prefetch_handler
data_handler_addr:      .word data_handler
unused_handler_addr:    .word hang
irq_handler_addr:       .word FreeRTOS_IRQ_Handler
fiq_handler_addr:       .word fiq_handler

@ Default exception handlers
undefined_handler:
    b hang

prefetch_handler:
    b hang

data_handler:
    b hang

fiq_handler:
    b hang

@ Stack sections
.section .bss
.align 3
stack_base:
    .space 16384        @ 16KB main stack
stack_top:

irq_stack_base:
    .space 8192         @ 8KB IRQ stack
irq_stack_top:
