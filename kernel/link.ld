ENTRY(_start);
SECTIONS
{
    KERNEL_BASE = 0xC0000000;

    . = KERNEL_BASE + 0x00110000;
    . = ALIGN(4096);

    _text_start = .;
    .text : AT(ADDR(.text) - KERNEL_BASE) { *(.text*) }
    _text_end = .;

    . = ALIGN(4096);
    _data_start = .;
    .data : AT(ADDR(.data) - KERNEL_BASE) { *(.data*) }
    _data_end = .;

    . = ALIGN(4096);
    _rodata_start = .;
    .rodata : AT(ADDR(.rodata) - KERNEL_BASE) { *(.rodata*) }
    _rodata_end = .;

    . = ALIGN(4096);
    _eh_start = .;
    .eh_frame : AT(ADDR(.eh_frame) - KERNEL_BASE) { *(.eh_frame*) }
    _eh_end = .;

    . = ALIGN(4096);
    __init_array_start = .;
    .init_array : AT(ADDR(.init_array) - KERNEL_BASE) { *(.init_array*) }
    __init_array_end = .;

    . = ALIGN(4096);
    __fini_array_start = .;
    .fini_array : AT(ADDR(.fini_array) - KERNEL_BASE) { *(.fini_array*) }
    __fini_array_end = .;

    . = ALIGN(4096);
    _utext_start = .;
    .utext : AT(ADDR(.utext) - KERNEL_BASE) { *(.utext*) }
    _utext_end = ALIGN(4096);

    . = ALIGN(4096);
    _udata_start = .;
    .udata : AT(ADDR(.udata) - KERNEL_BASE) { *(.udata*) }
    _udata_end = ALIGN(4096);

    . = ALIGN(4096);
    _ubss_start = .;
    .ubss : AT(ADDR(.ubss) - KERNEL_BASE) { *(.ubss*) }
    _ubss_end = ALIGN(4096);

    . = ALIGN(4096);
    _bss_start = .;
    .bss : AT(ADDR(.bss) - KERNEL_BASE) { *(.bss) *(COMMON) }
    . = ALIGN(4096);
    _bss_end = .;
    _end = .;
}
