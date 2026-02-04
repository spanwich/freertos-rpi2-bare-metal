/*
 * RPi2 BCM2837 Support Functions
 * Provides minimal libc functions and hardware setup
 */

#include "FreeRTOS.h"
#include "task.h"
#include "bcm2837_irq.h"
#include <stddef.h>
#include <stdint.h>

/* ========== GIC Stub Registers for FreeRTOS ARM_CA9 Port ========== */
/*
 * BCM2837 doesn't have ARM GIC, but FreeRTOS ARM_CA9 port expects it.
 * These stubs prevent crashes when port code tries to access GIC registers.
 */
volatile uint32_t bcm2837_stub_gic_pmr = 0xFF;   /* Priority mask (all enabled) */
volatile uint32_t bcm2837_stub_gic_bpr = 0x00;   /* Binary point = 0 (as expected by port) */
volatile uint8_t bcm2837_stub_gic_priority[1024] = {0};  /* Priority array */

/* Minimal libc functions for FreeRTOS */
size_t strlen(const char *s) {
    const char *p = s;
    while (*p) p++;
    return p - s;
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

int snprintf(char *str, size_t size, const char *format, ...) {
    /* Minimal implementation - just copy format string for now */
    size_t i;
    for (i = 0; i < size - 1 && format[i]; i++) {
        str[i] = format[i];
    }
    str[i] = '\0';
    return i;
}

/* FreeRTOS hook functions */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    (void)pcTaskName;
    /* Hang on stack overflow */
    while(1) {}
}

void vApplicationMallocFailedHook(void) {
    /* Hang on malloc failure */
    while(1) {}
}

/* ========== BCM2837 Interrupt Controller Initialization ========== */

/*
 * Initialize BCM2837 interrupt controllers
 * BCM2837 has TWO interrupt controllers:
 * 1. ARM Local (QA7) at 0x40000000 - for timers and core interrupts
 * 2. VideoCore at 0x3F00B000 - for peripherals (UART, GPIO, etc.)
 */
void bcm2837_irq_init(void) {
    /* Initialize GIC stub registers to expected values */
    /* FreeRTOS port checks: ucMaxPriorityValue == portLOWEST_INTERRUPT_PRIORITY */
    /* For 32 priorities (5 bits), portLOWEST_INTERRUPT_PRIORITY = 31 */
    /* Priority register must read back as 0xF8 (top 5 bits set) for 32 priorities */
    bcm2837_stub_gic_priority[0] = 0xF8;  /* Simulate 5-bit priority (32 levels) */
    bcm2837_stub_gic_bpr = 0x00;          /* Binary point = 0 (all bits for priority) */
    bcm2837_stub_gic_pmr = 0xFF;          /* Priority mask (all interrupts enabled) */

    /* Disable all VideoCore interrupts initially */
    IRQ_VC_WRITE(IRQ_DISABLE_1, 0xFFFFFFFF);
    IRQ_VC_WRITE(IRQ_DISABLE_2, 0xFFFFFFFF);
    IRQ_VC_WRITE(IRQ_BASIC_DISABLE, 0xFFFFFFFF);

    /* Route all GPU interrupts to core 0 */
    ARM_LOCAL_WRITE(ARM_LOCAL_GPU_INT_ROUTING, 0x00);

    /* Clear any pending local timer interrupt */
    uint32_t timer_ctrl = ARM_LOCAL_REG(ARM_LOCAL_TIMER_CONTROL);
    timer_ctrl |= ARM_LOCAL_TIMER_CTRL_INT_FLAG;  /* Clear by writing 1 */
    ARM_LOCAL_WRITE(ARM_LOCAL_TIMER_CONTROL, timer_ctrl);
}

/*
 * Enable specific VideoCore peripheral interrupt
 * For GPIO (IRQ 49-52) and UART (IRQ 57)
 */
void bcm2837_enable_vc_irq(uint32_t irq_num) {
    if (irq_num < 32) {
        IRQ_VC_WRITE(IRQ_ENABLE_1, (1 << irq_num));
    } else if (irq_num < 64) {
        IRQ_VC_WRITE(IRQ_ENABLE_2, (1 << (irq_num - 32)));
    }
}

/*
 * Disable specific VideoCore peripheral interrupt
 */
void bcm2837_disable_vc_irq(uint32_t irq_num) {
    if (irq_num < 32) {
        IRQ_VC_WRITE(IRQ_DISABLE_1, (1 << irq_num));
    } else if (irq_num < 64) {
        IRQ_VC_WRITE(IRQ_DISABLE_2, (1 << (irq_num - 32)));
    }
}

/* ========== ARM Generic Timer Configuration ========== */

/*
 * BCM2837 ARM Generic Timer configuration for FreeRTOS tick
 *
 * NOTE: BCM2837 has multiple timer options:
 * 1. ARM Generic Timer (CP15) - 19.2 MHz
 * 2. ARM Local Timer (QA7) - configurable divider
 * 3. System Timer (VideoCore) - 1 MHz
 *
 * We use ARM Generic Timer (option 1) as it's most portable
 */
void vConfigureTickInterrupt(void) {
    /*
     * ARM Generic Timer frequency on BCM2837:
     * - Physical timer runs at 19.2 MHz (crystal frequency)
     * - For 1000 Hz tick (1ms), need: 19200000 / 1000 = 19200 counts
     */
    const uint32_t timer_freq = 19200000;  /* 19.2 MHz */
    const uint32_t counts_per_tick = timer_freq / configTICK_RATE_HZ;

    /* Read current timer count (64-bit value split into two 32-bit reads) */
    uint32_t timer_count_lo, timer_count_hi;
    __asm volatile("mrrc p15, 0, %0, %1, c14" : "=r" (timer_count_lo), "=r" (timer_count_hi));

    /* Calculate next tick time */
    timer_count_lo += counts_per_tick;
    /* Handle carry into high word if needed */
    if (timer_count_lo < counts_per_tick) {
        timer_count_hi++;
    }

    /* Set compare value (CNTP_CVAL) */
    __asm volatile("mcrr p15, 2, %0, %1, c14" :: "r" (timer_count_lo), "r" (timer_count_hi));

    /* Enable physical timer interrupt (CNTP_CTL)
     * Bit 0: ENABLE - Timer enabled
     * Bit 1: IMASK - Interrupt not masked (0 = enabled)
     * Bit 2: ISTATUS - Condition met (read-only)
     */
    uint32_t timer_ctrl = 0x01;  /* Enable timer, interrupt enabled */
    __asm volatile("mcr p15, 0, %0, c14, c2, 1" :: "r" (timer_ctrl));

    /* Enable physical timer IRQ in ARM local interrupt controller (core 0) */
    uint32_t int_ctrl = ARM_LOCAL_CORE_REG(0, ARM_LOCAL_TIMER_INT_CONTROL0);
    int_ctrl |= ARM_LOCAL_TIMER_INT_nCNTPNSIRQ;  /* Enable physical non-secure timer IRQ */
    ARM_LOCAL_WRITE(ARM_LOCAL_TIMER_INT_CONTROL0, int_ctrl);
}

/*
 * Clear/acknowledge timer interrupt
 */
void vClearTickInterrupt(void) {
    /* For ARM Generic Timer, we need to set next compare value */
    const uint32_t timer_freq = 19200000;  /* 19.2 MHz */
    const uint32_t counts_per_tick = timer_freq / configTICK_RATE_HZ;

    /* Read current compare value */
    uint32_t cmp_lo, cmp_hi;
    __asm volatile("mrrc p15, 2, %0, %1, c14" : "=r" (cmp_lo), "=r" (cmp_hi));

    /* Add tick interval to compare value */
    cmp_lo += counts_per_tick;
    if (cmp_lo < counts_per_tick) {
        cmp_hi++;
    }

    /* Write new compare value */
    __asm volatile("mcrr p15, 2, %0, %1, c14" :: "r" (cmp_lo), "r" (cmp_hi));
}
