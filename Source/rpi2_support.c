/*
 * RPi2 BCM2837 Support Functions
 * Provides minimal libc functions and hardware setup
 */

#include "FreeRTOS.h"
#include "task.h"
#include <stddef.h>

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

/* ARM Generic Timer configuration for BCM2837 */
void vConfigureTickInterrupt(void) {
    /*
     * BCM2837 uses ARM Generic Timer
     * Timer frequency is 19.2MHz on RPi3 (same chip as RPi2 v1.2)
     */
    uint32_t ulCompareMatch;

    /* Read current timer count */
    __asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r" (ulCompareMatch));

    /* Calculate next tick time
     * Timer runs at 19.2MHz, we want 1000 ticks/sec
     * So: 19200000 / 1000 = 19200 counts per tick
     */
    ulCompareMatch += (19200000 / configTICK_RATE_HZ);

    /* Set compare value */
    __asm volatile("mcrr p15, 2, %Q0, %R0, c14" :: "r" (ulCompareMatch));

    /* Enable timer interrupt */
    uint32_t ulControl = 0x01;  /* Enable timer */
    __asm volatile("mcr p15, 0, %0, c14, c2, 1" :: "r" (ulControl));
}

/* ARM Generic Timer interrupt handler */
void vClearTickInterrupt(void) {
    /* Timer auto-reloads, but we need to acknowledge interrupt */
    uint32_t ulControl = 0x01;  /* Re-enable */
    __asm volatile("mcr p15, 0, %0, c14, c2, 1" :: "r" (ulControl));
}
