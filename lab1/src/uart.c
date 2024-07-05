#include "../include/uart.h"

 /* Set baud rate and characteristics (115200 8N1) and map to GPIO */
void uart_init(void) {
    register unsigned int reg;

    /* initialize UART */
    *AUX_ENABLES        |=  1;      // enable UART1, AUX mini uart
    // Disable transmitter and receiver during configuration.
    *AUX_MU_CNTL_REG    =   0;  
    // Disable interrupt because currently you don’t need interrupt.
    *AUX_MU_IER_REG     =   0;
    *AUX_MU_LCR_REG     =   3;      // Set the data size to 8 bit.
    *AUX_MU_MCR_REG     =   0;      // Don’t need auto flow control.
    *AUX_MU_BAUD_REG    =   270;    // Set baud rate to 115200
    // set the interrupt status to impossible status 
    // and the writing status clear the FIFO
    *AUX_MU_IIR_REG     =   0x6;

    /* map UART1 to GPIO pins */
    // Check table 6-3 in p. 92
    reg = *GPFSEL1;
    
    // clean GPIO pin 14 and 15
    // [14:12]: FSEL14 (functionality of 14-th GPIO pin)
    // [17:15]: FSEL17 
    reg &= ~((7<<12) | (7<<15));

    // 2 = 010 = GPIO pin takes alternate function 5
    // ALT5: p.102
    reg |= (2<<12) | (2<<15);
    *GPFSEL1 = reg;

    // disable pull-up/down (p. 101)
    *GPPUD = 0;

    // Wait 150 cycles
    // this provides the required set-up time for the control signal 
    reg = 150; 
    while(reg--) asm volatile("nop");

    // enable pins 14 and 15
    *GPPUDCLK0 = (1<<14) | (1<<15);

    // Wait 150 cycles 
    // this provides the required hold time for the control signal 
    reg = 150; 
    while(reg--) asm volatile("nop");

    // flush GPIO setup
    *GPPUDCLK0 = 0;

    // Enable the transmitter and receiver.
    *AUX_MU_CNTL_REG = 3;
}

/**
 * Receive a character
 */
char uart_getc() {
    char reg;
    // wait until something is in the buffer
    // p.15: [0] is the field "symbol available"
    // if this field is set then mini UART receive FIFO 
    // contains at least 1 symbol
    do asm volatile("nop");
    while(!(*AUX_MU_LSR_REG & 0x01));

    /* read it and return */
    // [7:0]: receive data
    reg = (char) (*AUX_MU_IO_REG);

    /* convert carriage return to newline */
    return reg == '\r' ? '\n' : reg;
}

/**
 * Send a character
 */
void uart_putc(unsigned int c) {
    /* wait until we can send */
    do asm volatile("nop");
    while (!(*AUX_MU_LSR_REG & 0x20));

    /* write the character to the buffer */
    *AUX_MU_IO_REG = c;
}


/**
 * Display a string
 */
void uart_puts(const char *s) {
    while (*s) {
        /* convert newline to carriage return + newline */
        if (*s == '\n') uart_putc('\r');
        uart_putc(*s++);
    }
}
