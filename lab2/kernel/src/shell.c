#include "../include/shell.h"
#include "../include/uart.h"
#include "../include/utils.h"
#include "../include/mailbox.h"
#include "../include/reboot.h"
#include "../include/cpio.h"

#define BUFFER_SIZE 128

typedef enum {
    BACK_SPACE = 8,
    LINE_FEED = 10,
    CARRIAGE_RETURN = 13,
    DELETE = 127,
    UNKNOWN = 512,
    REGULAR_INPUT = 513,
} SPECIAL_CHAR;

static inline SPECIAL_CHAR parse_char(const char recv_char)
{
    if (recv_char > 127 || recv_char < 0) 
        return UNKNOWN;
    if (recv_char == BACK_SPACE || recv_char == DELETE)
        return BACK_SPACE;
    if (recv_char == LINE_FEED || recv_char == CARRIAGE_RETURN)
        return LINE_FEED;
    
    return REGULAR_INPUT;
}

static inline void input_buffer_overflow_protection(const char *buf)
{
    uart_puts("The command being entered is too long to process.\n");
    return;
}

static inline void help_command(void)
{
    uart_puts("help         : print this help menu\n");
    uart_puts("hello        : print Hello World!\n");
    uart_puts("sysinfo      : print the system informations\n");
    uart_puts("reboot       : reboot the device\n");
    uart_puts("ls           : list information about the files in the current"
              "directory\n");
    uart_puts("cat [FILE]   : concatenate FILE to standard output\n");

    return;
}

static inline void hello_command(void)
{
    uart_puts("Hello World!\n");
    return;
}

static inline void sysinfo_command(void)
{
    get_sys_info();

    return;
}

static inline void reboot_command(void)
{
    uart_puts("\nRebooting...\n\n");
    reset(100000);

    return;
}

static inline void ls_command()
{
    cpio_ls();

    return;
}

static inline void cat_command(const char* cat_filename)
{
    cpio_cat(cat_filename);

    return;
}

static inline void print_unknown(void)
{
    uart_puts("Unknown input!\n");
    return;
}

static inline void parse_commands(const char *buf)
{
    if (strcmp(buf, "help") == 0) {
        help_command();
    } else if (strcmp(buf, "hello") == 0) {
        hello_command();
    } else if (strcmp(buf, "reboot") == 0) {
        reboot_command();
    } else if (strcmp(buf, "sysinfo") == 0) {
        sysinfo_command();
    } else if (strcmp(buf, "ls") == 0) {
        ls_command();
    } else if (strncmp(buf, "cat", 3) == 0) {
        cat_command(buf+4);
    } else {
        print_unknown();
    }

    return;
}

static void put_char(const SPECIAL_CHAR schar, const char recv_char, 
                     char buf[], int * const buf_idx)
{
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
            parse_commands(buf);
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

void shell()
{
    SPECIAL_CHAR schar;
    int buf_idx = 0;
    char recv_char;
    char buf[BUFFER_SIZE];
    
    memset(buf, '\0', BUFFER_SIZE);
    uart_puts("# ");

    while (1) {
        recv_char = uart_getc(1);
        schar = parse_char(recv_char);
        put_char(schar, recv_char, buf, &buf_idx);
    }

    return;
}

