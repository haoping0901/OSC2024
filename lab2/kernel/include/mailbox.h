#include "gpio.h"

#ifndef MAILBOX_H
#define MAILBOX_H

extern volatile unsigned int mailbox[32];

// channels 
// https://github.com/raspberrypi/firmware/wiki/Mailboxes
#define MAILBOX_CH_POWER   0
#define MAILBOX_CH_FB      1
#define MAILBOX_CH_VUART   2
#define MAILBOX_CH_VCHIQ   3
#define MAILBOX_CH_LEDS    4
#define MAILBOX_CH_BTNS    5
#define MAILBOX_CH_TOUCH   6
#define MAILBOX_CH_COUNT   7
#define MAILBOX_CH_PROP    8

// buffer content
#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001

// tags (ARM to VC)
// Ref: https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
#define GET_BOARD_REVISION  0x00010002
#define GET_ARM_MEMORY      0x00010005

// tag format
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000

void get_sys_info();

#endif