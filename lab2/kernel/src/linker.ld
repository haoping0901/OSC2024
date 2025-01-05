ENTRY(_start)

/* Set the available size for heap and stack. */
_heap_stack_size = 256K;

SECTIONS
{
  . = 0x80000;
  .text : 
  { 
    KEEP(*(.text.boot))
    *(.text .text.*)
    *(.rodata)
  }
  
  /* section for initialized global/static variables */
  .data : 
  {
    *(.data)
  }

  .bss (NOLOAD) : 
  {
    /* 
      To easily use the `str` instruction to initialize all data 
      in .bss section to 0, we align the section to an address that 
      is a multiple of 8. 
    */
    . = ALIGN(8);
    _bss = .;
    *(.bss)
    *(COMMON)
    _bss_end = .;
  }

  .heap :
  {
    . = ALIGN(8);
    _heap = .;
  }
  . += _heap_stack_size;
  
  .stack :
  {
    _stack = .;
  }
}

/* 
  Calculate the size of the .bss section in the size of 
  xzr register (64-bit zero register).
*/
_bss_size = (SIZEOF(.bss)) >> 3;