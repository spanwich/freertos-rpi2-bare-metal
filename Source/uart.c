/*
 * UART Driver for RPi2 BCM2837
 * PL011 UART0 Implementation
 */

#include "uart.h"
#include <stdarg.h>

/* PL011 UART0 registers - BCM2837 uses 0x3F000000 peripheral base */
#define UART0_BASE      0x3F201000

#define UART0_DR        (*(volatile uint32_t *)(UART0_BASE + 0x00))  /* Data register */
#define UART0_FR        (*(volatile uint32_t *)(UART0_BASE + 0x18))  /* Flag register */
#define UART0_IBRD      (*(volatile uint32_t *)(UART0_BASE + 0x24))  /* Integer baud rate */
#define UART0_FBRD      (*(volatile uint32_t *)(UART0_BASE + 0x28))  /* Fractional baud rate */
#define UART0_LCRH      (*(volatile uint32_t *)(UART0_BASE + 0x2C))  /* Line control */
#define UART0_CR        (*(volatile uint32_t *)(UART0_BASE + 0x30))  /* Control register */
#define UART0_ICR       (*(volatile uint32_t *)(UART0_BASE + 0x44))  /* Interrupt clear */

/* Flag register bits */
#define UART_FR_TXFF    (1 << 5)  /* Transmit FIFO full */
#define UART_FR_RXFE    (1 << 4)  /* Receive FIFO empty */

/* Control register bits */
#define UART_CR_UARTEN  (1 << 0)  /* UART enable */
#define UART_CR_TXE     (1 << 8)  /* Transmit enable */
#define UART_CR_RXE     (1 << 9)  /* Receive enable */

/* Line control bits */
#define UART_LCRH_WLEN_8BIT  (3 << 5)  /* 8-bit word length */
#define UART_LCRH_FEN        (1 << 4)  /* Enable FIFOs */

void uart_init(void) {
    /* Disable UART */
    UART0_CR = 0;

    /* Clear all interrupts */
    UART0_ICR = 0x7FF;

    /* Set baud rate to 115200 */
    /* UART clock = 48MHz (Pi 2B default PL011 clock)
     * Divisor = 48000000 / (16 * 115200) = 26.0416...
     * Integer part: 26, Fractional part: 0.0416... * 64 = 2.66 ≈ 3
     */
    UART0_IBRD = 26;
    UART0_FBRD = 3;

    /* 8-bit, no parity, 1 stop bit, FIFOs enabled */
    UART0_LCRH = UART_LCRH_WLEN_8BIT | UART_LCRH_FEN;

    /* Enable UART, TX, and RX */
    UART0_CR = UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE;
}

void uart_putc(char c) {
    /* Wait until TX FIFO is not full */
    while (UART0_FR & UART_FR_TXFF);

    /* Write character */
    UART0_DR = (uint32_t)c;

    /* Convert \n to \r\n for proper terminal output */
    if (c == '\n') {
        while (UART0_FR & UART_FR_TXFF);
        UART0_DR = '\r';
    }
}

char uart_getc(void) {
    /* Wait until RX FIFO has data */
    while (UART0_FR & UART_FR_RXFE);

    /* Read character */
    return (char)(UART0_DR & 0xFF);
}

void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}

void uart_hex(uint32_t val) {
    uart_puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        int digit = (val >> i) & 0xF;
        uart_putc(digit < 10 ? '0' + digit : 'A' + digit - 10);
    }
}

void uart_decimal(uint32_t val) {
    char buffer[12];
    int i = 0;

    if (val == 0) {
        uart_putc('0');
        return;
    }

    while (val > 0) {
        buffer[i++] = '0' + (val % 10);
        val /= 10;
    }

    /* Print in reverse order */
    while (i > 0) {
        uart_putc(buffer[--i]);
    }
}

/* Simple printf implementation supporting %s, %d, %x, %c */
int uart_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 's': {
                    const char *s = va_arg(args, const char *);
                    uart_puts(s ? s : "(null)");
                    break;
                }
                case 'd': {
                    int val = va_arg(args, int);
                    if (val < 0) {
                        uart_putc('-');
                        val = -val;
                    }
                    uart_decimal((uint32_t)val);
                    break;
                }
                case 'u': {
                    uint32_t val = va_arg(args, uint32_t);
                    uart_decimal(val);
                    break;
                }
                case 'x': {
                    uint32_t val = va_arg(args, uint32_t);
                    uart_hex(val);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    uart_putc(c);
                    break;
                }
                case '%': {
                    uart_putc('%');
                    break;
                }
                default:
                    uart_putc('%');
                    uart_putc(*format);
                    break;
            }
        } else {
            uart_putc(*format);
        }
        format++;
    }

    va_end(args);
    return 0;
}
