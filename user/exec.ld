ENTRY(_start)
PHDRS
{
	text PT_LOAD FILEHDR PHDRS;
	data PT_LOAD;
	dynamic PT_DYNAMIC;
}
SECTIONS
{
	PAGE_SIZE = 4096;
	
	. = 0x08000000;
    . += SIZEOF_HEADERS;
    . = ALIGN(PAGE_SIZE);
    .text : { *(.text*) } : text
    .rodata : { *(.rodata*) } : text
    . = ALIGN(PAGE_SIZE);
    .data : { *(.data*) } : data
    .dynamic : { *(.dynamic*) } : data : dynamic
    .bss : { *(.bss*) *(COMMON) } : data
    end = .;
}
