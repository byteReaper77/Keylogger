
section .text
bits 64
global debug
extern ExitProcess

debug : 
    mov rax, gs:[0x60] 
    mov al, byte [rax + 0x02] 
    test al, al 
    jz noDebug
    mov rdi, 1
    call ExitProcess
    
    noDebug:
        mov rdi, 0
        call ExitProcess

