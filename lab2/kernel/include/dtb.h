#ifndef DTH_H
#define DTH_H

typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
extern char *dtb_addr;

#define FDT_HEADER_MAGIC 0xD00DFEED
/* 
    Ref. https://github.com/devicetree-org/devicetree-specification/blob/main/
         source/chapter5-flattened-format.rst#id9 
 */
struct fdt_header {
    uint32_t magic;             /* shall contain the value 0xd00dfeed 
                                   (big-endian) */
    uint32_t totalsize;         /* shall contain the total size in bytes of 
                                   the devicetree data structure */
    uint32_t off_dt_struct;     /* shall contain the offset in bytes of the 
                                   structure block */
    uint32_t off_dt_strings;    /* shall contain the offset in bytes of the 
                                   strings block */
    uint32_t off_mem_rsvmap;    /* hall contain the offset in bytes of the 
                                   memory reservation block */
    uint32_t version;           /* shall contain the version of the devicetree 
                                   data structure */
    uint32_t last_comp_version; /* shall contain the lowest version of the 
                                   devicetree data structure with which the 
                                   version used is backwards compatible */
    uint32_t boot_cpuid_phys;   /* shall contain the physical ID of the 
                                   system’s boot CPU */
    uint32_t size_dt_strings;   /* shall contain the length in bytes of the 
                                   strings block section of the devicetree 
                                   blob */
    uint32_t size_dt_struct;    /* shall contain the length in bytes of the 
                                   structure block section of the devicetree 
                                   blob */
};

/* info. of structure block */
#define FDT_BEGIN_NODE  0x00000001  /* the beginning of a node’s 
                                       representation */
#define FDT_END_NODE    0x00000002  /* the beginning of a node’s 
                                       representation */
#define FDT_PROP        0x00000003  /* the beginning of the representation of 
                                       one property in the devicetree */
struct fdt_prop {
    /* Both the fields in this structure are 32-bit big-endian integers. */
    uint32_t len;
    uint32_t nameoff;
};

#define FDT_NOP         0x00000004  /* the deleted node */
#define FDT_END         0x00000009  /* the end of the structure block */


typedef void (*dtb_callback)(uint32_t token_type, const char* name, 
                             const void* value);
void dtb_traverse(dtb_callback callback);

void dtb_initramfs_callback(uint32_t token_type, const char* name, 
                            const void* value);

#endif