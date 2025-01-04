#include "../include/uart.h"
#include "../include/shell.h"

extern char* _relocate_addr;
extern char* _code_start;
extern unsigned long long _code_size;

void code_relocate(void);

static int had_relocated = 0;

void main(void) {
    if (had_relocated == 0) {
        had_relocated = 1;
        code_relocate();
    }

    uart_init();
    uart_puts("Hello World!\n");

    shell();

    return;
}

void code_relocate(void)
{
    char *relocate_addr = (char*) &_relocate_addr;
    char* _code_start = (char*) &_code_start;
    unsigned long long loader_size = (unsigned long long) &_code_size;

    for (unsigned long long i=0; i<loader_size; ++i)
        relocate_addr[i] = _code_start[i];
    
    ((void (*)()) relocate_addr)();
}