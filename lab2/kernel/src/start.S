.section ".text.boot"

.global _start

// .extern dtb_addr

_start: 
    mrs     x1, mpidr_el1
    and     x1, x1, #0xFF
    cbz     x1, core0
hang:
    wfe
    b       hang
core0:
setup_stack:
    ldr     x1, =_start
    mov     sp, x1
load_bss:
    ldr     x1, =_bss
    ldr     w2, =_bss_size
init_bss:
    cbz     w2, set_dtb_addr
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, init_bss
set_dtb_addr:
    ldr     x1, =dtb_addr
    str     x0, [x1]
exec_c_prog:
    bl      main
    b       hang

// .section ".data"

// .global _dtb_addr
// _dtb_addr:
//     .quad 0x0

// .end