ENTRY(_start)

SECTIONS
{
  . = 0x80000;
  .text : 
  { 
    KEEP(*(.text.boot))
    *(.text .text.*)
    *(.rodata)
  }
  
  .data : 
  {
    *(.data)
  }

  .bss : 
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
}

/* 
  Calculate the size of the .bss section in the size of 
  xzr register (64-bit zero register).
*/
_bss_size = (SIZEOF(.bss)) >> 3;