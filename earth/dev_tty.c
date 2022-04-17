/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple tty device driver
 * uart_getc() and uart_putc() are implemented in bus_uart.c
 * printf() is implemented in the C library, see library/libc/print.c
 */

#include <stdio.h>
#include <stdarg.h>
#include "bus_uart.c"

void tty_init() {
    uart_init(115200);

    /* Wait for the tty device to be ready */
    for (int i = 0; i < 2000000; i++);
    for (int c = 0; c != -1; uart_getc(&c));
}

static int c, is_reading;
int tty_intr() {
    return (is_reading)? 0 : (uart_getc(&c) == 3);
}

int tty_write(char* buf, int len) {
    for (int i = 0; i < len; i++) uart_putc(buf[i]);
}

int tty_read(char* buf, int len) {
    is_reading = 1;
    for (int i = 0; i < len - 1; i++) {
        for (c = -1; c == -1; uart_getc(&c));
        buf[i] = (char)c;

        switch (c) {
        case 0x03:  /* Ctrl+C    */
            buf[0] = 0;
        case 0x0d:  /* Enter     */
            buf[i] = is_reading = 0;
            printf("\r\n");
            return c == 0x03? 0 : i;
        case 0x7f:  /* Backspace */
            c = 0;
            if (i) printf("\b \b");
            i = i ? i - 2 : i - 1;
        }

        if (c) printf("%c", c);
        fflush(stdout);
    }

    buf[len - 1] = is_reading = 0;    
    return len - 1;
}

#define LOG(x, y)  printf(x); \
                   va_list args; \
                   va_start(args, format); \
                   vprintf(format, args); \
                   va_end(args); \
                   printf(y); \

int tty_printf(const char *format, ...)
{
    LOG("", "")
    fflush(stdout);
}

int tty_info(const char *format, ...)
{
    LOG("[INFO] ", "\r\n")
}

int tty_fatal(const char *format, ...)
{
    LOG("\x1B[1;31m[FATAL] ", "\x1B[1;0m\r\n")
    while(1);
}

int tty_success(const char *format, ...)
{
    LOG("\x1B[1;32m[SUCCESS] ", "\x1B[1;0m\r\n")
}

int tty_critical(const char *format, ...)
{
    LOG("\x1B[1;33m[CRITICAL] ", "\x1B[1;0m\r\n")
}
