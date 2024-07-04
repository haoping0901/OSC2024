#include "../include/uart.h"
#include "../include/shell.h"

void main(void) {
    uart_init();
    shell();

    return;
}