.section ".text.boot"

.global _start
_start:
    mrs     x1, mpidr_el1
    and     x1, x1, #0xFF
    cbz     x1, core0
hang:
    wfe
    b       hang
core0:
setup_stack:
    mov     x10, x0
    ldr     x1, =_start
    mov     sp, x1
load_bss:
    ldr     x1, =_bss
    ldr     w2, =_bss_size

init_bss:
    cbz     w2, exec_c_prog
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, init_bss
exec_c_prog:
    bl      main
    b       hang

.end