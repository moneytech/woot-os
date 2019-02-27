.text
.global __cp_begin
.hidden __cp_begin
.global __cp_end
.hidden __cp_end
.global __cp_cancel
.hidden __cp_cancel
.hidden __cancel
.global __syscall_cp_asm
.hidden __syscall_cp_asm
.type   __syscall_cp_asm,@function
__syscall_cp_asm:
    mov 4(%esp),%ecx
    pushl %ebx
    pushl %esi
    pushl %edi
    pushl %ebp
__cp_begin:
    movl (%ecx),%eax
    testl %eax,%eax
    jnz __cp_cancel
    movl %esp, %eax
    pushl 48(%eax)
    pushl 44(%eax)
    pushl 49(%eax)
    pushl 36(%eax)
    pushl 32(%eax)
    pushl 28(%eax)
    pushl 24(%eax)
    call __do_teh_syscall
    addl $28, %esp
__cp_end:
    popl %ebp
    popl %edi
    popl %esi
    popl %ebx
    ret
__cp_cancel:
    popl %ebp
    popl %edi
    popl %esi
    popl %ebx
    jmp __cancel

__do_teh_syscall:
    pushl %ebp
    movl %esp, %ebp
    sysenter
