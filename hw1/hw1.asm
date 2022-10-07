section .bss
str1:   resb  100
op1:    resb  30
op2:    resb  30
res:    resb  1000
flag1:  resb  1
flag2:  resb  1
operator resb 1
tem_char resb 1
tem_str: resb 1000
section .data
invalid_str:  db "invalid__"
ten:     dd  10
lf:      db  10
section .text

global  _start

memset: ;memset(ptr: eax, c: bl, n: ecx)
    push ecx
.l1:
    cmp  ecx, 0
    jz   .finished
    mov  byte[eax], bl
    inc  eax
    sub  ecx, 1
    jmp  .l1
.finished:
    pop  ecx
    ret



strlen:      ;strlen函数，eax传入字符串指针，eax返回字符串长度
    push ebx
    mov  ebx, eax   ;首地址存放于ebx中
_loop:
    cmp  byte[eax], 0
    jz   end_of_loop
    inc  eax
    jmp  _loop
end_of_loop:
    sub  eax, ebx
    pop  ebx
    ret

read_op:    ;read_op(str: eax, op: ebx, flag: ecx)->eax
    cmp  byte[eax], '0'
    jl   .label1 ;小于'0',判断是否与'-'相等
    cmp  byte[eax], '9'
    jle   read_continue ;既小于等于9也大于等于0，继续读取
.label1:
    cmp  byte[eax], '-'
    jne  return_null ;不等于'-'返回空指针
    jmp  read_continue
return_null:
    mov  eax, 0
    ret
read_continue:
    mov  byte[ecx], 1
    cmp  byte[eax], '-'
    jne  not_change_flag
change_flag:
    inc  eax
    mov  byte[ecx], 0
not_change_flag:
    push edx ;edx做循环计数器
    mov  edx, 0
.loop_1: ;循环直到遇到非数字字符为止
    cmp  byte[eax], '0'
    jl   .finished_1
    cmp  byte[eax], '9'
    jg   .finished_1
    push ecx
    mov  cl, byte[eax] 
    mov  byte[ebx+edx], cl ;将输入字符串内容写入操作数字符串
    pop  ecx
    inc  edx
    inc  eax
    jmp  .loop_1
.finished_1:
    mov  byte[ebx+edx], 0
    pop  edx
    ret

reverse:  ;reverse(str: eax)
    push ebx
    push ecx
    mov  ebx, 0
    push eax
    call strlen
    mov  ecx, eax
    sub  ecx, 1
    pop  eax
.l1:
    cmp  ebx, ecx
    jnl  .finished
    push edx
    mov  dl, byte[eax+ebx] ;进行交换
    mov  byte[tem_char], dl
    mov  dl, byte[eax+ecx]
    mov  byte[eax+ebx], dl
    mov  dl, byte[tem_char]
    mov  byte[eax+ecx], dl
    pop  edx
    inc  ebx
    sub  ecx, 1
    jmp  .l1
.finished:
    pop ecx
    pop ebx
    ret
    
func_add: ;add(op1: eax, op2: ebx) op2 = op1+op2
    mov  ebp, esp
    push eax
    call strlen
    push eax ;l1 ebp-8
    mov  eax, ebx
    call strlen
    push eax ;l2 ebp-12
    mov  eax, [esp+8]
    push ecx
    push edx
    mov  ecx, 0 ;ecx用作循环计数 i
    mov  edx, 0 ;edx用于进位 carry
.l1: ;while(i<l1&&i<l2)
    cmp  ecx, [ebp-8]
    jnl  .l2
    cmp  ecx, [ebp-12]
    jnl  .l2
    push ecx ;ecx临时存放两书相加取模的结果
    add  dl, byte[eax+ecx]
    add  dl, byte[ebx+ecx]
    sub  edx, '0'
    sub  edx, '0'
    push eax
    mov  eax, edx
    mov  edx, 0
    div  dword[ten] ;eax/10 quotient->eax, remainder->edx
    mov  ecx, edx
    mov  edx, eax
    pop  eax
    add  ecx, '0'
    push eax
    mov  eax, [esp+4]
    mov  byte[ebx+eax], cl
    pop  eax
    pop  ecx
    inc  ecx
    jmp  .l1
.l2:
    cmp  ecx, [ebp-8]
    jnl  .l3
    push ecx ;ecx临时存放两数相加取模的结果
    add  dl, byte[eax+ecx]
    sub  edx, '0'
    push eax
    mov  eax, edx
    mov  edx, 0
    div  dword [ten] ;eax/10 quotient->eax, remainder->edx
    mov  ecx, edx
    mov  edx, eax
    pop  eax
    add  ecx, '0'
    push eax
    mov  eax, [esp+4]
    mov  byte[ebx+eax], cl
    pop  eax
    pop  ecx
    inc  ecx
    jmp  .l2
.l3:
    cmp  ecx, [ebp-12]
    jnl  .l4
    push ecx ;ecx临时存放两数相加取模的结果
    add  dl, byte[ebx+ecx]
    sub  edx, '0'
    push eax
    mov  eax, edx
    mov  edx, 0
    div  dword[ten] ;eax/10 quotient->eax, remainder->edx
    mov  ecx, edx
    mov  edx, eax
    pop  eax
    add  ecx, '0'
    push eax
    mov  eax, [esp+4]
    mov  byte[ebx+eax], cl
    pop  eax
    pop  ecx
    inc  ecx
    jmp  .l3
.l4:
    cmp  edx, 0
    jz   .finish
    add  edx, '0'
    mov  byte[ebx+ecx], dl
    inc  ecx 
.finish:
    mov  byte[ebx+ecx+1], 0;添加\0
    pop  edx
    pop  ecx
    pop  ebp
    pop  ebp
    pop  eax
    ret

func_mul: ;mul(op1: eax, op2: ebx, res: ecx)
    push eax
    mov  eax, tem_str
    push ebx
    mov  ebx, 48
    push ecx
    mov  ecx, 100
    call memset
    mov  eax, [esp]
    mov  ebx, 0
    mov  ecx, 100
    call memset
    pop  ecx
    pop  ebx
    pop  eax
    mov  byte[ecx], '0'
    push edx
    mov  edx, eax
    call strlen
    push eax ;l1->[ebp]
    mov  ebp, esp
    mov  eax, ebx
    call strlen
    push eax ;l2->[ebp-4]
    mov  eax, edx
    mov  edx, 0 ;carry->edx
    push eax ;op1->[ebp-8]
    push ebx ;op2->[ebp-12]
    mov  eax, 0 ;i->eax
    mov  ebx, 0 ;j->ebx
.l1: ;for(i=0;i<l1;i++)
    cmp  eax, dword[ebp]
    jnl  .f1
    mov  ebx, 0
.l2: ;for(j=0;j<l2;j++)
    cmp  ebx, dword[ebp-4]
    jnl  .f2
    push ebx
    push eax
    push ecx
    push esi
    mov  ecx, eax
    mov  esi, ebx
    mov  eax, [ebp-8]
    mov  ebx, [ebp-12]
    mov  al, byte[eax+ecx]
    sub  al, '0'
    mov  bl, byte[ebx+esi]
    sub  bl, '0'
    mul  bl
    add  ax, dx
    mov  dx, 0
    div  word[ten] ;ax/10 r->dx, q->ax
    mov  byte[tem_str+ecx+esi], dl
    add  byte[tem_str+ecx+esi], '0'
    mov  edx, 0
    mov  dx, ax
    pop  esi
    pop  ecx
    pop  eax
    pop  ebx
    inc  ebx
    jmp  .l2
.f2:
    cmp  edx, 0
    jz  .carry
    push ebx
    mov  ebx, dword[ebp-4]
    mov  byte[tem_str+ebx+eax], dl
    add  byte[tem_str+ebx+eax], '0'
    mov  byte[tem_str+ebx+eax+1], 0
    pop  ebx
    jmp  .l3
.carry:
    push ebx
    mov  ebx, dword[ebp-4]
    mov  byte[tem_str+eax+ebx], 0
    pop  ebx
.l3:
    push ebp
    push eax
    push ebx
    mov  eax, tem_str
    mov  ebx, ecx
    call func_add
    mov  eax, tem_str
    mov  ebx, '0'
    push ecx
    mov  ecx, 100
    call memset
    pop  ecx
    pop  ebx
    pop  eax
    pop  ebp
    mov  edx, 0
    inc  eax
    jmp  .l1
.f1:
    pop  ebp
    pop  ebp
    pop  ebp
    pop  ebp
    pop  edx
    mov  ebp, 0
    ret


read:   ;read(str: eax, len: ebx) 读取一行
    pusha
    mov  ebp, eax
.l1:
    mov  eax, 3 ;sys_read
    mov  ebx, 0 ;stdin
    mov  ecx, tem_char ;临时字符存放区
    mov  edx, 1 ;读取一个字符
    int  80h
    cmp  byte[ecx], 10
    jz   .finished
    mov  cl, byte[ecx]
    mov  byte[ebp], cl
    inc  ebp
    jmp  .l1
.finished:
    mov  byte[ebp], 0
    popa
    ret


print:  ;print(str: eax)
    push ecx
    push edx
    push ebx
    mov  ecx, eax
    call strlen ;获取字符串长度
    mov  edx, eax
    mov  eax, 4
    mov  ebx, 1
    int  80h
    mov  eax, ecx; 恢复寄存器
    pop  ebx
    pop  edx
    pop  ecx
    ret

_start:
    mov  byte[invalid_str+7], 10
    mov  byte[invalid_str+8], 0
.l1:
    mov  eax, str1
    mov  ebx, 0
    mov  ecx, 100
    call memset
    mov  eax, op1
    mov  ecx, 30
    call memset
    mov  eax, op2
    call memset
    mov  eax, res
    mov  ecx, 100
    call memset
    mov  eax, str1
    mov  ebx, 100
    call read
    cmp  byte[eax], 'q'
    jz   exit
    mov  ebx, op1
    mov  ecx, flag1
    call read_op
    cmp  eax, 0
    jz   .invalid
    mov  bl, byte[eax]
    mov  byte[operator], bl
    cmp  byte[operator], '+'
    jz  .l2
    cmp  byte[operator], '*'
    jnz  .invalid
.l2:
    inc  eax
    mov  ebx, op2
    mov  ecx, flag2
    call read_op
    cmp  eax, 0
    jz   .invalid
    cmp  byte[eax], 0
    jnz  .invalid
    cmp  byte[operator], '*'
    jz   .l_mul
    mov  cl, byte[flag2]
    cmp  byte[flag1], cl
    jnz  .l_sub
    mov  eax, op1
    call reverse
    mov  eax, op2
    call reverse
    mov  eax, op1
    mov  ebx, op2
    call func_add
    cmp  byte[flag1], 1

    mov  eax, op2
    call reverse
    mov  eax, op2
    call strlen
    mov  byte[op2+eax], 10
    mov  byte[op2+eax+1], 0
    mov  eax, op2
    call print
    jmp .l1
.l_sub:
.l_mul:
    mov  eax, op1
    call reverse
    mov  eax, op2
    call reverse
    mov  eax, op1
    mov  ebx, op2
    mov  ecx, res
    call func_mul
    mov  eax, res
    call reverse
    mov  eax, res
    call print
    mov  eax, lf
    call print
    jmp  .l1
.invalid:
    mov  eax, invalid_str
    call print
    jmp  .l1 ;continue
    
exit:
    mov eax, 1
    mov ebx, 0
    int 80h
