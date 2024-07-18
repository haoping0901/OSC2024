.section ".text.boot"

.global _start
_start: 
    mrs     x0, mpidr_el1
    and     x0, x0, #0xFF
    cbz     x0, core0
hang:
    wfe
    b       hang
core0:
    ldr     x0, =_start
    mov     sp, x0

    // clear bss
    ldr     x0, =_bss
    ldr     w1, =_bss_size

    // initialize .bss section
init_bss:
    cbz     w1, exec_c_prog
    str     xzr, [x0], #8
    sub     w1, w1, #1
    cbnz    w1, init_bss
exec_c_prog:
    bl      main
    b       hang

.end