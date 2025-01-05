#include "../include/cpio.h"
#include "../include/utils.h"
#include "../include/uart.h"

/* parse hexadecimal fields to decimal */
static inline unsigned long parse_hex(const char* hex, unsigned int width)
{
    unsigned long idx, dec = 0;

    for (idx=0; idx<width; ++idx) {
        dec *= 16;

        if (hex[idx] >= '0' && hex[idx] <= '9') {
            dec += hex[idx] - '0';
        } else if (hex[idx] >= 'a' && hex[idx] <= 'f') {
            dec += hex[idx] - 'a' + 10;
        } else if (hex[idx] >= 'A' && hex[idx] <= 'F') {
            dec += hex[idx] - 'A' + 10;
        } else {
            return dec;
        }
    }

    return dec;
}

static int parse_header(struct cpio_newc_header** header, 
                        const char** filename, unsigned long* filesize, 
                        const char** data)
{
    /* check magic */
    if (strncmp((*header)->c_magic, CPIO_HEADER_MAGIC, 
                sizeof((*header)->c_magic)) != 0) {
        uart_puts("Mismatched cpio format!\n");
        return CPIO_MAGIC_NUMBER_MISMATCH;
    }

    /* get filename */
    *filename = (char*) *header + sizeof(struct cpio_newc_header);
    
    /* check Trailer case */
    if (strncmp(*filename, CPIO_TAIL_FILE, 10) == 0) {
        return CPIO_END_OF_ARCHIVE;
    }
    
    /* get offset to data */
    unsigned int filename_size = parse_hex((*header)->c_namesize, 8);
    unsigned int offset = filename_size + sizeof(struct cpio_newc_header);

    /* 
     * Align the pointer due to the total size of the fixed header plus
     * pathname is a multiple of four.
     */
    offset = (offset + 3) & ~3;

    /* get the pointer to data */
    *data = (char*) header + offset;

    /* move the pointer of the header to next file */
    /* get filesize */
    *filesize = parse_hex((*header)->c_filesize, 8);

    /* 
     * Likewise, align the pointer due to the file data is padded to a multiple
     * of four bytes.
     */
    offset = (*filesize + 3) & ~0x11;   /* align */
    (*header) = (struct cpio_newc_header*) (*data + offset);

    return CPIO_PARSE_SUCCEED;
}

void cpio_ls(void)
{
    struct cpio_newc_header* header = \
        (struct cpio_newc_header*) CPIO_DEFAULT_ADDR;
    const char *parsed_file, *data;
    unsigned long parsed_filesize = 0;

    while (header) {
        int parsing_status = parse_header(&header, &parsed_file, 
                                          &parsed_filesize, &data);
        
        if (parsing_status != CPIO_PARSE_SUCCEED) {
            break;
        }
        
        uart_puts(parsed_file);
        uart_puts("\n");
    }
}

void cpio_cat(const char* cat_file)
{
    struct cpio_newc_header* header = \
        (struct cpio_newc_header*) CPIO_DEFAULT_ADDR;
    const char *parsed_file, *data;
    unsigned long parsed_filesize = 0;

    while (header) {
        int parsing_status = parse_header(&header, &parsed_file, 
                                          &parsed_filesize, &data);
        
        if (parsing_status != CPIO_PARSE_SUCCEED) {
            break;
        }
        
        if (strcmp(cat_file, parsed_file) == 0) {
            while (parsed_filesize--) {
                if (*data == '\n') {
                    uart_putc('\r');
                }
                uart_putc(*data++);
            }
            uart_puts("\n");
            
            break;
        }
    }
}