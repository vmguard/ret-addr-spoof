; The strat:
;   RBX is not volatile -> the callee (funcPtr) preserves it across
;   its prologue/epilogue. You set RBX = real return address before
;   the tail call, overwrite the return address slot with a jmp-rbx
;   gadget from the target module, then jmp to funcPtr.
;
;   When funcPtr returns:
;     ret -> pops jmp-rbx gadget  (inside target module)
;     jmp rbx  -> jumps to RBX = real return address
;     RSP = entry_RSP + 8  
;   I don't really know how much to comment asm but just writing 
;   stuff out as I write the code helps me have a coherent mental
;   model of what the code is doing.

.data

ALIGN 8
PUBLIC proxy_call_returns
proxy_call_returns  QWORD 32 DUP(0)  ; FF E3 (jmp rbx) gadget addresses

.code

PUBLIC spoofcall_stub
spoofcall_stub PROC
    ; save non volatile registers 
    push    rbx
    push    rsi
    push    rdi
    push    rbp
    push    r12
    push    r13
    push    r14
    push    r15
    sub     rsp, 28h

    ; save arguments 
    ; RCX = funcPtr, RDX = MAGIC, R8 = user arg0, R9 = user arg1
    mov     r10, rcx ; r10 = funcPtr (volatile, survives pops)
    mov     r13, rdx ; r13 = MAGIC
    mov     r14, r8 ; r14 = user arg0
    mov     r15, r9 ; r15 = user arg1

    ; verify MAGIC
    cmp     r13d, 052A3450h
    jne     cleanup_and_ret
    test    r10, r10
    jz      cleanup_and_ret

    ; pick a jmp-rbx gadget 
    mov     rax, r10
    and     eax, 1Fh
    lea     rcx, proxy_call_returns
    mov     r11, [rcx + rax * 8] ; r11 = gadget (volatile, survives pops)

    ; read the original return address
    ; rax is free now, use it to hold real_ret (volatile)
    mov     rax, [rsp + 28h + 40h]   ; rax = [entry_RSP] = real ret addr

    ; remap user args for the real function
    mov     rcx, r14 ; arg0 -> RCX
    mov     rdx, r15 ; arg1 -> RDX

    ; destroy our frame 
    add     rsp, 28h
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbp
    pop     rdi
    pop     rsi
    pop     rbx ; RSP = entry_RSP

    ; check gadget
    test    r11, r11
    jz      direct_call

    ; spoof path 
    mov     [rsp], r11 ; [entry_RSP] = jmp-rbx gadget
    mov     rbx, rax ; rbx = real_ret (callee preserves RBX)
    jmp     r10  ; tail call to funcPtr

direct_call:
    ; no gadget -> direct tail call 
    jmp     r10

cleanup_and_ret:
    xor     eax, eax
    add     rsp, 28h
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbp
    pop     rdi
    pop     rsi
    pop     rbx
    ret

spoofcall_stub ENDP

; long story short, asm is pretty EVIL
END
