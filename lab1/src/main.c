#include "../include/uart.h"
#include "../include/shell.h"
#include "../include/mailbox.h"

void main(void) {
    uart_init();
    uart_puts("Hello World!\n");
    shell();

    return;
}