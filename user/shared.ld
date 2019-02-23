ENTRY(_start)
PHDRS
{
	text PT_LOAD FILEHDR PHDRS;
	data PT_LOAD;
	dynamic PT_DYNAMIC;
}
SECTIONS
{
	. = SIZEOF_HEADERS;
    .text : { *(.text*) } : text
    . = ALIGN(4096);
    .rodata : { *(.rodata*) } : text
    . = ALIGN(4096);
    .data : { *(.data*) } : data
    . = ALIGN(4096);
    .dynamic : { *(.dynamic*) } : data : dynamic
    . = ALIGN(4096);
    .bss : { *(.bss*) *(COMMON) } : data
    . = ALIGN(4096);
    end = .;
}
