#ifndef CPIO_H
#define CPIO_H

#define CPIO_HEADER_MAGIC           "070701"
#define CPIO_TAIL_FILE              "TRAILER!!!"
#define CPIO_PARSE_SUCCEED          0
#define CPIO_MAGIC_NUMBER_MISMATCH  1
#define CPIO_END_OF_ARCHIVE         2

#define CPIO_DEFAULT_ADDR 0x20000000
// void* CPIO_DEFAULT_ADDR;

struct cpio_newc_header {
    char    c_magic[6];     /* 070701 */
    char    c_ino[8];       /* inode */
    char    c_mode[8];      /* permissions and file types */
    char    c_uid[8];       /* user ID */
    char    c_gid[8];       /* group ID */
    char    c_nlink[8];     /* number of hard links */
    char    c_mtime[8];     /* modification time */
    char    c_filesize[8];
    char    c_devmajor[8];
    char    c_devminor[8];
    char    c_rdevmajor[8];
    char    c_rdevminor[8];
    char    c_namesize[8];
    char    c_check[8];
};

void cpio_ls(void);
void cpio_cat(const char* cat_filename);

#endif