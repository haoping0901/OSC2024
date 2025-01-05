#include "../include/uart.h"
#include "../include/shell.h"
#include "../include/mailbox.h"
#include "../include/allocator.h"
#include "../include/utils.h"
// #include "../include/dtb.h"

void main(void)
{
    // dtb_traverse(dtb_initramfs_callback);
    
    uart_init();
    uart_puts("Hello World!\n");

    char *test_str1 = simple_malloc(sizeof("test str1\n"));
    memcpy(test_str1, "test str1\n", sizeof("test str1\n"));
    uart_puts(test_str1);

    char *test_str2 = simple_malloc(sizeof("Hello test!\n"));
    memcpy(test_str2, "Hello test!\n", sizeof("Hello test!\n"));
    uart_puts(test_str2);

    shell();

    return;
}