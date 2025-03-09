// #ifndef __GNUC__
// #define __asm__ asm
// #endif

#include "../include/uart.h"
#include "../include/shell.h"

extern char* _relocate_addr;
extern char* const _code_start;
extern unsigned long long _code_size;

void code_relocate(void);

static int had_relocated = 0;

void main(void) {
    if (had_relocated == 0) {
        had_relocated = 1;
        code_relocate();
    } else {
        uart_puts("Run in new place!\n");
    }

    uart_init();
    uart_puts("Hello World!\n");

    shell();

    return;
}

void code_relocate(void)
{
    uart_puts("Start relocating!\n");
    char *relocate_addr = (char*) &_relocate_addr;
    char* const code_start = (char*) &_code_start;
    unsigned long long loader_size = (unsigned long long) &_code_size;

    for (unsigned long long i=0; i<loader_size; ++i)
        relocate_addr[i] = code_start[i];
    
    uart_puts("Relocating completed!\n");
    __asm__ volatile (
        "mov x0, x10;"
        "ldr x30, =_relocate_addr;"
        "ret;"
    );
    /* ((void (*)()) relocate_addr)(); */
}