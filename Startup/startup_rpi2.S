.section .init
.global _start

_start:
    @ Small delay to let UART stabilize after GPU handoff
    ldr r2, =0x100000
delay_loop:
    subs r2, r2, #1
    bne delay_loop

    @ EARLY DEBUG: Write 'S' to UART immediately to prove we got here
    ldr r0, =0x3F201000          @ UART0 base
    mov r1, #0x53                @ ASCII 'S' for Start
    str r1, [r0]                 @ Write to UART data register

    @ Check CPU ID - only CPU0 continues, others park
    mrc p15, 0, r0, c0, c0, 5    @ Read MPIDR
    ands r0, r0, #3              @ Extract CPU ID (bits 0-1)
    bne cpu_park                 @ If not CPU0, park

    @ DEBUG: Write 'C' for CPU0
    ldr r0, =0x3F201000
    mov r1, #0x43                @ ASCII 'C'
    str r1, [r0]

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

    @ DEBUG: Write 'B' before BSS clear
    ldr r0, =0x3F201000
    mov r1, #0x42                @ ASCII 'B'
    str r1, [r0]

    @ Initialize BSS section
    ldr r0, =__bss_start__
    ldr r1, =__bss_end__
    mov r2, #0
bss_clear_loop:
    cmp r0, r1
    strlt r2, [r0], #4
    blt bss_clear_loop

    @ DEBUG: Write 'M' before main
    ldr r0, =0x3F201000
    mov r1, #0x4D                @ ASCII 'M'
    str r1, [r0]

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
