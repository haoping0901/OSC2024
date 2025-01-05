#include "../include/uart.h"

// Check BCM2835 p. 8 for detail
#define AUX_ENABLES     ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO_REG   ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER_REG  ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR_REG  ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR_REG  ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR_REG  ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR_REG  ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR_REG  ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL_REG ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT_REG ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD_REG ((volatile unsigned int*)(MMIO_BASE+0x00215068))

 /* Set baud rate and characteristics (115200 8N1) and map to GPIO */
void uart_init(void)
{
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
    while(reg--) {
        asm volatile("nop");
    }

    // enable pins 14 and 15
    *GPPUDCLK0 = (1<<14) | (1<<15);

    // Wait 150 cycles 
    // this provides the required hold time for the control signal 
    reg = 150; 
    while(reg--) {
        asm volatile("nop");
    }
    // flush GPIO setup
    *GPPUDCLK0 = 0;

    // Enable the transmitter and receiver.
    *AUX_MU_CNTL_REG = 3;
}

/**
 * Receive a character
 */
char uart_getc(const unsigned int cvt2nl)
{
    char reg;

    /* 
     * wait until something is in the buffer
     * p.15: [0] is the field "symbol available"
     * if this field is set then mini UART receive FIFO 
     * contains at least 1 symbol
     */
    do {
        asm volatile("nop");
    } while(!(*AUX_MU_LSR_REG & 0x01));

    /* read it and return */
    /* [7:0]: receive data */
    reg = (char) (*AUX_MU_IO_REG);

    /* convert carriage return to newline */
    if (cvt2nl) {
        return reg == '\r' ? '\n' : reg;
    } else {
        return reg;
    }
}

/**
 * Send a character
 */
void uart_putc(unsigned int c)
{
    /* wait until we can send */
    do {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR_REG & 0x20));
    
    /* write the character to the buffer */
    *AUX_MU_IO_REG = c;
}


/**
 * Display a string
 */
void uart_puts(const char *s)
{
    while (*s) {
        /* convert newline to carriage return + newline */
        if (*s == '\n') {
            uart_putc('\r');
        }
        uart_putc(*s++);
    }
}


// Convert binary value in hexadecimal
void uart_puthex(unsigned int d)
{
    unsigned int n;
    int c;

    for (c=28; c>=0; c-=4) {
        // get highest tetrad
        n = (d>>c) & 0xF;

        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n += n>9 ? 0x37 : 0x30;
        uart_putc(n);
    }
}