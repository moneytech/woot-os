[bits 16]

global __V86CodeStart
__V86CodeStart:

; FIXME: uses self modifying code
global __Int86
__Int86:
    push cs
    pop ds

    ; modify the int n instruction
    mov byte [__Int86.doInt - __V86CodeStart + 1], al

    ; load registers
    ; layout if these registers MUST match
    ; V86Regs structure in v86.h
    push dword [0x801C] ; ds
    mov eax, dword [0x8020]
    mov es, ax
    mov eax, dword [0x8024]
    mov fs, ax
    mov eax, dword [0x8028]
    mov gs, ax
    mov eax, dword [0x8000]
    mov ebx, dword [0x8004]
    mov ecx, dword [0x8008]
    mov edx, dword [0x800C]
    mov esi, dword [0x8010]
    mov edi, dword [0x8014]
    mov ebp, dword [0x8018]
    pop ds
    ; do the int n instruction
.doInt:
    int 0xFE	; dummy int instruction
            ; will be overwritten
    ; store registers
    push ds
    push cs
    pop ds
    mov ebp, dword [0x8018]
    mov edi, dword [0x8014]
    mov esi, dword [0x8010]
    mov edx, dword [0x800C]
    mov ecx, dword [0x8008]
    mov ebx, dword [0x8004]
    mov eax, dword [0x8000]
    xor eax, eax
    mov ax, gs
    mov dword [0x8028], eax
    mov ax, fs
    mov dword [0x8024], eax
    mov ax, es
    mov dword [0x8020], eax
    pop dword [0x801C] ; ds

    ; return to kernel
    int 0xFE

global __V86CodeEnd
__V86CodeEnd:

[bits 32]
global v86EnterInt
v86EnterInt:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    mov eax, [ebp + 8]
    mov ebx, [ebp + 12]
    mov ecx, [ebp + 16]
    mov edx, [ebp + 20]
    mov esi, [ebp + 24]
    mov edi, [ebp + 28]
    int 0xFD ; MUST match interrupt number registered in vm86.c
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret

global v86Enter
v86Enter:
    mov ebp, esp
    push 0                ; GS
    push 0                ; FS
    push 0                ; DS
    push 0                ; ES
    push dword [ebp + 4]  ; SS
    push dword [ebp + 8]  ; ESP
    pushfd                ; EFLAGS
    or dword [esp], (1 << 17)
    push dword [ebp + 12] ; CS
    push dword [ebp + 16] ; EIP
    mov eax, dword [ebp + 20]
    ;  hlt
    iretd
