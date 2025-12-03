/*
 * UART Driver for RPi2 BCM2837
 * PL011 UART0 at 0x3F201000
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stddef.h>

/* Initialize UART */
void uart_init(void);

/* Basic character I/O */
void uart_putc(char c);
char uart_getc(void);
void uart_puts(const char *s);

/* Formatted output */
void uart_hex(uint32_t val);
void uart_decimal(uint32_t val);

/* Printf-style output (simple version) */
int uart_printf(const char *format, ...);

#endif /* UART_H */
