ENTRY(_module_start)
PHDRS
{
    all PT_LOAD FILEHDR PHDRS;
}
SECTIONS
{
    . = ALIGN(4096);
    _module_text_start = .;
    .text : { *(.text*) } : all
    _module_text_end = .;

    . = ALIGN(4096);
    _module_data_start = .;
    .data : { *(.data*) }
    _module_data_end = .;

    . = ALIGN(4096);
    _module_rodata_start = .;
    .rodata : { *(.rodata*) }
    _module_rodata_end = .;

    . = ALIGN(4096);
    _module_eh_start = .;
    .eh_frame : { *(.eh_frame*) }
    _module_eh_end = .;

    . = ALIGN(4096);
    _module__init_array_start = .;
    .init_array : { *(.init_array*) }
    _module__init_array_end = .;

    . = ALIGN(4096);
    _module__fini_array_start = .;
    .fini_array : { *(.fini_array*) }
    _module__fini_array_end = .;

    . = ALIGN(4096);
    _module_utext_start = .;
    .utext : { *(.utext*) }
    _module_utext_end = ALIGN(4096);

    . = ALIGN(4096);
    _module_udata_start = .;
    .udata : { *(.udata*) }
    _module_udata_end = ALIGN(4096);

    . = ALIGN(4096);
    _module_ubss_start = .;
    .ubss : { *(.ubss*) }
    _module_ubss_end = ALIGN(4096);

    . = ALIGN(4096);
    _module_bss_start = .;
    .bss : { *(.bss) *(COMMON) }
    . = ALIGN(4096);
    _module_bss_end = .;
    _module_end = .;
}
