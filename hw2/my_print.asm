global	my_print

section .text
my_print:
        push    ebp
        mov     ebp, esp
        push    edx
        push    ecx
        push    eax
        mov     edx, 1
        mov     ecx, [ebp+8]
        mov     ebx, 1
        mov     eax, 4
.l1:
        cmp     byte[ecx], 0
        jz      .end
        int     80h
        mov     eax, 4
        inc     ecx
        jmp     .l1
.end:
        pop     eax
        pop     ecx
        pop     edx
        pop     ebp
        ret