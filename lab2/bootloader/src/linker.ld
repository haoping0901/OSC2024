ENTRY(_start)

SECTIONS
{
  /* Address to load our kernel. */
  . = 0x60000;
  _relocate_addr = .;

  . = 0x80000;
  _code_start = .;
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
  _code_end = .;
}

/* 
  Calculate the size of the .bss section in the size of 
  xzr register (64-bit zero register).
*/
_bss_size = (SIZEOF(.bss)) >> 3;
_code_size = _code_end - _code_start;