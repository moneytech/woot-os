[bits 32]

segment .text

; segments must match ones in sysdefs.h
SEG_CODE32_USER equ 0x001B
SEG_DATA32_USER equ 0x0023

global cpuEnableInterrupts
cpuEnableInterrupts:
    sti
    ret

global cpuDisableInterrupts
cpuDisableInterrupts:
    pushfd
    cli
    pop eax
    shr eax, 9
    and eax, 1
    ret

global cpuAreInterruptsEnabled
cpuAreInterruptsEnabled:
    pushfd
    pop eax
    shr eax, 9
    and eax, 1
    ret

global cpuRestoreInterrupts
cpuRestoreInterrupts:
    mov eax, [esp + 4]
    or eax, eax
    jz .disable
    sti
    ret
.disable:
    cli
    ret

global cpuSystemHalt
cpuSystemHalt:
    mov eax, [esp + 4]
    cli
    hlt

global cpuWaitForInterrupt
cpuWaitForInterrupt:
    mov eax, [esp + 4]
    hlt
    ret

global cpuGetCR0
cpuGetCR0:
    mov eax, cr0
    ret

global cpuSetCR0
cpuSetCR0:
    mov eax, [esp + 4]
    mov cr0, eax
    ret

global cpuGetCR2
cpuGetCR2:
    mov eax, cr2
    ret

global cpuGetCR3
cpuGetCR3:
    mov eax, cr3
    ret

global cpuSetCR3
cpuSetCR3:
    mov eax, [esp + 4]
    mov cr3, eax
    ret

global cpuInvalidatePage
cpuInvalidatePage:
    mov eax, [esp + 4]
    invlpg [eax]
    ret

global cpuGetEIP
cpuGetEIP:
    pop eax
    jmp eax

global cpuGetESP
cpuGetESP:
    mov eax, esp
    add eax, 4
    ret

global cpuGetEBP
cpuGetEBP:
    mov eax, ebp
    ret

global cpuLGDT
cpuLGDT:
    mov eax, [esp + 4]
    lgdt [eax]
    ret

global cpuSGDT
cpuSGDT:
    mov eax, [esp + 4]
    sgdt [eax]
    ret

global cpuLIDT
cpuLIDT:
    mov eax, [esp + 4]
    lidt [eax]
    ret

global cpuSIDT
cpuSIDT:
    mov eax, [esp + 4]
    sidt [eax]
    ret

global cpuLTR
cpuLTR:
    mov eax, [esp + 4]
    ltr ax
    ret

global cpuSTR
cpuSTR:
    str ax
    ret

global cpuFixSegments
cpuFixSegments:
    mov eax, 0x00000010 ; SEG_DATA32_KERNEL
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    jmp 0x0008:.fixCS ; SEG_CODE32_KERNEL
.fixCS:
    ret

global _inb
_inb:
    mov edx, [esp + 4]
    xor eax, eax
    in al, dx
    ret

global _inw
_inw:
    mov edx, [esp + 4]
    xor eax, eax
    in ax, dx
    ret

global _inl
global _ind
_inl:
_ind:
    mov edx, [esp + 4]
    in eax, dx
    ret

global _outb
_outb:
    mov edx, [esp + 4]
    mov eax, [esp + 8]
    out dx, al
    ret

global _outw
_outw:
    mov edx, [esp + 4]
    mov eax, [esp + 8]
    out dx, ax
    ret

global _outl
global _outd
_outl:
_outd:
    mov edx, [esp + 4]
    mov eax, [esp + 8]
    out dx, eax
    ret

global _insb
_insb:
    cld
    push edi
    mov edi, [esp + 8]
    mov edx, [esp + 12]
    mov ecx, [esp + 16]
    rep insb
    pop edi
    ret

global _insw
_insw:
    cld
    push edi
    mov edi, [esp + 8]
    mov edx, [esp + 12]
    mov ecx, [esp + 16]
    rep insw
    pop edi
    ret

global _insl
global _insd
_insl:
_insd:
    cld
    push edi
    mov edi, [esp + 8]
    mov edx, [esp + 12]
    mov ecx, [esp + 16]
    rep insd
    pop edi
    ret

global _outsb
_outsb:
    cld
    push esi
    mov esi, [esp + 8]
    mov edx, [esp + 12]
    mov ecx, [esp + 16]
    rep outsb
    pop esi
    ret

global _outsw
_outsw:
    cld
    push esi
    mov esi, [esp + 8]
    mov edx, [esp + 12]
    mov ecx, [esp + 16]
    rep outsw
    pop esi
    ret

global _outsl
global _outsd
_outsl:
_outsd:
    cld
    push esi
    mov esi, [esp + 8]
    mov edx, [esp + 12]
    mov ecx, [esp + 16]
    rep outsd
    pop esi
    ret

global cpuIOSetBitsB
cpuIOSetBitsB:
    mov dx, [esp + 4]
    in al, dx
    or al, [esp + 8]
    out dx, al
    ret

global cpuIOSetBitsW
cpuIOSetBitsW:
    mov dx, [esp + 4]
    in ax, dx
    or ax, [esp + 8]
    out dx, ax
    ret

global cpuIOSetBitsL
global cpuIOSetBitsD
cpuIOSetBitsL:
cpuIOSetBitsD:
    mov dx, [esp + 4]
    in eax, dx
    or eax, [esp + 8]
    out dx, eax
    ret

global cpuIOClrBitsB
cpuIOClrBitsB:
    mov dx, [esp + 4]
    in al, dx
    mov ecx, [esp + 8]
    not ecx
    and al, cl
    out dx, al
    ret

global cpuIOClrBitsW
cpuIOClrBitsW:
    mov dx, [esp + 4]
    in ax, dx
    mov ecx, [esp + 8]
    not ecx
    and ax, cx
    out dx, ax
    ret

global cpuIOClrBitsL
global cpuIOClrBitsD
cpuIOClrBitsL:
cpuIOClrBitsD:
    mov dx, [esp + 4]
    in eax, dx
    mov ecx, [esp + 8]
    not ecx
    and eax, ecx
    out dx, eax
    ret

_INT00:
    int 0x00
    ret

_INT01:
    int 0x01
    ret

_INT02:
    int 0x02
    ret

_INT03:
    int 0x03
    ret

_INT04:
    int 0x04
    ret

_INT05:
    int 0x05
    ret

_INT06:
    int 0x06
    ret

_INT07:
    int 0x07
    ret

_INT08:
    int 0x08
    ret

_INT09:
    int 0x09
    ret

_INT0A:
    int 0x0A
    ret

_INT0B:
    int 0x0B
    ret

_INT0C:
    int 0x0C
    ret

_INT0D:
    int 0x0D
    ret

_INT0E:
    int 0x0E
    ret

_INT0F:
    int 0x0F
    ret

_INT10:
    int 0x10
    ret

_INT11:
    int 0x11
    ret

_INT12:
    int 0x12
    ret

_INT13:
    int 0x13
    ret

_INT14:
    int 0x14
    ret

_INT15:
    int 0x15
    ret

_INT16:
    int 0x16
    ret

_INT17:
    int 0x17
    ret

_INT18:
    int 0x18
    ret

_INT19:
    int 0x19
    ret

_INT1A:
    int 0x1A
    ret

_INT1B:
    int 0x1B
    ret

_INT1C:
    int 0x1C
    ret

_INT1D:
    int 0x1D
    ret

_INT1E:
    int 0x1E
    ret

_INT1F:
    int 0x1F
    ret

_INT20:
    int 0x20
    ret

_INT21:
    int 0x21
    ret

_INT22:
    int 0x22
    ret

_INT23:
    int 0x23
    ret

_INT24:
    int 0x24
    ret

_INT25:
    int 0x25
    ret

_INT26:
    int 0x26
    ret

_INT27:
    int 0x27
    ret

_INT28:
    int 0x28
    ret

_INT29:
    int 0x29
    ret

_INT2A:
    int 0x2A
    ret

_INT2B:
    int 0x2B
    ret

_INT2C:
    int 0x2C
    ret

_INT2D:
    int 0x2D
    ret

_INT2E:
    int 0x2E
    ret

_INT2F:
    int 0x2F
    ret

_INT30:
    int 0x30
    ret

_INT31:
    int 0x31
    ret

_INT32:
    int 0x32
    ret

_INT33:
    int 0x33
    ret

_INT34:
    int 0x34
    ret

_INT35:
    int 0x35
    ret

_INT36:
    int 0x36
    ret

_INT37:
    int 0x37
    ret

_INT38:
    int 0x38
    ret

_INT39:
    int 0x39
    ret

_INT3A:
    int 0x3A
    ret

_INT3B:
    int 0x3B
    ret

_INT3C:
    int 0x3C
    ret

_INT3D:
    int 0x3D
    ret

_INT3E:
    int 0x3E
    ret

_INT3F:
    int 0x3F
    ret

_INT40:
    int 0x40
    ret

_INT41:
    int 0x41
    ret

_INT42:
    int 0x42
    ret

_INT43:
    int 0x43
    ret

_INT44:
    int 0x44
    ret

_INT45:
    int 0x45
    ret

_INT46:
    int 0x46
    ret

_INT47:
    int 0x47
    ret

_INT48:
    int 0x48
    ret

_INT49:
    int 0x49
    ret

_INT4A:
    int 0x4A
    ret

_INT4B:
    int 0x4B
    ret

_INT4C:
    int 0x4C
    ret

_INT4D:
    int 0x4D
    ret

_INT4E:
    int 0x4E
    ret

_INT4F:
    int 0x4F
    ret

_INT50:
    int 0x50
    ret

_INT51:
    int 0x51
    ret

_INT52:
    int 0x52
    ret

_INT53:
    int 0x53
    ret

_INT54:
    int 0x54
    ret

_INT55:
    int 0x55
    ret

_INT56:
    int 0x56
    ret

_INT57:
    int 0x57
    ret

_INT58:
    int 0x58
    ret

_INT59:
    int 0x59
    ret

_INT5A:
    int 0x5A
    ret

_INT5B:
    int 0x5B
    ret

_INT5C:
    int 0x5C
    ret

_INT5D:
    int 0x5D
    ret

_INT5E:
    int 0x5E
    ret

_INT5F:
    int 0x5F
    ret

_INT60:
    int 0x60
    ret

_INT61:
    int 0x61
    ret

_INT62:
    int 0x62
    ret

_INT63:
    int 0x63
    ret

_INT64:
    int 0x64
    ret

_INT65:
    int 0x65
    ret

_INT66:
    int 0x66
    ret

_INT67:
    int 0x67
    ret

_INT68:
    int 0x68
    ret

_INT69:
    int 0x69
    ret

_INT6A:
    int 0x6A
    ret

_INT6B:
    int 0x6B
    ret

_INT6C:
    int 0x6C
    ret

_INT6D:
    int 0x6D
    ret

_INT6E:
    int 0x6E
    ret

_INT6F:
    int 0x6F
    ret

_INT70:
    int 0x70
    ret

_INT71:
    int 0x71
    ret

_INT72:
    int 0x72
    ret

_INT73:
    int 0x73
    ret

_INT74:
    int 0x74
    ret

_INT75:
    int 0x75
    ret

_INT76:
    int 0x76
    ret

_INT77:
    int 0x77
    ret

_INT78:
    int 0x78
    ret

_INT79:
    int 0x79
    ret

_INT7A:
    int 0x7A
    ret

_INT7B:
    int 0x7B
    ret

_INT7C:
    int 0x7C
    ret

_INT7D:
    int 0x7D
    ret

_INT7E:
    int 0x7E
    ret

_INT7F:
    int 0x7F
    ret

_INT80:
    int 0x80
    ret

_INT81:
    int 0x81
    ret

_INT82:
    int 0x82
    ret

_INT83:
    int 0x83
    ret

_INT84:
    int 0x84
    ret

_INT85:
    int 0x85
    ret

_INT86:
    int 0x86
    ret

_INT87:
    int 0x87
    ret

_INT88:
    int 0x88
    ret

_INT89:
    int 0x89
    ret

_INT8A:
    int 0x8A
    ret

_INT8B:
    int 0x8B
    ret

_INT8C:
    int 0x8C
    ret

_INT8D:
    int 0x8D
    ret

_INT8E:
    int 0x8E
    ret

_INT8F:
    int 0x8F
    ret

_INT90:
    int 0x90
    ret

_INT91:
    int 0x91
    ret

_INT92:
    int 0x92
    ret

_INT93:
    int 0x93
    ret

_INT94:
    int 0x94
    ret

_INT95:
    int 0x95
    ret

_INT96:
    int 0x96
    ret

_INT97:
    int 0x97
    ret

_INT98:
    int 0x98
    ret

_INT99:
    int 0x99
    ret

_INT9A:
    int 0x9A
    ret

_INT9B:
    int 0x9B
    ret

_INT9C:
    int 0x9C
    ret

_INT9D:
    int 0x9D
    ret

_INT9E:
    int 0x9E
    ret

_INT9F:
    int 0x9F
    ret

_INTA0:
    int 0xA0
    ret

_INTA1:
    int 0xA1
    ret

_INTA2:
    int 0xA2
    ret

_INTA3:
    int 0xA3
    ret

_INTA4:
    int 0xA4
    ret

_INTA5:
    int 0xA5
    ret

_INTA6:
    int 0xA6
    ret

_INTA7:
    int 0xA7
    ret

_INTA8:
    int 0xA8
    ret

_INTA9:
    int 0xA9
    ret

_INTAA:
    int 0xAA
    ret

_INTAB:
    int 0xAB
    ret

_INTAC:
    int 0xAC
    ret

_INTAD:
    int 0xAD
    ret

_INTAE:
    int 0xAE
    ret

_INTAF:
    int 0xAF
    ret

_INTB0:
    int 0xB0
    ret

_INTB1:
    int 0xB1
    ret

_INTB2:
    int 0xB2
    ret

_INTB3:
    int 0xB3
    ret

_INTB4:
    int 0xB4
    ret

_INTB5:
    int 0xB5
    ret

_INTB6:
    int 0xB6
    ret

_INTB7:
    int 0xB7
    ret

_INTB8:
    int 0xB8
    ret

_INTB9:
    int 0xB9
    ret

_INTBA:
    int 0xBA
    ret

_INTBB:
    int 0xBB
    ret

_INTBC:
    int 0xBC
    ret

_INTBD:
    int 0xBD
    ret

_INTBE:
    int 0xBE
    ret

_INTBF:
    int 0xBF
    ret

_INTC0:
    int 0xC0
    ret

_INTC1:
    int 0xC1
    ret

_INTC2:
    int 0xC2
    ret

_INTC3:
    int 0xC3
    ret

_INTC4:
    int 0xC4
    ret

_INTC5:
    int 0xC5
    ret

_INTC6:
    int 0xC6
    ret

_INTC7:
    int 0xC7
    ret

_INTC8:
    int 0xC8
    ret

_INTC9:
    int 0xC9
    ret

_INTCA:
    int 0xCA
    ret

_INTCB:
    int 0xCB
    ret

_INTCC:
    int 0xCC
    ret

_INTCD:
    int 0xCD
    ret

_INTCE:
    int 0xCE
    ret

_INTCF:
    int 0xCF
    ret

_INTD0:
    int 0xD0
    ret

_INTD1:
    int 0xD1
    ret

_INTD2:
    int 0xD2
    ret

_INTD3:
    int 0xD3
    ret

_INTD4:
    int 0xD4
    ret

_INTD5:
    int 0xD5
    ret

_INTD6:
    int 0xD6
    ret

_INTD7:
    int 0xD7
    ret

_INTD8:
    int 0xD8
    ret

_INTD9:
    int 0xD9
    ret

_INTDA:
    int 0xDA
    ret

_INTDB:
    int 0xDB
    ret

_INTDC:
    int 0xDC
    ret

_INTDD:
    int 0xDD
    ret

_INTDE:
    int 0xDE
    ret

_INTDF:
    int 0xDF
    ret

_INTE0:
    int 0xE0
    ret

_INTE1:
    int 0xE1
    ret

_INTE2:
    int 0xE2
    ret

_INTE3:
    int 0xE3
    ret

_INTE4:
    int 0xE4
    ret

_INTE5:
    int 0xE5
    ret

_INTE6:
    int 0xE6
    ret

_INTE7:
    int 0xE7
    ret

_INTE8:
    int 0xE8
    ret

_INTE9:
    int 0xE9
    ret

_INTEA:
    int 0xEA
    ret

_INTEB:
    int 0xEB
    ret

_INTEC:
    int 0xEC
    ret

_INTED:
    int 0xED
    ret

_INTEE:
    int 0xEE
    ret

_INTEF:
    int 0xEF
    ret

_INTF0:
    int 0xF0
    ret

_INTF1:
    int 0xF1
    ret

_INTF2:
    int 0xF2
    ret

_INTF3:
    int 0xF3
    ret

_INTF4:
    int 0xF4
    ret

_INTF5:
    int 0xF5
    ret

_INTF6:
    int 0xF6
    ret

_INTF7:
    int 0xF7
    ret

_INTF8:
    int 0xF8
    ret

_INTF9:
    int 0xF9
    ret

_INTFA:
    int 0xFA
    ret

_INTFB:
    int 0xFB
    ret

_INTFC:
    int 0xFC
    ret

_INTFD:
    int 0xFD
    ret

_INTFE:
    int 0xFE
    ret

_INTFF:
    int 0xFF
    ret

global cpuINT
cpuINT:
    push eax
    mov eax, [esp + 8]
    shl eax, 2
    add eax, _INTJumpTable
    mov eax, [eax]
    mov [esp + 8], eax
    pop eax
    jmp [esp + 4]

  segment .data
_INTJumpTable:
    dd _INT00, _INT01, _INT02, _INT03, _INT04, _INT05, _INT06, _INT07, _INT08, _INT09, _INT0A, _INT0B, _INT0C, _INT0D, _INT0E, _INT0F
    dd _INT10, _INT11, _INT12, _INT13, _INT14, _INT15, _INT16, _INT17, _INT18, _INT19, _INT1A, _INT1B, _INT1C, _INT1D, _INT1E, _INT1F
    dd _INT20, _INT21, _INT22, _INT23, _INT24, _INT25, _INT26, _INT27, _INT28, _INT29, _INT2A, _INT2B, _INT2C, _INT2D, _INT2E, _INT2F
    dd _INT30, _INT31, _INT32, _INT33, _INT34, _INT35, _INT36, _INT37, _INT38, _INT39, _INT3A, _INT3B, _INT3C, _INT3D, _INT3E, _INT3F
    dd _INT40, _INT41, _INT42, _INT43, _INT44, _INT45, _INT46, _INT47, _INT48, _INT49, _INT4A, _INT4B, _INT4C, _INT4D, _INT4E, _INT4F
    dd _INT50, _INT51, _INT52, _INT53, _INT54, _INT55, _INT56, _INT57, _INT58, _INT59, _INT5A, _INT5B, _INT5C, _INT5D, _INT5E, _INT5F
    dd _INT60, _INT61, _INT62, _INT63, _INT64, _INT65, _INT66, _INT67, _INT68, _INT69, _INT6A, _INT6B, _INT6C, _INT6D, _INT6E, _INT6F
    dd _INT70, _INT71, _INT72, _INT73, _INT74, _INT75, _INT76, _INT77, _INT78, _INT79, _INT7A, _INT7B, _INT7C, _INT7D, _INT7E, _INT7F
    dd _INT80, _INT81, _INT82, _INT83, _INT84, _INT85, _INT86, _INT87, _INT88, _INT89, _INT8A, _INT8B, _INT8C, _INT8D, _INT8E, _INT8F
    dd _INT90, _INT91, _INT92, _INT93, _INT94, _INT95, _INT96, _INT97, _INT98, _INT99, _INT9A, _INT9B, _INT9C, _INT9D, _INT9E, _INT9F
    dd _INTA0, _INTA1, _INTA2, _INTA3, _INTA4, _INTA5, _INTA6, _INTA7, _INTA8, _INTA9, _INTAA, _INTAB, _INTAC, _INTAD, _INTAE, _INTAF
    dd _INTB0, _INTB1, _INTB2, _INTB3, _INTB4, _INTB5, _INTB6, _INTB7, _INTB8, _INTB9, _INTBA, _INTBB, _INTBC, _INTBD, _INTBE, _INTBF
    dd _INTC0, _INTC1, _INTC2, _INTC3, _INTC4, _INTC5, _INTC6, _INTC7, _INTC8, _INTC9, _INTCA, _INTCB, _INTCC, _INTCD, _INTCE, _INTCF
    dd _INTD0, _INTD1, _INTD2, _INTD3, _INTD4, _INTD5, _INTD6, _INTD7, _INTD8, _INTD9, _INTDA, _INTDB, _INTDC, _INTDD, _INTDE, _INTDF
    dd _INTE0, _INTE1, _INTE2, _INTE3, _INTE4, _INTE5, _INTE6, _INTE7, _INTE8, _INTE9, _INTEA, _INTEB, _INTEC, _INTED, _INTEE, _INTEF
    dd _INTF0, _INTF1, _INTF2, _INTF3, _INTF4, _INTF5, _INTF6, _INTF7, _INTF8, _INTF9, _INTFA, _INTFB, _INTFC, _INTFD, _INTFE, _INTFF

segment .text

global cpuFXSave
cpuFXSave:
    mov eax, [esp + 4]
    fxsave [eax]
    ret

global cpuFXRstor
cpuFXRstor:
    mov eax, [esp + 4]
    fxrstor [eax]
    ret

global cpuInitFPU
cpuInitFPU:
    fninit
    fldcw [esp + 4]
    ret

global cpuEnableSSE
cpuEnableSSE:
    mov eax, cr0
    and eax, 0xFFFFFFFB
    or eax, 0x00000022
    mov cr0, eax
    mov eax, cr4
    or eax, 0x00000600
    mov cr4, eax
    ret

global cpuReadMSR
cpuReadMSR:
    mov ecx, [esp + 4]
    rdmsr
    ret

global cpuWriteMSR
cpuWriteMSR:
    mov ecx, [esp + 4]
    mov eax, [esp + 8]
    mov edx, [esp + 12]
    wrmsr
    ret

global cpuEnterUserMode
cpuEnterUserMode:
    cli
    push ebp
    mov ebp, esp
    mov eax, SEG_DATA32_USER
    mov ds, ax
    mov es, ax
    mov fs, ax
    ;mov eax, SEG_TLS
    mov gs, ax
    push SEG_DATA32_USER  ; SS
    push dword [ebp + 8]  ; ESP
    push 0x00000202       ; EFLAGS
    push SEG_CODE32_USER  ; CS
    push dword [ebp + 12] ; EIP
    ;hlt
    xor ebp, ebp ; clear frame pointer
    iretd
    mov esp, ebp
    pop ebp
    ret
