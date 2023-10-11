/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple tty device driver
 * uart_getc() and uart_putc() are implemented in bus_uart.c
 * printf-related functions are linked from the compiler's C library
 */

#define LIBC_STDIO
#include "egos.h"
#include <stdio.h>
#include <stdarg.h>

void uart0_init(int baud_rate);
void uart0_putc(char c);
char uart0_getc();
int uart0_has_data();

static int c, is_reading;
int tty_recv_intr() { 
    if (is_reading) return 0;
    else return uart0_has_data() && (uart0_getc() == 3); 
}

int tty_write(char* buf, int len) {
    for (int i = 0; i < len; i++) uart0_putc(buf[i]);
}

int tty_read(char* buf, int len) {
    is_reading = 1;
    for (int i = 0; i < len - 1; i++) {
        c = uart0_getc();
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
    LOG("", "") //LOG("\x1B[38;5;250m[DEBUG] ", "\x1B[1;0m\r\n")
    fflush(stdout);
}

int tty_info(const char *format, ...) { LOG("[INFO] ", "\r\n") }

int tty_fatal(const char *format, ...)
{
    LOG("\x1B[1;31m[FATAL] ", "\x1B[1;0m\r\n") /* red color */
    while(1);
}

int tty_success(const char *format, ...)
{
    LOG("\x1B[1;32m[SUCCESS] ", "\x1B[1;0m\r\n") /* green color */
}

int tty_critical(const char *format, ...)
{
    LOG("\x1B[1;33m[CRITICAL] ", "\x1B[1;0m\r\n") /* yellow color */
}

void tty_init() {

    uart0_init(115200);

    /* Wait for the tty device to be ready
       TODO: replace with usleep
     */
    for (int i = 0; i < 2000000; i++);

    earth->tty_read = tty_read;
    earth->tty_write = tty_write;
    earth->tty_recv_intr = tty_recv_intr;
    
    earth->tty_printf = tty_printf;
    earth->tty_info = tty_info;
    earth->tty_fatal = tty_fatal;
    earth->tty_success = tty_success;
    earth->tty_critical = tty_critical;
}
