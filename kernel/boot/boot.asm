[bits 32]

%define PAGE_SHIFT          12
%define LARGE_PAGE_SHIFT    22
%define STACK_SIZE          65536
%define KERNEL_BASE         0xC0000000
%define KERNEL_SPACE_SIZE   0x10000000

%define PAGE_SIZE           (1 << PAGE_SHIFT)
%define LARGE_PAGE_SIZE     (1 << LARGE_PAGE_SHIFT)
%define KERNEL_PAGE_NUMBER  (KERNEL_BASE >> LARGE_PAGE_SHIFT)
%define KERNEL_PAGE_COUNT   (KERNEL_SPACE_SIZE >> LARGE_PAGE_SHIFT)

%define MULTIBOOT_MAGIC 0x1BADB002
%define MULTIBOOT_FLAGS 0x00000007
%define VIDEO_USE_TEXT  1
%define VIDEO_WIDTH     80
%define VIDEO_HEIGHT    25
%define VIDEO_BPP       0

segment .text
align 4
_multiboot_header:
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)
    dd 0, 0, 0, 0, 0
    dd VIDEO_USE_TEXT, VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_BPP

extern kmain
global _start
_start:
; save multiboot magin value and info pointer
    mov [mbootMagic], eax
    mov [mbootInfoPtr], ebx

; set up know GDT
    lgdt [gdtDescr]
    jmp 0x0008:.fixCS
.fixCS:
    mov ax, 0x0010
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ax, 0x002B
    ltr ax

; set up known stack
    mov esp, kernelStack + STACK_SIZE

; enable PSE
    mov eax, cr4
    or eax, 0x10
    mov cr4, eax

; identity map whole address space (using large pages)
    mov ecx, 1 << PAGE_SHIFT
    mov edi, bootPageDir
    mov eax, 0x83
.next_pde:
    stosd
    add eax, 1 << LARGE_PAGE_SHIFT
    loop .next_pde

; enable paging
    mov eax, bootPageDir
    mov cr3, eax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

; call kmain
    xor ebp, ebp
    push ebp
    mov ebp, esp
    push dword [mbootInfoPtr]
    push dword [mbootMagic]
    call kmain
    add esp, 8
    leave
    cli
    hlt

segment .data
align PAGE_SIZE
gdt:
    dq 0x0000000000000000 ; null 0x0000
    dq 0x00CF9A000000FFFF ; kernel code 0x0008
    dq 0x00CF92000000FFFF ; kernel data 0x0010
    dq 0x00CFFA000000FFFF ; user code 0x0018
    dq 0x00CFF2000000FFFF ; user data 0x0020
    dq 0x0040890000000067 ; tss 0x0028
.end

gdtDescr:
    dw gdt.end - gdt - 1
    dd gdt

segment .bss

align STACK_SIZE
kernelStack:
    resb STACK_SIZE

align PAGE_SIZE
bootPageDir:
    resb PAGE_SIZE

align PAGE_SIZE
mbootMagic:
    resd 1
mbootInfoPtr:
    resd 1
