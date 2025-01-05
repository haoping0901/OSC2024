#include "../include/uart.h"
#include "../include/shell.h"
#include "../include/mailbox.h"
// #include "../include/dtb.h"

void main(void)
{
    // dtb_traverse(dtb_initramfs_callback);
    
    uart_init();
    uart_puts("Hello World!\n");
    shell();

    return;
}