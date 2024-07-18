#include "../include/mailbox.h"
#include "../include/uart.h"

// mailbox address and flags
#define VIDEOCORE_MBOX  (MMIO_BASE + 0xB880)

#define MAILBOX_READ    ((volatile unsigned int*) (VIDEOCORE_MBOX + 0x0))
#define MAILBOX_POLL    ((volatile unsigned int*) (VIDEOCORE_MBOX + 0x10))
#define MAILBOX_SENDER  ((volatile unsigned int*) (VIDEOCORE_MBOX + 0x14))
#define MAILBOX_STATUS  ((volatile unsigned int*) (VIDEOCORE_MBOX + 0x18))
#define MAILBOX_CONFIG  ((volatile unsigned int*) (VIDEOCORE_MBOX + 0x1C))
#define MAILBOX_WRITE   ((volatile unsigned int*) (VIDEOCORE_MBOX + 0x20))

#define MAILBOX_FULL    0x80000000
#define MAILBOX_EMPTY   0x40000000

// The buffer itself is 16-byte aligned as only the upper 28 bits of the address can be passed via the mailbox.
volatile unsigned int __attribute__((aligned(16))) mailbox[32];

int mailbox_call(unsigned char channel) {
    // 1. Combine the message address (upper 28 bits) with 
    // channel number (lower 4 bits)
    // The mailbox interface has 28 bits (MSB) available for the value(message address) and 4 bits (LSB) for the channel
    // First take the 64bits address of mailbox(is probably just to ensure it can fits), then take the lower 32bits
    // clear LSB 4 bits and fill with channel number.
    unsigned int mb_addr = ((unsigned int) ((unsigned long) &mailbox) & ~0xF) \
                            | (channel & 0xF);

    // 2. Check if Mailbox 0 status register’s full flag is set.
    do asm volatile("nop");
    while (*MAILBOX_STATUS & MAILBOX_FULL);

    // 3. If not, then you can write to Mailbox 1 Read/Write register.
    // write our address containing message to mailbox address
    *MAILBOX_WRITE = mb_addr;
    
    // Wait for response
    while(1) {
        // 4. Check if Mailbox 0 status register’s empty flag is set.
        do asm volatile("nop");
        while (*MAILBOX_STATUS & MAILBOX_EMPTY);

        // 5. If not, then you can read from Mailbox 0 Read/Write register.
        // 6. Check if the value is the same as you wrote in step 1.
        if (mb_addr == *MAILBOX_READ) 
            // if the response is successed
            return mailbox[1] == REQUEST_SUCCEED;
    }

    return 0;
}

void print_sys_info(unsigned int tag, const int buf_size) {
    mailbox[0] = (6 + (buf_size>>2)) << 2; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = tag; // tag identifier
    // maximum of request and response value buffer's length.
    mailbox[3] = buf_size;
    mailbox[4] = TAG_REQUEST_CODE;
    // value buffer
    mailbox[5] = 0;
    mailbox[6] = 0;
    // tags end
    mailbox[7] = END_TAG;

    // message passing procedure call
    if (mailbox_call(MAILBOX_CH_PROP)) {
        switch (tag) {
        case GET_BOARD_REVISION:
            // it should be 0xa020d3 for rpi3 b+
            uart_puts("Board revision: ");
            uart_puthex(mailbox[5]);
            uart_puts("\n");
            break;
        case GET_ARM_MEMORY:
            uart_puts("ARM memory base address: 0x");
            uart_puthex(mailbox[5]);
            uart_puts("\nARM memory size: 0x");
            uart_puthex(mailbox[6]);
            uart_puts("\n");
            break;
        default:
            uart_puts("default message1\n");
            break;
        }
    } else {
        switch (tag)
        {
        case GET_BOARD_REVISION:
            uart_puts("Get borad revision failed.\n");
            break;
        case GET_ARM_MEMORY:
            uart_puts("Get arm memory failed.\n");
            break;
        default:
            uart_puts("default message2\n");
            break;
        }
    }

    return;
}

void get_sys_info() {
    print_sys_info(GET_BOARD_REVISION, 4);
    print_sys_info(GET_ARM_MEMORY, 8);

    return;
}
