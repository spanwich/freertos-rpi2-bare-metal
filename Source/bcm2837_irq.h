/*
 * BCM2837 Interrupt Controller Definitions
 *
 * BCM2837 has two interrupt controllers:
 * 1. ARM Local Peripherals (QA7) at 0x40000000 - per-core interrupts
 * 2. VideoCore Interrupt Controller at 0x3F00B000 - GPU/peripheral interrupts
 */

#ifndef BCM2837_IRQ_H
#define BCM2837_IRQ_H

#include <stdint.h>

/* ========== ARM Local Peripherals (QA7) - Base 0x40000000 ========== */
/* Per-core interrupt controller and local timers */

#define ARM_LOCAL_BASE              0x40000000

/* Control register offsets from ARM_LOCAL_BASE */
#define ARM_LOCAL_CONTROL           0x00
#define ARM_LOCAL_PRESCALER         0x08

/* Core interrupt routing */
#define ARM_LOCAL_GPU_INT_ROUTING   0x0C
#define ARM_LOCAL_PM_ROUTING_SET    0x10
#define ARM_LOCAL_PM_ROUTING_CLR    0x14

/* Core timer access (per-core) - LS 32 bits */
#define ARM_LOCAL_TIMER_LS          0x1C
/* Core timer access (per-core) - MS 32 bits */
#define ARM_LOCAL_TIMER_MS          0x20

/* Local interrupt routing */
#define ARM_LOCAL_INT_ROUTING       0x24

/* Local timer control & status */
#define ARM_LOCAL_TIMER_CONTROL     0x34
#define ARM_LOCAL_TIMER_WRITE       0x38

/* Core timer interrupt control (per core) */
#define ARM_LOCAL_TIMER_INT_CONTROL0 0x40
#define ARM_LOCAL_TIMER_INT_CONTROL1 0x44
#define ARM_LOCAL_TIMER_INT_CONTROL2 0x48
#define ARM_LOCAL_TIMER_INT_CONTROL3 0x4C

/* Mailboxes (per core) */
#define ARM_LOCAL_MAILBOX_INT_CONTROL0 0x50
#define ARM_LOCAL_MAILBOX_INT_CONTROL1 0x54
#define ARM_LOCAL_MAILBOX_INT_CONTROL2 0x58
#define ARM_LOCAL_MAILBOX_INT_CONTROL3 0x5C

/* IRQ/FIQ pending registers (per core) */
#define ARM_LOCAL_IRQ_PENDING0      0x60
#define ARM_LOCAL_IRQ_PENDING1      0x64
#define ARM_LOCAL_IRQ_PENDING2      0x68
#define ARM_LOCAL_IRQ_PENDING3      0x6C

#define ARM_LOCAL_FIQ_PENDING0      0x70
#define ARM_LOCAL_FIQ_PENDING1      0x74
#define ARM_LOCAL_FIQ_PENDING2      0x78
#define ARM_LOCAL_FIQ_PENDING3      0x7C

/* Local timer control bits */
#define ARM_LOCAL_TIMER_CTRL_RELOAD_SHIFT   0
#define ARM_LOCAL_TIMER_CTRL_RELOAD_MASK    0x0FFFFFFF
#define ARM_LOCAL_TIMER_CTRL_INT_ENABLE     (1 << 29)
#define ARM_LOCAL_TIMER_CTRL_INT_FLAG       (1 << 31)
#define ARM_LOCAL_TIMER_CTRL_ENABLE         (1 << 28)

/* Core timer interrupt control bits */
#define ARM_LOCAL_TIMER_INT_nCNTPNSIRQ  (1 << 0)  /* Physical non-secure timer */
#define ARM_LOCAL_TIMER_INT_nCNTPSIRQ   (1 << 1)  /* Physical secure timer */
#define ARM_LOCAL_TIMER_INT_nCNTHPIRQ   (1 << 2)  /* Hypervisor timer */
#define ARM_LOCAL_TIMER_INT_nCNTVIRQ    (1 << 3)  /* Virtual timer */


/* ========== VideoCore Interrupt Controller - Base 0x3F00B000 ========== */
/* Handles GPU and peripheral interrupts (UART, GPIO, Timer, etc.) */

#define IRQ_VC_BASE                 0x3F00B000

/* Interrupt controller register offsets */
#define IRQ_BASIC_PENDING           0x00
#define IRQ_PENDING_1               0x04
#define IRQ_PENDING_2               0x08
#define IRQ_FIQ_CONTROL             0x0C
#define IRQ_ENABLE_1                0x10
#define IRQ_ENABLE_2                0x14
#define IRQ_BASIC_ENABLE            0x18
#define IRQ_DISABLE_1               0x1C
#define IRQ_DISABLE_2               0x20
#define IRQ_BASIC_DISABLE           0x24

/* Interrupt numbers for IRQ_ENABLE_1 / IRQ_PENDING_1 (0-31) */
#define IRQ_SYSTEM_TIMER_0          0
#define IRQ_SYSTEM_TIMER_1          1
#define IRQ_SYSTEM_TIMER_2          2
#define IRQ_SYSTEM_TIMER_3          3
#define IRQ_AUX                     29   /* UART1, SPI1, SPI2 */

/* Interrupt numbers for IRQ_ENABLE_2 / IRQ_PENDING_2 (32-63) */
#define IRQ_GPIO_0                  49
#define IRQ_GPIO_1                  50
#define IRQ_GPIO_2                  51
#define IRQ_GPIO_3                  52
#define IRQ_I2C                     53
#define IRQ_SPI                     54
#define IRQ_PCM                     55
#define IRQ_UART                    57   /* PL011 UART0 */

/* Basic pending register bits */
#define IRQ_BASIC_ARM_TIMER         (1 << 0)
#define IRQ_BASIC_ARM_MAILBOX       (1 << 1)
#define IRQ_BASIC_ARM_DOORBELL_0    (1 << 2)
#define IRQ_BASIC_ARM_DOORBELL_1    (1 << 3)
#define IRQ_BASIC_GPU_0_HALTED      (1 << 4)
#define IRQ_BASIC_GPU_1_HALTED      (1 << 5)
#define IRQ_BASIC_ACCESS_ERROR_1    (1 << 6)
#define IRQ_BASIC_ACCESS_ERROR_0    (1 << 7)
#define IRQ_BASIC_PENDING_1         (1 << 8)
#define IRQ_BASIC_PENDING_2         (1 << 9)


/* ========== Helper Macros ========== */

/* Read from ARM local peripheral register */
#define ARM_LOCAL_REG(offset) \
    (*(volatile uint32_t *)(ARM_LOCAL_BASE + (offset)))

/* Write to ARM local peripheral register */
#define ARM_LOCAL_WRITE(offset, value) \
    do { *(volatile uint32_t *)(ARM_LOCAL_BASE + (offset)) = (value); } while(0)

/* Read from VideoCore interrupt controller */
#define IRQ_VC_REG(offset) \
    (*(volatile uint32_t *)(IRQ_VC_BASE + (offset)))

/* Write to VideoCore interrupt controller */
#define IRQ_VC_WRITE(offset, value) \
    do { *(volatile uint32_t *)(IRQ_VC_BASE + (offset)) = (value); } while(0)

/* Core-specific register access (for multi-core) */
#define ARM_LOCAL_CORE_REG(core, base_offset) \
    ARM_LOCAL_REG((base_offset) + ((core) * 4))


/* ========== FreeRTOS ARM_CA9 Port Compatibility Layer ========== */
/*
 * The FreeRTOS ARM_CA9 port expects ARM GIC registers.
 * BCM2837 doesn't have GIC - it has custom QA7 controller.
 * We create stub registers to satisfy the port's requirements.
 */

/* Stub GIC registers - these will be read/written but don't affect BCM2837 */
extern volatile uint32_t bcm2837_stub_gic_pmr;   /* Priority Mask Register stub */
extern volatile uint32_t bcm2837_stub_gic_bpr;   /* Binary Point Register stub */
extern volatile uint8_t bcm2837_stub_gic_priority[1024];  /* Priority registers stub */

/* Map GIC register accesses to stubs */
#define BCM2837_GIC_STUB_BASE   ((uint32_t)&bcm2837_stub_gic_priority[0])

#endif /* BCM2837_IRQ_H */
