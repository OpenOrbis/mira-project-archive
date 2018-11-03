.intel_syntax noprefix
.text

.global syscall1, syscall2, syscall3, syscall4, syscall5
.global _mmap

syscall:
    mov rax,rdi
    syscall
    ret

syscall1:
    mov rax,rdi
    mov rdi,rsi
    syscall
    ret

syscall2:
    mov rax,rdi
    mov rdi,rsi
    mov rsi,rdx
    syscall
    ret

syscall3:
    mov rax,rdi
    mov rdi,rsi
    mov rsi,rdx
    mov rdx,rcx
    syscall
    ret

syscall4:
    mov rax,rdi
    mov rdi,rsi
    mov rsi,rdx
    mov rdx,rcx
    mov r10,r8
    syscall
    ret

_mmap:
    mov rax,477
    syscall
    ret
