#include "../include/shell.h"
#include "../include/uart.h"
#include "../include/utils.h"
#include "../include/mailbox.h"
#include "../include/reboot.h"

#define BUFFER_SIZE 128

typedef enum {
    BACK_SPACE = 8,
    LINE_FEED = 10,
    CARRIAGE_RETURN = 13,
    DELETE = 127,
    UNKNOWN = 512,
    REGULAR_INPUT = 513,
} SPECIAL_CHAR;

SPECIAL_CHAR parse_char(const char recv_char) {
    if (recv_char > 127 || recv_char < 0) 
        return UNKNOWN;
    if (recv_char == BACK_SPACE || recv_char == DELETE)
        return BACK_SPACE;
    if (recv_char == LINE_FEED || recv_char == CARRIAGE_RETURN)
        return LINE_FEED;
    
    return REGULAR_INPUT;
}

void input_buffer_overflow_protection(const char *buf) {
    uart_puts("The command being entered is too long to process.\n");
    return;
}

void help_command(void) {
    uart_puts("help     : print this help menu\n");
    uart_puts("hello    : print Hello World!\n");
    uart_puts("sysinfo  : print the system informations\n");
    uart_puts("reboot   : reboot the device\n");
    // uart_puts("h        : halt rebooting\n");

    return;
}

void hello_command(void) {
    uart_puts("Hello World!\n");
    return;
}

void sysinfo_command(void) {
    get_sys_info();

    return;
}

void reboot_command(void) {
    uart_puts("\nRebooting...\n\n");
    reset(100000);
    return;
}

void halt_command(void) {
    uart_puts("Halt the reboot... ");
    cancel_reset();
    uart_puts("Finished!\n");
    return;
}

void print_unknown(void) {
    uart_puts("Unknown input!\n");
    return;
}

void commands_cmp(const char *buf) {
    if (!strcmp(buf, "help")) help_command();
    else if (!strcmp(buf, "hello")) hello_command();
    else if (!strcmp(buf, "reboot")) reboot_command();
    else if (!strcmp(buf, "sysinfo")) sysinfo_command();
    // else if (!strcmp(buf, "h")) halt_command();
    else print_unknown();

    return;
}

void put_char(const SPECIAL_CHAR schar, const char recv_char, 
              char buf[], int * const buf_idx) {
    switch (schar) {
        case BACK_SPACE: 
            if (*buf_idx > 0) --*buf_idx;
            uart_puts("\b \b");
            break;
        case LINE_FEED:
            uart_puts("\n");
            
            if (*buf_idx == BUFFER_SIZE)
                input_buffer_overflow_protection(buf);
            else {
                buf[*buf_idx] = '\0';

                // check commands
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

            if (*buf_idx < BUFFER_SIZE) 
                buf[(*buf_idx)++] = recv_char;
            
            break;
        default:
            break;
    } 

    return;
}

void shell() {
    int buf_idx;
    char recv_char;
    char buf[BUFFER_SIZE];
    SPECIAL_CHAR schar;
    
    buf_idx = 0;
    memset(buf, '\0', BUFFER_SIZE);
    uart_puts("# ");

    // keep reading the input character 
    while (1) {
        recv_char = uart_getc();
        schar = parse_char(recv_char);
        put_char(schar, recv_char, buf, &buf_idx);
    }

    return;
}

