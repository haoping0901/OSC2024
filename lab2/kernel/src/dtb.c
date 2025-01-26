#include "../include/dtb.h"
#include "../include/uart.h"
#include "../include/utils.h"
#include "../include/cpio.h"

char *dtb_addr;

// #define bswap_32(x) \
//     ((x & 0xFF000000) >> 24) | \
//     ((x & 0x00FF0000) >>  8) | \
//     ((x & 0x0000FF00) <<  8) | \
//     ((x & 0x000000FF) << 24)

static inline uint32_t bswap_32(uint32_t x)
{
    return ((x & 0xFF000000) >> 24) | 
           ((x & 0x00FF0000) >>  8) | 
           ((x & 0x0000FF00) <<  8) | 
           ((x & 0x000000FF) << 24);
}

static inline uint64_t align_32(uint64_t addr)
{
    return addr += (4 - (addr & 0x3)) & 0x3;
}

void dtb_traverse(dtb_callback callback)
{
    struct fdt_header* header = (struct fdt_header*) dtb_addr;

    /* check magic number */
    if (bswap_32(header->magic) != FDT_HEADER_MAGIC) {
        uart_puts("Mismatched magic number in DTB!\n");
        return;
    }

    /* locate the addresses of the structure and string blocks */
    uint32_t struct_size = bswap_32(header->size_dt_struct);
    char* dt_struct = (char*) header + bswap_32(header->off_dt_struct);

    char* dt_strings = (char*) header + bswap_32(header->off_dt_strings);

    /* traverse the dtb */
    char *cur = dt_struct, *struct_end = dt_struct + struct_size;
    while (cur < struct_end) {
        uint32_t token_type = bswap_32(*(uint32_t*) cur);
        cur += 4;   /* token size: 4-byte */

        switch (token_type) {
        case FDT_BEGIN_NODE:
            callback(token_type, cur, 0);
            cur += strlen(cur);

            /* align pointer to 32-bit boundary */
            cur = (char*) align_32(cur);
            break;
        case FDT_END_NODE:
            callback(token_type, 0, 0);
        case FDT_PROP:
            struct fdt_prop* dt_prop = (struct fdt_prop*) cur;
            cur += sizeof(struct fdt_prop);
            uint32_t prop_len = bswap_32(dt_prop->len);
            char* prop_name = dt_strings + bswap_32(dt_prop->nameoff);
            callback(token_type, prop_name, cur);

            cur += prop_len;
            cur = (char*) align_32(cur);

            break;
        case FDT_NOP:
        case FDT_END:
            /* both of these two tokens do nothing */
            continue;

        default:
            break;
        }
    }
}

void dtb_initramfs_callback(uint32_t token_type, const char* prop_name, 
                            const void* value)
{
    if (token_type == FDT_PROP && 
            strcmp(prop_name, "linux,initrd-start") == 0) {
        CPIO_DEFAULT_ADDR = (void*)(uint64_t) bswap_32(*(uint32_t*) value);
    }
}