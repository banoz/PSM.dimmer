ENTRY(_start)

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 16K
  RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 2K
}

PROVIDE(_eusrstack = ORIGIN(RAM) + LENGTH(RAM));

SECTIONS
{
  .init :
  {
    KEEP(*(SORT_NONE(.init)))
  } > FLASH

  .vector :
  {
    KEEP(*(.vector))
  } > FLASH

  .text :
  {
    *(.text .text.*)
    *(.rodata .rodata.*)
    *(.srodata .srodata.*)
    KEEP(*(.eh_frame*))
  } > FLASH

  .preinit_array :
  {
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN(__preinit_array_end = .);
  } > FLASH

  .init_array :
  {
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
    KEEP(*(.init_array EXCLUDE_FILE(*crtbegin*.o *crtend*.o) .ctors))
    PROVIDE_HIDDEN(__init_array_end = .);
  } > FLASH

  .fini_array :
  {
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
    KEEP(*(.fini_array EXCLUDE_FILE(*crtbegin*.o *crtend*.o) .dtors))
    PROVIDE_HIDDEN(__fini_array_end = .);
  } > FLASH

  . = ALIGN(4);
  _etext = .;

  .highcode : AT(_etext)
  {
    . = ALIGN(4);
    _highcode_vma_start = .;
    *(.highcode .highcode.*)
    . = ALIGN(4);
    _highcode_vma_end = .;
  } > RAM

  _highcode_lma = LOADADDR(.highcode);

  .data : AT(_highcode_lma + SIZEOF(.highcode))
  {
    . = ALIGN(4);
    _data_vma = .;
    _data = .;
    PROVIDE(__global_pointer$ = . + 0x800);
    *(.data .data.*)
    *(.sdata .sdata.*)
    . = ALIGN(4);
    _edata = .;
  } > RAM

  _data_lma = LOADADDR(.data);

  .bss (NOLOAD) :
  {
    . = ALIGN(4);
    _sbss = .;
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } > RAM

  . = ALIGN(8);
  _end = .;
  PROVIDE(end = .);
}
