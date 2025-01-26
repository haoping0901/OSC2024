#ifndef __GNUC__
#define __asm__ asm
#endif

#include "../include/shell.h"
#include "../include/uart.h"
#include "../include/utils.h"
#include "../include/reboot.h"

#define BUFFER_SIZE 128

extern char* const _code_start;

typedef enum {
    BACK_SPACE = 8,
    LINE_FEED = 10,
    CARRIAGE_RETURN = 13,
    DELETE = 127,
    UNKNOWN = 512,
    REGULAR_INPUT = 513,
} SPECIAL_CHAR;

SPECIAL_CHAR parse_char(const char recv_char)
{
    if (recv_char > 127 || recv_char < 0) 
        return UNKNOWN;
    if (recv_char == BACK_SPACE || recv_char == DELETE)
        return BACK_SPACE;
    if (recv_char == LINE_FEED || recv_char == CARRIAGE_RETURN)
        return LINE_FEED;
    
    return REGULAR_INPUT;
}

void input_buffer_overflow_protection(const char *buf)
{
    uart_puts("The command being entered is too long to process.\n");
    return;
}

void help_command(void)
{
    uart_puts("help     : print this help menu\n");
    uart_puts("hello    : print Hello World!\n");
    uart_puts("reboot   : reboot the device\n");
    uart_puts("load     : load the kernel through uart1\n");

    return;
}

void hello_command(void)
{
    uart_puts("Hello World!\n");
    return;
}

void reboot_command(void)
{
    uart_puts("\nRebooting...\n\n");
    reset(100000);
    return;
}

void load_command(void)
{
    char* const kernel_addr = (char*) &_code_start;
    unsigned long long kernel_size = 0;
    char ch;

    for (int i=0; i<4; ++i) {
        ch = uart_getc(0);

        kernel_size += ch << (i << 3);
    }

    for (unsigned long long idx=0; idx<kernel_size; ++idx) {
        ch = uart_getc(0);
        kernel_addr[idx] = ch;
    }

    uart_puts("Kernel successfully loaded!\n");

    __asm__ volatile(
        "mov x0, x10;"
        "mov x30, 0x80000;"
        "ret;"
    );
    // ((void (*)()) kernel_addr)();/
}

void print_unknown(void)
{
    uart_puts("Unknown input!\n");
    return;
}

void commands_cmp(const char *buf)
{
    if (!strcmp(buf, "help")) {
        help_command();
    } else if (!strcmp(buf, "hello")) {
        hello_command();
    } else if (!strcmp(buf, "reboot")) {
        reboot_command();
    } else if (!strcmp(buf, "load")) {
        load_command();
    } else {
        print_unknown();
    }

    return;
}

void put_char(const SPECIAL_CHAR schar, const char recv_char, 
              char buf[], int* const buf_idx)
{
    switch (schar) {
    case BACK_SPACE: 
        if (*buf_idx > 0) {
            --*buf_idx;
        }
        uart_puts("\b \b");
        break;
    case LINE_FEED:
        uart_puts("\n");
        
        if (*buf_idx == BUFFER_SIZE) {
            input_buffer_overflow_protection(buf);
        } else {
            buf[*buf_idx] = '\0';
            commands_cmp(buf);
        }

        *buf_idx = 0;
        memset(buf, 0, BUFFER_SIZE);
        uart_puts("# ");
        break;
    case UNKNOWN:
        return;
    case REGULAR_INPUT:
        uart_putc(recv_char);

        if (*buf_idx < BUFFER_SIZE) {
            buf[(*buf_idx)++] = recv_char;
        }
        
        break;
    default:
        break;
    } 

    return;
}

void shell() {
    SPECIAL_CHAR schar;
    int buf_idx;
    char recv_char;
    char buf[BUFFER_SIZE];
    
    buf_idx = 0;
    memset(buf, '\0', BUFFER_SIZE);
    uart_puts("# ");

    // keep reading the input character 
    while (1) {
        recv_char = uart_getc(1);
        schar = parse_char(recv_char);
        put_char(schar, recv_char, buf, &buf_idx);
    }

    return;
}

