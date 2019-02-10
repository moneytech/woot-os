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

%define SYSCALL_INT_VECTOR  128
%define ISR_STUB_SIZE       16  ; this MUST match the size of ISR_ERRCODE and
                                ; ISR_NOERRCODE macro expansions in isrs.asm

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
extern initializePaging
extern initializeHeap
extern initializeDebugStream
extern isr0
extern __init_array_start
extern __init_array_end
extern __fini_array_start
extern __fini_array_end
extern __cxa_finalize
global _start
_start:
; save multiboot magic value and info pointer
    mov [mbootMagic - KERNEL_BASE], eax
    add ebx, KERNEL_BASE    ; adjust multiboot info pointer
    mov [mbootInfoPtr - KERNEL_BASE], ebx

; enable PSE
.enable_pse:
    mov eax, cr4
    or eax, 0x10
    mov cr4, eax

; make sure page directory is zeroed
.zero_page_dir:
    mov ecx, PAGE_SIZE / 4
    mov edi, bootPageDir - KERNEL_BASE
    xor eax, eax
    rep stosd

; identity map memory below kernel base
.map_identity:
    mov ecx, KERNEL_BASE >> LARGE_PAGE_SHIFT
    mov edi, bootPageDir - KERNEL_BASE
    mov eax, 0x83
.next_id_pde:
    stosd
    add eax, LARGE_PAGE_SIZE
    loop .next_id_pde

; map kernel space
.map_kernel:
    mov ecx, KERNEL_PAGE_COUNT
    mov edi, bootPageDir + KERNEL_PAGE_NUMBER * 4 - KERNEL_BASE
    mov eax, 0x83
.next_kernel_pde:
    stosd
    add eax, LARGE_PAGE_SIZE
    loop .next_kernel_pde

; enable paging
.enable_paging:
    mov eax, bootPageDir - KERNEL_BASE
    mov cr3, eax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

; jump to higher half
    mov eax, .starthi
    jmp eax
.starthi:

; unmap memory below kernel base
.unmap_low:
    mov ecx, KERNEL_BASE >> LARGE_PAGE_SHIFT
    mov edi, bootPageDir - KERNEL_BASE
    xor eax, eax
    rep stosd
    mov eax, cr3    ; invalidate TLB
    mov cr3, eax

; set up known stack
.setup_stack:
    mov esp, kernelStack + STACK_SIZE

; set up known GDT
.setup_gdt:
    lgdt [gdtDescr]
    jmp 0x0008:.fixCS
.fixCS:
    mov ax, 0x0010
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

; set up TSS
    mov eax, tss
    mov edi, gdt + 0x2A
    stosw
    shr eax, 16
    stosb
    inc edi
    inc edi
    mov [edi], ah
    mov ax, 0x002B
    ltr ax

; set up IDT
.setup_idt:
    mov ecx, 256
    mov edi, idt
    mov edx, isr0
.next_idt_entry
    mov eax, edx
    stosw           ; offset 15..0
    mov ax, 0x0008
    stosw           ; 32 bit kernel code selector
    mov al, 0
    stosb           ; zero
    mov al, 0x8E
    stosb           ; type and attributes
    shr eax, 16
    stosw           ; offset 31..16
    add edx, ISR_STUB_SIZE
    loop .next_idt_entry
.setup_syscall_idt:
    mov edi, idt
    mov byte [edi + 8 * SYSCALL_INT_VECTOR + 5], 0xEE   ; syscall int vector must be accessible from ring 3
.load_idt:
    lidt [idtDescr]

; initialize paging properly
.init_paging:
    push dword [mbootInfoPtr]
    call initializePaging
    add esp, 4

; initialize kernel heap
.init_kernel_heap:
    call initializeHeap

; call static constructors
.static_constructors:
    mov edx, __init_array_start
    mov ecx, __init_array_end
.next_constructor:
    cmp edx, ecx
    je .constructors_done
    push ecx
    push edx
    call [edx]
    pop edx
    pop ecx
    add edx, 4
    jmp .next_constructor
.constructors_done:

; initialize debug stream
.init_debug_stream:
    call initializeDebugStream

; call kmain
.call_kmain:
    xor ebp, ebp
    push ebp
    mov ebp, esp
    push dword [mbootInfoPtr]
    push dword [mbootMagic]
    call kmain
    add esp, 8
    leave

; call static destructor
.static_descructors:
    push eax    ; we want return value from kmain to be put in eax register
                ; after system halt so we save it on the stack
    push 0
    call __cxa_finalize
    add esp, 4

    mov edx, __fini_array_start
    mov ecx, __fini_array_end
.next_destructor:
    cmp edx, ecx
    je .destructors_done
    push ecx
    push edx
    call [edx]
    pop edx
    pop ecx
    add edx, 4
    jmp .next_destructor
.destructors_done:
    pop eax
    cli
    hlt

segment .data
align PAGE_SIZE
gdt:
    dq 0x0000000000000000   ; null 0x0000
    dq 0x00CF9A000000FFFF   ; kernel code 0x0008
    dq 0x00CF92000000FFFF   ; kernel data 0x0010
    dq 0x00CFFA000000FFFF   ; user code 0x0018
    dq 0x00CFF2000000FFFF   ; user data 0x0020
    dq 0x0040890000000067   ; tss 0x0028
.end

gdtDescr:
    dw gdt.end - gdt - 1
    dd gdt

idtDescr:
    dw idt.end - idt - 1
    dd idt

segment .bss

align STACK_SIZE
kernelStack:
    resb STACK_SIZE

align PAGE_SIZE
bootPageDir:
    resb PAGE_SIZE

align PAGE_SIZE
tss:
    resb PAGE_SIZE

align PAGE_SIZE
idt:
    resq 256
.end

align PAGE_SIZE
mbootMagic:
    resd 1
mbootInfoPtr:
    resd 1
