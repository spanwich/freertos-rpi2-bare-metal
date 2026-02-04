.section .init
.global _start

_start:
    @ Park secondary CPUs IMMEDIATELY (before ANY other code)
    mrc p15, 0, r0, c0, c0, 5    @ Read MPIDR
    ands r0, r0, #3              @ Extract CPU ID (bits 0-1)
    bne cpu_park                 @ If not CPU0, park

    @ Only CPU0 continues from here

    @ Small delay to let UART stabilize after GPU handoff
    ldr r2, =0x100000
delay_loop:
    subs r2, r2, #1
    bne delay_loop

    @ EARLY DEBUG: Write 'X' to UART immediately to prove we got here
    ldr r0, =0x3F201000          @ UART0 base
    mov r1, #0x58                @ ASCII 'X' for Start
    str r1, [r0]                 @ Write to UART data register

    @ DEBUG: Write 'Y' for CPU0
    ldr r0, =0x3F201000
    mov r1, #0x59                @ ASCII 'Y'
    str r1, [r0]

    @ Disable interrupts
    cpsid if

    @ DEBUG: After interrupt disable
    ldr r0, =0x3F201000
    mov r1, #0x49                @ ASCII 'I'
    str r1, [r0]

    @ Set up vector table
    ldr r0, =vector_table
    mcr p15, 0, r0, c12, c0, 0   @ Write VBAR

    @ DEBUG: After VBAR
    ldr r0, =0x3F201000
    mov r1, #0x56                @ ASCII 'V'
    str r1, [r0]

    @ Skip FPU initialization - not needed for memory painting
    @ Enabling FPU causes undefined instruction fault on this platform
    @ and adds unnecessary context-switching overhead

    @ DEBUG: Before mode check
    ldr r0, =0x3F201000
    mov r1, #0x48                @ ASCII 'H'
    str r1, [r0]

    @ Check if we're in HYP mode and drop to SVC if needed
    mrs r0, cpsr                 @ Read current mode
    and r0, r0, #0x1F            @ Extract mode bits
    cmp r0, #0x1A                @ Check if HYP mode (0x1A)
    bne not_hyp                  @ If not HYP, skip

    @ DEBUG: We ARE in HYP mode
    ldr r1, =0x3F201000
    mov r2, #0x59                @ ASCII 'Y' for Yes HYP
    str r2, [r1]

    @ We're in HYP mode - need to use eret to drop to SVC
    @ Set SPSR_hyp to SVC mode with IRQ/FIQ disabled
    mov r0, #0xD3                @ SVC mode (0x13) + IRQ/FIQ disabled (0xC0)
    msr spsr_hyp, r0             @ Set return mode to SVC

    @ Set return address to instruction after eret
    adr lr, hyp_exit             @ Load address of hyp_exit label
    msr elr_hyp, lr              @ Set exception return address
    eret                         @ Drop to SVC mode

hyp_exit:
    @ DEBUG: Successfully exited HYP mode
    ldr r0, =0x3F201000
    mov r1, #0x45                @ ASCII 'E' for Exit HYP
    str r1, [r0]

not_hyp:
    @ DEBUG: Not in HYP mode (now in SVC mode)
    ldr r0, =0x3F201000
    mov r1, #0x4E                @ ASCII 'N' for No HYP
    str r1, [r0]

    @ NOW we can enable FPU (we're in SVC mode, not HYP)
    @ FreeRTOS ARM_CA9 port requires VFP to be enabled
    mrc p15, 0, r0, c1, c0, 2    @ Read CPACR
    orr r0, r0, #0xF00000        @ Enable CP10 and CP11 (VFP/NEON)
    mcr p15, 0, r0, c1, c0, 2    @ Write CPACR
    isb                          @ Instruction sync barrier
    mov r0, #0x40000000          @ Enable FPU
    vmsr fpexc, r0               @ Write to FPEXC

    @ DEBUG: FPU enabled
    ldr r0, =0x3F201000
    mov r1, #0x46                @ ASCII 'F' for FPU enabled
    str r1, [r0]

    @ DEBUG: Before IRQ mode switch
    ldr r0, =0x3F201000
    mov r1, #0x31                @ ASCII '1'
    str r1, [r0]

    @ Set up stack pointer for IRQ mode
    cps #0x12                    @ Switch to IRQ mode

    @ DEBUG: After IRQ mode switch
    ldr r0, =0x3F201000
    mov r1, #0x32                @ ASCII '2'
    str r1, [r0]

    ldr sp, =irq_stack_top

    @ DEBUG: After setting IRQ stack
    ldr r0, =0x3F201000
    mov r1, #0x33                @ ASCII '3'
    str r1, [r0]

    @ Set up stack pointer for SVC mode (supervisor)
    cps #0x13                    @ Switch to SVC mode

    @ DEBUG: After SVC mode switch
    ldr r0, =0x3F201000
    mov r1, #0x34                @ ASCII '4'
    str r1, [r0]

    ldr sp, =stack_top

    @ DEBUG: After setting SVC stack
    ldr r0, =0x3F201000
    mov r1, #0x35                @ ASCII '5'
    str r1, [r0]

    @ Initialize BSS section
    ldr r0, =__bss_start__

    @ DEBUG: After loading BSS start
    ldr r3, =0x3F201000
    mov r1, #0x36                @ ASCII '6'
    str r1, [r3]

    ldr r1, =__bss_end__

    @ DEBUG: After loading BSS end
    ldr r3, =0x3F201000
    mov r4, #0x37                @ ASCII '7'
    str r4, [r3]

    mov r2, #0
bss_clear_loop:
    cmp r0, r1
    strlt r2, [r0], #4
    blt bss_clear_loop

    @ DEBUG: After BSS clear
    ldr r0, =0x3F201000
    mov r1, #0x38                @ ASCII '8'
    str r1, [r0]

    @ Jump to main
    ldr r0, =0x3F201000
    mov r1, #0x39                @ ASCII '9'
    str r1, [r0]

    bl main

    @ DEBUG: If main returns
    ldr r0, =0x3F201000
    mov r1, #0x52                @ ASCII 'R' for Return
    str r1, [r0]

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

@ Default exception handlers with debug output
undefined_handler:
    ldr r0, =0x3F201000
    mov r1, #0x55                @ ASCII 'U' for Undefined
    str r1, [r0]
    b hang

prefetch_handler:
    ldr r0, =0x3F201000
    mov r1, #0x50                @ ASCII 'P' for Prefetch
    str r1, [r0]
    b hang

data_handler:
    ldr r0, =0x3F201000
    mov r1, #0x44                @ ASCII 'D' for Data abort
    str r1, [r0]
    b hang

fiq_handler:
    ldr r0, =0x3F201000
    mov r1, #0x46                @ ASCII 'F' for FIQ
    str r1, [r0]
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
