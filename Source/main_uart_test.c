/*
 * Minimal UART test - no FreeRTOS, just print a known pattern.
 * Used to diagnose serial hardware vs software issues.
 */
#include <stdint.h>
#include <stddef.h>
#include "uart.h"

/* Stubs needed by startup code (no FreeRTOS) */
void vAssertCalled(unsigned long ulLine, const char * const pcFileName) { (void)ulLine; (void)pcFileName; for(;;); }

/* Required by linker since startup references these */
void FreeRTOS_SWI_Handler(void);
void FreeRTOS_IRQ_Handler(void);
void FreeRTOS_SWI_Handler(void) { for(;;); }
void FreeRTOS_IRQ_Handler(void) { for(;;); }

void delay_ms(volatile uint32_t ms) {
    /* Rough delay at ~1GHz core clock */
    for (volatile uint32_t i = 0; i < ms * 10000; i++) {
        __asm__ volatile("nop");
    }
}

int main(void) {
    uart_init();

    uart_puts("\n\n");
    uart_puts("========================================\n");
    uart_puts("UART HARDWARE TEST - NO FREERTOS\n");
    uart_puts("Platform: Raspberry Pi 2B\n");
    uart_puts("Baud: 115200 8N1\n");
    uart_puts("========================================\n");
    uart_puts("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n");
    uart_puts("abcdefghijklmnopqrstuvwxyz\n");
    uart_puts("0123456789\n");
    uart_puts("The quick brown fox jumps over the lazy dog.\n");
    uart_puts("========================================\n\n");

    uint32_t count = 0;
    while (1) {
        uart_puts("PING ");
        uart_decimal(count);
        uart_puts("\n");
        count++;
        delay_ms(1000);
    }

    return 0;
}
