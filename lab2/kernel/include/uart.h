#include "gpio.h"

#ifndef UART_H
#define UART_H

void uart_init(void);
char uart_getc(const unsigned int cvt2nl);
void uart_putc(unsigned int c);
void uart_puts(const char *s);
void uart_puthex(unsigned int d);

#endif