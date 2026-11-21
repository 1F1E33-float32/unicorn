EXTERN _setjmp: proc
PUBLIC _setjmp_wrapper
PUBLIC _longjmp_wrapper

_TEXT SEGMENT

_setjmp_wrapper PROC

; Why do we need this wrapper?
; Short answer: Windows default implementation of setjmp/longjmp is incompatible with generated code.
; A longer answer: https://blog.lazym.io/2020/09/21/Unicorn-Devblog-setjmp-longjmp-on-Windows/.

; From qemu os-win32 comments:
; > On w64, setjmp is implemented by _setjmp which needs a second parameter.
; > If this parameter is NULL, longjump does no stack unwinding.
; > That is what we need for QEMU. Passing the value of register rsp (default)
; > lets longjmp try a stack unwinding which will crash with generated code.
; It's true indeed, but MSVC doesn't has a setjmp signature which receives two arguements.
; Therefore, we add a wrapper to keep the second argument zero.
xor rdx, rdx
jmp _setjmp

_setjmp_wrapper ENDP

; Restore the context directly. CRT longjmp may resolve to a managed host's
; implementation and attempt to unwind generated code without unwind metadata.
_longjmp_wrapper PROC
    mov eax, edx
    test eax, eax
    jnz restore_context
    inc eax
restore_context:
    mov rbx, [rcx+08h]
    mov rbp, [rcx+18h]
    mov rsi, [rcx+20h]
    mov rdi, [rcx+28h]
    mov r12, [rcx+30h]
    mov r13, [rcx+38h]
    mov r14, [rcx+40h]
    mov r15, [rcx+48h]
    ldmxcsr dword ptr [rcx+58h]
    fldcw word ptr [rcx+5Ch]
    movdqu xmm6, [rcx+60h]
    movdqu xmm7, [rcx+70h]
    movdqu xmm8, [rcx+80h]
    movdqu xmm9, [rcx+90h]
    movdqu xmm10, [rcx+0A0h]
    movdqu xmm11, [rcx+0B0h]
    movdqu xmm12, [rcx+0C0h]
    movdqu xmm13, [rcx+0D0h]
    movdqu xmm14, [rcx+0E0h]
    movdqu xmm15, [rcx+0F0h]
    mov rsp, [rcx+10h]
    jmp qword ptr [rcx+50h]
_longjmp_wrapper ENDP

_TEXT ENDS

END
